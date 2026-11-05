#ifndef MAPMARKER_H
#define MAPMARKER_H

#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsItem>
#include <QPointer>
#include <QString>
#include <QTimer>

#include "maptooltip.h"

namespace MarkerZ {
inline constexpr qreal Label = 10;
inline constexpr qreal Lock = 20;
inline constexpr qreal Switch = 20;
inline constexpr qreal Extract = 30;
inline constexpr qreal Transit = 40;
inline constexpr qreal Task = 50;
inline constexpr qreal Player = 100;
} // namespace MarkerZ

class MapMarker : public QGraphicsItem
{
public:
    explicit MapMarker(QGraphicsItem *parent = nullptr);
    ~MapMarker();

    void setGroup(const QString &group) { m_group = group; }
    const QString &group() const { return m_group; }

    void setDirection(qreal degrees);

    QString label() const { return m_label; }
    void setLabel(const QString &label);

    void setMapToolTip(const QString &title, const QString &description,
                       const QString &link = QString());

private:
    enum class ToolTipAction { Show, Hide };

    void showMapToolTip();

protected:
    QString m_group;
    QString m_label;
    QFont m_labelFont{ "Sans Serif", 10 };

    // Prevent dangling pointer if Qt deletes the tooltip
    QPointer<MapToolTip> m_toolTip = nullptr;
    QTimer m_toolTipTimer;
    ToolTipAction m_toolTipAction = ToolTipAction::Show;

    QGraphicsDropShadowEffect *m_glow = nullptr;

    QRectF labelRect(const QRectF &iconRect) const;
    void paintLabel(QPainter *painter, const QRectF &iconRect) const;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
};

#endif // MAPMARKER_H
