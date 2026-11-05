#ifndef ARROWMAPMARKER_H
#define ARROWMAPMARKER_H

#include "mapmarker.h"

class ArrowMapMarker : public MapMarker
{
public:
    using MapMarker::MapMarker;

    QRectF boundingRect() const override { return { -14, -16, 28, 32 }; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
};

#endif // ARROWMAPMARKER_H
