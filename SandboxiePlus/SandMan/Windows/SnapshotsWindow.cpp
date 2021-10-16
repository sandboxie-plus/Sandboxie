#include "stdafx.h"
#include "SnapshotsWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/TreeItemModel.h"


CSnapshotsWindow::CSnapshotsWindow(const CSandBoxPtr& pBox, QWidget *parent)
	: QDialog(parent)
{
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	//flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	//flags &= ~Qt::WindowMinMaxButtonsHint;
	flags |= Qt::WindowMinimizeButtonHint;
	//flags &= ~Qt::WindowCloseButtonHint;
	setWindowFlags(flags);

	bool bAlwaysOnTop = theConf->GetBool("Options/AlwaysOnTop", false);
	this->setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);

	ui.setupUi(this);
	this->setWindowTitle(tr("%1 - Snapshots").arg(pBox->GetName()));

	m_pBox = pBox;
	m_SaveInfoPending = 0;

#ifdef WIN32
	QStyle* pStyle = QStyleFactory::create("windows");
	ui.treeSnapshots->setStyle(pStyle);
#endif
	ui.treeSnapshots->setExpandsOnDoubleClick(false);

	m_pSnapshotModel = new CSimpleTreeModel();
	m_pSnapshotModel->AddColumn(tr("Snapshot"), "Name");

	/*m_pSortProxy = new CSortFilterProxyModel(false, this);
	m_pSortProxy->setSortRole(Qt::EditRole);
	m_pSortProxy->setSourceModel(m_pSnapshotModel);
	m_pSortProxy->setDynamicSortFilter(true);*/

	//ui.treeSnapshots->setItemDelegate(theGUI->GetItemDelegate());

	//ui.treeSnapshots->setModel(m_pSortProxy);
	ui.treeSnapshots->setModel(m_pSnapshotModel);

	connect(ui.treeSnapshots, SIGNAL(clicked(const QModelIndex&)), this, SLOT(UpdateSnapshot(const QModelIndex&)));
	connect(ui.treeSnapshots->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(UpdateSnapshot(const QModelIndex&)));
	connect(ui.treeSnapshots, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnSelectSnapshot()));

	connect(ui.btnTake, SIGNAL(clicked(bool)), this, SLOT(OnTakeSnapshot()));
	connect(ui.btnSelect, SIGNAL(clicked(bool)), this, SLOT(OnSelectSnapshot()));
	connect(ui.btnRemove, SIGNAL(clicked(bool)), this, SLOT(OnRemoveSnapshot()));
	
	connect(ui.txtName, SIGNAL(textEdited(const QString&)), this, SLOT(SaveInfo()));
	connect(ui.txtInfo, SIGNAL(textChanged()), this, SLOT(SaveInfo()));

	ui.groupBox->setEnabled(false);
	ui.btnSelect->setEnabled(false);
	ui.btnRemove->setEnabled(false);

	//statusBar();

	restoreGeometry(theConf->GetBlob("SnapshotsWindow/Window_Geometry"));

	for (int i = 0; i < m_pSnapshotModel->columnCount(); i++)
		m_pSnapshotModel->SetColumnEnabled(i, true);

	UpdateSnapshots(true);
}

CSnapshotsWindow::~CSnapshotsWindow()
{
	theConf->SetBlob("SnapshotsWindow/Window_Geometry",saveGeometry());
}

void CSnapshotsWindow::closeEvent(QCloseEvent *e)
{
	emit Closed();
	this->deleteLater();
}

void CSnapshotsWindow::UpdateSnapshots(bool AndSelect)
{
	m_SnapshotMap.clear();
	QList<SBoxSnapshot> SnapshotList = m_pBox->GetSnapshots(&m_CurSnapshot);
	foreach(const SBoxSnapshot& Snapshot, SnapshotList)
	{
		QVariantMap BoxSnapshot;
		BoxSnapshot["ID"] = Snapshot.ID;
		BoxSnapshot["ParentID"] = Snapshot.Parent;
		BoxSnapshot["Name"] = Snapshot.NameStr;
		BoxSnapshot["Info"] = Snapshot.InfoStr;
		BoxSnapshot["Date"] = Snapshot.SnapDate;
		if(m_CurSnapshot == Snapshot.ID)
			BoxSnapshot["IsBold"] = true;
		m_SnapshotMap.insert(Snapshot.ID, BoxSnapshot);
	}
	m_pSnapshotModel->Sync(m_SnapshotMap);
	ui.treeSnapshots->expandAll();

	if (AndSelect)
	{
		QModelIndex CurIndex = m_pSnapshotModel->FindIndex(m_CurSnapshot);
		if (CurIndex.isValid()) {
			ui.treeSnapshots->selectionModel()->select(CurIndex, QItemSelectionModel::ClearAndSelect);
			UpdateSnapshot(CurIndex);
		}
	}
}

