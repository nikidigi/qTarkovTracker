#ifndef SVGMAPITEM_H
#define SVGMAPITEM_H

#include <QGraphicsItem>
#include <QSvgRenderer>

class SvgMapItem final : public QGraphicsItem
{
public:
    bool load(const QString &filePath, const QString &layerId,
              const QString &defaultLayerId);

    QRectF boundingRect() const override { return m_bounds; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    void paintLayer(QPainter *painter, const QString &id);

    QSvgRenderer m_renderer;
    QRectF m_bounds;
    QString m_layerId;
    QString m_defaultLayerId;
};

#endif // SVGMAPITEM_H
