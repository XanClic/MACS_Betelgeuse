#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <string>
#include <sys/time.h>

#include <SDL/SDL.h>

#include "macs.hpp"


#define DOUBLE_BUF


namespace hardray
{
    bool init(int width, int height)
    { return macs::init(width, height); }


    class scene;
    class object;

    struct material
    {
        macs::types::vec3 color, ambient;
        float r, p;
    };

    class instance
    {
        public:
            void update_transformation(void);

            macs::types::mat4 trans;

            material mat;


            friend class scene;

        private:
            macs::types::mat4 inv_trans;
            macs::types::mat3 normal;
    };

    class object
    {
        public:
            object(const char *min_isct, const char *norm, const char *tang = NULL);
            ~object(void);

            instance *instantiate(void);


            friend class scene;
            friend class instance;

        private:
            char *global_src;
            macs::render *isct;
            macs::types::named<macs::types::mat4> cur_trans, cur_inv_trans;
            macs::types::named<macs::types::mat3> cur_normal;
            macs::types::named<macs::types::vec3> cur_color, cur_ambient;
            macs::types::named<float> cur_r, cur_p;

            std::list<instance *> insts;
    };

    class light
    {
        public:
            light(const char *atten_func);
            ~light(void);

            macs::types::named<macs::types::vec4> position;
            macs::types::named<macs::types::vec3> direction, color;
            macs::types::named<float> distr_exp, limit_angle_cos, atten_par;


            friend class scene;

        private:
            macs::render *shade;

            const char *atten_func;
    };

    class scene
    {
        public:
            scene(void);

            void set_fov(float fov);
            void set_aspect(float aspect);

            void new_object_type(object *obj);
            void add_light(light *lgt);


            void render_view(void);
            void render_intersection(void);
            void display(void);


        private:
            float aspect;
            macs::types::named<float> yfov, xfov, zfar;

            macs::types::named<macs::types::vec4> cam_pos;
            macs::types::named<macs::types::vec3> cam_fwd, cam_rgt, cam_up;

            std::list<object *> objs;
            std::list<light *> lgts;

            macs::stencildepth sd;

            macs::texture ray_stt, ray_dir;
            macs::texture glob_isct, norm_map, tang_map;
            macs::texture color_map, ambient_map, rp_map;

            macs::texture asten; // artificial stencil

            macs::texture output;

            macs::render *rnd_view;
    };
}


int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);

#ifdef DOUBLE_BUF
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#else
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
#endif
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#ifdef DOUBLE_BUF
    SDL_SetVideoMode(512, 512, 32, SDL_OPENGL | SDL_DOUBLEBUF);
#else
    SDL_SetVideoMode(512, 512, 32, SDL_OPENGL);
#endif


    if (!hardray::init(512, 512))
        return 1;


    hardray::scene rts;


    hardray::object sphere(
        "if (length(cross(dir, start)) > length(dir))\n"
        "    discard;\n\n"
        "float a =  dot(dir  , dir  );\n"
        "float b =  dot(start, dir  )       / a;\n"
        "float c = (dot(start, start) - 1.) / a;\n\n"
        "float sq = sqrt(b * b - c);\n"
        "float t1 = -sq - b, t2 = sq - b;\n\n"
        "if (t1 < 0.)\n"
        "{\n"
        "    if (t2 < 0.)\n"
        "        discard;\n"
        "    return t2;\n"
        "}\n\n"
        "if ((t2 < 0.) || (t1 < t2))\n"
        "    return t1;\n\n"
        "return t2;",

        "return point;"
    );


    rts.new_object_type(&sphere);

    hardray::instance *spi = sphere.instantiate();


    struct timeval tv_start, tv_end, tv_cur;

    gettimeofday(&tv_start, NULL);


    hardray::light lgt("return 1. / (attenuation_parameter * distance * distance);");
    *lgt.position = macs::types::vec4(0.f, 2.f, 0.f, 1.f);
    *lgt.atten_par = .01f;

    rts.add_light(&lgt);


    int frame_count = 0;


    bool quit = false;

    while (!quit)
    {
        gettimeofday(&tv_cur, NULL);

        unsigned long usecs_gone = (tv_cur.tv_sec % 16) * 1000000UL + tv_cur.tv_usec;

        *lgt.color = macs::types::vec3(1.f, sinf(usecs_gone * static_cast<float>(M_PI) / 2000000.f) * .375f + .625f,
                                            sinf(usecs_gone * static_cast<float>(M_PI) / 2000000.f) * .500f + .500f);

        spi->mat.r = sinf(usecs_gone * static_cast<float>(M_PI) / 8000000.f) * .49f + .51f;

        spi->trans = macs::types::mat4();
        spi->trans.translate(macs::types::vec3(sinf(usecs_gone * static_cast<float>(M_PI) / 1000000.f),
                                               cosf(usecs_gone * static_cast<float>(M_PI) / 1000000.f),
                                               -5.f));
        spi->trans.rotate(usecs_gone * static_cast<float>(M_PI) / 4000000.f, macs::types::vec3(1.f, .5f, 0.f));
        spi->trans.scale(macs::types::vec3(1.f, 1.5f, 1.f));
        spi->update_transformation();

        rts.render_view();
        rts.render_intersection();
        rts.display();

        SDL_GL_SwapBuffers();

        if (++frame_count >= 100)
        {
            gettimeofday(&tv_end, NULL);

            long diff = tv_end.tv_usec - tv_start.tv_usec + (tv_end.tv_sec - tv_start.tv_sec) * 1000000L;
            printf("FPS: %g      \r", 100000000.f / diff);
            fflush(stdout);

            frame_count = 0;

            gettimeofday(&tv_start, NULL);
        }


        SDL_Event evt;

        while (SDL_PollEvent(&evt))
            if (evt.type == SDL_QUIT)
                quit = true;
    }


    return 0;
}


using namespace hardray;
using namespace macs;
using namespace macs::types;


scene::scene(void):
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
    asten("stencil"),
    output("output")

{
    rnd_view = new render(
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

    obj->isct = new render(
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

    lgt->shade = new render(
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


    bool first_light = true;

    for (auto lgt: lgts)
    {
        lgt->shade->prepare();

        if (first_light)
            lgt->shade->clear_output({ 0.f, 0.f, 0.f, 0.f });

        lgt->shade->execute();
    }
}

void scene::display(void)
{
#ifdef DOUBLE_BUF
    macs::render_to_screen(true);
#else
    macs::render_to_screen(false);
#endif

    glClear(GL_DEPTH_BUFFER_BIT);

    output.display();
}


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
    instance *i = new instance;
    i->mat.color = vec3(1.f, 1.f, 1.f);
    i->mat.ambient = vec3(0.f, 0.f, 0.f);
    i->mat.r = i->mat.p = 1.f;

    insts.push_back(i);

    return i;
}


void instance::update_transformation(void)
{
    inv_trans = trans.inv();
    normal = mat3(inv_trans.transposed());
}


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
