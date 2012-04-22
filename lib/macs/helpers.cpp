#include <cstring>

#include "macs.hpp"
#include "macs-internals.hpp"


void macs::internals::draw_quad(void)
{
    glBegin(GL_QUADS);
    glVertex2f(-1.f,  1.f);
    glVertex2f(-1.f, -1.f);
    glVertex2f( 1.f, -1.f);
    glVertex2f( 1.f,  1.f);
    glEnd();
}


void macs::opengl_version(int &major, int &minor)
{
    major = macs::internals::ogl_maj;
    minor = macs::internals::ogl_min;
}

int macs::max_output_textures(void)
{
    return macs::internals::draw_bufs;
}

int macs::max_input_textures(void)
{
    return macs::internals::tex_units;
}
