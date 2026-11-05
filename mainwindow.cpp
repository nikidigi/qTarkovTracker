#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <algorithm>

#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonValue>
#include <QList>
#include <QResource>
#include <QSet>

#include "mapconfigloader.h"
#include "screenshotparser.h"
#include "tarkovapi/mapitem.h"
#include "widgets/checklistpopup.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->header->initialize();

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    loadMaps();
    loadSettings();

    ui->header->setExtracts({
            { tr("PMC extracts"), "extracts/pmc", m_settings.markerVisible("extracts/pmc") },
            { tr("Scav extracts"), "extracts/scav", m_settings.markerVisible("extracts/scav") },
            { tr("Shared extracts"), "extracts/shared",
              m_settings.markerVisible("extracts/shared") },
            { tr("Transits"), "extracts/transit", m_settings.markerVisible("extracts/transit") },
            { tr("Locks"), "locks", m_settings.markerVisible("locks") },
            { tr("Switches"), "switches", m_settings.markerVisible("switches") },
    });

    connect(ui->header, &HeaderWidget::modeChanged, this, [this](const QString &mode) {
        m_settings.setMode(mode);
        fetchAll();
    });

    connect(ui->header, &HeaderWidget::languageChanged, this, [this](const QString &language) {
        m_settings.setLanguage(language);
        fetchAll();
    });

    connect(ui->header, &HeaderWidget::extractToggled, this,
            [this](const QString &group, bool checked) {
                m_settings.setMarkerVisible(group, checked);
                ui->mapView->setGroupVisible(group, checked);
            });

    connect(ui->header, &HeaderWidget::taskToggled, this, &MainWindow::showTasks);

    ui->mapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->mapView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(ui->mapsComboBox, &QComboBox::currentIndexChanged, this, &MainWindow::updateMap);
    connect(ui->mapsComboBox, &QComboBox::currentIndexChanged, this, &MainWindow::updateTasks);
    connect(ui->browseScreenshotsDirButton, &QToolButton::clicked, this,
            &MainWindow::browseScreenshotsDir);

    connect(&m_screenshotWatcher, &ScreenshotWatcher::screenshotAdded, this,
            &MainWindow::updatePlayerPosition);

    connect(&m_api, &TarkovApi::Client::mapsUpdated, this, [this]() {
        translateMaps();
        updateMap();
        updateTasks();
    });

    connect(&m_api, &TarkovApi::Client::tasksUpdated, this, [this]() {
        updateTasks();
        restoreTasks();
    });

    connect(&m_api, &TarkovApi::Client::itemsUpdated, this, [this]() { updateItems(); });

    connect(&m_api, &TarkovApi::Client::errorOccurred, this, [this](const QString &error) {
        qWarning() << "Tarkov API error:" << error;

        statusBar()->showMessage(tr("Tarkov API error: %1").arg(error), 10000);
    });

    fetchAll();
}

MainWindow::~MainWindow()
{
    m_settings.setGeometry(saveGeometry());
    delete ui;
}

void MainWindow::loadMaps()
{
    QDir mapsDir("maps");
    QFile mapsFile(mapsDir.absoluteFilePath("maps.json"));

    if (!mapsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Error opening file:" << mapsFile.fileName();
        return;
    }

    auto maps = MapConfigLoader::parse(mapsFile.readAll());

    std::sort(maps.begin(), maps.end(),
              [](const MapConfig &a, const MapConfig &b) { return a.id < b.id; });

    ui->mapsComboBox->clear();

    for (const auto &config : maps) {
        const QString svgPath = mapsDir.absoluteFilePath(config.id + ".svg");

        if (!QFileInfo::exists(svgPath))
            continue;

        ui->mapsComboBox->addItem(config.id, QVariant::fromValue(config));
    }
}

void MainWindow::loadSettings()
{
    m_api.setCacheLifetime(m_settings.cacheLifetimeHours());

    ui->header->setMode(m_settings.mode());
    ui->header->setLanguage(m_settings.language());

    ui->mapView->setGroupVisible("extracts/pmc", m_settings.markerVisible("extracts/pmc"));
    ui->mapView->setGroupVisible("extracts/scav", m_settings.markerVisible("extracts/scav"));
    ui->mapView->setGroupVisible("extracts/shared", m_settings.markerVisible("extracts/shared"));
    ui->mapView->setGroupVisible("extracts/transit", m_settings.markerVisible("extracts/transit"));
    ui->mapView->setGroupVisible("locks", m_settings.markerVisible("locks"));
    ui->mapView->setGroupVisible("switches", m_settings.markerVisible("switches"));

    restoreGeometry(m_settings.geometry());

    const QString savedMap = m_settings.selectedMap();

    for (int i = 0; i < ui->mapsComboBox->count(); ++i) {
        const auto config = ui->mapsComboBox->itemData(i).value<MapConfig>();

        if (config.id == savedMap) {
            ui->mapsComboBox->setCurrentIndex(i);
            break;
        }
    }

    if (!m_settings.screenshotsDirectory().isEmpty()) {
        m_screenshotWatcher.setDirectory(m_settings.screenshotsDirectory());
    }
}

void MainWindow::translateMaps()
{
    const auto &response = m_api.mapsResponse();

    for (int i = 0; i < ui->mapsComboBox->count(); ++i) {
        const auto config = ui->mapsComboBox->itemData(i).value<MapConfig>();
        const auto map = response.findByNormalizedName(config.id);

        if (map.isEmpty())
            continue;

        ui->mapsComboBox->setItemText(i, response.translate(map.value("name").toString()));
    }
}

