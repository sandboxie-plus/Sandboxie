#ifndef CCheckableComboBox_H
#define CCheckableComboBox_H

#include <QComboBox>
#include <QStandardItemModel>
#include <QListView>
#include <QMouseEvent>
#include <QLineEdit>
#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CCheckableComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit CCheckableComboBox(QWidget* parent = nullptr);

    //void addItem(const QString& text, const QVariant& userData = QVariant());
    void addItems(const QStringList& texts);
    QStringList checkedItems() const;
    void setItemCheckState(int index, Qt::CheckState state);
    Qt::CheckState itemCheckState(int index) const;
    void clearItems();

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

private slots:
    void updateText();

public:
    QString m_SelectItems;
};

#endif // CCheckableComboBox_H