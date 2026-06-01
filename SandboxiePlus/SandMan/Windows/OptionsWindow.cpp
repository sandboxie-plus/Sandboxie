#include "stdafx.h"
#include "OptionsWindow.h"
#include "EditorSettingsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"
#include "../Wizards/TemplateWizard.h"
#include "Helpers/TabOrder.h"
#include "../MiscHelpers/Common/CodeEdit.h"
#include "Helpers/IniHighlighter.h"
#include <climits>
#include <algorithm>
#include <QIntValidator>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStandardItemModel>

static bool ParseRecursiveUiSpec(const QString& rawValue, QString* pDepthValue, bool* pAnchorFromLast, bool* pHasExplicitAnchor)
{
	QString value = rawValue.trimmed();
	QString depthValue;
	bool anchorFromLast = true;
	bool hasExplicitAnchor = false;

	if (value.isEmpty() || value == "-")
		return false;

	int anchorPos = value.indexOf(';');
	if (anchorPos >= 0) {
		depthValue = value.left(anchorPos).trimmed();
		QString anchorValue = value.mid(anchorPos + 1).trimmed().toLower();
		if (depthValue.isEmpty())
			return false;
		if (anchorValue == "first") {
			anchorFromLast = false;
			hasExplicitAnchor = true;
		}
		else if (anchorValue == "last") {
			anchorFromLast = true;
			hasExplicitAnchor = true;
		}
		else {
			return false;
		}
	}
	else {
		depthValue = value;
	}

	QString lower = depthValue.toLower();
	QString normalizedDepth;
	if (lower == "*" || lower == "y" || lower == "yes" || lower == "true")
		normalizedDepth = "y";
	else if (lower == "n" || lower == "no" || lower == "false" || lower == "0")
		normalizedDepth = "n";
	else {
		int rangePos = depthValue.indexOf('-');
		if (rangePos > 0 && rangePos + 1 < depthValue.length()) {
			QString left = depthValue.left(rangePos).trimmed();
			QString right = depthValue.mid(rangePos + 1).trimmed();
			bool minOk = false;
			bool maxOk = false;
			int minDepth = left.toInt(&minOk);
			int maxDepth = right.toInt(&maxOk);
			if (!minOk || !maxOk || minDepth < 0 || maxDepth < minDepth)
				return false;
			normalizedDepth = QString::number(minDepth) + "-" + QString::number(maxDepth);
		}
		else {
			bool ok = false;
			int depth = depthValue.toInt(&ok);
			if (!ok || depth < 0)
				return false;
			normalizedDepth = QString::number(depth);
		}
	}

	if (pDepthValue)
		*pDepthValue = normalizedDepth;
	if (pAnchorFromLast)
		*pAnchorFromLast = anchorFromLast;
	if (pHasExplicitAnchor)
		*pHasExplicitAnchor = hasExplicitAnchor;
	return true;
}

static QString NormalizeRecursiveUiValue(const QString& rawValue)
{
	QString depthValue;
	bool anchorFromLast = true;
	bool hasExplicitAnchor = false;

	if (!ParseRecursiveUiSpec(rawValue, &depthValue, &anchorFromLast, &hasExplicitAnchor))
		return QString();

	return hasExplicitAnchor ? (depthValue + ";" + (anchorFromLast ? "last" : "first")) : depthValue;
}

static bool GetWildcardAnchorForRule(const QString& baseRule, bool anchorFromLast, QString* pAnchor, QChar* pWildcardChar = nullptr)
{
	QString rule = baseRule.trimmed();
	if (rule.isEmpty())
		return false;

	int lastSlash = -1;
	int anchorSlash = -1;
	int wildcardPos = -1;
	QChar wildcardChar;
	for (int i = 0; i < rule.length(); ++i) {
		QChar ch = rule[i];
		if (ch == '\\' || ch == '/') {
			lastSlash = i;
		}
		else if (ch == '*' || ch == '?') {
			wildcardPos = i;
			wildcardChar = ch;
			anchorSlash = lastSlash;
			if (!anchorFromLast)
				break;
		}
	}

	if (wildcardPos < 0 || anchorSlash < 0)
		return false;

	if (pAnchor)
		*pAnchor = rule.left(anchorSlash + 1).trimmed();
	if (pWildcardChar)
		*pWildcardChar = wildcardChar;

	return true;
}

static QString RecursiveDisplayTextForValue(const QString& value)
{
	QString normalized = NormalizeRecursiveUiValue(value);
	QString depthValue;
	bool anchorFromLast = true;
	bool hasExplicitAnchor = false;
	QString display;

	if (normalized.isEmpty() || !ParseRecursiveUiSpec(normalized, &depthValue, &anchorFromLast, &hasExplicitAnchor))
		return value;

	QString lower = depthValue.toLower();
	if (lower == "n")
		display = QObject::tr("no (0)");
	else if (lower == "y")
		display = QObject::tr("yes (unlimited)");
	else if (depthValue.contains('-'))
		display = QObject::tr("depth %1").arg(depthValue);
	else
		display = QObject::tr("depth %1").arg(depthValue);

	if (hasExplicitAnchor)
		display += QObject::tr(" (%1)").arg(anchorFromLast ? QObject::tr("last") : QObject::tr("first"));

	return display;
}

static QString BuildRecursiveOptionTooltip(const QString& optionValue, const QString& baseRule)
{
	QString normalized = NormalizeRecursiveUiValue(optionValue);
	QString depthValue;
	bool anchorFromLast = true;
	bool hasExplicitAnchor = false;
	QString tip;
	QString trimmedRule = baseRule.trimmed();

	if (optionValue.trimmed() == "-")
		return QObject::tr("No explicit recursion mode.");

	if (normalized.isEmpty() || !ParseRecursiveUiSpec(normalized, &depthValue, &anchorFromLast, &hasExplicitAnchor))
		return QString();

	if (depthValue == "n")
		tip = QObject::tr("No recursion, only the matched folder.");
	else if (depthValue == "y")
		tip = QObject::tr("Unlimited recursion from the wildcard anchor.");
	else if (depthValue.contains('-')) {
		QStringList parts = depthValue.split('-', Qt::KeepEmptyParts);
		if (parts.size() == 2)
			tip = QObject::tr("Include subfolder levels from %1 to %2 (inclusive) from the wildcard anchor.").arg(parts[0].trimmed(), parts[1].trimmed());
	}
	else
		tip = QObject::tr("Include up to %1 subfolder level(s) from the wildcard anchor.").arg(depthValue);

	QString anchor;
	QChar wildcardChar;
	bool hasWildcardAnchor = GetWildcardAnchorForRule(baseRule, anchorFromLast, &anchor, &wildcardChar);
	QString anchorMode = anchorFromLast ? QObject::tr("last") : QObject::tr("first");
	if (hasWildcardAnchor)
		tip += (tip.isEmpty() ? QString() : QString("\n")) + QObject::tr("Wildcard anchor (%1 wildcard): %2 (anchor folder depth is 0)").arg(anchorMode, anchor);
	else
		tip += (tip.isEmpty() ? QString() : QString("\n")) + QObject::tr("Wildcard anchor: not applicable (exact path, no '*' or '?').");

	if (hasExplicitAnchor)
		tip += QString("\n") + QObject::tr("Explicit anchor mode: %1").arg(anchorMode);

	if (hasWildcardAnchor)
		tip += QString("\n") + QObject::tr("Wildcard operator: %1").arg(QString(wildcardChar));

	if (trimmedRule.endsWith("\\*") || trimmedRule.endsWith("/*"))
		tip += (tip.isEmpty() ? QString() : QString("\n")) + QObject::tr("Rule ends with '\\*': matching starts beyond the anchor folder, so the anchor folder itself is not matched. This is legacy behavior.");

	return tip;
}


class NoEditDelegate : public QStyledItemDelegate {
public:
	NoEditDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
		return NULL;
	}
};

class QTreeWidgetHacker : public QTreeWidget
{
public:
	friend class ProgramsDelegate;
	//QModelIndex indexFromItem(const QTreeWidgetItem *item, int column = 0) const;
	//QTreeWidgetItem *itemFromIndex(const QModelIndex &index) const;
};


//////////////////////////////////////////////////////////////////////////
// ProgramsDelegate

class ProgramsDelegate : public QStyledItemDelegate {
public:
	ProgramsDelegate(COptionsWindow* pOptions, QTreeWidget* pTree, int Column, QObject* parent = 0) : QStyledItemDelegate(parent) {
		m_pOptions = pOptions; 
		m_pTree = pTree; 
		m_Column = (m_Group = (Column == -2)) ? -1 : Column;
	}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
		QTreeWidgetItem* pItem = ((QTreeWidgetHacker*)m_pTree)->itemFromIndex(index);
		if (!pItem->data(index.column(), Qt::UserRole).isValid())
			return NULL;

		if(m_Group && !pItem->parent()) // for groups use simple edit
			return QStyledItemDelegate::createEditor(parent, option, index);

		if (m_Column == -1 || pItem->data(m_Column, Qt::UserRole).toInt() == COptionsWindow::eProcess) {
			QComboBox* pBox = new QComboBox(parent);
			pBox->setEditable(true);
			foreach(const QString Group, m_pOptions->GetCurrentGroups()) {
				QString GroupName = Group.mid(1, Group.length() - 2);
				pBox->addItem(tr("Group: %1").arg(GroupName), Group);
			}
			foreach(const QString & Name, m_pOptions->GetPrograms())
				pBox->addItem(Name, Name);

			connect(pBox->lineEdit(), &QLineEdit::textEdited, [pBox](const QString& text){
				/*if (pBox->currentIndex() != -1) {
					int pos = pBox->lineEdit()->cursorPosition();
					pBox->setCurrentIndex(-1);
					pBox->setCurrentText(text);
					pBox->lineEdit()->setCursorPosition(pos);
				}*/
				pBox->setProperty("value", text);
			});
			connect(pBox->lineEdit(), &QLineEdit::returnPressed, [pBox](){
				/*if (pBox->currentIndex() != -1) {
					int pos = pBox->lineEdit()->cursorPosition();
					pBox->setCurrentIndex(-1);
					pBox->setCurrentText(text);
					pBox->lineEdit()->setCursorPosition(pos);
				}*/
				pBox->setProperty("value", pBox->lineEdit()->text());
			});

			connect(pBox, qOverload<int>(&QComboBox::currentIndexChanged), [pBox](int index){
				if (index != -1) {
					QString Program = pBox->itemData(index).toString();
					pBox->setProperty("value", Program);
					pBox->lineEdit()->setReadOnly(Program.left(1) == "<");
				}
			});

			return pBox;
		}
		else if (pItem->data(0, Qt::UserRole).toInt() == COptionsWindow::ePath)
			return QStyledItemDelegate::createEditor(parent, option, index);
		else
			return NULL;
	}

	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const {
		QComboBox* pBox = qobject_cast<QComboBox*>(editor);
		if (pBox) {
			QTreeWidgetItem* pItem = ((QTreeWidgetHacker*)m_pTree)->itemFromIndex(index);
			QString Program = pItem->data(index.column(), Qt::UserRole).toString();

			pBox->setProperty("value", Program);
			pBox->lineEdit()->setReadOnly(Program.left(1) == "<");

			int Index = pBox->findData(Program);
			pBox->setCurrentIndex(Index);
			if (Index == -1)
				pBox->setCurrentText(Program);
		}
		else
			QStyledItemDelegate::setEditorData(editor, index);
	}

	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
		QTreeWidgetItem* pItem = ((QTreeWidgetHacker*)m_pTree)->itemFromIndex(index);

		QComboBox* pBox = qobject_cast<QComboBox*>(editor);
		if (pBox) {
			
			QString Value = pBox->property("value").toString();
			bool prev = m_pTree->blockSignals(true);
			pItem->setText(index.column(), pBox->currentText());
			m_pTree->blockSignals(prev);
			//QString Text = pBox->currentText();
			//QVariant Data = pBox->currentData();
			pItem->setData(index.column(), Qt::UserRole, Value);
		}

		QLineEdit* pEdit = qobject_cast<QLineEdit*>(editor);
		if (pEdit) {
			bool prev = m_pTree->blockSignals(true);
			pItem->setText(index.column(), pEdit->text());
			m_pTree->blockSignals(prev);
			QString Value = pEdit->text();
			if (m_Group) Value = "<" + Value + ">";
			pItem->setData(index.column(), Qt::UserRole, Value);
		}
	}

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QSize size = QStyledItemDelegate::sizeHint(option, index);
		if(size.height() < 20) size.setHeight(20); // ensure enough room for the combo box
		return size;
	}

protected:
	COptionsWindow* m_pOptions;
	QTreeWidget* m_pTree;
	int m_Column;
	bool m_Group;
};

class RuleExtensionsDelegate : public QStyledItemDelegate {
public:
	RuleExtensionsDelegate(COptionsWindow* pOptions, QTreeWidget* pTree, bool bBreakoutTree, int column, QObject* parent = 0)
		: QStyledItemDelegate(parent)
	{
		m_pOptions = pOptions;
		m_pTree = pTree;
		m_BreakoutTree = bBreakoutTree;
		m_Column = column;
	}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
		Q_UNUSED(option);

		QTreeWidgetItem* pItem = ((QTreeWidgetHacker*)m_pTree)->itemFromIndex(index);
		if (!pItem)
			return NULL;

		if (!m_pOptions->IsRuleExtensionsEnabled())
			return NULL;

		int type = pItem->data(0, Qt::UserRole).toInt();
		if (type == COptionsWindow::eTemplate || !SupportsColumn(type))
			return NULL;

