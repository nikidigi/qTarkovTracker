#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsSvgItem>
#include <QGraphicsView>
#include <QHash>
#include <QWheelEvent>
#include <QWidget>

#include "mapconfig.h"

#include "mapmarker.h"
#include "arrowmapmarker.h"

#include "tarkovapi/mapitem.h"

class MapView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MapView(QWidget *parent = nullptr);

    void clearItems();

    MapMarker player();

    void setMapFile(const QString &fileName);
    void setMapWidth(int w);

    void removeGroup(const QString &group);
    void removeGroups(const QString &prefix);
    void setGroupVisible(const QString &group, bool visible);

    void setPlayerPosition(double nx, double ny, double direction);

    void addItems(const QList<TarkovApi::MapItemPtr> &items, const MapConfig &config);

    void addLabels(const MapConfig &config);

private:
    QGraphicsSvgItem *m_map = nullptr;
    ArrowMapMarker *m_playerMarker = nullptr;
    QList<MapMarker *> m_markers;
    QHash<QString, bool> m_groupVisibility;

protected:
    void wheelEvent(QWheelEvent *event) override;

signals:
};

#endif // MAPVIEW_H
