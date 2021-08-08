#include "stdafx.h"
#include "SelectBoxWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../SbiePlusAPI.h"

#if defined(Q_OS_WIN)
#include <wtypes.h>
#include <QAbstractNativeEventFilter>
#include <dbt.h>
#endif

CSelectBoxWindow::CSelectBoxWindow(const QStringList& Commands, const QString& BoxName, QWidget *parent)
	: QDialog(parent)
{
	m_Commands = Commands;

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	//flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	//flags &= ~Qt::WindowMinMaxButtonsHint;
	//flags |= Qt::WindowMinimizeButtonHint;
	//flags &= ~Qt::WindowCloseButtonHint;
	flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	setWindowFlags(flags);

	//setWindowState(Qt::WindowActive);
	SetForegroundWindow((HWND)QWidget::winId());

	ui.setupUi(this);
	this->setWindowTitle(tr("Sandboxie-Plus - Run Sandboxed"));


	connect(ui.radBoxed, SIGNAL(clicked(bool)), this, SLOT(OnBoxType()));
	connect(ui.radUnBoxed, SIGNAL(clicked(bool)), this, SLOT(OnBoxType()));

	connect(ui.buttonBox, SIGNAL(accepted()), SLOT(OnRun()));
	connect(ui.buttonBox, SIGNAL(rejected()), SLOT(reject()));

	QMap<QString, CSandBoxPtr> Boxes = theAPI->GetAllBoxes();

	foreach(const CSandBoxPtr & pBox, Boxes) 
	{
		if (!pBox->IsEnabled() || !pBox->GetBool("ShowForRunIn", true))
			continue;

		CSandBoxPlus* pBoxEx = qobject_cast<CSandBoxPlus*>(pBox.data());

		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		pItem->setText(0, pBox->GetName().replace("_", " "));
		pItem->setData(0, Qt::UserRole, pBox->GetName());
		pItem->setData(0, Qt::DecorationRole, theGUI->GetBoxIcon(pBox->GetActiveProcessCount(), pBoxEx->GetType()));
		ui.treeBoxes->addTopLevelItem(pItem);

		if (pBox->GetName().compare(BoxName, Qt::CaseInsensitive) == 0)
			ui.treeBoxes->setCurrentItem(pItem);
	}

	//restoreGeometry(theConf->GetBlob("SelectBoxWindow/Window_Geometry"));
}

CSelectBoxWindow::~CSelectBoxWindow()
{
	//theConf->SetBlob("SelectBoxWindow/Window_Geometry", saveGeometry());
}

void CSelectBoxWindow::closeEvent(QCloseEvent *e)
{
	//emit Closed();
	this->deleteLater();
}

void CSelectBoxWindow::OnBoxType()
{
	ui.treeBoxes->setEnabled(!ui.radUnBoxed->isChecked());
}

void CSelectBoxWindow::OnRun()
{
	QTreeWidgetItem* pItem = ui.treeBoxes->currentItem();

	QString BoxName;
	if (ui.radUnBoxed->isChecked())
	{
		if (QMessageBox("Sandboxie-Plus", tr("Are you sure you want to run the program outside the sandbox?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			return;
		pItem = NULL;
	}
	else if (pItem == NULL) {
		QMessageBox("Sandboxie-Plus", tr("Please select a sandbox."), QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this).exec();
		return;
	}
	else {
		BoxName = pItem->data(0, Qt::UserRole).toString();
	}


	//QList<SB_STATUS> Results;
	foreach(const QString & Command, m_Commands) {
		theAPI->RunStart(BoxName, Command, NULL, ui.chkAdmin->isChecked());
	}
	//CSandMan::CheckResults(Results);

	close();
}