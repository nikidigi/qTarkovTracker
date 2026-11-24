#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <qnamespace.h>

#include <QDir>
#include <QFileDialog>

#include "screenshotparser.h"
#include "tarkovapi/cachemanager.h"

SettingsDialog::SettingsDialog(const AppSettings &settings, QWidget *parent):
    m_settings(settings),
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    setWindowFlags(
            Qt::Dialog |
            Qt::CustomizeWindowHint |
            Qt::WindowTitleHint |
            Qt::WindowCloseButtonHint
            );

    connect(ui->okButton, &QPushButton::clicked,
            this, &QDialog::accept);

    connect(ui->cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);

    connect(ui->removeScreenshotsButton, &QPushButton::clicked, this, &SettingsDialog::clearScreenshots);
    connect(ui->clearCacheButton, &QPushButton::clicked, this, &SettingsDialog::clearCache);

    connect(ui->browseButton, &QPushButton::clicked, this, &SettingsDialog::browseScreenshotsDir);

    ui->screenshotsDirectoryEdit->setText(m_settings.screenshotsDirectory());
    ui->cacheLifeTimeHours->setValue(m_settings.cacheLifetimeHours());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

QString SettingsDialog::screenshotsDirectory() const
{
    return ui->screenshotsDirectoryEdit->text();
}

int SettingsDialog::cacheLifetimeHours() const
{
    return ui->cacheLifeTimeHours->value();
}

void SettingsDialog::clearScreenshots()
{
    QDir dir(ui->screenshotsDirectoryEdit->text());

    if (!dir.exists()) {
        ui->screenshotsDescription->showStatus(
                tr("Screenshots directory does not exist"),
                "error"
                );
        return;
    }

    const QFileInfoList files =
            dir.entryInfoList({ "*.png", "*.PNG" }, QDir::Files);

    int removedFiles = 0;

    for (const auto &file : files) {
        const auto position = ScreenshotParser::parse(file.fileName());

        if (position && QFile::remove(file.absoluteFilePath()))
            ++removedFiles;
    }

    ui->screenshotsDescription->showStatus(
            tr("%1 of %2 screenshots removed")
                    .arg(removedFiles)
                    .arg(files.count()),
            "success"
            );
}

void SettingsDialog::clearCache()
{
    const bool cleared = TarkovApi::CacheManager::clear();

    ui->cacheDescription->showStatus(
            cleared ? tr("Cache cleared") : tr("Error clearing cache"),
            cleared ? "success" : "error"
            );
}

void SettingsDialog::browseScreenshotsDir()
{
    const QString directory = QFileDialog::getExistingDirectory(
            this, tr("Select screenshots directory"),
            ui->screenshotsDirectoryEdit->text().isEmpty() ? QDir::currentPath()
                                                        : ui->screenshotsDirectoryEdit->text());

    if (!directory.isEmpty())
        ui->screenshotsDirectoryEdit->setText(directory);
}