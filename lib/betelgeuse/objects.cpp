#include <cstdio>
#include <cstdlib>
#include <list>

#include <macs/macs.hpp>

#include "betelgeuse.hpp"

using namespace betelgeuse;
using namespace macs::types;


object::object(const char *min_isct, const char *norm, const char *tang):
    isct(NULL),
    cur_trans("mat_transformation", mat4()),
    cur_inv_trans("mat_inverse_transformation", mat4()),
    cur_normal("mat_normal", mat3()),
    cur_color("color", vec3()),
    cur_ambient("ambient", vec3()),
    cur_r("roughness", 1.f),
    cur_p("isotropy", 1.f)
{
    if (tang != NULL)
        asprintf(&global_src, "#define HAS_TANGENTS\n"
                              "float min_intersection(vec3 start, vec3 dir)\n{\n%s\n}\n"
                              "vec3 get_normal(vec3 point)\n{\n%s\n}\n"
                              "vec3 get_tangent(vec3 point)\n{\n%s\n}\n",
                              min_isct, norm, tang);
    else
        asprintf(&global_src, "float min_intersection(vec3 start, vec3 dir)\n{\n%s\n}\n"
                              "vec3 get_normal(vec3 point)\n{\n%s\n}\n",
                              min_isct, norm);
}

object::~object(void)
{
    free(global_src);

    delete isct;
}

instance *object::instantiate(void)
{
    instance *i = new instance(this);
    i->mat.color = vec3(1.f, 1.f, 1.f);
    i->mat.ambient = vec3(0.f, 0.f, 0.f);
    i->mat.r = i->mat.p = 1.f;

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