		auto connectEditable = [](QComboBox* combo) {
			if (!combo || !combo->lineEdit())
				return;

			combo->setInsertPolicy(QComboBox::NoInsert);

			connect(combo->lineEdit(), &QLineEdit::textEdited, [combo](const QString& text){
				combo->setProperty("value", text.trimmed());
			});
			connect(combo, &QComboBox::editTextChanged, [combo](const QString& text){
				combo->setProperty("value", text.trimmed());
			});
			connect(combo->lineEdit(), &QLineEdit::returnPressed, [combo](){
				combo->setProperty("value", combo->lineEdit()->text().trimmed());
			});
			connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), [combo](int idx){
				if (idx >= 0)
					combo->setProperty("value", combo->itemData(idx).toString().trimmed());
			});
		};

		auto setupWideDropdown = [](QComboBox* combo) {
			if (!combo)
				return;

			combo->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
			if (combo->view())
				combo->view()->setTextElideMode(Qt::ElideNone);

			QFontMetrics fm(combo->font());
			int maxTextWidth = 0;
			for (int i = 0; i < combo->count(); ++i)
				maxTextWidth = qMax(maxTextWidth, fm.horizontalAdvance(combo->itemText(i)));

			const int padding = fm.horizontalAdvance(QStringLiteral("WWWW"));
			const int viewPadding = fm.horizontalAdvance(QStringLiteral("WW"));
			const int minWidth = qMax(combo->minimumSizeHint().width(), maxTextWidth + padding);

			combo->setMinimumWidth(minWidth);
			if (combo->view())
				combo->view()->setMinimumWidth(minWidth + viewPadding);
		};

		if (m_Column == 2) {
			QComboBox* pBox = new QComboBox(parent);
			pBox->setEditable(true);
			pBox->addItem(tr("Not set"), "-1");
			pBox->setItemData(0, tr("no explicit priority"), Qt::ToolTipRole);
			for (int i = 0; i <= 99; ++i)
				pBox->addItem(QString::number(i), QString::number(i));

			QRegularExpression rx("^(?:-1|[0-9]+)$");
			pBox->lineEdit()->setValidator(new QRegularExpressionValidator(rx, pBox));

			QMap<int, QStringList> usedPrioritySources = m_pOptions->GetUsedRulePrioritySources(pItem);
			QList<int> customPriorityValues;
			for (auto it = usedPrioritySources.constBegin(); it != usedPrioritySources.constEnd(); ++it) {
				if (it.key() > 99)
					customPriorityValues.append(it.key());
			}
			std::sort(customPriorityValues.begin(), customPriorityValues.end());
			for (int value : customPriorityValues) {
				QString valueText = QString::number(value);
				if (pBox->findData(valueText) < 0)
					pBox->addItem(valueText, valueText);
			}

			QStandardItemModel* pModel = qobject_cast<QStandardItemModel*>(pBox->model());
			if (pModel) {
				for (int row = 0; row < pModel->rowCount(); ++row) {
					QStandardItem* pEntry = pModel->item(row);
					if (!pEntry)
						continue;

					bool ok = false;
					int value = pEntry->data(Qt::UserRole).toString().toInt(&ok);
					if (!ok || value < 0)
						continue;

					if (usedPrioritySources.contains(value)) {
						if (!pEntry->text().contains(tr("(in use)"), Qt::CaseInsensitive))
							pEntry->setText(tr("%1 (in use)").arg(pEntry->text()));
						pEntry->setEnabled(false);
						QStringList sources = usedPrioritySources.value(value);
						if (!sources.isEmpty())
							pEntry->setToolTip(tr("Used by:\n%1").arg(sources.join("\n")));
					}
				}
			}

			setupWideDropdown(pBox);
			connectEditable(pBox);
			return pBox;
		}

		if (m_Column == 3) {
			QComboBox* pBox = new QComboBox(parent);
			pBox->setEditable(true);
			setupWideDropdown(pBox);
			pBox->addItem(tr("Not set"), "-");
			pBox->setItemData(0, tr("unlimited"), Qt::ToolTipRole);
			pBox->addItem(RecursiveDisplayTextForValue("n"), "n");
			pBox->setItemData(1, tr("No recursion, only the matched folder."), Qt::ToolTipRole);
			pBox->addItem(RecursiveDisplayTextForValue("y"), "y");
			pBox->setItemData(2, tr("Unlimited recursion from the wildcard anchor."), Qt::ToolTipRole);
			for (int i = 1; i <= 9; ++i) {
				pBox->addItem(RecursiveDisplayTextForValue(QString::number(i)), QString::number(i));
				pBox->setItemData(pBox->count() - 1, tr("Include up to %1 subfolder level(s) from the wildcard anchor.").arg(i), Qt::ToolTipRole);
			}
			const QStringList rangePresets = { "1-2", "1-3", "1-5", "2-4", "2-2" };
			for (const QString& preset : rangePresets) {
				if (pBox->findData(preset) < 0)
					pBox->addItem(RecursiveDisplayTextForValue(preset), preset);
			}

			QSet<QString> usedRecursiveValues = m_pOptions->GetUsedRuleRecursiveValues(pItem);
			QStringList customRecursiveValues = usedRecursiveValues.values();
			std::sort(customRecursiveValues.begin(), customRecursiveValues.end(), [](const QString& left, const QString& right) {
				return left.compare(right, Qt::CaseInsensitive) < 0;
			});
			for (const QString& valueText : customRecursiveValues) {
				if (valueText.isEmpty())
					continue;
				if (pBox->findData(valueText) < 0)
					pBox->addItem(RecursiveDisplayTextForValue(valueText), valueText);
			}

			setupWideDropdown(pBox);
			QRegularExpression rx("^(?:-|[yYnN]|[0-9]+(?:\\s*-\\s*[0-9]+)?)(?:\\s*;\\s*(?:[Ff][Ii][Rr][Ss][Tt]|[Ll][Aa][Ss][Tt]))?$");
			pBox->lineEdit()->setValidator(new QRegularExpressionValidator(rx, pBox));
			connect(pBox, qOverload<int>(&QComboBox::currentIndexChanged), [pBox](int idx){
				if (idx < 0)
					return;
				QString raw = pBox->itemData(idx).toString().trimmed();
				if (!raw.isEmpty())
					pBox->setEditText(raw);
			});
			connectEditable(pBox);
			return pBox;
		}

		if (m_Column == 4) {
			QComboBox* pBox = new QComboBox(parent);
			pBox->setEditable(true);
			setupWideDropdown(pBox);
			pBox->addItem(tr("Not set"), "");
			pBox->setItemData(0, tr("unsandboxed"), Qt::ToolTipRole);
			QMap<QString, CSandBoxPtr> allBoxes = theAPI->GetAllBoxes();
			for (auto it = allBoxes.constBegin(); it != allBoxes.constEnd(); ++it) {
				if (!it.value() || !it.value()->IsEnabled())
					continue;
				QString boxName = it.value()->GetName();
				if (boxName.isEmpty())
					continue;
				pBox->addItem(boxName, boxName);
			}
			setupWideDropdown(pBox);
			connectEditable(pBox);
			return pBox;
		}

		return new QLineEdit(parent);
	}

	virtual void setEditorData(QWidget* editor, const QModelIndex& index) const {
		QComboBox* pBox = qobject_cast<QComboBox*>(editor);
		if (pBox) {
			QTreeWidgetItem* pItem = ((QTreeWidgetHacker*)m_pTree)->itemFromIndex(index);
			if (!pItem)
				return;

			QString value = pItem->data(index.column(), Qt::UserRole).toString().trimmed();
			if (m_Column == 3) {
				value = NormalizeRecursiveUiValue(value);
				if (value.isEmpty())
					value = "-";
			}

			if (m_Column == 3) {
				QString baseRule = pItem->data(1, Qt::UserRole).toString();
				for (int row = 0; row < pBox->count(); ++row) {
					QString optionValue = pBox->itemData(row).toString();
					QString tip = BuildRecursiveOptionTooltip(optionValue, baseRule);
					if (!tip.isEmpty())
						pBox->setItemData(row, tip, Qt::ToolTipRole);
				}
			}

			// If a persisted value is outside preset ranges, expose it in the dropdown list.
			if (m_Column == 2 || m_Column == 3) {
				if (m_Column == 3) {
					if (!value.isEmpty() && pBox->findData(value) < 0)
						pBox->addItem(RecursiveDisplayTextForValue(value), value);
				}
				else {
					bool ok = false;
					qlonglong customValue = value.toLongLong(&ok);
					if (ok && customValue >= 0 && pBox->findData(value) < 0)
						pBox->addItem(value, value);
				}
			}

			// Empty value for TargetBox is normal (no extension)
			pBox->setProperty("value", value);

			int idx = pBox->findData(value);
			pBox->setCurrentIndex(idx);
			if (m_Column == 2 || m_Column == 3)
				pBox->setEditText(value);
			else if (idx == -1)
				pBox->setCurrentText(value);
			return;
		}

		QLineEdit* pEdit = qobject_cast<QLineEdit*>(editor);
		if (!pEdit)
			return;

		QTreeWidgetItem* pItem = ((QTreeWidgetHacker*)m_pTree)->itemFromIndex(index);
		if (!pItem)
			return;

		pEdit->setText(pItem->data(index.column(), Qt::UserRole).toString());
	}

	virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
		Q_UNUSED(model);

		QTreeWidgetItem* pItem = ((QTreeWidgetHacker*)m_pTree)->itemFromIndex(index);
		QComboBox* pBox = qobject_cast<QComboBox*>(editor);
		if (pItem && pBox) {
			QString value = pBox->property("value").toString().trimmed();
			QString typedValue = pBox->currentText().trimmed();
			int idx = pBox->currentIndex();
			if (idx >= 0) {
				QString itemText = pBox->itemText(idx).trimmed();
				if (!typedValue.isEmpty() && typedValue.compare(itemText, Qt::CaseSensitive) != 0) {
					value = typedValue;
				}
				else {
					QVariant itemData = pBox->itemData(idx);
					QString itemValue = itemData.toString().trimmed();
					if (itemData.isValid() && (m_Column == 4 || !itemValue.isEmpty()))
						value = itemValue;
					else if (value.isEmpty())
						value = typedValue;
				}
			}
			else if (value.isEmpty())
				value = typedValue;

			if (m_Column == 3) {
				value = NormalizeRecursiveUiValue(value);
			}

			// Empty value already signals no TargetBox extension

			QString displayValue = value;
			if (m_Column == 3) {
				displayValue = RecursiveDisplayTextForValue(value);
			}

			bool prev = m_pTree->blockSignals(true);
			pItem->setText(index.column(), displayValue);
			m_pTree->blockSignals(prev);
			pItem->setData(index.column(), Qt::UserRole, value);

			if (m_Column == 2) {
				if (value.isEmpty() || value == "-1")
					pItem->setToolTip(index.column(), tr("No explicit priority."));
				else
					pItem->setToolTip(index.column(), tr("Priority: %1").arg(value));
			}
			else if (m_Column == 3) {
				QString baseRule = pItem->data(1, Qt::UserRole).toString();
				pItem->setToolTip(index.column(), BuildRecursiveOptionTooltip(value, baseRule));
			}
			else if (m_Column == 4) {
				pItem->setToolTip(index.column(), value);
			}
			return;
		}

		QLineEdit* pEdit = qobject_cast<QLineEdit*>(editor);
		if (!pItem || !pEdit)
			return;

		QString value = pEdit->text().trimmed();
		if (m_Column == 2 && value.isEmpty())
			value = "-1";
		bool prev = m_pTree->blockSignals(true);
		pItem->setText(index.column(), value);
		m_pTree->blockSignals(prev);
		pItem->setData(index.column(), Qt::UserRole, value);
	}

private:
	bool SupportsColumn(int type) const {
		if (m_Column == 2)
			return (type == COptionsWindow::eProcess || type == COptionsWindow::ePath || type == COptionsWindow::eText || type == COptionsWindow::eParent);

		if (m_Column == 3)
			return (type == COptionsWindow::ePath);

		if (m_Column == 4)
			return (m_BreakoutTree && (type == COptionsWindow::eProcess || type == COptionsWindow::ePath || type == COptionsWindow::eText));

		return false;
	}

	COptionsWindow* m_pOptions;
	QTreeWidget* m_pTree;
	bool m_BreakoutTree;
	int m_Column;
};


//////////////////////////////////////////////////////////////////////////
// COptionsWindow

