#include "autowidthcombobox.h"

#include <QFontMetrics>
#include <QStyle>
#include <QStyleOptionComboBox>

AutoWidthComboBox::AutoWidthComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMinimumWidth(150);

    connect(this, &QComboBox::currentTextChanged,
            this, [this]() {
                updateGeometry();
            });
}

QSize AutoWidthComboBox::sizeHint() const
{
    QStyleOptionComboBox option;

    initStyleOption(&option);

    const QFontMetrics fm(font());

    QSize contentSize(fm.horizontalAdvance(currentText()),
                      fm.height());

    QSize size = style()->sizeFromContents(
            QStyle::CT_ComboBox,
            &option,
            contentSize,
            this
            );

    size.setWidth(qMax(size.width(), minimumWidth()));

    return size;
}

QSize AutoWidthComboBox::minimumSizeHint() const
{
    return sizeHint();
}