#include <list>

#include <macs/macs.hpp>
#include <macs/macs-internals.hpp>

#include "betelgeuse.hpp"

using namespace betelgeuse;
using namespace macs;
using namespace macs::types;


#ifdef __WIN32
extern void asprintf(char **dst, const char *format, ...);
#endif


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
    ambient_map("ambient_map"), mirror_map("mirror_map"), refract_map("refract_map"), uv_map("uv_map"),
    color0_map("color0_map"), color1_map("color1_map"), rp_map("rp_map"),
    asten("stencil"),

    cur_light_pos("light_pos", vec4())

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


    rnd_ambient = new macs::render(
        { &ambient_map, &asten },
        { &output },
        "",
        "if (stencil.x < .5)\n"
        "    discard;",

        "ambient_map"
    );

    rnd_ambient->blend_func(render::use, render::use);
}

scene::~scene(void)
{
    delete rnd_view;
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

    texture_placebo amb_plac("ambient_tex"), mir_plac("mirror_tex"), ref_plac("refract_tex");
    texture_placebo co0_plac("color0_tex"), rp0_plac("rp0_tex"), co1_plac("color1_tex"), rp1_plac("rp1_tex");

    obj->isct = new macs::render(
        { &ray_stt, &ray_dir, &zfar, &obj->cur_trans, &obj->cur_inv_trans, &obj->cur_normal,
          &obj->cur_ambient_flat_tex, &obj->cur_mirror_flat_tex, &obj->cur_refract_flat_tex,
          &obj->cur_color0_flat_tex, &obj->cur_rp0_flat_tex,
          &obj->cur_color1_flat_tex, &obj->cur_rp1_flat_tex,
          &obj->cur_ambient_flat, &obj->cur_mirror_flat, &obj->cur_refract_flat,
          &obj->cur_color0_flat, &obj->cur_rp0_flat,
          &obj->cur_color1_flat, &obj->cur_rp1_flat,
          &amb_plac, &mir_plac, &ref_plac, &co0_plac, &rp0_plac, &co1_plac, &rp1_plac },
        { &glob_isct, &norm_map, &tang_map, &ambient_map, &mirror_map, &refract_map, &uv_map,
          &color0_map, &color1_map, &rp_map, &asten, &sd },

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
        "    n = -n;\n\n"
        "vec2 uv = get_uv(local_coord);\n\n"
        "vec3 point_ambient = ambient_switch ? texture2D(raw_ambient_tex, uv).xyz : ambient_flat;\n"
        "vec3 point_mirror  = mirror_switch  ? texture2D(raw_mirror_tex,  uv).xyz : mirror_flat;\n"
        "vec4 point_refract = refract_switch ? texture2D(raw_refract_tex, uv)     : refract_flat;\n"
        "vec3 point_color0  = color0_switch  ? texture2D(raw_color0_tex,  uv).xyz : color0_flat;\n"
        "vec2 point_rp0     = rp0_switch     ? texture2D(raw_rp0_tex,     uv).xy  : rp0_flat;\n"
        "vec3 point_color1  = color1_switch  ? texture2D(raw_color1_tex,  uv).xyz : color1_flat;\n"
        "vec2 point_rp1     = rp1_switch     ? texture2D(raw_rp1_tex,     uv).xy  : rp1_flat;",

        "global_coord", "vec4(n, ndy)", "vec4(t, 0.)",
        "vec4(point_ambient, 0.)",
        "vec4(point_mirror, 0.)",
        "     point_refract",
        "vec4(uv, 0., 0.)",
        "vec4(point_color0, 0.)",
        "vec4(point_color1, 0.)",
        "vec4(point_rp0, point_rp1)",
        "vec4(1., 0., 0., 0.)", "par / zfar"
    );

    obj->isct->use_depth(true);


    texture_placebo shadow_map_plac("shadow_map");

    obj->shadow = new macs::render(
        { &glob_isct, &cur_light_pos, &asten, &obj->cur_inv_trans },
        { &shadow_map_plac },

        obj->global_shadow_src,

        "if (stencil.x < .5)\n"
        "    discard;\n\n" // nobody cares anyway
        "vec4 dir_vec = global_intersection - light_pos;\n",

        "line_intersects((mat_inverse_transformation * light_pos).xyz,"
                        "(mat_inverse_transformation * dir_vec).xyz * .95)" // FIXME
                        "? vec4(1.f, 0.f, 0.f, 0.f) : vec4(0.f, 0.f, 0.f, 0.f)"
    );

    obj->shadow->blend_func(render::use, render::use);
}

