#pragma once

#include "ecs/types.hpp"
#include <array>
#include <cassert>

// --- Abstract base so the ComponentManager can store arrays of different T together ---
class IComponentArray
{
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray
{
public:
    ComponentArray()
    {
        sparse.fill(MAX_ENTITIES);
    }
    void InsertData(Entity entity, T component)
    {
        // Guard: entity must not already have this component.
        assert(sparse[entity] == MAX_ENTITIES && "Component added to same entity more than once.");


        // 1. new element goes at dense index == size
        denseEntities[size] = entity;
        // 2. write component into denseComponents[size]
        denseComponents[size] = component;
        // 3. write entity into denseEntities[size]
        // 4. record mapping: sparse[entity] = size
        sparse[entity] = size;
        // 5. ++size
        ++size;
    }
    // Detach the component from an entity (swap-and-pop to stay packed).
    void RemoveData(Entity entity)
    {
        // Guard: entity must currently have this component.
        assert(sparse[entity] != MAX_ENTITIES && "Removing non-existent component.");

        // (swap-and-pop):
        // 1. removedIndex = sparse[entity]
        // 2. lastIndex    = size - 1
        // 3. copy denseComponents[lastIndex] -> denseComponents[removedIndex]
        //    copy denseEntities[lastIndex]   -> denseEntities[removedIndex]
        // 4. movedEntity = denseEntities[removedIndex]  (the entity that was last)
        //    sparse[movedEntity] = removedIndex          (fix its pointer!)
        // 5. sparse[entity] = MAX_ENTITIES                (mark removed entity absent)
        // 6. --size
        size_t removedIndex = sparse[entity];
        size_t lastIndex = size - 1;

        denseComponents[removedIndex] = denseComponents[lastIndex];
        denseEntities[removedIndex] = denseEntities[lastIndex];

        Entity movedEntity = denseEntities[removedIndex];
        sparse[movedEntity] = removedIndex;
        sparse[entity] = MAX_ENTITIES;
        --size;
    }

    // Retrieve an entity's component (by reference, so systems can modify it).
    T &GetData(Entity entity)
    {
        assert(sparse[entity] != MAX_ENTITIES && "Retrieving non-existent component.");
        // Return the component at the dense index sparse[entity] points to
        return denseComponents[sparse[entity]];
    }

    // Called when an entity is destroyed — remove its data if it has any.
    void EntityDestroyed(Entity entity) override
    {
        // if this entity has the component (sparse[entity] != MAX_ENTITIES),
        //       call RemoveData(entity).
        if (sparse[entity] != MAX_ENTITIES)
        {
            RemoveData(entity);
        }
    }

private:
    std::array<T, MAX_ENTITIES> denseComponents;    // packed component data (systems iterate this)
    std::array<Entity, MAX_ENTITIES> denseEntities; // parallel: which entity owns each dense slot
    std::array<size_t, MAX_ENTITIES> sparse;        // entity ID -> dense index (or MAX_ENTITIES if absent)
    size_t size = 0;                                // number of live components (dense length)
};
