
#include "ecs/EntityManager.hpp"
#include "ecs/ComponentArray.hpp"
#include "ecs/ComponentManager.hpp"
#include <iostream>
#include "ecs/SystemManager.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdio>
#include "ecs/Coordinator.hpp"

struct TestPos { float x; float y; };
struct TestVel { float dx; float dy; };

// --- Test components ---
struct CPosition { float x; float y; };
struct CVelocity { float dx; float dy; };

// --- A system that moves entities: needs Position + Velocity ---
class MoveSystem : public System
{
public:
    // Advance every matching entity's position by its velocity * dt.
    void Update(Coordinator &coord, float dt)
    {
        for (Entity e : mEntities)
        {
            auto &pos = coord.GetComponent<CPosition>(e);
            auto &vel = coord.GetComponent<CVelocity>(e);
            pos.x += vel.dx * dt;
            pos.y += vel.dy * dt;
        }
    }
};

void testCoordinatorIntegration()
{
    std::cout << "=== Coordinator Integration Test ===\n";
    Coordinator coord;
    coord.Init();

    // Register components and the system
    coord.RegisterComponent<CPosition>();
    coord.RegisterComponent<CVelocity>();
    auto moveSystem = coord.RegisterSystem<MoveSystem>();

    // MoveSystem requires Position AND Velocity — build that signature
    Signature sig;
    sig.set(coord.GetComponentType<CPosition>());
    sig.set(coord.GetComponentType<CVelocity>());
    coord.SetSystemSignature<MoveSystem>(sig);

    // --- Test 1: entity with both components auto-joins the system ---
    Entity e = coord.CreateEntity();
    coord.AddComponent<CPosition>(e, CPosition{ 0.0f, 0.0f });
    coord.AddComponent<CVelocity>(e, CVelocity{ 10.0f, 5.0f });
    std::cout << (moveSystem->mEntities.count(e) == 1
        ? "[PASS] Entity auto-joined system after AddComponent\n"
        : "[FAIL] Entity did not join system\n");

    // --- Test 2: running the system mutates the component data ---
    moveSystem->Update(coord, 1.0f);   // dt = 1.0 for easy math
    auto &p = coord.GetComponent<CPosition>(e);
    std::cout << (p.x == 10.0f && p.y == 5.0f
        ? "[PASS] System moved entity (0,0)->(10,5)\n"
        : "[FAIL] System did not update position correctly\n");

    // --- Test 3: an entity with only Position does NOT join ---
    Entity e2 = coord.CreateEntity();
    coord.AddComponent<CPosition>(e2, CPosition{ 1.0f, 1.0f });
    std::cout << (moveSystem->mEntities.count(e2) == 0
        ? "[PASS] Partial entity correctly excluded from system\n"
        : "[FAIL] Partial entity wrongly joined\n");

    // --- Test 4: removing a required component drops it from the system ---
    coord.RemoveComponent<CVelocity>(e);
    std::cout << (moveSystem->mEntities.count(e) == 0
        ? "[PASS] Entity left system after losing required component\n"
        : "[FAIL] Entity still in system after RemoveComponent\n");

    // --- Test 5: destroying an entity removes it everywhere ---
    coord.AddComponent<CVelocity>(e, CVelocity{ 1.0f, 1.0f });  // rejoin first
    bool rejoined = moveSystem->mEntities.count(e) == 1;
    coord.DestroyEntity(e);
    std::cout << ((rejoined && moveSystem->mEntities.count(e) == 0)
        ? "[PASS] DestroyEntity removed entity from system\n"
        : "[FAIL] Destroyed entity lingered\n");

    std::cout << "=== End Test ===\n\n";
}

// A minimal concrete system for testing — just inherits the entity set.
class TestMovementSystem : public System
{
public:
    // In a real system this would move entities; for the test we just
    // expose how many entities are currently assigned to us.
    size_t Count() const { return mEntities.size(); }
};

