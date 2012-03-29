#include <SDL/SDL.h>

#include "macs.hpp"

int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_SetVideoMode(512, 512, 32, SDL_OPENGL | SDL_DOUBLEBUF);


    macs::init();


    return 0;
}
