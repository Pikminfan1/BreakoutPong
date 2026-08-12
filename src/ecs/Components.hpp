#pragma once

#include <cstdint>

struct Position {
    float x, y;
};

struct PreviousPosition {
    float x, y;
};

struct Velocity {
    float dx, dy;
};

struct Renderable {
    float w, h;
    uint8_t r, g, b, a;
};

struct PlayerControlled {};   // empty tag — marks the input-driven entity
