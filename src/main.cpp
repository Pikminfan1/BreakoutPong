
#include "ecs/EntityManager.hpp"
#include "ecs/ComponentArray.hpp"
#include "ecs/ComponentManager.hpp"
#include <iostream>
#include "ecs/systems/MovementSystem.hpp"
#include "ecs/systems/RenderSystem.hpp"
#include "ecs/SystemManager.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdio>
#include "ecs/Coordinator.hpp"
#include "ecs/Components.hpp"
#include "ecs/systems/InputSystem.hpp"

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

    // --- timing setup ---
    const double FIXED_DT = 1.0 / 60.0;               // physics step: exactly 1/60s
    Uint64 perfFreq = SDL_GetPerformanceFrequency();  // ticks per second (constant)
    Uint64 currentTime = SDL_GetPerformanceCounter(); // starting timestamp
    double accumulator = 0.0;                         // banked leftover time

    Coordinator world;
    world.Init();
    world.RegisterComponent<Position>();
    world.RegisterComponent<Velocity>();
    world.RegisterComponent<Renderable>();
    world.RegisterComponent<PlayerControlled>();

    auto movementSystem = world.RegisterSystem<MovementSystem>();
    {
        Signature sig;
        sig.set(world.GetComponentType<Position>());
        sig.set(world.GetComponentType<Velocity>());
        world.SetSystemSignature<MovementSystem>(sig);
    }

    auto renderSystem = world.RegisterSystem<RenderSystem>();
    {
        Signature sig;
        sig.set(world.GetComponentType<Position>());
        sig.set(world.GetComponentType<Renderable>());
        world.SetSystemSignature<RenderSystem>(sig);
    }

    auto inputSystem = world.RegisterSystem<InputSystem>();
    {
        Signature sig;
        sig.set(world.GetComponentType<Velocity>());
        sig.set(world.GetComponentType<PlayerControlled>());
        world.SetSystemSignature<InputSystem>(sig);
    }

    auto entity = world.CreateEntity();
    world.AddComponent(entity, Position{100.0f, 100.0f});
    world.AddComponent(entity, Velocity{0.0f, 0.0f});
    world.AddComponent(entity, Renderable{50.0f, 50.0f, 255, 255, 0, 255});
    world.AddComponent(entity, PlayerControlled{});

    bool running = true;
    while (running)
    {
        // --- measure real time elapsed since last frame ---
        Uint64 newTime = SDL_GetPerformanceCounter();
        double frameTime = (double)(newTime - currentTime) / (double)perfFreq;
        currentTime = newTime;

        // --- spiral-of-death / anti-tunneling clamp ---
        if (frameTime > 0.25)
        {
            frameTime = 0.25;
        }

        accumulator += frameTime; // renderer "PRODUCES" time

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

        // --- physics: consume time in exact FIXED_DT chunks ---
        while (accumulator >= FIXED_DT)
        {
            inputSystem->Update(world, FIXED_DT);   
            // updateSystems(world, FIXED_DT);   // placeholder — your ECS later
            movementSystem->Update(world, FIXED_DT);
            accumulator -= FIXED_DT;
        }

        // --- render: clear → present ---
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderClear(renderer);


        
        renderSystem->Update(world, renderer);
    

        // renderSystem(world, renderer);
        // (draw things here later)
        SDL_RenderPresent(renderer);
    }
    // --- cleanup ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}