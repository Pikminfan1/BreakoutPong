#pragma once

#include "ecs/types.hpp"
#include <memory>          // std::shared_ptr, std::make_shared
#include <unordered_map>
#include <set>
#include <string>
#include <typeinfo>        // typeid
#include <cassert>

// --- Base class every concrete system inherits from ---
// A system's only shared state is the set of entities it currently operates on.
// The SystemManager keeps this set in sync as entity signatures change.
// Concrete systems (MovementSystem, RenderSystem, ...) derive from this and
// add their own Update(...) method that iterates mEntities.
class System
{
public:
    std::set<Entity> mEntities;
};

class SystemManager
{
public:
    // Register a system type and return a shared_ptr to it so callers can
    // hold onto it and call its Update() each frame.
    template <typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        const std::string typeName = typeid(T).name();

        assert(systems.find(typeName) == systems.end()
               && "Registering system more than once.");

        // 1. create the system:            auto system = std::make_shared<T>();
        // 2. store it in the map:          systems[typeName] = system;
        // 3. return it:                    return system;
        auto system = std::make_shared<T>();
        systems[typeName] = system;
        return system;
    }

    // Set the signature (required components) for a system type.
    template <typename T>
    void SetSignature(Signature signature)
    {
        const std::string typeName = typeid(T).name();

        assert(systems.find(typeName) != systems.end()
               && "System used before registered.");

        // store the signature for this system:
        signatures[typeName] = signature;
    }

    // Called when an entity is destroyed: remove it from EVERY system's set.
    void EntityDestroyed(Entity entity)
    {
        //loop over every (name -> system) pair in `systems` and
        //       erase `entity` from that system's mEntities set.
        //       (std::set::erase is a safe no-op if the entity isn't present.)
        for(auto &[name, system] : systems){
            system->mEntities.erase(entity);
        }
    }

    // Called when an entity's signature CHANGES (component added/removed).
    // Re-evaluate the entity against every system: if it now matches, add it
    // to that system's set; if it no longer matches, remove it.
    void EntitySignatureChanged(Entity entity, Signature entitySignature)
    {
        // for each (name -> system) pair in `systems`:
        //   1. look up that system's required signature:
        //        const Signature &systemSignature = signatures[name];
        //   2. match test: does the entity have ALL the required bits?
        //        (entitySignature & systemSignature) == systemSignature
        //   3. if it matches:   system->mEntities.insert(entity);
        //      if it doesn't:   system->mEntities.erase(entity);
        for(auto &[name, system] : systems){
            const Signature &systemSignature = signatures[name];
            if((entitySignature & systemSignature) == systemSignature){
                system->mEntities.insert(entity);
            } else {
                system->mEntities.erase(entity);
            }
        }
    }

private:
    // system type name -> that system's required component signature
    std::unordered_map<std::string, Signature> signatures{};

    // system type name -> the system instance
    std::unordered_map<std::string, std::shared_ptr<System>> systems{};
};