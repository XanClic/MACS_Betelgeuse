#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <string>
#include <sys/time.h>

#include <SDL/SDL.h>

#include <betelgeuse/betelgeuse.hpp>


#define DOUBLE_BUF true

#define RESOLUTION 512


int main(void)
{
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

        "return point;"
    );


    rts.new_object_type(&sphere);

    betelgeuse::instance *spi = sphere.instantiate();


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

        unsigned long usecs_gone = (tv_cur.tv_sec % 24) * 1000000UL + tv_cur.tv_usec;


        *lgt1.color = macs::types::vec3(1.f, sinf(usecs_gone * static_cast<float>(M_PI) / 2000000.f) * .375f + .625f,
                                             sinf(usecs_gone * static_cast<float>(M_PI) / 2000000.f) * .500f + .500f);

        spi->mat.r = sinf(usecs_gone * static_cast<float>(M_PI) / 8000000.f) * .49f + .51f;

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