COptionsWindow::COptionsWindow(const QSharedPointer<CSbieIni>& pBox, const QString& Name, QWidget *parent)
	: CConfigDialog(parent)
{
	m_pBox = pBox;

	m_Template = pBox->GetName().left(9).compare("Template_", Qt::CaseInsensitive) == 0;
	bool ReadOnly = /*pBox->GetAPI()->IsConfigLocked() ||*/ (m_Template && pBox->GetName().mid(9, 6).compare("Local_", Qt::CaseInsensitive) != 0);
	
	m_HoldChange = false;
	m_SkipSaveOnToggle = false;

	m_ImageSize = 2ull*1024*1024*1024;

	QSharedPointer<CSandBoxPlus> pBoxPlus = m_pBox.objectCast<CSandBoxPlus>();
	if (!pBoxPlus.isNull())
		m_Programs = pBoxPlus->GetRecentPrograms();
	m_Programs.insert("program.exe");

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	//flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	//flags &= ~Qt::WindowMinMaxButtonsHint;
	flags |= Qt::WindowMinimizeButtonHint;
	//flags &= ~Qt::WindowCloseButtonHint;
	setWindowFlags(flags);

	this->setWindowFlag(Qt::WindowStaysOnTopHint, theGUI->IsAlwaysOnTop());

	ui.setupUi(this);
	this->setWindowTitle(tr("Sandboxie Plus - '%1' Options").arg(QString(Name).replace("_", " ")));

	ui.tabs->setTabPosition(QTabWidget::West);

	ui.tabs->setCurrentIndex(0);
	ui.tabs->setTabIcon(0, CSandMan::GetIcon("Config"));
	ui.tabs->setTabIcon(1, CSandMan::GetIcon("Security"));
	ui.tabs->setTabIcon(2, CSandMan::GetIcon("Group"));
	ui.tabs->setTabIcon(3, CSandMan::GetIcon("Control"));
	ui.tabs->setTabIcon(4, CSandMan::GetIcon("Stop"));
	ui.tabs->setTabIcon(5, CSandMan::GetIcon("Start"));
	ui.tabs->setTabIcon(6, CSandMan::GetIcon("Ampel"));
	ui.tabs->setTabIcon(7, CSandMan::GetIcon("Network"));
	ui.tabs->setTabIcon(8, CSandMan::GetIcon("Recover"));
	ui.tabs->setTabIcon(9, CSandMan::GetIcon("Settings"));
	ui.tabs->setTabIcon(10, CSandMan::GetIcon("Advanced"));
	ui.tabs->setTabIcon(11, CSandMan::GetIcon("Compatibility"));
	ui.tabs->setTabIcon(12, CSandMan::GetIcon("Editor"));

	ui.tabsGeneral->setCurrentIndex(0);
	ui.tabsGeneral->setTabIcon(0, CSandMan::GetIcon("Box"));
	ui.tabsGeneral->setTabIcon(1, CSandMan::GetIcon("Folder"));
	ui.tabsGeneral->setTabIcon(2, CSandMan::GetIcon("Move"));
	ui.tabsGeneral->setTabIcon(3, CSandMan::GetIcon("NoAccess"));
	ui.tabsGeneral->setTabIcon(4, CSandMan::GetIcon("EFence"));
	ui.tabsGeneral->setTabIcon(5, CSandMan::GetIcon("Run"));

	ui.tabsSecurity->setCurrentIndex(0);
	ui.tabsSecurity->setTabIcon(0, CSandMan::GetIcon("Shield7"));
	ui.tabsSecurity->setTabIcon(1, CSandMan::GetIcon("Fence"));
	ui.tabsSecurity->setTabIcon(2, CSandMan::GetIcon("Shield15"));
	ui.tabsSecurity->setTabIcon(3, CSandMan::GetIcon("Job"));
	ui.tabsSecurity->setTabIcon(4, CSandMan::GetIcon("Shield12"));

	ui.tabsForce->setCurrentIndex(0);
	ui.tabsForce->setTabIcon(0, CSandMan::GetIcon("Force"));
	ui.tabsForce->setTabIcon(1, CSandMan::GetIcon("Breakout"));

	ui.tabsStop->setCurrentIndex(0);
	ui.tabsStop->setTabIcon(0, CSandMan::GetIcon("Fail"));
	ui.tabsStop->setTabIcon(1, CSandMan::GetIcon("Pass"));
	ui.tabsStop->setTabIcon(2, CSandMan::GetIcon("Policy"));
		
	ui.tabsInternet->setCurrentIndex(0);
	ui.tabsInternet->setTabIcon(0, CSandMan::GetIcon("EthSocket2"));
	ui.tabsInternet->setTabIcon(1, CSandMan::GetIcon("Wall"));
	ui.tabsInternet->setTabIcon(2, CSandMan::GetIcon("DNS"));
	ui.tabsInternet->setTabIcon(3, CSandMan::GetIcon("Proxy"));
	ui.tabsInternet->setTabIcon(4, CSandMan::GetIcon("Network3"));

	ui.tabsAccess->setCurrentIndex(0);
	ui.tabsAccess->setTabIcon(0, CSandMan::GetIcon("Folder"));
	ui.tabsAccess->setTabIcon(1, CSandMan::GetIcon("RegEdit"));
	ui.tabsAccess->setTabIcon(2, CSandMan::GetIcon("Port"));
	ui.tabsAccess->setTabIcon(3, CSandMan::GetIcon("Window"));
	ui.tabsAccess->setTabIcon(4, CSandMan::GetIcon("Objects"));
	//ui.tabsAccess->setTabIcon(0, CSandMan::GetIcon("Rules"));
	ui.tabsAccess->setTabIcon(5, CSandMan::GetIcon("Policy"));

	ui.tabsRecovery->setCurrentIndex(0);
	ui.tabsRecovery->setTabIcon(0, CSandMan::GetIcon("QuickRecovery"));
	ui.tabsRecovery->setTabIcon(1, CSandMan::GetIcon("ImmidiateRecovery"));

	ui.tabsOther->setCurrentIndex(0);
	ui.tabsOther->setTabIcon(0, CSandMan::GetIcon("Presets"));
	ui.tabsOther->setTabIcon(1, CSandMan::GetIcon("Dll"));

	ui.tabsAdvanced->setCurrentIndex(0);
	ui.tabsAdvanced->setTabIcon(0, CSandMan::GetIcon("Presets"));
	ui.tabsAdvanced->setTabIcon(1, CSandMan::GetIcon("Trigger"));
	ui.tabsAdvanced->setTabIcon(2, CSandMan::GetIcon("Shield2"));
	ui.tabsAdvanced->setTabIcon(3, CSandMan::GetIcon("Anon"));
	ui.tabsAdvanced->setTabIcon(4, CSandMan::GetIcon("Users"));
	ui.tabsAdvanced->setTabIcon(5, CSandMan::GetIcon("SetLogging"));
	ui.tabsAdvanced->setTabIcon(6, CSandMan::GetIcon("Bug"));

	ui.tabsTemplates->setCurrentIndex(0);
	ui.tabsTemplates->setTabIcon(0, CSandMan::GetIcon("Template"));
	ui.tabsTemplates->setTabIcon(1, CSandMan::GetIcon("Explore"));
	ui.tabsTemplates->setTabIcon(2, CSandMan::GetIcon("Accessibility"));


	int iViewMode = theConf->GetInt("Options/ViewMode", 1);
	int iOptionLayout = theConf->GetInt("Options/NewConfigLayout", 2);
	if (iOptionLayout == 2)
		iOptionLayout = iViewMode != 2 ? 1 : 0;

	if ((QGuiApplication::queryKeyboardModifiers() & Qt::AltModifier) != 0)
		iOptionLayout = !iOptionLayout;

	QWidget* pDummy = new QWidget(this);
	pDummy->setVisible(false);

	// merge recovery tabs
	QWidget* pWidget3 = new QWidget();
	pWidget3->setLayout(ui.gridLayout_10);
	ui.gridLayout_24->addWidget(pWidget3, 1, 0);
	QWidget* pWidget4 = new QWidget();
	pWidget4->setLayout(ui.gridLayout_56);
	ui.gridLayout_24->addWidget(pWidget4, 2, 0);
	delete ui.tabsRecovery;
	ui.gridLayout_24->setContentsMargins(0, 0, 0, 0);

	// collect file options on a new files tab

	QWidget* pWidget = new QWidget();
	QGridLayout* pLayout = new QGridLayout(pWidget);

	QTabWidget* pTabWidget = new QTabWidget();
	pLayout->addWidget(pTabWidget, 0, 0);
	ui.tabs->insertTab(1, pWidget, tr("File Options"));
	ui.tabs->setTabIcon(1, CSandMan::GetIcon("Folder"));

	pTabWidget->addTab(ui.tabsGeneral->widget(1), ui.tabsGeneral->tabText(1));
	pTabWidget->setTabIcon(0, CSandMan::GetIcon("Files"));
	pTabWidget->addTab(ui.tabsGeneral->widget(1), ui.tabsGeneral->tabText(1));
	pTabWidget->setTabIcon(1, CSandMan::GetIcon("Move"));
	pTabWidget->addTab(ui.tabs->widget(9), ui.tabs->tabText(9));
	pTabWidget->setTabIcon(2, CSandMan::GetIcon("Recover"));
	//

	// re structure the UI a bit
	if (iOptionLayout == 1)
	{
		// merge stop tabs
		QWidget* pWidget1 = new QWidget();
		pWidget1->setLayout(ui.gridLayout_57);
		ui.gridLayout_17->addWidget(pWidget1, 1, 0);
		QWidget* pWidget2 = new QWidget();
		pWidget2->setLayout(ui.gridLayout_61);
		ui.gridLayout_17->addWidget(pWidget2, 2, 0);
		QWidget* pWidget3 = new QWidget();
		pWidget3->setLayout(ui.gridLayout_82);
		ui.gridLayout_82->setContentsMargins(3, 3, 3, 3);
		ui.verticalSpacer_40->changeSize(0, 0);
		ui.lblStopOpt->setVisible(false);
		ui.lblStopOpt->setProperty("hidden", true);
		ui.gridLayout_17->addWidget(pWidget3, 3, 0);
		delete ui.tabsStop;
		ui.gridLayout_17->setContentsMargins(0, 0, 0, 0);

		// move stop and restrictions to program tab
		ui.tabsForce->addTab(ui.tabs->widget(5), ui.tabs->tabText(5));
		ui.tabsForce->setTabIcon(2, CSandMan::GetIcon("Stop"));
		ui.tabsForce->addTab(ui.tabs->widget(5), ui.tabs->tabText(5));
		ui.tabsForce->setTabIcon(3, CSandMan::GetIcon("Start"));
		ui.gridLayout_19->setContentsMargins(3, 6, 3, 3);

		// move grouping to program tab
		ui.tabsForce->insertTab(0, ui.tabGroups, tr("Grouping"));
		ui.tabsForce->setTabIcon(0, CSandMan::GetIcon("Group"));
		ui.tabsForce->setCurrentIndex(0);
		ui.gridLayout_18->setContentsMargins(3, 6, 3, 3);
	}

	if (iViewMode != 1 && (QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier) == 0)
	{
		if (iOptionLayout == 1) {
			//ui.tabs->removeTab(7); // ini edit
			ui.tabAdvanced->setParent(pDummy); //ui.tabs->removeTab(5); // advanced
			//ui.tabsForce->removeTab(2); // breakout
		}
		else {
			//ui.tabs->removeTab(11); // ini edit
			ui.tabAdvanced->setParent(pDummy); //ui.tabs->removeTab(9); // advanced
			//ui.tabsForce->removeTab(1); // breakout
		}
		ui.tabPrivileges->setParent(pDummy); //ui.tabsSecurity->removeTab(3); // advanced security
		ui.tabIsolation->setParent(pDummy); //ui.tabsSecurity->removeTab(1); // security isolation
		//ui.tabsAccess->removeTab(5); // policy
		ui.treeOptions = NULL;
	}

	foreach(QTreeWidget* pTree, this->findChildren<QTreeWidget*>()) {
		if (pTree == ui.treeFolders) continue;
		pTree->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
		pTree->setMinimumHeight(50);
	}

	int size = 16.0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	size *= (QApplication::desktop()->logicalDpiX() / 96.0); // todo Qt6
#endif
	AddIconToLabel(ui.lblAppearance, CSandMan::GetIcon("Design").pixmap(size,size));
	AddIconToLabel(ui.lblBoxType, CSandMan::GetIcon("Maintenance").pixmap(size,size));
	AddIconToLabel(ui.lblNotes, CSandMan::GetIcon("EditIni").pixmap(size,size));
	AddIconToLabel(ui.lblStructure, CSandMan::GetIcon("Structure").pixmap(size,size));
	AddIconToLabel(ui.lblMigration, CSandMan::GetIcon("Move").pixmap(size,size));
	AddIconToLabel(ui.lblDelete, CSandMan::GetIcon("Erase").pixmap(size,size));
	AddIconToLabel(ui.lblRawDisk, CSandMan::GetIcon("Disk").pixmap(size,size));
	AddIconToLabel(ui.lblJob, CSandMan::GetIcon("Job3").pixmap(size,size));
	AddIconToLabel(ui.lblLimit, CSandMan::GetIcon("Job2").pixmap(size,size));
	AddIconToLabel(ui.lblSecurity, CSandMan::GetIcon("Shield5").pixmap(size,size));
	AddIconToLabel(ui.lblElevation, CSandMan::GetIcon("Shield9").pixmap(size,size));
	AddIconToLabel(ui.lblACLs, CSandMan::GetIcon("Ampel").pixmap(size,size));
	AddIconToLabel(ui.lblBoxProtection, CSandMan::GetIcon("BoxConfig").pixmap(size,size));
	AddIconToLabel(ui.lblNetwork, CSandMan::GetIcon("Network").pixmap(size,size));
	AddIconToLabel(ui.lblBind, CSandMan::GetIcon("EthSocket2").pixmap(size,size));
	AddIconToLabel(ui.lblPrinting, CSandMan::GetIcon("Printer").pixmap(size,size));
	AddIconToLabel(ui.lblOther, CSandMan::GetIcon("NoAccess").pixmap(size,size));

	AddIconToLabel(ui.lblStopOpt, CSandMan::GetIcon("Stop").pixmap(size,size));

	AddIconToLabel(ui.lblPorts, CSandMan::GetIcon("Port").pixmap(size,size));

	AddIconToLabel(ui.lblMode, CSandMan::GetIcon("Anon").pixmap(size,size));
	AddIconToLabel(ui.lblPolicy, CSandMan::GetIcon("Policy").pixmap(size,size));

	AddIconToLabel(ui.lblCompatibility, CSandMan::GetIcon("Compatibility").pixmap(size,size));
	//AddIconToLabel(ui.lblComRpc, CSandMan::GetIcon("Objects").pixmap(size,size));

	AddIconToLabel(ui.lblPrivilege, CSandMan::GetIcon("Token").pixmap(size,size));
	AddIconToLabel(ui.lblToken, CSandMan::GetIcon("Sandbox").pixmap(size,size));
	AddIconToLabel(ui.lblIsolation, CSandMan::GetIcon("Fence").pixmap(size,size));
	AddIconToLabel(ui.lblDesktop, CSandMan::GetIcon("Monitor").pixmap(size,size));
	AddIconToLabel(ui.lblAccess, CSandMan::GetIcon("NoAccess").pixmap(size,size));
	AddIconToLabel(ui.lblProtection, CSandMan::GetIcon("EFence").pixmap(size,size));

	AddIconToLabel(ui.lblPrivacyProtection, CSandMan::GetIcon("Anon").pixmap(size,size));
	AddIconToLabel(ui.lblProcessHiding, CSandMan::GetIcon("Cmd").pixmap(size,size));

	AddIconToLabel(ui.lblMonitor, CSandMan::GetIcon("Monitor").pixmap(size,size));
	AddIconToLabel(ui.lblTracing, CSandMan::GetIcon("SetLogging").pixmap(size,size));


	if (theConf->GetBool("Options/AltRowColors", false)) {
		foreach(QTreeWidget* pTree, this->findChildren<QTreeWidget*>()) 
			pTree->setAlternatingRowColors(true);
	}

	// Initialize validation flag from config, fallback to checkbox if not set
	bool defaultValidation = theConf->GetBool("Options/ValidateIniKeys", ui.chkValidateIniKeys->isChecked());
	ui.chkValidateIniKeys->setChecked(defaultValidation);
	m_IniValidationEnabled = defaultValidation;

	int defaultTooltip = theConf->GetInt("Options/EnableIniTooltips", static_cast<int>(CIniHighlighter::GetTooltipMode()));
	ui.chkEnableTooltips->setTristate(true); // Enable tri-state
	ui.chkEnableTooltips->setCheckState(static_cast<Qt::CheckState>(defaultTooltip));
	CIniHighlighter::SetTooltipMode(defaultTooltip); // Initialize the mode

	LoadCompletionConsent();
	int defaultAutoCompletion = theConf->GetInt("Options/EnableAutoCompletion", static_cast<int>(CCodeEdit::GetAutoCompletionMode()));
	if (m_AutoCompletionConsent) { // Consented
		ui.chkEnableAutoCompletion->setTristate(true); // Enable tri-state
		ui.chkEnableAutoCompletion->setCheckState(static_cast<Qt::CheckState>(defaultAutoCompletion));
		CCodeEdit::SetAutoCompletionMode(defaultAutoCompletion); // Initialize the mode
	}
	else {
		CCodeEdit::SetAutoCompletionMode(Qt::Unchecked);
		ui.chkEnableAutoCompletion->setCheckState(Qt::Unchecked);
	}

	// Create initial highlighter and editor
	m_pIniHighlighter = new CIniHighlighter(theGUI->m_DarkTheme, nullptr, m_IniValidationEnabled);
	m_pCodeEdit = new CCodeEdit(m_pIniHighlighter);
	m_pCodeEdit->installEventFilter(this);
	ui.txtIniSection->parentWidget()->layout()->replaceWidget(ui.txtIniSection, m_pCodeEdit);
	delete ui.txtIniSection;
	ui.txtIniSection = nullptr;
	connect(m_pCodeEdit, SIGNAL(textChanged()), this, SLOT(OnIniChanged()));

	// Set fuzzy prefix length bounds from settings data
	CCodeEdit::SetMaxFuzzyPrefixLength(CIniHighlighter::getMaxSettingNameLengthOrDefault());
	CCodeEdit::SetMinFuzzyPrefixLength(CIniHighlighter::getMinSettingNameLengthOrDefault());
	// Pass fuzzy matching toggle from config (no UI checkbox required)
	m_pCodeEdit->SetFuzzyMatchingEnabled(theConf->GetBool("Options/EnableFuzzyMatching", false));

	// Show tooltips when navigating with keyboard
	{
		int iniMode = theConf->GetInt("Options/EnableIniTooltips", static_cast<int>(CIniHighlighter::GetTooltipMode()));
		int popupMode = theConf->GetInt("Options/EnablePopupTooltips", iniMode);
		CCodeEdit::SetPopupTooltipsEnabled(popupMode);
	}

	// Set up autocompletion based on mode
	QCompleter* completer = new QCompleter(this);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	completer->setFilterMode(Qt::MatchContains);
	
	// Set completer based on mode
	if (CCodeEdit::GetAutoCompletionMode() != CCodeEdit::AutoCompletionMode::Disabled) {
		m_pCodeEdit->SetCompleter(completer);
	}
	else {
		m_pCodeEdit->SetCompleter(nullptr);
	}

	m_pCodeEdit->SetCompletionFilterCallback([](const QString& keyName) -> bool {
		return CIniHighlighter::IsKeyHiddenFromPopup(keyName);
		});
	m_pCodeEdit->SetCaseCorrectionCallback([](const QString& wrongKey) -> QString {
		return CIniHighlighter::FindCaseCorrectedKey(wrongKey);
		});
	m_pCodeEdit->SetCaseCorrectionFilterCallback([](const QString& keyName) -> bool {
		return CIniHighlighter::IsKeyHiddenFromContext(keyName, 'c');
		});
	m_pCodeEdit->SetPopupTooltipCallback([](const QString& keyName) -> QString {
		return CIniHighlighter::GetSettingTooltipForPopup(keyName);
		});
	
	// Update completion model with current settings if auto completion is enabled
	if (CCodeEdit::GetAutoCompletionMode() != CCodeEdit::AutoCompletionMode::Disabled) {
		UpdateAutoCompletion();
	}

	CreateDebug();

	if (m_Template)
	{
		//ui.tabGeneral->setEnabled(false);
		//ui.tabStart->setEnabled(false);
		//ui.tabInternet->setEnabled(false);
		//ui.tabAdvanced->setEnabled(false);
		//ui.tabOther->setEnabled(false);
		//ui.tabTemplates->setEnabled(false);
		//
		//for (int i = 0; i < ui.tabs->count(); i++) 
		//	ui.tabs->setTabEnabled(i, ui.tabs->widget(i)->isEnabled());

		//ui.tabs->setCurrentIndex(ui.tabs->indexOf(ui.tabAccess));

		ui.chkShowGroupTmpl->setEnabled(false);
		ui.chkShowForceTmpl->setEnabled(false);
		ui.chkShowBreakoutTmpl->setEnabled(false);
		ui.chkShowStopTmpl->setEnabled(false);
		ui.chkShowLeaderTmpl->setEnabled(false);
		ui.chkShowStartTmpl->setEnabled(false);
		ui.chkShowFilesTmpl->setEnabled(false);
		ui.chkShowKeysTmpl->setEnabled(false);
		ui.chkShowIPCTmpl->setEnabled(false);
		ui.chkShowWndTmpl->setEnabled(false);
		ui.chkShowCOMTmpl->setEnabled(false);
		ui.chkShowNetFwTmpl->setEnabled(false);
		ui.chkShowRecoveryTmpl->setEnabled(false);
		ui.chkShowRecIgnoreTmpl->setEnabled(false);
		ui.chkShowTriggersTmpl->setEnabled(false);
		ui.chkShowHiddenProcTmpl->setEnabled(false);
		ui.chkShowHostProcTmpl->setEnabled(false);
		ui.chkShowOptionsTmpl->setEnabled(false);

		//ui.chkWithTemplates->setEnabled(false);
	}

	ui.tabs->setCurrentIndex(m_Template ? ui.tabs->count()-1 : 0);
	if(m_Template)
		OnTab();

	//connect(ui.chkWithTemplates, SIGNAL(clicked(bool)), this, SLOT(OnWithTemplates()));

	m_ConfigDirty = true;

	CreateGeneral();

	// Groups
	connect(ui.btnAddGroup, SIGNAL(clicked(bool)), this, SLOT(OnAddGroup()));
	connect(ui.btnAddProg, SIGNAL(clicked(bool)), this, SLOT(OnAddProg()));
	connect(ui.btnDelProg, SIGNAL(clicked(bool)), this, SLOT(OnDelProg()));
	connect(ui.chkShowGroupTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowGroupTmpl()));
	ui.treeGroups->setItemDelegateForColumn(0, new ProgramsDelegate(this, ui.treeGroups, -2, this));
	connect(ui.treeGroups, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnGroupsChanged(QTreeWidgetItem *, int)));
	//

	// Force
	ui.treeForced->setColumnCount(4);
	ui.treeForced->setHeaderLabels(QStringList() << tr("Type") << tr("Name") << tr("Priority") << tr("Recursive"));
	ui.treeBreakout->setColumnCount(5);
	ui.treeBreakout->setHeaderLabels(QStringList() << tr("Type") << tr("Name") << tr("Priority") << tr("Recursive") << tr("Target Box"));

	connect(ui.btnForceProg, SIGNAL(clicked(bool)), this, SLOT(OnForceProg()));
	QMenu* pFileBtnMenu = new QMenu(ui.btnForceProg);
	pFileBtnMenu->addAction(tr("Browse for File"), this, SLOT(OnForceBrowseProg()));
	ui.btnForceProg->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnForceProg->setMenu(pFileBtnMenu);

	connect(ui.btnForceChild, SIGNAL(clicked(bool)), this, SLOT(OnForceChild()));
	pFileBtnMenu = new QMenu(ui.btnForceChild);
	pFileBtnMenu->addAction(tr("Browse for File"), this, SLOT(OnForceBrowseChild()));
	ui.btnForceChild->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnForceChild->setMenu(pFileBtnMenu);

	connect(ui.btnForceDir, SIGNAL(clicked(bool)), this, SLOT(OnForceDir()));
	connect(ui.btnDelForce, SIGNAL(clicked(bool)), this, SLOT(OnDelForce()));
	connect(ui.chkShowForceTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowForceTmpl()));
	//ui.treeForced->setEditTriggers(QAbstractItemView::DoubleClicked);
	ui.treeForced->setItemDelegateForColumn(0, new NoEditDelegate(this));
	ui.treeForced->setItemDelegateForColumn(1, new ProgramsDelegate(this, ui.treeForced, -1, this));
	ui.treeForced->setItemDelegateForColumn(2, new RuleExtensionsDelegate(this, ui.treeForced, false, 2, this));
	ui.treeForced->setItemDelegateForColumn(3, new RuleExtensionsDelegate(this, ui.treeForced, false, 3, this));
	connect(ui.treeForced, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnForcedChanged(QTreeWidgetItem *, int)));
	connect(ui.chkDisableForced, SIGNAL(clicked(bool)), this, SLOT(OnForcedChanged()));

	connect(ui.btnBreakoutProg, SIGNAL(clicked(bool)), this, SLOT(OnBreakoutProg()));
	QMenu* pFileBtnMenu2 = new QMenu(ui.btnBreakoutProg);
	pFileBtnMenu2->addAction(tr("Browse for File"), this, SLOT(OnBreakoutBrowse()));
	ui.btnBreakoutProg->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnBreakoutProg->setMenu(pFileBtnMenu2);
	connect(ui.btnBreakoutDir, SIGNAL(clicked(bool)), this, SLOT(OnBreakoutDir()));
	connect(ui.btnBreakoutDoc, SIGNAL(clicked(bool)), this, SLOT(OnBreakoutDoc()));
	connect(ui.btnDelBreakout, SIGNAL(clicked(bool)), this, SLOT(OnDelBreakout()));
	connect(ui.chkShowBreakoutTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowBreakoutTmpl()));
	connect(ui.chkDisableBreakout, SIGNAL(clicked(bool)), this, SLOT(OnForcedChanged()));
	connect(ui.chkBreakoutUseTargetDir, SIGNAL(clicked(bool)), this, SLOT(OnForcedChanged()));
	connect(ui.chkUseForceBreakoutRuleExtensions, SIGNAL(stateChanged(int)), this, SLOT(OnRuleExtensionsToggled(int)));
	//ui.treeBreakout->setEditTriggers(QAbstractItemView::DoubleClicked);
	ui.treeBreakout->setItemDelegateForColumn(0, new NoEditDelegate(this));
	ui.treeBreakout->setItemDelegateForColumn(1, new ProgramsDelegate(this, ui.treeBreakout, -1, this));
	ui.treeBreakout->setItemDelegateForColumn(2, new RuleExtensionsDelegate(this, ui.treeBreakout, true, 2, this));
	ui.treeBreakout->setItemDelegateForColumn(3, new RuleExtensionsDelegate(this, ui.treeBreakout, true, 3, this));
	ui.treeBreakout->setItemDelegateForColumn(4, new RuleExtensionsDelegate(this, ui.treeBreakout, true, 4, this));
	connect(ui.treeBreakout, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnBreakoutChanged(QTreeWidgetItem *, int)));

	ui.treeForced->setColumnWidth(0, 130);
	ui.treeForced->setColumnWidth(1, 280);
	ui.treeForced->setColumnWidth(2, 90);
	ui.treeForced->setColumnWidth(3, 90);
	ui.treeForced->header()->setSectionsMovable(true);
	ui.treeForced->header()->moveSection(2, 1);
	ui.treeForced->header()->moveSection(3, 2);
	ui.treeForced->header()->moveSection(1, 3);
	ui.treeForced->setSortingEnabled(true);
	ui.treeForced->sortByColumn(2, Qt::AscendingOrder);
	ui.treeForced->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

	ui.treeBreakout->setColumnWidth(0, 130);
	ui.treeBreakout->setColumnWidth(1, 280);
	ui.treeBreakout->setColumnWidth(2, 90);
	ui.treeBreakout->setColumnWidth(3, 90);
	ui.treeBreakout->setColumnWidth(4, 200);
	ui.treeBreakout->header()->setSectionsMovable(true);
	ui.treeBreakout->header()->moveSection(2, 1);
	ui.treeBreakout->header()->moveSection(3, 2);
	ui.treeBreakout->header()->moveSection(4, 3);
	ui.treeBreakout->header()->moveSection(1, 4);
	ui.treeBreakout->setSortingEnabled(true);
	ui.treeBreakout->sortByColumn(2, Qt::AscendingOrder);
	ui.treeBreakout->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
	//

	// Stop
	connect(ui.btnAddLingering, SIGNAL(clicked(bool)), this, SLOT(OnAddLingering()));
	connect(ui.btnDelStopProg, SIGNAL(clicked(bool)), this, SLOT(OnDelStopProg()));
	connect(ui.chkShowStopTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowStopTmpl()));
	ui.treeStop->setItemDelegateForColumn(0, new ProgramsDelegate(this, ui.treeStop, -1, this));
	connect(ui.treeStop, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnStopChanged()));

	connect(ui.btnAddLeader, SIGNAL(clicked(bool)), this, SLOT(OnAddLeader()));
	connect(ui.btnDelLeader, SIGNAL(clicked(bool)), this, SLOT(OnDelLeader()));
	connect(ui.chkShowLeaderTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowLeaderTmpl()));
	ui.treeLeader->setItemDelegateForColumn(0, new ProgramsDelegate(this, ui.treeLeader, -1, this));
	connect(ui.treeLeader, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnStopChanged()));

	connect(ui.chkNoStopWnd, SIGNAL(clicked(bool)), this, SLOT(OnStopChanged()));
	connect(ui.chkLingerLeniency, SIGNAL(clicked(bool)), this, SLOT(OnStopChanged()));
	//

	// Start
	connect(ui.radStartAll, SIGNAL(clicked(bool)), this, SLOT(OnRestrictStart()));
	connect(ui.radStartExcept, SIGNAL(clicked(bool)), this, SLOT(OnRestrictStart()));
	connect(ui.radStartSelected, SIGNAL(clicked(bool)), this, SLOT(OnRestrictStart()));
	connect(ui.btnAddStartProg, SIGNAL(clicked(bool)), this, SLOT(OnAddStartProg()));
	connect(ui.btnDelStartProg, SIGNAL(clicked(bool)), this, SLOT(OnDelStartProg()));
	//connect(ui.chkShowStartTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowStartTmpl()));
	ui.chkShowStartTmpl->setVisible(false); // todo
	connect(ui.chkStartBlockMsg, SIGNAL(clicked(bool)), this, SLOT(OnStartChanged()));
	ui.treeStart->setItemDelegateForColumn(0, new ProgramsDelegate(this, ui.treeStart, -1, this));
	connect(ui.treeStart, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnStartChanged(QTreeWidgetItem *, int)));
	connect(ui.chkAlertBeforeStart, SIGNAL(clicked(bool)), this, SLOT(OnStartChanged()));
	//

	CreateNetwork();

	CreateAccess();

	// Recovery
	connect(ui.chkAutoRecovery, SIGNAL(clicked(bool)), this, SLOT(OnRecoveryChanged()));
	connect(ui.btnAddRecovery, SIGNAL(clicked(bool)), this, SLOT(OnAddRecFolder()));
	connect(ui.btnDelRecovery, SIGNAL(clicked(bool)), this, SLOT(OnDelRecEntry()));
	connect(ui.btnAddRecIgnore, SIGNAL(clicked(bool)), this, SLOT(OnAddRecIgnore()));
	connect(ui.btnAddRecIgnoreExt, SIGNAL(clicked(bool)), this, SLOT(OnAddRecIgnoreExt()));
	connect(ui.btnDelRecIgnore, SIGNAL(clicked(bool)), this, SLOT(OnDelRecIgnoreEntry()));
	connect(ui.chkShowRecoveryTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowRecoveryTmpl()));
	connect(ui.chkShowRecIgnoreTmpl, SIGNAL(clicked(bool)), this, SLOT(OnShowRecIgnoreTmpl()));
	//

	CreateAdvanced();

	// Templates
	connect(ui.cmbCategories, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFilterTemplates()));
	connect(ui.txtTemplates, SIGNAL(textChanged(const QString&)), this, SLOT(OnFilterTemplates()));
	//connect(ui.treeTemplates, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnTemplateClicked(QTreeWidgetItem*, int)));
	connect(ui.treeTemplates, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnTemplateClicked(QTreeWidgetItem*, int)));
	connect(ui.treeTemplates, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnTemplateDoubleClicked(QTreeWidgetItem*, int)));
	connect(ui.btnAddTemplate, SIGNAL(clicked(bool)), this, SLOT(OnAddTemplates()));
	QMenu* pTmplBtnMenu = new QMenu(ui.btnAddTemplate);
	for(int i = 1; i < CTemplateWizard::TmplMax; i++)
		pTmplBtnMenu->addAction(tr("Add %1 Template").arg(CTemplateWizard::GetTemplateLabel((CTemplateWizard::ETemplateType)i)), this, SLOT(OnTemplateWizard()))->setData(i);
	ui.btnAddTemplate->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnAddTemplate->setMenu(pTmplBtnMenu);
	connect(ui.btnOpenTemplate, SIGNAL(clicked(bool)), this, SLOT(OnOpenTemplate()));
	connect(ui.btnDelTemplate, SIGNAL(clicked(bool)), this, SLOT(OnDelTemplates()));
	connect(ui.chkScreenReaders, SIGNAL(clicked(bool)), this, SLOT(OnScreenReaders()));
	//

	connect(ui.tabs, SIGNAL(currentChanged(int)), this, SLOT(OnTab()));

	// edit
	ApplyIniEditFont();

	connect(ui.btnEditIni, SIGNAL(clicked(bool)), this, SLOT(OnEditIni()));
	connect(ui.chkValidateIniKeys, SIGNAL(stateChanged(int)), this, SLOT(OnIniValidationToggled(int)));
	connect(ui.chkEnableTooltips, SIGNAL(stateChanged(int)), this, SLOT(OnTooltipToggled(int)));
	connect(ui.chkEnableAutoCompletion, SIGNAL(stateChanged(int)), this, SLOT(OnAutoCompletionToggled(int)));
	connect(ui.btnEditorSettings, SIGNAL(clicked(bool)), this, SLOT(OnEditorSettings()));
	connect(ui.btnSaveIni, SIGNAL(clicked(bool)), this, SLOT(OnSaveIni()));
	connect(ui.btnCancelEdit, SIGNAL(clicked(bool)), this, SLOT(OnCancelEdit()));
	//connect(ui.txtIniSection, SIGNAL(textChanged()), this, SLOT(OnIniChanged()));

	connect(ui.buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked(bool)), this, SLOT(ok()));
	connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked(bool)), this, SLOT(apply()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(close()));

	if (ReadOnly)
	{
		ui.btnEditIni->setEnabled(false);
		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}

	if (theAPI->IsRunningAsAdmin())
	{
		ui.chkDropRights->setEnabled(false);
		ui.chkFakeElevation->setEnabled(false);
	}
	else
		ui.lblAdmin->setVisible(false);

	LoadConfig();

	m_pCurrentTab = ui.tabGeneral;
	UpdateCurrentTab();

	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	
	ui.treeCopy->viewport()->installEventFilter(this);
	ui.treeRun->viewport()->installEventFilter(this);
	ui.treeGroups->viewport()->installEventFilter(this);
	ui.treeForced->viewport()->installEventFilter(this);
	ui.treeBreakout->viewport()->installEventFilter(this);
	ui.treeStop->viewport()->installEventFilter(this);
	ui.treeLeader->viewport()->installEventFilter(this);
	ui.treeStart->viewport()->installEventFilter(this);
	ui.treeINet->viewport()->installEventFilter(this);
	ui.treeNetFw->viewport()->installEventFilter(this);
	ui.treeDns->viewport()->installEventFilter(this);
	ui.treeProxy->viewport()->installEventFilter(this);
	ui.treeFiles->viewport()->installEventFilter(this);
	ui.treeKeys->viewport()->installEventFilter(this);
	ui.treeIPC->viewport()->installEventFilter(this);
	ui.treeWnd->viewport()->installEventFilter(this);
	ui.treeCOM->viewport()->installEventFilter(this);
	ui.treeRecovery->viewport()->installEventFilter(this);
	ui.treeRecIgnore->viewport()->installEventFilter(this);
	//ui.treeAccess->viewport()->installEventFilter(this);
	if(ui.treeOptions) ui.treeOptions->viewport()->installEventFilter(this);
	ui.treeTriggers->viewport()->installEventFilter(this);
	ui.treeHideProc->viewport()->installEventFilter(this);
	ui.treeHostProc->viewport()->installEventFilter(this);
	ui.lstUsers->viewport()->installEventFilter(this);
	ui.treeTemplates->viewport()->installEventFilter(this);
	this->installEventFilter(this); // prevent enter from closing the dialog

	restoreGeometry(theConf->GetBlob("OptionsWindow/Window_Geometry"));

	foreach(QTreeWidget * pTree, this->findChildren<QTreeWidget*>()) {
		QByteArray Columns = theConf->GetBlob("OptionsWindow/" + pTree->objectName() + "_Columns");
		if (!Columns.isEmpty()) 
			pTree->header()->restoreState(Columns);
	}

	auto enforceColumnOrder = [](QTreeWidget* tree, const QList<int>& desiredLogicalOrder) {
		if (!tree || !tree->header())
			return;

		QHeaderView* header = tree->header();
		for (int visual = 0; visual < desiredLogicalOrder.size(); ++visual) {
			int logical = desiredLogicalOrder[visual];
			if (logical < 0 || logical >= tree->columnCount())
				continue;

			int currentVisual = header->visualIndex(logical);
			if (currentVisual >= 0 && currentVisual != visual)
				header->moveSection(currentVisual, visual);
		}
	};

	const char* migrationKey = "OptionsWindow/RuleExtensionsColumnOrderMigrated_v1";
	if (!theConf->GetBool(migrationKey, false)) {
		enforceColumnOrder(ui.treeForced, QList<int>() << 0 << 2 << 3 << 1);
		enforceColumnOrder(ui.treeBreakout, QList<int>() << 0 << 2 << 3 << 4 << 1);

		ui.treeForced->sortByColumn(2, Qt::AscendingOrder);
		ui.treeBreakout->sortByColumn(2, Qt::AscendingOrder);

		theConf->SetBlob("OptionsWindow/" + ui.treeForced->objectName() + "_Columns", ui.treeForced->header()->saveState());
		theConf->SetBlob("OptionsWindow/" + ui.treeBreakout->objectName() + "_Columns", ui.treeBreakout->header()->saveState());

		theConf->SetValue(migrationKey, true);
	}

	if (theAPI->GetGlobalSettings()->GetBool("EditAdminOnly", false) && !IsAdminUser())
	{
		for (int I = 0; I < ui.tabs->count(); I++) {
			QGridLayout* pGrid = qobject_cast<QGridLayout*>(ui.tabs->widget(I)->layout());
			QTabWidget* pSubTabs = pGrid ? qobject_cast<QTabWidget*>(pGrid->itemAt(0)->widget()) : NULL;
			if (!pSubTabs) {
				ui.tabs->widget(I)->setEnabled(false);
			}
			else {
				for (int J = 0; J < pSubTabs->count(); J++) {
					pSubTabs->widget(J)->setEnabled(false);
				}
			}
		}
	}

	int iOptionTree = theConf->GetInt("Options/OptionTree", 2);
	if (iOptionTree == 2)
		iOptionTree = iViewMode == 2 ? 1 : 0;

	if (iOptionTree) {
		m_HoldChange = true;
		OnSetTree();
		m_HoldChange = false;
	}
	else {
		//this->setMinimumHeight(390);

		QWidget* pSearch = AddConfigSearch(ui.tabs);
		ui.horizontalLayout->insertWidget(0, pSearch);
		QTimer::singleShot(0, [this]() {
			m_pSearch->setMaximumWidth(m_pTabs->tabBar()->width());
		});

		QAction* pSetTree = new QAction();
		connect(pSetTree, SIGNAL(triggered()), this, SLOT(OnSetTree()));
		pSetTree->setShortcut(QKeySequence("Ctrl+Alt+T"));
		pSetTree->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(pSetTree);
	}
	m_pSearch->setPlaceholderText(tr("Search for options"));
	
	SetTabOrder(this);
}

