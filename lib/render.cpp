#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

#include <GL/gl.h>
#include <GL/glext.h>

#include "macs.hpp"
#include "macs-internals.hpp"

using namespace macs;


render::render(void):
    sd(NULL),
    dcf(render::less),
    scf(render::not_equal),
    sref(0xFF),
    smask(0xFF),
    sosf(render::keep),
    sodf(render::keep),
    sodp(render::replace),
    prg(NULL)
{
    glGenFramebuffers(1, &id);

    dbgprintf("[rnd%u] New render object created.\n", id);
}

render::~render(void)
{
    delete prg;

    dbgprintf("[rnd%u] Deleting render object.\n", id);

    glDeleteFramebuffers(1, &id);
}


void render::input(std::initializer_list<root *> objs)
{
    if (prg != NULL)
        throw exc::inv_exec_order;


    inp_objs.insert(inp_objs.begin(), objs);


#ifdef DEBUG
    dbgprintf("[rnd%u] Attaching input objects:\n", id);
    for (auto obj: objs)
    {
        switch (obj->type)
        {
            case root::texture:
                dbgprintf("[rnd%u] texture “%s”\n", id, ((texture *)obj)->name);
                break;
            case root::texture_array:
                dbgprintf("[rnd%u] texture array “%s”\n", id, ((texture_array *)obj)->name);
                break;
            case root::stencildepth:
                dbgprintf("[rnd%u] stencil/depth buffer\n", id);
                break;
            default:
                dbgprintf("[rnd%u] Unknown object type\n", id);
                break;
        }
    }
#endif
}


void render::output(std::initializer_list<root *> objs)
{
    if (prg != NULL)
        throw exc::inv_exec_order;


    out_objs.insert(out_objs.begin(), objs);


#ifdef DEBUG
    dbgprintf("[rnd%u] Attaching output objects:\n", id);
    for (auto obj: objs)
    {
        switch (obj->type)
        {
            case root::texture:
                dbgprintf("[rnd%u] texture “%s”\n", id, ((texture *)obj)->name);
                break;
            case root::texture_array:
                dbgprintf("[rnd%u] texture array “%s”\n", id, ((texture_array *)obj)->name);
                break;
            case root::stencildepth:
                dbgprintf("[rnd%u] stencil/depth buffer\n", id);
                break;
            default:
                dbgprintf("[rnd%u] Unknown object type\n", id);
                break;
        }
    }
#endif
}


void render::bind(void)
{
    dbgprintf("[rnd%u] Bound.\n", id);

    glBindFramebuffer(GL_FRAMEBUFFER, id);
}


bool render::compile(const char *src)
{
    dbgprintf("[rnd%u] Compiling.\n", id);


    std::string final_src = "varying vec2 tex_coord;\n";


    for (auto obj: inp_objs)
    {
        switch (obj->type)
        {
            case root::texture:
            {
                std::string name = ((texture *)obj)->name;
                final_src += "sampler2D " + name + ";\n#define " + name + " texture2D(" + name + ", tex_coord)\n";
                break;
            }
            case root::texture_array:
            {
                std::string name = ((texture_array *)obj)->name;
                char layers[6]; // FIXME: Overflow
                sprintf(layers, "%i", ((texture_array *)obj)->elements);
                final_src += "sampler3D " + name + ";\n#define " + name + "(layer) texture3D(" + name + ", vec3(tex_coord, float(layer) / " + std::string(layers) + ".0))\n";
                break;
            }
            default:
                throw exc::inv_type;
        }
    }


    int i = 0;

    for (auto obj: out_objs)
    {
        switch (obj->type)
        {
            case root::texture:
            {
                dbgprintf("[rnd%u] texture “%s” is on attachment %i.\n", id, ((texture *)obj)->name, i);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, ((texture *)obj)->id, 0);

                char tmp[6]; // FIXME: Overflow
                sprintf(tmp, "%i", i++);
                final_src += "#define " + std::string(((texture *)obj)->name) + " gl_FragData[" + std::string(tmp) + "]\n";
                break;
            }
            case root::stencildepth:
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER, ((stencildepth *)obj)->id);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ((stencildepth *)obj)->id);

                final_src += "#define depth gl_FragDepth\n";
                break;

            default:
                throw exc::inv_type;
        }
    }


    // FIXME
    if (i > internals::draw_bufs)
        throw exc::rsrc_lim_exc;


    final_src += src;


    dbgprintf("[rnd%u] Final source:\n%s\n", id, final_src.c_str());


    delete prg;


    internals::shader *sh = new internals::shader(internals::shader::fragment);
    sh->load(final_src.c_str());

    if (!sh->compile())
    {
        delete sh;

        return false;
    }


    prg = new internals::program;
    prg->attach(internals::basic_vertex_shader);
    prg->attach(sh);

    if (!prg->link())
    {
        delete sh;
        delete prg;
        prg = NULL;

        return false;
    }

    delete sh;


    dbgprintf("[rnd%u] Success.\n", id);

    return true;
}


void render::prepare(void)
{
    dbgprintf("[rnd%u] Preparing.\n", id);


    bool *assigned = new bool[inp_objs.size()];


    internals::tmu_mgr->loosen();

    int i = 0;
    for (auto obj: inp_objs)
        assigned[i++] = *internals::tmu_mgr &= obj;

    i = 0;
    for (auto obj: inp_objs)
        if (!assigned[i++])
            *internals::tmu_mgr += obj;

    delete assigned;

    internals::tmu_mgr->update();



    GLenum *bufs = new GLenum[internals::draw_bufs];
    i = 0;

    for (auto obj: out_objs)
    {
        if (obj->type == root::texture)
        {
            bufs[i] = GL_COLOR_ATTACHMENT0 + i;
            i++;
        }
    }

    glDrawBuffers(i, bufs);

    delete bufs;

    dbgprintf("[rnd%u] Enabled drawing to %i buffer%s.\n", id, i, (i == 1) ? "" : "s");



    if (de)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if (se)
        glEnable(GL_STENCIL_TEST);
    else
        glDisable(GL_STENCIL_TEST);


    glDepthFunc(dcf);
    glStencilFunc(scf, sref, smask);
    glStencilOp(sosf, sodf, sodp);


    dbgprintf("[rnd%u] Putting shader into use.\n", id);

    prg->use();


    dbgprintf("[rnd%u] Assigning uniforms.\n", id);

    for (auto obj: inp_objs)
    {
        switch (obj->type)
        {
            case root::texture:
                prg->uniform(((texture *)obj)->name) = (texture *)obj;
                break;
            case root::texture_array:
                prg->uniform(((texture_array *)obj)->name) = (texture_array *)obj;
                break;
        }
    }
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



void macs::render_to_screen(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDrawBuffer(GL_BACK);
}
