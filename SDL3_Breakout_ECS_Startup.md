# SDL3 Pong/Breakout — ECS Architecture Project Startup

## Project Overview
**Goal:** Build Pong or Breakout using SDL3 + C++ with an Entity-Component-System (ECS) architecture, basic rendering, input handling, and collision detection. Must compile cross-platform on Linux, macOS, and Windows. This is a game portfolio piece.

## Technology Choice: SDL3
**Why SDL3 over SDL2 or SFML:**
- **SDL2:** Mature, tons of tutorials, but aging API. SDL3 is its successor.
- **SFML:** C++ native OOP design, easier for beginners, but less control, smaller community for game dev at scale, and no GPU-accelerated 2D renderer built-in.
- **SDL3 (chosen):** Modernized C API (2024+), GPU-accelerated 2D rendering by default, better event handling, improved gamepad support, cleaner resource management. Fewer tutorials exist, but the API is well-documented and the migration from SDL2 concepts is straightforward.

**Tradeoff accepted:** SDL3 is newer with fewer community examples, but it's the future of SDL, and using it demonstrates forward-thinking in a portfolio.

---

## Prerequisites — What to Study Before Coding

### 1. ECS Architecture (Entity-Component-System)
**What:** A data-oriented design pattern separating identity (Entity), data (Component), and logic (System).
**Where it applies:** The entire game structure. Every game object (ball, paddle, bricks) is an entity with attached components. Systems process components each frame.
**Study:**
- Entities = unique IDs (just an integer or UUID)
- Components = plain structs holding data (Position, Velocity, Sprite, Collider, Health)
- Systems = functions that iterate over entities with specific component combinations
- Consider using [EnTT](https://github.com/skypjack/entt) (header-only C++ ECS library) OR build a minimal ECS from scratch for learning
**Key insight:** ECS decouples data from behavior. A "Ball" isn't a class — it's an entity with Position + Velocity + CircleCollider + Renderable components.

### 2. SDL3 Core Concepts
**What:** Window creation, renderer, event polling, textures, keyboard/gamepad input.
**Where it applies:** Game initialization, the main loop, rendering, and input systems.
**Study:**
- `SDL_Init()`, `SDL_CreateWindow()`, `SDL_CreateRenderer()`
- `SDL_PollEvent()` and the event union (`SDL_EVENT_KEY_DOWN`, `SDL_EVENT_QUIT`, etc.)
- `SDL_RenderRect()`, `SDL_RenderFillRect()` for drawing shapes
- `SDL_GetKeyboardState()` for continuous input polling
- SDL3 uses `SDL_FRect` (float-based) by default — embrace floating point positions
- Resource cleanup: `SDL_DestroyRenderer()`, `SDL_DestroyWindow()`, `SDL_Quit()`
**Reference:** https://wiki.libsdl.org/SDL3/FrontPage

### 3. Game Loop Design & Fixed Timestep
**What:** How to structure the main loop for frame-rate-independent physics.
**Where it applies:** The core `while(running)` loop that drives the entire game.
**Study:**
- **Variable delta time:** `dt = currentTime - lastTime` — simple but physics can be non-deterministic
- **Fixed timestep with accumulator** (recommended): Physics updates at a fixed rate (e.g., 60Hz), rendering interpolates. Prevents tunneling at low FPS.
- **The classic article:** "Fix Your Timestep!" by Glenn Fiedler
**Key formula:**
```
accumulator += frameTime;
while (accumulator >= FIXED_DT) {
    updatePhysics(FIXED_DT);
    accumulator -= FIXED_DT;
}
float alpha = accumulator / FIXED_DT; // for render interpolation
```

### 4. AABB Collision Detection
**What:** Axis-Aligned Bounding Box — the simplest 2D collision test.
**Where it applies:** Ball-vs-paddle, ball-vs-brick, ball-vs-wall collisions.
**Study:**
- Two rectangles overlap if: `a.x < b.x+b.w && a.x+a.w > b.x && a.y < b.y+b.h && a.y+a.h > b.y`
- **Swept AABB** (important!): For a fast-moving ball, discrete checks can miss collisions (tunneling). Swept collision casts the AABB along its velocity vector to find the exact collision time `t` in [0,1].
- **Collision response:** Reflect the velocity component perpendicular to the surface hit. For Breakout, determine WHICH side was hit to reflect the correct axis.
**Research:** "Swept AABB collision detection and response"

### 5. 2D Vector Math
**What:** Basic vector operations for movement and collision response.
**Where it applies:** Velocity, position updates, ball reflection off surfaces.
**Study:**
- Vector addition/subtraction, scalar multiplication
- Normalization (unit vectors)
- Dot product (for angle-based paddle deflection in Pong)
- Reflection formula: `v' = v - 2(v·n)n` where n is the surface normal
**Note:** You don't need a library — a simple `struct Vec2 { float x, y; }` with operator overloads suffices.

### 6. CMake Build System (Cross-Platform)
**What:** The standard C++ build system generator.
**Where it applies:** The entire build pipeline — compiling on Linux/macOS/Windows from one config.
**Study:**
- `CMakeLists.txt` structure: `cmake_minimum_required`, `project()`, `add_executable()`
- `find_package(SDL3 REQUIRED)` or `FetchContent` to auto-download SDL3
- Platform-specific flags (`WIN32_EXECUTABLE` on Windows to suppress console)
- Compiler flags: `-Wall -Wextra -std=c++17`
- Out-of-source builds: `mkdir build && cd build && cmake .. && make`
**Recommended:** Use `FetchContent` to auto-clone SDL3 from GitHub — simplifies setup for all platforms.

### 7. Game State Management
**What:** Managing transitions between menu, playing, paused, game-over screens.
**Where it applies:** The outer game structure that determines which systems run.
**Study:**
- Simple enum-based state machine (sufficient for Pong/Breakout)
- State pattern (if you want to be fancy)
- Each state determines which systems are active (e.g., pause state skips physics but still renders)

---

## Architecture Overview

```
src/
├── main.cpp              # Entry point, SDL init, main loop
├── ecs/
│   ├── ecs.hpp           # Entity manager, component storage, system base
│   ├── components.hpp    # All component structs
│   └── systems/
│       ├── input_system.hpp
│       ├── movement_system.hpp
│       ├── collision_system.hpp
│       ├── render_system.hpp
│       └── game_logic_system.hpp
├── game/
│   ├── game.hpp          # Game class (owns ECS world, manages states)
│   └── factory.hpp       # Entity factory (createBall, createPaddle, createBrick)
└── utils/
    ├── vec2.hpp          # 2D vector math
    └── constants.hpp     # Screen size, physics constants
CMakeLists.txt
```

---

## Component Definitions (ECS Data)

```cpp
// components.hpp
struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct BoxCollider { float w, h; };  // AABB
struct Renderable { float w, h; SDL_Color color; };
struct PaddleTag {};      // Marker: this entity is the paddle
struct BallTag {};        // Marker: this entity is the ball
struct BrickTag { int health = 1; };  // Marker + data
struct WallTag {};        // Marker: this entity is a wall boundary
struct InputControlled {}; // This entity responds to player input
```

---

## Systems (ECS Logic)

| System | Processes | Purpose |
|--------|-----------|---------|
| InputSystem | InputControlled + Velocity | Reads keyboard → sets paddle velocity |
| MovementSystem | Position + Velocity | `pos += vel * dt` each fixed step |
| CollisionSystem | Position + BoxCollider | AABB tests, generates collision events |
| CollisionResponseSystem | Collision events | Reflects ball, destroys bricks, game-over checks |
| RenderSystem | Position + Renderable | Draws all visible entities via SDL3 renderer |
| GameLogicSystem | BrickTag, BallTag | Win/lose conditions, score tracking |

---

## Boilerplate Code

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)
project(BreakoutECS VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Fetch SDL3 from GitHub (cross-platform, no manual install needed) ---
include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG main  # or pin to a release tag for stability
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

# --- Define executable ---
add_executable(${PROJECT_NAME}
    src/main.cpp
    # Add more .cpp files as you create them
)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# --- Platform-specific: suppress console window on Windows ---
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
```
**[LEARN: CMake]** `FetchContent` eliminates the "install SDL first" problem. CMake generates Makefiles (Linux), Xcode projects (macOS), or VS solutions (Windows) from one config.

---

### src/utils/vec2.hpp
```cpp
#pragma once

struct Vec2 {
    float x = 0.f, y = 0.f;

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }

    float dot(const Vec2& o) const { return x * o.x + y * o.y; }
    float lengthSq() const { return x * x + y * y; }

    Vec2 normalized() const {
        float len = std::sqrt(lengthSq());
        return (len > 0.f) ? Vec2{x / len, y / len} : Vec2{0.f, 0.f};
    }

    // Reflect this vector across a surface normal
    // Formula: v' = v - 2(v·n)n
    Vec2 reflect(const Vec2& normal) const {
        float d = dot(normal);
        return {x - 2.f * d * normal.x, y - 2.f * d * normal.y};
    }
};
```
**[LEARN: 2D Vector Math]** The `reflect()` function is the core of ball bouncing. When the ball hits a horizontal surface, normal is (0,1) or (0,-1), flipping the Y velocity. Research the reflection formula derivation.

---

### src/utils/constants.hpp
```cpp
#pragma once

