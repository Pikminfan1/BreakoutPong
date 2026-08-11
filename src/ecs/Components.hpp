#pragma once

#include <cstdint>

struct Position {
    float x, y;
};

struct Velocity {
    float dx, dy;
};

struct Renderable {
    float w, h;
    uint8_t r, g, b, a;
};
