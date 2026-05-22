#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"
#include <limits>

namespace {

const int kRulePriorityColumn = 2;
const int kRuleRecursiveColumn = 3;
const int kRuleTargetBoxColumn = 4;
const int kRuleOriginalRuleRole = Qt::UserRole + 10;
const int kRuleActualTypeRole = Qt::UserRole + 11;

struct RuleExtensionsUi
{
	QString baseRule;
	QString priority;
	QString recursive;
	QString targetBox;
};

class CRuleTreeItem : public QTreeWidgetItem
{
public:
	using QTreeWidgetItem::QTreeWidgetItem;

	bool operator<(const QTreeWidgetItem& other) const override
	{
		QTreeWidget* pTree = treeWidget();
		int sortColumn = pTree ? pTree->sortColumn() : -1;

		if (sortColumn == kRulePriorityColumn) {
			auto toPriorityRank = [](const QString& text) -> qlonglong {
				QString value = text.trimmed();
				if (value.isEmpty() || value == "-")
					return (std::numeric_limits<qlonglong>::max)();

				bool ok = false;
				qlonglong parsed = value.toLongLong(&ok);
				if (!ok)
					return (std::numeric_limits<qlonglong>::max)();

				return parsed;
			};

			qlonglong leftRank = toPriorityRank(data(sortColumn, Qt::DisplayRole).toString());
			qlonglong rightRank = toPriorityRank(other.data(sortColumn, Qt::DisplayRole).toString());

			if (leftRank != rightRank)
				return leftRank < rightRank;
		}

		return QTreeWidgetItem::operator<(other);
	}
};

static bool RuleTypeSupportsPriority(int type)
{
	return (type == COptionsWindow::eProcess || type == COptionsWindow::ePath || type == COptionsWindow::eText || type == COptionsWindow::eParent);
}

static bool RuleTypeSupportsRecursive(int type)
{
	return (type == COptionsWindow::ePath);
}

static bool RuleTypeSupportsTargetBox(bool breakoutTree, int type)
{
	return (breakoutTree && (type == COptionsWindow::eProcess || type == COptionsWindow::ePath || type == COptionsWindow::eText));
}

static bool ParseLongValue(const QString& value, qlonglong* out)
{
	if (!out)
		return false;

	bool ok = false;
	qlonglong parsed = value.trimmed().toLongLong(&ok);
	if (!ok)
		return false;

	*out = parsed;
	return true;
}

static QString GetRuleKey(const QString& token)
{
	int eqPos = token.indexOf('=');
	return (eqPos >= 0 ? token.left(eqPos) : token).trimmed();
}

static bool IsKnownRuleExtension(const QString& token)
{
	QString key = GetRuleKey(token);
	return key.compare("TargetBox", Qt::CaseInsensitive) == 0
		|| key.compare("Priority", Qt::CaseInsensitive) == 0
		|| key.compare("Recursive", Qt::CaseInsensitive) == 0;
}

static QString BuildRuleWithBaseAndTokens(const QString& baseRule, const QStringList& tokens)
{
	QStringList out;
	out.reserve(tokens.size() + 1);
	out.append(baseRule);
	for (const QString& token : tokens) {
		QString trimmed = token.trimmed();
		if (!trimmed.isEmpty())
			out.append(trimmed);
	}
	return out.join("|");
}

static QString FormatRuleDisplayValue(bool enabled, bool supported, const QString& value, bool formatZeroAsNo = false)
{
	if (!supported)
		return "N/A";
	if (!enabled)
		return "-";
	if (value.trimmed().isEmpty())
		return "-";
	if (formatZeroAsNo && value == "0")
		return "n (0)";
	return value;
}

static QString CanonicalizeBoxNameCase(const QString& boxName)
{
	QString trimmed = boxName.trimmed();
	if (trimmed.isEmpty())
		return trimmed;

	QMap<QString, CSandBoxPtr> allBoxes = theAPI->GetAllBoxes();
	for (auto it = allBoxes.constBegin(); it != allBoxes.constEnd(); ++it) {
		if (!it.value() || !it.value()->IsEnabled())
			continue;

		QString existingName = it.value()->GetName();
		if (existingName.compare(trimmed, Qt::CaseInsensitive) == 0)
			return existingName;
	}

	return trimmed;
}

static void RefreshRuleExtensionsDisplayForTree(QTreeWidget* pTree, bool breakoutTree, bool enabled)
{
	if (!pTree)
		return;

	bool sortingEnabled = pTree->isSortingEnabled();
	if (sortingEnabled)
		pTree->setSortingEnabled(false);

	for (int i = 0; i < pTree->topLevelItemCount(); ++i) {
		QTreeWidgetItem* pItem = pTree->topLevelItem(i);
		if (!pItem)
			continue;

		int type = pItem->data(0, Qt::UserRole).toInt();
		int actualType = pItem->data(0, kRuleActualTypeRole).toInt();
		if (actualType == 0)
			actualType = type;

		QString priority = pItem->data(kRulePriorityColumn, Qt::UserRole).toString();
		QString recursive = pItem->data(kRuleRecursiveColumn, Qt::UserRole).toString();
		QString targetBox = CanonicalizeBoxNameCase(pItem->data(kRuleTargetBoxColumn, Qt::UserRole).toString());
		pItem->setData(kRuleTargetBoxColumn, Qt::UserRole, targetBox);

		if (kRulePriorityColumn < pTree->columnCount())
			pItem->setText(kRulePriorityColumn, FormatRuleDisplayValue(enabled, RuleTypeSupportsPriority(actualType), priority));
		if (kRuleRecursiveColumn < pTree->columnCount())
			pItem->setText(kRuleRecursiveColumn, FormatRuleDisplayValue(enabled, RuleTypeSupportsRecursive(actualType), recursive, true));
		if (kRuleTargetBoxColumn < pTree->columnCount()) {
			pItem->setText(kRuleTargetBoxColumn, FormatRuleDisplayValue(enabled, RuleTypeSupportsTargetBox(breakoutTree, actualType), targetBox));
			if (enabled && RuleTypeSupportsTargetBox(breakoutTree, actualType) && !targetBox.isEmpty())
				pItem->setToolTip(kRuleTargetBoxColumn, targetBox);
			else
				pItem->setToolTip(kRuleTargetBoxColumn, QString());
		}
	}

	if (sortingEnabled)
		pTree->setSortingEnabled(true);
}

static RuleExtensionsUi ParseRuleExtensionsForUi(const QString& rule)
{
	RuleExtensionsUi out;
	QStringList parts = rule.split("|", Qt::KeepEmptyParts);

	if (!parts.isEmpty())
		out.baseRule = parts.takeFirst();

	for (const QString& rawToken : parts) {
		QString token = rawToken.trimmed();
		if (token.isEmpty())
			continue;

		int eqPos = token.indexOf('=');
		QString key = (eqPos >= 0) ? token.left(eqPos).trimmed() : token;
		QString value = (eqPos >= 0) ? token.mid(eqPos + 1).trimmed() : QString();

		if (key.compare("TargetBox", Qt::CaseInsensitive) == 0 && !value.isEmpty())
			out.targetBox = value;
		else if (key.compare("Priority", Qt::CaseInsensitive) == 0 && !value.isEmpty())
			out.priority = value;
		else if (key.compare("Recursive", Qt::CaseInsensitive) == 0) {
			if (value.isEmpty())
				out.recursive = "y";
			else {
				QString normalized = value.toLower();
				if (normalized == "*" || normalized == "y" || normalized == "yes" || normalized == "true")
					out.recursive = "y";
				else if (normalized == "n" || normalized == "no" || normalized == "false")
					out.recursive = "0";
				else
					out.recursive = normalized;
			}
		}
	}

	return out;
}

static void ApplyRuleExtensionsToItem(QTreeWidgetItem* pItem, bool breakoutTree, int type, const RuleExtensionsUi& ext, bool showValues)
{
	if (!pItem)
		return;

	if (RuleTypeSupportsPriority(type)) {
		pItem->setData(kRulePriorityColumn, Qt::UserRole, ext.priority);
		pItem->setText(kRulePriorityColumn, FormatRuleDisplayValue(showValues, true, ext.priority));
	}
	else {
		pItem->setData(kRulePriorityColumn, Qt::UserRole, QString());
		pItem->setText(kRulePriorityColumn, FormatRuleDisplayValue(showValues, false, QString()));
	}

	if (RuleTypeSupportsRecursive(type)) {
		pItem->setData(kRuleRecursiveColumn, Qt::UserRole, ext.recursive);
		pItem->setText(kRuleRecursiveColumn, FormatRuleDisplayValue(showValues, true, ext.recursive, true));
	}
	else {
		pItem->setData(kRuleRecursiveColumn, Qt::UserRole, QString());
		pItem->setText(kRuleRecursiveColumn, FormatRuleDisplayValue(showValues, false, QString(), true));
	}

	if (RuleTypeSupportsTargetBox(breakoutTree, type)) {
		QString targetBox = CanonicalizeBoxNameCase(ext.targetBox);
		pItem->setData(kRuleTargetBoxColumn, Qt::UserRole, targetBox);
		pItem->setText(kRuleTargetBoxColumn, FormatRuleDisplayValue(showValues, true, targetBox));
		if (showValues && !targetBox.isEmpty())
			pItem->setToolTip(kRuleTargetBoxColumn, targetBox);
		else
			pItem->setToolTip(kRuleTargetBoxColumn, QString());
	}
	else {
		pItem->setData(kRuleTargetBoxColumn, Qt::UserRole, QString());
		pItem->setText(kRuleTargetBoxColumn, FormatRuleDisplayValue(showValues, false, QString()));
		pItem->setToolTip(kRuleTargetBoxColumn, QString());
	}
}

static QString BuildRuleWithExtensionsFromItem(QTreeWidgetItem* pItem, bool breakoutTree, int type, bool useExtensions)
{
	QString baseRule = pItem->data(1, Qt::UserRole).toString().trimmed();
	if (baseRule.isEmpty())
		return baseRule;

	QString originalRule = pItem->data(1, kRuleOriginalRuleRole).toString().trimmed();
	if (originalRule.isEmpty())
		originalRule = baseRule;

	QStringList originalParts = originalRule.split("|", Qt::KeepEmptyParts);
	if (!originalParts.isEmpty())
		originalParts.takeFirst();

	if (!useExtensions) {
		QString preservedRule = BuildRuleWithBaseAndTokens(baseRule, originalParts);
		pItem->setData(1, kRuleOriginalRuleRole, preservedRule);
		return preservedRule;
	}

	QStringList keptTokens;
	for (const QString& part : originalParts) {
		if (!IsKnownRuleExtension(part))
			keptTokens.append(part.trimmed());
	}

	if (RuleTypeSupportsTargetBox(breakoutTree, type)) {
		QString targetBox = pItem->data(kRuleTargetBoxColumn, Qt::UserRole).toString().trimmed();
		if (!targetBox.isEmpty())
			keptTokens.append("TargetBox=" + targetBox);
	}

	if (RuleTypeSupportsRecursive(type)) {
		QString recursive = pItem->data(kRuleRecursiveColumn, Qt::UserRole).toString().trimmed();
		if (!recursive.isEmpty()) {
			if (recursive == "-") {
				// Sentinel for clearing Recursive extension
			}
			else if (recursive.compare("y", Qt::CaseInsensitive) == 0 || recursive.compare("yes", Qt::CaseInsensitive) == 0 || recursive.compare("true", Qt::CaseInsensitive) == 0)
				keptTokens.append("Recursive=y");
			else if (recursive.compare("n", Qt::CaseInsensitive) == 0 || recursive.compare("no", Qt::CaseInsensitive) == 0 || recursive.compare("false", Qt::CaseInsensitive) == 0)
				keptTokens.append("Recursive=n");
			else {
				qlonglong depth = -1;
				if (ParseLongValue(recursive, &depth) && depth >= 0)
					keptTokens.append("Recursive=" + QString::number(depth));
			}
		}
	}

	if (RuleTypeSupportsPriority(type)) {
		QString priority = pItem->data(kRulePriorityColumn, Qt::UserRole).toString().trimmed();
		if (!priority.isEmpty() && priority != "-1") {
			qlonglong value = -1;
			if (ParseLongValue(priority, &value) && value >= 0)
				keptTokens.append("Priority=" + QString::number(value));
		}
	}

	QString finalRule = BuildRuleWithBaseAndTokens(baseRule, keptTokens);
	pItem->setData(1, kRuleOriginalRuleRole, finalRule);
	return finalRule;
}

}