void COptionsWindow::ApplyIniEditFont()
{
	QFont font; // defaults to application font
	auto fontName = theConf->GetString("UIConfig/IniFont", "").trimmed();
	if (!fontName.isEmpty()) {
		font.fromString(fontName); // ignore fromString() fail
		//ui.txtIniSection->setFont(font);
		m_pCodeEdit->SetFont(font);
	}
}

void COptionsWindow::UpdateAutoCompletion()
{
	if (CCodeEdit::GetAutoCompletionMode() == CCodeEdit::AutoCompletionMode::Disabled || !m_pCodeEdit || !m_pCodeEdit->GetCompleter())
		return;

	// Get completion candidates from the highlighter
	QStringList candidates = CIniHighlighter::GetCompletionCandidates();

	// Update the completion model
	m_pCodeEdit->UpdateCompletionModel(candidates);
}

void COptionsWindow::OnSetTree()
{
	if (!ui.tabs) return;
	QWidget* pAltView = ConvertToTree(ui.tabs);
	ui.verticalLayout->replaceWidget(ui.tabs, pAltView);
	ui.tabs->deleteLater();
	ui.tabs = NULL;
}

void COptionsWindow::OnOptChanged() 
{
	if (m_HoldChange)
		return;
	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

COptionsWindow::~COptionsWindow()
{
	theConf->SetBlob("OptionsWindow/Window_Geometry",saveGeometry());

	foreach(QTreeWidget * pTree, this->findChildren<QTreeWidget*>()) 
		theConf->SetBlob("OptionsWindow/" + pTree->objectName() + "_Columns", pTree->header()->saveState());
}

void COptionsWindow::closeEvent(QCloseEvent *e)
{
	emit Closed();
	this->deleteLater();
}

bool COptionsWindow::eventFilter(QObject *source, QEvent *event)
{
	auto isTreeViewport = [source](QAbstractItemView* view) {
		return view && source == view->viewport();
	};

	if (event->type() == QEvent::KeyPress && ((QKeyEvent*)event)->key() == Qt::Key_Escape 
		&& ((QKeyEvent*)event)->modifiers() == Qt::NoModifier
		&& source == m_pCodeEdit)
	{
		return true; // cancel event
	}

	if (event->type() == QEvent::KeyPress && ((QKeyEvent*)event)->key() == Qt::Key_Escape 
		&& ((QKeyEvent*)event)->modifiers() == Qt::NoModifier
		&& (source == ui.treeCopy->viewport()
			|| source == ui.treeINet->viewport() || source == ui.treeNetFw->viewport() 
			// || source == ui.treeAccess->viewport()
			|| source == ui.treeFiles->viewport() || source == ui.treeKeys->viewport() || source == ui.treeIPC->viewport() || source == ui.treeWnd->viewport() || source == ui.treeCOM->viewport() 
			|| (ui.treeOptions && source == ui.treeOptions->viewport())))
	{
		CloseCopyEdit(false);
		CloseINetEdit(false);
		CloseNetFwEdit(false);
		CloseAccessEdit(false);
		CloseOptionEdit(false);
        CloseNetProxyEdit(false);
		return true; // cancel event
	}

	if (event->type() == QEvent::KeyPress && (((QKeyEvent*)event)->key() == Qt::Key_Enter || ((QKeyEvent*)event)->key() == Qt::Key_Return) 
		&& (((QKeyEvent*)event)->modifiers() == Qt::NoModifier || ((QKeyEvent*)event)->modifiers() == Qt::KeypadModifier))
	{
		CloseCopyEdit(true);
		CloseINetEdit(true);
		CloseNetFwEdit(true);
		CloseAccessEdit(true);
		CloseOptionEdit(true);
		CloseNetProxyEdit(true);
		return true; // cancel event
	}

	if (event->type() == QEvent::KeyPress && ((QKeyEvent*)event)->key() == Qt::Key_Delete
		&& ((QKeyEvent*)event)->modifiers() == Qt::NoModifier)
	{
		CloseCopyEdit(true);
		CloseINetEdit(true);
		CloseNetFwEdit(true);
		CloseAccessEdit(true);
		CloseOptionEdit(true);
		CloseNetProxyEdit(true);

		if (isTreeViewport(ui.treeCopy))				OnDelCopyRule();
		else if (isTreeViewport(ui.treeRun))			OnDelCommand();
		else if (isTreeViewport(ui.treeGroups))		OnDelProg();
		else if (isTreeViewport(ui.treeForced))		OnDelForce();
		else if (isTreeViewport(ui.treeBreakout))		OnDelBreakout();
		else if (isTreeViewport(ui.treeStop))			OnDelStopProg();
		else if (isTreeViewport(ui.treeLeader))		OnDelLeader();
		else if (isTreeViewport(ui.treeStart))		OnDelStartProg();
		else if (isTreeViewport(ui.treeINet))			OnDelINetProg();
		else if (isTreeViewport(ui.treeNetFw))		OnDelNetFwRule();
		else if (isTreeViewport(ui.treeDns))			OnDelDnsFilter();
		else if (isTreeViewport(ui.treeProxy))		OnDelNetProxy();
		else if (isTreeViewport(ui.treeFiles))		OnDelFile();
		else if (isTreeViewport(ui.treeKeys))			OnDelKey();
		else if (isTreeViewport(ui.treeIPC))			OnDelIPC();
		else if (isTreeViewport(ui.treeWnd))			OnDelWnd();
		else if (isTreeViewport(ui.treeCOM))			OnDelCOM();
		else if (isTreeViewport(ui.treeRecovery))		OnDelRecEntry();
		else if (isTreeViewport(ui.treeRecIgnore))	OnDelRecIgnoreEntry();
		else if (ui.treeOptions && isTreeViewport(ui.treeOptions)) OnDelOption();
		else if (isTreeViewport(ui.treeTriggers))		OnDelAuto();
		else if (isTreeViewport(ui.treeHideProc))		OnDelProcess();
		else if (isTreeViewport(ui.treeHostProc))		OnDelHostProcess();
		else if (isTreeViewport(ui.lstUsers))			OnDelUser();
		else if (isTreeViewport(ui.treeTemplates))	OnDelTemplates();
		else return QDialog::eventFilter(source, event);

		return true;
	}
	
	if (source == ui.treeCopy->viewport() && event->type() == QEvent::MouseButtonPress)
	{
		CloseCopyEdit();
	}

	if (source == ui.treeINet->viewport() && event->type() == QEvent::MouseButtonPress)
	{
		CloseINetEdit();
	}

	if (source == ui.treeNetFw->viewport() && event->type() == QEvent::MouseButtonPress)
	{
		CloseNetFwEdit();
	}

    if (source == ui.treeProxy->viewport() && event->type() == QEvent::MouseButtonPress)
	{
		CloseNetProxyEdit();
	}

	if (//source == ui.treeAccess->viewport() 
		(source == ui.treeFiles->viewport() || source == ui.treeKeys->viewport() || source == ui.treeIPC->viewport() || source == ui.treeWnd->viewport() || source == ui.treeCOM->viewport())
		&& event->type() == QEvent::MouseButtonPress)
	{
		CloseAccessEdit();
	}


	if ((ui.treeOptions && source == ui.treeOptions->viewport()) && event->type() == QEvent::MouseButtonPress)
	{
		CloseOptionEdit();
	}

	// Tooltip handling
	if (source == m_pCodeEdit && event->type() == QEvent::ToolTip) {
		// Check if tooltips are completely disabled
		if (CIniHighlighter::GetTooltipMode() == CIniHighlighter::TooltipMode::Disabled)
			return false;

		QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);

		// Find the text edit widget inside CCodeEdit
		QTextEdit* pTextEdit = m_pCodeEdit->findChild<QTextEdit*>();
		if (pTextEdit) {
			// Convert mouse position to text cursor position
			QPoint pos = pTextEdit->viewport()->mapFrom(m_pCodeEdit, helpEvent->pos());
			QTextCursor cursor = pTextEdit->cursorForPosition(pos);

			// Get the current line to check if it's a comment
			QTextBlock block = cursor.block();
			QString currentLine = block.text();

			// Don't show tooltips for comment lines
			if (CIniHighlighter::IsCommentLine(currentLine))
				return false;

			// Check if we're on the value side of the equals sign (after the =)
			int equalsPos = currentLine.indexOf('=');
			if (equalsPos >= 0 && (cursor.position() - block.position()) > equalsPos) {
				// We're in the value part, don't show tooltip
				QToolTip::hideText();
				return false;
			}

			// Custom word selection that includes dots and underscores
			int initialPos = cursor.position() - block.position();
			int startPos = initialPos;
			int endPos = initialPos;

			// Move to start of the word
			while (startPos > 0) {
				QChar c = currentLine[startPos - 1];
				if (c.isLetterOrNumber() || c == '_' || c == '.')
					startPos--;
				else
					break;
			}

			// Move to end of the word
			while (endPos < currentLine.length()) {
				QChar c = currentLine[endPos];
				if (c.isLetterOrNumber() || c == '_' || c == '.')
					endPos++;
				else
					break;
			}

			// Show tooltip if it's a valid setting
			if (CIniHighlighter::IsValidTooltipContext(currentLine.left(endPos))) {
				// Only try to show tooltips if settings are loaded
				if (CIniHighlighter::IsSettingsLoaded()) {
					QString settingName = currentLine.mid(startPos, endPos - startPos);
					if (settingName.endsWith('='))
						settingName.chop(1);
					QString tooltipText = CIniHighlighter::GetSettingTooltip(settingName);
					if (!tooltipText.isEmpty()) {
						QToolTip::showText(helpEvent->globalPos(), tooltipText, pTextEdit);
						return true;
					}
				}
			}
			QToolTip::hideText();
		}
	}

	return QDialog::eventFilter(source, event);
}

