#include "checklistpopup.h"

#include <algorithm>

#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSignalBlocker>
#include <QToolButton>
#include <QVBoxLayout>

CheckListPopup::CheckListPopup(QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setObjectName("checkListPopup");
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedWidth(300);

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

    m_list->setMinimumHeight(80);
    m_list->setMaximumHeight(400);

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
    updateListHeight();
}

void CheckListPopup::setFilterEnabled(bool enabled)
{
    m_filter->setVisible(enabled);
}

void CheckListPopup::showBelow(QToolButton *button)
{
    const QPoint position = button->mapToGlobal(QPoint(0, button->height() + 4));

    move(position);
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
}

void CheckListPopup::updateListHeight()
{
    int visibleCount = 0;

    for (int i = 0; i < m_list->count(); ++i) {
        if (!m_list->item(i)->isHidden())
            ++visibleCount;
    }

    constexpr int rowHeight = 38;

    const int contentHeight = visibleCount * rowHeight;

    m_list->setFixedHeight(std::clamp(contentHeight, 80, 400));

    adjustSize();
}