namespace Constants {
    constexpr int SCREEN_WIDTH = 800;
    constexpr int SCREEN_HEIGHT = 600;
    constexpr float FIXED_DT = 1.0f / 60.0f;  // 60 Hz physics
    constexpr float PADDLE_SPEED = 400.0f;     // pixels/sec
    constexpr float BALL_SPEED = 350.0f;       // pixels/sec
    constexpr int BRICK_ROWS = 5;
    constexpr int BRICK_COLS = 10;
    constexpr float BRICK_WIDTH = 70.0f;
    constexpr float BRICK_HEIGHT = 25.0f;
    constexpr float BRICK_PADDING = 5.0f;
}
```

---

### src/main.cpp (Annotated Boilerplate)
```cpp
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>  // Handles platform entry point differences
#include <cstdio>
#include "utils/constants.hpp"

// [LEARN: Game Loop / Fixed Timestep]
// This main loop uses a fixed-timestep accumulator pattern.
// Physics runs at a constant rate regardless of frame rate.

int main(int argc, char* argv[]) {
    // --- INITIALIZATION ---
    // [LEARN: SDL3 Core] SDL_Init sets up subsystems (video, audio, events)
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return 1;
    }

    // [LEARN: SDL3 Core] Window + Renderer creation
    SDL_Window* window = SDL_CreateWindow(
        "Breakout ECS",
        Constants::SCREEN_WIDTH,
        Constants::SCREEN_HEIGHT,
        0  // flags: 0 = windowed
    );
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // [LEARN: SDL3 Core] The renderer is your drawing context.
    // SDL3 renderers are GPU-accelerated by default.
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // --- ECS WORLD SETUP ---
    // TODO: Initialize your ECS world here
    // TODO: Use factory functions to spawn entities:
    //   createPaddle(world), createBall(world), createBricks(world), createWalls(world)

    // --- GAME LOOP ---
    // [LEARN: Fixed Timestep] The accumulator pattern ensures physics
    // runs at exactly FIXED_DT intervals, independent of render frame rate.
    bool running = true;
    Uint64 lastTime = SDL_GetPerformanceCounter();
    float accumulator = 0.0f;
    float freq = static_cast<float>(SDL_GetPerformanceFrequency());

    while (running) {
        // Calculate frame delta time
        Uint64 currentTime = SDL_GetPerformanceCounter();
        float frameTime = (currentTime - lastTime) / freq;
        lastTime = currentTime;

        // Clamp to prevent spiral of death (e.g., after breakpoint)
        if (frameTime > 0.25f) frameTime = 0.25f;
        accumulator += frameTime;

        // --- EVENT HANDLING ---
        // [LEARN: SDL3 Core / Input] Poll all pending events each frame
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) running = false;
            }
        }

        // --- FIXED UPDATE (PHYSICS) ---
        // [LEARN: Fixed Timestep + ECS Systems]
        // All physics/logic systems run here at a fixed rate.
        while (accumulator >= Constants::FIXED_DT) {
            // TODO: inputSystem.update(world, FIXED_DT);
            // TODO: movementSystem.update(world, FIXED_DT);
            // TODO: collisionSystem.update(world, FIXED_DT);
            // TODO: gameLogicSystem.update(world, FIXED_DT);
            accumulator -= Constants::FIXED_DT;
        }

        // --- RENDER ---
        // [LEARN: SDL3 Rendering] Clear → Draw → Present each frame
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255); // dark background
        SDL_RenderClear(renderer);

        // TODO: renderSystem.update(world, renderer);
        // This system iterates all entities with Position + Renderable
        // and calls SDL_RenderFillRect for each.

        SDL_RenderPresent(renderer);
    }

    // --- CLEANUP ---
    // [LEARN: SDL3 Resource Management] Always clean up in reverse order
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
```

---

## Key Algorithms to Research Deeper

| Algorithm/Concept | Why It Matters | When You'll Need It |
|---|---|---|
| **Swept AABB** | Prevents ball tunneling through thin objects at high speed | CollisionSystem — when ball velocity × dt > brick thickness |
| **Reflection Vector** | `v' = v - 2(v·n)n` | CollisionResponseSystem — ball bounce direction |
| **Fixed Timestep Accumulator** | Deterministic physics regardless of FPS | Main loop structure |
| **Angle-based deflection** | In Pong, where the ball hits the paddle affects bounce angle | CollisionResponseSystem — adds skill-based gameplay |
| **Spatial partitioning** (optional) | Optimize collision checks when many bricks exist | CollisionSystem — grid-based broad phase |

