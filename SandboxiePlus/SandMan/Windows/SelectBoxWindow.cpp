#include "stdafx.h"
#include "SelectBoxWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../SbiePlusAPI.h"
#include "../Views/SbieView.h"
#include "../MiscHelpers/Common/Finder.h"

#if defined(Q_OS_WIN)
#include <wtypes.h>
#include <QAbstractNativeEventFilter>
#include <dbt.h>
#endif

//////////////////////////////////////////////////////////////////////////////////////
// CBoxPicker
//

CBoxPicker::CBoxPicker(QString DefaultBox, QWidget* parent)
	: QWidget(parent)
{
	m_pTreeBoxes = new QTreeWidget();
	m_pTreeBoxes->setHeaderLabels(tr("Sandbox").split("|"));
	connect(m_pTreeBoxes, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SIGNAL(BoxDblClick()));
	m_pTreeBoxes->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));
	QVBoxLayout* pLayout = new QVBoxLayout(this);
	pLayout->setContentsMargins(0, 0, 0, 0);
	pLayout->addWidget(new CFinder(this, this, 0));
	pLayout->insertWidget(0, m_pTreeBoxes);
	
	this->setMaximumWidth(300);

	if(DefaultBox.isEmpty() && theAPI->IsConnected())
		DefaultBox = theAPI->GetGlobalSettings()->GetText("DefaultBox", "DefaultBox");

	LoadBoxed("", DefaultBox);
}

void CBoxPicker::SetFilter(const QString& Exp, int iOptions, int Column)
{
	LoadBoxed(Exp);
}

void CBoxPicker::LoadBoxed(const QString& Filter, const QString& SelectBox)
{
	m_pTreeBoxes->clear();

	QList<CSandBoxPtr> Boxes = theAPI->GetAllBoxes().values(); // map is sorted by key (box name)
	QMap<QString, QStringList> Groups = theGUI->GetBoxView()->GetGroups();

	if (theConf->GetBool("MainWindow/BoxTree_UseOrder", false)) {
		QMultiMap<double, CSandBoxPtr> Boxes2;
		foreach(const CSandBoxPtr &pBox, Boxes) {
			Boxes2.insertMulti(GetBoxOrder(Groups, pBox->GetName()), pBox);
		}
		Boxes = Boxes2.values();
	}

	QFileIconProvider IconProvider;
	bool ColorIcons = theConf->GetBool("Options/ColorBoxIcons", false);

	QMap<QString, QTreeWidgetItem*> GroupItems;
	foreach(const CSandBoxPtr &pBox, Boxes) 
	{
		if (!pBox->IsEnabled() || !pBox->GetBool("ShowForRunIn", true))
			continue;

		if (!Filter.isEmpty() && !pBox->GetName().contains(Filter, Qt::CaseInsensitive))
			continue;

		CSandBoxPlus* pBoxEx = qobject_cast<CSandBoxPlus*>(pBox.data());

		QTreeWidgetItem* pParent = GetBoxParent(Groups, GroupItems, m_pTreeBoxes, pBox->GetName());
		
		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		pItem->setText(0, pBox->GetName().replace("_", " "));
		pItem->setData(0, Qt::UserRole, pBox->GetName());
		QIcon Icon;
		QString Action = pBox->GetText("DblClickAction");
		if (!Action.isEmpty() && Action.left(1) != "!")
			Icon = IconProvider.icon(QFileInfo(pBoxEx->GetCommandFile(Action)));
		else if(ColorIcons)
			Icon = theGUI->GetColorIcon(pBoxEx->GetColor(), pBox->GetActiveProcessCount());
		else
			Icon = theGUI->GetBoxIcon(pBoxEx->GetType(), pBox->GetActiveProcessCount() != 0);
		pItem->setData(0, Qt::DecorationRole, Icon);
		if (pParent)
			pParent->addChild(pItem);
		else
			m_pTreeBoxes->addTopLevelItem(pItem);

		if (pBox->GetName().compare(SelectBox, Qt::CaseInsensitive) == 0)
			m_pTreeBoxes->setCurrentItem(pItem);
	}

	m_pTreeBoxes->expandAll();
}

QString CBoxPicker::GetBoxName() const
{
	auto pItem = m_pTreeBoxes->currentItem();
	if (!pItem) return QString();
	return pItem->data(0, Qt::UserRole).toString();
}

