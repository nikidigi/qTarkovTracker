#include "arrowmapmarker.h"

#include <QPainter>
#include <QPolygonF>

void ArrowMapMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    static const QPolygonF arrow{ { 0, 12 }, { 8, -10 }, { 0, -5 }, { -8, -10 } };

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::white, 2));
    painter->setBrush(QColor("#FF9800"));
    painter->drawPolygon(arrow);
}