void COptionsWindow::LoadForced()
{
	ui.treeForced->clear();

	foreach(const QString& Value, m_pBox->GetTextList("ForceProcess", m_Template))
		AddForcedEntry(Value, (int)eProcess);

	foreach(const QString& Value, m_pBox->GetTextList("ForceProcessDisabled", m_Template))
		AddForcedEntry(Value, (int)eProcess, true);

	foreach(const QString& Value, m_pBox->GetTextList("ForceChildren", m_Template))
		AddForcedEntry(Value, (int)eParent);

	foreach(const QString& Value, m_pBox->GetTextList("ForceChildrenDisabled", m_Template))
		AddForcedEntry(Value, (int)eParent, true);

	foreach(const QString& Value, m_pBox->GetTextList("ForceFolder", m_Template))
		AddForcedEntry(Value, (int)ePath);

	foreach(const QString& Value, m_pBox->GetTextList("ForceFolderDisabled", m_Template))
		AddForcedEntry(Value, (int)ePath, true);

	ui.chkDisableForced->setChecked(m_pBox->GetBool("DisableForceRules", false));

	ui.treeBreakout->clear();

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutProcess", m_Template))
		AddBreakoutEntry(Value, (int)eProcess);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutProcessDisabled", m_Template))
		AddBreakoutEntry(Value, (int)eProcess, true);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutFolder", m_Template))
		AddBreakoutEntry(Value, (int)ePath);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutFolderDisabled", m_Template))
		AddBreakoutEntry(Value, (int)ePath, true);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutDocument", m_Template))
		AddBreakoutEntry(Value, (int)eText);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutDocumentDisabled", m_Template))
		AddBreakoutEntry(Value, (int)eText, true);

	ReadGlobalCheck(ui.chkUseForceBreakoutRuleExtensions, "UseForceBreakoutRuleExtensions", false);
	ui.chkBreakoutUseTargetDir->setChecked(m_pBox->GetBool("BreakoutUseTargetDir", false, true, true));

	LoadForcedTmpl();
	LoadBreakoutTmpl();
	RefreshRuleExtensionsDisplayForTree(ui.treeForced, false, ui.chkUseForceBreakoutRuleExtensions->isChecked());
	RefreshRuleExtensionsDisplayForTree(ui.treeBreakout, true, ui.chkUseForceBreakoutRuleExtensions->isChecked());

	m_ForcedChanged = false;
}

