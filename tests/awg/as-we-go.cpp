#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <string>
#include <sys/time.h>

#include "SDL.h"

#include <macs/macs.hpp>
#include <betelgeuse/betelgeuse.hpp>


#define DOUBLE_BUF true

#define RESOLUTION 512


macs::texture *tex_from_bitmap(const char *path)
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

    if ((bpp != 24) && (bpp != 32))
        return NULL;


    bool neg_h = h < 0;

    if (neg_h)
        h = -h;

    macs::formats::f2103 *buf = new macs::formats::f2103[w * h];

    if (neg_h)
    {
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                if (bpp == 24)
                    buf[y * w + x] = macs::formats::f2103({ fgetc(fp) / 255.f, fgetc(fp) / 255.f, fgetc(fp) / 255.f, 0.f });
                else
                    buf[y * w + x] = macs::formats::f2103({ fgetc(fp) / 255.f, fgetc(fp) / 255.f, fgetc(fp) / 255.f, fgetc(fp) / 255.f });
            }
        }
    }
    else
    {
        for (int y = h - 1; y >= 0; y--)
        {
            for (int x = 0; x < w; x++)
            {
                if (bpp == 24)
                    buf[y * w + x] = macs::formats::f2103({ fgetc(fp) / 255.f, fgetc(fp) / 255.f, fgetc(fp) / 255.f, 0.f });
                else
                    buf[y * w + x] = macs::formats::f2103({ fgetc(fp) / 255.f, fgetc(fp) / 255.f, fgetc(fp) / 255.f, fgetc(fp) / 255.f });
            }
        }
    }

    fclose(fp);


    macs::texture *t = new macs::texture("color_tex", false, w, h);
    t->write(buf);

    delete[] buf;


    return t;
}


extern "C" int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;


    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, DOUBLE_BUF);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_SetVideoMode(RESOLUTION, RESOLUTION, 32, SDL_OPENGL | (DOUBLE_BUF * SDL_DOUBLEBUF));


    if (!betelgeuse::init(RESOLUTION, RESOLUTION, DOUBLE_BUF))
        return 1;


    betelgeuse::scene rts;


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

    betelgeuse::object quad(
        "float i = -start.z / dir.z;\n\n"
        "if ((i < 0.) || (abs(start.x + i * dir.x) > .5) || (abs(start.y + i * dir.y) > .5))\n"
        "    discard;\n\n"
        "return i;",

        "float i = -start.z / dir.z;\n\n"
        "return (i > 0.) && (i < 1.) && (abs(start.x + i * dir.x) <= .5) && (abs(start.y + i * dir.y) < .5);",

        "return vec2(point.x + .5, point.y + .5);",

        "return vec3(0., 0., 1.);"
    );

    rts.new_object_type(&sphere);
    rts.new_object_type(&quad);


    betelgeuse::instance *spi = sphere.instantiate();

    spi->mat.color.tex = tex_from_bitmap("tests/awg/earth.bmp");
    spi->mat.color_texed = true;


    betelgeuse::instance *qui = quad.instantiate();

    qui->trans.translate(macs::types::vec3(0.f, 0.f, -10.f));
    qui->trans.scale(macs::types::vec3(20.f, 20.f, 1.f));
    qui->update_transformation();


    struct timeval tv_start, tv_end, tv_cur;

    gettimeofday(&tv_start, NULL);


    betelgeuse::light lgt1("return 1. / (attenuation_parameter * distance * distance);");
    *lgt1.position = macs::types::vec4(0.f, 2.f, 0.f, 1.f);
    *lgt1.atten_par = .01f;

    rts.add_light(&lgt1);


    betelgeuse::light lgt2("return 1. / (attenuation_parameter * distance * distance);");
    *lgt2.position = macs::types::vec4(2.f, -2.f, 0.f, 1.f);
    *lgt2.color = macs::types::vec3(0.f, .25f, 1.f);
    *lgt2.atten_par = .02f;

    rts.add_light(&lgt2);


    int frame_count = 0;


    bool quit = false;

    while (!quit)
    {
        gettimeofday(&tv_cur, NULL);

        unsigned long long usecs_gone = (tv_cur.tv_sec % 24) * 1000000ULL + tv_cur.tv_usec;


        *lgt1.color = macs::types::vec3(1.f, sinf(usecs_gone * static_cast<float>(M_PI) / 2000000.f) * .375f + .625f,
                                             sinf(usecs_gone * static_cast<float>(M_PI) / 2000000.f) * .500f + .500f);

        spi->mat.rp.flat[0] = sinf(usecs_gone * static_cast<float>(M_PI) / 8000000.f) * .49f + .51f;

        spi->trans = macs::types::mat4();
        spi->trans.translate(macs::types::vec3(sinf(usecs_gone * static_cast<float>(M_PI) / 1000000.f),
                                               cosf(usecs_gone * static_cast<float>(M_PI) / 1000000.f),
                                               -5.f));
        spi->trans.rotate(usecs_gone * static_cast<float>(M_PI) / 4000000.f, macs::types::vec3(1.f, .5f, 0.f));
        spi->trans.scale(macs::types::vec3(1.f, 1.5f, 1.f));
        spi->update_transformation();


        (*lgt2.position).z = sinf(usecs_gone * static_cast<float>(M_PI) / 6000000.f) * 10.f;


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
            if (evt.type == SDL_QUIT)
                quit = true;
    }


    return 0;
}
