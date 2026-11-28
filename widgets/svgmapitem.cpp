#include "svgmapitem.h"

#include <QPainter>

bool SvgMapItem::load(const QString &filePath, const QString &layerId,
                      const QString &defaultLayerId)
{
    if (!m_renderer.load(filePath))
        return false;

    if ((!layerId.isEmpty() && !m_renderer.elementExists(layerId)) ||
        (!defaultLayerId.isEmpty() && !m_renderer.elementExists(defaultLayerId)))
        return false;

    prepareGeometryChange();
    m_bounds = m_renderer.viewBoxF();
    m_layerId = layerId;
    m_defaultLayerId = defaultLayerId;
    return !m_bounds.isEmpty();
}

void SvgMapItem::paintLayer(QPainter *painter, const QString &id)
{
    // Element bounds are in local SVG coordinates. Map them into the
    // document's coordinate system so changing floors never shifts/scales the map.
    const QRectF bounds = m_renderer.transformForElement(id).mapRect(
            m_renderer.boundsOnElement(id));
    m_renderer.render(painter, id, bounds);
}

void SvgMapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (m_layerId.isEmpty()) {
        m_renderer.render(painter, m_bounds);
        return;
    }

    if (!m_defaultLayerId.isEmpty() && m_layerId != m_defaultLayerId) {
        painter->save();
        painter->setOpacity(0.10);
        paintLayer(painter, m_defaultLayerId);
        painter->restore();
    }

    paintLayer(painter, m_layerId);
}
