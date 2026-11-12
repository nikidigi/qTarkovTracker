#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <algorithm>

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonValue>
#include <QList>
#include <QSet>
#include <QSignalBlocker>
#include <utility>

#include "mapconfigloader.h"
#include "screenshotparser.h"
#include "tarkovapi/mapitem.h"
#include "widgets/checklistpopup.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->header->initialize();

    setWindowTitle(QString("qTarkovTracker %1").arg(QApplication::applicationVersion()));
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
        const QString currentLayer = ui->layersComboBox->currentData().toString();
        updateMapLayers(currentLayer);
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

    connect(ui->mapsComboBox, &QComboBox::currentIndexChanged, this, [this]() {
        const QString mapId = ui->mapsComboBox->currentData().value<MapConfig>().id;
        updateMapLayers(m_settings.selectedMapLayer(mapId));
        updateMap();
    });

    connect(ui->layersComboBox, &QComboBox::currentIndexChanged, this, [this]() { updateMap(); });

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

    QTimer::singleShot(100, [this]() {
        fetchAll();
    });
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
        const QString metadataPath = mapsDir.absoluteFilePath(config.id + "/metadata.json");

        if (!QFileInfo(svgPath).isFile() && !QFileInfo(metadataPath).isFile())
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

    {
        const QSignalBlocker blocker(ui->mapsComboBox);
        const QString savedMap = m_settings.selectedMap();

        for (int i = 0; i < ui->mapsComboBox->count(); ++i) {
            const auto config = ui->mapsComboBox->itemData(i).value<MapConfig>();
            if (config.id == savedMap) {
                ui->mapsComboBox->setCurrentIndex(i);
                break;
            }
        }
    }

    const QString mapId = ui->mapsComboBox->currentData().value<MapConfig>().id;
    updateMapLayers(m_settings.selectedMapLayer(mapId));

    if (!m_settings.screenshotsDirectory().isEmpty())
        m_screenshotWatcher.setDirectory(m_settings.screenshotsDirectory());
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

const ImageMapMetadata *MainWindow::imageMapMetadata(const QString &mapId)
{
    if (mapId.isEmpty())
        return nullptr;

    auto it = m_imageMaps.constFind(mapId);
    if (it == m_imageMaps.cend()) {
        const QString path = QDir("maps").filePath(mapId + "/metadata.json");
        if (!QFileInfo(path).isFile())
            return nullptr;

        auto metadata = ImageMapMetadata::load(path);
        if (!metadata)
            return nullptr;

        it = m_imageMaps.insert(mapId, std::move(*metadata));
    }

    return &it.value();
}

void MainWindow::updateMap()
{
    const MapConfig config = ui->mapsComboBox->currentData().value<MapConfig>();

    if (config.id.isEmpty())
        return;

    const QDir mapsDir("maps");

    const QString svgPath = mapsDir.filePath(config.id + ".svg");

    const QString metadataPath = mapsDir.filePath(config.id + "/metadata.json");

    const QString layerId = ui->layersComboBox->currentData().toString();

    bool loaded = false;

    if (QFileInfo(metadataPath).isFile()) {
        const ImageMapMetadata *metadata = imageMapMetadata(config.id);
        if (!metadata)
            return;

        const ImageMapLayer *layer = metadata->layer(layerId);

        if (!layer) {
            qWarning() << "Unknown image map layer:" << layerId;
            return;
        }

        loaded = ui->mapView->loadImageMap(*layer);
    } else if (QFileInfo(svgPath).isFile()) {
        loaded = ui->mapView->loadSvgMap(svgPath);
    }

    if (!loaded) {
        qWarning() << "Cannot load map:" << config.id;
        return;
    }

    m_settings.setSelectedMap(config.id, layerId);

    ui->mapView->clearItems();

    ui->mapView->addItems(m_api.mapsResponse().extracts(config.id), config);
    ui->mapView->addItems(m_api.mapsResponse().transits(config.id), config);
    ui->mapView->addItems(m_api.mapsResponse().switches(config.id), config);

    ui->mapView->addLabels(config);

    restoreTasks();

    updateItems();
    updatePlayerPosition(m_screenshotWatcher.lastScreenshot());
}

void MainWindow::updateMapLayers(const QString &preferredLayer)
{
    const QString mapId = ui->mapsComboBox->currentData().value<MapConfig>().id;
    const QSignalBlocker blocker(ui->layersComboBox);

    ui->layersComboBox->clear();
    ui->layersComboBox->setEnabled(false);

    const ImageMapMetadata *metadata = imageMapMetadata(mapId);
    if (!metadata) {
         ui->layersComboBox->setVisible(false);
        return;
    }

    QStringList layerIds = metadata->layers.keys();
    layerIds.sort();

    for (const QString &id : layerIds) {
        const auto it = metadata->layers.constFind(id);
        const QString name = it.value().name(m_settings.language());

        ui->layersComboBox->addItem(name.isEmpty() ? id : name, id);
    }

    int index = ui->layersComboBox->findData(preferredLayer);
    if (index < 0)
        index = ui->layersComboBox->findData(metadata->defaultLayer);
    if (index < 0 && ui->layersComboBox->count() > 0)
        index = 0;

    ui->layersComboBox->setCurrentIndex(index);
    ui->layersComboBox->setEnabled(index >= 0);
    ui->layersComboBox->setVisible(true);
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

    const auto config = ui->mapsComboBox->currentData().value<MapConfig>();

    ui->mapView->setPlayerPosition(config, position->x, position->z,
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
