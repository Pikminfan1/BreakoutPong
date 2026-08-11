#pragma once

#include "ecs/types.hpp"
#include "ecs/Coordinator.hpp"
#include "ecs/Components.hpp"

class MovementSystem : public System
{
public:
    void Update(Coordinator &coord, double dt)
    {
        for (Entity entity : mEntities)
        {
            auto& entityPos = coord.GetComponent<Position>(entity);
            const auto& entityVel = coord.GetComponent<Velocity>(entity);

            entityPos.x += entityVel.dx * dt;
            entityPos.y += entityVel.dy * dt;
        }
    }
};