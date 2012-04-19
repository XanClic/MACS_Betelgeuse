#include <list>

#include <GL/gl.h>

#include <macs/macs.hpp>

#include "betelgeuse.hpp"

using namespace betelgeuse;
using namespace macs;
using namespace macs::types;


scene::scene(void):
    output("output"),

    aspect(1.f),
    yfov("yfov", .57735f), // tan(30°) => 60° FOV
    xfov("xfov", .57735f), // == yfov  => aspect is 1

    zfar("zfar", 100.f),

    cam_pos("cam_pos", vec4(0.f, 0.f,  0.f, 1.f)),
    cam_fwd("cam_fwd", vec3(0.f, 0.f, -1.f)),
    cam_rgt("cam_rgt", vec3(1.f, 0.f,  0.f)),
    cam_up ("cam_up" , vec3(0.f, 1.f,  0.f)),

    ray_stt("ray_starting_points"), ray_dir("ray_directions"),
    glob_isct("global_intersection"), norm_map("normal_map"), tang_map("tangent_map"),
    color_map("color_map"), ambient_map("ambient_map"), rp_map("rp_map"),
    asten("stencil")

{
    rnd_view = new macs::render(
        { &cam_pos, &cam_fwd, &cam_rgt, &cam_up, &yfov, &xfov },
        { &ray_stt, &ray_dir, &asten, &sd },
        "", "",
        "cam_pos",
        "vec4(\n"
        "    normalize(\n"
        "        (tex_coord.x * 2. - 1.) * xfov * cam_rgt +\n"
        "        (tex_coord.y * 2. - 1.) * yfov * cam_up  +\n"
        "        cam_fwd\n"
        "    ),\n"
        "    0.\n"
        ")",
        "vec4(0., 0., 0., 0.)",
        "1."
    );

    rnd_view->use_depth(true, render::always);
}


void scene::set_fov(float fov)
{
    *xfov = (*yfov = fov) * aspect;
}

void scene::set_aspect(float asp)
{
    *xfov = (aspect = asp) * *yfov;
}


void scene::new_object_type(object *obj)
{
    objs.push_back(obj);

    obj->isct = new macs::render(
        { &ray_stt, &ray_dir, &zfar, &obj->cur_trans, &obj->cur_inv_trans, &obj->cur_normal,
          &obj->cur_color, &obj->cur_ambient, &obj->cur_r, &obj->cur_p },
        { &glob_isct, &norm_map, &tang_map, &color_map, &ambient_map, &rp_map, &asten, &sd },

        obj->global_src,

        "vec4 start = ray_starting_points;\n"
        "vec4 dir   = ray_directions;\n\n"
        "vec3 lstart = (mat_inverse_transformation * start).xyz;\n"
        "vec3 ldir   = (mat_inverse_transformation * dir  ).xyz;\n\n"
        "float par = min_intersection(lstart, ldir);\n\n"
        "if (par < .01)\n"
        "    discard;\n\n"
        "vec4 global_coord = start + par * dir;\n"
        "vec3 local_coord = lstart + par * ldir;\n\n"
        "vec3 n = normalize(mat_normal * get_normal(local_coord));\n"
        "#ifdef HAS_TANGENTS\n"
        "vec3 t = normalize((mat_transformation * vec4(get_tangent(local_coord), 0.)).xyz);\n"
        "#else\n"
        "vec3 t = vec3(0., 0., 0.);\n"
        "#endif\n\n"
        "float ndy = -dot(n, vec3(dir));\n"
        "if (ndy == 0.)\n"
        "    discard;\n\n"
        "else if (ndy < 0.)\n"
        "    n = -n;",

        "global_coord", "vec4(n, ndy)", "vec4(t, 0.)",
        "vec4(color, 0.)", "vec4(ambient, 0.)", "vec4(roughness, isotropy, 0., 0.)",
        "vec4(1., 0., 0., 0.)", "par / zfar"
    );

    obj->isct->use_depth(true);
}

