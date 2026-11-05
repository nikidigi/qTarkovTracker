#ifndef MAPTOOLTIP_H
#define MAPTOOLTIP_H

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QToolButton>
#include <QWidget>

class MapToolTip : public QWidget
{
    Q_OBJECT
public:
    explicit MapToolTip(QWidget *parent = nullptr);

    void setTitle(const QString &header);
    void setDescription(const QString &html);
    void setLink(const QString &link);

private:
    QLabel *m_title;
    QLabel *m_description;
    QLabel *m_link;

    QHBoxLayout *m_header;

    bool m_dragging = false;
    QPoint m_dragOffset;
    int m_headerHeight = 36;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // MAPTOOLTIP_H
