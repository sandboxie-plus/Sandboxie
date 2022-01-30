#ifndef CHECKABLEMESSAGEBOX_H
#define CHECKABLEMESSAGEBOX_H

#include <QDialogButtonBox>
#include <QMessageBox>

class CCheckableMessageBoxPrivate;

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CCheckableMessageBox : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
	Q_PROPERTY(Qt::TextFormat textFormat READ textFormat WRITE setTextFormat)
    Q_PROPERTY(QPixmap iconPixmap READ iconPixmap WRITE setIconPixmap)
    Q_PROPERTY(bool isChecked READ isChecked WRITE setChecked)
    Q_PROPERTY(QString checkBoxText READ checkBoxText WRITE setCheckBoxText)
    Q_PROPERTY(QDialogButtonBox::StandardButtons buttons READ standardButtons WRITE setStandardButtons)
    Q_PROPERTY(QDialogButtonBox::StandardButton defaultButton READ defaultButton WRITE setDefaultButton)

public:
    explicit CCheckableMessageBox(QWidget *parent = NULL);
    virtual ~CCheckableMessageBox();

    static QDialogButtonBox::StandardButton
        question(QWidget *parent,
                 const QString &title,
                 const QString &question,
                 const QString &checkBoxText,
                 bool *checkBoxSetting,
                 QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Yes|QDialogButtonBox::No,
                 QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::No,
				 QMessageBox::Icon icon = QMessageBox::Question);

    QString text() const;
    void setText(const QString &);

	Qt::TextFormat textFormat() const;
	void setTextFormat(Qt::TextFormat format);

    bool isChecked() const;
    void setChecked(bool s);

    QString checkBoxText() const;
    void setCheckBoxText(const QString &);

    bool isCheckBoxVisible() const;
    void setCheckBoxVisible(bool);

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

    // Conversion convenience
    static QMessageBox::StandardButton dialogButtonBoxToMessageBoxButton(QDialogButtonBox::StandardButton);

private slots:
    void slotClicked(QAbstractButton *b);

private:
    CCheckableMessageBoxPrivate *d;
};



#endif // CHECKABLEMESSAGEBOX_H
