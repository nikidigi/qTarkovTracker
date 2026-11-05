#ifndef TASKSRESPONSE_H
#define TASKSRESPONSE_H

#include "baseresponse.h"
#include "mapitem.h"

#include <QList>

namespace TarkovApi {

class TasksResponse : public BaseResponse
{
public:
    using BaseResponse::BaseResponse;

    QList<MapItemPtr> findByMap(const QString &mapId) const;

private:
    MapItemTaskPtr createTask(const QJsonObject &task, const QJsonObject &objective,
                                const QJsonObject &position) const;

    QList<MapItemPtr> parseZones(const QJsonObject &task, const QJsonObject &objective,
                                 const QString &mapId) const;

    QList<MapItemPtr> parsePossibleLocations(const QJsonObject &task, const QJsonObject &objective,
                                             const QString &mapId) const;
};

} // namespace TarkovApi

#endif // TASKSRESPONSE_H
