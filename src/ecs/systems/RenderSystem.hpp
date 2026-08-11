#pragma once

#include "ecs/types.hpp"
#include "ecs/Coordinator.hpp"
#include "ecs/Components.hpp"
#include <SDL3/SDL.h>

class RenderSystem : public System
{
public:
    void Update(Coordinator &coord, SDL_Renderer *renderer)
    {
        for (Entity entity : mEntities)
        {
            // 1. get this entity's Position and Renderable
            // 2. SDL_SetRenderDrawColor(renderer, rend.r, rend.g, rend.b, 255)
            // 3. SDL_FRect rect { pos.x, pos.y, rend.w, rend.h }
            // 4. SDL_RenderFillRect(renderer, &rect)

            const auto& entityPos = coord.GetComponent<Position>(entity);
            const auto& entityRend = coord.GetComponent<Renderable>(entity);
            
            SDL_SetRenderDrawColor(renderer, entityRend.r, entityRend.g, entityRend.b, entityRend.a);
            SDL_FRect rect { entityPos.x, entityPos.y, entityRend.w, entityRend.h };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};