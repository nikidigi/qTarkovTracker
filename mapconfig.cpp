#include "mapconfig.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

#include <QtMath>

QPointF MapConfig::convert(double gameX, double gameZ) const
{
    const std::array<QPointF, 4> corners{ { { bounds[0].x(), bounds[0].y() },
                                            { bounds[1].x(), bounds[0].y() },
                                            { bounds[0].x(), bounds[1].y() },
                                            { bounds[1].x(), bounds[1].y() } } };

    const auto first = toMapPixels(corners[0].x(), corners[0].y());

    double minX = first.x();
    double maxX = first.x();
    double minY = first.y();
    double maxY = first.y();

    for (const auto &corner : corners) {
        const auto p = toMapPixels(corner.x(), corner.y());

        minX = std::min(minX, p.x());
        maxX = std::max(maxX, p.x());
        minY = std::min(minY, p.y());
        maxY = std::max(maxY, p.y());
    }

    const double width = maxX - minX;
    const double height = maxY - minY;

    if (width <= 0 || height <= 0)
        throw std::runtime_error("Invalid map bounds");

    const QPointF point = toMapPixels(gameX, gameZ);

    return { (point.x() - minX) / width, (point.y() - minY) / height };
}

double MapConfig::convertDirection(double direction) const
{
    return std::remainder(direction + 180.0 - coordinateRotation, 360.0);
}

QPointF MapConfig::toMapPixels(double gameX, double gameZ) const
{
    const double angle = qDegreesToRadians(coordinateRotation);

    const double cosA = std::cos(angle);
    const double sinA = std::sin(angle);

    const double rotatedX = gameX * cosA - gameZ * sinA;
    const double rotatedZ = gameX * sinA + gameZ * cosA;

    return { transform[0] * rotatedX + transform[1], -transform[2] * rotatedZ + transform[3] };
}