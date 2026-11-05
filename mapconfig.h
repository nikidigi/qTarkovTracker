#ifndef MAPCONFIG_H
#define MAPCONFIG_H

#include <QList>
#include <QPointF>
#include <QString>
#include <QUrl>
#include <QMetaType>

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

    QPointF convert(double x, double z) const;
    double convertDirection(double direction) const;

private:
    QPointF toMapPixels(double gameX, double gameZ) const;
};

Q_DECLARE_METATYPE(MapConfig)

#endif // MAPCONFIG_H
