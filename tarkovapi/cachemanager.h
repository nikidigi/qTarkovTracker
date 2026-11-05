#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H

#include <QJsonObject>
#include <QUrl>

#include <optional>

namespace TarkovApi {

class CacheManager
{
public:
    explicit CacheManager(int lifetimeSeconds = 86400);

    void setLifetime(int seconds);

    std::optional<QJsonObject> get(const QUrl &url) const;
    bool put(const QUrl &url, const QJsonObject &data) const;

    void clear();

private:
    QString filePath(const QUrl &url) const;

    int m_lifetime;
};

} // namespace TarkovApi

#endif // CACHEMANAGER_H
