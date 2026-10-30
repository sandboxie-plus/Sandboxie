#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QDialog>
#include <QDropEvent>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMenu>
#include <QPalette>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSet>
#include <QSignalBlocker>
#include <QStyledItemDelegate>
#include <QTreeWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include "../Views/SbieView.h"
#include <algorithm>
#include <functional>

namespace {

const char kPriorityManagerBackupKey[] = "OptionsWindow/ForceBreakoutPriorityBackup";
const char kPriorityManagerShowDisabledKey[] = "OptionsWindow/ForceBreakoutPriorityShowDisabled";
const char kPriorityManagerShowGlobalKey[] = "OptionsWindow/ForceBreakoutPriorityShowGlobal";
const char kPriorityManagerShowTemplatesKey[] = "OptionsWindow/ForceBreakoutPriorityShowTemplates";
const char kPriorityManagerShowOtherBoxesKey[] = "OptionsWindow/ForceBreakoutPriorityShowOtherBoxes";
const char kPriorityManagerOptionsExpandedKey[] = "OptionsWindow/ForceBreakoutPriorityOptionsExpanded";
const char kPriorityManagerNumberingModeKey[] = "OptionsWindow/ForceBreakoutPriorityNumberingMode";
const char kPriorityManagerGeometryKey[] = "OptionsWindow/ForceBreakoutPriority_Geometry";
const char kPriorityManagerColumnsKey[] = "OptionsWindow/ForceBreakoutPriority_Columns";

const int kPriorityManagerSourceRole = Qt::UserRole + 120;
const int kPriorityManagerSettingRole = Qt::UserRole + 121;
const int kPriorityManagerIndexRole = Qt::UserRole + 122;
const int kPriorityManagerOriginalRuleRole = Qt::UserRole + 123;
const int kPriorityManagerSourceKindRole = Qt::UserRole + 125;
const int kPriorityManagerTypeRole = Qt::UserRole + 126;
const int kPriorityManagerBreakoutRole = Qt::UserRole + 127;
const int kPriorityManagerConflictRole = Qt::UserRole + 129;
const int kPriorityManagerSequenceRole = Qt::UserRole + 130;
const qlonglong kPriorityManagerMax = 9999;

const int kPriorityManagerPriorityColumn = 0;
const int kPriorityManagerKindColumn = 1;
const int kPriorityManagerTypeColumn = 2;
const int kPriorityManagerRuleColumn = 3;
const int kPriorityManagerRecursiveColumn = 4;
const int kPriorityManagerScopeColumn = 5;
const int kPriorityManagerSourceColumn = 6;
const int kPriorityManagerStateColumn = 7;
const int kPriorityManagerConflictColumn = 8;

enum PriorityManagerContextAction
{
	kPriorityManagerEnable,
	kPriorityManagerDisable,
	kPriorityManagerUnsetPriority,
	kPriorityManagerUnsetRecursive,
	kPriorityManagerUnsetScope
};

const int kPriorityManagerGlobalSource = 0;
const int kPriorityManagerBoxSource = 1;
const int kPriorityManagerTemplateSource = 2;

struct PriorityManagerSource
{
	QSharedPointer<CSbieIni> ini;
	QString key;
	QString label;
	QString toolTip;
	QSet<QString> forceScopes;
	QSet<QString> breakoutScopes;
	QSet<QString> forceExtensionDisabledScopes;
	QSet<QString> breakoutExtensionDisabledScopes;
	int kind = kPriorityManagerBoxSource;
	bool writable = true;
};

SB_STATUS CommitPriorityManagerSource(const PriorityManagerSource& source)
{
	CSandBoxPtr pBox = qSharedPointerDynamicCast<CSandBox>(source.ini);
	return pBox ? pBox->CommitIniChangesEx() : source.ini->GetAPI()->CommitIniChangesEx();
}

struct PriorityManagerSetting
{
	const char* name;
	int type;
	bool breakout;
};

struct PriorityManagerRule
{
	int sourceIndex = -1;
	QString setting;
	int index = -1;
	QString originalRule;
	QString priority;
	int numericPriority = 0;
	int type = COptionsWindow::eUndefined;
	bool breakout = false;
	bool disabled = false;
	int sequence = 0;
};

struct PriorityManagerItemChange
{
	QTreeWidgetItem* item = NULL;
	int sourceIndex = -1;
	QString originalSetting;
	int originalIndex = -1;
	QString originalRule;
	QString currentSetting;
	QString newRule;
};

struct PriorityManagerBackupEntry
{
	QString source;
	QString setting;
	QStringList values;
	QStringList afterValues;
	bool hasAfterValues = false;
};

static const PriorityManagerSetting kPriorityManagerSettings[] = {
	{ "ForceProcess", COptionsWindow::eProcess, false },
	{ "ForceProcessDisabled", COptionsWindow::eProcess, false },
	{ "ForceChildren", COptionsWindow::eParent, false },
	{ "ForceChildrenDisabled", COptionsWindow::eParent, false },
	{ "ForceFolder", COptionsWindow::ePath, false },
	{ "ForceFolderDisabled", COptionsWindow::ePath, false },
	{ "BreakoutProcess", COptionsWindow::eProcess, true },
	{ "BreakoutProcessDisabled", COptionsWindow::eProcess, true },
	{ "BreakoutFolder", COptionsWindow::ePath, true },
	{ "BreakoutFolderDisabled", COptionsWindow::ePath, true },
	{ "BreakoutDocument", COptionsWindow::eText, true },
	{ "BreakoutDocumentDisabled", COptionsWindow::eText, true }
};

static void AddPriorityManagerSource(QVector<PriorityManagerSource>& sources,
	QMap<QString, int>& sourceIndexes, const QString& key,
	const QSharedPointer<CSbieIni>& pIni, const QString& label, const QString& toolTip,
	int kind, bool writable = true)
{
	if (pIni.isNull() || sourceIndexes.contains(key))
		return;

	PriorityManagerSource source;
	source.ini = pIni;
	source.key = key;
	source.label = label;
	source.toolTip = toolTip;
	source.kind = kind;
	source.writable = writable;
	sourceIndexes.insert(key, sources.size());
	sources.append(source);
}

static QString PriorityManagerBoxDisplayName(const CSandBoxPtr& pBox)
{
	if (!pBox)
		return QString();

	QString boxName = pBox->GetName();
	QSharedPointer<CSandBoxPlus> pBoxPlus = pBox.objectCast<CSandBoxPlus>();
	if (pBoxPlus.isNull())
		return boxName;

	QString alias = pBoxPlus->GetText("BoxAlias").trimmed();
	return alias.isEmpty()
		? pBoxPlus->GetDisplayName(CSandBoxPlus::eDisplayCompact)
		: CSandBoxPlus::FormatDisplayName(boxName, alias, 0, CSandBoxPlus::eDisplayCompact);
}

static void PopulatePriorityManagerSourceScopes(QVector<PriorityManagerSource>& sources,
	CSbieAPI* pAPI)
{
	if (!pAPI)
		return;

	QMap<QString, CSandBoxPtr> allBoxes = pAPI->GetAllBoxes();
	QSharedPointer<CSbieIni> pGlobal = pAPI->GetGlobalSettings();
	QMap<QString, QString> globalTemplates;
	CSbieIni::CollectTemplateNames(pGlobal, globalTemplates);

	for (auto it = allBoxes.constBegin(); it != allBoxes.constEnd(); ++it) {
		const CSandBoxPtr& pBox = it.value();
		if (pBox.isNull() || !pBox->IsEnabled())
			continue;

		QString boxName = pBox->GetName().trimmed();
		if (boxName.isEmpty())
			continue;

		QString boxScope = boxName.toLower();
		QString boxSourceKey = "box:" + boxScope;
		bool forceEnabled = !pBox->GetBool("DisableForceRules", false, true, true);
		bool breakoutEnabled = !pBox->GetBool("DisableBreakoutRules", false, true, true);
		bool ruleExtensionsEnabled = pBox->GetBool("UseForceBreakoutRuleExtensions", false, true, true);
		QMap<QString, QString> boxTemplates;
		CSbieIni::CollectTemplateNames(pBox, boxTemplates);

		for (PriorityManagerSource& source : sources) {
			bool applies = false;
			if (source.key.compare("global", Qt::CaseInsensitive) == 0) {
				applies = true;
			}
			else if (source.kind == kPriorityManagerBoxSource) {
				applies = source.key.compare(boxSourceKey, Qt::CaseInsensitive) == 0;
			}
			else if (source.kind == kPriorityManagerTemplateSource
				&& source.key.startsWith("template:", Qt::CaseInsensitive)) {
				QString templateName = source.key.mid(9).toLower();
				applies = globalTemplates.contains(templateName) || boxTemplates.contains(templateName);
			}

			if (!applies)
				continue;
			if (forceEnabled)
				source.forceScopes.insert(boxScope);
			if (breakoutEnabled)
				source.breakoutScopes.insert(boxScope);
			if (forceEnabled) {
				if (ruleExtensionsEnabled)
					source.forceExtensionDisabledScopes.remove(boxScope);
				else
					source.forceExtensionDisabledScopes.insert(boxScope);
			}
			if (breakoutEnabled) {
				if (ruleExtensionsEnabled)
					source.breakoutExtensionDisabledScopes.remove(boxScope);
				else
					source.breakoutExtensionDisabledScopes.insert(boxScope);
			}
		}
	}
}

static QVector<PriorityManagerSource> BuildPriorityManagerSources(CSbieAPI* pAPI)
{
	QVector<PriorityManagerSource> sources;
	if (!pAPI)
		return sources;

	QMap<QString, int> sourceIndexes;
	QMap<QString, QString> templateNames;

	QSharedPointer<CSbieIni> pGlobal = pAPI->GetGlobalSettings();
	AddPriorityManagerSource(sources, sourceIndexes, "global", pGlobal,
		COptionsWindow::tr("Global settings"), "GlobalSettings", kPriorityManagerGlobalSource);
	CSbieIni::CollectTemplateNames(pGlobal, templateNames);

	QMap<QString, CSandBoxPtr> allBoxes = pAPI->GetAllBoxes();
	for (auto it = allBoxes.constBegin(); it != allBoxes.constEnd(); ++it) {
		const CSandBoxPtr& pBox = it.value();
		if (!pBox)
			continue;

		QString boxName = pBox->GetName();
		if (boxName.isEmpty())
			continue;

		QString displayName = PriorityManagerBoxDisplayName(pBox);
		if (!pBox->IsEnabled())
			displayName += COptionsWindow::tr(" (disabled)");

		QSharedPointer<CSbieIni> pBoxIni = pBox;
		AddPriorityManagerSource(sources, sourceIndexes, "box:" + boxName.toLower(), pBoxIni,
			COptionsWindow::tr("Box: %1").arg(displayName), boxName, kPriorityManagerBoxSource);
		CSbieIni::CollectTemplateNames(pBoxIni, templateNames);
	}

	for (auto it = templateNames.constBegin(); it != templateNames.constEnd(); ++it) {
		QString templateName = it.value();
		QString section = "Template_" + templateName;
		QSharedPointer<CSbieIni> pTemplate(new CSbieIni(section, pAPI));
		bool writable = templateName.startsWith("Local_", Qt::CaseInsensitive);
		QString label = COptionsWindow::tr("Template: %1").arg(templateName);
		if (!writable)
			label += COptionsWindow::tr(" (read-only)");
		AddPriorityManagerSource(sources, sourceIndexes, "template:" + templateName.toLower(),
			pTemplate, label, section, kPriorityManagerTemplateSource, writable);
	}
	PopulatePriorityManagerSourceScopes(sources, pAPI);

	return sources;
}

static QString PriorityManagerRuleTypeName(int type)
{
	switch (type)
	{
	case COptionsWindow::eProcess: return COptionsWindow::tr("Process");
	case COptionsWindow::ePath: return COptionsWindow::tr("Folder");
	case COptionsWindow::eText: return COptionsWindow::tr("Document");
	case COptionsWindow::eParent: return COptionsWindow::tr("Children");
	}
	return QString();
}

static QString PriorityManagerBaseSetting(const QString& setting)
{
	const QString suffix = "Disabled";
	return setting.endsWith(suffix, Qt::CaseInsensitive)
		? setting.left(setting.length() - suffix.length()) : setting;
}

static QString PriorityManagerSettingForState(const QString& setting, bool disabled)
{
	return PriorityManagerBaseSetting(setting) + (disabled ? "Disabled" : QString());
}

static bool IsPriorityManagerSetting(const QString& setting)
{
	for (const PriorityManagerSetting& settingInfo : kPriorityManagerSettings) {
		if (setting.compare(QString::fromLatin1(settingInfo.name), Qt::CaseInsensitive) == 0)
			return true;
	}
	return false;
}

static QString CanonicalizePriorityManagerTargetBox(CSbieAPI* pAPI, const QString& boxName)
{
	if (boxName.isEmpty() || !pAPI)
		return boxName;

	QMap<QString, CSandBoxPtr> allBoxes = pAPI->GetAllBoxes();
	for (auto it = allBoxes.constBegin(); it != allBoxes.constEnd(); ++it) {
		if (!it.value() || !it.value()->IsEnabled())
			continue;
		QString existingName = it.value()->GetName();
		if (existingName.compare(boxName, Qt::CaseInsensitive) == 0)
			return existingName;
	}
	return boxName;
}

static bool PriorityManagerTargetBoxExists(CSbieAPI* pAPI, const QString& boxName)
{
	if (boxName.isEmpty() || !pAPI)
		return true;

	QMap<QString, CSandBoxPtr> allBoxes = pAPI->GetAllBoxes();
	for (auto it = allBoxes.constBegin(); it != allBoxes.constEnd(); ++it) {
		if (it.value().isNull() || !it.value()->IsEnabled())
			continue;
		if (it.value()->GetName().compare(boxName, Qt::CaseInsensitive) == 0)
			return true;
	}
	return false;
}

static bool PriorityManagerSourceHasEnabledExtensions(const PriorityManagerSource& source,
	bool breakout)
{
	const QSet<QString>& scopes = breakout ? source.breakoutScopes : source.forceScopes;
	const QSet<QString>& disabledScopes = breakout
		? source.breakoutExtensionDisabledScopes : source.forceExtensionDisabledScopes;
	for (const QString& scope : scopes) {
		if (!disabledScopes.contains(scope))
			return true;
	}
	return false;
}

static QColor MixPriorityManagerColor(const QColor& base, const QColor& tint)
{
	return QColor((base.red() * 3 + tint.red()) / 4,
		(base.green() * 3 + tint.green()) / 4,
		(base.blue() * 3 + tint.blue()) / 4);
}

static QString PriorityManagerRecursiveDisplay(const QString& value)
{
	return value.trimmed().isEmpty() ? QString("-") : FormatRecursiveDisplayValue(value);
}

static QString PriorityManagerScopeDisplay(CSbieAPI* pAPI, const QString& value)
{
	QString canonical = CanonicalizePriorityManagerTargetBox(pAPI, value);
	return canonical.isEmpty() ? QString("-") : canonical;
}

static int PriorityManagerItemType(const QModelIndex& index)
{
	return index.sibling(index.row(), kPriorityManagerPriorityColumn)
		.data(kPriorityManagerTypeRole).toInt();
}

static bool PriorityManagerItemIsBreakout(const QModelIndex& index)
{
	return index.sibling(index.row(), kPriorityManagerPriorityColumn)
		.data(kPriorityManagerBreakoutRole).toBool();
}

class CForceBreakoutPriorityTree : public QTreeWidget
{
public:
	using QTreeWidget::QTreeWidget;
	using QTreeWidget::itemFromIndex;

	bool HasOrderChanged() const { return m_OrderChanged; }
	void ClearOrderChanged() { m_OrderChanged = false; }
	void SetMoveCallback(const std::function<void()>& callback) { m_MoveCallback = callback; }
	void SetReorderEnabled(bool enabled)
	{
		m_ReorderEnabled = enabled;
		setDragEnabled(enabled);
	}
	bool IsReorderEnabled() const { return m_ReorderEnabled; }

protected:
	void dropEvent(QDropEvent* pEvent) override
	{
		if (!m_ReorderEnabled) {
			pEvent->ignore();
			return;
		}

		bool sortingEnabled = isSortingEnabled();
		int sortColumn = header()->sortIndicatorSection();
		Qt::SortOrder sortOrder = header()->sortIndicatorOrder();
		if (sortColumn < 0)
			sortColumn = kPriorityManagerPriorityColumn;
		if (sortingEnabled)
			setSortingEnabled(false);
		QTreeWidget::dropEvent(pEvent);
		if (pEvent->isAccepted() && pEvent->dropAction() == Qt::MoveAction) {
			m_OrderChanged = true;
			if (m_MoveCallback)
				m_MoveCallback();
		}
		if (sortingEnabled) {
			setSortingEnabled(true);
			sortItems(sortColumn, sortOrder);
		}
	}

private:
	bool m_OrderChanged = false;
	bool m_ReorderEnabled = true;
	std::function<void()> m_MoveCallback;
};

class CForceBreakoutPriorityItem : public QTreeWidgetItem
{
public:
	using QTreeWidgetItem::QTreeWidgetItem;

	QTreeWidgetItem* clone() const override
	{
		return new CForceBreakoutPriorityItem(*this);
	}

	bool operator<(const QTreeWidgetItem& other) const override
	{
		const QTreeWidget* pTree = treeWidget();
		int column = pTree ? pTree->sortColumn() : kPriorityManagerPriorityColumn;
		if (column < 0)
			column = kPriorityManagerPriorityColumn;
		if (column == kPriorityManagerPriorityColumn) {
			qlonglong leftPriority = -1;
			qlonglong rightPriority = -1;
			bool leftHasPriority = ParseRulePriorityValue(
				data(kPriorityManagerPriorityColumn, Qt::UserRole).toString(), &leftPriority);
			bool rightHasPriority = ParseRulePriorityValue(
				other.data(kPriorityManagerPriorityColumn, Qt::UserRole).toString(), &rightPriority);
			if (leftHasPriority != rightHasPriority)
				return leftHasPriority;
			if (leftHasPriority && leftPriority != rightPriority)
				return leftPriority < rightPriority;
		}
		else if (column == kPriorityManagerConflictColumn) {
			int leftSeverity = data(kPriorityManagerConflictColumn,
				kPriorityManagerConflictRole).toInt();
			int rightSeverity = other.data(kPriorityManagerConflictColumn,
				kPriorityManagerConflictRole).toInt();
			if (leftSeverity != rightSeverity)
				return leftSeverity < rightSeverity;
		}

		int comparison = text(column).compare(other.text(column), Qt::CaseInsensitive);
		if (comparison != 0)
			return comparison < 0;
		return data(kPriorityManagerPriorityColumn, kPriorityManagerSequenceRole).toInt()
			< other.data(kPriorityManagerPriorityColumn, kPriorityManagerSequenceRole).toInt();
	}
};

class CForceBreakoutPriorityDelegate : public QStyledItemDelegate
{
public:
	explicit CForceBreakoutPriorityDelegate(
		const std::function<QMap<int, QStringList>(const QModelIndex&)>& getUsedPriorities,
		QObject* parent = NULL)
		: QStyledItemDelegate(parent), m_GetUsedPriorities(getUsedPriorities)
	{
	}

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const override
	{
		Q_UNUSED(option);

		QComboBox* pBox = new QComboBox(parent);
		pBox->setEditable(true);
		pBox->setInsertPolicy(QComboBox::NoInsert);
		pBox->addItem(COptionsWindow::tr("Not set"), QString());
		QMap<int, QStringList> usedSources = m_GetUsedPriorities
			? m_GetUsedPriorities(index) : QMap<int, QStringList>();
		QSet<int> usedPriorities;
		for (auto it = usedSources.constBegin(); it != usedSources.constEnd(); ++it)
			usedPriorities.insert(it.key());
		int currentPriority = -1;
		ParseRulePriorityValue(index.data(Qt::UserRole).toString(), &currentPriority);
		for (int priority : BuildRulePriorityValues(usedPriorities, currentPriority)) {
			QString value = QString::number(priority);
			pBox->addItem(value, value);
		}
		MarkUsedRulePriorityItems(pBox, usedSources, COptionsWindow::tr("(in use)"),
			COptionsWindow::tr("%1 (in use)"), COptionsWindow::tr("Used by:\n%1"));
		SetupWideDropdown(pBox);
		QRegularExpression expression("^[0-9]{0,4}$");
		pBox->lineEdit()->setValidator(new QRegularExpressionValidator(expression, pBox));
		return pBox;
	}