---

## Build & Run Instructions (Cross-Platform)

```bash
# Clone your project, then:
mkdir build && cd build
cmake ..
cmake --build .

# Run:
./BreakoutECS        # Linux/macOS
.\BreakoutECS.exe    # Windows
```

**Requirements:** CMake 3.20+, a C++17 compiler (GCC 9+, Clang 10+, MSVC 2019+), Git (for FetchContent).

---

## Implementation Order (Suggested)

1. **Phase 1 — Window & Loop:** Get SDL3 window open with the game loop running (this boilerplate)
2. **Phase 2 — Minimal ECS:** Implement entity creation, component storage (use `std::unordered_map<EntityID, Component>` or EnTT)
3. **Phase 3 — Rendering:** RenderSystem draws rectangles for paddle, ball, bricks
4. **Phase 4 — Input:** InputSystem moves the paddle with arrow keys / A+D
5. **Phase 5 — Movement:** MovementSystem applies velocity to position each tick
6. **Phase 6 — Collision:** AABB detection + reflection response (hardest phase — take time here)
7. **Phase 7 — Game Logic:** Brick destruction, lives, score, win/lose states
8. **Phase 8 — Polish:** Sound (SDL3_mixer), particles, menus, screen shake

---

## Notes for Claude

- The developer is building this for their game portfolio. Code quality, clean architecture, and clear separation of concerns matter.
- Prefer teaching *why* over just providing code. Annotate where concepts (from the study list above) apply.
- Use modern C++17 (structured bindings, std::optional, constexpr, etc.)
- Keep the ECS simple — don't over-engineer. A minimal hand-rolled ECS is fine; suggest EnTT only if complexity demands it.
- Always consider cross-platform: avoid platform-specific APIs, use SDL3 abstractions.
- When implementing collision, explicitly address the tunneling problem and suggest swept AABB if ball speed is high relative to object thickness.
