#include "stdafx.h"
#include "KeyValueInputDialog.h"

#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>


class CKeyValueInputDialogPrivate
{
public:
    CKeyValueInputDialogPrivate(QDialog *q)
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

        keyLabel = new QLabel(q);
		keyLabel->setText("Key:");

		keyEdit = new QLineEdit(q);

		valueLabel = new QLabel(q);
		valueLabel->setText("Value:");

		valueEdit = new QTextEdit(q);


        buttonBox = new QDialogButtonBox(q);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        QVBoxLayout *verticalLayout = new QVBoxLayout();
        verticalLayout->addWidget(pixmapLabel);
        verticalLayout->addItem(pixmapSpacer);

        QHBoxLayout *horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->addLayout(verticalLayout);
        horizontalLayout_2->addWidget(messageLabel);

        QFormLayout *horizontalLayout = new QFormLayout();
		horizontalLayout->addRow(keyLabel, keyEdit);
		horizontalLayout->addRow(valueLabel, valueEdit);

        QVBoxLayout *verticalLayout_2 = new QVBoxLayout(q);
        verticalLayout_2->addLayout(horizontalLayout_2);
        verticalLayout_2->addLayout(horizontalLayout);
        verticalLayout_2->addItem(buttonSpacer);
        verticalLayout_2->addWidget(buttonBox);
    }

    QLabel *pixmapLabel;
    QLabel *messageLabel;
	QLabel *keyLabel;
	QLineEdit* keyEdit;
	QLabel *valueLabel;
	QTextEdit* valueEdit;
    QDialogButtonBox *buttonBox;
    QAbstractButton *clickedButton;
};

CKeyValueInputDialog::CKeyValueInputDialog(QWidget *parent) :
    QDialog(parent),
    d(new CKeyValueInputDialogPrivate(this))
{
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(d->buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(d->buttonBox, SIGNAL(rejected()), SLOT(reject()));
    connect(d->buttonBox, SIGNAL(clicked(QAbstractButton*)),
        SLOT(slotClicked(QAbstractButton*)));
}

CKeyValueInputDialog::~CKeyValueInputDialog()
{
    delete d;
}

void CKeyValueInputDialog::slotClicked(QAbstractButton *b)
{
    d->clickedButton = b;
}

QAbstractButton *CKeyValueInputDialog::clickedButton() const
{
    return d->clickedButton;
}

QDialogButtonBox::StandardButton CKeyValueInputDialog::clickedStandardButton() const
{
    if (d->clickedButton)
        return d->buttonBox->standardButton(d->clickedButton);
    return QDialogButtonBox::NoButton;
}

QString CKeyValueInputDialog::text() const
{
    return d->messageLabel->text();
}

void CKeyValueInputDialog::setText(const QString &t)
{
    d->messageLabel->setText(t);
}

QString CKeyValueInputDialog::keyLabel() const
{
	return d->keyLabel->text();
}

void CKeyValueInputDialog::setKeyLabel(const QString &t)
{
	return d->keyLabel->setText(t);
}

QString CKeyValueInputDialog::key() const
{
	return d->keyEdit->text();
}

void CKeyValueInputDialog::setKey(const QString &t)
{
	return d->keyEdit->setText(t);
}

bool CKeyValueInputDialog::keyReadOnly() const
{
	return d->keyEdit->isReadOnly();
}

void CKeyValueInputDialog::setKeyReadOnly(bool r)
{
	return d->keyEdit->setReadOnly(r);
}

QString CKeyValueInputDialog::valueLabel() const
{
	return d->valueLabel->text();
}

void CKeyValueInputDialog::setValueLabel(const QString &t)
{
	return d->valueLabel->setText(t);
}

QString CKeyValueInputDialog::value() const
{
	return d->valueEdit->toPlainText();
}

void CKeyValueInputDialog::setValue(const QString &t)
{
	return d->valueEdit->setText(t);
}

QPixmap CKeyValueInputDialog::iconPixmap() const
{
    if (const QPixmap *p = d->pixmapLabel->pixmap())
        return QPixmap(*p);
    return QPixmap();
}

void CKeyValueInputDialog::setIconPixmap(const QPixmap &p)
{
    d->pixmapLabel->setPixmap(p);
    d->pixmapLabel->setVisible(!p.isNull());
}

QDialogButtonBox::StandardButtons CKeyValueInputDialog::standardButtons() const
{
    return d->buttonBox->standardButtons();
}

void CKeyValueInputDialog::setStandardButtons(QDialogButtonBox::StandardButtons s)
{
    d->buttonBox->setStandardButtons(s);
}

QPushButton *CKeyValueInputDialog::button(QDialogButtonBox::StandardButton b) const
{
    return d->buttonBox->button(b);
}

QPushButton *CKeyValueInputDialog::addButton(const QString &text, QDialogButtonBox::ButtonRole role)
{
    return d->buttonBox->addButton(text, role);
}

QDialogButtonBox::StandardButton CKeyValueInputDialog::defaultButton() const
{
    foreach (QAbstractButton *b, d->buttonBox->buttons())
        if (QPushButton *pb = qobject_cast<QPushButton *>(b))
            if (pb->isDefault())
               return d->buttonBox->standardButton(pb);
    return QDialogButtonBox::NoButton;
}

void CKeyValueInputDialog::setDefaultButton(QDialogButtonBox::StandardButton s)
{
	if (QPushButton *b = d->buttonBox->button(s)) {
		b->setDefault(true);
		b->setFocus();
	}
}