void COptionsWindow::LoadForcedTmpl(bool bUpdate)
{
	if (ui.chkShowForceTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("ForceProcess", Template))
				AddForcedEntry(Value, (int)eProcess, false, Template);

			foreach(const QString& Value, m_pBox->GetTextListTmpl("ForceChildren", Template))
				AddForcedEntry(Value, (int)eParent, false, Template);

			foreach(const QString& Value, m_pBox->GetTextListTmpl("ForceFolder", Template))
				AddForcedEntry(Value, (int)ePath, false, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeForced->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeForced->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == (int)eTemplate) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::LoadBreakoutTmpl(bool bUpdate)
{
	if (ui.chkShowBreakoutTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("BreakoutProcess", Template))
				AddBreakoutEntry(Value, (int)eProcess, false, Template);

			foreach(const QString& Value, m_pBox->GetTextListTmpl("BreakoutFolder", Template))
				AddBreakoutEntry(Value, (int)ePath, false, Template);

			foreach(const QString& Value, m_pBox->GetTextListTmpl("BreakoutDocument", Template))
				AddBreakoutEntry(Value, (int)eText, false, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeBreakout->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeBreakout->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == (int)eTemplate) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::AddForcedEntry(const QString& Name, int type, bool disabled, const QString& Template)
{
	RuleExtensionsUi ext = ParseRuleExtensionsForUi(Name);
	QString baseRule = ext.baseRule;
	if (baseRule.isEmpty())
		baseRule = Name;

	QTreeWidgetItem* pItem = new CRuleTreeItem();
	pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	QString Type;
	switch (type)
	{
	case eProcess: Type = tr("Process"); break;
	case ePath: Type = tr("Folder"); break;
	case eParent: Type = tr("Children"); break;
	}
	pItem->setText(0, Type + (Template.isEmpty() ? "" : (" (" + Template + ")")));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : (int)eTemplate);
	pItem->setData(0, kRuleActualTypeRole, type);
	SetProgramItem(baseRule, pItem, 1);
	pItem->setData(1, kRuleOriginalRuleRole, Name);
	ApplyRuleExtensionsToItem(pItem, false, type, ext, ui.chkUseForceBreakoutRuleExtensions->isChecked());
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeForced->addTopLevelItem(pItem);
}

void COptionsWindow::AddBreakoutEntry(const QString& Name, int type, bool disabled, const QString& Template)
{
	RuleExtensionsUi ext = ParseRuleExtensionsForUi(Name);
	QString baseRule = ext.baseRule;
	if (baseRule.isEmpty())
		baseRule = Name;

	QTreeWidgetItem* pItem = new CRuleTreeItem();
	pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	QString Type;
	switch (type)
	{
	case eProcess: Type = tr("Process"); break;
	case ePath: Type = tr("Folder"); break;
	case eText: Type = tr("Document"); break;
	}
	pItem->setText(0, Type + (Template.isEmpty() ? "" : (" (" + Template + ")")));

	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : (int)eTemplate);
	pItem->setData(0, kRuleActualTypeRole, type);
	SetProgramItem(baseRule, pItem, 1, QString(), type == eProcess);
	pItem->setData(1, kRuleOriginalRuleRole, Name);
	ApplyRuleExtensionsToItem(pItem, true, type, ext, ui.chkUseForceBreakoutRuleExtensions->isChecked());
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeBreakout->addTopLevelItem(pItem);
}