//void COptionsWindow::OnWithTemplates()
//{
//	m_Template = ui.chkWithTemplates->isChecked();
//	ui.buttonBox->setEnabled(!m_Template);
//	LoadConfig();
//}

void COptionsWindow::ReadAdvancedCheck(const QString& Name, QCheckBox* pCheck, const QString& Value)
{
	QString Data = m_pBox->GetText(Name);
	if (Data == Value)			pCheck->setCheckState(Qt::Checked);
	else if (Data.isEmpty())	pCheck->setCheckState(Qt::Unchecked);
	else						pCheck->setCheckState(Qt::PartiallyChecked);
}

int COptionsWindow__GetBoolConfig(const QString& Value)
{
	QString temp = Value.left(1).toLower();
	if (temp == "y")
		return 1;
	else if (temp == "n")
		return 0;
	return -1;
}

void COptionsWindow::ReadGlobalCheck(QCheckBox* pCheck, const QString& Setting, bool bDefault)
{
	int iLocal = COptionsWindow__GetBoolConfig(m_pBox->GetText(Setting));
	int iTemplate = COptionsWindow__GetBoolConfig(m_pBox->GetText(Setting, QString(), false, true, true));
	int iGlobal = COptionsWindow__GetBoolConfig(m_pBox->GetText(Setting, QString(), true));

	bool bTemplate = m_pBox->GetBool(Setting, bDefault, true, true);
	if (iLocal != -1) 
		pCheck->setChecked(iLocal == 1);
	else
		pCheck->setChecked(iTemplate != -1 ? iTemplate == 1 : iGlobal != -1 ? iGlobal == 1 : bDefault);
	QStringList Info;
	Info.append(tr("Box: %1").arg(iLocal == 1 ? "y" : "n"));
	if (iTemplate != -1)	Info.append(tr("Template: %1").arg(iTemplate == 1 ? "y" : "n"));
	if (iGlobal != -1)		Info.append(tr("Global: %1").arg(iGlobal == 1 ? "y" : "n"));
	Info.append(tr("Default: %1").arg(bDefault ? "y" : "n"));
	pCheck->setToolTip(Info.join("\r\n"));
}

