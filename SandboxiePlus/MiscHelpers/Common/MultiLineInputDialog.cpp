#include "stdafx.h"
#include "MultiLineInputDialog.h"

#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>


class CMultiLineInputDialogPrivate
{
public:
    CMultiLineInputDialogPrivate(QDialog *q)
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
        messageLabel->setWordWrap(true);
        messageLabel->setOpenExternalLinks(true);
        messageLabel->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

        QSpacerItem *buttonSpacer =
            new QSpacerItem(0, 1, QSizePolicy::Minimum, QSizePolicy::Minimum);

		edit = new QPlainTextEdit(q);


        buttonBox = new QDialogButtonBox(q);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        QVBoxLayout *verticalLayout = new QVBoxLayout();
        verticalLayout->addWidget(pixmapLabel);
        verticalLayout->addItem(pixmapSpacer);

        QHBoxLayout *horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->addLayout(verticalLayout);
        horizontalLayout_2->addWidget(messageLabel);

        QVBoxLayout *verticalLayout_2 = new QVBoxLayout(q);
        verticalLayout_2->addLayout(horizontalLayout_2);
		verticalLayout_2->addWidget(edit);
        verticalLayout_2->addItem(buttonSpacer);
        verticalLayout_2->addWidget(buttonBox);
    }

    QLabel *pixmapLabel;
    QLabel *messageLabel;
	QPlainTextEdit* edit;
    QDialogButtonBox *buttonBox;
    QAbstractButton *clickedButton;
};

CMultiLineInputDialog::CMultiLineInputDialog(QWidget *parent) :
    QDialog(parent),
    d(new CMultiLineInputDialogPrivate(this))
{
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(d->buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(d->buttonBox, SIGNAL(rejected()), SLOT(reject()));
    connect(d->buttonBox, SIGNAL(clicked(QAbstractButton*)),
        SLOT(slotClicked(QAbstractButton*)));
}

CMultiLineInputDialog::~CMultiLineInputDialog()
{
    delete d;
}

void CMultiLineInputDialog::slotClicked(QAbstractButton *b)
{
    d->clickedButton = b;
}

QAbstractButton *CMultiLineInputDialog::clickedButton() const
{
    return d->clickedButton;
}

QDialogButtonBox::StandardButton CMultiLineInputDialog::clickedStandardButton() const
{
    if (d->clickedButton)
        return d->buttonBox->standardButton(d->clickedButton);
    return QDialogButtonBox::NoButton;
}

QString CMultiLineInputDialog::text() const
{
    return d->messageLabel->text();
}

void CMultiLineInputDialog::setText(const QString &t)
{
    d->messageLabel->setText(t);
}

QString CMultiLineInputDialog::value() const
{
	return d->edit->toPlainText();
}

void CMultiLineInputDialog::setValue(const QString &t)
{
	d->edit->setPlainText(t);
}

QPixmap CMultiLineInputDialog::iconPixmap() const
{
    if (const QPixmap *p = d->pixmapLabel->pixmap())
        return QPixmap(*p);
    return QPixmap();
}

void CMultiLineInputDialog::setIconPixmap(const QPixmap &p)
{
    d->pixmapLabel->setPixmap(p);
    d->pixmapLabel->setVisible(!p.isNull());
}

QDialogButtonBox::StandardButtons CMultiLineInputDialog::standardButtons() const
{
    return d->buttonBox->standardButtons();
}

void CMultiLineInputDialog::setStandardButtons(QDialogButtonBox::StandardButtons s)
{
    d->buttonBox->setStandardButtons(s);
}

QPushButton *CMultiLineInputDialog::button(QDialogButtonBox::StandardButton b) const
{
    return d->buttonBox->button(b);
}

QPushButton *CMultiLineInputDialog::addButton(const QString &text, QDialogButtonBox::ButtonRole role)
{
    return d->buttonBox->addButton(text, role);
}

QDialogButtonBox::StandardButton CMultiLineInputDialog::defaultButton() const
{
    foreach (QAbstractButton *b, d->buttonBox->buttons())
        if (QPushButton *pb = qobject_cast<QPushButton *>(b))
            if (pb->isDefault())
               return d->buttonBox->standardButton(pb);
    return QDialogButtonBox::NoButton;
}

void CMultiLineInputDialog::setDefaultButton(QDialogButtonBox::StandardButton s)
{
	if (QPushButton *b = d->buttonBox->button(s)) {
		b->setDefault(true);
		b->setFocus();
	}
}
