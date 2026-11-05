#ifndef CHECKLISTPOPUP_H
#define CHECKLISTPOPUP_H

#include <QWidget>
#include <QVariant>
#include <QList>

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QToolButton;

class CheckListPopup : public QWidget
{
    Q_OBJECT

public:
    struct Item
    {
        QString text;
        QVariant data;
        bool checked = false;
    };

    explicit CheckListPopup(QWidget *parent = nullptr);

    void setItems(const QList<Item> &items);
    void setFilterEnabled(bool enabled);
    void setTitle(const QString &title);

    void showBelow(QToolButton *);

signals:
    void itemToggled(const QVariant &data, bool checked);

private:
    QLineEdit *m_filter;
    QListWidget *m_list;

    void filterItems(const QString &text);

    void updateListHeight();
};

#endif // CHECKLISTPOPUP_H
