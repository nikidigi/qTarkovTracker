#include "client.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

#include <utility>

namespace TarkovApi {

Client::Client(QObject *parent) : QObject(parent) { }

void Client::setCacheLifetime(int hours)
{
    m_cache.setLifetime(hours * 60);
}

void Client::fetchItems(const QString &mode, const QString &lang)
{
    fetchAll({ QUrl(QString("https://json.tarkov.dev/%1/items").arg(mode)),
               QUrl(QString("https://json.tarkov.dev/%1/items_%2").arg(mode, lang)) },
             [this](const QList<QJsonObject> &results) {
                 if (results.size() != 2)
                     return;

                 const auto items = results[0].value("data").toObject().value("items").toObject();

                 const auto translations = results[1].value("data").toObject();

                 m_itemsResponse = ItemsResponse(items, translations);

                 emit itemsUpdated();
             });
}

void Client::fetchMaps(const QString &mode, const QString &lang)
{
    fetchAll({ QUrl(QString("https://json.tarkov.dev/%1/maps").arg(mode)),
               QUrl(QString("https://json.tarkov.dev/%1/maps_%2").arg(mode).arg(lang)) },
             [this](const QList<QJsonObject> &results) {
                 const auto maps = results[0].value("data").toObject().value("maps").toObject();

                 const auto translations = results[1].value("data").toObject();

                 m_mapsResponse = MapsResponse(maps, translations);

                 emit mapsUpdated();
             });
}

void Client::fetchTasks(const QString &mode, const QString &lang)
{
    fetchAll({ QUrl(QString("https://json.tarkov.dev/%1/tasks").arg(mode)),
               QUrl(QString("https://json.tarkov.dev/%1/tasks_%2").arg(mode, lang)) },
             [this](const QList<QJsonObject> &results) {
                 const auto tasks = results[0].value("data").toObject().value("tasks").toObject();

                 const auto translations = results[1].value("data").toObject();

                 m_tasksResponse = TasksResponse(tasks, translations);

                 emit tasksUpdated();
             });
}

void Client::fetchAll(const QList<QUrl> &urls, JsonCallback callback)
{
    struct State
    {
        QList<QJsonObject> results;
        int remaining;
        bool failed = false;
    };

    auto state = std::make_shared<State>();
    state->results.resize(urls.size());
    state->remaining = urls.size();

    if (urls.isEmpty()) {
        callback({ });
        return;
    }

    auto finish = [this, state, callback](qsizetype index, const QJsonObject &result,
                                          bool success) {
        if (success)
            state->results[index] = result;
        else
            state->failed = true;

        if (--state->remaining == 0 && !state->failed) {
            QTimer::singleShot(0, this,
                               [state, callback]() { callback(std::move(state->results)); });
        }
    };

    for (qsizetype i = 0; i < urls.size(); ++i) {
        const QUrl url = urls[i];

        if (const auto cached = m_cache.get(url)) {
            finish(i, *cached, true);
            continue;
        }

        QNetworkRequest req(url);

        req.setHeader(QNetworkRequest::UserAgentHeader, "qTarkovTracker");

        auto *reply = m_network.get(req);

        connect(reply, &QNetworkReply::finished, this, [this, reply, url, i, finish]() {
            reply->deleteLater();

            if (reply->error() != QNetworkReply::NoError) {
                emit errorOccurred(reply->errorString());
                finish(i, { }, false);
                return;
            }

            QJsonParseError error;
            const auto document = QJsonDocument::fromJson(reply->readAll(), &error);

            if (error.error != QJsonParseError::NoError || !document.isObject()) {
                emit errorOccurred("Invalid JSON response");
                finish(i, { }, false);
                return;
            }

            const QJsonObject data = document.object();

            m_cache.put(url, data);
            finish(i, data, true);
        });
    }
}

} // namespace TarkovApi