void scene::add_light(light *lgt)
{
    lgts.push_back(lgt);

    char *global_src;
    asprintf(&global_src, "float attenuation(float distance)\n{\n%s\n}", lgt->atten_func);

    lgt->shade = new macs::render(
        { &glob_isct, &ray_dir, &norm_map, &tang_map, &ambient_map, &mirror_map, &refract_map, &uv_map,
          &color0_map, &color1_map, &rp_map, &asten, &lgt->shadow_map, &lgt->position, &lgt->direction,
          &lgt->color, &lgt->distr_exp, &lgt->limit_angle_cos, &lgt->atten_par },
        { &output },

        global_src,

        "if ((stencil.x < .5) || (shadow_map.x > .5))\n"
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
        "ndotny_sqr *= ndotny_sqr;\n\n\n"
        "vec3 point_color = attenuation(dist) * pow(xdotr, distribution_exponent) * color;\n\n"
        "float l0 = 0., l1 = 0.;\n"
        "if (color0_map.xyz != vec3(0., 0., 0.))\n"
        "{\n"
        "    float r = rp_map.x, p = rp_map.y;\n"
        "    float psqr = p * p;\n"
        "    float g_ndotx = ndotx / (r - r * ndotx + ndotx);\n"
        "    float g_ndoty = ndoty / (r - r * ndoty + ndoty);\n\n"
        "    float a = sqrt(p / (psqr - psqr * costan_sqr + costan_sqr));\n"
        "    float z = 1. / (1. + r * ndotny_sqr - ndotny_sqr);\n\n"
        "    z = r * (z * z);\n\n"
        "    l0 = .31830989 * a * (1. + g_ndotx * g_ndoty * (z / (4. * ndotx * ndoty) - 1.));\n\n\n"
        "    if (color1_map.xyz != vec3(0., 0., 0.))\n"
        "    {\n"
        "        r = rp_map.z; p = rp_map.w;\n"
        "        psqr = p * p;\n"
        "        g_ndotx = ndotx / (r - r * ndotx + ndotx);\n"
        "        g_ndotx = ndoty / (r - r * ndoty + ndoty);\n\n"
        "        a = sqrt(p / (psqr - psqr * costan_sqr + costan_sqr));\n"
        "        z = 1. / (1. + r * ndotny_sqr - ndotny_sqr);\n\n"
        "        z = r * (z * z);\n\n"
        "        l1 = .31830989 * a * (1. + g_ndotx * g_ndoty * (z / (4. * ndotx * ndoty) - 1.));\n"
        "    }\n"
        "}\n\n\n"
        "float fresnel_appr = pow(1. - xdotny, 5.);\n\n"
        "vec3 weight0 = color0_map.xyz + (vec3(1., 1., 1.) - color0_map.xyz) * fresnel_appr;\n"
        "vec3 weight1 = color1_map.xyz + (vec3(1., 1., 1.) - color1_map.xyz) * fresnel_appr;\n\n"
        "vec3 brdf = weight0 * l0 + (vec3(1., 1., 1.) - weight0) * weight1 * l1;",

        "vec4(point_color * brdf, 0.) * ndotx + vec4(ambient_map.xyz, 0.)"
    );

    lgt->shade->blend_func(render::use, render::use);

    free(global_src);
}


void scene::render(void)
{
    render_view();
    render_intersection();
    render_shadows();
    render_shading();
    render_ambient();
}


