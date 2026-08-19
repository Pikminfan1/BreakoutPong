#pragma once

#include "ecs/types.hpp"
#include "ecs/Coordinator.hpp"
#include "ecs/Components.hpp"

class BounceSystem : public System
{
public:
    void Update(Coordinator &coord, float screenW, float screenH)
    {
        for (Entity entity : mEntities)
        {
            auto &pos = coord.GetComponent<Position>(entity);
            auto &vel = coord.GetComponent<Velocity>(entity);
            const auto &rend = coord.GetComponent<Renderable>(entity);

            
            if (pos.x < 0.0f)
            {
                pos.x = 0.0f;
                vel.dx = -vel.dx;
            }
            else if (pos.x + rend.w > screenW)
            {
                pos.x = screenW - rend.w;
                vel.dx = -vel.dx;
            }

            if (pos.y < 0.0f)
            {
                pos.y = 0.0f;
                vel.dy = -vel.dy;
            }/*
            else if (pos.y + rend.h > screenH)
            {
                pos.y = screenH - rend.h;
                vel.dy = -vel.dy;
            }*/
        }
    }
};