#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <list>
#include <string>

#include <GL/gl.h>
#include <GL/glext.h>

#include "macs.hpp"
#include "macs-internals.hpp"

using namespace macs;
using namespace macs::types;


render::render(std::initializer_list<const in *> input, std::initializer_list<const out *> output, const char *global_src, const char *shared_src, ...)
{
    de = se = false;

    dcf = render::less_or_equal;
    scf = render::not_equal;

    sref = smask = 0xFF;
    sosf = sodf = render::keep;
    sodp = render::replace;

    bfsrc = render::use;
    bfdst = render::discard;


    fbos = 0;
    for (auto obj: output)
        if (obj->o_type == out::t_texture)
            fbos++;


    fbos = (fbos + internals::draw_bufs - 1) / internals::draw_bufs;

    if (fbos < 1)
        fbos = 1;

    dbgprintf("[rnd?] Creating %i render object%s.\n", fbos, (fbos == 1) ? "" : "s");


    ids = new GLuint[fbos];
    prgs = new internals::program[fbos];


    glGenFramebuffers(fbos, ids);

    for (int i = 0; i < fbos; i++)
        dbgprintf("[rnd%u] Created.\n", ids[i]);


    std::string *final_src = new std::string[fbos];

    final_src[0] = "varying vec2 tex_coord;\n";


    for (auto obj: input)
    {
        std::string name = obj->i_name;

        switch (obj->i_type)
        {
            case in::t_texture:
                final_src[0] += std::string("uniform sampler2D ") + name + ";\n#define " + name + " texture2D(" + name + ", tex_coord)\n";
                break;

            case in::t_texture_array:
            {
                char layers[6]; // FIXME: Overflow
                sprintf(layers, "%i", static_cast<const texture_array *>(obj)->elements);
                final_src[0] += std::string("uniform sampler3D ") + name + ";\n#define " + name + "(layer) texture3D(" + name + ", vec3(tex_coord, float(layer) / " + layers + ".0))\n";
                break;
            }

            case in::t_vec3:
                // final_src[0] += std::string("#define ") + name + " " + static_cast<std::string>(**static_cast<const named<vec3> *>(obj)) + "\n";
                final_src[0] += std::string("uniform vec3 ") + name + ";\n";
                break;

            case in::t_vec4:
                // final_src[0] += std::string("#define ") + name + " " + static_cast<std::string>(**static_cast<const named<vec4> *>(obj)) + "\n";
                final_src[0] += std::string("uniform vec4 ") + name + ";\n";
                break;

            case in::t_mat3:
                final_src[0] += std::string("uniform mat3 ") + name + ";\n";
                break;

            case in::t_mat4:
                final_src[0] += std::string("uniform mat4 ") + name + ";\n";
                break;

            case in::t_float:
            {
                /*
                char float_val[16]; // FIXME
                sprintf(float_val, "%f", **static_cast<const named<float> *>(obj));
                final_src[0] += std::string("#define ") + name + " " + float_val + "\n";
                */
                final_src[0] += std::string("uniform float ") + name + ";\n";
                break;
            }

            default: // This should never happen
                throw exc::inv_type;
        }
    }


    for (int i = 1; i < fbos; i++)
        final_src[i] = final_src[0];


    glBindFramebuffer(GL_FRAMEBUFFER, ids[0]);

    int i = 0, cur_fbo_i = 0;

    for (auto obj: output)
    {
        switch (obj->o_type)
        {
            case out::t_texture:
                if (i == internals::draw_bufs)
                {
                    i = 0;
                    glBindFramebuffer(GL_FRAMEBUFFER, ids[++cur_fbo_i]);
                }

                dbgprintf("[rnd%u] texture “%s” is on attachment %i.\n", ids[cur_fbo_i], obj->o_name, i);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, static_cast<const texture *>(obj)->id, 0);

                char tmp[6]; // FIXME: Overflow
                sprintf(tmp, "%i", i++);
                final_src[cur_fbo_i] += std::string("#define ") + obj->o_name + " gl_FragData[" + tmp + "]\n";

                break;

            case out::t_stencildepth:
                for (int j = 0; j < fbos; j++)
                {
                    dbgprintf("[rnd%u] Attaching stencil/depth buffer.\n", ids[j]);

                    glBindFramebuffer(GL_FRAMEBUFFER, ids[j]);

                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER, static_cast<const stencildepth *>(obj)->id);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, static_cast<const stencildepth *>(obj)->id);

                    final_src[j] += "#define depth gl_FragDepth\n";
                }

                break;

            default: // This should never happen
                throw exc::inv_type;
        }
    }


    for (i = 0; i < fbos; i++)
        final_src[i] += std::string(global_src) + "\nvoid main(void)\n{\n" + shared_src + "\n";


    va_list va;
    va_start(va, shared_src);

    i = 0;
    int j = 0;
    for (auto obj: output)
    {
        if (obj->o_type == out::t_stencildepth)
        {
            const char *val = va_arg(va, const char *);

            for (int k = 0; k < fbos; k++)
                final_src[k] += std::string(obj->o_name) + " = " + val + ";\n";
        }
        else
        {
            final_src[j] += std::string(obj->o_name) + " = " + va_arg(va, const char *) + ";\n";

            if (++i == internals::draw_bufs)
            {
                i = 0;
                j++;
            }
        }
    }

    va_end(va);


    for (j = 0; j < fbos; j++)
    {
        final_src[j] += "}\n";

        dbgprintf("[rnd%u] Final source:\n%s\n", ids[j], final_src[j].c_str());
    }


    for (j = 0; j < fbos; j++)
    {
        internals::shader *sh = new internals::shader(internals::shader::fragment);
        sh->load(final_src[j].c_str());

        if (!sh->compile())
        {
            delete sh;

            throw exc::shader_fail;
        }


        prgs[j].attach(internals::basic_vertex_shader);
        prgs[j].attach(sh);

        if (!prgs[j].link())
        {
            delete sh;

            throw exc::shader_fail;
        }

        delete sh;
    }


    delete[] final_src;


    inp_objs.insert(inp_objs.begin(), input);
    out_objs.insert(out_objs.begin(), output);
}

