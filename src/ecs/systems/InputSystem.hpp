#pragma once

#include "ecs/types.hpp"
#include "ecs/Coordinator.hpp"
#include "ecs/Components.hpp"
#include <SDL3/SDL_keyboard.h>

class InputSystem : public System
{
public:
    void Update(Coordinator &coord)
    {
        const bool *currentKeyState = SDL_GetKeyboardState(nullptr);
        for (Entity entity : mEntities)
        {

            //Controls For Paddle Movement
            auto& entityVel = coord.GetComponent<Velocity>(entity);
            if(currentKeyState[SDL_SCANCODE_LEFT])
            {
                entityVel.dx = -300.0f; // Move left
            }
            else if(currentKeyState[SDL_SCANCODE_RIGHT])
            {
                entityVel.dx = 300.0f; // Move right
            }
            else
            {
                entityVel.dx = 0.0f; // Stop horizontal movement
            }
        }
    }
};