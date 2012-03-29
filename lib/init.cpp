#include <GL/gl.h>

#include "macs.hpp"
#include "macs-internals.hpp"


using namespace macs;
using namespace macs::internals;

bool macs::init(void)
{
    // Check whether the system meets the requirements


    glGetIntegerv(GL_MAJOR_VERSION, &ogl_maj);
    glGetIntegerv(GL_MINOR_VERSION, &ogl_min);

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


    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &tex_units);

    dbgprintf("%i texture units encountered.\n", tex_units);

    if (tex_units < 1)
    {
        dbgprintf("At least one texture unit is required.\n");
        return false;
    }



    // Initialisation


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_COLOR);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);


    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClearDepth(1.f);


    glDisable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);


    return true;
}
