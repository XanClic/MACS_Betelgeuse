#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <string>
#include <vector>

#include <GL/gl.h>
#include <GL/glext.h>

#include "macs.hpp"
#include "macs-internals.hpp"

using namespace macs;
using namespace macs::types;


render::render(std::initializer_list<const in *> input, std::initializer_list<const out *> output, const char *global_src, const char *shared_src, ...)
{
    de = se = false;

    dcf = render::less;
    scf = render::not_equal;

    sref = smask = 0xFF;
    sosf = sodf = render::keep;
    sodp = render::replace;

    prg = NULL;


    glGenFramebuffers(1, &id);

    dbgprintf("[rnd%u] New render object created.\n", id);

    glBindFramebuffer(GL_FRAMEBUFFER, id);


    std::string final_src = "varying vec2 tex_coord;\n";


    for (auto obj: input)
    {
        std::string name = obj->i_name;

        switch (obj->i_type)
        {
            case in::t_texture:
                final_src += std::string("uniform sampler2D ") + name + ";\n#define " + name + " texture2D(" + name + ", tex_coord)\n";
                break;

            case in::t_texture_array:
            {
                char layers[6]; // FIXME: Overflow
                sprintf(layers, "%i", static_cast<const texture_array *>(obj)->elements);
                final_src += std::string("uniform sampler3D ") + name + ";\n#define " + name + "(layer) texture3D(" + name + ", vec3(tex_coord, float(layer) / " + layers + ".0))\n";
                break;
            }

            case in::t_vec3:
                final_src += std::string("#define ") + name + " " + static_cast<std::string>(**static_cast<const named<vec3> *>(obj)) + "\n";
                break;

            case in::t_vec4:
                final_src += std::string("#define ") + name + " " + static_cast<std::string>(**static_cast<const named<vec4> *>(obj)) + "\n";
                break;

            case in::t_mat3:
                final_src += std::string("uniform mat3 ") + name + ";\n";
                break;

            case in::t_mat4:
                final_src += std::string("uniform mat4 ") + name + ";\n";
                break;

            case in::t_float:
            {
                char float_val[16]; // FIXME
                sprintf(float_val, "%f", **static_cast<const named<float> *>(obj));
                final_src += std::string("#define ") + name + " " + float_val + "\n";
                break;
            }

            default: // This should never happen
                throw exc::inv_type;
        }
    }


    int i = 0;

    for (auto obj: output)
    {
        switch (obj->o_type)
        {
            case out::t_texture:
                dbgprintf("[rnd%u] texture “%s” is on attachment %i.\n", id, obj->o_name, i);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, static_cast<const texture *>(obj)->id, 0);

                char tmp[6]; // FIXME: Overflow
                sprintf(tmp, "%i", i++);
                final_src += std::string("#define ") + obj->o_name + " gl_FragData[" + tmp + "]\n";

                break;

            case out::t_stencildepth:
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER, static_cast<const stencildepth *>(obj)->id);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, static_cast<const stencildepth *>(obj)->id);

                final_src += "#define depth gl_FragDepth\n";

                break;

            default: // This should never happen
                throw exc::inv_type;
        }
    }


    // FIXME
    if (i > internals::draw_bufs)
        throw exc::rsrc_lim_exc;


    final_src += std::string(global_src) + "\nvoid main(void)\n{\n" + shared_src + "\n";


    va_list va;
    va_start(va, shared_src);

    for (auto obj: output)
        final_src += std::string(obj->o_name) + " = " + va_arg(va, const char *) + ";\n";

    va_end(va);


    final_src += "}\n";


    dbgprintf("[rnd%u] Final source:\n%s\n", id, final_src.c_str());


    delete prg;


    internals::shader *sh = new internals::shader(internals::shader::fragment);
    sh->load(final_src.c_str());

    if (!sh->compile())
    {
        delete sh;

        throw exc::shader_fail;
    }


    prg = new internals::program;
    prg->attach(internals::basic_vertex_shader);
    prg->attach(sh);

    if (!prg->link())
    {
        delete sh;
        delete prg;
        prg = NULL;

        throw exc::shader_fail;
    }

    delete sh;


    inp_objs.insert(inp_objs.begin(), input);
    out_objs.insert(out_objs.begin(), output);
}

render::~render(void)
{
    delete prg;

    dbgprintf("[rnd%u] Deleting render object.\n", id);

    glDeleteFramebuffers(1, &id);
}


void render::prepare(void)
{
    dbgprintf("[rnd%u] Preparing.\n", id);


    glBindFramebuffer(GL_FRAMEBUFFER, id);


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

    delete assigned;

    internals::tmu_mgr->update();



    GLenum *bufs = new GLenum[internals::draw_bufs];
    i = 0;

    for (auto obj: out_objs)
    {
        if (obj->o_type == out::t_texture)
        {
            bufs[i] = GL_COLOR_ATTACHMENT0 + i;
            i++;
        }
    }

    glDrawBuffers(i, bufs);

    delete bufs;

    dbgprintf("[rnd%u] Enabled drawing to %i buffer%s.\n", id, i, (i == 1) ? "" : "s");



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


    dbgprintf("[rnd%u] Putting shader into use.\n", id);

    prg->use();


    dbgprintf("[rnd%u] Assigning uniforms.\n", id);

    for (auto obj: inp_objs)
        if ((obj->i_type == in::t_texture) || (obj->i_type == in::t_texture_array))
            prg->uniform(obj->i_name) = obj;
}


void render::execute(void)
{
    dbgprintf("[rnd%u] Drawing quad.\n", id);
    internals::draw_quad();
}


void render::clear_output(formats::f0123 value)
{
    glClearColor(value.r, value.g, value.b, value.a);
    glClear(GL_COLOR_BUFFER_BIT);
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



void macs::render_to_screen(bool backbuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDrawBuffer(backbuffer ? GL_BACK : GL_FRONT);
}