void CSnapshotsWindow::UpdateSnapshot(const QModelIndex& Index)
{
	if (Index.isValid())
	{
		ui.groupBox->setEnabled(true);
		ui.btnSelect->setEnabled(true);
		ui.btnRemove->setEnabled(true);
	}

	//QModelIndex Index = ui.treeSnapshots->currentIndex();
	//QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
	//QVariant ID = m_pSnapshotModel->GetItemID(ModelIndex);
	QVariant ID = m_pSnapshotModel->GetItemID(Index);

	OnSaveInfo();
	m_SellectedID = ID;

	QVariantMap BoxSnapshot = m_SnapshotMap[ID];

	m_SaveInfoPending = -1;
	ui.txtName->setText(BoxSnapshot["Name"].toString());
	ui.txtInfo->setPlainText(BoxSnapshot["Info"].toString());
	m_SaveInfoPending = 0;

	//statusBar()->showMessage(tr("Snapshot: %1 taken: %2").arg(BoxSnapshot["Name"].toString()).arg(BoxSnapshot["Date"].toDateTime().toString()));
}

void CSnapshotsWindow::SaveInfo()
{
	if (m_SaveInfoPending)
		return;
	m_SaveInfoPending = 1;
	QTimer::singleShot(500, this, SLOT(OnSaveInfo()));
}

void CSnapshotsWindow::OnSaveInfo()
{
	if (m_SaveInfoPending != 1)
		return;
	m_SaveInfoPending = 0;

	m_pBox->SetSnapshotInfo(m_SellectedID.toString(), ui.txtName->text(), ui.txtInfo->toPlainText());
	UpdateSnapshots();
}

void CSnapshotsWindow::OnTakeSnapshot()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a name for the new Snapshot."), QLineEdit::Normal, tr("New Snapshot"));
	if (Value.isEmpty())
		return;

	HandleResult(m_pBox->TakeSnapshot(Value));

	UpdateSnapshots(true);
}

void CSnapshotsWindow::OnSelectSnapshot()
{
	QModelIndex Index = ui.treeSnapshots->currentIndex();
	//QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
	//QVariant ID = m_pSnapshotModel->GetItemID(ModelIndex);
	QVariant ID = m_pSnapshotModel->GetItemID(Index);

	if (QMessageBox("Sandboxie-Plus", tr("Do you really want to switch the active snapshot? Doing so will delete the current state!"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
		return;

	HandleResult(m_pBox->SelectSnapshot(ID.toString()));
}

void CSnapshotsWindow::OnRemoveSnapshot()
{
	QModelIndex Index = ui.treeSnapshots->currentIndex();
	//QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
	//QVariant ID = m_pSnapshotModel->GetItemID(ModelIndex);
	QVariant ID = m_pSnapshotModel->GetItemID(Index);

	if (QMessageBox("Sandboxie-Plus", tr("Do you really want to delete the selected snapshot?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
		return;

	ui.groupBox->setEnabled(false);
	ui.btnSelect->setEnabled(false);
	ui.btnRemove->setEnabled(false);

	HandleResult(m_pBox->RemoveSnapshot(ID.toString()));
}

void CSnapshotsWindow::HandleResult(SB_PROGRESS Status)
{
	if (Status.GetStatus() == OP_ASYNC)
	{
		connect(Status.GetValue().data(), SIGNAL(Finished()), this, SLOT(UpdateSnapshots()));
		theGUI->AddAsyncOp(Status.GetValue());
	}
	else if (Status.IsError())
		CSandMan::CheckResults(QList<SB_STATUS>() << Status);
	UpdateSnapshots();
}