void COptionsWindow::SaveForced()
{
	const bool useRuleExtensions = ui.chkUseForceBreakoutRuleExtensions->isChecked();

	QStringList ForceProcess;
	QStringList ForceProcessDisabled;
	QStringList ForceChildren;
	QStringList ForceChildrenDisabled;
	QStringList ForceFolder;
	QStringList ForceFolderDisabled;

	for (int i = 0; i < ui.treeForced->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeForced->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == (int)eTemplate)
			continue; // entry from template

		QString RuleValue = BuildRuleWithExtensionsFromItem(pItem, false, Type, useRuleExtensions);

		if (pItem->checkState(0) == Qt::Checked) {
			switch (Type) {
			case eProcess:	ForceProcess.append(RuleValue); break;
			case eParent:	ForceChildren.append(RuleValue); break;
			case ePath:		ForceFolder.append(RuleValue); break;
			}
		}
		else {
			switch (Type) {
			case eProcess:	ForceProcessDisabled.append(RuleValue); break;
			case eParent:	ForceChildrenDisabled.append(RuleValue); break;
			case ePath:		ForceFolderDisabled.append(RuleValue); break;
			}
		}
	}

	WriteTextList("ForceProcess", ForceProcess);
	WriteTextList("ForceProcessDisabled", ForceProcessDisabled);
	WriteTextList("ForceChildren", ForceChildren);
	WriteTextList("ForceChildrenDisabled", ForceChildrenDisabled);
	WriteTextList("ForceFolder", ForceFolder);
	WriteTextList("ForceFolderDisabled", ForceFolderDisabled);

	WriteAdvancedCheck(ui.chkDisableForced, "DisableForceRules", "y", "");


	QStringList BreakoutProcess;
	QStringList BreakoutProcessDisabled;
	QStringList BreakoutFolder;
	QStringList BreakoutFolderDisabled;
	QStringList BreakoutDocument;
	QStringList BreakoutDocumentDisabled;

	for (int i = 0; i < ui.treeBreakout->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeBreakout->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == (int)eTemplate)
			continue; // entry from template

		QString RuleValue = BuildRuleWithExtensionsFromItem(pItem, true, Type, useRuleExtensions);

		if (pItem->checkState(0) == Qt::Checked) {
			switch (Type) {
			case eProcess:	BreakoutProcess.append(RuleValue); break;
			case ePath: BreakoutFolder.append(RuleValue); break;
			case eText: BreakoutDocument.append(RuleValue); break;
			}
		}
		else {
			switch (Type) {
			case eProcess:	BreakoutProcessDisabled.append(RuleValue); break;
			case ePath: BreakoutFolderDisabled.append(RuleValue); break;
			case eText: BreakoutDocumentDisabled.append(RuleValue); break;
			}
		}
	}

	WriteTextList("BreakoutProcess", BreakoutProcess);
	WriteTextList("BreakoutProcessDisabled", BreakoutProcessDisabled);
	WriteTextList("BreakoutFolder", BreakoutFolder);
	WriteTextList("BreakoutFolderDisabled", BreakoutFolderDisabled);
	WriteTextList("BreakoutDocument", BreakoutDocument);
	WriteTextList("BreakoutDocumentDisabled", BreakoutDocumentDisabled);
	WriteAdvancedCheck(ui.chkBreakoutUseTargetDir, "BreakoutUseTargetDir", "y", "");
	WriteGlobalCheck(ui.chkUseForceBreakoutRuleExtensions, "UseForceBreakoutRuleExtensions", false);

	m_ForcedChanged = false;
}