	void setEditorData(QWidget* editor, const QModelIndex& index) const override
	{
		QComboBox* pBox = qobject_cast<QComboBox*>(editor);
		if (!pBox)
			return;

		QString value = index.data(Qt::UserRole).toString().trimmed();
		int itemIndex = pBox->findData(value);
		if (itemIndex < 0 && !value.isEmpty()) {
			itemIndex = pBox->count();
			pBox->addItem(value, value);
		}
		pBox->setCurrentIndex(itemIndex < 0 ? 0 : itemIndex);
		if (!value.isEmpty())
			pBox->setEditText(value);
	}

	void setModelData(QWidget* editor, QAbstractItemModel* model,
		const QModelIndex& index) const override
	{
		QComboBox* pBox = qobject_cast<QComboBox*>(editor);
		if (!pBox)
			return;

		QString value = pBox->currentText().trimmed();
		if (value.compare(COptionsWindow::tr("Not set"), Qt::CaseInsensitive) == 0)
			value.clear();
		if (!value.isEmpty()) {
			int priority = -1;
			if (!ParseRulePriorityValue(value, &priority))
				return;
			value = QString::number(priority);
		}

		model->setData(index, value, Qt::UserRole);
		model->setData(index, value.isEmpty() ? "-" : value, Qt::DisplayRole);
		model->setData(index, value.isEmpty()
			? COptionsWindow::tr("No explicit priority.")
			: COptionsWindow::tr("Priority: %1").arg(value), Qt::ToolTipRole);
	}

private:
	std::function<QMap<int, QStringList>(const QModelIndex&)> m_GetUsedPriorities;
};

class CForceBreakoutPriorityRuleDelegate : public QStyledItemDelegate
{
public:
	using QStyledItemDelegate::QStyledItemDelegate;

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const override
	{
		Q_UNUSED(option);
		Q_UNUSED(index);
		return new QLineEdit(parent);
	}

	void setEditorData(QWidget* editor, const QModelIndex& index) const override
	{
		QLineEdit* pEdit = qobject_cast<QLineEdit*>(editor);
		if (pEdit)
			pEdit->setText(index.data(Qt::UserRole).toString());
	}

	void setModelData(QWidget* editor, QAbstractItemModel* model,
		const QModelIndex& index) const override
	{
		QLineEdit* pEdit = qobject_cast<QLineEdit*>(editor);
		if (!pEdit)
			return;

		QString value = pEdit->text().trimmed();
		model->setData(index, value, Qt::UserRole);
		model->setData(index, value, Qt::DisplayRole);
		model->setData(index, value, Qt::ToolTipRole);
	}
};

static void SetPriorityManagerComboPopupWidth(QComboBox* pBox)
{
	if (!pBox || !pBox->view())
		return;

	int width = pBox->width();
	for (int index = 0; index < pBox->count(); ++index)
		width = qMax(width, pBox->fontMetrics().horizontalAdvance(pBox->itemText(index)));
	pBox->view()->setTextElideMode(Qt::ElideNone);
	pBox->view()->setMinimumWidth(width + 32);
}

class CForceBreakoutPriorityComboDelegate : public QStyledItemDelegate
{
public:
	CForceBreakoutPriorityComboDelegate(CSbieAPI* pAPI, int column, QObject* parent = NULL)
		: QStyledItemDelegate(parent), m_pAPI(pAPI), m_Column(column)
	{
	}

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const override
	{
		Q_UNUSED(option);
		int type = PriorityManagerItemType(index);
		bool breakout = PriorityManagerItemIsBreakout(index);
		if (m_Column == kPriorityManagerRecursiveColumn && !RuleTypeSupportsRecursive(type))
			return NULL;
		if (m_Column == kPriorityManagerScopeColumn && !RuleTypeSupportsTargetBox(breakout, type))
			return NULL;

		QComboBox* pBox = new QComboBox(parent);
		if (m_Column == kPriorityManagerRecursiveColumn) {
			pBox->setEditable(true);
			pBox->addItem("-", "");
			pBox->addItem(FormatRecursiveDisplayValue("n"), "n");
			pBox->addItem(FormatRecursiveDisplayValue("y"), "y");
			for (int depth = 1; depth <= 9; ++depth) {
				QString value = QString::number(depth);
				pBox->addItem(FormatRecursiveDisplayValue(value), value);
			}
			const QStringList rangePresets = { "1-2", "1-3", "1-5", "2-4", "2-2" };
			for (const QString& value : rangePresets)
				pBox->addItem(FormatRecursiveDisplayValue(value), value);
			QRegularExpression expression("^(?:[yYnN]|[0-9]+(?:\\s*-\\s*[0-9]+)?)(?:\\s*;\\s*(?:[Ff][Ii][Rr][Ss][Tt]|[Ll][Aa][Ss][Tt]))?$");
			pBox->lineEdit()->setValidator(new QRegularExpressionValidator(expression, pBox));
		}
		else if (m_Column == kPriorityManagerScopeColumn) {
			pBox->addItem(COptionsWindow::tr("(unset)"), "");
			QMap<QString, CSandBoxPtr> allBoxes = m_pAPI ? m_pAPI->GetAllBoxes() : QMap<QString, CSandBoxPtr>();
			for (auto it = allBoxes.constBegin(); it != allBoxes.constEnd(); ++it) {
				if (!it.value() || !it.value()->IsEnabled())
					continue;
				QString boxName = it.value()->GetName();
				if (boxName.isEmpty())
					continue;
				int itemIndex = pBox->count();
				pBox->addItem(PriorityManagerBoxDisplayName(it.value()), boxName);
				pBox->setItemData(itemIndex, boxName, Qt::ToolTipRole);
			}
		}
		else {
			pBox->addItem(COptionsWindow::tr("Enabled"), false);
			pBox->addItem(COptionsWindow::tr("Disabled"), true);
		}
		SetPriorityManagerComboPopupWidth(pBox);
		return pBox;
	}

	void setEditorData(QWidget* editor, const QModelIndex& index) const override
	{
		QComboBox* pBox = qobject_cast<QComboBox*>(editor);
		if (!pBox)
			return;

		if (m_Column == kPriorityManagerRecursiveColumn) {
			QString value = NormalizeRecursiveExtensionValue(index.data(Qt::UserRole).toString());
			int itemIndex = pBox->findData(value);
			if (itemIndex < 0) {
				itemIndex = pBox->count();
				pBox->addItem(PriorityManagerRecursiveDisplay(value), value);
			}
			pBox->setCurrentIndex(itemIndex);
			pBox->setEditText(value);
		}
		else if (m_Column == kPriorityManagerScopeColumn) {
			QString value = CanonicalizePriorityManagerTargetBox(m_pAPI, index.data(Qt::UserRole).toString());
			int itemIndex = -1;
			for (int i = 0; i < pBox->count(); ++i) {
				if (pBox->itemData(i).toString().compare(value, Qt::CaseInsensitive) == 0) {
					itemIndex = i;
					break;
				}
			}
			if (itemIndex < 0 && !value.isEmpty()) {
				itemIndex = pBox->count();
				pBox->addItem(COptionsWindow::tr("Unavailable: %1").arg(value), value);
				pBox->setItemData(itemIndex, value, Qt::ToolTipRole);
			}
			if (itemIndex >= 0)
				pBox->setCurrentIndex(itemIndex);
		}
		else {
			bool disabled = index.data(Qt::UserRole).toBool();
			pBox->setCurrentIndex(disabled ? 1 : 0);
		}
		SetPriorityManagerComboPopupWidth(pBox);
	}

	void setModelData(QWidget* editor, QAbstractItemModel* model,
		const QModelIndex& index) const override
	{
		QComboBox* pBox = qobject_cast<QComboBox*>(editor);
		if (!pBox)
			return;

		if (m_Column == kPriorityManagerRecursiveColumn) {
			QString rawValue = pBox->lineEdit()->text().trimmed();
			for (int itemIndex = 0; itemIndex < pBox->count(); ++itemIndex) {
				if (pBox->itemText(itemIndex).compare(rawValue, Qt::CaseSensitive) == 0) {
					rawValue = pBox->itemData(itemIndex).toString();
					break;
				}
			}
			if (rawValue == "-")
				rawValue.clear();
			QString value = rawValue.isEmpty() ? QString() : NormalizeRecursiveExtensionValue(rawValue);
			if (!rawValue.isEmpty() && value.isEmpty())
				return;
			model->setData(index, value, Qt::UserRole);
			model->setData(index, PriorityManagerRecursiveDisplay(value), Qt::DisplayRole);
		}
		else if (m_Column == kPriorityManagerScopeColumn) {
			QString value = CanonicalizePriorityManagerTargetBox(m_pAPI,
				pBox->currentData().toString());
			model->setData(index, value, Qt::UserRole);
			model->setData(index, PriorityManagerScopeDisplay(m_pAPI, value), Qt::DisplayRole);
			model->setData(index, value, Qt::ToolTipRole);
		}
		else {
			bool disabled = pBox->currentData().toBool();
			model->setData(index, disabled, Qt::UserRole);
			model->setData(index, disabled ? COptionsWindow::tr("Disabled") : COptionsWindow::tr("Enabled"), Qt::DisplayRole);
		}
	}

private:
	CSbieAPI* m_pAPI;
	int m_Column;
};

class CForceBreakoutPriorityReadOnlyDelegate : public QStyledItemDelegate
{
public:
	using QStyledItemDelegate::QStyledItemDelegate;

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
		const QModelIndex& index) const override
	{
		Q_UNUSED(parent);
		Q_UNUSED(option);
		Q_UNUSED(index);
		return NULL;
	}
};

static bool PriorityManagerItemsOverlap(QTreeWidgetItem* pLeft, QTreeWidgetItem* pRight,
	const QVector<PriorityManagerSource>& sources)
{
	if (!pLeft || !pRight || pLeft == pRight)
		return false;

	int leftSource = pLeft->data(kPriorityManagerPriorityColumn, kPriorityManagerSourceRole).toInt();
	int rightSource = pRight->data(kPriorityManagerPriorityColumn, kPriorityManagerSourceRole).toInt();
	if (leftSource < 0 || leftSource >= sources.size()
		|| rightSource < 0 || rightSource >= sources.size())
		return false;
	if (leftSource == rightSource) {
		QString leftSetting = pLeft->data(kPriorityManagerPriorityColumn,
			kPriorityManagerSettingRole).toString();
		QString rightSetting = pRight->data(kPriorityManagerPriorityColumn,
			kPriorityManagerSettingRole).toString();
		if (leftSetting.compare(rightSetting, Qt::CaseInsensitive) == 0)
			return false;
	}

	bool leftBreakout = pLeft->data(kPriorityManagerPriorityColumn, kPriorityManagerBreakoutRole).toBool();
	bool rightBreakout = pRight->data(kPriorityManagerPriorityColumn, kPriorityManagerBreakoutRole).toBool();

	const QSet<QString>& leftScopes = leftBreakout
		? sources[leftSource].breakoutScopes : sources[leftSource].forceScopes;
	const QSet<QString>& rightScopes = rightBreakout
		? sources[rightSource].breakoutScopes : sources[rightSource].forceScopes;
	if (leftScopes.isEmpty() || rightScopes.isEmpty())
		return false;

	if (!leftBreakout && !rightBreakout)
		return true;

	for (const QString& scope : leftScopes) {
		if (rightScopes.contains(scope))
			return true;
	}
	return false;
}

static QVector<QTreeWidgetItem*> PriorityManagerOverlappingItems(
	const QVector<QTreeWidgetItem*>& items, const QVector<PriorityManagerSource>& sources)
{
	QVector<QTreeWidgetItem*> overlapping;
	for (int left = 0; left < items.size(); ++left) {
		for (int right = left + 1; right < items.size(); ++right) {
			if (!PriorityManagerItemsOverlap(items[left], items[right], sources))
				continue;
			if (!overlapping.contains(items[left]))
				overlapping.append(items[left]);
			if (!overlapping.contains(items[right]))
				overlapping.append(items[right]);
		}
	}
	return overlapping;
}

static QVector<QTreeWidgetItem*> PriorityManagerOverlappingItems(
	const QVector<QTreeWidgetItem*>& leftItems, const QVector<QTreeWidgetItem*>& rightItems,
	const QVector<PriorityManagerSource>& sources)
{
	QVector<QTreeWidgetItem*> overlapping;
	for (QTreeWidgetItem* pLeft : leftItems) {
		for (QTreeWidgetItem* pRight : rightItems) {
			if (!PriorityManagerItemsOverlap(pLeft, pRight, sources))
				continue;
			if (!overlapping.contains(pLeft))
				overlapping.append(pLeft);
			if (!overlapping.contains(pRight))
				overlapping.append(pRight);
		}
	}
	return overlapping;
}

static QString PriorityManagerConflictPrioritySources(const QVector<QTreeWidgetItem*>& items,
	const QVector<PriorityManagerSource>& sources)
{
	QMap<QString, QStringList> sourcesByPriority;
	for (QTreeWidgetItem* pItem : items) {
		if (!pItem)
			continue;

		int sourceIndex = pItem->data(kPriorityManagerPriorityColumn,
			kPriorityManagerSourceRole).toInt();
		if (sourceIndex < 0 || sourceIndex >= sources.size())
			continue;

		QString priority = pItem->data(kPriorityManagerPriorityColumn,
			Qt::UserRole).toString().trimmed();
		if (priority.isEmpty())
			priority = COptionsWindow::tr("no priority");
		QString sourceLabel = sources[sourceIndex].label;
		QString setting = pItem->data(kPriorityManagerPriorityColumn,
			kPriorityManagerSettingRole).toString();
		if (!setting.isEmpty())
			sourceLabel += " [" + setting + "]";
		if (!sourcesByPriority[priority].contains(sourceLabel))
			sourcesByPriority[priority].append(sourceLabel);
	}

	QStringList entries;
	for (auto it = sourcesByPriority.constBegin(); it != sourcesByPriority.constEnd(); ++it)
		entries.append(COptionsWindow::tr("%1: %2").arg(it.key(), it.value().join(", ")));
	return entries.join("; ");
}

static bool BuildPriorityAssignment(const QVector<int>& oldPriorities, QVector<int>* assignedPriorities)
{
	if (!assignedPriorities || oldPriorities.size() > static_cast<int>(kPriorityManagerMax) + 1)
		return false;

	assignedPriorities->fill(-1, oldPriorities.size());
	if (oldPriorities.isEmpty())
		return true;

	const int count = oldPriorities.size();
	QVector<int> lengths(count, 1);
	QVector<int> previous(count, -1);
	int bestIndex = -1;
	for (int index = 0; index < count; ++index) {
		if (oldPriorities[index] < 0 || oldPriorities[index] > kPriorityManagerMax)
			continue;
		for (int before = 0; before < index; ++before) {
			if (lengths[before] > 0 && oldPriorities[before] >= 0
				&& oldPriorities[before] <= kPriorityManagerMax
				&& oldPriorities[before] < oldPriorities[index]
				&& lengths[before] + 1 > lengths[index]) {
				lengths[index] = lengths[before] + 1;
				previous[index] = before;
			}
		}
		if (bestIndex < 0 || lengths[index] > lengths[bestIndex])
			bestIndex = index;
	}

	QVector<bool> anchor(count, false);
	for (int index = bestIndex; index >= 0; index = previous[index])
		anchor[index] = true;

	auto assignGap = [&](int first, int last, int lower, int upper) {
		if (first >= last)
			return true;
		if (upper < lower || last - first > upper - lower + 1)
			return false;
		int previousValue = lower - 1;
		for (int index = first; index < last; ++index) {
			int minimum = previousValue + 1;
			int maximum = upper - (last - index - 1);
			int value = oldPriorities[index];
			if (value < minimum)
				value = minimum;
			else if (value > maximum)
				value = maximum;
			(*assignedPriorities)[index] = value;
			previousValue = value;
		}
		return true;
	};

	bool gapsFit = true;
	int previousAnchor = -1;
	for (int index = 0; index <= count; ++index) {
		if (index < count && !anchor[index])
			continue;
		int nextAnchor = index < count ? index : -1;
		if (nextAnchor >= 0)
			(*assignedPriorities)[nextAnchor] = oldPriorities[nextAnchor];
		int lower = previousAnchor >= 0 ? (*assignedPriorities)[previousAnchor] + 1 : 0;
		int upper = nextAnchor >= 0 ? (*assignedPriorities)[nextAnchor] - 1
			: static_cast<int>(kPriorityManagerMax);
		int first = previousAnchor + 1;
		if (!assignGap(first, index, lower, upper)) {
			gapsFit = false;
			break;
		}
		if (nextAnchor >= 0)
			previousAnchor = nextAnchor;
	}
	if (gapsFit)
		return true;

	int previousValue = -1;
	for (int index = 0; index < count; ++index) {
		int minimum = previousValue + 1;
		int maximum = static_cast<int>(kPriorityManagerMax) - (count - index - 1);
		int value = oldPriorities[index];
		if (value < minimum)
			value = minimum;
		else if (value > maximum)
			value = maximum;
		(*assignedPriorities)[index] = value;
		previousValue = value;
	}
	return true;
}

static bool BuildPriorityAssignmentInRange(const QVector<int>& oldPriorities,
	int lower, int upper, QVector<int>* assignedPriorities)
{
	if (!assignedPriorities)
		return false;
	assignedPriorities->fill(-1, oldPriorities.size());
	if (oldPriorities.isEmpty())
		return true;
	if (lower < 0 || upper > kPriorityManagerMax || lower > upper
		|| oldPriorities.size() > upper - lower + 1)
		return false;

	int previous = lower - 1;
	for (int index = 0; index < oldPriorities.size(); ++index) {
		int minimum = previous + 1;
		int maximum = upper - (oldPriorities.size() - index - 1);
		int priority = oldPriorities[index];
		if (priority < minimum)
			priority = minimum;
		else if (priority > maximum)
			priority = maximum;
		(*assignedPriorities)[index] = priority;
		previous = priority;
	}
	return true;
}

