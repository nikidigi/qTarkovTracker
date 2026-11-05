#ifndef MAPSRESPONSE_H
#define MAPSRESPONSE_H

#include "baseresponse.h"
#include "mapitem.h"

namespace TarkovApi {

class MapsResponse : public BaseResponse
{
public:
    using BaseResponse::BaseResponse;

    QJsonObject findByNormalizedName(const QString &name) const;

    QList<MapItemPtr> extracts(const QString &mapName) const;
    QList<MapItemPtr> transits(const QString &mapName) const;

    QList<MapItemPtr> locks(const QString &mapName) const;
    QList<MapItemPtr> switches(const QString &mapName) const;
};

} // namespace TarkovApi

#endif // MAPSRESPONSE_H