void scene::render_view(void)
{
    rnd_view->prepare();
    rnd_view->bind_input();
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

            if (i->mat.ambient_texed)           *obj->isct << i->mat.ambient.tex;
            else                                *obj->cur_ambient_flat = i->mat.ambient.flat;

            if (i->mat.mirror_texed)            *obj->isct << i->mat.mirror.tex;
            else                                *obj->cur_mirror_flat = i->mat.mirror.flat;

            if (i->mat.refract_texed)           *obj->isct << i->mat.refract.tex;
            else                                *obj->cur_refract_flat = i->mat.refract.flat;

            if (i->mat.layer[0].color_texed)    *obj->isct << i->mat.layer[0].color.tex;
            else                                *obj->cur_color0_flat = i->mat.layer[0].color.flat;

            if (i->mat.layer[0].rp_texed)       *obj->isct << i->mat.layer[0].rp.tex;
            else                                *obj->cur_rp0_flat = i->mat.layer[0].rp.flat;

            if (i->mat.layer[1].color_texed)    *obj->isct << i->mat.layer[1].color.tex;
            else                                *obj->cur_color1_flat = i->mat.layer[1].color.flat;

            if (i->mat.layer[1].rp_texed)       *obj->isct << i->mat.layer[1].rp.tex;
            else                                *obj->cur_rp1_flat = i->mat.layer[1].rp.flat;

            *obj->cur_ambient_flat_tex = i->mat.ambient_texed;
            *obj->cur_mirror_flat_tex = i->mat.mirror_texed;
            *obj->cur_refract_flat_tex = i->mat.refract_texed;
            *obj->cur_color0_flat_tex = i->mat.layer[0].color_texed;
            *obj->cur_rp0_flat_tex = i->mat.layer[0].rp_texed;
            *obj->cur_color1_flat_tex = i->mat.layer[1].color_texed;
            *obj->cur_rp1_flat_tex = i->mat.layer[1].rp_texed;

            obj->isct->bind_input();
            obj->isct->execute();

            if (i->mat.ambient_texed)           *obj->isct -= i->mat.ambient.tex;
            if (i->mat.mirror_texed)            *obj->isct -= i->mat.mirror.tex;
            if (i->mat.refract_texed)           *obj->isct -= i->mat.refract.tex;
            if (i->mat.layer[0].color_texed)    *obj->isct -= i->mat.layer[0].color.tex;
            if (i->mat.layer[0].rp_texed)       *obj->isct -= i->mat.layer[0].rp.tex;
            if (i->mat.layer[1].color_texed)    *obj->isct -= i->mat.layer[1].color.tex;
            if (i->mat.layer[1].rp_texed)       *obj->isct -= i->mat.layer[1].rp.tex;
        }
    }
}

void scene::render_shadows(void)
{
    bool first_instance = true;

    for (auto obj: objs)
    {
        obj->shadow->prepare();
        obj->shadow->bind_input();

        for (auto i: obj->insts)
        {
            if (!i->cast_shadows)
                continue;

            for (auto lgt: lgts)
            {
                *obj->shadow >> &lgt->shadow_map;

                if (first_instance)
                    obj->shadow->clear_output({ 0.f, 0.f, 0.f, 0.f });

                *cur_light_pos = *lgt->position;
                *obj->cur_inv_trans = i->inv_trans;

                obj->shadow->execute();

                *obj->shadow -= &lgt->shadow_map;
            }

            first_instance = false;
        }
    }
}

void scene::render_shading(void)
{
    bool first_light = true;

    for (auto lgt: lgts)
    {
        lgt->shade->prepare();
        lgt->shade->bind_input();

        if (first_light)
        {
            lgt->shade->clear_output({ 0.f, 0.f, 0.f, 0.f });
            first_light = false;
        }

        lgt->shade->execute();
    }
}

void scene::render_ambient(void)
{
    rnd_ambient->prepare();
    rnd_ambient->bind_input();
    rnd_ambient->execute();
}

void scene::display(void)
{
    render_to_screen(_dbl_buf);

    output.display();
}

