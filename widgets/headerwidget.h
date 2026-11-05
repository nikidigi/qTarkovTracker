#ifndef HEADERWIDGET_H
#define HEADERWIDGET_H

#include "checklistpopup.h"

#include <QMainWindow>
#include <QMouseEvent>
#include <QToolButton>
#include <QWidget>
#include <QList>

class HeaderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HeaderWidget(QWidget *parent = nullptr);

    // Must be called after ui->setupUi() in MainWindow
    void initialize();

    void setMode(const QString &mode);
    void setLanguage(const QString &language);

    void setExtracts(const QList<CheckListPopup::Item> &items);
    void setTasks(const QList<CheckListPopup::Item> &items);

signals:
    void modeChanged(const QString &mode);
    void languageChanged(const QString &language);

    void extractToggled(const QString &group, bool checked);
    void taskToggled(const QString &id, bool checked);

private:
    QToolButton *m_modeButton = nullptr;
    QToolButton *m_langButton = nullptr;

    CheckListPopup *m_markersPopup = nullptr;
    CheckListPopup *m_tasksPopup = nullptr;

    void initModeButton();
    void initLanguageButton();

    void initMarkersPopup();
    void initTasksPopup();

    void updateMaximizeButton();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
};

#endif // HEADERWIDGET_H
