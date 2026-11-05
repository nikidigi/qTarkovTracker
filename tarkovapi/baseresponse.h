#ifndef BASERESPONSE_H
#define BASERESPONSE_H

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

class BaseResponse
{
public:
    BaseResponse() = default;
    BaseResponse(QJsonObject data, QJsonObject translations);

    const QJsonObject &data() const { return m_data; }
    const QJsonObject &translations() const { return m_translations; }

    QJsonValue value(const QString &key) const { return m_data.value(key); }

    QString translate(const QString &id) const;
    bool isEmpty() const { return m_data.isEmpty(); }

protected:
    QJsonObject m_data;
    QJsonObject m_translations;
};

#endif // BASERESPONSE_H
