#include "iconmapmarker.h"

IconMapMarker::IconMapMarker(const QPixmap &icon, QGraphicsItem *parent)
    : MapMarker(parent), m_originalIcon(icon)
{
    m_icon = icon.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void IconMapMarker::resize(const int width, const int height)
{
    prepareGeometryChange();

    m_icon = m_originalIcon.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    update();
}

QRectF IconMapMarker::boundingRect() const
{
    const QRectF iconRect(-m_icon.width() / 2.0, -m_icon.height() / 2.0, m_icon.width(),
                          m_icon.height());

    if (m_label.isEmpty())
        return iconRect;

    return iconRect.united(labelRect(iconRect).adjusted(-5, -2, 5, 2));

    return { -m_icon.width() / 2.0, -m_icon.height() / 2.0, static_cast<qreal>(m_icon.width()),
             static_cast<qreal>(m_icon.height()) };
}

void IconMapMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const QRectF iconRect(-m_icon.width() / 2.0, -m_icon.height() / 2.0, m_icon.width(),
                          m_icon.height());

    painter->drawPixmap(iconRect, m_icon, m_icon.rect());
    paintLabel(painter, iconRect);
}