QTreeWidgetItem* CBoxPicker::GetBoxParent(const QMap<QString, QStringList>& Groups, QMap<QString, QTreeWidgetItem*>& GroupItems, QTreeWidget* treeBoxes, const QString& Name, int Depth)
{
	if (Depth > 100)
		return NULL;
	for (auto I = Groups.constBegin(); I != Groups.constEnd(); ++I) {
		if (I->contains(Name)) {
			if (I.key().isEmpty())
				return NULL; // global group
			QTreeWidgetItem*& pParent = GroupItems[I.key()];
			if (!pParent) {
				pParent = new QTreeWidgetItem();
				pParent->setText(0, I.key());
				QFont fnt = pParent->font(0);
				fnt.setBold(true);
				pParent->setFont(0, fnt);
				if (QTreeWidgetItem* pParent2 = GetBoxParent(Groups, GroupItems, treeBoxes, I.key(), ++Depth))
					pParent2->addChild(pParent);
				else
					treeBoxes->addTopLevelItem(pParent);
			}
			return pParent;
		}
	}
	return NULL;
}

double CBoxPicker::GetBoxOrder(const QMap<QString, QStringList>& Groups, const QString& Name, double value, int Depth) 
{
	if (Depth > 100)
		return 1000000000;
	for (auto I = Groups.constBegin(); I != Groups.constEnd(); ++I) {
		int Pos = I->indexOf(Name);
		if (Pos != -1) {
			value = double(Pos) + value / 10.0;
			if (I.key().isEmpty())
				return value;
			return GetBoxOrder(Groups, I.key(), value, ++Depth);
		}
	}
	return 1000000000;
}

//////////////////////////////////////////////////////////////////////////////////////
// CSelectBoxWindow
//

CSelectBoxWindow::CSelectBoxWindow(const QStringList& Commands, const QString& BoxName, const QString& WrkDir, QWidget *parent)
	: QDialog(parent)
{
	m_Commands = Commands;
	m_WrkDir = WrkDir;

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

	bool bAlwaysOnTop = theGUI->IsAlwaysOnTop();
	this->setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);

	if (!bAlwaysOnTop) {
		HWND hWnd = (HWND)this->winId();
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		QTimer::singleShot(100, this, [hWnd]() {
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		});
	}

	ui.setupUi(this);
	this->setWindowTitle(tr("Sandboxie-Plus - Run Sandboxed"));

	connect(ui.radBoxed, SIGNAL(clicked(bool)), this, SLOT(OnBoxType()));
	connect(ui.radBoxedNew, SIGNAL(clicked(bool)), this, SLOT(OnBoxType()));
	connect(ui.radUnBoxed, SIGNAL(clicked(bool)), this, SLOT(OnBoxType()));

	connect(ui.buttonBox, SIGNAL(accepted()), SLOT(OnRun()));
	connect(ui.buttonBox, SIGNAL(rejected()), SLOT(reject()));

	m_pBoxPicker = new CBoxPicker(BoxName);
	connect(m_pBoxPicker, SIGNAL(BoxDblClick()), this, SLOT(OnRun()));
	ui.treeBoxes->parentWidget()->layout()->replaceWidget(ui.treeBoxes, m_pBoxPicker);
	delete ui.treeBoxes;

	m_pBoxPicker->setFocus();

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
	m_pBoxPicker->setEnabled(ui.radBoxed->isChecked());
}

void CSelectBoxWindow::OnRun()
{
	QString BoxName;

	if (ui.radUnBoxed->isChecked())
	{
		if (QMessageBox("Sandboxie-Plus", tr("Are you sure you want to run the program outside the sandbox?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
			return;
	}
	else  if (ui.radBoxedNew->isChecked())
	{
		BoxName = theGUI->GetBoxView()->AddNewBox(true);
		if (BoxName.isEmpty()) {
			close();
			return;
		}
	}
	else 
	{
		BoxName = m_pBoxPicker->GetBoxName();
		if (BoxName.isEmpty()) {
			QMessageBox("Sandboxie-Plus", tr("Please select a sandbox."), QMessageBox::Information, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this).exec();
			return;
		}
	}

	foreach(const QString & Command, m_Commands)
		theGUI->RunStart(BoxName, Command, ui.chkAdmin->isChecked(), m_WrkDir);

	setResult(1);
	close();
}