#include "tasksresponse.h"

#include <QJsonArray>

namespace TarkovApi {

QList<MapItemPtr> TasksResponse::findByMap(const QString &mapId) const
{
    QList<MapItemPtr> result;

    for (auto it = m_data.constBegin(); it != m_data.constEnd(); ++it) {
        const auto task = it.value().toObject();

        for (const auto &value : task.value("objectives").toArray()) {
            const auto objective = value.toObject();

            result.append(parseZones(task, objective, mapId));
            result.append(parsePossibleLocations(task, objective, mapId));
        }
    }

    return result;
}

MapItemTaskPtr TasksResponse::createTask(const QJsonObject &task, const QJsonObject &objective,
                                         const QJsonObject &position) const
{
    auto result = std::make_shared<MapItemTask>();

    result->id = task.value("id").toString();
    result->name = translate(task.value("name").toString());
    result->objectiveId = objective.value("id").toString();
    result->description = translate(objective.value("description").toString());
    result->wikiLink = task.value("wikiLink").toString();

    result->position = QVector3D(position.value("x").toDouble(), position.value("y").toDouble(),
                                 position.value("z").toDouble());

    return result;
}

QList<MapItemPtr> TasksResponse::parseZones(const QJsonObject &task, const QJsonObject &objective,
                                            const QString &mapId) const
{
    QList<MapItemPtr> result;

    for (const auto &value : objective.value("zones").toArray()) {
        const auto zone = value.toObject();

        if (zone.value("map").toString() != mapId)
            continue;

        const auto position = zone.value("position").toObject();

        if (position.isEmpty())
            continue;

        result.append(createTask(task, objective, position));
    }

    return result;
}

QList<MapItemPtr> TasksResponse::parsePossibleLocations(const QJsonObject &task,
                                                        const QJsonObject &objective,
                                                        const QString &mapId) const
{
    QList<MapItemPtr> result;

    for (const auto &value : objective.value("possibleLocations").toArray()) {
        const auto location = value.toObject();

        if (location.value("map").toString() != mapId)
            continue;

        for (const auto &positionValue : location.value("positions").toArray()) {
            const auto position = positionValue.toObject();

            if (position.isEmpty())
                continue;

            result.append(createTask(task, objective, position));
        }
    }

    return result;
}

} // namespace TarkovApi