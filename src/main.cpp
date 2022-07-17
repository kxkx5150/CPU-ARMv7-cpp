#define SDL_MAIN_HANDLED

#include "PC.h"

#include <cstdint>
#include <SDL2/SDL.h>
#include <cstdio>
#include <time.h>
#include <thread>
#include <bitset>

int  Running = 1;
void render_loop(PC *pc, SDL_Renderer *render, int width, int height)
{
    while (Running) {
    }
}
int main(int ArgCount, char **Args)
{
    static const int width = 160, height = 100;
    PC              *pc = new PC();
    pc->boot();

    SDL_Window *window =
        SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
    SDL_Renderer *render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(render, 1, 1);

    // BitOps::test();
    // auto nuuu = new Number64(0, 0);
    // nuuu->TestNumber64();
    // std::thread th(render_loop, pc, render, width, height);

    while (Running) {
        pc->run();
        SDL_Event Event;
        while (SDL_PollEvent(&Event)) {
            if (Event.type == SDL_QUIT)
                Running = 0;
        }
    }

    // th.join();
    return 0;
}
