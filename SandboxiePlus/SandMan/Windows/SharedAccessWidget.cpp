#include "stdafx.h"

#include "SharedAccessWidget.h"

#include "../../QSbieAPI/SbieIni.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QToolButton>
#include <QMenu>
#include <QComboBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>


QTreeWidget* CSharedAccessWidget::CreateListWidget()
{
	QTreeWidget* pTree = new QTreeWidget(this);
	pTree->setHeaderLabels(QStringList() << tr("Type") << tr("Program") << tr("Access") << tr("Path"));
	pTree->header()->setStretchLastSection(false);
	pTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	pTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	pTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	pTree->header()->setSectionResizeMode(3, QHeaderView::Stretch);
	pTree->setRootIsDecorated(false);
	pTree->setAlternatingRowColors(false);
	pTree->setSortingEnabled(true);
	return pTree;
}

QToolButton* CSharedAccessWidget::CreateAddButton()
{
	QToolButton* pButton = new QToolButton(this);
	pButton->setText(tr("Add"));
	pButton->setPopupMode(QToolButton::MenuButtonPopup);
	pButton->setMenu(new QMenu(pButton)); // gets filled by FillAddMenu()
	return pButton;
}

CSharedAccessWidget::CSharedAccessWidget(QWidget* parent)
	: QWidget(parent)
	, m_pTree(NULL)
	, m_pShowTemplates(NULL)
	, m_pAddButton(NULL)
	, m_pRemoveButton(NULL)
	, m_bTemplate(false)
{
	QVBoxLayout* pLayout = new QVBoxLayout();
	pLayout->setContentsMargins(0, 0, 0, 0);

	m_pTree = CreateListWidget();
	m_pTree->viewport()->installEventFilter(this);
	pLayout->addWidget(m_pTree);

	QHBoxLayout* pToolbar = new QHBoxLayout();
	pToolbar->setContentsMargins(0, 2, 0, 0);

	m_pShowTemplates = new QCheckBox(tr("Show Templates"), this);
	pToolbar->addWidget(m_pShowTemplates);
	pToolbar->addStretch(1);

	m_pAddButton = CreateAddButton();
	pToolbar->addWidget(m_pAddButton);

	m_pRemoveButton = new QToolButton(this);
	m_pRemoveButton->setText(tr("Remove"));
	pToolbar->addWidget(m_pRemoveButton);

	pLayout->addLayout(pToolbar);

	setLayout(pLayout);

	FillAddMenu();

	connect(m_pTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnItemDoubleClicked(QTreeWidgetItem*, int)));
	connect(m_pTree, SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectionChanged()));
	connect(m_pTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnItemChanged(QTreeWidgetItem*, int)));
	connect(m_pAddButton, SIGNAL(clicked(bool)), this, SLOT(OnAdd()));
	connect(m_pRemoveButton, SIGNAL(clicked(bool)), this, SLOT(OnDel()));
	connect(m_pShowTemplates, SIGNAL(toggled(bool)), this, SLOT(OnChangeTemplates()));
}

CSharedAccessWidget::~CSharedAccessWidget()
{
}

void CSharedAccessWidget::FillAddMenu()
{
}

void CSharedAccessWidget::SetConfig(const QSharedPointer<CSbieIni>& pIni, bool bTemplate)
{
	m_pIni = pIni;
	m_bTemplate = bTemplate;
}

void CSharedAccessWidget::SetTemplatesEnabled(bool bEnabled)
{
	m_pShowTemplates->setEnabled(bEnabled);
}

void CSharedAccessWidget::SetShowTemplates(bool bShow)
{
	m_pShowTemplates->setChecked(bShow);
	OnChangeTemplates();
}

/////////////////////////////////////////////////////////////////////////////

QString CSharedAccessWidget::GetSettingName(eAccessMode::EAccessMode Mode) const
{
	foreach(const SAccessEntryConfig& Config, GetAccessConfig())
		if (Config.Mode == Mode)
			return Config.Setting;
	return "";
}

eAccessMode::EAccessMode CSharedAccessWidget::GetModeFromSetting(const QString& Setting) const
{
	foreach(const SAccessEntryConfig& Config, GetAccessConfig())
		if (Config.Setting.compare(Setting, Qt::CaseInsensitive) == 0)
			return Config.Mode;
	return eAccessMode::eMaxMode;
}