void MainWindow::updateItems()
{
    const auto config = ui->mapsComboBox->currentData().value<MapConfig>();

    if (config.id.isEmpty() || m_api.mapsResponse().isEmpty() || m_api.itemsResponse().isEmpty())
        return;

    const auto map = m_api.mapsResponse().findByNormalizedName(config.id);

    if (map.isEmpty())
        return;

    ui->mapView->removeGroups("locks");

    auto locks = m_api.mapsResponse().locks(config.id);

    QList<TarkovApi::MapItemPtr> items;

    for (const auto &item : locks) {
        auto *lock = static_cast<TarkovApi::MapItemLock *>(item.get());

        if (lock->keyId.isEmpty())
            continue;

        auto lockItem = m_api.itemsResponse().value(lock->keyId).toObject();

        lock->name = m_api.itemsResponse().translate(lockItem.value("name").toString());
        lock->description =
                m_api.itemsResponse().translate(lockItem.value("description").toString());
        lock->wikiLink = m_api.itemsResponse().translate(lockItem.value("wikiLink").toString());

        items.append(item);
    }

    ui->mapView->addItems(items, config);
}

void MainWindow::updateMap()
{
    const MapConfig config = ui->mapsComboBox->currentData().value<MapConfig>();

    if (config.id.isEmpty()) {
        return;
    }

    m_settings.setSelectedMap(config.id);

    ui->mapView->setMapFile("maps/" + config.id + ".svg");
    ui->mapView->clearItems();

    ui->mapView->addItems(m_api.mapsResponse().extracts(config.id), config);
    ui->mapView->addItems(m_api.mapsResponse().transits(config.id), config);
    ui->mapView->addItems(m_api.mapsResponse().switches(config.id), config);

    ui->mapView->addLabels(config);

    restoreTasks();

    updateItems();
    updatePlayerPosition(m_screenshotWatcher.lastScreenshot());
}

void MainWindow::updateTasks()
{
    const auto config = ui->mapsComboBox->currentData().value<MapConfig>();

    if (config.id.isEmpty() || m_api.mapsResponse().isEmpty() || m_api.tasksResponse().isEmpty()) {
        return;
    }

    QList<CheckListPopup::Item> items;
    QSet<QString> addedTasks;
    QStringList saved = m_settings.tasks(config.id);

    const auto map = m_api.mapsResponse().findByNormalizedName(config.id);
    const auto tasks = m_api.tasksResponse().findByMap(map.value("id").toString());

    for (const auto &item : tasks) {
        const auto *task = static_cast<const TarkovApi::MapItemTask *>(item.get());

        if (addedTasks.contains(task->id))
            continue;

        addedTasks.insert(task->id);

        items.append({ task->name, task->id, saved.contains(task->id) });
    }

    ui->header->setTasks(items);
}

void MainWindow::updatePlayerPosition(const QString &filename)
{
    const auto position = ScreenshotParser::parse(filename);

    if (!position)
        return;

    const MapConfig config = ui->mapsComboBox->currentData().value<MapConfig>();

    const QPointF point = config.convert(position->x, position->z);

    ui->mapView->setPlayerPosition(point.x(), point.y(),
                                   config.convertDirection(position->direction));
}

void MainWindow::showTasks(const QVariant &data, bool checked)
{
    const MapConfig config = ui->mapsComboBox->currentData().value<MapConfig>();

    if (config.id.isEmpty()) {
        return;
    }

    const auto taskId = data.toString();
    auto saved = m_settings.tasks(config.id);

    if (checked) {
        if (!saved.contains(taskId))
            saved.append(taskId);
    } else {
        saved.removeAll(taskId);
    }

    m_settings.setTasks(config.id, saved);

    ui->mapView->removeGroup("tasks/" + taskId);

    if (!checked)
        return;

    const auto map = m_api.mapsResponse().findByNormalizedName(config.id);

    if (map.isEmpty()) {
        return;
    }

    const auto tasks = m_api.tasksResponse().findByMap(map.value("id").toString());

    QList<TarkovApi::MapItemPtr> items;

    for (const auto &item : tasks) {
        const auto *task = static_cast<const TarkovApi::MapItemTask *>(item.get());

        if (task->id == taskId)
            items.append(item);
    }

    ui->mapView->addItems(items, config);
}

void MainWindow::restoreTasks()
{
    const auto config = ui->mapsComboBox->currentData().value<MapConfig>();

    if (config.id.isEmpty() || m_api.mapsResponse().isEmpty() || m_api.tasksResponse().isEmpty())
        return;

    const auto map = m_api.mapsResponse().findByNormalizedName(config.id);

    if (map.isEmpty())
        return;

    ui->mapView->removeGroups("tasks/");

    const auto saved = m_settings.tasks(config.id);
    const auto tasks = m_api.tasksResponse().findByMap(map.value("id").toString());

    QList<TarkovApi::MapItemPtr> items;

    for (const auto &item : tasks) {
        const auto *task = static_cast<const TarkovApi::MapItemTask *>(item.get());

        if (saved.contains(task->id))
            items.append(item);
    }

    ui->mapView->addItems(items, config);
}

void MainWindow::browseScreenshotsDir(bool)
{
    const QString directory = QFileDialog::getExistingDirectory(
            this, tr("Select screenshots directory"),
            m_settings.screenshotsDirectory().isEmpty() ? QDir::currentPath()
                                                        : m_settings.screenshotsDirectory());

    if (!directory.isEmpty()) {
        m_settings.setScreenshotsDirectory(directory);
        m_screenshotWatcher.setDirectory(directory);
    }
}

void MainWindow::fetchAll()
{
    const auto mode = m_settings.mode();
    const auto lang = m_settings.language();

    m_api.fetchMaps(mode, lang);
    m_api.fetchTasks(mode, lang);
    m_api.fetchItems(mode, lang);
}
