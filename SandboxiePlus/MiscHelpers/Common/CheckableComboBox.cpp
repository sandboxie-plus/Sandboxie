#include "stdafx.h"
#include "CheckableComboBox.h"
#include <QStandardItem>
#include <QApplication>
#include <QStyleOption>
#include <QDebug>


CCheckableComboBox::CCheckableComboBox(QWidget* parent)
    : QComboBox(parent)
{
    m_SelectItems = tr("Select Items...");

    // Use QListView as the view
    QListView* listView = new QListView(this);
    setView(listView);

    // Use QStandardItemModel as the model
    QStandardItemModel* model = new QStandardItemModel(this);
    setModel(model);

    // Install event filter on the view's viewport
    view()->viewport()->installEventFilter(this);

    // Make the combo box editable to display checked items
    setEditable(true);
    lineEdit()->setReadOnly(true);  // Prevent user from editing text

    // Update text when an item's check state changes
    connect(model, &QStandardItemModel::itemChanged, this, &CCheckableComboBox::updateText);
}

/*void CCheckableComboBox::addItem(const QString& text, const QVariant& userData)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model());
    QStandardItem* item = new QStandardItem(text);
    item->setData(userData, Qt::UserRole);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    item->setCheckable(true);
    item->setCheckState(Qt::Unchecked);
    //item->setTristate(true);  // Enable tri-state
    model->appendRow(item);
}*/

void CCheckableComboBox::addItems(const QStringList& texts)
{
    for (const QString& text : texts)
    {
        addItem(text);
    }
}

void CCheckableComboBox::clearItems()
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model());
    model->clear();
    updateText();
}

QStringList CCheckableComboBox::checkedItems() const
{
    QStringList checked;
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model());
    for (int i = 0; i < model->rowCount(); ++i)
    {
        QStandardItem* item = model->item(i);
        if (item->checkState() == Qt::Checked)
        {
            checked << item->text();
        }
    }
    return checked;
}

void CCheckableComboBox::setItemCheckState(int index, Qt::CheckState state)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model());
    QStandardItem* item = model->item(index);
    if (item)
    {
        item->setCheckState(state);
    }
}

Qt::CheckState CCheckableComboBox::itemCheckState(int index) const
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model());
    QStandardItem* item = model->item(index);
    if (item)
    {
        return item->checkState();
    }
    return Qt::Unchecked;
}

void CCheckableComboBox::updateText()
{
    QStringList checked = checkedItems();
    if (checked.isEmpty())
    {
        setEditText(m_SelectItems);
    }
    else
    {
        setEditText(checked.join(", "));
    }
}

bool CCheckableComboBox::eventFilter(QObject* object, QEvent* event)
{
    if (object == view()->viewport())
    {
        if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QModelIndex index = view()->indexAt(mouseEvent->pos());
            if (index.isValid())
            {
                QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model());
                QStandardItem* item = model->itemFromIndex(index);
                if (item && item->isCheckable())
                {
                    // Cycle through tri-state check states
                    if (item->checkState() == Qt::Unchecked)
                    {
                        item->setCheckState(Qt::Checked);
                    }
                    else if (item->checkState() == Qt::Checked)
                    {
                        item->setCheckState(Qt::PartiallyChecked);
                    }
                    else
                    {
                        item->setCheckState(Qt::Unchecked);
                    }

                    // emit change event
                    emit model->itemChanged(item);

                    // Do not close the combo box
                    return true;
                }
            }
        }
    }
    return QComboBox::eventFilter(object, event);
}