#ifndef COLLISIONSHAPE_H
#define COLLISIONSHAPE_H

#include "../Core/BasicStructs.h"
#include <vector>

enum class CollisionType
{
    Rectangle,
    Polygon
};

struct CollisionShape
{
    CollisionType type;

    // World space
    float x;
    float y;

    // Rectangle
    float width;
    float height;

    // Polygon
    std::vector<Point> points;
};

#endif // COLLISIONSHAPE_H