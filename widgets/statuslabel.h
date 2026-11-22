#ifndef STATUSLABEL_H
#define STATUSLABEL_H

#include <QLabel>
#include <QTimer>

class StatusLabel : public QLabel
{
    Q_OBJECT

public:
    explicit StatusLabel(QWidget *parent = nullptr);

    void showStatus(const QString &message,
                    const QString &status,
                    int duration = 3000);

private:
    QTimer m_timer;
    QString m_defaultText;

    void resetStatus();
};

#endif // STATUSLABEL_H
