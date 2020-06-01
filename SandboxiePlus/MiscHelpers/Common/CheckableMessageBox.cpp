#include "stdafx.h"
#include "CheckableMessageBox.h"

#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

/*!
    \class Utils::CCheckableMessageBox

     \brief A messagebox suitable for questions with a
     "Do not ask me again" checkbox.

    Emulates the QMessageBox API with
    static conveniences. The message label can open external URLs.
*/


class CCheckableMessageBoxPrivate
{
public:
    CCheckableMessageBoxPrivate(QDialog *q)
        : clickedButton(0)
    {
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

        pixmapLabel = new QLabel(q);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pixmapLabel->sizePolicy().hasHeightForWidth());
        pixmapLabel->setSizePolicy(sizePolicy);
        pixmapLabel->setVisible(false);

        QSpacerItem *pixmapSpacer =
            new QSpacerItem(0, 5, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

        messageLabel = new QLabel(q);
        messageLabel->setMinimumSize(QSize(300, 0));
		messageLabel->setSizePolicy(QSizePolicy::Expanding, messageLabel->sizePolicy().verticalPolicy());
        messageLabel->setWordWrap(true);
        messageLabel->setOpenExternalLinks(true);
        messageLabel->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

        QSpacerItem *checkBoxRightSpacer =
            new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);
        QSpacerItem *buttonSpacer =
            new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::Minimum);

        checkBox = new QCheckBox(q);
        checkBox->setText(CCheckableMessageBox::tr("Do not ask again"));

        buttonBox = new QDialogButtonBox(q);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        QVBoxLayout *verticalLayout = new QVBoxLayout();
        verticalLayout->addWidget(pixmapLabel);
        verticalLayout->addItem(pixmapSpacer);

        QHBoxLayout *horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->addLayout(verticalLayout);
        horizontalLayout_2->addWidget(messageLabel);

        QHBoxLayout *horizontalLayout = new QHBoxLayout();
        horizontalLayout->addWidget(checkBox);
        horizontalLayout->addItem(checkBoxRightSpacer);

        QVBoxLayout *verticalLayout_2 = new QVBoxLayout(q);
        verticalLayout_2->addLayout(horizontalLayout_2);
        verticalLayout_2->addLayout(horizontalLayout);
        verticalLayout_2->addItem(buttonSpacer);
        verticalLayout_2->addWidget(buttonBox);
    }

    QLabel *pixmapLabel;
    QLabel *messageLabel;
    QCheckBox *checkBox;
    QDialogButtonBox *buttonBox;
    QAbstractButton *clickedButton;
};

CCheckableMessageBox::CCheckableMessageBox(QWidget *parent) :
    QDialog(parent),
    d(new CCheckableMessageBoxPrivate(this))
{
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(d->buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(d->buttonBox, SIGNAL(rejected()), SLOT(reject()));
    connect(d->buttonBox, SIGNAL(clicked(QAbstractButton*)),
        SLOT(slotClicked(QAbstractButton*)));
}

CCheckableMessageBox::~CCheckableMessageBox()
{
    delete d;
}

void CCheckableMessageBox::slotClicked(QAbstractButton *b)
{
    d->clickedButton = b;
}

QAbstractButton *CCheckableMessageBox::clickedButton() const
{
    return d->clickedButton;
}

QDialogButtonBox::StandardButton CCheckableMessageBox::clickedStandardButton() const
{
    if (d->clickedButton)
        return d->buttonBox->standardButton(d->clickedButton);
    return QDialogButtonBox::NoButton;
}

QString CCheckableMessageBox::text() const
{
    return d->messageLabel->text();
}

void CCheckableMessageBox::setText(const QString &t)
{
    d->messageLabel->setText(t);
}

QPixmap CCheckableMessageBox::iconPixmap() const
{
    if (const QPixmap *p = d->pixmapLabel->pixmap())
        return QPixmap(*p);
    return QPixmap();
}

void CCheckableMessageBox::setIconPixmap(const QPixmap &p)
{
    d->pixmapLabel->setPixmap(p);
    d->pixmapLabel->setVisible(!p.isNull());
}

bool CCheckableMessageBox::isChecked() const
{
    return d->checkBox->isChecked();
}

void CCheckableMessageBox::setChecked(bool s)
{
    d->checkBox->setChecked(s);
}

QString CCheckableMessageBox::checkBoxText() const
{
    return d->checkBox->text();
}

void CCheckableMessageBox::setCheckBoxText(const QString &t)
{
    d->checkBox->setText(t);
}

bool CCheckableMessageBox::isCheckBoxVisible() const
{
    return d->checkBox->isVisible();
}

void CCheckableMessageBox::setCheckBoxVisible(bool v)
{
    d->checkBox->setVisible(v);
}

QDialogButtonBox::StandardButtons CCheckableMessageBox::standardButtons() const
{
    return d->buttonBox->standardButtons();
}

void CCheckableMessageBox::setStandardButtons(QDialogButtonBox::StandardButtons s)
{
    d->buttonBox->setStandardButtons(s);
}

QPushButton *CCheckableMessageBox::button(QDialogButtonBox::StandardButton b) const
{
    return d->buttonBox->button(b);
}

QPushButton *CCheckableMessageBox::addButton(const QString &text, QDialogButtonBox::ButtonRole role)
{
    return d->buttonBox->addButton(text, role);
}

QDialogButtonBox::StandardButton CCheckableMessageBox::defaultButton() const
{
    foreach (QAbstractButton *b, d->buttonBox->buttons())
        if (QPushButton *pb = qobject_cast<QPushButton *>(b))
            if (pb->isDefault())
               return d->buttonBox->standardButton(pb);
    return QDialogButtonBox::NoButton;
}

void CCheckableMessageBox::setDefaultButton(QDialogButtonBox::StandardButton s)
{
    if (QPushButton *b = d->buttonBox->button(s)) {
        b->setDefault(true);
        b->setFocus();
    }
}

QDialogButtonBox::StandardButton
CCheckableMessageBox::question(QWidget *parent,
                              const QString &title,
                              const QString &question,
                              const QString &checkBoxText,
                              bool *checkBoxSetting,
                              QDialogButtonBox::StandardButtons buttons,
                              QDialogButtonBox::StandardButton defaultButton, 
							  QMessageBox::Icon icon)
{
    CCheckableMessageBox mb(parent);
    mb.setWindowTitle(title);
    mb.setIconPixmap(QMessageBox::standardIcon(icon));
    mb.setText(question);
    mb.setCheckBoxText(checkBoxText);
    mb.setChecked(*checkBoxSetting);
    mb.setStandardButtons(buttons);
    mb.setDefaultButton(defaultButton);
    mb.exec();
    *checkBoxSetting = mb.isChecked();
    return mb.clickedStandardButton();
}

QMessageBox::StandardButton CCheckableMessageBox::dialogButtonBoxToMessageBoxButton(QDialogButtonBox::StandardButton db)
{
    return static_cast<QMessageBox::StandardButton>(int(db));
}

