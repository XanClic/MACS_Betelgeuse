#include <cmath>
#include <cstdlib>
#include <string>

#include <boost/lexical_cast.hpp>

#include <SDL/SDL.h>

#include "macs.hpp"


using namespace macs::types;

int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_SetVideoMode(512, 512, 32, SDL_OPENGL | SDL_DOUBLEBUF);


    if (!macs::init(512, 512))
        return 1;


    named<vec4> cam_pos("cam_pos", vec4(0.f, 0.f,  0.f, 1.f));
    named<vec3> cam_fwd("cam_fwd", vec3(0.f, 0.f, -1.f));
    named<vec3> cam_rgt("cam_rgt", vec3(1.f, 0.f,  0.f));
    named<vec3> cam_up ("cam_up" , vec3(0.f, 1.f,  0.f));


    named<float> yfov("yfov", tanf(45.f * static_cast<float>(M_PI) / 180.f));
    named<float> xfov("xfov", *yfov * 1.f);


    macs::texture ray_stt("ray_starting_points");
    macs::texture ray_dir("ray_directions");


    macs::render rnd_view(
        { &cam_pos, &cam_fwd, &cam_rgt, &cam_up, &yfov, &xfov },
        { &ray_stt, &ray_dir },
        "", "",
        "cam_pos",
        "vec4(\n"
        "    normalize(\n"
        "        (tex_coord.x * 2. - 1.) * xfov * cam_rgt +\n"
        "        (tex_coord.y * 2. - 1.) * yfov * cam_up  +\n"
        "        cam_fwd\n"
        "    ),\n"
        "    1.\n"
        ")"
    );


    bool quit = false;

    while (!quit)
    {
        rnd_view.prepare();
        rnd_view.execute();

        macs::render_to_screen(true);

        ray_dir.display();

        SDL_GL_SwapBuffers();


        SDL_Event evt;

        while (SDL_PollEvent(&evt))
            if (evt.type == SDL_QUIT)
                quit = true;
    }


    return 0;
}
