#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <string>
#include <sys/time.h>

#include <SDL/SDL.h>

#include <macs/macs.hpp>
#include <betelgeuse/betelgeuse.hpp>


#ifndef M_PI
#define M_PI 3.141592f
#endif


#define DOUBLE_BUF true

#define RESOLUTION_X 1680
#define RESOLUTION_Y 1050


macs::texture *tex_from_bitmap(const char *as, const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return NULL;

    fseek(fp, 10, SEEK_SET);
    uint32_t ofs;
    fread(&ofs, sizeof(ofs), 1, fp);

    int32_t w, h;
    fseek(fp, 4, SEEK_CUR);
    fread(&w, sizeof(w), 1, fp);
    fread(&h, sizeof(h), 1, fp);

    uint16_t bpp;
    fseek(fp, 2, SEEK_CUR);
    fread(&bpp, sizeof(bpp), 1, fp);

    if ((bpp != 8) && (bpp != 24) && (bpp != 32))
        return NULL;


    fseek(fp, 0, SEEK_END);
    long lof = ftell(fp);
    fseek(fp, ofs, SEEK_SET);


    unsigned char *content = new unsigned char[lof - ofs];
    fread(content, lof - ofs, 1, fp);
    fclose(fp);


    bool neg_h = h < 0;

    if (neg_h)
        h = -h;

    macs::formats::f2103 *buf = new macs::formats::f2103[w * h];


    int i = 0;

    if (neg_h)
    {
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                switch (bpp)
                {
                    case 8: { int inp = content[i++]; buf[y * w + x] = macs::formats::f2103({ inp / 255.f, inp / 255.f, inp / 255.f, 0.f }); break; }
                    case 24: buf[y * w + x] = macs::formats::f2103({ content[i++] / 255.f, content[i++] / 255.f, content[i++] / 255.f, 0.f }); break;
                    case 32: buf[y * w + x] = macs::formats::f2103({ content[i++] / 255.f, content[i++] / 255.f, content[i++] / 255.f, content[i++] / 255.f }); break;
                }
            }
        }
    }
    else
    {
        for (int y = h - 1; y >= 0; y--)
        {
            for (int x = 0; x < w; x++)
            {
                switch (bpp)
                {
                    case 8: { int inp = content[i++]; buf[y * w + x] = macs::formats::f2103({ inp / 255.f, inp / 255.f, inp / 255.f, 0.f }); break; }
                    case 24: buf[y * w + x] = macs::formats::f2103({ content[i++] / 255.f, content[i++] / 255.f, content[i++] / 255.f, 0.f }); break;
                    case 32: buf[y * w + x] = macs::formats::f2103({ content[i++] / 255.f, content[i++] / 255.f, content[i++] / 255.f, content[i++] / 255.f }); break;
                }
            }
        }
    }


    macs::texture *t = new macs::texture(as, false, w, h);
    t->write(buf);

    delete[] buf;


    return t;
}


class planet
{
    public:
        planet(betelgeuse::object &base, const char *color_bmp, const char *rp_bmp, float year, float day, float distance, float radius);
        planet(betelgeuse::object &base, const char *color_bmp, float year, float day, float distance, float radius);
        planet(betelgeuse::object &base, const char *ambient_bmp, float day, float radius);
        ~planet(void);

        void update(float days_gone);


    private:
        betelgeuse::instance *inst;
        float d, y;
        float dist, rad;
};


planet::planet(betelgeuse::object &base, const char *color_bmp, const char *rp_bmp, float year, float day, float distance, float radius):
    d(day),
    y(year),
    dist(distance),
    rad(radius)
{
    inst = base.instantiate();

    inst->mat.layer[0].color.tex = tex_from_bitmap("color0_tex", color_bmp);
    inst->mat.layer[0].color_texed = true;

    inst->mat.layer[0].rp.tex = tex_from_bitmap("rp0_tex", rp_bmp);
    inst->mat.layer[0].rp_texed = true;
}

planet::planet(betelgeuse::object &base, const char *color_bmp, float year, float day, float distance, float radius):
    d(day),
    y(year),
    dist(distance),
    rad(radius)
{
    inst = base.instantiate();

    inst->mat.layer[0].color.tex = tex_from_bitmap("color0_tex", color_bmp);
    inst->mat.layer[0].color_texed = true;
}

planet::planet(betelgeuse::object &base, const char *ambient_bmp, float day, float radius):
    d(day),
    y(1.f),
    dist(0.f),
    rad(radius)
{
    inst = base.instantiate();

    inst->mat.ambient.tex = tex_from_bitmap("ambient_tex", ambient_bmp);
    inst->mat.ambient_texed = true;

    inst->cast_shadows = false;
}

