#include "stdafx.h"

#include "AccessControlWidget.h"

#include <QTreeWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QVBoxLayout>


CAccessControlWidget::CAccessControlWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->setContentsMargins(0, 0, 0, 0);

    m_pEntryList = new QTreeWidget(this);
    m_pEntryList->setHeaderLabels({ tr("Path"), tr("Type") });
    m_pEntryList->setRootIsDecorated(false);
    m_pEntryList->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));
    pLayout->addWidget(m_pEntryList);

    QHBoxLayout* pAddRow = new QHBoxLayout();
    pAddRow->setContentsMargins(0, 4, 0, 0);

    m_pPathEdit = new QLineEdit(this);
    m_pPathEdit->setPlaceholderText(tr("Enter file or folder path"));
    pAddRow->addWidget(m_pPathEdit, 1);

    m_pTypeCombo = new QComboBox(this);
    pAddRow->addWidget(m_pTypeCombo);

    QPushButton* pBrowseFileBtn = new QPushButton(tr("Browse"), this);
    connect(pBrowseFileBtn, &QPushButton::clicked, this, &CAccessControlWidget::OnBrowseFile);
    pAddRow->addWidget(pBrowseFileBtn);

    QPushButton* pBrowseDirBtn = new QPushButton(tr("Folder"), this);
    connect(pBrowseDirBtn, &QPushButton::clicked, this, &CAccessControlWidget::OnBrowseFolder);
    pAddRow->addWidget(pBrowseDirBtn);

    QPushButton* pAddBtn = new QPushButton(tr("Add"), this);
    connect(pAddBtn, &QPushButton::clicked, this, &CAccessControlWidget::OnAddEntry);
    pAddRow->addWidget(pAddBtn);

    QPushButton* pRemoveBtn = new QPushButton(tr("Remove"), this);
    connect(pRemoveBtn, &QPushButton::clicked, this, &CAccessControlWidget::OnRemoveEntry);
    pAddRow->addWidget(pRemoveBtn);

    pLayout->addLayout(pAddRow);

    setLayout(pLayout);
}

void CAccessControlWidget::SetTypes(const QList<QPair<QString, QString>>& Types)
{
    m_Types = Types;
    m_pTypeCombo->clear();
    for (const auto& Type : m_Types)
        m_pTypeCombo->addItem(Type.first, Type.second);
}

void CAccessControlWidget::SetEntries(const QList<QPair<QString, QString>>& Entries)
{
    m_pEntryList->clear();
    for (const auto& Entry : Entries)
        AddEntryWidget(Entry.first, Entry.second);
}

void CAccessControlWidget::AddEntryWidget(const QString& Path, const QString& TypeKey)
{
    QTreeWidgetItem* pItem = new QTreeWidgetItem();
    pItem->setText(0, Path);
    pItem->setData(0, Qt::UserRole, Path);
    m_pEntryList->addTopLevelItem(pItem);

    QComboBox* pType = new QComboBox(this);
    int TypeIndex = 0;
    for (int i = 0; i < m_Types.count(); i++) {
        pType->addItem(m_Types[i].first, m_Types[i].second);
        if (m_Types[i].second.compare(TypeKey, Qt::CaseInsensitive) == 0)
            TypeIndex = i;
    }
    pType->setCurrentIndex(TypeIndex);
    connect(pType, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() { emit changed(); });
    m_pEntryList->setItemWidget(pItem, 1, pType);
}

QList<QPair<QString, QString>> CAccessControlWidget::GetEntries() const
{
    QList<QPair<QString, QString>> Entries;
    for (int i = 0; i < m_pEntryList->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* pItem = m_pEntryList->topLevelItem(i);
        QString Path = pItem->text(0);
        QString TypeKey = QString();
        if (QComboBox* pType = (QComboBox*)m_pEntryList->itemWidget(pItem, 1))
            TypeKey = pType->currentData().toString();
        if (!Path.isEmpty() && !TypeKey.isEmpty())
            Entries.append(qMakePair(Path, TypeKey));
    }
    return Entries;
}

void CAccessControlWidget::OnBrowseFile()
{
    QString FilePath = QFileDialog::getOpenFileName(this, tr("Select File")).replace("/", "\\");
    if (!FilePath.isEmpty())
        m_pPathEdit->setText(FilePath);
}

void CAccessControlWidget::OnBrowseFolder()
{
    QString DirPath = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
    if (!DirPath.isEmpty()) {
        if (!DirPath.endsWith("\\"))
            DirPath.append("\\");
        m_pPathEdit->setText(DirPath);
    }
}

void CAccessControlWidget::OnAddEntry()
{
    QString Path = m_pPathEdit->text().trimmed();
    if (Path.isEmpty())
        return;
    Path.replace("/", "\\");
    QString TypeKey = m_pTypeCombo->currentData().toString();
    AddEntryWidget(Path, TypeKey);
    m_pPathEdit->clear();
    emit changed();
}

void CAccessControlWidget::OnRemoveEntry()
{
    QTreeWidgetItem* pItem = m_pEntryList->currentItem();
    if (!pItem)
        return;
    delete m_pEntryList->takeTopLevelItem(m_pEntryList->indexOfTopLevelItem(pItem));
    emit changed();
}