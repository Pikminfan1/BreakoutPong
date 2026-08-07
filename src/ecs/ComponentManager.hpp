#pragma once

#include "ecs/types.hpp"
#include "ecs/ComponentArray.hpp"
#include <memory>          // std::shared_ptr, std::static_pointer_cast
#include <unordered_map>
#include <typeinfo>        // typeid
#include <cassert>
#include <string>

class ComponentManager
{
public:
    // Register a component type so it can be used. Assigns it the next
    // ComponentType id (also its bit position in a Signature) and creates
    // its backing ComponentArray<T>.
    template <typename T>
    void RegisterComponent()
    {
        std::string typeName = typeid(T).name();

        assert(componentTypes.find(typeName) == componentTypes.end()
               && "Registering component type more than once.");

        // 1. record the type -> id mapping:   componentTypes[typeName] = nextComponentType;
        // 2. create its array and store it:    componentArrays[typeName] =
        //        std::make_shared<ComponentArray<T>>();
        // 3. advance the id counter:           ++nextComponentType;

        componentTypes[typeName] = nextComponentType;
        componentArrays[typeName] = std::make_shared<ComponentArray<T>>();
        ++nextComponentType;
    }

    // Return the ComponentType id (bit position) assigned to type T.
    template <typename T>
    ComponentType GetComponentType()
    {
        std::string typeName = typeid(T).name();

        assert(componentTypes.find(typeName) != componentTypes.end()
               && "Component type not registered before use.");

        //return the stored id for this type (componentTypes[typeName]).
        return componentTypes[typeName];
    }

    // Attach a component of type T to an entity (routes to the right array).
    template <typename T>
    void AddComponent(Entity entity, T component)
    {
        // get the ComponentArray<T> (see GetComponentArray helper below)
        // and call its InsertData(entity, component).

        GetComponentArray<T>()->InsertData(entity, component);
    }

    // Detach a component of type T from an entity.
    template <typename T>
    void RemoveComponent(Entity entity)
    {
        // get the ComponentArray<T> and call its RemoveData(entity).
        GetComponentArray<T>()->RemoveData(entity);
    }

    // Get a reference to an entity's component of type T.
    template <typename T>
    T &GetComponent(Entity entity)
    {
        // get the ComponentArray<T> and return its GetData(entity).
        return GetComponentArray<T>()->GetData(entity);
    }

    // Called when an entity is destroyed: notify EVERY component array so
    // each one can drop that entity's data if it has any.
    void EntityDestroyed(Entity entity)
    {
        //       loop over every entry in componentArrays and call
        //       entry.second->EntityDestroyed(entity);
        for (auto const &entry : componentArrays)
        {
            entry.second->EntityDestroyed(entity);
        }
    }

private:
    // Convenience: fetch the correctly-typed ComponentArray<T> for type T.
    // The map stores base-class pointers (IComponentArray); we down-cast back
    // to the concrete ComponentArray<T>. Safe because the key guarantees the
    // stored object's real type is ComponentArray<T>.
    template <typename T>
    std::shared_ptr<ComponentArray<T>> GetComponentArray()
    {
        std::string typeName = typeid(T).name();

        assert(componentArrays.find(typeName) != componentArrays.end()
               && "Component type not registered before use.");

        return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
    }

    // type name -> component id (bit position in a Signature)
    std::unordered_map<std::string, ComponentType> componentTypes{};

    // type name -> the array storing that component (as base pointer)
    std::unordered_map<std::string, std::shared_ptr<IComponentArray>> componentArrays{};

    // the next id to hand out on registration
    ComponentType nextComponentType = 0;
};