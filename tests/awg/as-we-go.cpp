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


    macs::texture *ray_stt = new macs::texture("ray_starting_points");
    macs::texture *ray_dir = new macs::texture("ray_directions");


    vec4 cam_pos(0.f, 0.f,  0.f, 1.f);
    vec3 cam_fwd(0.f, 0.f, -1.f);
    vec3 cam_rgt(1.f, 0.f,  0.f);
    vec3 cam_up (0.f, 1.f,  0.f);

    float yfov = tanf(45.f * static_cast<float>(M_PI) / 180.f);
    float aspect = 1.f;


    macs::render *rnd = new macs::render(
            { },
            { ray_stt, ray_dir },
            "#define cam_pos " + static_cast<std::string>(cam_pos) + "\n"
            "#define cam_fwd " + static_cast<std::string>(cam_fwd) + "\n"
            "#define cam_rgt " + static_cast<std::string>(cam_rgt) + "\n"
            "#define cam_up  " + static_cast<std::string>(cam_up ) + "\n"
            "void main(void)\n"
            "{\n"
            "    ray_starting_points = cam_pos;\n"
            "    ray_directions = vec4(\n"
            "        normalize(\n"
            "            (tex_coord.x * 2. - 1.) * %f * cam_rgt +\n"
            "            (tex_coord.y * 2. - 1.) * %f * cam_up  +\n"
            "            cam_fwd\n"
            "        ),\n"
            "        1.\n"
            "    );\n"
            "}",
            yfov * aspect, yfov
    );

    rnd->prepare();
    rnd->execute();


    macs::render_to_screen(true);

    delete rnd;


    bool quit = false;

    while (!quit)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        ray_dir->display();
        SDL_GL_SwapBuffers();


        SDL_Event evt;

        while (SDL_PollEvent(&evt))
            if (evt.type == SDL_QUIT)
                quit = true;
    }


    return 0;
}
