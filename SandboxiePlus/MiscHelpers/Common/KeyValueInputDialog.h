#ifndef KEYVALUEINPUTDIALOG_H
#define KEYVALUEINPUTDIALOG_H

#include <QDialogButtonBox>
#include <QMessageBox>

class CKeyValueInputDialogPrivate;

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CKeyValueInputDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QPixmap iconPixmap READ iconPixmap WRITE setIconPixmap)
    Q_PROPERTY(QDialogButtonBox::StandardButtons buttons READ standardButtons WRITE setStandardButtons)
    Q_PROPERTY(QDialogButtonBox::StandardButton defaultButton READ defaultButton WRITE setDefaultButton)

public:
    explicit CKeyValueInputDialog(QWidget *parent = NULL);
    virtual ~CKeyValueInputDialog();

    QString text() const;
    void setText(const QString &);

	QString keyLabel() const;
    void setKeyLabel(const QString &);

	QString key() const;
    void setKey(const QString &);

	bool keyReadOnly() const;
	void setKeyReadOnly(bool);

	QString valueLabel() const;
    void setValueLabel(const QString &);

	QString value() const;
    void setValue(const QString &);


    QDialogButtonBox::StandardButtons standardButtons() const;
    void setStandardButtons(QDialogButtonBox::StandardButtons s);
    QPushButton *button(QDialogButtonBox::StandardButton b) const;
    QPushButton *addButton(const QString &text, QDialogButtonBox::ButtonRole role);

    QDialogButtonBox::StandardButton defaultButton() const;
    void setDefaultButton(QDialogButtonBox::StandardButton s);

    // See static QMessageBox::standardPixmap()
    QPixmap iconPixmap() const;
    void setIconPixmap (const QPixmap &p);

    // Query the result
    QAbstractButton *clickedButton() const;
    QDialogButtonBox::StandardButton clickedStandardButton() const;

private slots:
    void slotClicked(QAbstractButton *b);

private:
    CKeyValueInputDialogPrivate *d;
};



#endif // HCOMBODIALOG_H
