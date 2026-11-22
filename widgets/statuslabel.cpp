#include "statuslabel.h"

#include <QStyle>

StatusLabel::StatusLabel(QWidget *parent)
    : QLabel(parent)
{
    m_timer.setSingleShot(true);

    connect(&m_timer, &QTimer::timeout,
            this, &StatusLabel::resetStatus);
}

void StatusLabel::showStatus(const QString &message,
                             const QString &status,
                             int duration)
{
    if (!m_timer.isActive())
        m_defaultText = text();

    setProperty("status", status);
    setText(message);

    style()->unpolish(this);
    style()->polish(this);
    update();

    m_timer.start(duration);
}

void StatusLabel::resetStatus()
{
    setProperty("status", "");
    setText(m_defaultText);

    style()->unpolish(this);
    style()->polish(this);
    update();
}