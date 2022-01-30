#ifndef QCHECKLIST
#define QCHECKLIST

#include <QWidget>
#include <QComboBox>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QEvent>
#include <QStyledItemDelegate>
#include <QListView>

/**
 * @brief QComboBox with support of checkboxes
 * http://stackoverflow.com/questions/8422760/combobox-of-checkboxes
 */

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT QCheckList : public QComboBox
{
    Q_OBJECT

public:
    /**
     * @brief Additional value to Qt::CheckState when some checkboxes are Qt::PartiallyChecked
     */
    static const int StateUnknown = 3;

private:
    QStandardItemModel* m_model;
    /**
     * @brief Text displayed when no item is checked
     */
    QString m_noneCheckedText;
    /**
     * @brief Text displayed when all items are checked
     */
    QString m_allCheckedText;
    /**
     * @brief Text displayed when some items are partially checked
     */
    QString m_unknownlyCheckedText;

signals:
    void globalCheckStateChanged(int);

public:
    QCheckList(QWidget* _parent = 0) : QComboBox(_parent)
    {
        m_model = new QStandardItemModel();
        setModel(m_model);

        setEditable(true);
        lineEdit()->setReadOnly(true);
        lineEdit()->installEventFilter(this);
        setItemDelegate(new QCheckListStyledItemDelegate(this));

        connect(lineEdit(), &QLineEdit::selectionChanged, lineEdit(), &QLineEdit::deselect);
        connect((QListView*) view(), SIGNAL(pressed(QModelIndex)), this, SLOT(on_itemPressed(QModelIndex)));
        connect(m_model, SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)), this, SLOT(on_modelDataChanged()));
    }

    ~QCheckList()
    {
        delete m_model;
    }

    void setAllCheckedText(const QString &text)
    {
        m_allCheckedText = text;
        updateText();
    }

    void setNoneCheckedText(const QString &text)
    {
        m_noneCheckedText = text;
        updateText();
    }

    void setUnknownlyCheckedText(const QString &text)
    {
        m_unknownlyCheckedText = text;
        updateText();
    }

    /**
     * @brief Adds a item to the checklist (setChecklist must have been called)
     * @return the new QStandardItem
     */
    QStandardItem* addCheckItem(const QString &label, const QVariant &data, const Qt::CheckState checkState)
    {
        QStandardItem* item = new QStandardItem(label);
        item->setCheckState(checkState);
        item->setData(data);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);

        m_model->appendRow(item);

        updateText();

        return item;
    }

    /**
     * @brief Computes the global state of the checklist :
     *      - if there is no item: StateUnknown
     *      - if there is at least one item partially checked: StateUnknown
     *      - if all items are checked: Qt::Checked
     *      - if no item is checked: Qt::Unchecked
     *      - else: Qt::PartiallyChecked
     */
    int globalCheckState()
    {
        int nbRows = m_model->rowCount(), nbChecked = 0, nbUnchecked = 0;

        if (nbRows == 0)
        {
            return StateUnknown;
        }

        for (int i = 0; i < nbRows; i++)
        {
            if (m_model->item(i)->checkState() == Qt::Checked)
            {
                nbChecked++;
            }
            else if (m_model->item(i)->checkState() == Qt::Unchecked)
            {
                nbUnchecked++;
            }
            else
            {
                return StateUnknown;
            }
        }

        return nbChecked == nbRows ? Qt::Checked : nbUnchecked == nbRows ? Qt::Unchecked : Qt::PartiallyChecked;
    }

protected:
    /*bool eventFilter(QObject* _object, QEvent* _event)
    {
        if (_object == lineEdit() && _event->type() == QEvent::MouseButtonPress)
        {
            showPopup();
            return true;
        }

        return false;
    }*/

private:
    void updateText()
    {
        QString text;

        switch (globalCheckState())
        {
        case Qt::Checked:
            text = m_allCheckedText;
            break;

        case Qt::Unchecked:
            text = m_noneCheckedText;
            break;

        case Qt::PartiallyChecked:
            for (int i = 0; i < m_model->rowCount(); i++)
            {
                if (m_model->item(i)->checkState() == Qt::Checked)
                {
                    if (!text.isEmpty())
                    {
                        text+= ", ";
                    }

                    text+= m_model->item(i)->text();
                }
            }
            break;

        default:
            text = m_unknownlyCheckedText;
        }

        lineEdit()->setText(text);
    }

private slots:
    void on_modelDataChanged()
    {
        updateText();
        emit globalCheckStateChanged(globalCheckState());
    }

    void on_itemPressed(const QModelIndex &index)
    {
        QStandardItem* item = m_model->itemFromIndex(index);

        if (item->checkState() == Qt::Checked)
        {
            item->setCheckState(Qt::Unchecked);
        }
        else
        {
            item->setCheckState(Qt::Checked);
        }
    }

public:
    class QCheckListStyledItemDelegate : public QStyledItemDelegate
    {
    public:
        QCheckListStyledItemDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}

        void paint(QPainter * painter_, const QStyleOptionViewItem & option_, const QModelIndex & index_) const
        {
            QStyleOptionViewItem & refToNonConstOption = const_cast<QStyleOptionViewItem &>(option_);
            refToNonConstOption.showDecorationSelected = false;
            QStyledItemDelegate::paint(painter_, refToNonConstOption, index_);
        }
    };
};

#endif // QCHECKLIST