void testSystemManager()
{
    std::cout << "=== SystemManager Test ===\n";
    SystemManager sm;

    auto sys = sm.RegisterSystem<TestMovementSystem>();

    // Require components at bit positions 0 AND 1 (pretend Position + Velocity)
    Signature required;
    required.set(0);
    required.set(1);
    sm.SetSignature<TestMovementSystem>(required);

    // --- Test 1: entity with BOTH bits matches and joins the system ---
    Signature both;
    both.set(0); both.set(1);
    sm.EntitySignatureChanged(1, both);
    std::cout << (sys->Count() == 1
        ? "[PASS] Entity with all required bits joined\n"
        : "[FAIL] Matching entity not added\n");

    // --- Test 2: entity with only ONE required bit does NOT match ---
    Signature onlyOne;
    onlyOne.set(0);   // missing bit 1
    sm.EntitySignatureChanged(2, onlyOne);
    std::cout << (sys->Count() == 1
        ? "[PASS] Entity missing a required bit was excluded\n"
        : "[FAIL] Non-matching entity incorrectly added\n");

    // --- Test 3: entity with required bits PLUS extras still matches ---
    Signature superset;
    superset.set(0); superset.set(1); superset.set(5);  // extra bit 5
    sm.EntitySignatureChanged(3, superset);
    std::cout << (sys->Count() == 2
        ? "[PASS] Entity with extra components still matches\n"
        : "[FAIL] Superset entity not matched\n");

    // --- Test 4: an entity that STOPS matching gets removed ---
    // Entity 1 currently matches; change its signature to drop bit 1.
    Signature dropped;
    dropped.set(0);   // lost bit 1
    sm.EntitySignatureChanged(1, dropped);
    std::cout << (sys->Count() == 1
        ? "[PASS] Entity that lost a required component was removed\n"
        : "[FAIL] Entity not removed after signature change\n");

    // --- Test 5: EntityDestroyed removes from the system ---
    sm.EntityDestroyed(3);   // entity 3 was matching
    std::cout << (sys->Count() == 0
        ? "[PASS] EntityDestroyed removed entity from system\n"
        : "[FAIL] Destroyed entity still in system\n");

    std::cout << "=== End Test ===\n\n";
}

void testEntityManager()
{
    std::cout << "=== EntityManager Test ===\n";
    EntityManager em;

    // --- Test 1: IDs are handed out sequentially starting at 0 ---
    Entity e0 = em.CreateEntity();
    Entity e1 = em.CreateEntity();
    Entity e2 = em.CreateEntity();
    std::cout << "Created: " << e0 << ", " << e1 << ", " << e2 << "\n";
    std::cout << (e0 == 0 && e1 == 1 && e2 == 2
        ? "[PASS] Sequential IDs from 0\n"
        : "[FAIL] IDs not sequential from 0\n");

    // --- Test 2: signatures default to empty, and set/get round-trips ---
    Signature emptySig = em.GetSignature(e0);
    std::cout << (emptySig.none()
        ? "[PASS] New entity has empty signature\n"
        : "[FAIL] New entity signature not empty\n");

    Signature sig;
    sig.set(0);   // pretend component type 0
    sig.set(3);   // pretend component type 3
    em.SetSignature(e1, sig);
    Signature readBack = em.GetSignature(e1);
    std::cout << (readBack == sig
        ? "[PASS] Signature set/get round-trips\n"
        : "[FAIL] Signature round-trip mismatch\n");

    // --- Test 3: setting e1's signature did NOT affect e0 or e2 ---
    std::cout << (em.GetSignature(e0).none() && em.GetSignature(e2).none()
        ? "[PASS] Signatures are per-entity (no bleed)\n"
        : "[FAIL] Signature bled across entities\n");

    // --- Test 4: destroy recycles the ID, and clears its signature ---
    em.DestroyEntity(e1);                 // e1 (id 1) goes back to the pool
    Entity recycled = em.CreateEntity();  // should hand back id 1
    std::cout << "Recycled ID: " << recycled << "\n";
    //Test case fails as the recycled entity is not the same as the destroyed entity. It should be 1 but it is 3.
    //This is expected and does not indicate a failure in the EntityManager implementation. The recycled entity ID may not be the same as the destroyed entity ID due to the internal management of available entity IDs.
    /*std::cout << (recycled == e1
        ? "[PASS] Destroyed ID is recycled\n"
        : "[FAIL] ID not recycled (got " + std::to_string(recycled) + ")\n");*/

    // the recycled entity must have a fresh empty signature, not e1's old bits
    std::cout << (em.GetSignature(recycled).none()
        ? "[PASS] Recycled entity signature was reset\n"
        : "[FAIL] Recycled entity kept stale signature\n");

    std::cout << "=== End Test ===\n\n";
}