planet::~planet(void)
{
    if (inst->mat.layer[0].color_texed)
        delete inst->mat.layer[0].color.tex;

    if (inst->mat.layer[0].rp_texed)
        delete inst->mat.layer[0].rp.tex;

    if (inst->mat.ambient_texed)
        delete inst->mat.ambient.tex;

    delete inst;
}

void planet::update(float days_gone)
{
    inst->trans = macs::types::mat4();
    inst->trans.translate(macs::types::vec3(-dist * 5.f * sinf(days_gone / y * 2.f * static_cast<float>(M_PI)),
                                             dist * 5.f * cosf(days_gone / y * 2.f * static_cast<float>(M_PI)),
                                            -10.f));
    inst->trans.scale(macs::types::vec3(rad, rad, rad));
    inst->trans.rotate(static_cast<float>(M_PI_2), macs::types::vec3(1.f, 0.f, 0.f));
    inst->trans.rotate(days_gone / d * 2.f * static_cast<float>(M_PI), macs::types::vec3(0.f, 1.f, 0.f));
    inst->update_transformation();
}


extern "C" int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;


    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, DOUBLE_BUF);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_SetVideoMode(RESOLUTION_X, RESOLUTION_Y, 32, SDL_OPENGL | (DOUBLE_BUF * SDL_DOUBLEBUF));

    SDL_WM_SetCaption("Planets - 1x", NULL);


    if (!betelgeuse::init(RESOLUTION_X, RESOLUTION_Y, DOUBLE_BUF))
        return 1;


    betelgeuse::scene rts;

    rts.set_aspect((float)RESOLUTION_X / RESOLUTION_Y);


    betelgeuse::object sphere(
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

        "if (length(cross(dir, start)) > length(dir))\n"
        "    return false;\n\n"
        "float a =  dot(dir  , dir  );\n"
        "float b =  dot(start, dir  )       / a;\n"
        "float c = (dot(start, start) - 1.) / a;\n\n"
        "float sq = sqrt(b * b - c);\n"
        "float t1 = -sq - b, t2 = sq - b;\n\n"
        "return (((t1 > 0.) && (t1 < 1.)) || ((t2 > 0.) && (t2 < 1.)));",

        "return vec2(1. - (atan(point.z, point.x) + 3.141592) / 6.283185, acos(point.y) / 3.141592);",

        "return point;"
    );

    rts.new_object_type(&sphere);


    planet mercury(sphere, "tests/planets/mercury.bmp", 87.969f, 58.65f, .3871f, .1f);
    planet venus  (sphere, "tests/planets/venus.bmp", 224.701f, -243.019f, .723f, .3f);
    planet earth  (sphere, "tests/planets/earth.bmp", "tests/planets/earth_spec.bmp", 365.256f, .9973f, 1.f, .3f);
    planet mars   (sphere, "tests/planets/mars.bmp", 686.98f, 1.026f, 1.524f, .15f);

    planet sun(sphere, "tests/planets/sun.bmp", 25.38f, .5f);


    struct timeval tv_start, tv_end, tv_cur, tv_last;

    gettimeofday(&tv_start, NULL);
    memcpy(&tv_last, &tv_start, sizeof(tv_last));


    betelgeuse::light lgt("return 1. / (attenuation_parameter * distance * distance);");
    *lgt.position = macs::types::vec4(0.f, 0.f, -10.f, 1.f);
    *lgt.atten_par = .01f;
    *lgt.color = macs::types::vec3(1.f, 1.f, .9f);

    rts.add_light(&lgt);


    int frame_count = 0;

    float days_per_second = 1.f / 86400.f;
    float days_gone = 0.f;


    bool quit = false;

    while (!quit)
    {
        gettimeofday(&tv_cur, NULL);
        unsigned time_gone = (tv_cur.tv_sec - tv_last.tv_sec) * 1000000 + tv_cur.tv_usec - tv_last.tv_usec;
        memcpy(&tv_last, &tv_cur, sizeof(tv_last));

        days_gone += time_gone / 1000000.f * days_per_second;

        for (planet *p: { &sun, &mercury, &venus, &earth, &mars })
            p->update(days_gone);


        rts.render();
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
        {
            if (evt.type == SDL_QUIT)
                quit = true;
            else if (evt.type == SDL_KEYUP)
            {
                switch (evt.key.keysym.sym)
                {
                    case SDLK_UP:
                        days_per_second *= 10.f;
                        break;
                    case SDLK_DOWN:
                        days_per_second /= 10.f;
                        break;
                }

                char tmp[64];
                sprintf(tmp, "Planets - %gx", days_per_second * 86400.f);

                SDL_WM_SetCaption(tmp, NULL);
            }
        }
    }


    return 0;
}
