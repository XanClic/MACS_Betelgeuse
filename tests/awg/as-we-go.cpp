#include <SDL/SDL.h>

#include "macs.hpp"


int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_SetVideoMode(512, 512, 32, SDL_OPENGL | SDL_DOUBLEBUF);


    if (!macs::init(512, 512))
        return 1;


    macs::texture *tex = new macs::texture("tex");
    tex->allocate();


    macs::render *rnd = new macs::render;
    *rnd >> tex;

    rnd->bind();
    rnd->compile("void main(void) { tex = vec4(tex_coord, 0., 1.); }");

    rnd->prepare();
    rnd->execute();


    macs::render_to_screen();

    delete rnd;


    bool quit = false;

    while (!quit)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        tex->display();
        SDL_GL_SwapBuffers();


        SDL_Event evt;

        while (SDL_PollEvent(&evt))
            if (evt.type == SDL_QUIT)
                quit = true;
    }


    return 0;
}
