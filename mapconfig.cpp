#include "mapconfig.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <qsize.h>
#include <stdexcept>

#include <QtMath>

QPointF MapConfig::convert(double gameX, double gameZ) const
{
    return normalizeToBounds(gameX, gameZ);
}

QPointF MapConfig::convert(double gameX, double gameZ, int zoom, const QSize &canvasSize) const
{
    if (canvasSize.isEmpty())
        throw std::invalid_argument("Invalid map size");

    const QPointF pixels = toMapPixels(gameX, gameZ, zoom);

    return { pixels.x() / canvasSize.width(), pixels.y() / canvasSize.height() };
}

double MapConfig::convertDirection(double direction) const
{
    return std::remainder(direction + 180.0 - coordinateRotation, 360.0);
}

QPointF MapConfig::rotateCoordinates(double gameX, double gameZ) const
{
    if (coordinateRotation == 0)
        return { gameX, gameZ };

    const double angle = qDegreesToRadians(coordinateRotation);
    const double cosA = std::cos(angle);
    const double sinA = std::sin(angle);

    return { gameX * cosA - gameZ * sinA, gameX * sinA + gameZ * cosA };
}

QPointF MapConfig::toMapPixels(double gameX, double gameZ, int zoom) const
{
    const QPointF rotated = rotateCoordinates(gameX, gameZ);
    const double scale = std::ldexp(1.0, zoom);

    return { scale * (transform[0] * rotated.x() + transform[1]),
             scale * (-transform[2] * rotated.y() + transform[3]) };
}

QPointF MapConfig::normalizeToBounds(double gameX, double gameZ) const
{
    const std::array<QPointF, 4> corners = { { { bounds[0].x(), bounds[0].y() },
                                               { bounds[1].x(), bounds[0].y() },
                                               { bounds[0].x(), bounds[1].y() },
                                               { bounds[1].x(), bounds[1].y() } } };

    const QPointF first = toMapPixels(corners[0].x(), corners[0].y(), 0);

    double minX = first.x();
    double maxX = first.x();
    double minY = first.y();
    double maxY = first.y();

    for (const QPointF &corner : corners) {
        const QPointF point = toMapPixels(corner.x(), corner.y(), 0);

        minX = std::min(minX, point.x());
        maxX = std::max(maxX, point.x());
        minY = std::min(minY, point.y());
        maxY = std::max(maxY, point.y());
    }

    const double width = maxX - minX;
    const double height = maxY - minY;

    if (width <= 0.0 || height <= 0.0)
        throw std::runtime_error("Invalid map bounds");

    const QPointF point = toMapPixels(gameX, gameZ, 0);

    return { (point.x() - minX) / width, (point.y() - minY) / height };
}
