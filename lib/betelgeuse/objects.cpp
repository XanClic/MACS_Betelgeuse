#include <cstdio>
#include <cstdlib>
#include <list>

#include <macs/macs.hpp>

#include "betelgeuse.hpp"

using namespace betelgeuse;
using namespace macs::types;


#ifdef __WIN32
extern void asprintf(char **dst, const char *format, ...);
#endif


object::object(const char *min_isct, const char *uv, const char *norm, const char *tang):
    isct(NULL),
    cur_trans("mat_transformation", mat4()),
    cur_inv_trans("mat_inverse_transformation", mat4()),
    cur_normal("mat_normal", mat3()),
    cur_color_flat_tex("color_switch", false),
    cur_ambient_flat_tex("ambient_switch", false),
    cur_rp_flat_tex("rp_switch", false),
    cur_color_flat("color_flat", vec3()),
    cur_ambient_flat("ambient_flat", vec3()),
    cur_rp_flat("rp_flat", vec2())
{
    if (tang != NULL)
        asprintf(&global_src, "#define HAS_TANGENTS\n"
                              "float min_intersection(vec3 start, vec3 dir)\n{\n%s\n}\n"
                              "vec2 get_uv(vec3 point)\n{\n%s\n}\n"
                              "vec3 get_normal(vec3 point)\n{\n%s\n}\n"
                              "vec3 get_tangent(vec3 point)\n{\n%s\n}\n",
                              min_isct, uv, norm, tang);
    else
        asprintf(&global_src, "float min_intersection(vec3 start, vec3 dir)\n{\n%s\n}\n"
                              "vec2 get_uv(vec3 point)\n{\n%s\n}\n"
                              "vec3 get_normal(vec3 point)\n{\n%s\n}\n",
                              min_isct, uv, norm);
}

object::~object(void)
{
    free(global_src);

    delete isct;
}


instance *object::instantiate(void)
{
    instance *i = new instance(this);

    i->mat.color_texed = i->mat.ambient_texed = i->mat.rp_texed = false;

    i->mat.color.flat = vec3(1.f, 1.f, 1.f);
    i->mat.ambient.flat = vec3(0.f, 0.f, 0.f);
    i->mat.rp.flat = vec2(1.f, 1.f);


    insts.push_back(i);


    return i;
}


instance::instance(object *o):
    obj(o)
{
}

instance::~instance(void)
{
    obj->insts.remove(this);
}

void instance::update_transformation(void)
{
    inv_trans = trans.inv();
    normal = mat3(inv_trans.transposed());
}

