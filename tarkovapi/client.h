#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QNetworkAccessManager>

#include "./cachemanager.h"

#include "./itemsresponse.h"
#include "./mapsresponse.h"
#include "./tasksresponse.h"

namespace TarkovApi {

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);

    void setCacheLifetime(int hours);

    void fetchItems(const QString &mode, const QString &lang);
    void fetchMaps(const QString &mode, const QString &lang);
    void fetchTasks(const QString &mode, const QString &lang);

    const ItemsResponse &itemsResponse() const { return m_itemsResponse; }
    const MapsResponse &mapsResponse() const { return m_mapsResponse; }
    const TasksResponse &tasksResponse() const { return m_tasksResponse; }

signals:
    void itemsUpdated();
    void mapsUpdated();
    void tasksUpdated();

    void errorOccurred(const QString &message);

private:
    QNetworkAccessManager m_network;
    CacheManager m_cache;

    MapsResponse m_mapsResponse;
    TasksResponse m_tasksResponse;
    ItemsResponse m_itemsResponse;

    using JsonCallback = std::function<void(QList<QJsonObject>)>;

    void fetchAll(const QList<QUrl> &urls, JsonCallback callback);
};

} // namespace TarkovApi

#endif // CLIENT_H
