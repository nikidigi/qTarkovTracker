#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>
#include <QString>
#include <QByteArray>
#include <QStringList>

class AppSettings
{
public:
    int cacheLifetimeHours() const;
    void setCacheLifetimeHours(int hours);

    QByteArray geometry();
    void setGeometry(const QByteArray &geometry);

    QString language() const;
    void setLanguage(const QString &language);

    QString mode() const;
    void setMode(const QString &mode);

    QString screenshotsDirectory() const;
    void setScreenshotsDirectory(const QString &path);

    QString selectedMap() const;
    void setSelectedMap(const QString &map);

    QStringList tasks(const QString &map) const;
    void setTasks(const QString &map, const QStringList &tasks);

    bool markerVisible(const QString &group) const;
    void setMarkerVisible(const QString &group, bool visible);

private:
    QSettings m_settings{
        QSettings::IniFormat,
        QSettings::UserScope,
        "qTarkovTracker",
        "qTarkovTracker"
    };
};

#endif // APPSETTINGS_H
