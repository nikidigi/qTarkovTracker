#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsSvgItem>
#include <QGraphicsView>
#include <QHash>
#include <QWheelEvent>
#include <QWidget>

#include "mapconfig.h"
#include "imagemapmetadata.h"

#include "mapmarker.h"
#include "playermapmarker.h"
#include "imagemapitem.h"

#include "tarkovapi/mapitem.h"

class MapView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = nullptr);

    void clearItems();

    MapMarker player();

    bool loadSvgMap(const QString &path, const QString &layerId = {},
                    const QString &defaultLayerId = {});
    bool loadImageMap(const ImageMapLayer &layer, const ImageMapLayer &defaultLayer);

    void removeGroup(const QString &group);
    void removeGroups(const QString &prefix);
    void setGroupVisible(const QString &group, bool visible);

    void setPlayerPosition(const MapConfig &config, double gameX, double gameZ, double direction);

    void addItems(const QList<TarkovApi::MapItemPtr> &items, const MapConfig &config);

    void addLabels(const MapConfig &config, const QString &layerId = {});

private:
    QGraphicsItem *m_map = nullptr;
    ImageMapItem *m_baseMap = nullptr;
    PlayerMapMarker *m_playerMarker = nullptr;
    QList<MapMarker *> m_markers;
    QHash<QString, bool> m_groupVisibility;
    std::optional<ImageMapLayer> m_imageLayer;
    qreal m_zoomFactor = 1.0;

    QPointF convertPosition(const MapConfig &config, double gameX, double gameZ) const;

    std::optional<QPointF> normalizedViewCenter() const;
    void restoreViewCenter(const std::optional<QPointF> &center);

    void fitMap();

protected:
    void wheelEvent(QWheelEvent *event) override;

signals:
};

#endif // MAPVIEW_H
