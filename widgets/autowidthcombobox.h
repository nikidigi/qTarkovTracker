#ifndef AUTOWIDTHCOMBOBOX_H
#define AUTOWIDTHCOMBOBOX_H

#include <QComboBox>

class AutoWidthComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit AutoWidthComboBox(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
};


#endif // AUTOWIDTHCOMBOBOX_H