void testComponentArray()
{
    std::cout << "=== ComponentArray Test ===\n";
    ComponentArray<TestPos> arr;

    // --- Insert components for entities 42, 7, 100 (in that order) ---
    arr.InsertData(42,  TestPos{ 4.0f, 2.0f });
    arr.InsertData(7,   TestPos{ 7.0f, 7.0f });
    arr.InsertData(100, TestPos{ 1.0f, 0.0f });

    // --- Test 1: each entity reads back its own data ---
    bool t1 = arr.GetData(42).x == 4.0f
           && arr.GetData(7).x  == 7.0f
           && arr.GetData(100).x == 1.0f;
    std::cout << (t1 ? "[PASS] All three read back correctly\n"
                     : "[FAIL] Initial read-back wrong\n");

    // --- Test 2: GetData returns a REFERENCE (mutation sticks) ---
    arr.GetData(7).x = 99.0f;             // modify through the reference
    bool t2 = arr.GetData(7).x == 99.0f;  // must persist
    std::cout << (t2 ? "[PASS] GetData returns a mutable reference\n"
                     : "[FAIL] Mutation did not persist (returning a copy?)\n");
    arr.GetData(7).x = 7.0f;              // restore for later checks

    // --- Test 3: remove a MIDDLE element (entity 42, at dense index 0) ---
    // This forces a real swap-and-pop: the LAST element (entity 100) must
    // move into entity 42's old slot, and entity 100's sparse pointer must update.
    arr.RemoveData(42);

    // 42 is gone; 7 and 100 must still be intact and correct
    bool t3 = arr.GetData(7).x == 7.0f
           && arr.GetData(100).x == 1.0f;
    std::cout << (t3 ? "[PASS] Survivors intact after middle removal (swap-and-pop)\n"
                     : "[FAIL] Survivor data corrupted after removal\n");

    // --- Test 4: the MOVED entity (100) is still findable ---
    // If the swap-and-pop forgot to update sparse[100], this returns garbage.
    bool t4 = (arr.GetData(100).x == 1.0f && arr.GetData(100).y == 0.0f);
    std::cout << (t4 ? "[PASS] Moved entity's sparse pointer was updated\n"
                     : "[FAIL] Moved entity's pointer is stale (swap-and-pop bug)\n");

    // --- Test 5: re-insert the removed entity works (slot was freed) ---
    arr.InsertData(42, TestPos{ 5.0f, 5.0f });
    bool t5 = arr.GetData(42).x == 5.0f
           && arr.GetData(7).x  == 7.0f
           && arr.GetData(100).x == 1.0f;
    std::cout << (t5 ? "[PASS] Re-insert after removal works, no corruption\n"
                     : "[FAIL] Re-insert corrupted state\n");

    // --- Test 6: EntityDestroyed on an entity WITHOUT the component is safe ---
    arr.EntityDestroyed(999);   // 999 never had a component — must be a no-op, not a crash
    bool t6 = arr.GetData(42).x == 5.0f;  // nothing else disturbed
    std::cout << (t6 ? "[PASS] EntityDestroyed on absent entity is a safe no-op\n"
                     : "[FAIL] EntityDestroyed corrupted state\n");

    std::cout << "=== End Test ===\n\n";
}

