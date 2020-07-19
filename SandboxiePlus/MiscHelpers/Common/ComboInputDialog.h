#ifndef HCOMBOINPUTDIALOG_H
#define HCOMBOINPUTDIALOG_H

#include <QDialogButtonBox>
#include <QMessageBox>

class CComboInputDialogPrivate;

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CComboInputDialog: public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QPixmap iconPixmap READ iconPixmap WRITE setIconPixmap)
    Q_PROPERTY(QDialogButtonBox::StandardButtons buttons READ standardButtons WRITE setStandardButtons)
    Q_PROPERTY(QDialogButtonBox::StandardButton defaultButton READ defaultButton WRITE setDefaultButton)

public:
    explicit CComboInputDialog(QWidget *parent = NULL);
    virtual ~CComboInputDialog();

    QString text() const;
    void setText(const QString &);

	void addItem(const QString&, const QVariant & = QVariant());
	void setEditable(bool);

	QString value() const;
    void setValue(const QString &);
	int findValue(const QString &) const;

	QVariant data() const;
    void setData(const QVariant &);
	int findData(const QVariant &) const;

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
    CComboInputDialogPrivate *d;
};



#endif // HCOMBOINPUTDIALOG_H
