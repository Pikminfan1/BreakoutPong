#pragma once

#include "ecs/types.hpp"
#include "ecs/EntityManager.hpp"
#include "ecs/ComponentManager.hpp"
#include "ecs/SystemManager.hpp"
#include <memory>

// The Coordinator is the single facade the whole game talks to. It owns the
// four managers and keeps them consistent, so game code never has to update
// signatures or notify systems by hand.
class Coordinator
{
public:
    void Init()
    {
        entityManager = std::make_unique<EntityManager>();
        componentManager = std::make_unique<ComponentManager>();
        systemManager = std::make_unique<SystemManager>();
    }

    // ---------- Entity ----------

    Entity CreateEntity()
    {
        return entityManager->CreateEntity();
    }

    void DestroyEntity(Entity entity)
    {
        //  An entity being destroyed must be removed EVERYWHERE:
        //   1. entityManager->DestroyEntity(entity);      // recycle id + clear signature
        //   2. componentManager->EntityDestroyed(entity); // drop its data from all arrays
        //   3. systemManager->EntityDestroyed(entity);    // pull it from all systems
        entityManager->DestroyEntity(entity);
        componentManager->EntityDestroyed(entity);
        systemManager->EntityDestroyed(entity);
    }
    Signature GetEntitySignature(Entity entity)
    {
        return entityManager->GetSignature(entity);
    }

    // ---------- Component ----------

    template <typename T>
    void RegisterComponent()
    {
        componentManager->RegisterComponent<T>();
    }

    template <typename T>
    void AddComponent(Entity entity, T component)
    {
        // Adding a component is THREE coordinated steps:
        //   1. store the data:
        //        componentManager->AddComponent<T>(entity, component);
        //   2. update the entity's signature (flip this component's bit ON):
        //        Signature signature = entityManager->GetSignature(entity);
        //        signature.set(componentManager->GetComponentType<T>(), true);
        //        entityManager->SetSignature(entity, signature);
        //   3. tell the systems the signature changed so they re-evaluate:
        //        systemManager->EntitySignatureChanged(entity, signature);
        componentManager->AddComponent<T>(entity, component);
        Signature signature = entityManager->GetSignature(entity);
        signature.set(componentManager->GetComponentType<T>(), true);
        entityManager->SetSignature(entity, signature);
        systemManager->EntitySignatureChanged(entity, signature);
    }

    template <typename T>
    void RemoveComponent(Entity entity)
    {
        // The mirror of AddComponent:
        //   1. componentManager->RemoveComponent<T>(entity);
        //   2. fetch signature, flip this component's bit OFF (set(..., false)),
        //      write it back with entityManager->SetSignature(...)
        //   3. systemManager->EntitySignatureChanged(entity, signature);

        componentManager->RemoveComponent<T>(entity);
        Signature signature = entityManager->GetSignature(entity);
        signature.set(componentManager->GetComponentType<T>(), false);
        entityManager->SetSignature(entity, signature);
        systemManager->EntitySignatureChanged(entity, signature);
    }

    template <typename T>
    T &GetComponent(Entity entity)
    {
        return componentManager->GetComponent<T>(entity);
    }

    template <typename T>
    bool HasComponent(Entity entity)
    {
        Signature signature = entityManager->GetSignature(entity);
        return signature.test(componentManager->GetComponentType<T>());
    }

    template <typename T>
    ComponentType GetComponentType()
    {
        return componentManager->GetComponentType<T>();
    }

    // ---------- System ----------

    template <typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        return systemManager->RegisterSystem<T>();
    }

    template <typename T>
    void SetSystemSignature(Signature signature)
    {
        systemManager->SetSignature<T>(signature);
    }

private:
    std::unique_ptr<EntityManager> entityManager;
    std::unique_ptr<ComponentManager> componentManager;
    std::unique_ptr<SystemManager> systemManager;
};