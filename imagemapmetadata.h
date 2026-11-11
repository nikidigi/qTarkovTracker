#ifndef IMAGEMAPMETADATA_H
#define IMAGEMAPMETADATA_H

#include <QHash>
#include <QPoint>
#include <QSize>
#include <QString>

#include <optional>

struct ImageMapLayer
{
    int zoom = 0;
    QString filePath;
    QPoint offset;
    QSize size;
    QSize sourceSize;
    QHash<QString, QString> names;

    QString name(const QString &language) const
    {
        const QString translated = names.value(language);

        if (!translated.isEmpty())
            return translated;

        return names.value("en");
    }
};

struct ImageMapMetadata
{
    int zoom = 0;
    QString defaultLayer;
    QHash<QString, ImageMapLayer> layers;

    static std::optional<ImageMapMetadata> load(const QString &filePath);

    const ImageMapLayer *layer(const QString &layerId = { }) const;
};

#endif // IMAGEMAPMETADATA_H
