#include "screenshotwatcher.h"

#include <QDir>
#include <QFileInfo>

ScreenshotWatcher::ScreenshotWatcher(QObject *parent) : QObject(parent)
{
    m_timer.setInterval(1000);

    connect(&m_timer, &QTimer::timeout, this, &ScreenshotWatcher::checkDirectory);
}

void ScreenshotWatcher::setDirectory(const QString &path)
{
    m_timer.stop();

    m_directory = path;
    m_lastScreenshot.clear();

    if (!QDir(path).exists())
        return;

    m_lastScreenshot = latestScreenshot();

    m_timer.start();
}

QString ScreenshotWatcher::latestScreenshot() const
{
    QDir dir(m_directory);

    const QFileInfoList files = dir.entryInfoList({ "*.png", "*.PNG" }, QDir::Files, QDir::Time);

    return files.isEmpty() ? QString() : files.first().absoluteFilePath();
}

void ScreenshotWatcher::checkDirectory()
{
    const QString filename = latestScreenshot();

    if (filename.isEmpty() || filename == m_lastScreenshot)
        return;

    m_lastScreenshot = filename;
    emit screenshotAdded(filename);
}