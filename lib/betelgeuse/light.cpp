#include <macs/macs.hpp>

#include "betelgeuse.hpp"

using namespace betelgeuse;
using namespace macs::types;


light::light(const char *atten_fnc):
    position("position", vec4()),
    direction("direction", vec3()),
    color("color", vec3(1.f, 1.f, 1.f)),
    distr_exp("distribution_exponent", 0.f),
    limit_angle_cos("limit_angle", -1.f),
    atten_par("attenuation_parameter", 0.f),
    shade(NULL),
    atten_func(atten_fnc)
{
}

light::~light(void)
{
    delete shade;
}