void COptionsWindow::OnRuleExtensionsToggled(bool checked)
{
	if (checked) {
		QMessageBox::StandardButton choice = QMessageBox::warning(
			this,
			"Sandboxie-Plus",
			tr("Enabling Force/Breakout rule extensions changes how Force* and Breakout* rules are parsed for sandboxed and unsandboxed launches.\n\n"
			   "Use this only if you understand Priority, Recursive, and TargetBox semantics.\n\n"
			   "Click Help to open the documentation page for this setting.\n\n"
			   "Do you want to enable rule extensions?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Help,
			QMessageBox::No);

		if (choice == QMessageBox::Help) {
			theGUI->OpenUrl("sbie://docs/UseForceBreakoutRuleExtensions");
			QSignalBlocker blocker(ui.chkUseForceBreakoutRuleExtensions);
			ui.chkUseForceBreakoutRuleExtensions->setChecked(false);
			return;
		}

		if (choice != QMessageBox::Yes)
		{
			QSignalBlocker blocker(ui.chkUseForceBreakoutRuleExtensions);
			ui.chkUseForceBreakoutRuleExtensions->setChecked(false);
			return;
		}
	}

	OnForcedChanged();
	RefreshRuleExtensionsDisplayForTree(ui.treeForced, false, ui.chkUseForceBreakoutRuleExtensions->isChecked());
	RefreshRuleExtensionsDisplayForTree(ui.treeBreakout, true, ui.chkUseForceBreakoutRuleExtensions->isChecked());
}

void COptionsWindow::OnForceProg()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	if (!CheckForcedItem(Value, (int)eProcess))
		return;
	AddForcedEntry(Value, (int)eProcess);
	OnForcedChanged();
}

