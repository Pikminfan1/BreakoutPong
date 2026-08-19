// src/utils/collision.hpp
#pragma once

// Pure geometry — knows nothing about ECS. Takes raw box edges/dims.
inline bool aabbOverlap(float aX, float aY, float aW, float aH,
                        float bX, float bY, float bW, float bH)
{
    return aX < bX + bW && aX + aW > bX &&
           aY < bY + bH && aY + aH > bY;
}