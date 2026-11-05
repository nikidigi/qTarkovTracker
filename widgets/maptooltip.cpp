#include "maptooltip.h"

MapToolTip::MapToolTip(QWidget *parent) : QWidget{ parent }
{
    setWindowFlags(Qt::Widget);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_StyledBackground, true);
    setCursor(Qt::ArrowCursor);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    m_header = new QHBoxLayout();
    m_header->setContentsMargins(0, 0, 0, 0);

    m_title = new QLabel(this);
    m_title->setObjectName("tooltipTitle");
    m_title->setText("Title");

    auto *closeButton = new QToolButton(this);
    closeButton->setIcon(QPixmap(":/resources/icons/header/x.svg"));
    closeButton->setIconSize(QSize(12, 12));
    closeButton->setObjectName("tooltipClose");

    m_header->addWidget(m_title);
    m_header->addStretch();

    m_header->addWidget(closeButton);

    auto *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setObjectName("tooltipSeparator");

    m_description = new QLabel(this);
    m_description->setWordWrap(true);

    m_link = new QLabel(this);
    m_link->setTextFormat(Qt::RichText);
    m_link->setOpenExternalLinks(true);
    m_link->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    layout->addLayout(m_header);
    layout->addWidget(separator);
    layout->addWidget(m_description);
    layout->addWidget(m_link);

    connect(closeButton, &QToolButton::clicked, this, [this]() { hide(); });
}

void MapToolTip::setTitle(const QString &title)
{
    m_title->setText(title);
    adjustSize();
}

void MapToolTip::setDescription(const QString &html)
{
    m_description->setText(html);
    adjustSize();
}

void MapToolTip::setLink(const QString &link)
{
    m_link->setText(link);
    adjustSize();
}

void MapToolTip::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton
        && m_header->geometry().contains(event->position().toPoint())) {

        m_dragOffset = event->position().toPoint();
        m_dragging = true;

        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void MapToolTip::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && parentWidget()) {
        const QPoint cursorPos = parentWidget()->mapFromGlobal(event->globalPosition().toPoint());

        move(cursorPos - m_dragOffset);

        event->accept();
        return;
    }

    QWidget::mouseMoveEvent(event);
}

void MapToolTip::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}