#include "mapview.h"

#include "iconmapmarker.h"

#include <QtDebug>
#include <QDir>
#include <QSvgRenderer>

MapView::MapView(QWidget *parent) : QGraphicsView{ parent }
{
    auto *scene = new QGraphicsScene(this);

    setScene(scene);

    setDragMode(QGraphicsView::ScrollHandDrag);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setBackgroundBrush(Qt::darkGray);
}

void MapView::clearItems()
{
    qDeleteAll(m_markers);
    m_markers.clear();
}

void MapView::setMapFile(const QString &fileName)
{
    scene()->clear();
    m_markers.clear();

    m_map = new QGraphicsSvgItem(fileName);
    m_playerMarker = nullptr;

    resetTransform();
    scene()->addItem(m_map);
    // scene()->setSceneRect(m_map->boundingRect());

    const QRectF bounds = m_map->sceneBoundingRect();
    const qreal margin = bounds.width() * 0.15;

    scene()->setSceneRect(bounds.adjusted(-margin, -margin, margin, margin));
}

void MapView::setMapWidth(int w)
{
    const qreal mapWidth = sceneRect().width();

    if (mapWidth > 0) {
        const qreal factor = w / mapWidth;

        scale(factor, factor);
    }
}

void MapView::removeGroup(const QString &group)
{
    for (auto it = m_markers.begin(); it != m_markers.end();) {
        MapMarker *marker = *it;

        if (marker->group() == group) {
            it = m_markers.erase(it);
            delete marker;
        } else {
            ++it;
        }
    }
}

void MapView::removeGroups(const QString &prefix)
{
    for (auto it = m_markers.begin(); it != m_markers.end();) {
        MapMarker *marker = *it;

        if (marker->group().startsWith(prefix)) {
            it = m_markers.erase(it);
            delete marker;
        } else {
            ++it;
        }
    }
}

void MapView::setGroupVisible(const QString &group, bool visible)
{
    m_groupVisibility[group] = visible;

    for (auto *marker : m_markers) {
        if (marker->group() == group)
            marker->setVisible(visible);
    }
}

void MapView::setPlayerPosition(double nx, double ny, double direction)
{
    if (!m_map) {
        return;
    }

    if (!m_playerMarker) {
        m_playerMarker = new ArrowMapMarker();
        m_playerMarker->setZValue(MarkerZ::Player);
        scene()->addItem(m_playerMarker);
    }

    const QRectF bounds = m_map->sceneBoundingRect();

    m_playerMarker->setPos(bounds.left() + nx * bounds.width(),
                           bounds.top() + ny * bounds.height());

    m_playerMarker->setRotation(direction);
    m_playerMarker->show();
}