void CSharedAccessWidget::ClearEntries()
{
	m_pTree->clear();
}

void CSharedAccessWidget::AddEntry(eAccessMode::EAccessMode Mode, QString Program, const QString& Path, bool Enabled, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();

	// column 0: type
	pItem->setText(0, GetAccessTypeStr() + (Template.isEmpty() ? "" : " (" + Template + ")"));
	pItem->setData(0, Qt::UserRole, !Template.isEmpty() ? -1 : GetAccessTypeId());

	// column 1: program
	bool bAll = Program.isEmpty();
	bool Not = !bAll && Program.left(1) == "!";
	if (Not)
		Program.remove(0, 1);
	if (Program.left(1) == "<")
		Program = tr("Group: %1").arg(Program.mid(1, Program.length() - 2));
	else if (!bAll)
		m_Programs.insert(Program);
	pItem->setText(1, (Not ? "NOT " : "") + (bAll ? tr("All Programs") : Program));
	pItem->setData(1, Qt::UserRole, (Not ? "!" : "") + (bAll ? "" : Program));

	// column 2: access mode
	QString ModeStr, ModeTip;
	foreach(const SAccessEntryConfig& Config, GetAccessConfig())
		if (Config.Mode == Mode) {
			ModeStr = Config.ModeName;
			ModeTip = Config.ModeTip;
			break;
		}
	pItem->setText(2, ModeStr);
	pItem->setData(2, Qt::UserRole, (int)Mode);
	if (!ModeTip.isEmpty())
		pItem->setToolTip(2, ModeTip);

	// column 3: path
	pItem->setText(3, ExpandPath(Path));
	pItem->setData(3, Qt::UserRole, Path);

	if (Template.isEmpty())
		pItem->setCheckState(0, Enabled ? Qt::Checked : Qt::Unchecked);

	m_pTree->addTopLevelItem(pItem);
}

void CSharedAccessWidget::ParseAndAddEntry(const QString& Setting, const QString& Value, bool disabled, const QString& Template)
{
	eAccessMode::EAccessMode Mode = GetModeFromSetting(Setting);
	if (Mode == eAccessMode::eMaxMode)
		return;

	QStringList Values = Value.split(",");

	if (Values.count() >= 2)
		AddEntry(Mode, Values[0], Values[1], !disabled, Template);
	else if (Values.count() == 1)
		AddEntry(Mode, "", Values[0], !disabled, Template);
}

void CSharedAccessWidget::LoadAccessList()
{
	ClearEntries();

	foreach(const SAccessEntryConfig& Config, GetAccessConfig())
	{
		foreach(const QString & Value, m_pIni->GetTextList(Config.Setting, m_bTemplate))
			ParseAndAddEntry(Config.Setting, Value);

		foreach(const QString & Value, m_pIni->GetTextList(Config.Setting + "Disabled", m_bTemplate))
			ParseAndAddEntry(Config.Setting, Value, true);
	}

	if (m_pShowTemplates->isChecked())
		LoadTemplates(true);
}

void CSharedAccessWidget::LoadTemplates(bool bShow)
{
	if (bShow)
	{
		foreach(const SAccessEntryConfig& Config, GetAccessConfig())
		{
			foreach(const QString & Template, m_pIni->GetTemplates())
			{
				foreach(const QString & Value, m_pIni->GetTextListTmpl(Config.Setting, Template))
					ParseAndAddEntry(Config.Setting, Value, false, Template);
			}
		}
	}
	else
	{
		for (int i = 0; i < m_pTree->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = m_pTree->topLevelItem(i);
			if (pItem->data(0, Qt::UserRole).toInt() == -1)
				delete pItem;
			else
				i++;
		}
	}
}

void CSharedAccessWidget::OnChangeTemplates()
{
	if (m_pShowTemplates->isChecked())
		LoadTemplates(true);
	else
		LoadTemplates(false);
	OnConfigChanged();
}

