#include <cstdlib>
#include <cstring>

#include <GL/gl.h>

#include "macs.hpp"
#include "macs-internals.hpp"


using namespace macs;


texture::texture(const char *n, int w, int h)
{
    width  = (w <= 0) ? internals::width  : w;
    height = (h <= 0) ? internals::height : h;

    i_type = in::t_texture;
    o_type = out::t_texture;

    i_name = o_name = strdup(n);

    glGenTextures(1, &id);

    (*internals::tmu_mgr)[0] = this;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
}

texture::~texture(void)
{
    glDeleteTextures(1, &id);

    free(const_cast<char *>(i_name));
}


#define texture_write(format, gl_format) \
    void texture::write(const formats::format *src) \
    { \
        (*internals::tmu_mgr)[0] = this; \
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, gl_format, GL_FLOAT, src); \
    }

texture_write(f0123, GL_RGBA)
texture_write(f2103, GL_BGRA)
texture_write(f012,  GL_RGB )
texture_write(f210,  GL_BGR )
texture_write(f0,    GL_RED )


#define texture_read(format, gl_format) \
    void texture::read(formats::format *dst) \
    { \
        (*internals::tmu_mgr)[0] = this; \
        glGetTexImage(GL_TEXTURE_2D, 0, gl_format, GL_FLOAT, dst); \
    }

texture_read(f0123, GL_RGBA)
texture_read(f2103, GL_BGRA)
texture_read(f012,  GL_RGB )
texture_read(f210,  GL_BGR )
texture_read(f0,    GL_RED )



void texture::display(void)
{
    (*internals::tmu_mgr)[0] = this;

    internals::basic_pipeline->use();
    internals::basic_pipeline->uniform("tex") = this;
    internals::draw_quad();
}