static bool BuildPriorityAssignmentInAvailableSlots(const QVector<int>& oldPriorities,
	int lower, int upper, const QSet<int>& occupied, QVector<int>* assignedPriorities)
{
	if (!assignedPriorities)
		return false;
	assignedPriorities->fill(-1, oldPriorities.size());
	if (oldPriorities.isEmpty())
		return true;
	if (lower < 0 || upper > kPriorityManagerMax || lower > upper)
		return false;

	QVector<int> available;
	for (int priority = lower; priority <= upper; ++priority) {
		if (!occupied.contains(priority))
			available.append(priority);
	}
	if (available.size() < oldPriorities.size())
		return false;

	int previousSlot = -1;
	for (int index = 0; index < oldPriorities.size(); ++index) {
		int firstSlot = previousSlot + 1;
		int lastSlot = available.size() - (oldPriorities.size() - index);
		int slot = firstSlot;
		while (slot <= lastSlot && available[slot] < oldPriorities[index])
			++slot;
		if (slot > lastSlot)
			slot = lastSlot;
		else if (slot > firstSlot) {
			int previousDistance = oldPriorities[index] - available[slot - 1];
			int currentDistance = available[slot] - oldPriorities[index];
			if (previousDistance <= currentDistance)
				--slot;
		}
		(*assignedPriorities)[index] = available[slot];
		previousSlot = slot;
	}
	return true;
}

class CForceBreakoutPriorityDialog : public QDialog
{
public:
	CForceBreakoutPriorityDialog(CSbieAPI* pAPI, QWidget* parent,
		const std::function<bool()>& hasPendingChanges,
		const std::function<void()>& refreshMain, const QString& originBoxKey,
		const QString& originTemplateKey)
		: QDialog(parent), m_pAPI(pAPI), m_HasPendingChanges(hasPendingChanges),
		m_RefreshMain(refreshMain), m_OriginBoxKey(originBoxKey),
		m_OriginTemplateKey(originTemplateKey)
	{
		setWindowTitle(COptionsWindow::tr("Force/Breakout Priority Manager"));
		setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
			| Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint
			| Qt::WindowCloseButtonHint);
		setMinimumSize(1100, 520);
		resize(1400, 720);
		setModal(false);
		setWindowModality(Qt::NonModal);
		if (theConf)
			restoreGeometry(theConf->GetBlob(kPriorityManagerGeometryKey));
		m_Sources = BuildPriorityManagerSources(m_pAPI);

		QVBoxLayout* pLayout = new QVBoxLayout(this);

		m_pInfo = new QLabel(COptionsWindow::tr(
			"Lower numeric priorities win. This view combines global settings, all boxes, and their referenced templates. "
			"Predefined templates are shown for information only and cannot be edited. "
			"Rules without an explicit priority keep legacy ordering. Drag prioritized rules to reorder them, "
			"or edit Rule, Priority, Recursive, Scope, and State directly. Select multiple rows and right-click "
			"for bulk enable/disable or unset actions. Click headers to sort, "
			"press Ctrl+F to search, or Ctrl+Shift+Plus to resize columns. Changes are written when you press Apply or OK. "
			"Conflict checks cover matching base rules and overlapping enabled-box scopes; wildcard and path-containment "
			"overlaps are not inferred automatically."), this);
		m_pInfo->setWordWrap(true);
		pLayout->addWidget(m_pInfo);
		m_InfoText = m_pInfo->text();

		QHBoxLayout* pFilterLayout = new QHBoxLayout();
		pFilterLayout->addWidget(new QLabel(COptionsWindow::tr("Source:"), this));
		m_pSourceFilter = new QComboBox(this);
		m_pSourceFilter->setEditable(true);
		m_pSourceFilter->setInsertPolicy(QComboBox::NoInsert);
		m_pSourceFilter->lineEdit()->setPlaceholderText(COptionsWindow::tr("Search sources"));
		m_pSourceFilter->lineEdit()->setClearButtonEnabled(true);
		if (m_pSourceFilter->completer()) {
			m_pSourceFilter->completer()->setCaseSensitivity(Qt::CaseInsensitive);
			m_pSourceFilter->completer()->setFilterMode(Qt::MatchContains);
		}
		m_pSourceFilter->setMinimumWidth(190);
		PopulateSourceFilter(QString());
		pFilterLayout->addWidget(m_pSourceFilter, 2);
		pFilterLayout->addWidget(new QLabel(COptionsWindow::tr("Setting:"), this));
		m_pSettingFilter = new QComboBox(this);
		m_pSettingFilter->addItem(COptionsWindow::tr("All settings"), QStringLiteral("all"));
		m_pSettingFilter->addItem(COptionsWindow::tr("Force"), QStringLiteral("force"));
		m_pSettingFilter->addItem(COptionsWindow::tr("Breakout"), QStringLiteral("breakout"));
		QSet<QString> settingNames;
		for (const PriorityManagerSetting& settingInfo : kPriorityManagerSettings) {
			QString setting = QString::fromLatin1(settingInfo.name);
			if (setting.endsWith("Disabled", Qt::CaseInsensitive))
				continue;
			if (!settingNames.contains(setting)) {
				settingNames.insert(setting);
				m_pSettingFilter->addItem(setting, setting);
			}
		}
		m_pSettingFilter->setMinimumWidth(175);
		pFilterLayout->addWidget(m_pSettingFilter, 1);
		pFilterLayout->addWidget(new QLabel(COptionsWindow::tr("Search:"), this));
		m_pSearch = new QLineEdit(this);
		m_pSearch->setClearButtonEnabled(true);
		m_pSearch->setMinimumWidth(180);
		m_pSearch->setPlaceholderText(COptionsWindow::tr("Rule, source, type, or priority"));
		m_pSearch->setToolTip(COptionsWindow::tr(
			"Filter rows by rule, source, type, priority, scope, state, or conflict. Press Ctrl+F to focus the search field."));
		pFilterLayout->addWidget(m_pSearch, 2);
		m_pRefresh = new QPushButton(COptionsWindow::tr("Refresh"), this);
		m_pRefresh->setToolTip(COptionsWindow::tr(
			"Reload Force/Breakout rules from global settings, all boxes, and their referenced templates."));
		pFilterLayout->addWidget(m_pRefresh);
		m_pOptions = new QToolButton(this);
		m_pOptions->setIcon(CSandMan::GetIcon("List"));
		m_pOptions->setText(COptionsWindow::tr("Options"));
		m_pOptions->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		m_pOptions->setCheckable(true);
		m_pOptions->setAutoRaise(true);
		m_pOptions->setToolTip(COptionsWindow::tr("Show or hide Force/Breakout Priority Manager options."));
		pFilterLayout->addWidget(m_pOptions);
		pLayout->addLayout(pFilterLayout);

		QWidget* pOptionsWidget = new QWidget(this);
		QHBoxLayout* pOptionsLayout = new QHBoxLayout(pOptionsWidget);
		pOptionsLayout->setContentsMargins(0, 0, 0, 0);
		m_pShowDisabled = new QCheckBox(COptionsWindow::tr("Disabled"), pOptionsWidget);
		m_pShowGlobal = new QCheckBox(COptionsWindow::tr("Global"), pOptionsWidget);
		m_pShowTemplates = new QCheckBox(COptionsWindow::tr("Templates"), pOptionsWidget);
		m_pShowOtherBoxes = new QCheckBox(COptionsWindow::tr("Other boxes"), pOptionsWidget);
		m_pShowDisabled->setChecked(!theConf || theConf->GetBool(kPriorityManagerShowDisabledKey, true));
		m_pShowGlobal->setChecked(!theConf || theConf->GetBool(kPriorityManagerShowGlobalKey, true));
		m_pShowTemplates->setChecked(!theConf || theConf->GetBool(kPriorityManagerShowTemplatesKey, true));
		m_pShowOtherBoxes->setChecked(!theConf || theConf->GetBool(kPriorityManagerShowOtherBoxesKey, true));
		m_pShowOtherBoxes->setEnabled(!m_OriginBoxKey.isEmpty());
		m_pShowOtherBoxes->setToolTip(m_OriginBoxKey.isEmpty()
			? COptionsWindow::tr("Available only when opened from a sandbox; all box rules remain visible.")
			: COptionsWindow::tr("Show rules from other boxes. Global and Template rows are unaffected."));
		pOptionsLayout->addWidget(m_pShowDisabled);
		pOptionsLayout->addWidget(m_pShowGlobal);
		pOptionsLayout->addWidget(m_pShowTemplates);
		pOptionsLayout->addWidget(m_pShowOtherBoxes);
		pOptionsLayout->addStretch(1);
		pOptionsLayout->addWidget(new QLabel(COptionsWindow::tr("Numbering mode:"), pOptionsWidget));
		m_pNumberingMode = new QComboBox(pOptionsWidget);
		m_pNumberingMode->addItem(COptionsWindow::tr("Keep existing priorities and fill gaps when possible"), 0);
		m_pNumberingMode->addItem(COptionsWindow::tr(
			"Use consecutive priorities, starting at 1 (maximum 9999)"), 1);
		int numberingMode = theConf ? theConf->GetInt(kPriorityManagerNumberingModeKey, 0) : 0;
		m_pNumberingMode->setCurrentIndex(numberingMode == 1 ? 1 : 0);
		m_NumberingMode = m_pNumberingMode->currentIndex();
		m_pNumberingMode->setToolTip(COptionsWindow::tr(
			"Keep existing priority numbers where possible and use empty gaps before renumbering. "
			"Compact mode assigns consecutive priorities starting at 1, with a maximum of 9999."));
		SetPriorityManagerComboPopupWidth(m_pNumberingMode);
		pOptionsLayout->addWidget(m_pNumberingMode);
		const bool optionsExpanded = !theConf
			|| theConf->GetBool(kPriorityManagerOptionsExpandedKey, true);
		m_pOptions->setChecked(optionsExpanded);
		pOptionsWidget->setVisible(optionsExpanded);
		pLayout->addWidget(pOptionsWidget);

		m_pConflictLabel = new QLabel(this);
		m_pConflictLabel->setWordWrap(true);
		pLayout->addWidget(m_pConflictLabel);

