#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdio>

int main(int argc, char *argv[])
{

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "SDL Window",
        800,
        600,
        0);

    if (!window)
    {
        SDL_Log("SDL create window failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        SDL_Log("SDL create renderer failed: %s", SDL_GetError());

        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN)
            { // a key was pressed
                if (event.key.key == SDLK_ESCAPE)
                { // ...specifically Escape
                    running = false;
                }
            }
        }
        // --- render: clear → present ---
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 200, 60, 60, 255);  // switch pen to red
        SDL_FRect rect = { 350.0f, 250.0f, 100.0f, 100.0f };  // x, y, w, h
        SDL_RenderFillRect(renderer, &rect);  // draw a filled rectangle

        // (draw things here later)
        SDL_RenderPresent(renderer);
    }
    return 0;
}