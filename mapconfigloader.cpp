#include "mapconfigloader.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

QList<MapConfig> MapConfigLoader::parse(const QByteArray &json)
{
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(json, &error);

    if (error.error != QJsonParseError::NoError || !document.isArray()) {
        qWarning() << "Invalid maps JSON:" << error.errorString();
        return { };
    }

    QList<MapConfig> configs;

    for (const auto &entry : document.array()) {
        const auto root = entry.toObject();
        const auto maps = root.value("maps").toArray();

        for (const auto &mapValue : maps) {
            const auto map = mapValue.toObject();

            if (map.value("projection").toString() != "interactive")
                continue;

            const auto svgPath = map.value("svgPath").toString();
            const auto tilePath = map.value("tilePath").toString();

            if (svgPath.isEmpty() && tilePath.isEmpty())
                continue;

            const auto transform = map.value("transform").toArray();
            const auto bounds = map.value("bounds").toArray();

            if (transform.size() != 4 || bounds.size() != 2)
                continue;

            const auto bound0 = bounds[0].toArray();
            const auto bound1 = bounds[1].toArray();

            if (bound0.size() != 2 || bound1.size() != 2)
                continue;

            MapConfig config;

            config.id = root.value("normalizedName").toString();
            config.svgPath = QUrl(svgPath);
            config.coordinateRotation = map.value("coordinateRotation").toDouble();

            for (int i = 0; i < 4; ++i)
                config.transform[i] = transform[i].toDouble();

            config.bounds[0] = QPointF(bound0[0].toDouble(), bound0[1].toDouble());

            config.bounds[1] = QPointF(bound1[0].toDouble(), bound1[1].toDouble());

            for (const auto &labelValue : map.value("labels").toArray()) {
                const auto labelObject = labelValue.toObject();
                const auto position = labelObject.value("position").toArray();

                if (position.size() != 2)
                    continue;

                MapLabel label;

                label.text = labelObject.value("text").toString();

                label.position = QPointF(position[0].toDouble(), position[1].toDouble());

                const auto rotation = labelObject.value("rotation");

                label.rotation =
                        rotation.isString() ? rotation.toString().toDouble() : rotation.toDouble();

                label.size = labelObject.value("size").toDouble(100);

                label.bottom = labelObject.value("bottom").toDouble(-10000);

                label.top = labelObject.value("top").toDouble(10000);

                config.labels.append(label);
            }

            if (!config.id.isEmpty() && (!svgPath.isEmpty() || !tilePath.isEmpty()))
                configs.append(config);

            break;
        }
    }

    return configs;
}