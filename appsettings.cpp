#include "appsettings.h"

int AppSettings::cacheLifetimeHours() const
{
    return m_settings.value("api/cacheLifetimeHours", 24).toInt();
}

void AppSettings::setCacheLifetimeHours(int hours)
{
    m_settings.setValue("api/cacheLifetimeHours", hours);
}

QByteArray AppSettings::geometry()
{
    return m_settings.value("window/geometry").toByteArray();
}

void AppSettings::setGeometry(const QByteArray &value)
{
    m_settings.setValue("window/geometry", value);
}

QString AppSettings::language() const
{
    return m_settings.value("app/language", "en").toString();
}

void AppSettings::setLanguage(const QString &language)
{
    m_settings.setValue("app/language", language);
}

QString AppSettings::mode() const
{
    return m_settings.value("app/mode", "pve").toString();
}

void AppSettings::setMode(const QString &mode)
{
    m_settings.setValue("app/mode", mode);
}

QString AppSettings::screenshotsDirectory() const
{
    return m_settings.value("screenshots/directory").toString();
}

void AppSettings::setScreenshotsDirectory(const QString &value)
{
    m_settings.setValue("screenshots/directory", value);
}

QString AppSettings::selectedMap() const
{
    return m_settings.value("map/selected", "ground-zero").toString();
}

void AppSettings::setSelectedMap(const QString &value)
{
    m_settings.setValue("map/selected", value);
}

QStringList AppSettings::tasks(const QString &map) const
{
    return m_settings.value("tasks/" + map).toStringList();
}

void AppSettings::setTasks(const QString &map, const QStringList &tasks)
{
    m_settings.setValue("tasks/" + map, tasks);
}

bool AppSettings::markerVisible(const QString &group) const
{
    return m_settings.value("markers/" + group, true).toBool();
}

void AppSettings::setMarkerVisible(const QString &group, bool visible)
{
    m_settings.setValue("markers/" + group, visible);
}