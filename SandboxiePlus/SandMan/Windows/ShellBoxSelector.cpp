#include "stdafx.h"
#include "ShellBoxSelector.h"
#include "../SandMan.h"
#include "ui_ShellBoxSelector.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QDialog>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

namespace
{

class CShellBoxItemDelegate : public QStyledItemDelegate
{
public:
	explicit CShellBoxItemDelegate(QObject* pParent) : QStyledItemDelegate(pParent) {}

protected:
	void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
	{
		QStyledItemDelegate::initStyleOption(option, index);
		option->text = index.data(Qt::UserRole + 1).toString();
	}
};

class CShellBoxCompleter : public QCompleter
{
public:
	explicit CShellBoxCompleter(QAbstractItemModel* pModel, QObject* pParent) : QCompleter(pModel, pParent) {}

	QString pathFromIndex(const QModelIndex& index) const override
	{
		return index.data(Qt::UserRole).toString();
	}
};

}

CSandBoxPtr SelectBoxForShellAction(QWidget* pParent)
{
	QList<CSandBoxPtr> Boxes;
	foreach(const CSandBoxPtr& pBox, theAPI->GetAllBoxes()) {
		if (pBox->IsEnabled())
			Boxes.append(pBox);
	}
	if (Boxes.isEmpty()) {
		QMessageBox::warning(pParent, CSandMan::tr("Sandboxie-Plus Warning"),
			CSandMan::tr("No enabled sandbox is available. Please create or enable a sandbox and try again."), QMessageBox::Ok, 0);
		return CSandBoxPtr();
	}
	bool skipSingleBox = theConf->GetBool("Options/SkipSingleBoxForShellActions", false);
	if (Boxes.count() == 1 && skipSingleBox)
		return Boxes.first();

	QDialog boxDialog(pParent);
	Ui::ShellBoxSelector ui;
	ui.setupUi(&boxDialog);
	boxDialog.setWindowTitle(CSandMan::tr("Which box you want to add in?"));
	boxDialog.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	QComboBox* pBoxes = ui.cmbBoxes;
	pBoxes->setItemDelegate(new CShellBoxItemDelegate(pBoxes));
	foreach(const CSandBoxPtr& pBox, Boxes) {
		QString name = pBox->GetName();
		QString alias = pBox->GetText("BoxAlias").trimmed();
		QString label = alias.isEmpty() || alias.compare(name, Qt::CaseInsensitive) == 0
			? name : CSandBoxPlus::FormatDisplayName(name, alias, 2);
		// Keep canonical names in the editor; only the popup paints alias labels.
		pBoxes->addItem(name, name);
		pBoxes->setItemData(pBoxes->count() - 1, label, Qt::UserRole + 1);
	}
	CShellBoxCompleter* pCompleter = new CShellBoxCompleter(pBoxes->model(), pBoxes);
	pCompleter->setCompletionMode(QCompleter::PopupCompletion);
	pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	pCompleter->setFilterMode(Qt::MatchContains);
	pCompleter->setCompletionRole(Qt::UserRole + 1);
	pCompleter->popup()->setItemDelegate(new CShellBoxItemDelegate(pCompleter->popup()));
	pBoxes->setCompleter(pCompleter);
	pBoxes->setCurrentIndex(Boxes.count() == 1 ? 0 : -1);
	QCheckBox* pSkipSingleBox = ui.chkSkipSingleBox;
	pSkipSingleBox->setChecked(skipSingleBox);
	pBoxes->setFocus();

	if (theGUI->SafeExec(&boxDialog) != QDialog::Accepted || pBoxes->currentText().trimmed().isEmpty()) {
		QMessageBox::warning(pParent, CSandMan::tr("Sandboxie-Plus Warning"), CSandMan::tr("User canceled this operation."), QMessageBox::Yes, 0);
		return CSandBoxPtr();
	}

	QString response = pBoxes->currentText().trimmed();
	// Editable text can differ from the item whose data is still selected.
	int index = pBoxes->currentIndex();
	if (index >= 0 && response == pBoxes->itemText(index))
		response = pBoxes->itemData(index).toString();

	CSandBoxPtr pBox = theAPI->GetBoxByName(response);
	if (!pBox) {
		foreach(const CSandBoxPtr& pCandidate, theAPI->GetAllBoxes()) {
			if (pCandidate->GetText("BoxAlias").trimmed().compare(response, Qt::CaseInsensitive) != 0)
				continue;
			if (pBox) {
				QMessageBox::warning(pParent, CSandMan::tr("Sandboxie-Plus Warning"),
					CSandMan::tr("More than one sandbox has this alias. Select a sandbox from the list or type its actual name. Nothing was changed."), QMessageBox::Ok, 0);
				return CSandBoxPtr();
			}
			pBox = pCandidate;
		}
	}
	if (!pBox)
		QMessageBox::warning(pParent, CSandMan::tr("Sandboxie-Plus Warning"), CSandMan::tr("You typed a wrong box name! Nothing was changed."), QMessageBox::Ok, 0);
	else
		theConf->SetValue("Options/SkipSingleBoxForShellActions", pSkipSingleBox->isChecked());
	return pBox;
}
