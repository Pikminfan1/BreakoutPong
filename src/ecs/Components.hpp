#pragma once

#include <cstdint>

struct Position
{
    float x, y;
};

struct PreviousPosition
{
    float x, y;
};

struct Velocity
{
    float dx, dy;
};

struct Renderable
{
    float w, h;
    uint8_t r, g, b, a;
};

struct PlayerControlled
{
}; // empty tag — marks the input-driven entity

struct Ball
{
}; // empty tag — marks the ball entity

struct Brick
{
    int scoreValue = 10; // default score value for a brick
}; 

struct Collider
{
    float w, h;             // collision box size (independent of Renderable)
    float offsetX, offsetY; // offset from Position's top-left (usually 0,0)
};