void MapView::addItems(const QList<TarkovApi::MapItemPtr> &items, const MapConfig &config)
{
    if (!m_map) {
        return;
    }

    for (const auto &item : items) {
        switch (item->type) {
        case TarkovApi::MapItemType::Extract: {
            auto *extract = static_cast<TarkovApi::MapItemExtract *>(item.get());
            if (!extract)
                break;

            const QPointF point = config.convert(extract->position.x(), extract->position.z());

            auto *marker = new IconMapMarker(
                    QPixmap(":/resources/icons/markers/extract_" + extract->faction + ".webp"));

            const QRectF bounds = m_map->sceneBoundingRect();

            marker->setPos(bounds.left() + point.x() * bounds.width(),
                           bounds.top() + point.y() * bounds.height());

            marker->setLabel(extract->name);
            marker->setGroup("extracts/" + extract->faction);
            marker->setVisible(m_groupVisibility.value(marker->group(), true));
            marker->setZValue(MarkerZ::Extract);

            m_markers.append(marker);
            scene()->addItem(marker);
            break;
        }

        case TarkovApi::MapItemType::Transit: {
            auto *transit = static_cast<TarkovApi::MapItemTransit *>(item.get());
            if (!transit)
                break;

            const QPointF point = config.convert(transit->position.x(), transit->position.z());

            auto *marker =
                    new IconMapMarker(QPixmap(":/resources/icons/markers/extract_transit.webp"));

            const QRectF bounds = m_map->sceneBoundingRect();

            marker->setPos(bounds.left() + point.x() * bounds.width(),
                           bounds.top() + point.y() * bounds.height());

            marker->setLabel(transit->description);
            marker->setGroup("extracts/transit");
            marker->setVisible(m_groupVisibility.value(marker->group(), true));
            marker->setZValue(MarkerZ::Transit);

            m_markers.append(marker);
            scene()->addItem(marker);
            break;
        }

        case TarkovApi::MapItemType::Task: {
            auto *task = static_cast<TarkovApi::MapItemTask *>(item.get());
            if (!task)
                break;

            const QPointF point = config.convert(task->position.x(), task->position.z());

            auto *marker = new IconMapMarker(QPixmap(":/resources/icons/markers/circle-alert.svg"));

            const QRectF bounds = m_map->sceneBoundingRect();

            marker->setPos(bounds.left() + point.x() * bounds.width(),
                           bounds.top() + point.y() * bounds.height());

            marker->setLabel(task->name);
            marker->setMapToolTip(task->name, task->description,
                                  QString("<a style=\"color:#83A8E8\" href=\"%1\">%2</a>")
                                          .arg(task->wikiLink)
                                          .arg(tr("View on Wiki")));

            marker->setGroup("tasks/" + task->id);
            marker->setVisible(m_groupVisibility.value(marker->group(), true));
            marker->setZValue(MarkerZ::Task);

            m_markers.append(marker);
            scene()->addItem(marker);
            break;
        }

        case TarkovApi::MapItemType::Lock: {
            auto *lock = static_cast<TarkovApi::MapItemLock *>(item.get());
            if (!lock)
                break;

            const QPointF point = config.convert(lock->position.x(), lock->position.z());

            auto *marker = new IconMapMarker(QPixmap(":/resources/icons/markers/lock.svg"));

            const QRectF bounds = m_map->sceneBoundingRect();

            marker->setPos(bounds.left() + point.x() * bounds.width(),
                           bounds.top() + point.y() * bounds.height());

            QString description = "<p>" + lock->description + "</p>";

            if (lock->needsPower) {
                description += QString("<p style=\"color:#F2C879\"><b>%1</b></p>")
                                       .arg(tr("Requires power"));
            }

            marker->setMapToolTip(lock->name, description,
                                  QString("<p><a style=\"color:#83A8E8\" href=\"%1\">%2</a></p>")
                                          .arg(lock->wikiLink.toHtmlEscaped(), tr("View on Wiki")));

            marker->setGroup("locks");
            marker->setVisible(m_groupVisibility.value(marker->group(), true));
            marker->setZValue(MarkerZ::Lock);
            marker->resize(18, 18);

            m_markers.append(marker);
            scene()->addItem(marker);
            break;
        }

        case TarkovApi::MapItemType::Switch: {
            auto *sw = static_cast<TarkovApi::MapItemSwitch *>(item.get());
            if (!sw)
                break;

            const QPointF point = config.convert(sw->position.x(), sw->position.z());

            auto *marker = new IconMapMarker(QPixmap(":/resources/icons/markers/zap.svg"));

            const QRectF bounds = m_map->sceneBoundingRect();

            marker->setPos(bounds.left() + point.x() * bounds.width(),
                           bounds.top() + point.y() * bounds.height());

            marker->setMapToolTip(sw->name, "<p>" + sw->name + "</p>");

            marker->setGroup("switches");
            marker->setVisible(m_groupVisibility.value(marker->group(), true));
            marker->setZValue(MarkerZ::Switch);
            marker->resize(18, 18);

            m_markers.append(marker);
            scene()->addItem(marker);
            break;
        }
        }
    }
}

void MapView::addLabels(const MapConfig &config)
{
    if (!m_map)
        return;

    const QRectF mapRect = m_map->sceneBoundingRect();

    constexpr double referenceWidth = 1472.7926; // Woods
    constexpr double referenceFontSize = 12.0;
    const int minFontSize = config.id == "factory" ? 2 : 6;

    const double fontScale = m_map->boundingRect().width() / referenceWidth;

    QFont font;
    font.setPixelSize(std::max(qRound(referenceFontSize * fontScale), minFontSize));
    font.setBold(true);

    for (const auto &label : config.labels) {
        const QPointF pos = config.convert(label.position.x(), label.position.y());
        auto *item = new QGraphicsSimpleTextItem(label.text);

        item->setFont(font);
        item->setBrush(QColor("#D2D8E2"));
        item->setScale(label.size / 100.0);
        item->setRotation(label.rotation);

        item->setTransformOriginPoint(item->boundingRect().center());

        const QPointF scenePos(mapRect.left() + pos.x() * mapRect.width(),
                               mapRect.top() + pos.y() * mapRect.height());

        item->setPos(scenePos - item->transformOriginPoint());

        item->setZValue(MarkerZ::Label);
        scene()->addItem(item);
    }
}

void MapView::wheelEvent(QWheelEvent *event)
{
    const double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;

    scale(factor, factor);
}
