
#ifndef IMAGEMAPITEM_H
#define IMAGEMAPITEM_H

#include <QGraphicsItem>
#include <QPixmap>

#include "../imagemapmetadata.h"

class ImageMapItem final : public QGraphicsItem
{
public:
    bool load(const ImageMapLayer &layer);

    QRectF boundingRect() const override { return m_bounds; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    QPixmap m_pixmap;
    QPoint m_offset;
    QRectF m_bounds;
};

#endif // IMAGEMAPITEM_H