render::~render(void)
{
    dbgprintf("[rnd%u..] Deleting render objects.\n", ids[0]);

    glDeleteFramebuffers(fbos, ids);


    delete[] ids;
    delete[] prgs;
}


void render::bind_fbo(int i)
{
    dbgprintf("[rnd%u] Binding.\n", ids[i]);

    glBindFramebuffer(GL_FRAMEBUFFER, ids[i]);


    GLenum *bufs = new GLenum[internals::draw_bufs];
    int j = 0, k = 0;

    for (auto obj: out_objs)
    {
        if (obj->o_type == out::t_texture)
        {
            if (k == i)
                bufs[j] = GL_COLOR_ATTACHMENT0 + j;

            j++;

            if (j == internals::draw_bufs)
            {
                if (++k > i)
                    break;

                j = 0;
            }
        }
    }

    glDrawBuffers(j, bufs);

    delete[] bufs;

    dbgprintf("[rnd%u] Enabled drawing to %i buffer%s.\n", ids[i], j, (j == 1) ? "" : "s");
}

void render::prepare(void)
{
    dbgprintf("[rnd%u..] Preparing.\n", ids[0]);


    bool *assigned = new bool[inp_objs.size()];


    internals::tmu_mgr->loosen();

    int i = 0;
    for (auto obj: inp_objs)
        if ((obj->i_type == in::t_texture) || (obj->i_type == in::t_texture_array))
            assigned[i++] = *internals::tmu_mgr &= static_cast<const textures_in *>(obj);

    i = 0;
    for (auto obj: inp_objs)
        if (!assigned[i++] && ((obj->i_type == in::t_texture) || (obj->i_type == in::t_texture_array)))
            *internals::tmu_mgr += static_cast<const textures_in *>(obj);

    delete[] assigned;

    internals::tmu_mgr->update();


    bind_fbo(0);



    if (!de)
        glDisable(GL_DEPTH_TEST);
    else
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(dcf);
    }

    if (!se)
        glDisable(GL_STENCIL_TEST);
    else
    {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(scf, sref, smask);
        glStencilOp(sosf, sodf, sodp);
    }


    if ((bfsrc == use) && (bfdst == discard))
        glDisable(GL_BLEND);
    else
    {
        glEnable(GL_BLEND);
        glBlendFunc(bfsrc, bfdst);
    }


    dbgprintf("[rnd%u] Putting shader into use.\n", ids[0]);

    prgs[0].use();


    freshly_prepared = true;
}

void render::execute(void)
{
    for (int i = 0; i < fbos; i++)
    {
        if (i || !freshly_prepared)
        {
            bind_fbo(i);

            dbgprintf("[rnd%u] Putting consecutive shader into use.\n", ids[i]);

            prgs[i].use();
        }


        dbgprintf("[rnd%u] Assigning uniforms.\n", ids[i]);

        for (auto obj: inp_objs)
              //if ((obj->i_type == in::t_texture) || (obj->i_type == in::t_texture_array) ||
              //    (obj->i_type == in::t_mat3)    || (obj->i_type == in::t_mat4))
                prgs[i].uniform(obj->i_name) = obj;


        dbgprintf("[rnd%u] Drawing quad.\n", ids[i]);
        internals::draw_quad();
    }


    if (fbos > 1)
        freshly_prepared = false;
}


void render::clear_output(formats::f0123 value)
{
    glClearColor(value.r, value.g, value.b, value.a);

    for (int i = 0; i < fbos; i++)
    {
        if (i || !freshly_prepared)
            bind_fbo(i);

        glClear(GL_COLOR_BUFFER_BIT);
    }


    if (fbos > 1)
        freshly_prepared = false;
}

void render::clear_depth(formats::f0 value)
{
    glClearDepth(value.r);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void render::clear_stencil(uint8_t value)
{
    glClearStencil(value);
    glClear(GL_STENCIL_BUFFER_BIT);
}


void render::use_depth(bool dt, render::comparison comp)
{
    de = dt;

    if (dt)
        dcf = comp;
}

void render::use_stencil(bool st, render::comparison comp)
{
    se = st;

    if (st)
        scf = comp;
}

void render::stencil_values(uint8_t ref, uint8_t mask)
{
    sref = ref; smask = mask;
}

void render::stencil_operation(render::stencil_op sf, render::stencil_op df, render::stencil_op dp)
{
    sosf = sf; sodf = df; sodp = dp;
}

void render::blend_func(render::blend_fact src, render::blend_fact dst)
{
    bfsrc = src; bfdst = dst;
}



void macs::render_to_screen(bool backbuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDrawBuffer(backbuffer ? GL_BACK : GL_FRONT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
}
