#include "headerwidget.h"
#include "checklistpopup.h"

#include <QAbstractItemView>
#include <QActionGroup>
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include <QStyle>
#include <QWindow>

QString displayMode(const QString &mode)
{
    if (mode == "regular")
        return "PVP";

    if (mode == "pvp-season")
        return "PVP SEASON";

    return mode.toUpper();
}

HeaderWidget::HeaderWidget(QWidget *parent) : QWidget{ parent } { }

void HeaderWidget::initialize()
{
    initModeButton();
    initLanguageButton();
    initMarkersPopup();
    initTasksPopup();

    auto *mapsComboBox = findChild<QComboBox *>("mapsComboBox");

    // Fix white lines
    mapsComboBox->view()->parentWidget()->setStyleSheet(R"(
        background-color: #252D3B;
        border: none;
    )");

    window()->installEventFilter(this);
    updateMaximizeButton();

    auto *minimizeButton = findChild<QToolButton *>("minimizeButton");
    auto *maximizeButton = findChild<QToolButton *>("maximizeButton");

    connect(minimizeButton, &QToolButton::clicked, this, [this]() { window()->showMinimized(); });

    connect(maximizeButton, &QToolButton::clicked, this, [this]() {
        auto *w = window();

        if (w->isMaximized())
            w->showNormal();
        else
            w->showMaximized();
    });
}

void HeaderWidget::setMode(const QString &mode)
{
    m_modeButton->setText(displayMode(mode));
    m_modeButton->setProperty("gameMode", mode);

    for (auto *action : m_modeButton->menu()->actions()) {
        action->setChecked(action->data().toString() == mode);
    }

    m_modeButton->style()->unpolish(m_modeButton);
    m_modeButton->style()->polish(m_modeButton);
    m_modeButton->update();
}

void HeaderWidget::setLanguage(const QString &language)
{
    m_langButton->setText(language.toUpper());

    for (auto *action : m_langButton->menu()->actions()) {
        action->setChecked(action->data().toString() == language);
    }
}

void HeaderWidget::setExtracts(const QList<CheckListPopup::Item> &items)
{
    m_markersPopup->setItems(items);
}

void HeaderWidget::setTasks(const QList<CheckListPopup::Item> &items)
{
    m_tasksPopup->setItems(items);
}

void HeaderWidget::initModeButton()
{
    m_modeButton = findChild<QToolButton *>("modeButton");
    m_modeButton->setPopupMode(QToolButton::InstantPopup);
    m_modeButton->setCursor(Qt::PointingHandCursor);

    auto *menu = new QMenu(m_modeButton);
    auto *group = new QActionGroup(menu);
    group->setExclusive(true);

    for (const auto &mode : { "pve", "regular", "pvp-season" }) {
        auto *action = menu->addAction(displayMode(mode));

        action->setData(mode);
        action->setCheckable(true);
        group->addAction(action);
    }

    m_modeButton->setMenu(menu);

    connect(menu, &QMenu::triggered, this, [this](QAction *action) {
        const QString mode = action->data().toString();

        setMode(mode);
        emit modeChanged(mode);
    });
}

void HeaderWidget::initLanguageButton()
{
    m_langButton = findChild<QToolButton *>("langButton");
    m_langButton->setPopupMode(QToolButton::InstantPopup);
    m_langButton->setCursor(Qt::PointingHandCursor);

    auto *menu = new QMenu(m_langButton);
    auto *group = new QActionGroup(menu);
    group->setExclusive(true);

    for (const auto &language : { "de", "en", "es", "fr", "ru", "zh" }) {
        auto *action = menu->addAction(QString(language).toUpper());

        action->setData(language);
        action->setCheckable(true);
        group->addAction(action);
    }

    m_langButton->setMenu(menu);

    connect(menu, &QMenu::triggered, this, [this](QAction *action) {
        const QString language = action->data().toString();

        setLanguage(language);
        emit languageChanged(language);
    });
}

void HeaderWidget::initMarkersPopup()
{
    auto *markersButton = findChild<QToolButton *>("markersButton");

    m_markersPopup = new CheckListPopup(this);
    m_markersPopup->setFilterEnabled(false);

    connect(markersButton, &QToolButton::clicked, this,
            [this, markersButton]() { m_markersPopup->showBelow(markersButton); });

    connect(m_markersPopup, &CheckListPopup::itemToggled, this,
            [this](const QVariant &data, bool checked) {
                emit extractToggled(data.toString(), checked);
            });
}

void HeaderWidget::initTasksPopup()
{
    auto *tasksButton = findChild<QToolButton *>("tasksButton");

    m_tasksPopup = new CheckListPopup(this);
    m_tasksPopup->setFilterEnabled(true);

    connect(tasksButton, &QToolButton::clicked, this,
            [this, tasksButton]() { m_tasksPopup->showBelow(tasksButton); });

    connect(m_tasksPopup, &CheckListPopup::itemToggled, this,
            [this](const QVariant &data, bool checked) {
                emit taskToggled(data.toString(), checked);
            });
}

void HeaderWidget::updateMaximizeButton()
{
    const bool maximized = window()->isMaximized();
    auto *maximizeButton = findChild<QToolButton *>("maximizeButton");

    maximizeButton->setIcon(QIcon(maximized ? ":/resources/icons/header/minimize-2.svg"
                                            : ":/resources/icons/header/maximize-2.svg"));
}

bool HeaderWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == window() && event->type() == QEvent::WindowStateChange) {
        updateMaximizeButton();
    }

    return QWidget::eventFilter(watched, event);
}

void HeaderWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (QWindow *handle = window()->windowHandle()) {
            if (handle->startSystemMove()) {
                event->accept();
                return;
            }
        }
    }

    QWidget::mousePressEvent(event);
}
