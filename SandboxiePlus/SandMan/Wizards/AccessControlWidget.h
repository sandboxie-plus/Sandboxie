#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTreeWidget;
class QLineEdit;
class QComboBox;
QT_END_NAMESPACE

//////////////////////////////////////////////////////////////////////////////////////////
// CAccessControlWidget
// 
// A reusable widget for editing path based access control entries. Each entry is a
// pair of a path and an access type (Block, Normal, Write, Read, ...). The widget
// displays the entries in a list widget and provides buttons to browse for a file or
// folder, add an entry and remove the selected one. The type of an entry can be
// changed at any time by editing the type column.
// 
// This widget is intended to be shared between the box options (where the same kind
// of entries are edited) and any other part of the UI, like the New Box Wizard.
//

class CAccessControlWidget : public QWidget
{
    Q_OBJECT

public:
    CAccessControlWidget(QWidget* parent = nullptr);

    // Set the list of available access types, each as (display name, config key).
    void SetTypes(const QList<QPair<QString, QString>>& Types);

    // Set / retrieve the configured entries as (path, config key) pairs.
    void SetEntries(const QList<QPair<QString, QString>>& Entries);
    QList<QPair<QString, QString>> GetEntries() const;

signals:
    void changed();

private slots:
    void OnBrowseFile();
    void OnBrowseFolder();
    void OnAddEntry();
    void OnRemoveEntry();

private:
    void AddEntryWidget(const QString& Path, const QString& TypeKey);

    QTreeWidget*     m_pEntryList;
    QLineEdit*       m_pPathEdit;
    QComboBox*       m_pTypeCombo;

    QList<QPair<QString, QString>> m_Types;
};