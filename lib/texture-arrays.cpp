#include <cstdlib>
#include <cstring>
#include <vector>

#include <GL/gl.h>

#include "macs.hpp"
#include "macs-internals.hpp"


using namespace macs;


texture_array::texture_array(const char *n, int c):
    elements(c)
{
    type = root::texture_array;

    name = strdup(n);

    glGenTextures(1, &id);

    (*internals::tmu_mgr)[0] = this;

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F_ARB, internals::width, internals::height, elements, 0, GL_RGBA, GL_FLOAT, NULL);
}

texture_array::~texture_array(void)
{
    glDeleteTextures(1, &id);

    free((void *)name);
}


#define tex_array_write(format, gl_format) \
    void texture_array::write(const formats::format *src) \
    { \
        (*internals::tmu_mgr)[0] = this; \
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, internals::width, internals::height, elements, gl_format, GL_FLOAT, src); \
    }

tex_array_write(f0123, GL_RGBA)
tex_array_write(f2103, GL_BGRA)
tex_array_write(f012,  GL_RGB )
tex_array_write(f210,  GL_BGR )
tex_array_write(f0,    GL_RED )


#define tex_array_read(format, gl_format) \
    void texture_array::read(formats::format *dst) \
    { \
        (*internals::tmu_mgr)[0] = this; \
        glGetTexImage(GL_TEXTURE_3D, 0, gl_format, GL_FLOAT, dst); \
    }

tex_array_read(f0123, GL_RGBA)
tex_array_read(f2103, GL_BGRA)
tex_array_read(f012,  GL_RGB )
tex_array_read(f210,  GL_BGR )
tex_array_read(f0,    GL_RED )
