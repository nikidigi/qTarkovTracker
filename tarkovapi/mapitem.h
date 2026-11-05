#ifndef MAPITEM_H
#define MAPITEM_H

#include <QJsonObject>
#include <QString>
#include <QVector3D>

namespace TarkovApi {

enum MapItemType { Extract, Transit, Task, Lock, Switch };

struct MapItem
{
    explicit MapItem(MapItemType type) : type(type) { }

    virtual ~MapItem() = default;

    MapItemType type;
};

using MapItemPtr = std::shared_ptr<MapItem>;

struct MapItemExtract : MapItem
{
    MapItemExtract() : MapItem(MapItemType::Extract) { }

    QString id;
    QString name;
    QString faction;
    QVector3D position;
};

struct MapItemTransit : MapItem
{
    MapItemTransit() : MapItem(MapItemType::Transit) { }

    QString id;
    QString description;
    QVector3D position;
};

struct MapItemTask : MapItem
{
    MapItemTask() : MapItem(MapItemType::Task) { }

    QString id;
    QString objectiveId;
    QString name;
    QString description;
    QString wikiLink;
    QVector3D position;
};

using MapItemTaskPtr = std::shared_ptr<MapItemTask>;

struct MapItemLock : MapItem
{
    MapItemLock() : MapItem(MapItemType::Lock) { }

    QString id;
    QString keyId;
    QString name;
    QString description;
    QString wikiLink;
    QString lockType;
    bool needsPower = false;

    double bottom = -10000;
    double top = 10000;

    QVector3D position;
};

struct MapItemSwitch : MapItem
{
    MapItemSwitch() : MapItem(MapItemType::Switch) { }

    QString id;
    QString name;
    QString switchType;
    QVector3D position;
};

} // namespace TarkovApi

#endif // MAPITEM_H
