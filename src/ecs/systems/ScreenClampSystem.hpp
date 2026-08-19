#pragma once

#include "ecs/types.hpp"
#include "ecs/Coordinator.hpp"
#include "ecs/Components.hpp"

class ScreenClampSystem : public System
{
public:
    void Update(Coordinator &coord, float screenWidth, float screenHeight)
    {
        for (Entity entity : mEntities)
        {
            auto &pos = coord.GetComponent<Position>(entity);
            const auto &rend = coord.GetComponent<Renderable>(entity);

            if (pos.x < 0.0f)
            {
                pos.x = 0.0f;
            }
            else if (pos.x > screenWidth - rend.w)
            {
                pos.x = screenWidth - rend.w;
            }

            if (pos.y < 0.0f)
            {
                pos.y = 0.0f;
            }
            else if (pos.y > screenHeight - rend.h)
            {
                pos.y = screenHeight - rend.h;
            }
        }
    }
};