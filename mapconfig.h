#ifndef MAPCONFIG_H
#define MAPCONFIG_H

#include <QList>
#include <QMetaType>
#include <QPointF>
#include <QSize>
#include <QString>
#include <QUrl>

struct MapLabel
{
    QString text;
    QPointF position;

    double rotation = 0;
    double size = 100;

    double bottom = -10000;
    double top = 10000;
};

struct MapConfig
{
    QString id;
    QUrl svgPath;

    // [scaleX, offsetX, scaleY, offsetY]
    double transform[4] = { };

    double coordinateRotation = 0;

    QPointF bounds[2];

    QList<MapLabel> labels;

    // SVG: normalize coordinates using map bounds
    QPointF convert(double gameX, double gameZ) const;

    // Interactive map: normalize using image dimensions
    QPointF convert(double gameX, double gameZ, int zoom, const QSize &canvasSize) const;

    double convertDirection(double direction) const;

private:
    QPointF rotateCoordinates(double gameX, double gameZ) const;
    QPointF toMapPixels(double gameX, double gameZ, int zoom) const;
    QPointF normalizeToBounds(double gameX, double gameZ) const;
};

Q_DECLARE_METATYPE(MapConfig)

#endif // MAPCONFIG_H
