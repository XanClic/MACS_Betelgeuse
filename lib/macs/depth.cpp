#include "macs.hpp"
#include "macs-internals.hpp"

using namespace macs;


stencildepth::stencildepth(void)
{
    o_type = out::t_stencildepth;

    o_name = "depth";

    glGenRenderbuffers(1, &id);
    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, internals::width, internals::height);
}

stencildepth::~stencildepth(void)
{
    glDeleteRenderbuffers(1, &id);
}


texture *stencildepth::depth_to_texture(const char *name)
{
    macs::texture *t = new macs::texture(name);

    (*internals::tmu_mgr)[0] = t;

    // Of course this is slow, but there's a reason we warn the user about just that.
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, internals::width, internals::height, 0);

    return t;
}
