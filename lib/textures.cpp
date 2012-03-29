#include <GL/gl.h>

#include "macs.hpp"


using namespace macs;

texture::texture(int w, int h):
    width(w),
    height(h)
{
    glGenTextures(1, &id);
}

texture::~texture(void)
{
    glDeleteTextures(1, &id);
}


void texture::allocate(void)
{
    if (allocated)
        return;


    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

    allocated = true;
}


#define texture_write(format, gl_format) \
    void texture::write(const formats::format *src) \
    { \
        glBindTexture(GL_TEXTURE_2D, id); \
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width, height, 0, gl_format, GL_FLOAT, src); \
        allocated = true; \
    }

texture_write(f0123, GL_RGBA)
texture_write(f2103, GL_BGRA)
texture_write(f012,  GL_RGB )
texture_write(f210,  GL_BGR )
texture_write(f0,    GL_RED )


#define texture_read(format, gl_format) \
    void texture::read(formats::format *dst) \
    { \
        if (!allocated) \
            return; \
        glBindTexture(GL_TEXTURE_2D, id); \
        glGetTexImage(GL_TEXTURE_2D, 0, gl_format, GL_FLOAT, dst); \
    }

texture_read(f0123, GL_RGBA)
texture_read(f2103, GL_BGRA)
texture_read(f012,  GL_RGB )
texture_read(f210,  GL_BGR )
texture_read(f0,    GL_RED )



void texture::_assign(int tex_unit)
{
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(GL_TEXTURE_2D, id);

    unit = tex_unit;
}
