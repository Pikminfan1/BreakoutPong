#pragma once

#include "types.hpp"
#include <queue>
#include <array>

class EntityManager
{
public:
    EntityManager();
    Entity CreateEntity();
    void DestroyEntity(Entity entity);
    void SetSignature(Entity entity, Signature signature);
    Signature GetSignature(Entity entity) const;

private:
    std::queue<Entity> availableEntities;                 // Pool (Queue) of available entity IDs
    std::array<Signature, MAX_ENTITIES> entitySignatures; // Array of entity signatures
    uint32_t livingEntityCount = 0;                           // Count of currently living entities
};