// Returns true if the rule (before optional |TargetBox suffix) ends with only a
// wildcard or wildcard+extension, indicating a potentially broad match pattern.
static bool IsBreakoutRuleBroad(const QString& Value)
{
	QString RulePart = Value.split("|").first();
	QString BaseName = RulePart.contains("\\") ? RulePart.split("\\").last() : RulePart;
	return BaseName.startsWith("*");
}

void COptionsWindow::OnBreakoutProg()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	if (IsBreakoutRuleBroad(Value)) {
		if (QMessageBox::warning(this, "Sandboxie-Plus",
			tr("The specified BreakoutProcess rule \"%1\" uses a broad wildcard pattern and may cause unintended processes to break out of the sandbox.\nAre you sure you want to add this rule?").arg(Value),
			QDialogButtonBox::Yes, QDialogButtonBox::No) != QDialogButtonBox::Yes)
			return;
	}
	AddBreakoutEntry(Value, (int)eProcess);
	OnForcedChanged();
}

void COptionsWindow::OnForceBrowseProg()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Executable File"), "", tr("Executable Files (*.exe)"));
	if (Value.isEmpty())
		return;
	if (!CheckForcedItem(Value, (int)eProcess))
		return;
	AddForcedEntry(Split2(Value, "/", true).second, (int)eProcess);
	OnForcedChanged();
}

