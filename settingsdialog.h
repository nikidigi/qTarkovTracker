#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

#include "appsettings.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const AppSettings &settings, QWidget *parent = nullptr);
    ~SettingsDialog();

    QString screenshotsDirectory() const;
    int cacheLifetimeHours() const;

public slots:
    void clearScreenshots();
    void clearCache();

    void browseScreenshotsDir();

private:
    Ui::SettingsDialog *ui;

    const AppSettings &m_settings;
};

#endif // SETTINGSDIALOG_H