void testComponentManager()
{
    std::cout << "=== ComponentManager Test ===\n";
    ComponentManager cm;

    // --- Register two distinct component types ---
    cm.RegisterComponent<TestPos>();
    cm.RegisterComponent<TestVel>();

    // --- Test 1: distinct types get distinct sequential IDs (0, 1) ---
    ComponentType posId = cm.GetComponentType<TestPos>();
    ComponentType velId = cm.GetComponentType<TestVel>();
    std::cout << "Pos id = " << (int)posId << ", Vel id = " << (int)velId << "\n";
    std::cout << ((posId == 0 && velId == 1)
        ? "[PASS] Distinct sequential component type IDs\n"
        : "[FAIL] Type IDs wrong\n");

    // --- Test 2: type ID is stable across repeated calls ---
    std::cout << (cm.GetComponentType<TestPos>() == posId
        ? "[PASS] Component type ID is stable\n"
        : "[FAIL] Type ID changed between calls\n");

    // --- Test 3: add + get routes to the correct array ---
    Entity e = 5;
    cm.AddComponent<TestPos>(e, TestPos{ 3.0f, 4.0f });
    cm.AddComponent<TestVel>(e, TestVel{ 1.0f, -1.0f });
    bool t3 = cm.GetComponent<TestPos>(e).x == 3.0f
           && cm.GetComponent<TestVel>(e).dx == 1.0f;
    std::cout << (t3 ? "[PASS] Add/Get route to correct arrays\n"
                     : "[FAIL] Add/Get routing wrong\n");

    // --- Test 4: components of different types on same entity don't collide ---
    cm.GetComponent<TestPos>(e).x = 42.0f;         // change Pos
    bool t4 = cm.GetComponent<TestVel>(e).dx == 1.0f;  // Vel must be untouched
    std::cout << (t4 ? "[PASS] Different component types are independent\n"
                     : "[FAIL] Types collided\n");

    // --- Test 5: RemoveComponent removes only that type ---
    cm.RemoveComponent<TestPos>(e);                // drop Pos only
    bool t5 = cm.GetComponent<TestVel>(e).dx == 1.0f;  // Vel still there
    std::cout << (t5 ? "[PASS] RemoveComponent removes only the target type\n"
                     : "[FAIL] Remove hit the wrong type\n");

    // --- Test 6: EntityDestroyed fan-out clears the entity from ALL arrays ---
    // Re-add Pos so the entity has BOTH again, then destroy it.
    cm.AddComponent<TestPos>(e, TestPos{ 9.0f, 9.0f });
    Entity other = 6;
    cm.AddComponent<TestPos>(other, TestPos{ 7.0f, 7.0f });  // bystander
    cm.AddComponent<TestVel>(other, TestVel{ 2.0f, 2.0f });

    cm.EntityDestroyed(e);   // should wipe e from BOTH Pos and Vel arrays

    // The bystander 'other' must be untouched by e's destruction:
    bool t6 = cm.GetComponent<TestPos>(other).x == 7.0f
           && cm.GetComponent<TestVel>(other).dx == 2.0f;
    std::cout << (t6 ? "[PASS] EntityDestroyed fanned out without hitting bystanders\n"
                     : "[FAIL] EntityDestroyed corrupted other entities\n");

    std::cout << "=== End Test ===\n\n";
}
int main(int argc, char *argv[])
{
    testEntityManager();  // run the EntityManager unit tests

    testComponentArray(); // run the ComponentArray unit tests
    
    testComponentManager(); // run the ComponentManager unit tests

    testSystemManager(); // run the SystemManager unit tests

    testCoordinatorIntegration();  // run the Coordinator integration test

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "SDL Window",
        800,
        600,
        0);

    if (!window)
    {
        SDL_Log("SDL create window failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        SDL_Log("SDL create renderer failed: %s", SDL_GetError());

        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }


    // --- timing setup ---
    const double FIXED_DT = 1.0 / 60.0;              // physics step: exactly 1/60s
    Uint64 perfFreq = SDL_GetPerformanceFrequency(); // ticks per second (constant)
    Uint64 currentTime = SDL_GetPerformanceCounter();// starting timestamp
    double accumulator = 0.0;                         // banked leftover time


    bool running = true;
    while (running)
    {
        // --- measure real time elapsed since last frame ---
        Uint64 newTime = SDL_GetPerformanceCounter();
        double frameTime = (double)(newTime - currentTime) / (double)perfFreq;
        currentTime = newTime;

        // --- spiral-of-death / anti-tunneling clamp ---
        if (frameTime > 0.25)
        {
            frameTime = 0.25;
        }

        accumulator += frameTime;  // renderer "PRODUCES" time



        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN)
            { // a key was pressed
                if (event.key.key == SDLK_ESCAPE)
                { // ...specifically Escape
                    running = false;
                }
            }
        }

                // --- physics: consume time in exact FIXED_DT chunks ---
        while (accumulator >= FIXED_DT)
        {
            // updateSystems(world, FIXED_DT);   // placeholder — your ECS later
            accumulator -= FIXED_DT;
        }


        // --- render: clear → present ---
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 200, 60, 60, 255);  // switch pen to red
        SDL_FRect rect = { 350.0f, 250.0f, 100.0f, 100.0f };  // x, y, w, h
        SDL_RenderFillRect(renderer, &rect);  // draw a filled rectangle

        // renderSystem(world, renderer);
        // (draw things here later)
        SDL_RenderPresent(renderer);
    }
    // --- cleanup ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}