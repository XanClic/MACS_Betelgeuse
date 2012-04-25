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


object::object(const char *min_isct, const char *line_isct, const char *uv, const char *norm, const char *tang):
    isct(NULL),
    cur_trans("mat_transformation", mat4()),
    cur_inv_trans("mat_inverse_transformation", mat4()),
    cur_normal("mat_normal", mat3()),
    cur_ambient_flat_tex("ambient_switch", false),
    cur_mirror_flat_tex("mirror_switch", false),
    cur_refract_flat_tex("refract_switch", false),
    cur_ambient_flat("ambient_flat", vec3()),
    cur_mirror_flat("mirror_flat", vec3()),
    cur_refract_flat("refract_flat", vec4()),
    cur_color0_flat_tex("color0_switch", false),
    cur_rp0_flat_tex("rp0_switch", false),
    cur_color0_flat("color0_flat", vec3()),
    cur_rp0_flat("rp0_flat", vec2()),
    cur_color1_flat_tex("color1_switch", false),
    cur_rp1_flat_tex("rp1_switch", false),
    cur_color1_flat("color1_flat", vec3()),
    cur_rp1_flat("rp1_flat", vec2()),
    shadow(NULL)
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

    asprintf(&global_shadow_src, "bool line_intersects(vec3 start, vec3 dir)\n{\n%s\n}", line_isct);
}

object::~object(void)
{
    free(global_src);
    free(global_shadow_src);

    delete isct;
    delete shadow;
}


instance *object::instantiate(void)
{
    instance *i = new instance(this);

    i->mat.ambient_texed = i->mat.mirror_texed = i->mat.refract_texed = false;
    i->mat.layer[0].color_texed = i->mat.layer[0].rp_texed = false;
    i->mat.layer[1].color_texed = i->mat.layer[1].rp_texed = false;

    i->mat.ambient.flat = i->mat.mirror.flat = vec3(0.f, 0.f, 0.f);
    i->mat.refract.flat = vec4(0.f, 0.f, 0.f, 1.f);

    i->mat.layer[0].color.flat = vec3(1.f, 1.f, 1.f);
    i->mat.layer[0].rp.flat = vec2(1.f, 1.f);
    i->mat.layer[1].color.flat = vec3(0.f, 0.f, 0.f);
    i->mat.layer[1].rp.flat = vec2(1.f, 1.f);


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

