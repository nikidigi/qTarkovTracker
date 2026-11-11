#include "imagemapmetadata.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

std::optional<ImageMapMetadata> ImageMapMetadata::load(const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open image map metadata:" << filePath;
        return std::nullopt;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);

    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        qWarning() << "Invalid image map metadata:" << filePath << error.errorString();
        return std::nullopt;
    }

    const QJsonObject root = document.object();
    const QJsonObject layersObject = root.value("layers").toObject();

    if (layersObject.isEmpty())
        return std::nullopt;

    ImageMapMetadata metadata;

    metadata.zoom = root.value("zoom").toInt(-1);
    metadata.defaultLayer = root.value("defaultLayer").toString();

    if (metadata.zoom < 0)
        return std::nullopt;

    const QDir directory = QFileInfo(file).absoluteDir();

    for (auto it = layersObject.constBegin(); it != layersObject.constEnd(); ++it) {

        if (!it.value().isObject())
            return std::nullopt;

        const QJsonObject object = it.value().toObject();

        const QString filename = object.value("file").toString();

        ImageMapLayer layer;

        layer.zoom = metadata.zoom;
        layer.filePath = directory.absoluteFilePath(filename);

        layer.offset = QPoint(object.value("offsetX").toInt(-1), object.value("offsetY").toInt(-1));

        layer.size = QSize(object.value("width").toInt(), object.value("height").toInt());

        layer.sourceSize =
                QSize(object.value("sourceWidth").toInt(), object.value("sourceHeight").toInt());

        if (filename.isEmpty() || QFileInfo(filename).fileName() != filename || layer.offset.x() < 0
            || layer.offset.y() < 0 || layer.size.isEmpty() || layer.sourceSize.isEmpty()
            || qint64(layer.offset.x()) + layer.size.width() > layer.sourceSize.width()
            || qint64(layer.offset.y()) + layer.size.height() > layer.sourceSize.height()
            || !QFileInfo(layer.filePath).isFile()) {

            qWarning() << "Invalid image map layer:" << it.key();
            return std::nullopt;
        }

        const QJsonObject names = object.value("name").toObject();

        for (auto it = names.constBegin(); it != names.constEnd(); ++it)
            layer.names.insert(it.key(), it.value().toString());

        metadata.layers.insert(it.key(), layer);
    }

    if (!metadata.layers.contains(metadata.defaultLayer)) {
        qWarning() << "Invalid default image map layer:" << metadata.defaultLayer;
        return std::nullopt;
    }

    return metadata;
}

const ImageMapLayer *ImageMapMetadata::layer(const QString &layerId) const
{
    const QString id = layerId.isEmpty() ? defaultLayer : layerId;

    const auto it = layers.constFind(id);

    return it == layers.constEnd() ? nullptr : &it.value();
}
