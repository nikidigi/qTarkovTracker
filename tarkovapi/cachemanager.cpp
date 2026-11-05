#include "cachemanager.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>

namespace TarkovApi {

CacheManager::CacheManager(int lifetimeSeconds) : m_lifetime(qMax(0, lifetimeSeconds)) { }

void CacheManager::setLifetime(int seconds)
{
    m_lifetime = qMax(0, seconds);
}

QString CacheManager::filePath(const QUrl &url) const
{
    const QString directory =
            QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/api";

    const QByteArray hash = QCryptographicHash::hash(url.toString(QUrl::FullyEncoded).toUtf8(),
                                                     QCryptographicHash::Sha256)
                                    .toHex();

    return directory + "/" + QString::fromLatin1(hash) + ".json";
}

std::optional<QJsonObject> CacheManager::get(const QUrl &url) const
{
    if (m_lifetime <= 0)
        return std::nullopt;

    const QFileInfo info(filePath(url));

    if (!info.isFile())
        return std::nullopt;

    const qint64 age = info.lastModified().secsTo(QDateTime::currentDateTimeUtc());

    if (age < 0 || age >= m_lifetime)
        return std::nullopt;

    QFile file(info.absoluteFilePath());

    if (!file.open(QIODevice::ReadOnly))
        return std::nullopt;

    QJsonParseError error;

    const auto document = QJsonDocument::fromJson(file.readAll(), &error);

    if (error.error != QJsonParseError::NoError || !document.isObject())
        return std::nullopt;

    return document.object();
}

bool CacheManager::put(const QUrl &url, const QJsonObject &data) const
{
    if (m_lifetime <= 0)
        return false;

    const QString path = filePath(url);

    if (!QDir().mkpath(QFileInfo(path).absolutePath()))
        return false;

    QSaveFile file(path);

    if (!file.open(QIODevice::WriteOnly))
        return false;

    const QByteArray json = QJsonDocument(data).toJson(QJsonDocument::Compact);

    return file.write(json) == json.size() && file.commit();
}

void CacheManager::clear()
{
    const QString directory =
            QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/api";

    QDir(directory).removeRecursively();
}

} // namespace TarkovApi