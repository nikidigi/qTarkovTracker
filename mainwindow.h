#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "appsettings.h"
#include "screenshotwatcher.h"

#include "tarkovapi/client.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void loadMaps();
    void loadSettings();

public slots:
    void translateMaps();

    void updateItems();
    void updateMap();
    void updateTasks();
    void updatePlayerPosition(const QString &);

    void showTasks(const QVariant &, bool);
    void restoreTasks();

    void browseScreenshotsDir(bool checked = false);

private:
    Ui::MainWindow *ui;

    AppSettings m_settings;
    TarkovApi::Client m_api;
    ScreenshotWatcher m_screenshotWatcher;

    void fetchAll();
};
#endif // MAINWINDOW_H
