#include "stdafx.h"
#include "ComboInputDialog.h"

#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>


class CComboInputDialogPrivate
{
public:
    CComboInputDialogPrivate(QDialog *q)
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

		combo = new QComboBox(q);


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
		verticalLayout_2->addWidget(combo);
        verticalLayout_2->addItem(buttonSpacer);
        verticalLayout_2->addWidget(buttonBox);
    }

    QLabel *pixmapLabel;
    QLabel *messageLabel;
	QComboBox* combo;
    QDialogButtonBox *buttonBox;
    QAbstractButton *clickedButton;
};

CComboInputDialog::CComboInputDialog(QWidget *parent) :
    QDialog(parent),
    d(new CComboInputDialogPrivate(this))
{
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(d->buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(d->buttonBox, SIGNAL(rejected()), SLOT(reject()));
    connect(d->buttonBox, SIGNAL(clicked(QAbstractButton*)),
        SLOT(slotClicked(QAbstractButton*)));
}

CComboInputDialog::~CComboInputDialog()
{
    delete d;
}

void CComboInputDialog::slotClicked(QAbstractButton *b)
{
    d->clickedButton = b;
}

QAbstractButton *CComboInputDialog::clickedButton() const
{
    return d->clickedButton;
}

QDialogButtonBox::StandardButton CComboInputDialog::clickedStandardButton() const
{
    if (d->clickedButton)
        return d->buttonBox->standardButton(d->clickedButton);
    return QDialogButtonBox::NoButton;
}

QString CComboInputDialog::text() const
{
    return d->messageLabel->text();
}

void CComboInputDialog::setText(const QString &t)
{
    d->messageLabel->setText(t);
}

void CComboInputDialog::addItem(const QString& t, const QVariant & v)
{
	d->combo->addItem(t, v);
}

void CComboInputDialog::setEditable(bool b)
{
	d->combo->setEditable(b);
}

QString CComboInputDialog::value() const
{
	return d->combo->currentText();
}

void CComboInputDialog::setValue(const QString &t)
{
	int idx = d->combo->findText(t);
	if(idx == -1)
		d->combo->setEditText(t);
	else
		d->combo->setCurrentIndex(idx);
}

int CComboInputDialog::findValue(const QString &t) const
{
	return d->combo->findText(t);
}

QVariant CComboInputDialog::data() const
{
	return d->combo->currentData();
}

void CComboInputDialog::setData(const QVariant & v)
{
	d->combo->setCurrentIndex(d->combo->findData(v));
}

int CComboInputDialog::findData(const QVariant & v) const
{
	return d->combo->findData(v);
}

QPixmap CComboInputDialog::iconPixmap() const
{
    if (const QPixmap *p = d->pixmapLabel->pixmap())
        return QPixmap(*p);
    return QPixmap();
}

void CComboInputDialog::setIconPixmap(const QPixmap &p)
{
    d->pixmapLabel->setPixmap(p);
    d->pixmapLabel->setVisible(!p.isNull());
}

QDialogButtonBox::StandardButtons CComboInputDialog::standardButtons() const
{
    return d->buttonBox->standardButtons();
}

void CComboInputDialog::setStandardButtons(QDialogButtonBox::StandardButtons s)
{
    d->buttonBox->setStandardButtons(s);
}

QPushButton *CComboInputDialog::button(QDialogButtonBox::StandardButton b) const
{
    return d->buttonBox->button(b);
}

QPushButton *CComboInputDialog::addButton(const QString &text, QDialogButtonBox::ButtonRole role)
{
    return d->buttonBox->addButton(text, role);
}

QDialogButtonBox::StandardButton CComboInputDialog::defaultButton() const
{
    foreach (QAbstractButton *b, d->buttonBox->buttons())
        if (QPushButton *pb = qobject_cast<QPushButton *>(b))
            if (pb->isDefault())
               return d->buttonBox->standardButton(pb);
    return QDialogButtonBox::NoButton;
}

void CComboInputDialog::setDefaultButton(QDialogButtonBox::StandardButton s)
{
	if (QPushButton *b = d->buttonBox->button(s)) {
		b->setDefault(true);
		b->setFocus();
	}
}