void COptionsWindow::WriteGlobalCheck(QCheckBox* pCheck, const QString& Setting, bool bDefault)
{
	bool bLocal = pCheck->isChecked();
	bool bPreset = m_pBox->GetBool(Setting, bDefault, true, true);
	SB_STATUS Status;
	if(bPreset == bLocal)
		Status = m_pBox->DelValue(Setting);
	else 
		Status = m_pBox->SetText(Setting, bLocal ? "y" : "n");

	if (!Status)
		throw Status;
}

void COptionsWindow::LoadConfig()
{
	m_ConfigDirty = false;

	m_HoldChange = true;

	LoadTemplates();

	LoadGeneral();

	LoadGroups();

	LoadForced();

	LoadStop();

	LoadStart();

	LoadINetAccess();
	LoadNetFwRules();
	LoadDnsFilter();
	LoadNetProxy();
	LoadNetwork();

	LoadAccessList();

	LoadRecoveryList();

	LoadAdvanced();
	LoadDebug();
	
	UpdateBoxType();

	// Update autocompletion after all settings are loaded
	UpdateAutoCompletion();

	m_HoldChange = false;
}

void COptionsWindow::WriteAdvancedCheck(QCheckBox* pCheck, const QString& Name, const QString& Value)
{
	SB_STATUS Status;
	if (pCheck->checkState() == Qt::Checked)		
		Status = m_pBox->SetText(Name, Value);
	else if (pCheck->checkState() == Qt::Unchecked) 
		Status = m_pBox->DelValue(Name);
	
	if (!Status)
		throw Status;
}

void COptionsWindow::WriteAdvancedCheck(QCheckBox* pCheck, const QString& Name, const QString& OnValue, const QString& OffValue)
{
	//if (pCheck->checkState() == Qt::PartiallyChecked)
	//	return;

	if (!pCheck->isEnabled())
		return;

	QString StrValue;
	if (pCheck->checkState() == Qt::Checked)
	{
		if (!OnValue.isEmpty())
			StrValue = OnValue;
	}
	else if (pCheck->checkState() == Qt::Unchecked)
	{
		if (!OffValue.isEmpty())
			StrValue = OffValue;
	}

	QStringList Values = m_pBox->GetTextList(Name, false);
	foreach(const QString & CurValue, Values) {
		if (CurValue.contains(","))
			continue;
		if (!StrValue.isEmpty() && CurValue == StrValue)
			StrValue.clear();
		else
			m_pBox->DelValue(Name, CurValue);
	}

	if (!StrValue.isEmpty()) {
		SB_STATUS Status = m_pBox->AppendText(Name, StrValue);
		if (!Status)
			throw Status;
	}
}

void COptionsWindow::WriteText(const QString& Name, const QString& Value)
{
	SB_STATUS Status = m_pBox->SetText(Name, Value);
	if (!Status)
		throw Status;
}

void COptionsWindow::WriteTextList(const QString& Setting, const QStringList& List)
{
	SB_STATUS Status = m_pBox->UpdateTextList(Setting, List, m_Template);
	if (!Status)
		throw Status;
}

void COptionsWindow::WriteTextSafe(const QString& Name, const QString& Value)
{
	QStringList List = m_pBox->GetTextList(Name, false);

	// clear all non per process (name=program.exe,value) entries 
	for (int i = 0; i < List.count(); i++) {
		if (!List[i].contains(","))
			List.removeAt(i--);
	}

	// Prepend the global entry
	if (!Value.isEmpty()) List.append(Value);

	WriteTextList(Name, List);
}

QString COptionsWindow::ReadTextSafe(const QString& Name, const QString& Default)
{
	QStringList List = m_pBox->GetTextList(Name, false);

	for (int i = 0; i < List.count(); i++) {
		if (!List[i].contains(","))
			return List[i];
	}

	return Default;
}

void COptionsWindow::SaveConfig()
{
	bool UpdatePaths = false;

	m_pBox->SetRefreshOnChange(false);

	try
	{
		if (m_GeneralChanged)
			SaveGeneral();
		if (m_CopyRulesChanged)
			SaveCopyRules();

		if (m_GroupsChanged)
			SaveGroups();

		if (m_ForcedChanged)
			SaveForced();

		if (m_StopChanged)
			SaveStop();

		if (m_StartChanged)
			SaveStart();

		if (m_INetBlockChanged)
			SaveINetAccess();
		if (m_NetFwRulesChanged)
			SaveNetFwRules();
		if (m_DnsFilterChanged)
			SaveDnsFilter();
		if (m_NetProxyChanged)
			SaveNetProxy();
		if (m_NetworkChanged)
			SaveNetwork();

		if (m_AccessChanged) {
			SaveAccessList();
			UpdatePaths = true;
		}

		if (m_RecoveryChanged)
			SaveRecoveryList();

		if (m_AdvancedChanged)
			SaveAdvanced();
		SaveDebug();

		if (m_TemplatesChanged)
			SaveTemplates();

		if (m_FoldersChanged)
			SaveFolders();
	}
	catch (SB_STATUS Status)
	{
		theGUI->CheckResults(QList<SB_STATUS>() << Status, theGUI);
	}

	m_pBox->SetRefreshOnChange(true);
	m_pBox->CommitIniChanges();

	if (UpdatePaths)
		TriggerPathReload();
}

bool COptionsWindow::apply()
{
	if (m_pBox->GetText("Enabled").isEmpty() && !(m_Template && m_pBox->GetName().mid(9, 6).compare("Local_", Qt::CaseInsensitive) == 0)) {
		QMessageBox::critical(this, "Sandboxie-Plus", tr("This sandbox has been deleted hence configuration can not be saved."));
		return false;
	}

	CloseINetEdit();
	CloseNetFwEdit();
	CloseAccessEdit();
	CloseOptionEdit();
    CloseNetProxyEdit();

	if (!ui.btnEditIni->isEnabled())
		SaveIniSection();
	else
	{
		if (m_GeneralChanged) {
			auto pBoxEx = m_pBox.objectCast<CSandBoxPlus>();
			if (ui.chkEncrypt->isChecked() && !QFile::exists(pBoxEx->GetBoxImagePath())) {
				if (m_Password.isEmpty())
					OnSetPassword();
				if (m_Password.isEmpty())
					return false;
				pBoxEx->ImBoxCreate(m_ImageSize / 1024, m_Password);
			}
		}

		SaveConfig();
	}

	LoadConfig();

	UpdateCurrentTab();

	//emit OptionsChanged();

	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

	return true;
}

void COptionsWindow::ok()
{
	if(apply())
		close();
}

