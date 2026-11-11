#include "imagemapitem.h"

#include <QDebug>
#include <QPainter>

#include <utility>

bool ImageMapItem::load(const ImageMapLayer &layer)
{
    QPixmap pixmap(layer.filePath);

    if (pixmap.isNull() || pixmap.size() != layer.size) {
        qWarning() << "Cannot load image map layer:" << layer.filePath;
        return false;
    }

    prepareGeometryChange();

    m_pixmap = std::move(pixmap);
    m_offset = layer.offset;
    m_bounds = QRectF(QPointF(0, 0), QSizeF(layer.sourceSize));

    update();
    return true;
}

void ImageMapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_pixmap.isNull())
        painter->drawPixmap(m_offset, m_pixmap);
}