void CSharedAccessWidget::SaveAccessList()
{
	QStringList Keys;
	foreach(const SAccessEntryConfig& Config, GetAccessConfig()) {
		if (!Keys.contains(Config.Setting))
			Keys << Config.Setting;
	}

	QMap<QString, QList<QString>> AccessMap;

	for (int i = 0; i < m_pTree->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = m_pTree->topLevelItem(i);

		if (pItem->data(0, Qt::UserRole).toInt() == -1)
			continue; // entry from template

		eAccessMode::EAccessMode Mode = (eAccessMode::EAccessMode)pItem->data(2, Qt::UserRole).toInt();
		QString Setting = GetSettingName(Mode);
		if (Setting.isEmpty())
			continue;

		QString Program = pItem->data(1, Qt::UserRole).toString();
		QString Value = pItem->data(3, Qt::UserRole).toString();
		if (!Program.isEmpty())
			Value.prepend(Program + ",");

		if (pItem->checkState(0) == Qt::Unchecked)
			Setting += "Disabled";
		AccessMap[Setting].append(Value);
	}

	foreach(const QString & Key, Keys) {
		m_pIni->UpdateTextList(Key, AccessMap[Key], m_bTemplate);
		m_pIni->UpdateTextList(Key + "Disabled", AccessMap[Key + "Disabled"], m_bTemplate);
	}
}

//////////////////////////////////////////////////////////////////// entry lookup (options integration)

QTreeWidgetItem* CSharedAccessWidget::FindEntry(eAccessMode::EAccessMode Mode, const QString& Program, const QString& Path, bool bOnlyEnabled)
{
	for (int i = 0; i < m_pTree->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = m_pTree->topLevelItem(i);
		if (pItem->data(0, Qt::UserRole).toInt() == GetAccessTypeId()
			&& pItem->data(1, Qt::UserRole).toString().compare(Program, Qt::CaseInsensitive) == 0
			&& pItem->data(2, Qt::UserRole).toInt() == (int)Mode
			&& pItem->data(3, Qt::UserRole).toString().compare(Path, Qt::CaseInsensitive) == 0
			&& (!bOnlyEnabled || pItem->checkState(0) == Qt::Checked))
			return pItem;
	}
	return NULL;
}

bool CSharedAccessWidget::HasEntry(int Mode, const QString& Program, const QString& Path)
{
	return FindEntry((eAccessMode::EAccessMode)Mode, Program, Path, true) != NULL;
}

void CSharedAccessWidget::SetEntry(int Mode, const QString& Program, const QString& Path)
{
	eAccessMode::EAccessMode AccessMode = (eAccessMode::EAccessMode)Mode;
	if (FindEntry(AccessMode, Program, Path))
		return; // already set
	AddEntry(AccessMode, Program, Path);
	OnConfigChanged();
}

void CSharedAccessWidget::DelEntry(int Mode, const QString& Program, const QString& Path)
{
	if (QTreeWidgetItem* pItem = FindEntry((eAccessMode::EAccessMode)Mode, Program, Path))
	{
		delete pItem;
		OnConfigChanged();
	}
}

///////////////////////////////////////////////////////////////////// editing

void CSharedAccessWidget::OnAdd()
{
	AddEntry(GetDefaultMode(), "", "");
	OnConfigChanged();
}

void CSharedAccessWidget::OnDel()
{
	QTreeWidgetItem* pItem = m_pTree->currentItem();
	if (!pItem)
		return;
	if (pItem->data(0, Qt::UserRole).toInt() == -1) {
		QMessageBox::warning(this, "SandboxiePlus", tr("Template values can not be removed."));
		return;
	}
	delete pItem;
	OnConfigChanged();
}

void CSharedAccessWidget::OnItemChanged(QTreeWidgetItem* pItem, int Column)
{
	if (Column != 0)
		return;
	OnConfigChanged();
}

void CSharedAccessWidget::OnSelectionChanged()
{
	CloseAccessEdit(false);
	OnConfigChanged();
}

