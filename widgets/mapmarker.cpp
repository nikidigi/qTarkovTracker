#include "mapmarker.h"

#include <QFontMetricsF>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneHoverEvent>

MapMarker::MapMarker(QGraphicsItem *parent) : QGraphicsItem(parent)
{
    setFlag(ItemIgnoresTransformations);
    setCursor(Qt::ArrowCursor);
    setAcceptHoverEvents(true);

    m_toolTipTimer.setSingleShot(true);
    m_toolTipTimer.setInterval(667);

    QObject::connect(&m_toolTipTimer, &QTimer::timeout, [this]() {
        if (!m_toolTip)
            return;

        if (m_toolTipAction == ToolTipAction::Show)
            showMapToolTip();
        else
            m_toolTip->hide();
    });

    m_glow = new QGraphicsDropShadowEffect();
    m_glow->setBlurRadius(24);
    m_glow->setOffset(0, 0);
    m_glow->setColor(QColor(0x83, 0xA8, 0xE8, 210));
    m_glow->setEnabled(false);

    setGraphicsEffect(m_glow);
}

MapMarker::~MapMarker()
{
    m_toolTipTimer.stop();

    delete m_toolTip;
}

void MapMarker::setDirection(qreal degrees)
{
    setRotation(degrees);
}

void MapMarker::setLabel(const QString &label)
{
    if (m_label == label)
        return;

    prepareGeometryChange();
    m_label = label;
    update();
}

void MapMarker::setMapToolTip(const QString &title, const QString &description, const QString &link)
{
    if (!m_toolTip)
        m_toolTip = new MapToolTip();

    m_toolTip->setTitle(title);
    m_toolTip->setDescription(description);

    if (!link.isEmpty())
        m_toolTip->setLink(link);
}

void MapMarker::showMapToolTip()
{
    if (!m_toolTip || !scene())
        return;

    if (m_toolTip->isVisible()) {
        m_toolTip->raise();
        return;
    }

    auto *view = scene()->views().value(0);

    if (!view)
        return;

    auto *viewport = view->viewport();

    if (m_toolTip->parentWidget() != viewport)
        m_toolTip->setParent(viewport);

    const QPoint pos = viewport->mapFromGlobal(QCursor::pos());

    m_toolTip->adjustSize();
    m_toolTip->move(pos + QPoint(10, 10));
    m_toolTip->show();
    m_toolTip->raise();
}

QRectF MapMarker::labelRect(const QRectF &iconRect) const
{
    if (m_label.isEmpty())
        return { };

    QFontMetricsF metrics(m_labelFont);

    constexpr qreal spacing = 6.0;

    return { iconRect.right() + spacing, -metrics.height() / 2.0,
             metrics.horizontalAdvance(m_label), metrics.height() };
}

void MapMarker::paintLabel(QPainter *painter, const QRectF &iconRect) const
{
    if (m_label.isEmpty())
        return;

    const QRectF rect = labelRect(iconRect);

    painter->save();
    painter->setFont(m_labelFont);

    const QRectF background = rect.adjusted(-5, -2, 5, 2);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(20, 25, 32, 210));
    painter->drawRoundedRect(background, 4, 4);

    painter->setPen(Qt::white);
    painter->drawText(rect, Qt::AlignCenter, m_label);

    painter->restore();
}

void MapMarker::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        showMapToolTip();
        event->accept();
        return;
    }

    QGraphicsItem::mousePressEvent(event);
}

void MapMarker::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_glow)
        m_glow->setEnabled(true);

    QGraphicsItem::hoverEnterEvent(event);
}

void MapMarker::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_glow)
        m_glow->setEnabled(false);

    QGraphicsItem::hoverLeaveEvent(event);
}