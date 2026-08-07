#include "EntityManager.hpp"
#include <iostream>

EntityManager::EntityManager()
{
    // Initialize the queue with every possible entity ID (0 .. MAX_ENTITIES-1)
    for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
    {
        availableEntities.push(entity);
    }
    livingEntityCount = 0;
}

Entity EntityManager::CreateEntity()
{
    if (livingEntityCount >= MAX_ENTITIES)
    {
        std::cerr << "Error: Maximum number of entities reached." << std::endl;
        return Entity(); // NOTE: 0 is a valid ID; revisit sentinel later
    }
    if (availableEntities.empty())
    {
        std::cerr << "Error: No available entity IDs." << std::endl;
        return Entity();
    }

    Entity entity = availableEntities.front();
    availableEntities.pop();
    ++livingEntityCount;
    return entity;
}

void EntityManager::DestroyEntity(Entity entity)
{
    if (entity >= MAX_ENTITIES)
    {
        std::cerr << "Error: Invalid entity ID." << std::endl;
        return;
    }

    // Clear the destroyed entity's signature and recycle its ID
    entitySignatures[entity].reset();
    availableEntities.push(entity);
    --livingEntityCount;
}

void EntityManager::SetSignature(Entity entity, Signature signature)
{
    if (entity >= MAX_ENTITIES)
    {
        std::cerr << "Error: Invalid entity ID." << std::endl;
        return;
    }

    entitySignatures[entity] = signature;
}

Signature EntityManager::GetSignature(Entity entity) const
{
    if (entity >= MAX_ENTITIES)
    {
        std::cerr << "Error: Invalid entity ID." << std::endl;
        return Signature();
    }

    return entitySignatures[entity];
}