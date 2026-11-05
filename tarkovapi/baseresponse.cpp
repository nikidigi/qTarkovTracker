#include "baseresponse.h"

BaseResponse::BaseResponse(QJsonObject data, QJsonObject translations)
    : m_data(std::move(data)), m_translations(std::move(translations))
{
}

QString BaseResponse::translate(const QString &key) const
{
    const QString value = m_translations.value(key).toString(key).trimmed();

    return value.isEmpty() ? key : value;
}