void COptionsWindow::OnBreakoutBrowse()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Executable File"), "", tr("Executable Files (*.exe)"));
	if (Value.isEmpty())
		return;
	AddBreakoutEntry(Split2(Value, "/", true).second, (int)eProcess);
	OnForcedChanged();
}

void COptionsWindow::OnForceChild()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	if (!CheckForcedItem(Value, (int)eParent))
		return;
	AddForcedEntry(Value, (int)eParent);
	OnForcedChanged();
}

void COptionsWindow::OnForceBrowseChild()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Executable File"), "", tr("Executable Files (*.exe)"));
	if (Value.isEmpty())
		return;
	if (!CheckForcedItem(Value, (int)eParent))
		return;
	AddForcedEntry(Split2(Value, "/", true).second, (int)eParent);
	OnForcedChanged();
}

void COptionsWindow::OnForceDir()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;
	if (!CheckForcedItem(Value, (int)ePath))
		return;
	AddForcedEntry(Value, (int)ePath);
	OnForcedChanged();
}

void COptionsWindow::OnBreakoutDir()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;
	AddBreakoutEntry(Value, (int)ePath);
	OnForcedChanged();
}

void COptionsWindow::OnBreakoutDoc()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Document Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	QString Ext = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter Document File Extension."));
	if (Ext.isEmpty())
		return;
	
	if (Ext.left(1) == ".")
		Ext.prepend("*");
	else if (Ext.left(1) != "*")
		Ext.prepend("*.");

	if (Ext.right(1) == "*") {
		QMessageBox::warning(this, "Sandboxie-Plus", tr("For security reasons it is not permitted to create entirely wildcard BreakoutDocument presets."));
		return;
	}
	QStringList BannedExt = QString(// from: https://learn.microsoft.com/en-us/troubleshoot/developer/browsers/security-privacy/information-about-the-unsafe-file-list
		"*.ade;*.adp;*.app;*.asp;*.bas;*.bat;*.cer;*.chm;*.cmd;*.cnt;*.com;*.cpl;*.crt;*.csh;*.der;*.exe;*.fxp;*.gadget;*.grp;*.hlp;*.hpj;*.hta;"
		"*.img;*.inf;*.ins;*.iso;*.isp;*.its;*.js;*.jse;*.ksh;*.lnk;*.mad;*.maf;*.mag;*.mam;*.maq;*.mar;*.mas;*.mat;*.mau;*.mav;*.maw;*.mcf;*.mda;"
		"*.mdb;*.mde;*.mdt;*.mdw;*.mdz;*.msc;*.msh;*.msh1;*.msh1xml;*.msh2;*.msh2xml;*.mshxml;*.msi;*.msp;*.mst;*.msu;*.ops;*.pcd;*.pif;*.pl;*.plg;"
		"*.prf;*.prg;*.printerexport;*.ps1;*.ps1xml;*.ps2;*.ps2xml;*.psc1;*.psc2;*.psd1;*.psm1;*.pst;*.reg;*.scf;*.scr;*.sct;*.shb;*.shs;*.theme;"
		"*.tmp;*.url;*.vb;*.vbe;*.vbp;*.vbs;*.vhd;*.vhdx;*.vsmacros;*.vsw;*.webpnp;*.ws;*.wsc;*.wsf;*.wsh;*.xnk").split(";");
	if (BannedExt.contains(Ext.toLower())) {
		QMessageBox::warning(this, "Sansboxie-Plus", tr("For security reasons the specified extension %1 should not be broken out.").arg(Ext));
		// bypass security by holding down Ctr+Alt
		if ((QGuiApplication::queryKeyboardModifiers() & (Qt::AltModifier | Qt::ControlModifier)) != (Qt::AltModifier | Qt::ControlModifier))
			return;
	}

	Value += "\\" + Ext;

	AddBreakoutEntry(Value, (int)eText);
	OnForcedChanged();
}

