#include "checklistpopup.h"

#include <algorithm>

#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScreen>
#include <QSignalBlocker>
#include <QToolButton>
#include <QVBoxLayout>

#include <QGuiApplication>

CheckListPopup::CheckListPopup(QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    setObjectName("checkListPopup");
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedWidth(300);
    setMaximumHeight(400);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(2, 2, 2, 2);
    outerLayout->setSpacing(0);

    auto *container = new QFrame(this);
    container->setObjectName("popupContainer");
    outerLayout->addWidget(container);

    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(9, 9, 9, 9);
    layout->setSpacing(8);

    m_filter = new QLineEdit(container);
    m_filter->setObjectName("popupFilter");
    m_filter->setPlaceholderText(tr("Search..."));
    m_filter->setClearButtonEnabled(true);

    m_list = new QListWidget(container);
    m_list->setObjectName("popupList");
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    m_list->setMouseTracking(true);
    m_list->setSpacing(2);

    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    layout->addWidget(m_filter);
    layout->addWidget(m_list);

    connect(m_filter, &QLineEdit::textChanged, this, &CheckListPopup::filterItems);

    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        auto newState = item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked;

        item->setCheckState(newState);

        emit itemToggled(item->data(Qt::UserRole), newState == Qt::Checked);
    });
}

void CheckListPopup::setItems(const QList<Item> &items)
{
    QSignalBlocker blocker(m_list);

    m_list->clear();

    for (const auto &entry : items) {
        auto *item = new QListWidgetItem(entry.text, m_list);

        item->setData(Qt::UserRole, entry.data);
        item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
        item->setCheckState(entry.checked ? Qt::Checked : Qt::Unchecked);

        item->setSizeHint(QSize(0, 38));
    }

    m_list->doItemsLayout();
    m_list->viewport()->update();

    filterItems(m_filter->text());
}

void CheckListPopup::setFilterEnabled(bool enabled)
{
    m_filter->setVisible(enabled);
}

void CheckListPopup::showBelow(QToolButton *button)
{
    m_filter->clear();

    QScreen *screen = button->screen();

    if (!screen)
        screen = QGuiApplication::primaryScreen();

    if (!screen)
        return;

    const QPoint buttonPos = button->mapToGlobal(QPoint(0, 0));
    const QRect available = screen->availableGeometry();

    constexpr int spacing = 4;

    const QSize popupSize = size();

    int x = buttonPos.x();
    int y = buttonPos.y() + button->height() + spacing;

    if (y + popupSize.height() > available.bottom() + 1) {
        y = buttonPos.y() - popupSize.height() - spacing;
    }

    x = std::clamp(
            x,
            available.left(),
            std::max(available.left(), available.right() + 1 - popupSize.width())
            );

    y = std::clamp(
            y,
            available.top(),
            std::max(available.top(), available.bottom() + 1 - popupSize.height())
            );

    move(x, y);
    show();

    button->setDown(false);
    button->clearFocus();
    button->update();
}

void CheckListPopup::filterItems(const QString &text)
{
    for (int i = 0; i < m_list->count(); ++i) {
        auto *item = m_list->item(i);
        item->setHidden(!item->text().contains(text, Qt::CaseInsensitive));
    }

    updateListHeight();
}

void CheckListPopup::updateListHeight()
{
    int visibleCount = 0;

    for (int i = 0; i < m_list->count(); ++i) {
        if (!m_list->item(i)->isHidden())
            ++visibleCount;
    }

    constexpr int rowHeight = 38;
    constexpr int maxHeight = 400;

    const int height = std::min(
            visibleCount * (rowHeight + m_list->spacing()),
            maxHeight
            );

    m_list->setFixedHeight(std::max(height, rowHeight));

    m_list->updateGeometry();

    auto *innerLayout = m_list->parentWidget()->layout();
    innerLayout->invalidate();
    innerLayout->activate();

    layout()->invalidate();
    layout()->activate();

    resize(width(), layout()->sizeHint().height());
}
