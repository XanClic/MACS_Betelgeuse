#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "macs.hpp"
#include "macs-internals.hpp"


using namespace macs;
using namespace macs::internals;

bool macs::init(int width, int height)
{
#ifdef __WIN32
    glewInit();
#endif


    // Check whether the system meets the requirements


    glGetIntegerv(GL_MAJOR_VERSION, &ogl_maj);
    glGetIntegerv(GL_MINOR_VERSION, &ogl_min);

#ifdef __WIN32
    // FIXME
    if (!ogl_maj)
        ogl_maj = 2;
#endif

    dbgprintf("OpenGL version %i.%i found.\n", ogl_maj, ogl_min);

    if (ogl_maj < 2)
    {
        dbgprintf("OpenGL >= 2.0 required.\n");
        return false;
    }


    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &draw_bufs);

    dbgprintf("%i draw buffers detected.\n", draw_bufs);

    if (draw_bufs < 1)
    {
        dbgprintf("At least one draw buffer is required.\n");
        return false;
    }


    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &col_attach);

    dbgprintf("%i color buffer attachments allowed.\n", col_attach);

    if (col_attach < 1)
    {
        dbgprintf("At least one color buffer attachment is required.\n");
        return false;
    }


    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &tex_units);

    dbgprintf("%i texture units encountered.\n", tex_units);

    if (tex_units < 1)
    {
        dbgprintf("At least one texture unit is required.\n");
        return false;
    }



    // Initialisation


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClearDepth(1.f);


    glDisable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);



    internals::basic_vertex_shader = new internals::shader(internals::shader::vertex);


    FILE *fp = fopen("shaders/basic-vertex.glsl", "r");
    if (fp == NULL)
    {
        dbgprintf("Could not load basic vertex shader file: %s\n", strerror(errno));
        return false;
    }

    internals::basic_vertex_shader->load(fp);

    fclose(fp);


    if (!internals::basic_vertex_shader->compile())
    {
        dbgprintf("Could not compile the basic vertex shader.\n");
        return false;
    }



    shader *frag_sh = new internals::shader(internals::shader::fragment);


    fp = fopen("shaders/basic-fragment.glsl", "r");
    if (fp == NULL)
    {
        dbgprintf("Could not load basic fragment shader file: %s\n", strerror(errno));
        return false;
    }

    frag_sh->load(fp);

    fclose(fp);


    if (!frag_sh->compile())
    {
        dbgprintf("Could not compile the basic fragment shader.\n");
        return false;
    }



    internals::basic_pipeline = new internals::program;
    internals::basic_pipeline->attach(internals::basic_vertex_shader);
    internals::basic_pipeline->attach(frag_sh);

    if (!internals::basic_pipeline->link())
    {
        dbgprintf("Could not link basic pipeline.\n");
        return false;
    }


    delete frag_sh;



    glViewport(0, 0, width, height);



    internals::width = width;
    internals::height = height;

    internals::tmu_mgr = new internals::tmu_manager(tex_units);



    return true;
}
