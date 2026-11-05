#include "mapsresponse.h"

#include <QJsonArray>

namespace TarkovApi {

QJsonObject MapsResponse::findByNormalizedName(const QString &name) const
{
    for (auto it = m_data.constBegin(); it != m_data.constEnd(); ++it) {
        const auto map = it.value().toObject();

        if (map.value("normalizedName").toString() == name)
            return map;
    }

    return { };
}

QList<MapItemPtr> MapsResponse::extracts(const QString &mapName) const
{
    QList<MapItemPtr> result;

    const auto map = findByNormalizedName(mapName);

    for (const auto &value : map.value("extracts").toArray()) {
        const auto data = value.toObject();
        const auto position = data.value("position").toObject();

        if (position.isEmpty())
            continue;

        auto extract = std::make_shared<MapItemExtract>();

        extract->id = data.value("id").toString();
        extract->name = translate(data.value("name").toString());
        extract->faction = data.value("faction").toString();

        extract->position =
                QVector3D(position.value("x").toDouble(), position.value("y").toDouble(),
                          position.value("z").toDouble());

        result.append(extract);
    }

    return result;
}

QList<MapItemPtr> MapsResponse::transits(const QString &mapName) const
{
    QList<MapItemPtr> result;

    const auto map = findByNormalizedName(mapName);

    for (const auto &value : map.value("transits").toArray()) {
        const auto data = value.toObject();
        const auto position = data.value("position").toObject();

        if (position.isEmpty())
            continue;

        auto extract = std::make_shared<MapItemTransit>();

        extract->id = data.value("id").toString();
        extract->description = translate(data.value("description").toString());

        extract->position =
                QVector3D(position.value("x").toDouble(), position.value("y").toDouble(),
                          position.value("z").toDouble());

        result.append(extract);
    }

    return result;
}

QList<MapItemPtr> MapsResponse::locks(const QString &mapName) const
{
    QList<MapItemPtr> items;

    const auto map = findByNormalizedName(mapName);

    for (const auto &value : map.value("locks").toArray()) {
        const auto object = value.toObject();
        const auto position = object.value("position").toObject();

        if (position.isEmpty())
            continue;

        auto lock = std::make_shared<MapItemLock>();

        lock->id = object.value("id").toString();
        lock->keyId = object.value("key").toString();
        lock->lockType = object.value("lockType").toString();
        lock->needsPower = object.value("needsPower").toBool();
        lock->top = object.value("top").toDouble(10000);
        lock->bottom = object.value("bottom").toDouble(-10000);

        lock->position = QVector3D(position.value("x").toDouble(), position.value("y").toDouble(),
                                   position.value("z").toDouble());

        items.append(lock);
    }

    return items;
}

QList<MapItemPtr> MapsResponse::switches(const QString &mapName) const
{
    QList<MapItemPtr> items;

    const auto map = findByNormalizedName(mapName);

    for (const auto &value : map.value("switches").toArray()) {
        const auto object = value.toObject();
        const auto position = object.value("position").toObject();

        if (position.isEmpty())
            continue;

        auto sw = std::make_shared<MapItemSwitch>();

        sw->id = object.value("id").toString();
        sw->name = translate(object.value("name").toString());
        sw->switchType = object.value("switchType").toString();

        sw->position = QVector3D(position.value("x").toDouble(), position.value("y").toDouble(),
                                 position.value("z").toDouble());

        items.append(sw);
    }

    return items;
}

} // namespace TarkovApi