void CSharedAccessWidget::OnItemDoubleClicked(QTreeWidgetItem* pItem, int Column)
{
	int Type = pItem->data(0, Qt::UserRole).toInt();
	if (Type == -1) {
		QMessageBox::warning(this, "Sandboxie-Plus", tr("Template values can not be edited."));
		return;
	}

	eAccessMode::EAccessMode Mode = (eAccessMode::EAccessMode)pItem->data(2, Qt::UserRole).toInt();

	QString Program = pItem->data(1, Qt::UserRole).toString();

	QWidget* pProgram = new QWidget();
	pProgram->setAutoFillBackground(true);
	QHBoxLayout* pLayout = new QHBoxLayout();
	pLayout->setContentsMargins(0, 0, 0, 0);
	pLayout->setSpacing(0);
	pProgram->setLayout(pLayout);
	QToolButton* pNot = new QToolButton(pProgram);
	pNot->setText("!");
	pNot->setCheckable(true);
	if (Program.left(1) == "!") {
		pNot->setChecked(true);
		Program.remove(0, 1);
	}
	pLayout->addWidget(pNot);

	QComboBox* pCombo = new QComboBox(pProgram);
	pCombo->addItem(tr("All Programs"), "");

	if (m_GetGroups) {
		foreach (const QString & Group, m_GetGroups()) {
			QString GroupName = Group.mid(1, Group.length() - 2);
			pCombo->addItem(tr("Group: %1").arg(GroupName), Group);
		}
	}

	foreach(const QString & Name, m_Programs)
		pCombo->addItem(Name, Name);

	pCombo->setEditable(true);
	int Index = pCombo->findData(Program);
	pCombo->setCurrentIndex(Index);
	if (Index == -1)
		pCombo->setCurrentText(Program);
	pLayout->addWidget(pCombo);

	m_pTree->setItemWidget(pItem, 1, pProgram);

	QComboBox* pMode = new QComboBox();
	foreach(const SAccessEntryConfig& Config, GetAccessConfig()) {
		pMode->addItem(Config.ModeName, (int)Config.Mode);
		pMode->setItemData(pMode->count() - 1, Config.ModeTip, Qt::ToolTipRole);
	}
	pMode->setCurrentIndex(pMode->findData((int)Mode));
	m_pTree->setItemWidget(pItem, 2, pMode);

	QLineEdit* pPath = new QLineEdit();
	pPath->setText(pItem->data(3, Qt::UserRole).toString());
	m_pTree->setItemWidget(pItem, 3, pPath);
}

void CSharedAccessWidget::CloseAccessEdit(bool bSave)
{
	for (int i = 0; i < m_pTree->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = m_pTree->topLevelItem(i);
		CloseAccessEdit(pItem, bSave);
	}
}

void CSharedAccessWidget::CloseAccessEdit(QTreeWidgetItem* pItem, bool bSave)
{
	QTreeWidget* pTree = m_pTree;

	QWidget* pProgram = pTree->itemWidget(pItem, 1);
	if (!pProgram)
		return;

	if (bSave)
	{
		QHBoxLayout* pLayout = (QHBoxLayout*)pProgram->layout();
		QToolButton* pNot = (QToolButton*)pLayout->itemAt(0)->widget();
		QComboBox* pCombo = (QComboBox*)pLayout->itemAt(1)->widget();

		QComboBox* pMode = (QComboBox*)pTree->itemWidget(pItem, 2);
		QLineEdit* pPath = (QLineEdit*)pTree->itemWidget(pItem, 3);

		QString Program = pCombo->currentText();
		int Index = pCombo->findText(Program);
		if (Index != -1)
			Program = pCombo->itemData(Index, Qt::UserRole).toString();
		if (!Program.isEmpty() && Program.left(1) != "<")
			m_Programs.insert(Program);

		eAccessMode::EAccessMode Mode = (eAccessMode::EAccessMode)pMode->currentData().toInt();
		QString Path = pPath->text();

		pItem->setText(1, (pNot->isChecked() ? "NOT " : "") + pCombo->currentText());
		pItem->setData(1, Qt::UserRole, (pNot->isChecked() ? "!" : "") + Program);

		// update mode
		QString ModeStr, ModeTip;
		foreach(const SAccessEntryConfig& Config, GetAccessConfig())
			if (Config.Mode == Mode) {
				ModeStr = Config.ModeName;
				ModeTip = Config.ModeTip;
				break;
			}
		pItem->setText(2, ModeStr);
		pItem->setData(2, Qt::UserRole, (int)Mode);
		if (!ModeTip.isEmpty())
			pItem->setToolTip(2, ModeTip);

		pItem->setText(3, ExpandPath(Path));
		pItem->setData(3, Qt::UserRole, Path);

		OnConfigChanged();
	}

	pTree->setItemWidget(pItem, 1, NULL);
	pTree->setItemWidget(pItem, 2, NULL);
	pTree->setItemWidget(pItem, 3, NULL);
}

