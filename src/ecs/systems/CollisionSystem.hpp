#pragma once

#include "ecs/types.hpp"
#include "ecs/Coordinator.hpp"
#include "ecs/Components.hpp"
#include "utils/Collision.hpp"
#include <vector>
#include <algorithm>

class CollisionSystem : public System
{
public:
    void Update(Coordinator &coord, int &scoreDelta)
    {
        // Entities marked for destruction during this pass. We CANNOT destroy
        // entities while iterating mEntities (structural change mid-loop is
        // undefined behavior), so we collect them here and destroy after.
        std::vector<Entity> toDestroy;

        // --- BROAD PHASE: every pair, once ---
        for (auto itA = mEntities.begin(); itA != mEntities.end(); ++itA)
        {
            auto itB = itA;
            ++itB;
            for (; itB != mEntities.end(); ++itB)
            {
                Entity a = *itA;
                Entity b = *itB;

                // --- NARROW PHASE: overlap test ---
                const auto &aPos = coord.GetComponent<Position>(a);
                const auto &aCol = coord.GetComponent<Collider>(a);
                const auto &bPos = coord.GetComponent<Position>(b);
                const auto &bCol = coord.GetComponent<Collider>(b);

                bool hit = aabbOverlap(
                    aPos.x + aCol.offsetX, aPos.y + aCol.offsetY, aCol.w, aCol.h,
                    bPos.x + bCol.offsetX, bPos.y + bCol.offsetY, bCol.w, bCol.h);

                if (!hit)
                    continue;

                // --- RESPONSE ---
                // Only ball-vs-something collisions matter here. Figure out
                // which of a/b is the ball; if neither is, skip (e.g. two
                // bricks can't collide, paddle-vs-brick we ignore).
                Entity ball, other;
                if (coord.HasComponent<Ball>(a))      { ball = a; other = b; }
                else if (coord.HasComponent<Ball>(b)) { ball = b; other = a; }
                else                                   { continue; }  // no ball in this pair

                // Grab the ball's mutable velocity + position, and both boxes.
                auto &ballVel       = coord.GetComponent<Velocity>(ball);
                auto &ballPos       = coord.GetComponent<Position>(ball);
                const auto &ballCol = coord.GetComponent<Collider>(ball);
                const auto &otherPos = coord.GetComponent<Position>(other);
                const auto &otherCol = coord.GetComponent<Collider>(other);

                // --- DIRECTIONAL REFLECTION ---
                // Compute overlap depth on each axis; the SMALLER overlap is
                // the axis the ball crossed to get in, so reflect that axis.
                float ballLeft = ballPos.x + ballCol.offsetX;
                float ballRight = ballLeft + ballCol.w;
                float ballTop = ballPos.y + ballCol.offsetY;
                float ballBottom = ballTop + ballCol.h;

                float otherLeft = otherPos.x + otherCol.offsetX;
                float otherRight = otherLeft + otherCol.w;
                float otherTop = otherPos.y + otherCol.offsetY;
                float otherBottom = otherTop + otherCol.h;

                float overlapX = std::min(ballRight, otherRight) - std::max(ballLeft, otherLeft);
                float overlapY = std::min(ballBottom, otherBottom) - std::max(ballTop, otherTop);

                if (overlapX < overlapY)
                    ballVel.dx = -ballVel.dx;   // hit a vertical face → reflect X
                else
                    ballVel.dy = -ballVel.dy;   // hit a horizontal face → reflect Y

                // --- DESTRUCTION (bricks only) ---
                // TODO: if `other` is a brick (coord.HasComponent<Brick>(other)),
                //       add it to toDestroy so it's removed after the loop.
                //       (Paddle is NOT destroyed — it just reflects the ball.)

                if (coord.HasComponent<Brick>(other))
                {
                    scoreDelta += coord.GetComponent<Brick>(other).scoreValue;
                    toDestroy.push_back(other);
                }
                
            }
        }

        // --- deferred destruction: safe now that iteration is done ---
        for (Entity e : toDestroy)
        {
            coord.DestroyEntity(e);
        }
    }
};