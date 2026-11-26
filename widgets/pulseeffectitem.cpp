#include "pulseeffectitem.h"

#include <QPainter>
#include <QPen>

#include <cmath>

PulseEffectItem::PulseEffectItem(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setAcceptedMouseButtons(Qt::NoButton);
    setZValue(-1);

    m_animation.setStartValue(0.0);
    m_animation.setEndValue(1.0);
    m_animation.setDuration(2000);
    m_animation.setLoopCount(-1);
    m_animation.setEasingCurve(QEasingCurve::Linear);

    connect(&m_animation, &QVariantAnimation::valueChanged,
            this, [this](const QVariant &value) {
                m_progress = value.toReal();
                update();
            });
}

QRectF PulseEffectItem::boundingRect() const
{
    const qreal radius = MaxRadius + PenWidth;

    return QRectF(-radius, -radius,
                  radius * 2, radius * 2);
}

void PulseEffectItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *,
                            QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < 2; ++i) {
        const qreal progress = std::fmod(m_progress + i * 0.5, 1.0);

        const qreal radius =
                MinRadius + progress * (MaxRadius - MinRadius);

        const int alpha = static_cast<int>(
                150 * (1.0 - progress)
                );

        painter->setPen(QPen(
                QColor(80, 180, 255, alpha),
                PenWidth
                ));

        painter->drawEllipse(QPointF(0, 0), radius, radius);
    }
}

void PulseEffectItem::start()
{
    if (m_animation.state() != QAbstractAnimation::Running)
        m_animation.start();
}

void PulseEffectItem::stop()
{
    m_animation.stop();
    m_progress = 0.0;
    update();
}