		m_pTree = new CForceBreakoutPriorityTree(this);
		m_pTree->setColumnCount(9);
		m_pTree->setHeaderLabels(QStringList()
			<< COptionsWindow::tr("Priority")
			<< COptionsWindow::tr("Kind")
			<< COptionsWindow::tr("Type")
			<< COptionsWindow::tr("Rule")
			<< COptionsWindow::tr("Recursive")
			<< COptionsWindow::tr("Scope")
			<< COptionsWindow::tr("Source")
			<< COptionsWindow::tr("State")
			<< COptionsWindow::tr("Conflicts"));
		m_pTree->setRootIsDecorated(false);
		m_pTree->setAlternatingRowColors(true);
		m_pTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_pTree->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_pTree->setContextMenuPolicy(Qt::CustomContextMenu);
		m_pTree->setDragEnabled(true);
		m_pTree->setAcceptDrops(true);
		m_pTree->setDropIndicatorShown(true);
		m_pTree->setDragDropMode(QAbstractItemView::InternalMove);
		m_pTree->setDefaultDropAction(Qt::MoveAction);
		m_pTree->header()->setSortIndicatorShown(true);
		m_pTree->setEditTriggers(QAbstractItemView::DoubleClicked
			| QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
		m_pTree->setItemDelegateForColumn(kPriorityManagerPriorityColumn,
			new CForceBreakoutPriorityDelegate([this](const QModelIndex& index) {
				return GetUsedRulePrioritySources(m_pTree->itemFromIndex(index));
			}, m_pTree));
		m_pTree->setItemDelegateForColumn(kPriorityManagerRuleColumn,
			new CForceBreakoutPriorityRuleDelegate(m_pTree));
		m_pTree->setItemDelegateForColumn(kPriorityManagerRecursiveColumn,
			new CForceBreakoutPriorityComboDelegate(m_pAPI, kPriorityManagerRecursiveColumn, m_pTree));
		m_pTree->setItemDelegateForColumn(kPriorityManagerScopeColumn,
			new CForceBreakoutPriorityComboDelegate(m_pAPI, kPriorityManagerScopeColumn, m_pTree));
		m_pTree->setItemDelegateForColumn(kPriorityManagerStateColumn,
			new CForceBreakoutPriorityComboDelegate(m_pAPI, kPriorityManagerStateColumn, m_pTree));
		for (int column : { kPriorityManagerKindColumn, kPriorityManagerTypeColumn,
			kPriorityManagerSourceColumn, kPriorityManagerConflictColumn })
			m_pTree->setItemDelegateForColumn(column, new CForceBreakoutPriorityReadOnlyDelegate(m_pTree));
		m_pTree->header()->setSectionResizeMode(QHeaderView::Interactive);
		m_pTree->header()->setStretchLastSection(false);
		m_pTree->header()->resizeSection(kPriorityManagerPriorityColumn, 85);
		m_pTree->header()->resizeSection(kPriorityManagerKindColumn, 95);
		m_pTree->header()->resizeSection(kPriorityManagerTypeColumn, 90);
		m_pTree->header()->resizeSection(kPriorityManagerRuleColumn, 300);
		m_pTree->header()->resizeSection(kPriorityManagerRecursiveColumn, 115);
		m_pTree->header()->resizeSection(kPriorityManagerScopeColumn, 150);
		m_pTree->header()->resizeSection(kPriorityManagerSourceColumn, 190);
		m_pTree->header()->resizeSection(kPriorityManagerStateColumn, 90);
		m_pTree->header()->resizeSection(kPriorityManagerConflictColumn, 100);
		if (theConf && m_pTree->header()->restoreState(theConf->GetBlob(kPriorityManagerColumnsKey))) {
			int sortColumn = m_pTree->header()->sortIndicatorSection();
			if (sortColumn >= 0) {
				m_SortColumn = sortColumn;
				m_SortOrder = m_pTree->header()->sortIndicatorOrder();
			}
		}
		pLayout->addWidget(m_pTree, 1);
		QHBoxLayout* pRuleCountLayout = new QHBoxLayout();
		m_pRuleCount = new QLabel(COptionsWindow::tr("Rules: 0 / 0"), this);
		pRuleCountLayout->addWidget(m_pRuleCount);
		pRuleCountLayout->addStretch(1);
		pLayout->addLayout(pRuleCountLayout);

		QHBoxLayout* pButtonLayout = new QHBoxLayout();
		m_pRevert = new QPushButton(COptionsWindow::tr("Revert..."), this);
		m_pRevert->setToolTip(COptionsWindow::tr(
			"Restore the Force/Breakout priorities and rule values saved before the last manager save."));
		pButtonLayout->addWidget(m_pRevert);
		pButtonLayout->addStretch(1);
		QPushButton* pOk = new QPushButton(COptionsWindow::tr("OK"), this);
		pOk->setDefault(true);
		pOk->setToolTip(COptionsWindow::tr("Apply the priority changes and close the manager."));
		pButtonLayout->addWidget(pOk);
		QPushButton* pCancel = new QPushButton(COptionsWindow::tr("Cancel"), this);
		pCancel->setToolTip(COptionsWindow::tr("Discard unapplied priority changes and keep the manager open."));
		pButtonLayout->addWidget(pCancel);
		m_pApply = new QPushButton(COptionsWindow::tr("Apply"), this);
		m_pApply->setToolTip(COptionsWindow::tr("Apply the priority changes and keep the manager open."));
		pButtonLayout->addWidget(m_pApply);
		QPushButton* pClose = new QPushButton(COptionsWindow::tr("Close"), this);
		pClose->setToolTip(COptionsWindow::tr("Close the manager and discard unapplied priority changes."));
		pButtonLayout->addWidget(pClose);
		connect(pOk, &QPushButton::clicked, this, [this]() { SaveAndAccept(); });
		connect(pCancel, &QPushButton::clicked, this, [this]() { CancelChanges(); });
		connect(m_pApply, &QPushButton::clicked, this, [this]() { ApplyChanges(); });
		connect(pClose, &QPushButton::clicked, this, [this]() { CloseManager(); });
		connect(m_pRevert, &QPushButton::clicked, this, [this]() { RevertLastSaved(); });
		connect(m_pRefresh, &QPushButton::clicked, this, [this]() { RefreshView(); });
		pLayout->addLayout(pButtonLayout);

		QAction* pFindAction = new QAction(COptionsWindow::tr("Focus priority search"), this);
		pFindAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+F")));
		pFindAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		addAction(pFindAction);
		connect(pFindAction, &QAction::triggered, this, [this]() {
			m_pSearch->setFocus();
			m_pSearch->selectAll();
		});
		QAction* pClearSearchAction = new QAction(COptionsWindow::tr("Clear priority search"), this);
		pClearSearchAction->setShortcut(QKeySequence(Qt::Key_Escape));
		pClearSearchAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		pClearSearchAction->setEnabled(false);
		addAction(pClearSearchAction);
		connect(pClearSearchAction, &QAction::triggered, this, [this]() {
			if (m_pSearch && !m_pSearch->text().isEmpty())
				m_pSearch->clear();
		});

		QAction* pResizeColumnsAction = new QAction(
			COptionsWindow::tr("Resize All Columns to Contents"), this);
		pResizeColumnsAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift++")));
		pResizeColumnsAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		addAction(pResizeColumnsAction);
		connect(pResizeColumnsAction, &QAction::triggered, this, [this]() {
			for (int column = 0; column < m_pTree->columnCount(); ++column)
				m_pTree->resizeColumnToContents(column);
		});

		Populate();
		UpdateRevertButton();
		UpdateApplyButton();

		connect(m_pShowDisabled, &QCheckBox::toggled, this, [this](bool checked) {
			if (theConf)
				theConf->SetValue(kPriorityManagerShowDisabledKey, checked);
			ApplyVisibility();
		});
		connect(m_pShowGlobal, &QCheckBox::toggled, this, [this](bool checked) {
			if (theConf)
				theConf->SetValue(kPriorityManagerShowGlobalKey, checked);
			ApplyVisibility();
		});
		connect(m_pShowTemplates, &QCheckBox::toggled, this, [this](bool checked) {
			if (theConf)
				theConf->SetValue(kPriorityManagerShowTemplatesKey, checked);
			ApplyVisibility();
		});
		connect(m_pShowOtherBoxes, &QCheckBox::toggled, this, [this](bool checked) {
			if (theConf)
				theConf->SetValue(kPriorityManagerShowOtherBoxesKey, checked);
			ApplyVisibility();
		});
		connect(m_pSourceFilter, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this, [this](int) { ApplyVisibility(); });
		connect(m_pSourceFilter->lineEdit(), &QLineEdit::textEdited, this,
			[this](const QString&) { ApplyVisibility(); });
		connect(m_pSettingFilter, qOverload<int>(&QComboBox::currentIndexChanged),
			this, [this](int) { ApplyVisibility(); });
		connect(m_pSearch, &QLineEdit::textChanged, this, [this, pClearSearchAction](const QString& text) {
			pClearSearchAction->setEnabled(!text.isEmpty());
			ApplyVisibility();
		});
		connect(m_pOptions, &QToolButton::toggled, this, [pOptionsWidget](bool expanded) {
			if (theConf)
				theConf->SetValue(kPriorityManagerOptionsExpandedKey, expanded);
			pOptionsWidget->setVisible(expanded);
		});
		connect(m_pNumberingMode, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
			if (!IsReorderView()) {
				QSignalBlocker blocker(m_pNumberingMode);
				m_pNumberingMode->setCurrentIndex(m_NumberingMode);
				return;
			}
			int previousMode = m_NumberingMode;
			if (!ApplyNumberingMode(index)) {
				QSignalBlocker blocker(m_pNumberingMode);
				m_pNumberingMode->setCurrentIndex(previousMode);
				return;
			}
			m_NumberingMode = index;
			if (theConf)
				theConf->SetValue(kPriorityManagerNumberingModeKey, index == 1 ? 1 : 0);
			RecomputeConflicts();
			UpdateApplyButton();
		});
		connect(m_pTree, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* pItem, int column) {
			if (!pItem)
				return;
			if (column == kPriorityManagerPriorityColumn)
				UpdatePriorityItem(pItem);
			else if (column == kPriorityManagerRuleColumn)
				UpdateRuleItem(pItem);
			else if (column == kPriorityManagerRecursiveColumn)
				UpdateRecursiveItem(pItem);
			else if (column == kPriorityManagerScopeColumn)
				UpdateScopeItem(pItem);
			else if (column == kPriorityManagerStateColumn)
				UpdateStateItem(pItem);
			if (column == kPriorityManagerRuleColumn || column == kPriorityManagerRecursiveColumn) {
				int type = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerTypeRole).toInt();
				QString originalRule = pItem->data(kPriorityManagerPriorityColumn,
					kPriorityManagerOriginalRuleRole).toString();
				QString baseRule = pItem->data(kPriorityManagerRuleColumn, Qt::UserRole).toString();
				QString recursive = pItem->data(kPriorityManagerRecursiveColumn, Qt::UserRole).toString();
				if (type == COptionsWindow::ePath
					&& HasRecursiveRuleEdits(originalRule, baseRule, recursive)
					&& IsRecursiveFolderRuleMissingTerminalWildcard(baseRule, recursive))
					WarnRecursiveFolderRuleEdit(this, baseRule);
			}
			if (column == kPriorityManagerRuleColumn || column == kPriorityManagerPriorityColumn || column == kPriorityManagerRecursiveColumn
				|| column == kPriorityManagerScopeColumn || column == kPriorityManagerStateColumn)
				RecomputeConflicts();
			ApplyVisibility();
			UpdateApplyButton();
		});
		connect(m_pTree, &QTreeWidget::itemDoubleClicked, this,
			[this](QTreeWidgetItem* pItem, int column) {
				if (column == kPriorityManagerSourceColumn)
					OpenSource(pItem);
			});
		connect(m_pTree, &QTreeWidget::customContextMenuRequested, this,
			[this](const QPoint& pos) { ShowContextMenu(pos); });
		connect(m_pTree->header(), &QHeaderView::sortIndicatorChanged, this,
			[this](int column, Qt::SortOrder order) {
				m_SortColumn = column;
				m_SortOrder = order;
				UpdateReorderingState();
			});
		m_pTree->SetMoveCallback([this]() {
			if (ApplyNumberingMode(m_NumberingMode))
				RecomputeConflicts();
			UpdateApplyButton();
		});
	}

	void SetCallbacks(const std::function<bool()>& hasPendingChanges,
		const std::function<void()>& refreshMain)
	{
		m_HasPendingChanges = hasPendingChanges;
		m_RefreshMain = refreshMain;
	}

	void SetOriginContext(const QString& originBoxKey, const QString& originTemplateKey)
	{
		m_OriginBoxKey = originBoxKey;
		m_OriginTemplateKey = originTemplateKey;
		m_pShowOtherBoxes->setEnabled(!m_OriginBoxKey.isEmpty());
		m_pShowOtherBoxes->setToolTip(m_OriginBoxKey.isEmpty()
			? COptionsWindow::tr("Available only when opened from a sandbox; all box rules remain visible.")
			: COptionsWindow::tr("Show rules from other boxes. Global and Template rows are unaffected."));
		ApplyVisibility();
	}

	~CForceBreakoutPriorityDialog() override
	{
		if (theConf) {
			theConf->SetBlob(kPriorityManagerGeometryKey, saveGeometry());
			theConf->SetBlob(kPriorityManagerColumnsKey, m_pTree->header()->saveState());
		}
	}

	void reject() override
	{
		if (!RestorePendingRevert())
			return;
		QDialog::reject();
	}

	void keyPressEvent(QKeyEvent* pEvent) override
	{
		if (pEvent && pEvent->key() == Qt::Key_Escape && m_pSearch
			&& !m_pSearch->text().isEmpty()) {
			m_pSearch->clear();
			pEvent->accept();
			return;
		}
		QDialog::keyPressEvent(pEvent);
	}

	private:
	QString SelectedSourceFilterKey() const
	{
		if (!m_pSourceFilter)
			return QString();
		int index = m_pSourceFilter->currentIndex();
		if (index < 0 || m_pSourceFilter->currentText().compare(
			m_pSourceFilter->itemText(index), Qt::CaseInsensitive) != 0)
			return QString();
		return m_pSourceFilter->currentData().toString();
	}

	void PopulateSourceFilter(const QString& selectedKey)
	{
		if (!m_pSourceFilter)
			return;
		QSignalBlocker blocker(m_pSourceFilter);
		m_pSourceFilter->clear();
		m_pSourceFilter->addItem(COptionsWindow::tr("All sources"), QString());
		int selectedIndex = 0;
		for (int i = 0; i < m_Sources.size(); ++i) {
			m_pSourceFilter->addItem(m_Sources[i].label, m_Sources[i].key);
			if (!selectedKey.isEmpty()
				&& m_Sources[i].key.compare(selectedKey, Qt::CaseInsensitive) == 0)
				selectedIndex = i + 1;
		}
		m_pSourceFilter->setCurrentIndex(selectedIndex);
	}

	void RefreshSources()
	{
		QString selectedKey = SelectedSourceFilterKey();
		m_Sources = BuildPriorityManagerSources(m_pAPI);
		PopulateSourceFilter(selectedKey);
	}

	bool IsSourceFilterSelectionValid() const
	{
		int index = m_pSourceFilter ? m_pSourceFilter->currentIndex() : -1;
		return index >= 0 && m_pSourceFilter->currentText().compare(
			m_pSourceFilter->itemText(index), Qt::CaseInsensitive) == 0;
	}

	bool IsAllSourceFilter() const
	{
		return IsSourceFilterSelectionValid()
			&& m_pSourceFilter->currentData().toString().isEmpty();
	}

	bool IsAllSettingFilter() const
	{
		return m_pSettingFilter
			&& m_pSettingFilter->currentData().toString() == QStringLiteral("all");
	}

	void CancelChanges()
	{
		if (!HasPendingManagerChanges())
			return;
		if (!RestorePendingRevert())
			return;

		RefreshSources();
		Populate();
		UpdateRevertButton();
		UpdateApplyButton();
	}

	void CloseManager()
	{
		reject();
	}

	void Populate()
	{
		m_pTree->setSortingEnabled(false);
		QSignalBlocker blocker(m_pTree);
		m_pTree->clear();
		m_pTree->ClearOrderChanged();

		QVector<PriorityManagerRule> rules;
		int sequence = 0;
		for (int sourceIndex = 0; sourceIndex < m_Sources.size(); ++sourceIndex) {
			const PriorityManagerSource& source = m_Sources[sourceIndex];
			for (const PriorityManagerSetting& settingInfo : kPriorityManagerSettings) {
				QString setting = QString::fromLatin1(settingInfo.name);
				// CONF_GET_NO_TEMPLS also hides entire built-in template sections.
				QStringList values = source.ini->GetTextList(setting, source.kind == kPriorityManagerTemplateSource);
				for (int index = 0; index < values.size(); ++index) {
					const QString& rule = values[index];
					RuleExtensionsUi extensions = ParseRuleExtensionsForUi(rule);
					qlonglong numericPriority = -1;
					bool hasPriority = ParseRulePriorityValue(extensions.priority, &numericPriority);

					PriorityManagerRule managerRule;
					managerRule.sourceIndex = sourceIndex;
					managerRule.setting = setting;
					managerRule.index = index;
					managerRule.originalRule = rule;
					managerRule.priority = hasPriority ? QString::number(numericPriority) : QString();
					managerRule.numericPriority = hasPriority ? static_cast<int>(numericPriority) : static_cast<int>(kPriorityManagerMax + 1);
					managerRule.type = settingInfo.type;
					managerRule.breakout = settingInfo.breakout;
					managerRule.disabled = setting.endsWith("Disabled", Qt::CaseInsensitive);
					managerRule.sequence = sequence++;
					rules.append(managerRule);
				}
			}
		}

		std::stable_sort(rules.begin(), rules.end(), [](const PriorityManagerRule& left,
			const PriorityManagerRule& right) {
			bool leftHasPriority = !left.priority.isEmpty();
			bool rightHasPriority = !right.priority.isEmpty();
			if (leftHasPriority != rightHasPriority)
				return leftHasPriority > rightHasPriority;
			if (left.numericPriority != right.numericPriority)
				return left.numericPriority < right.numericPriority;
			return left.sequence < right.sequence;
		});

		m_CanReorder = true;
		for (const PriorityManagerRule& managerRule : rules) {
			if (!managerRule.priority.isEmpty() && !m_Sources[managerRule.sourceIndex].writable) {
				m_CanReorder = false;
				break;
			}
		}

		QColor baseColor = m_pTree->palette().color(QPalette::Base);
		QColor forceColor = MixPriorityManagerColor(baseColor, QColor(75, 150, 220));
		QColor breakoutColor = MixPriorityManagerColor(baseColor, QColor(235, 170, 65));
		QColor disabledText = m_pTree->palette().color(QPalette::Disabled, QPalette::Text);

		for (const PriorityManagerRule& managerRule : rules) {
			const PriorityManagerSource& source = m_Sources[managerRule.sourceIndex];
			RuleExtensionsUi extensions = ParseRuleExtensionsForUi(managerRule.originalRule);
			QString targetBox = CanonicalizePriorityManagerTargetBox(m_pAPI, extensions.targetBox);
			QString recursive = extensions.recursive;
			QTreeWidgetItem* pItem = new CForceBreakoutPriorityItem();
			pItem->setData(kPriorityManagerPriorityColumn, Qt::UserRole, managerRule.priority);
			pItem->setData(kPriorityManagerPriorityColumn, kPriorityManagerSourceRole, managerRule.sourceIndex);
			pItem->setData(kPriorityManagerPriorityColumn, kPriorityManagerSettingRole, managerRule.setting);
			pItem->setData(kPriorityManagerPriorityColumn, kPriorityManagerIndexRole, managerRule.index);
			pItem->setData(kPriorityManagerPriorityColumn, kPriorityManagerOriginalRuleRole, managerRule.originalRule);
			pItem->setData(kPriorityManagerPriorityColumn, kPriorityManagerSourceKindRole, source.kind);
			pItem->setData(kPriorityManagerPriorityColumn, kPriorityManagerTypeRole, managerRule.type);
			pItem->setData(kPriorityManagerPriorityColumn, kPriorityManagerBreakoutRole, managerRule.breakout);
			pItem->setData(kPriorityManagerPriorityColumn, kPriorityManagerSequenceRole, managerRule.sequence);
			pItem->setText(kPriorityManagerPriorityColumn, managerRule.priority.isEmpty() ? "-" : managerRule.priority);
			pItem->setText(kPriorityManagerKindColumn, managerRule.breakout
				? COptionsWindow::tr("Breakout") : COptionsWindow::tr("Force"));
			pItem->setText(kPriorityManagerTypeColumn, PriorityManagerRuleTypeName(managerRule.type));
			pItem->setText(kPriorityManagerRuleColumn, extensions.baseRule.isEmpty()
				? managerRule.originalRule : extensions.baseRule);
			pItem->setText(kPriorityManagerRecursiveColumn,
				RuleTypeSupportsRecursive(managerRule.type) ? PriorityManagerRecursiveDisplay(recursive) : "N/A");
			pItem->setText(kPriorityManagerScopeColumn,
				RuleTypeSupportsTargetBox(managerRule.breakout, managerRule.type)
					? PriorityManagerScopeDisplay(m_pAPI, targetBox) : "N/A");
			pItem->setText(kPriorityManagerSourceColumn, source.label);
			pItem->setText(kPriorityManagerStateColumn, managerRule.disabled
				? COptionsWindow::tr("Disabled") : COptionsWindow::tr("Enabled"));
			pItem->setText(kPriorityManagerConflictColumn, QString());
			pItem->setData(kPriorityManagerRuleColumn, Qt::UserRole, extensions.baseRule);
			pItem->setData(kPriorityManagerRecursiveColumn, Qt::UserRole, recursive);
			pItem->setData(kPriorityManagerScopeColumn, Qt::UserRole, targetBox);
			pItem->setData(kPriorityManagerStateColumn, Qt::UserRole, managerRule.disabled);
			pItem->setData(kPriorityManagerConflictColumn, kPriorityManagerConflictRole, 0);
			pItem->setToolTip(kPriorityManagerPriorityColumn, managerRule.priority.isEmpty()
				? COptionsWindow::tr("No explicit priority.")
				: COptionsWindow::tr("Priority: %1").arg(managerRule.priority));
			pItem->setToolTip(kPriorityManagerRuleColumn, managerRule.originalRule);
			pItem->setToolTip(kPriorityManagerRecursiveColumn,
				recursive.isEmpty() ? QString() : BuildRecursiveTooltip(extensions.baseRule, recursive));
			pItem->setToolTip(kPriorityManagerScopeColumn, targetBox);
			pItem->setToolTip(kPriorityManagerSourceColumn, source.toolTip + "\n"
				+ COptionsWindow::tr("Double-click to open this source."));

			QColor rowColor = managerRule.breakout ? breakoutColor : forceColor;
			for (int column = 0; column < m_pTree->columnCount(); ++column) {
				pItem->setBackground(column, rowColor);
				if (managerRule.disabled)
					pItem->setForeground(column, disabledText);
			}

			Qt::ItemFlags flags = pItem->flags();
			flags &= ~Qt::ItemIsDropEnabled;
			if (!source.writable)
				flags &= ~Qt::ItemIsEditable;
			else
				flags |= Qt::ItemIsEditable;
			if (managerRule.priority.isEmpty() || !source.writable || !m_CanReorder)
				flags &= ~Qt::ItemIsDragEnabled;
			else
				flags |= Qt::ItemIsDragEnabled;
			pItem->setFlags(flags);
			m_pTree->addTopLevelItem(pItem);
		}

		m_pTree->setSortingEnabled(true);
		m_pTree->header()->setSortIndicator(m_SortColumn, m_SortOrder);
		m_pTree->sortItems(m_SortColumn, m_SortOrder);
		ApplyVisibility();
		RecomputeConflicts();
		UpdateReorderingState();
		m_pInfo->setText(m_InfoText + (m_CanReorder ? QString() : QString("\n\n")
			+ COptionsWindow::tr("Drag reordering is disabled while a read-only template has an explicit priority. You can still edit writable sources individually.")));
		UpdateApplyButton();
	}

	bool IsReorderView() const
	{
		return m_CanReorder && m_SortColumn == kPriorityManagerPriorityColumn
			&& m_SortOrder == Qt::AscendingOrder
			&& (!m_pSearch || m_pSearch->text().trimmed().isEmpty())
			&& IsAllSourceFilter() && IsAllSettingFilter();
	}

	void UpdateReorderingState()
	{
		if (!m_pTree)
			return;

		bool enabled = IsReorderView();
		m_pTree->SetReorderEnabled(enabled);
		for (int row = 0; row < m_pTree->topLevelItemCount(); ++row)
			UpdateRowFlags(m_pTree->topLevelItem(row));

		if (!m_pNumberingMode)
			return;
		m_pNumberingMode->setEnabled(enabled);
		if (!m_CanReorder)
			m_pNumberingMode->setToolTip(COptionsWindow::tr(
				"Numbering is unavailable while a read-only template has an explicit priority."));
		else if (!enabled)
			m_pNumberingMode->setToolTip(COptionsWindow::tr(
				"Select All sources and All settings, clear the search, and sort by Priority "
				"in ascending order to enable drag reordering and numbering."));
		else
			m_pNumberingMode->setToolTip(COptionsWindow::tr(
				"Keep existing priority numbers where possible and use empty gaps before renumbering. "
				"Compact mode assigns consecutive priorities starting at 1, with a maximum of 9999."));
	}

	bool MatchesSearch(QTreeWidgetItem* pItem) const
	{
		if (!pItem || !m_pSearch)
			return true;
		QString search = m_pSearch->text().trimmed();
		if (search.isEmpty())
			return true;
		for (int column = 0; column < m_pTree->columnCount(); ++column) {
			if (pItem->text(column).contains(search, Qt::CaseInsensitive))
				return true;
		}
		return pItem->data(kPriorityManagerPriorityColumn,
			kPriorityManagerOriginalRuleRole).toString().contains(search, Qt::CaseInsensitive);
	}

	QMap<int, QStringList> GetUsedRulePrioritySources(const QTreeWidgetItem* pExclude) const
	{
		QMap<int, QSet<QString> > sourceMap;
		int editedSourceIndex = pExclude
			? pExclude->data(kPriorityManagerPriorityColumn, kPriorityManagerSourceRole).toInt() : -1;
		const PriorityManagerSource* editedSource = editedSourceIndex >= 0
			&& editedSourceIndex < m_Sources.size() ? &m_Sources[editedSourceIndex] : NULL;
		bool editedTemplateContext = editedSource
			&& editedSource->kind == kPriorityManagerTemplateSource;
		bool templateContext = editedTemplateContext
			|| (!m_OriginTemplateKey.isEmpty()
				&& (!editedSource || editedSource->kind == kPriorityManagerGlobalSource));
		QString templateContextKey = editedTemplateContext ? editedSource->key : m_OriginTemplateKey;
		QString contextBoxKey = editedSource && editedSource->kind == kPriorityManagerBoxSource
			? editedSource->key : (editedTemplateContext ? QString() : m_OriginBoxKey);
		QSet<QString> relevantTemplates;
		auto addTemplates = [&relevantTemplates](const QSharedPointer<CSbieIni>& ini) {
			QMap<QString, QString> names;
			CSbieIni::CollectTemplateNames(ini, names);
			for (auto it = names.constBegin(); it != names.constEnd(); ++it)
				relevantTemplates.insert("template:" + it.value().toLower());
		};
		auto addApplicableTemplates = [&relevantTemplates](const QSharedPointer<CSbieIni>& ini) {
			if (ini.isNull())
				return;
			foreach (const QString& name, ini->GetTemplates()) {
				if (name.compare("GlobalSettings", Qt::CaseInsensitive) != 0)
					relevantTemplates.insert("template:" + name.toLower());
			}
		};
		if (m_pAPI && !templateContext)
			addTemplates(m_pAPI->GetGlobalSettings());
		if (m_pAPI && !contextBoxKey.isEmpty()) {
			CSandBoxPtr pContextBox = m_pAPI->GetBoxByName(contextBoxKey.mid(4));
			if (!pContextBox.isNull()) {
				QSharedPointer<CSbieIni> contextIni = pContextBox;
				addTemplates(contextIni);
			}
		}
		if (templateContext) {
			for (const PriorityManagerSource& source : m_Sources) {
				if (source.key.compare(templateContextKey, Qt::CaseInsensitive) == 0) {
					addApplicableTemplates(source.ini);
					break;
				}
			}
			relevantTemplates.insert(templateContextKey.toLower());
		}
		if (editedSource && editedSource->kind == kPriorityManagerTemplateSource)
			relevantTemplates.insert(editedSource->key.toLower());

		for (int row = 0; m_pTree && row < m_pTree->topLevelItemCount(); ++row) {
			QTreeWidgetItem* pItem = m_pTree->topLevelItem(row);
			if (!pItem || pItem == pExclude)
				continue;

			int type = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerTypeRole).toInt();
			if (type != COptionsWindow::eProcess && type != COptionsWindow::ePath
				&& type != COptionsWindow::eText && type != COptionsWindow::eParent)
				continue;

			int sourceIndex = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerSourceRole).toInt();
			if (sourceIndex < 0 || sourceIndex >= m_Sources.size())
				continue;
			const PriorityManagerSource& source = m_Sources[sourceIndex];
			bool includeSource = source.kind == kPriorityManagerGlobalSource;
			if (source.kind == kPriorityManagerTemplateSource)
				includeSource = relevantTemplates.contains(source.key.toLower());
			else if (source.kind == kPriorityManagerBoxSource) {
				bool currentBox = !contextBoxKey.isEmpty()
					&& source.key.compare(contextBoxKey, Qt::CaseInsensitive) == 0;
				if (currentBox)
					includeSource = true;
				else if (!templateContext && m_pAPI) {
					CSandBoxPtr pSourceBox = m_pAPI->GetBoxByName(source.ini->GetName());
					includeSource = !pSourceBox.isNull() && pSourceBox->IsEnabled();
				}
			}
			if (!includeSource)
				continue;

			int priority = -1;
			if (!ParseRulePriorityValue(
				pItem->data(kPriorityManagerPriorityColumn, Qt::UserRole).toString(), &priority))
				continue;
			QString sourceName = source.kind == kPriorityManagerGlobalSource
				? COptionsWindow::tr("global settings") : source.toolTip;
			QString ruleKind = pItem->data(kPriorityManagerPriorityColumn,
				kPriorityManagerBreakoutRole).toBool()
				? COptionsWindow::tr("breakout") : COptionsWindow::tr("force");
			QString state = pItem->data(kPriorityManagerStateColumn, Qt::UserRole).toBool()
				? COptionsWindow::tr("disabled") : COptionsWindow::tr("enabled");
			sourceMap[priority].insert(COptionsWindow::tr("%1, %2, %3")
				.arg(sourceName).arg(ruleKind).arg(state));
		}

		QMap<int, QStringList> result;
		for (auto it = sourceMap.constBegin(); it != sourceMap.constEnd(); ++it) {
			QStringList sources = it.value().values();
			sources.sort(Qt::CaseInsensitive);
			result.insert(it.key(), sources);
		}
		return result;
	}

	void ApplyVisibility()
	{
		if (!m_pTree)
			return;
		bool showDisabled = m_pShowDisabled->isChecked();
		bool showGlobal = m_pShowGlobal->isChecked();
		bool showTemplates = m_pShowTemplates->isChecked();
		bool showOtherBoxes = m_pShowOtherBoxes->isChecked();
		bool sourceFilterValid = IsSourceFilterSelectionValid();
		QString sourceFilterKey = sourceFilterValid
			? m_pSourceFilter->currentData().toString() : QString();
		QString settingFilter = m_pSettingFilter->currentData().toString();
		int disabledCount = 0;
		int globalCount = 0;
		int templateCount = 0;
		int visibleCount = 0;
		int totalCount = m_pTree->topLevelItemCount();
		for (int row = 0; row < totalCount; ++row) {
			QTreeWidgetItem* pItem = m_pTree->topLevelItem(row);
			int sourceKind = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerSourceKindRole).toInt();
			int sourceIndex = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerSourceRole).toInt();
			bool disabled = pItem->data(kPriorityManagerStateColumn, Qt::UserRole).toBool();
			bool sourceMatch = sourceFilterValid;
			if (sourceMatch && !sourceFilterKey.isEmpty())
				sourceMatch = sourceIndex >= 0 && sourceIndex < m_Sources.size()
					&& m_Sources[sourceIndex].key.compare(sourceFilterKey, Qt::CaseInsensitive) == 0;
			bool breakout = pItem->data(kPriorityManagerPriorityColumn,
				kPriorityManagerBreakoutRole).toBool();
			QString setting = PriorityManagerBaseSetting(pItem->data(
				kPriorityManagerPriorityColumn, kPriorityManagerSettingRole).toString());
			bool settingMatch = settingFilter == QStringLiteral("all")
				|| (settingFilter == QStringLiteral("force") && !breakout)
				|| (settingFilter == QStringLiteral("breakout") && breakout)
				|| setting.compare(settingFilter, Qt::CaseInsensitive) == 0;
			bool hidden = (disabled && !showDisabled)
				|| (sourceKind == kPriorityManagerGlobalSource && !showGlobal)
				|| (sourceKind == kPriorityManagerTemplateSource && !showTemplates)
				|| (!m_OriginBoxKey.isEmpty() && !showOtherBoxes
					&& sourceKind == kPriorityManagerBoxSource
					&& sourceIndex >= 0 && sourceIndex < m_Sources.size()
					&& m_Sources[sourceIndex].key.compare(m_OriginBoxKey, Qt::CaseInsensitive) != 0)
				|| !sourceMatch || !settingMatch
				|| !MatchesSearch(pItem);
			pItem->setHidden(hidden);
			if (!hidden) {
				++visibleCount;
				if (disabled)
					++disabledCount;
				if (sourceKind == kPriorityManagerGlobalSource)
					++globalCount;
				if (sourceKind == kPriorityManagerTemplateSource)
					++templateCount;
			}
		}
		m_pShowDisabled->setText(COptionsWindow::tr("Disabled (%1)").arg(disabledCount));
		m_pShowGlobal->setText(COptionsWindow::tr("Global (%1)").arg(globalCount));
		m_pShowTemplates->setText(COptionsWindow::tr("Templates (%1)").arg(templateCount));
		m_pRuleCount->setText(COptionsWindow::tr("Rules: %1 / %2").arg(visibleCount).arg(totalCount));
		UpdateReorderingState();
	}

	void RefreshView()
	{
		if (HasPendingMainChanges()) {
			QMessageBox::information(this, COptionsWindow::tr("Sandboxie-Plus"),
				COptionsWindow::tr("Apply or discard the pending Force/Breakout changes in an options window before refreshing priorities."));
			return;
		}
		if ((m_RevertApplied || HasPendingChanges()) && QMessageBox::question(this, COptionsWindow::tr("Sandboxie-Plus"),
			COptionsWindow::tr("Discard the unsaved priority-manager changes and reload the current Force/Breakout rules?"),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
			return;
		if (!RestorePendingRevert())
			return;

		RefreshSources();
		Populate();
		UpdateRevertButton();
		UpdateApplyButton();
	}

	void OpenSource(QTreeWidgetItem* pItem)
	{
		if (!pItem)
			return;
		int sourceIndex = pItem->data(kPriorityManagerPriorityColumn,
			kPriorityManagerSourceRole).toInt();
		if (sourceIndex < 0 || sourceIndex >= m_Sources.size())
			return;

		OpenSource(m_Sources[sourceIndex], pItem->data(kPriorityManagerPriorityColumn,
			kPriorityManagerBreakoutRole).toBool());
	}

	void OpenSource(const PriorityManagerSource& source, bool breakout)
	{
		if (!m_pAPI || !theGUI || source.ini.isNull())
			return;

		if (source.kind == kPriorityManagerBoxSource) {
			CSandBoxPtr pBox = m_pAPI->GetBoxByName(source.ini->GetName());
			if (!pBox.isNull() && theGUI->GetBoxView()) {
				theGUI->GetBoxView()->ShowOptions(pBox);
				QSharedPointer<CSandBoxPlus> pBoxPlus = pBox.objectCast<CSandBoxPlus>();
				if (!pBoxPlus.isNull() && pBoxPlus->m_pOptionsWnd)
					pBoxPlus->m_pOptionsWnd->showForceBreakoutTab(breakout);
			}
			return;
		}

		if (source.kind == kPriorityManagerTemplateSource) {
			COptionsWindow* pOptions = new COptionsWindow(source.ini, source.label, this);
			pOptions->setAttribute(Qt::WA_DeleteOnClose);
			pOptions->showForceBreakoutTab(breakout);
			return;
		}

		theGUI->OpenSettings("Edit");
	}

	void OpenTargetBoxOptions(const QString& targetBoxName, bool breakout)
	{
		if (!m_pAPI || !theGUI || targetBoxName.trimmed().isEmpty())
			return;

		CSandBoxPtr pBox = m_pAPI->GetBoxByName(targetBoxName.trimmed());
		if (pBox.isNull() || !theGUI->GetBoxView())
			return;

		theGUI->GetBoxView()->ShowOptions(pBox);
		QSharedPointer<CSandBoxPlus> pBoxPlus = pBox.objectCast<CSandBoxPlus>();
		if (!pBoxPlus.isNull() && pBoxPlus->m_pOptionsWnd)
			pBoxPlus->m_pOptionsWnd->showForceBreakoutTab(breakout);
	}

	void ShowContextMenu(const QPoint& pos)
	{
		QTreeWidgetItem* pItem = m_pTree->itemAt(pos);
		if (!pItem)
			return;
		if (!pItem->isSelected()) {
			m_pTree->clearSelection();
			pItem->setSelected(true);
		}
		int column = m_pTree->columnAt(pos.x());
		if (column < 0)
			column = kPriorityManagerPriorityColumn;
		m_pTree->setCurrentItem(pItem, column, QItemSelectionModel::NoUpdate);
		int clickedSourceIndex = pItem->data(kPriorityManagerPriorityColumn,
			kPriorityManagerSourceRole).toInt();
		PriorityManagerSource clickedSource;
		bool hasClickedSource = clickedSourceIndex >= 0 && clickedSourceIndex < m_Sources.size();
		if (hasClickedSource)
			clickedSource = m_Sources[clickedSourceIndex];
		bool clickedBreakout = pItem->data(kPriorityManagerPriorityColumn,
			kPriorityManagerBreakoutRole).toBool();
		QString targetBoxName = pItem->data(kPriorityManagerScopeColumn, Qt::UserRole).toString().trimmed();
		CSandBoxPtr pTargetBox = targetBoxName.isEmpty() || !m_pAPI
			? CSandBoxPtr() : m_pAPI->GetBoxByName(targetBoxName);

		bool hasWritable = false;
		bool hasPriority = false;
		bool hasRecursive = false;
		bool hasScope = false;
		for (QTreeWidgetItem* pSelected : m_pTree->selectedItems()) {
			int sourceIndex = pSelected->data(kPriorityManagerPriorityColumn,
				kPriorityManagerSourceRole).toInt();
			if (sourceIndex < 0 || sourceIndex >= m_Sources.size()
				|| !m_Sources[sourceIndex].writable)
				continue;

			hasWritable = true;
			int type = pSelected->data(kPriorityManagerPriorityColumn,
				kPriorityManagerTypeRole).toInt();
			bool breakout = pSelected->data(kPriorityManagerPriorityColumn,
				kPriorityManagerBreakoutRole).toBool();
			QString priority = pSelected->data(kPriorityManagerPriorityColumn,
				Qt::UserRole).toString().trimmed();
			QString recursive = pSelected->data(kPriorityManagerRecursiveColumn,
				Qt::UserRole).toString().trimmed();
			QString scope = pSelected->data(kPriorityManagerScopeColumn,
				Qt::UserRole).toString().trimmed();
			hasPriority = hasPriority || (RuleTypeSupportsPriority(type) && !priority.isEmpty());
			hasRecursive = hasRecursive || (RuleTypeSupportsRecursive(type) && !recursive.isEmpty());
			hasScope = hasScope || (RuleTypeSupportsTargetBox(breakout, type) && !scope.isEmpty());
		}

		QMenu menu(this);
		menu.setToolTipsVisible(true);
		QAction* pEnable = menu.addAction(COptionsWindow::tr("Enable"));
		QAction* pDisable = menu.addAction(COptionsWindow::tr("Disable"));
		pEnable->setEnabled(hasWritable);
		pDisable->setEnabled(hasWritable);
		pEnable->setToolTip(COptionsWindow::tr(
			"Enable all selected writable Force/Breakout rules."));
		pDisable->setToolTip(COptionsWindow::tr(
			"Disable all selected writable Force/Breakout rules."));
		connect(pEnable, &QAction::triggered, this,
			[this]() { ApplyContextAction(kPriorityManagerEnable); });
		connect(pDisable, &QAction::triggered, this,
			[this]() { ApplyContextAction(kPriorityManagerDisable); });

		menu.addSeparator();
		QAction* pUnsetPriority = menu.addAction(
			COptionsWindow::tr("Unset priority"));
		pUnsetPriority->setEnabled(hasPriority);
		pUnsetPriority->setToolTip(COptionsWindow::tr(
			"Remove the explicit priority from all selected writable rules."));
		connect(pUnsetPriority, &QAction::triggered, this,
			[this]() { ApplyContextAction(kPriorityManagerUnsetPriority); });

		QAction* pUnsetRecursive = menu.addAction(
			COptionsWindow::tr("Unset recursive"));
		pUnsetRecursive->setEnabled(hasWritable && hasRecursive);
		pUnsetRecursive->setToolTip(COptionsWindow::tr(
			"Remove the Recursive extension from all selected writable folder rules."));
		connect(pUnsetRecursive, &QAction::triggered, this,
			[this]() { ApplyContextAction(kPriorityManagerUnsetRecursive); });

		QAction* pUnsetScope = menu.addAction(
			COptionsWindow::tr("Unset Scope"));
		pUnsetScope->setEnabled(hasWritable && hasScope);
		pUnsetScope->setToolTip(COptionsWindow::tr(
			"Remove the TargetBox extension from all selected writable breakout rules."));
		connect(pUnsetScope, &QAction::triggered, this,
			[this]() { ApplyContextAction(kPriorityManagerUnsetScope); });

		QMenu* pPrioritize = menu.addMenu(COptionsWindow::tr("Prioritize over"));
		QAction* pPrioritizeForce = pPrioritize->addAction(COptionsWindow::tr("Force"));
		QAction* pPrioritizeBreakout = pPrioritize->addAction(COptionsWindow::tr("Breakout"));
		pPrioritizeForce->setEnabled(CanPrioritizeSelection(false));
		pPrioritizeBreakout->setEnabled(CanPrioritizeSelection(true));
		pPrioritize->setToolTip(COptionsWindow::tr(
			"Prioritize selected enabled box-local rules over the chosen rule class in the same box."));
		connect(pPrioritizeForce, &QAction::triggered, this,
			[this]() { PrioritizeSelection(false); });
		connect(pPrioritizeBreakout, &QAction::triggered, this,
			[this]() { PrioritizeSelection(true); });

		menu.addSeparator();
		QAction* pAddRemoveRule = menu.addAction(COptionsWindow::tr("Add/Remove rule"));
		pAddRemoveRule->setEnabled(hasClickedSource);
		pAddRemoveRule->setToolTip(COptionsWindow::tr(
			"Open the rule's owning source, as when double-clicking the Source cell."));
		connect(pAddRemoveRule, &QAction::triggered, this,
			[this, clickedSource, clickedBreakout, hasClickedSource]() {
				if (hasClickedSource)
					OpenSource(clickedSource, clickedBreakout);
			});

		if (column == kPriorityManagerScopeColumn) {
			QAction* pOpenSandbox = menu.addAction(COptionsWindow::tr("Open sandbox"));
			pOpenSandbox->setEnabled(!pTargetBox.isNull());
			pOpenSandbox->setToolTip(COptionsWindow::tr(
				"Open the TargetBox sandbox on the matching Force or Breakout rule page."));
			connect(pOpenSandbox, &QAction::triggered, this,
				[this, targetBoxName, clickedBreakout]() {
					OpenTargetBoxOptions(targetBoxName, clickedBreakout);
				});
		}

		menu.exec(m_pTree->viewport()->mapToGlobal(pos));
	}

	bool SourceExtensionsEnabledForBox(int sourceIndex, bool breakout) const
	{
		if (sourceIndex < 0 || sourceIndex >= m_Sources.size())
			return false;
		const PriorityManagerSource& source = m_Sources[sourceIndex];
		if (source.kind != kPriorityManagerBoxSource || !source.writable
			|| !source.key.startsWith("box:", Qt::CaseInsensitive))
			return false;
		QString boxScope = source.key.mid(4).toLower();
		const QSet<QString>& scopes = breakout ? source.breakoutScopes : source.forceScopes;
		const QSet<QString>& disabledScopes = breakout
			? source.breakoutExtensionDisabledScopes : source.forceExtensionDisabledScopes;
		return scopes.contains(boxScope) && !disabledScopes.contains(boxScope);
	}

	bool CanPrioritizeSelection(bool overBreakout) const
	{
		QVector<QTreeWidgetItem*> selected = m_pTree->selectedItems();
		for (QTreeWidgetItem* pItem : selected) {
			int sourceIndex = pItem->data(kPriorityManagerPriorityColumn,
				kPriorityManagerSourceRole).toInt();
			if (pItem->isHidden()
				|| !RuleTypeSupportsPriority(pItem->data(kPriorityManagerPriorityColumn,
					kPriorityManagerTypeRole).toInt())
				|| !SourceExtensionsEnabledForBox(sourceIndex,
					pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerBreakoutRole).toBool())
				|| !SourceExtensionsEnabledForBox(sourceIndex, overBreakout))
				continue;

			for (int row = 0; row < m_pTree->topLevelItemCount(); ++row) {
				QTreeWidgetItem* pTarget = m_pTree->topLevelItem(row);
				if ((selected.contains(pTarget) && !pTarget->isHidden())
					|| pTarget->data(kPriorityManagerPriorityColumn,
						kPriorityManagerSourceRole).toInt() != sourceIndex
					|| pTarget->data(kPriorityManagerPriorityColumn,
						kPriorityManagerBreakoutRole).toBool() != overBreakout
					|| !RuleTypeSupportsPriority(pTarget->data(kPriorityManagerPriorityColumn,
						kPriorityManagerTypeRole).toInt()))
					continue;
				return true;
			}
		}
		return false;
	}

	void PrioritizeSelection(bool overBreakout)
	{
		struct PriorityGroup
		{
			int priority = -1;
			QVector<QTreeWidgetItem*> items;
		};
		struct PlannedPriority
		{
			QTreeWidgetItem* item = NULL;
			int priority = -1;
		};

		QVector<QTreeWidgetItem*> selected = m_pTree->selectedItems();
		QVector<QTreeWidgetItem*> actionSelected;
		QMap<int, QVector<QTreeWidgetItem*> > selectedBySource;
		for (QTreeWidgetItem* pItem : selected) {
			int sourceIndex = pItem->data(kPriorityManagerPriorityColumn,
				kPriorityManagerSourceRole).toInt();
			if (pItem->isHidden()
				|| !RuleTypeSupportsPriority(pItem->data(kPriorityManagerPriorityColumn,
					kPriorityManagerTypeRole).toInt())
				|| !SourceExtensionsEnabledForBox(sourceIndex,
					pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerBreakoutRole).toBool())
				|| !SourceExtensionsEnabledForBox(sourceIndex, overBreakout))
				continue;
			actionSelected.append(pItem);
			selectedBySource[sourceIndex].append(pItem);
		}

		QVector<PlannedPriority> plan;
		for (auto selectedGroup = selectedBySource.constBegin();
			selectedGroup != selectedBySource.constEnd(); ++selectedGroup) {
			const int sourceIndex = selectedGroup.key();
			QVector<QTreeWidgetItem*> selectedItems = selectedGroup.value();
			QVector<QTreeWidgetItem*> targetItems;

			auto priorityValue = [](QTreeWidgetItem* pItem, bool* valid) {
				QString value = pItem->data(kPriorityManagerPriorityColumn, Qt::UserRole).toString().trimmed();
				if (value.isEmpty())
					return -1;
				qlonglong parsed = -1;
				if (!ParseRulePriorityValue(value, &parsed)) {
					*valid = false;
					return -1;
				}
				return static_cast<int>(parsed);
			};
			bool valuesValid = true;
			auto priorityLess = [&](QTreeWidgetItem* pLeft, QTreeWidgetItem* pRight) {
				int leftPriority = priorityValue(pLeft, &valuesValid);
				int rightPriority = priorityValue(pRight, &valuesValid);
				if (leftPriority < 0 || rightPriority < 0) {
					if ((leftPriority < 0) != (rightPriority < 0))
						return leftPriority >= 0;
				}
				else if (leftPriority != rightPriority)
					return leftPriority < rightPriority;
				return pLeft->data(kPriorityManagerPriorityColumn,
					kPriorityManagerSequenceRole).toInt()
					< pRight->data(kPriorityManagerPriorityColumn,
						kPriorityManagerSequenceRole).toInt();
			};
			std::stable_sort(selectedItems.begin(), selectedItems.end(), priorityLess);
			if (!valuesValid) {
				QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
					COptionsWindow::tr("A selected rule has an invalid priority. No priorities were changed."));
				return;
			}

			QMap<int, QVector<QTreeWidgetItem*> > unselectedByPriority;
			for (int row = 0; row < m_pTree->topLevelItemCount(); ++row) {
				QTreeWidgetItem* pItem = m_pTree->topLevelItem(row);
				if (pItem->data(kPriorityManagerPriorityColumn,
					kPriorityManagerSourceRole).toInt() != sourceIndex
					|| !RuleTypeSupportsPriority(pItem->data(kPriorityManagerPriorityColumn,
						kPriorityManagerTypeRole).toInt()))
					continue;

				if (!actionSelected.contains(pItem)
					&& pItem->data(kPriorityManagerPriorityColumn,
						kPriorityManagerBreakoutRole).toBool() == overBreakout)
					targetItems.append(pItem);

				if (actionSelected.contains(pItem))
					continue;
				int priority = priorityValue(pItem, &valuesValid);
				if (priority >= 0)
					unselectedByPriority[priority].append(pItem);
			}
			if (!valuesValid) {
				QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
					COptionsWindow::tr("A local rule has an invalid priority. No priorities were changed."));
				return;
			}
			if (targetItems.isEmpty())
				continue;

			bool allSelectedExplicit = true;
			int selectedMaximum = -1;
			for (QTreeWidgetItem* pItem : selectedItems) {
				int priority = priorityValue(pItem, &valuesValid);
				if (priority < 0)
					allSelectedExplicit = false;
				else
					selectedMaximum = qMax(selectedMaximum, priority);
			}
			int targetMinimum = static_cast<int>(kPriorityManagerMax) + 1;
			for (QTreeWidgetItem* pItem : targetItems) {
				int priority = priorityValue(pItem, &valuesValid);
				if (priority >= 0)
					targetMinimum = qMin(targetMinimum, priority);
			}
			if (allSelectedExplicit && selectedMaximum < targetMinimum)
				continue;

			QVector<PriorityGroup> priorityGroups;
			auto appendGroup = [](QVector<PriorityGroup>& groups, QTreeWidgetItem* pItem,
				int priority) {
				if (groups.isEmpty() || groups.last().priority != priority) {
					PriorityGroup group;
					group.priority = priority;
					groups.append(group);
				}
				groups.last().items.append(pItem);
			};
			int lastSelectedPriority = -1;
			for (QTreeWidgetItem* pItem : selectedItems) {
				int priority = priorityValue(pItem, &valuesValid);
				if (priority < 0) {
					int unsetPriority = lastSelectedPriority < 0 ? 0
						: lastSelectedPriority < kPriorityManagerMax
							? lastSelectedPriority + 1
							: static_cast<int>(kPriorityManagerMax) + 1;
					appendGroup(priorityGroups, pItem, unsetPriority);
				}
				else {
					appendGroup(priorityGroups, pItem, priority);
					lastSelectedPriority = priority;
				}
			}
			if (!valuesValid) {
				QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
					COptionsWindow::tr("A selected rule has an invalid priority. No priorities were changed."));
				return;
			}

			QVector<int> selectedPriorities;
			for (const PriorityGroup& group : priorityGroups)
				selectedPriorities.append(group.priority);
			QVector<int> assignedPriorities;
			int selectedUpper = qMin(static_cast<int>(kPriorityManagerMax), targetMinimum - 1);
			QSet<int> occupiedPriorities;
			for (auto it = unselectedByPriority.constBegin();
				it != unselectedByPriority.constEnd(); ++it)
				occupiedPriorities.insert(it.key());
			if (BuildPriorityAssignmentInAvailableSlots(selectedPriorities, 0,
				selectedUpper, occupiedPriorities, &assignedPriorities)) {
				for (int groupIndex = 0; groupIndex < priorityGroups.size(); ++groupIndex) {
					for (QTreeWidgetItem* pItem : priorityGroups[groupIndex].items) {
						int oldPriority = priorityValue(pItem, &valuesValid);
						if (oldPriority == assignedPriorities[groupIndex])
							continue;
						PlannedPriority change;
						change.item = pItem;
						change.priority = assignedPriorities[groupIndex];
						plan.append(change);
					}
				}
				continue;
			}

			if (targetMinimum > kPriorityManagerMax) {
				QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
					COptionsWindow::tr("The selected rules cannot be prioritized ahead of this class within the 0 to 9999 range. No priorities were changed."));
				return;
			}

			QVector<int> prefixPriorities;
			QVector<int> suffixPriorities;
			QSet<int> prefixPrioritySet;
			for (auto it = unselectedByPriority.constBegin();
				it != unselectedByPriority.constEnd(); ++it) {
				if (it.key() < targetMinimum) {
					prefixPriorities.append(it.key());
					prefixPrioritySet.insert(it.key());
				}
				else
					suffixPriorities.append(it.key());
			}

			int targetFloor = qMax(targetMinimum,
				prefixPriorities.size() + selectedPriorities.size());
			QVector<int> suffixAssignments;
			if (targetFloor > kPriorityManagerMax
				|| !BuildPriorityAssignmentInAvailableSlots(selectedPriorities, 0,
					targetFloor - 1, prefixPrioritySet, &assignedPriorities)
				|| !BuildPriorityAssignmentInRange(suffixPriorities, targetFloor,
					static_cast<int>(kPriorityManagerMax), &suffixAssignments)) {
				QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
					COptionsWindow::tr("The selected rules cannot be prioritized ahead of this class within the 0 to 9999 range. No priorities were changed."));
				return;
			}
			for (int groupIndex = 0; groupIndex < priorityGroups.size(); ++groupIndex) {
				for (QTreeWidgetItem* pItem : priorityGroups[groupIndex].items) {
					int oldPriority = priorityValue(pItem, &valuesValid);
					if (oldPriority == assignedPriorities[groupIndex])
						continue;
					PlannedPriority change;
					change.item = pItem;
					change.priority = assignedPriorities[groupIndex];
					plan.append(change);
				}
			}
			int suffixIndex = 0;
			for (auto it = unselectedByPriority.constBegin();
				it != unselectedByPriority.constEnd(); ++it) {
				if (it.key() < targetMinimum)
					continue;
				int assignedPriority = suffixAssignments[suffixIndex];
				for (QTreeWidgetItem* pItem : it.value()) {
					if (it.key() == assignedPriority)
						continue;
					PlannedPriority change;
					change.item = pItem;
					change.priority = assignedPriority;
					plan.append(change);
				}
				++suffixIndex;
			}
		}

		if (plan.isEmpty())
			return;
		{
			QSignalBlocker blocker(m_pTree);
			for (const PlannedPriority& change : plan) {
				change.item->setData(kPriorityManagerPriorityColumn, Qt::UserRole,
					QString::number(change.priority));
				UpdatePriorityItem(change.item);
			}
		}
		RecomputeConflicts();
		ApplyVisibility();
		UpdateApplyButton();
	}

	void ApplyContextAction(PriorityManagerContextAction action)
	{
		bool changed = false;
		{
			QSignalBlocker blocker(m_pTree);
			for (QTreeWidgetItem* pItem : m_pTree->selectedItems()) {
				int sourceIndex = pItem->data(kPriorityManagerPriorityColumn,
					kPriorityManagerSourceRole).toInt();
				if (sourceIndex < 0 || sourceIndex >= m_Sources.size()
					|| !m_Sources[sourceIndex].writable)
					continue;

				int type = pItem->data(kPriorityManagerPriorityColumn,
					kPriorityManagerTypeRole).toInt();
				bool breakout = pItem->data(kPriorityManagerPriorityColumn,
					kPriorityManagerBreakoutRole).toBool();
				switch (action) {
				case kPriorityManagerEnable:
				case kPriorityManagerDisable: {
					bool disabled = action == kPriorityManagerDisable;
					if (pItem->data(kPriorityManagerStateColumn, Qt::UserRole).toBool()
						== disabled)
						break;
					pItem->setData(kPriorityManagerStateColumn, Qt::UserRole, disabled);
					pItem->setText(kPriorityManagerStateColumn, disabled
						? COptionsWindow::tr("Disabled") : COptionsWindow::tr("Enabled"));
					UpdateRowAppearance(pItem);
					changed = true;
					break;
				}
				case kPriorityManagerUnsetPriority:
					if (!RuleTypeSupportsPriority(type)
						|| pItem->data(kPriorityManagerPriorityColumn,
							Qt::UserRole).toString().trimmed().isEmpty())
						break;
					pItem->setData(kPriorityManagerPriorityColumn, Qt::UserRole, QString());
					pItem->setText(kPriorityManagerPriorityColumn, "-");
					pItem->setToolTip(kPriorityManagerPriorityColumn,
						COptionsWindow::tr("No explicit priority."));
					UpdateRowFlags(pItem);
					changed = true;
					break;
				case kPriorityManagerUnsetRecursive:
					if (!RuleTypeSupportsRecursive(type)
						|| pItem->data(kPriorityManagerRecursiveColumn,
							Qt::UserRole).toString().trimmed().isEmpty())
						break;
					pItem->setData(kPriorityManagerRecursiveColumn, Qt::UserRole, QString());
					pItem->setText(kPriorityManagerRecursiveColumn,
						PriorityManagerRecursiveDisplay(QString()));
					pItem->setToolTip(kPriorityManagerRecursiveColumn, QString());
					changed = true;
					break;
				case kPriorityManagerUnsetScope:
					if (!RuleTypeSupportsTargetBox(breakout, type)
						|| pItem->data(kPriorityManagerScopeColumn,
							Qt::UserRole).toString().trimmed().isEmpty())
						break;
					pItem->setData(kPriorityManagerScopeColumn, Qt::UserRole, QString());
					pItem->setText(kPriorityManagerScopeColumn,
						PriorityManagerScopeDisplay(m_pAPI, QString()));
					pItem->setToolTip(kPriorityManagerScopeColumn, QString());
					changed = true;
					break;
				}
			}
		}
		if (!changed)
			return;
		RecomputeConflicts();
		ApplyVisibility();
		UpdateApplyButton();
	}

	void UpdateRowAppearance(QTreeWidgetItem* pItem)
	{
		if (!pItem)
			return;

		QColor baseColor = m_pTree->palette().color(QPalette::Base);
		QColor rowColor = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerBreakoutRole).toBool()
			? MixPriorityManagerColor(baseColor, QColor(235, 170, 65))
			: MixPriorityManagerColor(baseColor, QColor(75, 150, 220));
		QColor textColor = pItem->data(kPriorityManagerStateColumn, Qt::UserRole).toBool()
			? m_pTree->palette().color(QPalette::Disabled, QPalette::Text)
			: m_pTree->palette().color(QPalette::Text);
		int severity = pItem->data(kPriorityManagerConflictColumn, kPriorityManagerConflictRole).toInt();
		if (severity == 2)
			rowColor = MixPriorityManagerColor(rowColor, QColor(220, 70, 70));
		else if (severity == 1)
			rowColor = MixPriorityManagerColor(rowColor, QColor(230, 165, 50));
		for (int column = 0; column < m_pTree->columnCount(); ++column) {
			pItem->setBackground(column, rowColor);
			pItem->setForeground(column, textColor);
		}
		if (severity == 2)
			pItem->setForeground(kPriorityManagerConflictColumn, QColor(190, 30, 30));
		else if (severity == 1)
			pItem->setForeground(kPriorityManagerConflictColumn, QColor(190, 110, 20));
	}

	void UpdateRowFlags(QTreeWidgetItem* pItem)
	{
		if (!pItem)
			return;
		int sourceIndex = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerSourceRole).toInt();
		bool writable = sourceIndex >= 0 && sourceIndex < m_Sources.size() && m_Sources[sourceIndex].writable;
		QString priority = pItem->data(kPriorityManagerPriorityColumn, Qt::UserRole).toString().trimmed();
		Qt::ItemFlags flags = pItem->flags();
		flags &= ~(Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
		if (writable)
			flags |= Qt::ItemIsEditable;
		if (writable && m_pTree->IsReorderEnabled() && !priority.isEmpty()) {
			qlonglong parsed = -1;
			if (ParseRulePriorityValue(priority, &parsed))
				flags |= Qt::ItemIsDragEnabled;
		}
		if (pItem->flags() == flags)
			return;
		QSignalBlocker blocker(m_pTree);
		pItem->setFlags(flags);
	}

	void UpdatePriorityItem(QTreeWidgetItem* pItem)
	{
		if (!pItem)
			return;
		QSignalBlocker blocker(m_pTree);
		QString value = pItem->data(kPriorityManagerPriorityColumn, Qt::UserRole).toString().trimmed();
		pItem->setText(kPriorityManagerPriorityColumn, value.isEmpty() ? "-" : value);
		pItem->setToolTip(kPriorityManagerPriorityColumn, value.isEmpty()
			? COptionsWindow::tr("No explicit priority.")
			: COptionsWindow::tr("Priority: %1").arg(value));
		UpdateRowFlags(pItem);
	}

	void UpdateRuleItem(QTreeWidgetItem* pItem)
	{
		if (!pItem)
			return;
		QSignalBlocker blocker(m_pTree);
		QString value = pItem->data(kPriorityManagerRuleColumn, Qt::UserRole).toString().trimmed();
		if (value.isEmpty()) {
			RuleExtensionsUi original = ParseRuleExtensionsForUi(
				pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerOriginalRuleRole).toString());
			value = original.baseRule.trimmed();
		}
		pItem->setData(kPriorityManagerRuleColumn, Qt::UserRole, value);
		pItem->setText(kPriorityManagerRuleColumn, value);
		pItem->setToolTip(kPriorityManagerRuleColumn, BuildPriorityManagerRule(pItem));
		int type = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerTypeRole).toInt();
		bool breakout = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerBreakoutRole).toBool();
		if (breakout && (type == COptionsWindow::eProcess || type == COptionsWindow::ePath)
			&& IsBreakoutRuleBroad(value)) {
			QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
				COptionsWindow::tr("The breakout rule \"%1\" uses a broad wildcard pattern and may cause unintended processes to break out of the sandbox.")
					.arg(value));
		}
		QString recursive = pItem->data(kPriorityManagerRecursiveColumn, Qt::UserRole).toString().trimmed();
		pItem->setToolTip(kPriorityManagerRecursiveColumn,
			RuleTypeSupportsRecursive(type) && !recursive.isEmpty()
				? BuildRecursiveTooltip(value, recursive) : QString());
	}

	void UpdateRecursiveItem(QTreeWidgetItem* pItem)
	{
		if (!pItem)
			return;
		QSignalBlocker blocker(m_pTree);
		int type = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerTypeRole).toInt();
		QString value = pItem->data(kPriorityManagerRecursiveColumn, Qt::UserRole).toString().trimmed();
		if (RuleTypeSupportsRecursive(type) && !value.isEmpty()) {
			QString normalized = NormalizeRecursiveExtensionValue(value);
			if (!normalized.isEmpty())
				value = normalized;
		}
		pItem->setData(kPriorityManagerRecursiveColumn, Qt::UserRole, value);
		pItem->setText(kPriorityManagerRecursiveColumn, RuleTypeSupportsRecursive(type)
			? PriorityManagerRecursiveDisplay(value) : "N/A");
		pItem->setToolTip(kPriorityManagerRecursiveColumn, value.isEmpty()
			? QString() : BuildRecursiveTooltip(pItem->data(kPriorityManagerRuleColumn, Qt::UserRole).toString(), value));
	}

	void UpdateScopeItem(QTreeWidgetItem* pItem)
	{
		if (!pItem)
			return;
		QSignalBlocker blocker(m_pTree);
		int type = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerTypeRole).toInt();
		bool breakout = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerBreakoutRole).toBool();
		QString value = CanonicalizePriorityManagerTargetBox(m_pAPI,
			pItem->data(kPriorityManagerScopeColumn, Qt::UserRole).toString());
		pItem->setData(kPriorityManagerScopeColumn, Qt::UserRole, value);
		pItem->setText(kPriorityManagerScopeColumn,
			RuleTypeSupportsTargetBox(breakout, type) ? PriorityManagerScopeDisplay(m_pAPI, value) : "N/A");
		pItem->setToolTip(kPriorityManagerScopeColumn, value);
	}

	void UpdateStateItem(QTreeWidgetItem* pItem)
	{
		if (!pItem)
			return;
		QSignalBlocker blocker(m_pTree);
		bool disabled = pItem->data(kPriorityManagerStateColumn, Qt::UserRole).toBool();
		pItem->setText(kPriorityManagerStateColumn, disabled
			? COptionsWindow::tr("Disabled") : COptionsWindow::tr("Enabled"));
		UpdateRowAppearance(pItem);
		ApplyVisibility();
	}

	bool ValidatePriorities()
	{
		for (int row = 0; row < m_pTree->topLevelItemCount(); ++row) {
			QTreeWidgetItem* pItem = m_pTree->topLevelItem(row);
			QString rule = pItem->data(kPriorityManagerRuleColumn, Qt::UserRole).toString().trimmed();
			if (rule.isEmpty() || rule.contains('|')) {
				QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
					COptionsWindow::tr("The Rule value for this entry is invalid. Enter only the rule value without extension separators."));
				m_pTree->setCurrentItem(pItem);
				return false;
			}
			pItem->setData(kPriorityManagerRuleColumn, Qt::UserRole, rule);

			QString value = pItem->data(kPriorityManagerPriorityColumn, Qt::UserRole).toString().trimmed();
			if (!value.isEmpty()) {
				qlonglong priority = -1;
				if (!ParseRulePriorityValue(value, &priority)) {
					QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
						COptionsWindow::tr("The priority value for rule \"%1\" is invalid. Enter a number from 0 to %2 or clear the value.")
							.arg(pItem->text(kPriorityManagerRuleColumn)).arg(kPriorityManagerMax));
					m_pTree->setCurrentItem(pItem);
					return false;
				}
				value = QString::number(priority);
				pItem->setData(kPriorityManagerPriorityColumn, Qt::UserRole, value);
			}

			int type = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerTypeRole).toInt();
			if (RuleTypeSupportsRecursive(type)) {
				QString recursive = pItem->data(kPriorityManagerRecursiveColumn, Qt::UserRole).toString().trimmed();
				if (recursive == "-")
					recursive.clear();
				if (!recursive.isEmpty()) {
					QString normalized = NormalizeRecursiveExtensionValue(recursive);
					if (normalized.isEmpty()) {
						QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
							COptionsWindow::tr("The Recursive value for rule \"%1\" is invalid.").arg(pItem->text(kPriorityManagerRuleColumn)));
						m_pTree->setCurrentItem(pItem);
						return false;
					}
					recursive = normalized;
				}
				pItem->setData(kPriorityManagerRecursiveColumn, Qt::UserRole, recursive);
			}

			if (RuleTypeSupportsTargetBox(pItem->data(kPriorityManagerPriorityColumn,
				kPriorityManagerBreakoutRole).toBool(), type)) {
				QString targetBox = CanonicalizePriorityManagerTargetBox(m_pAPI,
					pItem->data(kPriorityManagerScopeColumn, Qt::UserRole).toString());
				pItem->setData(kPriorityManagerScopeColumn, Qt::UserRole, targetBox);
			}
		}
		{
			QSignalBlocker blocker(m_pTree);
			for (int row = 0; row < m_pTree->topLevelItemCount(); ++row) {
				QTreeWidgetItem* pItem = m_pTree->topLevelItem(row);
				UpdatePriorityItem(pItem);
				UpdateRecursiveItem(pItem);
				UpdateScopeItem(pItem);
				UpdateStateItem(pItem);
			}
		}
		return true;
	}

	bool AssignSequentialPriorities(const QVector<QTreeWidgetItem*>& items, int first)
	{
		if (first < 0 || first + items.size() - 1 > static_cast<int>(kPriorityManagerMax))
			return false;
		QSignalBlocker blocker(m_pTree);
		for (int index = 0; index < items.size(); ++index) {
			QString value = QString::number(first + index);
			items[index]->setData(kPriorityManagerPriorityColumn, Qt::UserRole, value);
			items[index]->setText(kPriorityManagerPriorityColumn, value);
			items[index]->setToolTip(kPriorityManagerPriorityColumn, COptionsWindow::tr("Priority: %1").arg(value));
		}
		return true;
	}

	bool ApplyKeepPriorities(const QVector<QTreeWidgetItem*>& items,
		const QVector<int>& oldPriorities)
	{
		if (items.size() != oldPriorities.size())
			return false;
		QVector<int> assigned;
		if (!BuildPriorityAssignment(oldPriorities, &assigned))
			return false;

		QSignalBlocker blocker(m_pTree);
		for (int index = 0; index < items.size(); ++index) {
			QString value = QString::number(assigned[index]);
			items[index]->setData(kPriorityManagerPriorityColumn, Qt::UserRole, value);
			items[index]->setText(kPriorityManagerPriorityColumn, value);
			items[index]->setToolTip(kPriorityManagerPriorityColumn, COptionsWindow::tr("Priority: %1").arg(value));
		}
		return true;
	}

	bool ApplyNumberingMode(int numberingMode)
	{
		if (!IsReorderView())
			return true;

		QVector<QTreeWidgetItem*> items;
		QVector<int> oldPriorities;
		for (int row = 0; row < m_pTree->topLevelItemCount(); ++row) {
			QTreeWidgetItem* pItem = m_pTree->topLevelItem(row);
			QString value = pItem->data(kPriorityManagerPriorityColumn, Qt::UserRole).toString().trimmed();
			if (value.isEmpty())
				continue;
			qlonglong priority = -1;
			if (!ParseRulePriorityValue(value, &priority))
				return true;
			items.append(pItem);
			oldPriorities.append(static_cast<int>(priority));
		}

		if (numberingMode == 1) {
			if (items.size() > static_cast<int>(kPriorityManagerMax)) {
				QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
					COptionsWindow::tr("There are more than 9999 explicitly prioritized rules. Remove some priorities before using compact numbering."));
				return false;
			}
			return AssignSequentialPriorities(items, 1);
		}

		if (items.size() > static_cast<int>(kPriorityManagerMax) + 1) {
			QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
				COptionsWindow::tr("There are too many explicitly prioritized rules to assign unique values from 0 to %1.").arg(kPriorityManagerMax));
			return false;
		}
		return ApplyKeepPriorities(items, oldPriorities);
	}

	bool PriorityManagerItemHasChanges(QTreeWidgetItem* pItem, QString* pNewRule = NULL,
		QString* pCurrentSetting = NULL) const
	{
		if (!pItem)
			return false;
		QString originalSetting = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerSettingRole).toString();
		QString originalRule = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerOriginalRuleRole).toString();
		bool disabled = pItem->data(kPriorityManagerStateColumn, Qt::UserRole).toBool();
		QString currentSetting = PriorityManagerSettingForState(originalSetting, disabled);
		QString newRule = BuildPriorityManagerRule(pItem);
		if (pNewRule)
			*pNewRule = newRule;
		if (pCurrentSetting)
			*pCurrentSetting = currentSetting;
		return newRule != originalRule || currentSetting.compare(originalSetting, Qt::CaseSensitive) != 0;
	}

	bool HasPendingChanges() const
	{
		for (int row = 0; row < m_pTree->topLevelItemCount(); ++row) {
			QTreeWidgetItem* pItem = m_pTree->topLevelItem(row);
			if (PriorityManagerItemHasChanges(pItem))
				return true;
		}
		return false;
	}

	bool HasPendingManagerChanges() const
	{
		return m_RevertApplied || HasPendingChanges();
	}

	QString BuildPriorityManagerRule(QTreeWidgetItem* pItem) const
	{
		QString originalRule = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerOriginalRuleRole).toString();
		int type = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerTypeRole).toInt();
		bool breakout = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerBreakoutRole).toBool();
		RuleExtensionsUi current;
		current.baseRule = pItem->data(kPriorityManagerRuleColumn, Qt::UserRole).toString().trimmed();
		if (current.baseRule.isEmpty())
			current.baseRule = ParseRuleExtensionsForUi(originalRule).baseRule.trimmed();
		if (current.baseRule.isEmpty())
			return originalRule;
		current.priority = pItem->data(kPriorityManagerPriorityColumn, Qt::UserRole).toString();
		current.recursive = pItem->data(kPriorityManagerRecursiveColumn, Qt::UserRole).toString();
		current.targetBox = CanonicalizePriorityManagerTargetBox(m_pAPI,
			pItem->data(kPriorityManagerScopeColumn, Qt::UserRole).toString());
		return BuildRuleWithEditedExtensions(originalRule, current, RuleTypeSupportsPriority(type),
			RuleTypeSupportsRecursive(type), RuleTypeSupportsTargetBox(breakout, type));
	}

	void RecomputeConflicts()
	{
		QMap<QString, QVector<QTreeWidgetItem*> > groups;
		for (int row = 0; row < m_pTree->topLevelItemCount(); ++row) {
			QTreeWidgetItem* pItem = m_pTree->topLevelItem(row);
			if (pItem->data(kPriorityManagerStateColumn, Qt::UserRole).toBool())
				continue;
			QString baseRule = pItem->data(kPriorityManagerRuleColumn, Qt::UserRole).toString().trimmed();
			if (baseRule.isEmpty())
				continue;
			QString key = baseRule.toCaseFolded();
			groups[key].append(pItem);
		}

		QMap<QTreeWidgetItem*, int> severities;
		QMap<QTreeWidgetItem*, QStringList> details;
		m_ConflictCount = 0;
		m_WarningCount = 0;
		for (int row = 0; row < m_pTree->topLevelItemCount(); ++row) {
			QTreeWidgetItem* pItem = m_pTree->topLevelItem(row);
			if (!pItem || pItem->data(kPriorityManagerStateColumn, Qt::UserRole).toBool())
				continue;

			int type = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerTypeRole).toInt();
			bool breakout = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerBreakoutRole).toBool();
			int sourceIndex = pItem->data(kPriorityManagerPriorityColumn,
				kPriorityManagerSourceRole).toInt();
			QString priority = pItem->data(kPriorityManagerPriorityColumn,
				Qt::UserRole).toString().trimmed();
			QString recursive = pItem->data(kPriorityManagerRecursiveColumn,
				Qt::UserRole).toString().trimmed();
			QString targetBox = pItem->data(kPriorityManagerScopeColumn, Qt::UserRole).toString();
			bool hasRuleExtension = !priority.isEmpty() || !recursive.isEmpty() || !targetBox.isEmpty();
			bool extensionsEnabled = false;
			if (hasRuleExtension && sourceIndex >= 0 && sourceIndex < m_Sources.size()) {
				const PriorityManagerSource& source = m_Sources[sourceIndex];
				const QSet<QString>& disabledScopes = breakout
					? source.breakoutExtensionDisabledScopes
					: source.forceExtensionDisabledScopes;
				extensionsEnabled = PriorityManagerSourceHasEnabledExtensions(source, breakout);
				if (!disabledScopes.isEmpty()) {
					++m_WarningCount;
					severities[pItem] = qMax(severities.value(pItem), 1);
					details[pItem].append(COptionsWindow::tr(
						"Rule extensions are disabled for at least one enabled box using this source; "
						"Priority, Recursive, and TargetBox are ignored there."));
				}
			}
			if (RuleTypeSupportsTargetBox(breakout, type) && !targetBox.isEmpty()
				&& extensionsEnabled
				&& !PriorityManagerTargetBoxExists(m_pAPI, targetBox)) {
				++m_WarningCount;
				severities[pItem] = qMax(severities.value(pItem), 1);
				details[pItem].append(COptionsWindow::tr(
					"TargetBox \"%1\" is not an enabled sandbox; targeted breakout routing cannot resolve it.")
					.arg(targetBox));
			}
		}
		for (auto group = groups.constBegin(); group != groups.constEnd(); ++group) {
			const QVector<QTreeWidgetItem*>& items = group.value();
			QVector<QTreeWidgetItem*> overlappingItems =
				PriorityManagerOverlappingItems(items, m_Sources);
			if (overlappingItems.isEmpty())
				continue;

			QMap<QString, QVector<QTreeWidgetItem*> > prioritized;
			QVector<QTreeWidgetItem*> unprioritized;
			QVector<QTreeWidgetItem*> prioritizedItems;
			for (QTreeWidgetItem* pItem : overlappingItems) {
				QString priority = pItem->data(kPriorityManagerPriorityColumn, Qt::UserRole).toString().trimmed();
				if (priority.isEmpty())
					unprioritized.append(pItem);
				else {
					prioritized[priority].append(pItem);
					prioritizedItems.append(pItem);
				}
			}

			for (auto priority = prioritized.constBegin(); priority != prioritized.constEnd(); ++priority) {
				QVector<QTreeWidgetItem*> overlappingPriorityItems =
					PriorityManagerOverlappingItems(priority.value(), m_Sources);
				if (overlappingPriorityItems.size() < 2)
					continue;
				++m_ConflictCount;
				QString detail = COptionsWindow::tr("Overlapping active rules have the same explicit priority: %1.")
					.arg(PriorityManagerConflictPrioritySources(overlappingPriorityItems, m_Sources));
				for (QTreeWidgetItem* pItem : overlappingPriorityItems) {
					severities[pItem] = qMax(severities.value(pItem), 2);
					details[pItem].append(detail);
				}
			}

			if (prioritized.size() > 1 && prioritizedItems.size() > 1) {
				++m_WarningCount;
				QString detail = COptionsWindow::tr("Overlapping active rules have different explicit priorities; the lowest number wins: %1.")
					.arg(PriorityManagerConflictPrioritySources(prioritizedItems, m_Sources));
				for (QTreeWidgetItem* pItem : overlappingItems) {
					severities[pItem] = qMax(severities.value(pItem), 1);
					details[pItem].append(detail);
				}
			}

			QVector<QTreeWidgetItem*> overlappingUnprioritizedItems =
				PriorityManagerOverlappingItems(unprioritized, m_Sources);
			if (overlappingUnprioritizedItems.size() > 1) {
				++m_ConflictCount;
				QString detail = COptionsWindow::tr("Overlapping active rules have no explicit priority; source/list order decides: %1.")
					.arg(PriorityManagerConflictPrioritySources(overlappingUnprioritizedItems, m_Sources));
				for (QTreeWidgetItem* pItem : overlappingUnprioritizedItems) {
					severities[pItem] = qMax(severities.value(pItem), 2);
					details[pItem].append(detail);
				}
			}

			QVector<QTreeWidgetItem*> mixedItems = PriorityManagerOverlappingItems(
				prioritizedItems, unprioritized, m_Sources);
			if (!mixedItems.isEmpty()) {
				++m_WarningCount;
				QString detail = COptionsWindow::tr("The active rule is both explicitly prioritized and unprioritized across overlapping sources; explicit priority wins: %1.")
					.arg(PriorityManagerConflictPrioritySources(mixedItems, m_Sources));
				for (QTreeWidgetItem* pItem : mixedItems) {
					severities[pItem] = qMax(severities.value(pItem), 1);
					details[pItem].append(detail);
				}
			}
		}

		QSignalBlocker blocker(m_pTree);
		for (int row = 0; row < m_pTree->topLevelItemCount(); ++row) {
			QTreeWidgetItem* pItem = m_pTree->topLevelItem(row);
			int severity = severities.value(pItem, 0);
			pItem->setData(kPriorityManagerConflictColumn, kPriorityManagerConflictRole, severity);
			pItem->setText(kPriorityManagerConflictColumn, severity == 2
				? COptionsWindow::tr("Conflict") : (severity == 1 ? COptionsWindow::tr("Warning") : QString()));
			pItem->setToolTip(kPriorityManagerConflictColumn, details.value(pItem).join("\n"));
			UpdateRowAppearance(pItem);
		}
		if (m_ConflictCount == 0 && m_WarningCount == 0)
			m_pConflictLabel->setText(COptionsWindow::tr(
				"No exact-base Force/Breakout conflicts detected. Disabled rules are excluded; wildcard and path-containment overlaps are not inferred."));
		else
			m_pConflictLabel->setText(COptionsWindow::tr("%1 conflict(s), %2 warning(s). Disabled rules are excluded from this check. Review the Conflicts column before saving.")
				.arg(m_ConflictCount).arg(m_WarningCount));
	}

	void WriteBackup(const QVector<PriorityManagerBackupEntry>& entries) const
	{
		if (!theConf)
			return;

		QJsonObject backup;
		backup.insert("version", 2);
		QJsonArray jsonEntries;
		for (const PriorityManagerBackupEntry& backupEntry : entries) {
			QJsonObject object;
			object.insert("source", backupEntry.source);
			object.insert("setting", backupEntry.setting);
			QJsonArray values;
			for (const QString& value : backupEntry.values)
				values.append(value);
			object.insert("values", values);
			if (backupEntry.hasAfterValues) {
				QJsonArray afterValues;
				for (const QString& value : backupEntry.afterValues)
					afterValues.append(value);
				object.insert("after", afterValues);
			}
			jsonEntries.append(object);
		}
		backup.insert("entries", jsonEntries);
		theConf->SetBlob(kPriorityManagerBackupKey,
			QJsonDocument(backup).toJson(QJsonDocument::Compact));
	}

	bool ReadBackup(QVector<PriorityManagerBackupEntry>& entries) const
	{
		entries.clear();
		if (!theConf)
			return false;
		QByteArray data = theConf->GetBlob(kPriorityManagerBackupKey);
		if (data.isEmpty())
			return false;
		QJsonParseError error;
		QJsonDocument document = QJsonDocument::fromJson(data, &error);
		if (error.error != QJsonParseError::NoError || !document.isObject())
			return false;
		QJsonArray jsonEntries = document.object().value("entries").toArray();
		for (const QJsonValue& jsonValue : jsonEntries) {
			QJsonObject object = jsonValue.toObject();
			QString source = object.value("source").toString().trimmed();
			QString setting = object.value("setting").toString().trimmed();
			QJsonValue valuesValue = object.value("values");
			if (source.isEmpty() || !IsPriorityManagerSetting(setting) || !valuesValue.isArray())
				return false;
			QJsonArray values = valuesValue.toArray();
			PriorityManagerBackupEntry entry;
			entry.source = source;
			entry.setting = setting;
			for (const QJsonValue& value : values)
				entry.values.append(value.toString());
			QJsonValue after = object.value("after");
			if (!after.isUndefined() && !after.isArray())
				return false;
			if (after.isArray()) {
				for (const QJsonValue& value : after.toArray())
					entry.afterValues.append(value.toString());
				entry.hasAfterValues = true;
			}
			entries.append(entry);
		}
		return !entries.isEmpty();
	}

	void UpdateRevertButton()
	{
		QVector<PriorityManagerBackupEntry> entries;
		m_pRevert->setEnabled(!m_RevertApplied && ReadBackup(entries));
		m_pRevert->setToolTip(COptionsWindow::tr("Restore the Force/Breakout priorities and rule values saved before the last manager save."));
	}

	void UpdateApplyButton()
	{
		if (m_pApply)
			m_pApply->setEnabled(HasPendingManagerChanges());
	}

	bool HasPendingMainChanges() const
	{
		return m_HasPendingChanges && m_HasPendingChanges();
	}

	bool RestorePriorityManagerEntries(const QVector<PriorityManagerBackupEntry>& entries,
		const QVector<PriorityManagerSource>& sources, const QString& errorText)
	{
		QMap<QString, int> sourceIndexes;
		for (int index = 0; index < sources.size(); ++index)
			sourceIndexes.insert(sources[index].ini->GetName().toLower(), index);

		QSet<int> changedSources;
		QVector<PriorityManagerBackupEntry> rollbackEntries;
		for (const PriorityManagerBackupEntry& entry : entries) {
			QString key = entry.source.toLower();
			if (!sourceIndexes.contains(key) || !sources[sourceIndexes[key]].writable) {
				QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
					COptionsWindow::tr("The saved source \"%1\" is no longer available or is read-only.").arg(entry.source));
				return false;
			}
			changedSources.insert(sourceIndexes[key]);
			PriorityManagerBackupEntry rollbackEntry = entry;
			rollbackEntry.values = sources[sourceIndexes[key]].ini->GetTextList(entry.setting, false);
			rollbackEntries.append(rollbackEntry);
		}

		for (int sourceIndex : changedSources)
			sources[sourceIndex].ini->SetRefreshOnChange(false);
		SB_STATUS status = SB_OK;
		for (int entryIndex = 0; entryIndex < entries.size(); ++entryIndex) {
			const PriorityManagerBackupEntry& entry = entries[entryIndex];
			int sourceIndex = sourceIndexes[entry.source.toLower()];
			status = sources[sourceIndex].ini->UpdateTextList(entry.setting, entry.values, false);
			if (!status) {
				for (int rollbackIndex = entryIndex; rollbackIndex >= 0; --rollbackIndex) {
					const PriorityManagerBackupEntry& rollbackEntry = rollbackEntries[rollbackIndex];
					int rollbackSourceIndex = sourceIndexes[rollbackEntry.source.toLower()];
					sources[rollbackSourceIndex].ini->RestoreTextList(rollbackEntry.setting, rollbackEntry.values);
				}
				break;
			}
		}
		for (int sourceIndex : changedSources)
			sources[sourceIndex].ini->SetRefreshOnChange(true);
		for (int sourceIndex : changedSources) {
			SB_STATUS commitStatus = CommitPriorityManagerSource(sources[sourceIndex]);
			if (status && !commitStatus)
				status = commitStatus;
		}

		if (!status) {
			QMessageBox::critical(this, COptionsWindow::tr("Sandboxie-Plus"),
				errorText.arg(CSbieAPI__FormatNtStatus(status.GetStatus())));
			return false;
		}
		return true;
	}

	bool RestorePendingRevert()
	{
		if (!m_RevertApplied)
			return true;
		if (HasPendingMainChanges()) {
			QMessageBox::information(this, COptionsWindow::tr("Sandboxie-Plus"),
				COptionsWindow::tr("Apply or discard the pending Force/Breakout changes in an options window before cancelling the priority revert."));
			return false;
		}

		QVector<PriorityManagerSource> sources = BuildPriorityManagerSources(m_pAPI);
		QMap<QString, int> sourceIndexes;
		for (int index = 0; index < sources.size(); ++index)
			sourceIndexes.insert(sources[index].ini->GetName().toLower(), index);
		QStringList changedEntries;
		for (const PriorityManagerBackupEntry& entry : m_RevertAppliedEntries) {
			QString key = entry.source.toLower();
			if (!sourceIndexes.contains(key) || !sources[sourceIndexes[key]].writable) {
				QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
					COptionsWindow::tr("The source \"%1\" used by the pending revert is no longer available or is read-only.").arg(entry.source));
				return false;
			}
			int sourceIndex = sourceIndexes[key];
			if (sources[sourceIndex].ini->GetTextList(entry.setting, false) != entry.values)
				changedEntries.append(entry.source + ":" + entry.setting);
		}
		if (!changedEntries.isEmpty()
			&& QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
				COptionsWindow::tr("Some sources changed after the revert (%1). Restore the values from before the revert anyway?")
					.arg(changedEntries.join(", ")), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
			return false;

		if (!RestorePriorityManagerEntries(m_RevertUndoEntries, sources,
			COptionsWindow::tr("Unable to undo the priority revert: %1.")))
			return false;

		m_RevertApplied = false;
		m_RevertUndoEntries.clear();
		m_RevertAppliedEntries.clear();
		if (m_RefreshMain)
			m_RefreshMain();
		return true;
	}

	void RevertLastSaved()
	{
		if (m_RevertApplied)
			return;
		if (HasPendingMainChanges()) {
			QMessageBox::information(this, COptionsWindow::tr("Sandboxie-Plus"),
				COptionsWindow::tr("Apply or discard the pending Force/Breakout changes in an options window before using the priority manager."));
			return;
		}

		QVector<PriorityManagerBackupEntry> entries;
		if (!ReadBackup(entries)) {
			UpdateRevertButton();
			return;
		}
		if (HasPendingChanges() && QMessageBox::question(this, COptionsWindow::tr("Sandboxie-Plus"),
			COptionsWindow::tr("Discard the unsaved priority-manager changes and restore the last saved values?"),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
			return;

		QVector<PriorityManagerSource> sources = BuildPriorityManagerSources(m_pAPI);
		QMap<QString, int> sourceIndexes;
		for (int index = 0; index < sources.size(); ++index)
			sourceIndexes.insert(sources[index].ini->GetName().toLower(), index);
		QVector<PriorityManagerBackupEntry> undoEntries;
		for (const PriorityManagerBackupEntry& entry : entries) {
			QString key = entry.source.toLower();
			if (!sourceIndexes.contains(key) || !sources[sourceIndexes[key]].writable) {
				QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
					COptionsWindow::tr("The saved source \"%1\" is no longer available or is read-only.").arg(entry.source));
				return;
			}
			PriorityManagerBackupEntry undoEntry = entry;
			undoEntry.values = sources[sourceIndexes[key]].ini->GetTextList(entry.setting, false);
			undoEntry.afterValues.clear();
			undoEntry.hasAfterValues = false;
			undoEntries.append(undoEntry);
		}
		QStringList changedEntries;
		for (const PriorityManagerBackupEntry& entry : entries) {
			if (!entry.hasAfterValues)
				continue;
			int sourceIndex = sourceIndexes[entry.source.toLower()];
			if (sources[sourceIndex].ini->GetTextList(entry.setting, false) != entry.afterValues)
				changedEntries.append(entry.source + ":" + entry.setting);
		}
		if (!changedEntries.isEmpty()
			&& QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
				COptionsWindow::tr("Some saved Force/Breakout sources changed after the last manager save (%1). Restore the older values anyway?")
					.arg(changedEntries.join(", ")), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
			return;

		if (!RestorePriorityManagerEntries(entries, sources,
			COptionsWindow::tr("Unable to restore the saved priority values: %1.")))
			return;

		m_RevertUndoEntries = undoEntries;
		m_RevertAppliedEntries = entries;
		m_RevertApplied = true;
		QString selectedSourceKey = SelectedSourceFilterKey();
		m_Sources = sources;
		PopulateSourceFilter(selectedSourceKey);
		Populate();
		UpdateRevertButton();
		if (m_RefreshMain)
			m_RefreshMain();
	}

	bool SaveChanges()
	{
		if (HasPendingMainChanges()) {
			QMessageBox::information(this, COptionsWindow::tr("Sandboxie-Plus"),
				COptionsWindow::tr("Apply or discard the pending Force/Breakout changes in an options window before saving priorities."));
			return false;
		}

		QVector<PriorityManagerItemChange> itemChanges;
		QMap<int, QMap<QString, bool> > affected;
		for (int row = 0; row < m_pTree->topLevelItemCount(); ++row) {
			QTreeWidgetItem* pItem = m_pTree->topLevelItem(row);
			PriorityManagerItemChange change;
			change.item = pItem;
			change.sourceIndex = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerSourceRole).toInt();
			change.originalSetting = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerSettingRole).toString();
			change.originalIndex = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerIndexRole).toInt();
			change.originalRule = pItem->data(kPriorityManagerPriorityColumn, kPriorityManagerOriginalRuleRole).toString();
			change.currentSetting = PriorityManagerSettingForState(change.originalSetting,
				pItem->data(kPriorityManagerStateColumn, Qt::UserRole).toBool());
			change.newRule = BuildPriorityManagerRule(pItem);
			if (change.sourceIndex < 0 || change.sourceIndex >= m_Sources.size() || change.originalSetting.isEmpty())
				continue;
			if (change.newRule != change.originalRule || change.currentSetting.compare(change.originalSetting, Qt::CaseSensitive) != 0) {
				if (!m_Sources[change.sourceIndex].writable) {
					QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
						COptionsWindow::tr("The selected template is read-only and cannot be changed."));
					return false;
				}
				affected[change.sourceIndex][change.originalSetting] = true;
				affected[change.sourceIndex][change.currentSetting] = true;
			}
			itemChanges.append(change);
		}

		if (affected.isEmpty())
			return true;

		QMap<int, QMap<QString, QStringList> > oldValues;
		for (auto source = affected.constBegin(); source != affected.constEnd(); ++source) {
			for (auto setting = source.value().constBegin(); setting != source.value().constEnd(); ++setting)
				oldValues[source.key()][setting.key()] = m_Sources[source.key()].ini->GetTextList(setting.key(), false);
		}

		for (const PriorityManagerItemChange& change : itemChanges) {
			if (!affected.value(change.sourceIndex).contains(change.originalSetting))
				continue;
			const QStringList& values = oldValues[change.sourceIndex][change.originalSetting];
			if (change.originalIndex < 0 || change.originalIndex >= values.size()
				|| values[change.originalIndex] != change.originalRule) {
				QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
					COptionsWindow::tr("The configuration changed while the priority manager was open. Close and reopen it before saving."));
				return false;
			}
		}

		QMap<int, QMap<QString, QMap<int, int> > > itemIndexes;
		for (int index = 0; index < itemChanges.size(); ++index) {
			const PriorityManagerItemChange& change = itemChanges[index];
			itemIndexes[change.sourceIndex][change.originalSetting][change.originalIndex] = index;
		}

		QMap<int, QMap<QString, QStringList> > updates;
		for (auto source = affected.constBegin(); source != affected.constEnd(); ++source) {
			for (auto setting = source.value().constBegin(); setting != source.value().constEnd(); ++setting) {
				QStringList rebuilt;
				const QStringList& values = oldValues[source.key()][setting.key()];
				const QMap<int, int>& sourceItems = itemIndexes[source.key()][setting.key()];
				for (int index = 0; index < values.size(); ++index) {
					auto itemIndex = sourceItems.constFind(index);
					if (itemIndex == sourceItems.constEnd()) {
						rebuilt.append(values[index]);
						continue;
					}
					const PriorityManagerItemChange& change = itemChanges[itemIndex.value()];
					if (change.currentSetting.compare(setting.key(), Qt::CaseSensitive) == 0)
						rebuilt.append(change.newRule);
				}
				for (const PriorityManagerItemChange& change : itemChanges) {
					if (change.sourceIndex == source.key()
						&& change.originalSetting.compare(setting.key(), Qt::CaseSensitive) != 0
						&& change.currentSetting.compare(setting.key(), Qt::CaseSensitive) == 0)
						rebuilt.append(change.newRule);
				}
				if (rebuilt != values)
					updates[source.key()][setting.key()] = rebuilt;
			}
		}
		if (updates.isEmpty())
			return true;

		QVector<PriorityManagerBackupEntry> backupEntries;
		for (auto source = updates.constBegin(); source != updates.constEnd(); ++source) {
			for (auto setting = source.value().constBegin(); setting != source.value().constEnd(); ++setting) {
				PriorityManagerBackupEntry entry;
				entry.source = m_Sources[source.key()].ini->GetName();
				entry.setting = setting.key();
				entry.values = oldValues[source.key()][setting.key()];
				backupEntries.append(entry);
			}
		}
		if (theConf)
			WriteBackup(backupEntries);

		QList<int> sourceIndexes = updates.keys();
		for (int sourceIndex : sourceIndexes)
			m_Sources[sourceIndex].ini->SetRefreshOnChange(false);

		SB_STATUS status = SB_OK;
		QVector<QPair<int, QString> > attemptedUpdates;
		for (int sourceIndex : sourceIndexes) {
			for (auto setting = updates[sourceIndex].constBegin(); setting != updates[sourceIndex].constEnd(); ++setting) {
				attemptedUpdates.append(qMakePair(sourceIndex, setting.key()));
				status = m_Sources[sourceIndex].ini->UpdateTextList(setting.key(), setting.value(), false);
				if (!status) {
					for (int rollbackIndex = attemptedUpdates.size() - 1; rollbackIndex >= 0; --rollbackIndex) {
						int rollbackSourceIndex = attemptedUpdates[rollbackIndex].first;
						const QString& rollbackSetting = attemptedUpdates[rollbackIndex].second;
						m_Sources[rollbackSourceIndex].ini->RestoreTextList(rollbackSetting, oldValues[rollbackSourceIndex][rollbackSetting]);
					}
					break;
				}
			}
			if (!status)
				break;
		}
		for (int sourceIndex : sourceIndexes)
			m_Sources[sourceIndex].ini->SetRefreshOnChange(true);
		SB_STATUS commitStatus = SB_OK;
		for (int sourceIndex : sourceIndexes) {
			SB_STATUS sourceStatus = CommitPriorityManagerSource(m_Sources[sourceIndex]);
			if (commitStatus && !sourceStatus)
				commitStatus = sourceStatus;
		}

		if (!status) {
			QMessageBox::critical(this, COptionsWindow::tr("Sandboxie-Plus"),
				COptionsWindow::tr("Unable to save the priority changes: %1. The changes were rolled back where possible; the saved backup can be used to verify or revert them.")
					.arg(CSbieAPI__FormatNtStatus(status.GetStatus())));
			return false;
		}
		if (!commitStatus) {
			QMessageBox::critical(this, COptionsWindow::tr("Sandboxie-Plus"),
				COptionsWindow::tr("Unable to commit the priority changes: %1. Your edits have been retained; some changes may already have been saved.")
					.arg(CSbieAPI__FormatNtStatus(commitStatus.GetStatus())));
			return false;
		}
		if (theConf) {
			for (PriorityManagerBackupEntry& entry : backupEntries) {
				for (int sourceIndex : sourceIndexes) {
					if (m_Sources[sourceIndex].ini->GetName().compare(entry.source, Qt::CaseInsensitive) != 0)
						continue;
					entry.afterValues = m_Sources[sourceIndex].ini->GetTextList(entry.setting, false);
					entry.hasAfterValues = true;
					break;
				}
			}
			WriteBackup(backupEntries);
		}
		UpdateRevertButton();
		return true;
	}

	bool ApplyChanges()
	{
		if (!ValidatePriorities())
			return false;
		if (m_pTree->HasOrderChanged() && !ApplyNumberingMode(m_NumberingMode))
			return false;
		RecomputeConflicts();
		if (HasPendingChanges() && (m_ConflictCount > 0 || m_WarningCount > 0)) {
			if (QMessageBox::warning(this, COptionsWindow::tr("Sandboxie-Plus"),
				COptionsWindow::tr("The priority view contains %1 conflict(s) and %2 warning(s). Continue saving anyway?")
					.arg(m_ConflictCount).arg(m_WarningCount),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
				return false;
		}
		bool hadPendingChanges = HasPendingChanges();
		if (!SaveChanges())
			return false;

		if (m_RevertApplied) {
			if (!hadPendingChanges && theConf)
				theConf->DelValue(kPriorityManagerBackupKey);
			m_RevertApplied = false;
			m_RevertUndoEntries.clear();
			m_RevertAppliedEntries.clear();
		}
		RefreshSources();
		Populate();
		UpdateRevertButton();
		UpdateApplyButton();
		if (m_RefreshMain)
			m_RefreshMain();
		return true;
	}

	void SaveAndAccept()
	{
		if (ApplyChanges())
			QDialog::accept();
	}

	CSbieAPI* m_pAPI;
	std::function<bool()> m_HasPendingChanges;
	std::function<void()> m_RefreshMain;
	CForceBreakoutPriorityTree* m_pTree = NULL;
	QVector<PriorityManagerSource> m_Sources;
	QLabel* m_pInfo = NULL;
	QLabel* m_pConflictLabel = NULL;
	QLabel* m_pRuleCount = NULL;
	QComboBox* m_pSourceFilter = NULL;
	QComboBox* m_pSettingFilter = NULL;
	QLineEdit* m_pSearch = NULL;
	QCheckBox* m_pShowDisabled = NULL;
	QCheckBox* m_pShowGlobal = NULL;
	QCheckBox* m_pShowTemplates = NULL;
	QCheckBox* m_pShowOtherBoxes = NULL;
	QComboBox* m_pNumberingMode = NULL;
	QPushButton* m_pRevert = NULL;
	QPushButton* m_pApply = NULL;
	QPushButton* m_pRefresh = NULL;
	QToolButton* m_pOptions = NULL;
	QVector<PriorityManagerBackupEntry> m_RevertUndoEntries;
	QVector<PriorityManagerBackupEntry> m_RevertAppliedEntries;
	QString m_InfoText;
	QString m_OriginBoxKey;
	QString m_OriginTemplateKey;
	int m_ConflictCount = 0;
	int m_WarningCount = 0;
	int m_NumberingMode = 0;
	int m_SortColumn = kPriorityManagerPriorityColumn;
	Qt::SortOrder m_SortOrder = Qt::AscendingOrder;
	bool m_CanReorder = true;
	bool m_RevertApplied = false;
};

QPointer<CForceBreakoutPriorityDialog> g_pForceBreakoutPriorityDialog;

}