bool CSharedAccessWidget::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == m_pTree->viewport())
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent* pKey = (QKeyEvent*)event;
			if (pKey->key() == Qt::Key_Delete && pKey->modifiers() == Qt::NoModifier)
			{
				CloseAccessEdit(true);
				OnDel();
				return true;
			}
			if (pKey->key() == Qt::Key_Escape && pKey->modifiers() == Qt::NoModifier)
			{
				CloseAccessEdit(false);
				return true;
			}
			if ((pKey->key() == Qt::Key_Enter || pKey->key() == Qt::Key_Return)
			 && (pKey->modifiers() == Qt::NoModifier || pKey->modifiers() == Qt::KeypadModifier))
			{
				CloseAccessEdit(true);
				return true;
			}
		}
		else if (event->type() == QEvent::MouseButtonPress)
			CloseAccessEdit(false);
	}
	return QWidget::eventFilter(watched, event);
}

void CSharedAccessWidget::OnConfigChanged()
{
	emit Changed();
}

void CSharedAccessWidget::OnReloadPathDisplay()
{
	for (int i = 0; i < m_pTree->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = m_pTree->topLevelItem(i);
		pItem->setText(3, ExpandPath(pItem->data(3, Qt::UserRole).toString()));
	}
}

void CSharedAccessWidget::ReloadPathDisplay()
{
	OnReloadPathDisplay();
}

////////////////////////////////////////////////////////////////////////////////////////
// CSharedFileWidget
////////////////////////////////////////////////////////////////////////////////////////

CSharedFileWidget::CSharedFileWidget(QWidget* parent)
	: CSharedAccessWidget(parent)
{
	FillAddMenu();
}

QList<SAccessEntryConfig> CSharedFileWidget::GetAccessConfig()
{
	QList<SAccessEntryConfig> Config;

	SAccessEntryConfig Entry;
	Entry.Mode = eAccessMode::eNormal;
	Entry.Setting = "NormalFilePath";
	Entry.ModeName = tr("Normal");
	Entry.ModeTip = tr("Regular Sandboxie behavior - allow read and also copy on write.");
	Config.append(Entry);

	Entry.Mode = eAccessMode::eOpen;
	Entry.Setting = "OpenFilePath";
	Entry.ModeName = tr("Open");
	Entry.ModeTip = tr("Allow write-access outside the sandbox.");
	Config.append(Entry);

	Entry.Mode = eAccessMode::eOpen4All;
	Entry.Setting = "OpenPipePath";
	Entry.ModeName = tr("Open for All");
	Entry.ModeTip = tr("Allow write-access outside the sandbox, also for applications installed inside the sandbox.");
	Config.append(Entry);

	Entry.Mode = eAccessMode::eClosed;
	Entry.Setting = "ClosedFilePath";
	Entry.ModeName = tr("Closed");
	Entry.ModeTip = tr("Deny access to host location and prevent creation of sandboxed copies.");
	Config.append(Entry);

	Entry.Mode = eAccessMode::eReadOnly;
	Entry.Setting = "ReadFilePath";
	Entry.ModeName = tr("Read Only");
	Entry.ModeTip = tr("Allow read-only access only.");
	Config.append(Entry);

	Entry.Mode = eAccessMode::eBoxOnly;
	Entry.Setting = "WriteFilePath";
	Entry.ModeName = tr("Box Only (Write Only)");
	Entry.ModeTip = tr("Hide host files, folders or registry keys from sandboxed processes.");
	Config.append(Entry);

	return Config;
}

QString CSharedFileWidget::ExpandPath(const QString& Path) const
{
	QString sPath = CSharedAccessWidget::ExpandPath(Path);

	if (!sPath.isEmpty()) {
		if (sPath.left(1) == "|")
			return sPath.mid(1);
		else if (!sPath.contains("*") && sPath.right(1) != "*")
			return sPath + "*";
	}
	return sPath;
}

void CSharedFileWidget::FillAddMenu()
{
	QMenu* pMenu = m_pAddButton->menu();
	pMenu->addAction(tr("Browse for File"), this, SLOT(OnBrowseFile()));
	pMenu->addAction(tr("Browse for Folder"), this, SLOT(OnBrowseFolder()));
}

void CSharedFileWidget::OnBrowseFile()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select File"), "", tr("All Files (*.*)")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	AddEntry(GetDefaultMode(), "", Value);
	OnConfigChanged();
}

void CSharedFileWidget::OnBrowseFolder()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	// Add a trailing backslash if it does not exist
	if (!Value.endsWith("\\"))
		Value.append("\\");

	AddEntry(GetDefaultMode(), "", Value);
	OnConfigChanged();
}