void COptionsWindow::reject()
{
	if (m_GeneralChanged
	 || m_CopyRulesChanged
	 || m_GroupsChanged
	 || m_ForcedChanged
	 || m_StopChanged
	 || m_StartChanged
	// ||  m_RestrictionChanged
	 || m_INetBlockChanged
	 || m_NetFwRulesChanged
	 || m_DnsFilterChanged
	 || m_NetProxyChanged
	 || m_AccessChanged
	 || m_TemplatesChanged
	 || m_FoldersChanged
	 || m_RecoveryChanged
	 || m_AdvancedChanged)
	{
		if (QMessageBox("Sandboxie-Plus", tr("Some changes haven't been saved yet, do you really want to close this options window?")
		, QMessageBox::Warning, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			return;
	}

	this->close();
}

void COptionsWindow::showTab(const QString& Name)
{
	QWidget* pWidget = this->findChild<QWidget*>("tab" + Name);

	if (ui.tabs) {
		for (int i = 0; i < ui.tabs->count(); i++) {
			QGridLayout* pGrid = qobject_cast<QGridLayout*>(ui.tabs->widget(i)->layout());
			QTabWidget* pSubTabs = pGrid ? qobject_cast<QTabWidget*>(pGrid->itemAt(0)->widget()) : NULL;
			if(ui.tabs->widget(i) == pWidget)
				ui.tabs->setCurrentIndex(i);
			else if(pSubTabs) {
				for (int j = 0; j < pSubTabs->count(); j++) {
					if (pSubTabs->widget(j) == pWidget) {
						ui.tabs->setCurrentIndex(i);
						pSubTabs->setCurrentIndex(j);
					}
				}
			}
		}
	} 
	else
		m_pStack->setCurrentWidget(pWidget);

	CSandMan::SafeShow(this);
}

void COptionsWindow::SetProgramItem(QString Program, QTreeWidgetItem* pItem, int Column, const QString& Suffix, bool bList)
{
	pItem->setData(Column, Qt::UserRole, Program);
	if (Program.left(1) == "<")
		Program = tr("Group: %1").arg(Program.mid(1, Program.length() - 2));
	else if (Program.isEmpty() || Program == "*")
		Program = tr("All Programs");
	else if(bList)
		m_Programs.insert(Program);
	pItem->setText(Column, Program + Suffix);
}

bool COptionsWindow::IsRuleExtensionsEnabled() const
{
	Qt::CheckState state = ui.chkUseForceBreakoutRuleExtensions->checkState();
	if (state == Qt::Checked)
		return true;
	if (state == Qt::Unchecked)
		return false;
	return m_pBox->GetBool("UseForceBreakoutRuleExtensions", false, true, true);
}

QSet<int> COptionsWindow::GetUsedRulePriorities(const QTreeWidgetItem* pExclude) const
{
	QSet<int> used;
	QMap<int, QStringList> sources = GetUsedRulePrioritySources(pExclude);
	for (auto it = sources.constBegin(); it != sources.constEnd(); ++it)
		used.insert(it.key());
	return used;
}

QMap<int, QStringList> COptionsWindow::GetUsedRulePrioritySources(const QTreeWidgetItem* pExclude) const
{
	QMap<int, QSet<QString>> sourceMap;

	auto parsePriorityFromRule = [](const QString& rule, int* outValue) {
		QStringList parts = rule.split("|", Qt::KeepEmptyParts);
		for (int i = 1; i < parts.size(); ++i) {
			const QString& part = parts[i].trimmed();
			if (!part.startsWith("Priority=", Qt::CaseInsensitive))
				continue;

			QString valueStr = part.mid(9).trimmed();
			bool ok = false;
			int value = valueStr.toInt(&ok);
			if (!ok || value < 0)
				continue;

			*outValue = value;
			return true;
		}
		return false;
	};

	auto settingKind = [&](const QString& settingName) {
		return settingName.startsWith("Force", Qt::CaseInsensitive) ? tr("force") : tr("breakout");
	};

	auto settingState = [&](const QString& settingName) {
		return settingName.endsWith("Disabled", Qt::CaseInsensitive) ? tr("disabled") : tr("enabled");
	};

	auto addSource = [&](int priority, const QString& source) {
		sourceMap[priority].insert(source);
	};

	auto collectTree = [&](QTreeWidget* pTree, const QString& kind) {
		if (!pTree)
			return;

		for (int i = 0; i < pTree->topLevelItemCount(); ++i) {
			QTreeWidgetItem* pItem = pTree->topLevelItem(i);
			if (!pItem || pItem == pExclude)
				continue;

			int type = pItem->data(0, Qt::UserRole).toInt();
			if (type == eTemplate)
				continue;

			if (type != eProcess && type != ePath && type != eText && type != eParent)
				continue;

			QString text = pItem->data(2, Qt::UserRole).toString().trimmed();
			if (text.isEmpty())
				continue;

			bool ok = false;
			int value = text.toInt(&ok);
			if (!ok || value < 0)
				continue;

			const QString state = (pItem->checkState(0) == Qt::Checked) ? tr("enabled") : tr("disabled");
			addSource(value, tr("box, %1, %2").arg(kind, state));
		}
	};

	auto collectRuleList = [&](const QStringList& rules, const QString& sourceLabel) {
		for (const QString& rule : rules) {
			int value = -1;
			if (parsePriorityFromRule(rule, &value))
				addSource(value, sourceLabel);
		}
	};

	const QStringList settings = {
		"ForceProcess", "ForceProcessDisabled",
		"ForceChildren", "ForceChildrenDisabled",
		"ForceFolder", "ForceFolderDisabled",
		"BreakoutProcess", "BreakoutProcessDisabled",
		"BreakoutFolder", "BreakoutFolderDisabled",
		"BreakoutDocument", "BreakoutDocumentDisabled"
	};

	const QStringList crossBoxRuleSettings = {
		"ForceProcess", "ForceProcessDisabled",
		"ForceChildren", "ForceChildrenDisabled",
		"ForceFolder", "ForceFolderDisabled",
		"BreakoutProcess", "BreakoutProcessDisabled",
		"BreakoutFolder", "BreakoutFolderDisabled",
		"BreakoutDocument", "BreakoutDocumentDisabled"
	};

	collectTree(ui.treeForced, tr("force"));
	collectTree(ui.treeBreakout, tr("breakout"));

	for (const QString& setting : settings) {
		collectRuleList(
			m_pBox->GetTextList(setting, m_Template),
			tr("box, %1, %2").arg(settingKind(setting), settingState(setting)));
	}

	QSharedPointer<CSbieIni> pGlobalSettings = m_pBox->GetAPI()->GetGlobalSettings();
	if (pGlobalSettings) {
		for (const QString& setting : settings) {
			collectRuleList(
				pGlobalSettings->GetTextList(setting, false),
				tr("global, %1, %2").arg(settingKind(setting), settingState(setting)));
		}
	}

	foreach(const QString& Template, m_pBox->GetTemplates()) {
		if (Template.compare("GlobalSettings", Qt::CaseInsensitive) == 0)
			continue;

		for (const QString& setting : settings) {
			collectRuleList(
				m_pBox->GetTextListTmpl(setting, Template),
				tr("template %1, %2, %3").arg(Template, settingKind(setting), settingState(setting)));
		}
	}

	// Include rule priorities from other enabled boxes so cross-box conflicts are shown as unavailable.
	if (!m_Template) {
		QMap<QString, CSandBoxPtr> allBoxes = m_pBox->GetAPI()->GetAllBoxes();
		QString currentBoxName = m_pBox->GetName();

		for (auto it = allBoxes.constBegin(); it != allBoxes.constEnd(); ++it) {
			const CSandBoxPtr& pOtherBox = it.value();
			if (!pOtherBox || !pOtherBox->IsEnabled())
				continue;

			QString otherBoxName = pOtherBox->GetName();
			if (otherBoxName.isEmpty() || otherBoxName.compare(currentBoxName, Qt::CaseInsensitive) == 0)
				continue;

			for (const QString& setting : crossBoxRuleSettings) {
				collectRuleList(
					pOtherBox->GetTextList(setting, false),
					tr("box %1, %2, %3").arg(otherBoxName, settingKind(setting), settingState(setting)));
			}
		}
	}

	QMap<int, QStringList> result;
	for (auto it = sourceMap.constBegin(); it != sourceMap.constEnd(); ++it) {
		QStringList sourceList = it.value().values();
		sourceList.sort(Qt::CaseInsensitive);
		result.insert(it.key(), sourceList);
	}

	return result;
}

QSet<QString> COptionsWindow::GetUsedRuleRecursiveValues(const QTreeWidgetItem* pExclude) const
{
	QSet<QString> used;

	auto addRecursive = [&](const QString& text) {
		QString value = NormalizeRecursiveUiValue(text);
		if (!value.isEmpty())
			used.insert(value);
	};

	auto collectTree = [&](QTreeWidget* pTree) {
		if (!pTree)
			return;

		for (int i = 0; i < pTree->topLevelItemCount(); ++i) {
			QTreeWidgetItem* pItem = pTree->topLevelItem(i);
			if (!pItem || pItem == pExclude)
				continue;

			int type = pItem->data(0, Qt::UserRole).toInt();
			if (type == eTemplate)
				continue;

			if (type != eProcess && type != ePath && type != eText && type != eParent)
				continue;

			QString text = pItem->data(3, Qt::UserRole).toString().trimmed();
			if (text.isEmpty())
				continue;

			addRecursive(text);
		}
	};

	auto collectRuleList = [&](const QStringList& rules) {
		for (const QString& rule : rules) {
			QStringList parts = rule.split("|", Qt::KeepEmptyParts);
			for (int i = 1; i < parts.size(); ++i) {
				const QString part = parts[i].trimmed();
				if (!part.startsWith("Recursive=", Qt::CaseInsensitive))
					continue;

				addRecursive(part.mid(10));
				break;
			}
		}
	};

	const QStringList settings = {
		"ForceProcess", "ForceProcessDisabled",
		"ForceChildren", "ForceChildrenDisabled",
		"ForceFolder", "ForceFolderDisabled",
		"BreakoutProcess", "BreakoutProcessDisabled",
		"BreakoutFolder", "BreakoutFolderDisabled",
		"BreakoutDocument", "BreakoutDocumentDisabled"
	};

	collectTree(ui.treeForced);
	collectTree(ui.treeBreakout);

	for (const QString& setting : settings)
		collectRuleList(m_pBox->GetTextList(setting, m_Template));

	QSharedPointer<CSbieIni> pGlobalSettings = m_pBox->GetAPI()->GetGlobalSettings();
	if (pGlobalSettings) {
		for (const QString& setting : settings)
			collectRuleList(pGlobalSettings->GetTextList(setting, false));
	}

	foreach(const QString& Template, m_pBox->GetTemplates()) {
		if (Template.compare("GlobalSettings", Qt::CaseInsensitive) == 0)
			continue;

		for (const QString& setting : settings)
			collectRuleList(m_pBox->GetTextListTmpl(setting, Template));
	}

	return used;
}

QString COptionsWindow::SelectProgram(bool bOrGroup)
{
	CComboInputDialog progDialog(this);
	progDialog.setText(tr("Enter program:"));
	progDialog.setEditable(true);

	if (bOrGroup)
	{
		foreach(const QString Group, GetCurrentGroups()){
			QString GroupName = Group.mid(1, Group.length() - 2);
			progDialog.addItem(tr("Group: %1").arg(GroupName), Group);
		}
	}

	foreach(const QString & Name, m_Programs)
		progDialog.addItem(Name, Name);

	progDialog.setValue("");

	if (theGUI->SafeExec(&progDialog) != QDialog::Accepted)
		return QString();

	// Note: pressing enter adds the value to the combo list !
	QString Program = progDialog.value(); 
	int Index = progDialog.findValue(Program);
	if (Index != -1 && progDialog.data().isValid())
		Program = progDialog.data().toString();

	return Program;
}

void COptionsWindow::OnTab(QWidget* pTab)
{
	m_pCurrentTab = pTab;

	if (pTab == ui.tabEdit)
	{
		LoadIniSection();
		//ui.txtIniSection->setReadOnly(true);
	}
	else 
	{
		if (m_ConfigDirty)
			LoadConfig();

		UpdateCurrentTab();
	}
}

void COptionsWindow::UpdateCurrentTab()
{
	if (m_pCurrentTab == ui.tabRestrictions || m_pCurrentTab == ui.tabOptions || m_pCurrentTab == ui.tabGeneral) 
	{
		ui.chkVmRead->setChecked(IsAccessEntrySet(eIPC, "", eReadOnly, "$:*"));
	}
	else if (m_pCurrentTab == ui.tabStart || m_pCurrentTab == ui.tabForce)
	{
		if (IsAccessEntrySet(eIPC, "!<StartRunAccess>", eClosed, "*"))
			ui.radStartSelected->setChecked(true);
		else if (IsAccessEntrySet(eIPC, "<StartRunAccess>", eClosed, "*"))
			ui.radStartExcept->setChecked(true);
		else
			ui.radStartAll->setChecked(true);
		ui.treeStart->clear();
		CopyGroupToList("<StartRunAccess>", ui.treeStart);
		CopyGroupToList("<StartRunAccessDisabled>", ui.treeStart, true);

		OnRestrictStart();
	}
	else if (m_pCurrentTab == ui.tabInternet || m_pCurrentTab == ui.tabINet || m_pCurrentTab == ui.tabNetConfig)
	{
		if (!m_INetBlockChanged)
			LoadBlockINet();
	}
	else if (m_pCurrentTab == ui.tabDNS || m_pCurrentTab == ui.tabNetProxy)
	{
		if (!m_HoldChange && !m_pCurrentTab->isEnabled())
			theGUI->CheckCertificate(this, 2);
	}
	else if (m_pCurrentTab == ui.tabCOM) {
		CheckOpenCOM();
	}
	else if (m_pCurrentTab == ui.tabWnd)
	{
		if (IsAccessEntrySet(eWnd, "", eOpen, "*"))
		{
			ui.chkNoWindowRename->setEnabled(false);
			ui.chkNoWindowRename->setChecked(true);
		}
		else
		{
			ui.chkNoWindowRename->setEnabled(true);
			ui.chkNoWindowRename->setChecked(IsAccessEntrySet(eWnd, "", eNoRename, "*"));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Raw section ini Editor
//

void COptionsWindow::SetIniEdit(bool bEnable)
{
	if (m_pTree) {
		m_pTree->setEnabled(!bEnable);
	}
	else {
		for (int i = 0; i < ui.tabs->count() - 1; i++) {
			bool Enabled = ui.tabs->widget(i)->isEnabled();
			ui.tabs->setTabEnabled(i, !bEnable && Enabled);
			ui.tabs->widget(i)->setEnabled(Enabled);
		}
	}
	ui.btnSaveIni->setEnabled(bEnable);
	ui.btnCancelEdit->setEnabled(bEnable);
	//ui.txtIniSection->setReadOnly(!bEnable);
	ui.btnEditIni->setEnabled(!bEnable);
}

void COptionsWindow::OnEditIni()
{
	SetIniEdit(true);
}

void COptionsWindow::OnIniValidationToggled(int state)
{
	m_HoldChange = true;

	m_IniValidationEnabled = (state == Qt::Checked);
	
	// Only save to config if not in a reset-skip context
	if (!m_SkipSaveOnToggle) {
		theConf->SetValue("Options/ValidateIniKeys", m_IniValidationEnabled);
	}

	if (state == Qt::Unchecked) {
		CIniHighlighter::MarkSettingsDirty();
		CIniHighlighter::MarkUserSettingsDirty();
	}

	if (m_pIniHighlighter) {
		delete m_pIniHighlighter;
		m_pIniHighlighter = nullptr;
	}

	QTextEdit* pTextEdit = m_pCodeEdit->findChild<QTextEdit*>();
	if (pTextEdit) {
		m_pIniHighlighter = new CIniHighlighter(theGUI->m_DarkTheme, pTextEdit->document(), m_IniValidationEnabled);
		m_pIniHighlighter->rehighlight();
	}

	m_HoldChange = false;
}

void COptionsWindow::OnTooltipToggled(int state)
{
	m_HoldChange = true;

	// Only save to config if not in a reset-skip context
	if (!m_SkipSaveOnToggle) {
		theConf->SetValue("Options/EnableIniTooltips", state);
	}

	CIniHighlighter::SetTooltipMode(state);

	{
		int iniMode = theConf->GetInt("Options/EnableIniTooltips", static_cast<int>(CIniHighlighter::GetTooltipMode()));
		int popupMode = theConf->GetInt("Options/EnablePopupTooltips", iniMode);
		CCodeEdit::SetPopupTooltipsEnabled(popupMode);
	}

	if (state == Qt::Unchecked) {
		CIniHighlighter::ClearLanguageCache();
		CIniHighlighter::ClearThemeCache();
	}

	m_HoldChange = false;
}

void COptionsWindow::OnAutoCompletionToggled(int state)
{
	m_HoldChange = true;

	// Show consent dialog if enabling and not yet consented
	if (state != Qt::Unchecked && !m_AutoCompletionConsent) {
		int chosenState = ShowConsentDialog();
		
		if (chosenState == Qt::Unchecked) {
			// Cancel - revert the checkbox and return
			ui.chkEnableAutoCompletion->setCheckState(Qt::Unchecked);
			m_HoldChange = false;
			return;
		}
		
		// Consent was given, update UI and state
		ui.chkEnableAutoCompletion->setEnabled(true);
		ui.chkEnableAutoCompletion->setTristate(true);
		ui.chkEnableAutoCompletion->setCheckState(static_cast<Qt::CheckState>(chosenState));
		state = chosenState;
	}

	// Only save to config if not in a reset-skip context
	if (!m_SkipSaveOnToggle) {
		theConf->SetValue("Options/EnableAutoCompletion", state);
	}

	CCodeEdit::SetAutoCompletionMode(state); // Use static method like tooltip

	// Enable or disable the completer based on mode
	if (m_pCodeEdit) {
		if (CCodeEdit::GetAutoCompletionMode() != CCodeEdit::AutoCompletionMode::Disabled) {
			// Create completer if it doesn't exist
			if (!m_pCodeEdit->GetCompleter()) {
				QCompleter* completer = new QCompleter(this);
				completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
				completer->setFilterMode(Qt::MatchContains);
				m_pCodeEdit->SetCompleter(completer);

				// Update completion model with current settings
				UpdateAutoCompletion();
			}
		}
		else {
			// Disable completer
			m_pCodeEdit->SetCompleter(nullptr);
			CCodeEdit::ClearFuzzyCache();
		}
	}

	m_HoldChange = false;
}

void COptionsWindow::OnEditorSettings()
{
	CEditorSettingsWindow editorWindow(this);
	if (theGUI->SafeExec(&editorWindow) == QDialog::Accepted) {
		// Settings were saved by the dialog, now update the current UI to reflect changes
		bool previousConsent = m_AutoCompletionConsent;
		LoadCompletionConsent();
		bool newConsent = m_AutoCompletionConsent;
		
		// If consent was just granted (changed from false to true), show the consent dialog
		if (!previousConsent && newConsent) {
			int chosenState = ShowConsentDialog();
			
			// Save the chosen autocomplete mode to config
			theConf->SetValue("Options/EnableAutoCompletion", chosenState);
		}
		
		// Update the current checkboxes to reflect the new settings
		// Note: OptionsWindow only has UI checkboxes for 3 settings:
		// - ValidateIniKeys (ui.chkValidateIniKeys)
		// - EnableIniTooltips (ui.chkEnableTooltips)
		// - EnableAutoCompletion (ui.chkEnableAutoCompletion)
		// The other 3 settings (EnablePopupTooltips, EnableFuzzyMatching, AutoCompletionConsent)
		// are managed by EditorSettings but don't have corresponding UI in OptionsWindow
		
		// Block signals while updating checkboxes to prevent toggle handlers from being called prematurely
		ui.chkValidateIniKeys->blockSignals(true);
		ui.chkEnableTooltips->blockSignals(true);
		ui.chkEnableAutoCompletion->blockSignals(true);
		
		// Read current values from config (will be defaults if settings were reset/deleted)
		bool defaultValidation = theConf->GetBool("Options/ValidateIniKeys", true);
		ui.chkValidateIniKeys->setChecked(defaultValidation);
		
		int defaultTooltip = theConf->GetInt("Options/EnableIniTooltips", 1); // 1 = BasicInfo
		ui.chkEnableTooltips->setCheckState(static_cast<Qt::CheckState>(defaultTooltip));
		
		int defaultAutoCompletion = theConf->GetInt("Options/EnableAutoCompletion", 0); // 0 = Disabled
		if (m_AutoCompletionConsent) { // Consented
			ui.chkEnableAutoCompletion->setTristate(true);
			ui.chkEnableAutoCompletion->setCheckState(static_cast<Qt::CheckState>(defaultAutoCompletion));
		}
		else {
			ui.chkEnableAutoCompletion->setTristate(false);
			ui.chkEnableAutoCompletion->setCheckState(Qt::Unchecked);
		}
		
		// Unblock signals before calling toggle handlers manually
		ui.chkValidateIniKeys->blockSignals(false);
		ui.chkEnableTooltips->blockSignals(false);
		ui.chkEnableAutoCompletion->blockSignals(false);
		
		// Apply the settings immediately
		// Set skip flag for reset settings to prevent re-saving them to config
		// For non-reset settings, allow normal save behavior
		
		// ValidateIniKeys
		m_SkipSaveOnToggle = editorWindow.WasValidateIniKeysReset();
		OnIniValidationToggled(defaultValidation ? Qt::Checked : Qt::Unchecked);
		m_SkipSaveOnToggle = false;
		
		// EnableIniTooltips
		m_SkipSaveOnToggle = editorWindow.WasEnableIniTooltipsReset();
		OnTooltipToggled(defaultTooltip);
		m_SkipSaveOnToggle = false;
		
		// EnableAutoCompletion
		m_SkipSaveOnToggle = editorWindow.WasEnableAutoCompletionReset();
		OnAutoCompletionToggled(defaultAutoCompletion);
		m_SkipSaveOnToggle = false;
		
		// Apply settings that don't have UI checkboxes in OptionsWindow
		// These are managed via EditorSettings only
		if (editorWindow.HasResetOccurred()) {
			// If any reset occurred, update these settings from config
			bool fuzzyEnabled = theConf->GetBool("Options/EnableFuzzyMatching", false);
			m_pCodeEdit->SetFuzzyMatchingEnabled(fuzzyEnabled);
		}
		
		// Always update autocompletion list regardless of reset status
		UpdateAutoCompletion();
	}
}

void COptionsWindow::OnSaveIni()
{
	SaveIniSection();
	SetIniEdit(false);
}

void COptionsWindow::OnIniChanged()
{
	if (m_HoldChange)
		return;
	if(ui.btnEditIni->isEnabled())
		SetIniEdit(true);
	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void COptionsWindow::OnCancelEdit()
{
	SetIniEdit(false);
	LoadIniSection();
}

void COptionsWindow::LoadIniSection()
{
	QString Section;

	// Note: the service only caches sandboxie.ini not templates.ini, hence for global templates we need to load the section through the driver
	if (m_Template && m_pBox->GetName().mid(9, 6).compare("Local_", Qt::CaseInsensitive) != 0)
	{
		m_Settings = m_pBox->GetIniSection(NULL, m_Template);

		for (QList<CSbieIni::SbieIniValue>::const_iterator I = m_Settings.begin(); I != m_Settings.end(); ++I)
			Section += I->Name + "=" + I->Value + "\n";
	}
	else
		Section = m_pBox->SbieIniGetEx(m_pBox->GetName(), "");

	m_HoldChange = true;
	//ui.txtIniSection->setPlainText(Section);
	m_pCodeEdit->SetCode(Section);
	m_HoldChange = false;
}

void COptionsWindow::SaveIniSection()
{
	m_ConfigDirty = true;

	/*m_pBox->SetRefreshOnChange(false);

	// Note: an incremental update would be more elegant but it would change the entry order in the ini,
	//			hence it's better for the user to fully rebuild the section each time.
	//
	for (QList<QPair<QString, QString>>::const_iterator I = m_Settings.begin(); I != m_Settings.end(); ++I)
		m_pBox->DelValue(I->first, I->second);

	//QList<QPair<QString, QString>> NewSettings;
	//QList<QPair<QString, QString>> OldSettings = m_Settings;

	QStringList Section = SplitStr(ui.txtIniSection->toPlainText(), "\n");
	foreach(const QString& Line, Section)
	{
		if (Line.isEmpty())
			return;
		StrPair Settings = Split2(Line, "=");
		
		//if (!OldSettings.removeOne(Settings))
		//	NewSettings.append(Settings);

		m_pBox->AppendText(Settings.first, Settings.second);
	}

	//for (QList<QPair<QString, QString>>::const_iterator I = OldSettings.begin(); I != OldSettings.end(); ++I)
	//	m_pBox->DelValue(I->first, I->second);
	//
	//for (QList<QPair<QString, QString>>::const_iterator I = NewSettings.begin(); I != NewSettings.end(); ++I)
	//	m_pBox->AppendText(I->first, I->second);

	m_pBox->SetRefreshOnChange(true);
	m_pBox->CommitIniChanges();*/

	//m_pBox->GetAPI()->SbieIniSet(m_pBox->GetName(), "", ui.txtIniSection->toPlainText());
	m_pBox->SbieIniSet(m_pBox->GetName(), "", m_pCodeEdit->GetCode());

	//LoadIniSection();
}

#include "OptionsAccess.cpp"
#include "OptionsAdvanced.cpp"
#include "OptionsForce.cpp"
#include "OptionsGeneral.cpp"
#include "OptionsGrouping.cpp"
#include "OptionsNetwork.cpp"
#include "OptionsRecovery.cpp"
#include "OptionsStart.cpp"
#include "OptionsStop.cpp"
#include "OptionsTemplates.cpp"

#include <windows.h>

void COptionsWindow::TriggerPathReload()
{
	//
	// this message makes all boxes reload their path presets
	//

	DWORD bsm_app = BSM_APPLICATIONS;
	BroadcastSystemMessage(BSF_POSTMESSAGE, &bsm_app, WM_DEVICECHANGE, 'sb', 0);
}

// Helper to load/save consent from config
void COptionsWindow::LoadCompletionConsent()
{
	m_AutoCompletionConsent = theConf->GetBool("Options/AutoCompletionConsent", false);
}

void COptionsWindow::SaveCompletionConsent()
{
	theConf->SetValue("Options/AutoCompletionConsent", m_AutoCompletionConsent);
}

QString COptionsWindow::localizedCompletionShortcut()
{
	QKeySequence shortcut = QKeySequence(Qt::CTRL + Qt::Key_Space);
	return shortcut.toString(QKeySequence::NativeText); // Returns the localized shortcut
}

// Show consent dialog and return the chosen autocomplete state
// Returns: Qt::Unchecked (0) if cancelled, Qt::PartiallyChecked (1) for Basic, Qt::Checked (2) for Full
int COptionsWindow::ShowConsentDialog()
{
	QMessageBox consentBox(this);
	consentBox.setWindowTitle(tr("Autocomplete Consent Required"));
	consentBox.setIcon(QMessageBox::Question);
	consentBox.setText(tr("Autocomplete feature requires your consent to proceed."));
	consentBox.setInformativeText(
		tr("If you are unsure about the settings displayed in the autocomplete popup, we strongly recommend consulting the software's documentation or source code before proceeding. Enabling this feature without proper understanding may lead to unintended consequences, for which you will be solely responsible.\n\n"
			"Choose autocomplete mode:\n"
			"%1 Manual: Autocomplete suggestions with %2.\n"
			"%1 While Typing: Autocomplete suggestions while typing.")
		.arg(QChar(0x2022))   // Bullet symbol
		.arg(localizedCompletionShortcut()) // Localized Ctrl+Space
	);

	QPushButton* basicButton = consentBox.addButton(tr("Manual"), QMessageBox::YesRole);
	basicButton->setToolTip(tr("Triggers autocomplete suggestions with %1.").arg(localizedCompletionShortcut()));

	QPushButton* fullButton = consentBox.addButton(tr("While Typing"), QMessageBox::YesRole);
	fullButton->setToolTip(tr("Triggers autocomplete suggestions while typing."));

	QPushButton* cancelButton = consentBox.addButton(tr("Cancel"), QMessageBox::NoRole);
	cancelButton->setToolTip(tr("Keeps autocomplete suggestions disabled."));
	
	consentBox.setDefaultButton(basicButton);
	
	consentBox.exec();
	QAbstractButton* clickedButton = consentBox.clickedButton();
	
	if (clickedButton == basicButton) {
		m_AutoCompletionConsent = true;
		SaveCompletionConsent();
		return Qt::PartiallyChecked; // Basic mode
	}
	else if (clickedButton == fullButton) {
		m_AutoCompletionConsent = true;
		SaveCompletionConsent();
		return Qt::Checked; // Full mode
	}
	else { // Cancel
		m_AutoCompletionConsent = false;
		SaveCompletionConsent();
		return Qt::Unchecked; // Cancelled
	}
}