void COptionsWindow::OnDelForce()
{
	DeleteAccessEntry(ui.treeForced->currentItem());
	OnForcedChanged();
}

void COptionsWindow::OnDelBreakout()
{
	DeleteAccessEntry(ui.treeBreakout->currentItem());
	OnForcedChanged();
}

void COptionsWindow::OnForcedChanged()
{
	m_ForcedChanged = true; 
	OnOptChanged();
}

bool COptionsWindow::CheckForcedItem(const QString& Value, int type)
{
	bool bDangerous = false;

	QString winPath = QString::fromUtf8(qgetenv("SystemRoot"));

	if (type == eProcess || type == eParent)
	{
		if (Value.compare("explorer.exe", Qt::CaseInsensitive) == 0 || Value.compare(winPath + "\\explorer.exe", Qt::CaseInsensitive) == 0)
			bDangerous = true;
		else if (Value.compare("taskmgr.exe", Qt::CaseInsensitive) == 0 || Value.compare(winPath + "\\system32\\taskmgr.exe", Qt::CaseInsensitive) == 0)
			bDangerous = true;
		else if (Value.contains("sbiesvc.exe", Qt::CaseInsensitive))
			bDangerous = true;
		else if (Value.contains("sandman.exe", Qt::CaseInsensitive))
			bDangerous = true;
	}
	else if (type == ePath)
	{
		if (Value.compare(winPath.left(3), Qt::CaseInsensitive) == 0)
			bDangerous = true; // SystemDrive (C:\)
		else if (Value.compare(winPath, Qt::CaseInsensitive) == 0)
			bDangerous = true; // SystemRoot (C:\Windows)
		else if (Value.left(winPath.length() + 1).compare(winPath + "\\", Qt::CaseInsensitive) == 0)
			bDangerous = true; // sub path of C:\Windows
	}

	if (bDangerous && QMessageBox::warning(this, "Sandboxie-Plus", tr("Forcing the specified entry will most likely break Windows, are you sure you want to proceed?")
		, QDialogButtonBox::Yes, QDialogButtonBox::No) != QDialogButtonBox::Yes)
		return false;
	return true;
}

void COptionsWindow::OnForcedChanged(QTreeWidgetItem *pItem, int) 
{
	QString Value = pItem->data(1, Qt::UserRole).toString();
	if (pItem->checkState(0) == Qt::Checked && !CheckForcedItem(Value, pItem->data(0, Qt::UserRole).toInt()))
		pItem->setCheckState(0, Qt::Unchecked);
	//qDebug() << Test;
	OnForcedChanged();
}

void COptionsWindow::OnBreakoutChanged(QTreeWidgetItem *pItem, int Column)
{
	// Warn when a rule is edited inline and results in a broad wildcard pattern.
	if (Column == 1) {
		int type = pItem->data(0, Qt::UserRole).toInt();
		if (type == eProcess || type == ePath) {
			QString Value = pItem->data(1, Qt::UserRole).toString();
			if (IsBreakoutRuleBroad(Value)) {
				QMessageBox::warning(this, "Sandboxie-Plus",
					tr("The breakout rule \"%1\" uses a broad wildcard pattern and may cause unintended processes to break out of the sandbox.").arg(Value));
			}
		}
	}
	OnForcedChanged();
}
