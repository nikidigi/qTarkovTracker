#ifndef MAPCONFIGLOADER_H
#define MAPCONFIGLOADER_H

#include "mapconfig.h"

#include <QByteArray>
#include <QList>

class MapConfigLoader
{
public:
    static QList<MapConfig> parse(const QByteArray &json);
};

#endif // MAPCONFIGLOADER_H