void COptionsWindow::OnManageForceBreakoutPriority()
{
	if (m_pBox.isNull() || !m_pBox->GetAPI())
		return;

	CSbieAPI* pAPI = m_pBox->GetAPI();
	QString originBoxKey;
	QString originTemplateKey;
	QSharedPointer<CSandBoxPlus> pOriginBox = m_pBox.objectCast<CSandBoxPlus>();
	if (!m_Template && !pOriginBox.isNull())
		originBoxKey = "box:" + pOriginBox->GetName().trimmed().toLower();
	else if (m_Template) {
		QString templateName = m_pBox->GetName().mid(9).trimmed();
		if (!templateName.isEmpty())
			originTemplateKey = "template:" + templateName.toLower();
	}
	QPointer<COptionsWindow> pOwner = this;
	auto collectOptionsWindows = [pAPI, pOwner]() {
		QList<COptionsWindow*> options;
		auto addOptions = [&options](COptionsWindow* pOptions) {
			if (pOptions && !options.contains(pOptions))
				options.append(pOptions);
		};
		if (!pOwner.isNull()) {
			addOptions(pOwner.data());
			for (COptionsWindow* pOptions : pOwner->findChildren<COptionsWindow*>())
				addOptions(pOptions);
		}
		if (g_pForceBreakoutPriorityDialog) {
			for (COptionsWindow* pOptions : g_pForceBreakoutPriorityDialog->findChildren<COptionsWindow*>())
				addOptions(pOptions);
		}
		QMap<QString, CSandBoxPtr> allBoxes = pAPI->GetAllBoxes();
		for (auto it = allBoxes.constBegin(); it != allBoxes.constEnd(); ++it) {
			QSharedPointer<CSandBoxPlus> pBoxPlus = it.value().objectCast<CSandBoxPlus>();
			if (!pBoxPlus.isNull())
				addOptions(pBoxPlus->m_pOptionsWnd);
		}
		return options;
	};
	std::function<bool()> hasPendingChanges = [collectOptionsWindows]() {
		for (COptionsWindow* pOptions : collectOptionsWindows()) {
			if (pOptions->m_ForcedChanged)
				return true;
		}
		return false;
	};
	std::function<void()> refreshMain = [collectOptionsWindows]() {
		for (COptionsWindow* pOptions : collectOptionsWindows())
			pOptions->LoadForced();
	};
	if (hasPendingChanges()) {
		QMessageBox::information(this, tr("Sandboxie-Plus"),
			tr("Apply or discard the pending Force/Breakout changes in an options window before opening the priority manager."));
		return;
	}
	if (g_pForceBreakoutPriorityDialog) {
		g_pForceBreakoutPriorityDialog->SetCallbacks(hasPendingChanges, refreshMain);
		g_pForceBreakoutPriorityDialog->SetOriginContext(originBoxKey, originTemplateKey);
		g_pForceBreakoutPriorityDialog->show();
		g_pForceBreakoutPriorityDialog->raise();
		g_pForceBreakoutPriorityDialog->activateWindow();
		return;
	}

	CForceBreakoutPriorityDialog* pDialog = new CForceBreakoutPriorityDialog(pAPI, NULL,
		hasPendingChanges, refreshMain, originBoxKey, originTemplateKey);
	g_pForceBreakoutPriorityDialog = pDialog;
	pDialog->setAttribute(Qt::WA_DeleteOnClose);
	if (theGUI)
		connect(theGUI, SIGNAL(Closed()), pDialog, SLOT(close()));
	connect(pDialog, &QDialog::finished, [](int) {
		g_pForceBreakoutPriorityDialog = NULL;
	});
	pDialog->show();
	pDialog->raise();
	pDialog->activateWindow();
}