void scene::add_light(light *lgt)
{
    lgts.push_back(lgt);

    char *global_src;
    asprintf(&global_src, "float attenuation(float distance)\n{\n%s\n}", lgt->atten_func);

    lgt->shade = new macs::render(
        { &glob_isct, &ray_dir, &norm_map, &tang_map, &color_map, &ambient_map, &rp_map, &asten,
          &lgt->position, &lgt->direction, &lgt->color, &lgt->distr_exp, &lgt->limit_angle_cos, &lgt->atten_par },
        { &output },

        global_src,

        "if (stencil.x < .5)\n"
        "    discard;\n\n"
        "vec3 g = global_intersection.xyz;\n"
        "vec4 ni = normal_map;\n"
        "vec3 n = ni.xyz;\n"
        "float ndoty = abs(ni.w);\n"
        "vec3 t = tangent_map.xyz;\n"
        "vec3 y = -ray_directions.xyz;\n\n"
        "vec3 x = position.xyz - g;\n\n"
        "float ndotx = dot(n, x);\n\n"
        "if (ndotx <= 0.)\n"
        "    discard;\n\n"
        "float dist = length(x);\n\n"
        "x = normalize(x);\n"
        "ndotx /= dist;\n\n"
        "float xdotr = -dot(x, direction);\n\n"
        "if (xdotr < limit_angle)\n"
        "    discard;\n\n"
        "vec3 n_ny = normalize(x + y);\n\n"
        "float ndotny_sqr = dot(n, n_ny);\n"
        "float xdotny = dot(x, n_ny);\n\n"
        "vec3 facet_proj = n_ny - ndotny_sqr * n;\n\n"
        "float costan_sqr;\n"
        "float facet_proj_sqr = dot(facet_proj, facet_proj);\n\n"
        "if (facet_proj_sqr == 0.)\n"
        "    costan_sqr = 0.;\n"
        "else\n"
        "{\n"
        "    costan_sqr = dot(facet_proj, t);\n"
        "    costan_sqr = (costan_sqr * costan_sqr) / facet_proj_sqr;\n"
        "}\n\n"
        "ndotny_sqr *= ndotny_sqr;\n\n"
        "vec3 point_color = attenuation(dist) * pow(xdotr, distribution_exponent) * color;\n\n"
        "float r = rp_map.x, p = rp_map.y;\n"
        "float psqr = p * p;\n"
        "float g_ndotx = ndotx / (r - r * ndotx + ndotx);\n"
        "float g_ndoty = ndoty / (r - r * ndoty + ndoty);\n\n"
        "float a = sqrt(p / (psqr - psqr * costan_sqr + costan_sqr));\n"
        "float z = 1. / (1. + r * ndotny_sqr - ndotny_sqr);\n\n"
        "z = r * (z * z);\n\n"
        "float layer = .31830989 * a * (1. + g_ndotx * g_ndoty * (z / (4. * ndotx * ndoty) - 1.));\n\n"
        "float fresnel_appr = pow(1. - xdotny, 5.);\n\n"
        "vec3 brdf = (color_map.rgb + (vec3(1., 1., 1.) - color_map.rgb) * fresnel_appr) * layer;",

        "vec4(point_color.r * brdf.r, point_color.g * brdf.g, point_color.b * brdf.b, 0.) * ndotx"
    );

    lgt->shade->blend_func(render::use, render::use);

    free(global_src);
}


void scene::render(void)
{
    render_view();
    render_intersection();
    render_shading();
}


void scene::render_view(void)
{
    rnd_view->prepare();
    rnd_view->execute();
}

void scene::render_intersection(void)
{
    for (auto obj: objs)
    {
        obj->isct->prepare();

        for (auto i: obj->insts)
        {
            *obj->cur_trans = i->trans;
            *obj->cur_inv_trans = i->inv_trans;
            *obj->cur_normal = i->normal;

            *obj->cur_color = i->mat.color;
            *obj->cur_ambient = i->mat.ambient;
            *obj->cur_r = i->mat.r;
            *obj->cur_p = i->mat.p;

            obj->isct->execute();
        }
    }
}

void scene::render_shading(void)
{
    bool first_light = true;

    for (auto lgt: lgts)
    {
        lgt->shade->prepare();

        if (first_light)
        {
            lgt->shade->clear_output({ 0.f, 0.f, 0.f, 0.f });
            first_light = false;
        }

        lgt->shade->execute();
    }
}

void scene::display(void)
{
    render_to_screen(_dbl_buf);

    output.display();
}

