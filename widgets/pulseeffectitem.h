#ifndef PULSEEFFECTITEM_H
#define PULSEEFFECTITEM_H

#include <QGraphicsObject>
#include <QVariantAnimation>

class PulseEffectItem : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit PulseEffectItem(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void start();
    void stop();

private:
    QVariantAnimation m_animation;
    qreal m_progress = 0.0;

    static constexpr qreal MinRadius = 10.0;
    static constexpr qreal MaxRadius = 50.0;
    static constexpr qreal PenWidth = 2.0;
};


#endif // PULSEEFFECTITEM_H
