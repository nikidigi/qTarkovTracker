#ifndef ICONMAPMARKER_H
#define ICONMAPMARKER_H

#include "mapmarker.h"

#include <QPainter>
#include <QPixmap>

class IconMapMarker : public MapMarker
{
public:
    explicit IconMapMarker(const QPixmap &icon, QGraphicsItem *parent = nullptr);

    void resize(const int width, const int height);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    QPixmap m_icon;
    QPixmap m_originalIcon;
};

#endif // ICONMAPMARKER_H
