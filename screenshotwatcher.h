#ifndef SCREENSHOTWATCHER_H
#define SCREENSHOTWATCHER_H

#include <QObject>
#include <QTimer>
#include <QString>

class ScreenshotWatcher : public QObject
{
    Q_OBJECT

public:
    explicit ScreenshotWatcher(QObject *parent = nullptr);

    inline const QString &lastScreenshot() const { return m_lastScreenshot; }

    void setDirectory(const QString &path);

signals:
    void screenshotAdded(const QString &filename);

private:
    QTimer m_timer;
    QString m_directory;
    QString m_lastScreenshot;

    QString latestScreenshot() const;
    void checkDirectory();
};

#endif // SCREENSHOTWATCHER_H
