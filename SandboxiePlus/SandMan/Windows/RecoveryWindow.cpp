#include "stdafx.h"
#include "RecoveryWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/TreeItemModel.h"
#include "../MiscHelpers/Common/Common.h"


#if defined(Q_OS_WIN)
#include <wtypes.h>
#include <QAbstractNativeEventFilter>
#include <dbt.h>
#endif


CRecoveryWindow::CRecoveryWindow(const CSandBoxPtr& pBox, QWidget *parent)
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

	//setWindowState(Qt::WindowActive);
	SetForegroundWindow((HWND)QWidget::winId());

	bool bAlwaysOnTop = theConf->GetBool("Options/AlwaysOnTop", false);
	this->setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);

	if (!bAlwaysOnTop) {
		HWND hWnd = (HWND)this->winId();
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		QTimer::singleShot(100, this, [hWnd]() {
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		});
	}

	ui.setupUi(this);
	this->setWindowTitle(tr("%1 - File Recovery").arg(pBox->GetName()));

	m_pBox = pBox;

	m_pCounter = NULL;

	m_LastTargetIndex = 0;
	m_bTargetsChanged = false;
	m_bReloadPending = false;

#ifdef WIN32
	QStyle* pStyle = QStyleFactory::create("windows");
	ui.treeFiles->setStyle(pStyle);
#endif
	ui.treeFiles->setExpandsOnDoubleClick(false);

	ui.btnDeleteAll->setVisible(false);

	m_pFileModel = new CSimpleTreeModel();
	m_pFileModel->SetUseIcons(true);
	m_pFileModel->AddColumn(tr("File Name"), "FileName");
	m_pFileModel->AddColumn(tr("File Size"), "FileSize");
	m_pFileModel->AddColumn(tr("Full Path"), "DiskPath");

	m_pSortProxy = new CSortFilterProxyModel(false, this);
	m_pSortProxy->setSortRole(Qt::EditRole);
	m_pSortProxy->setSourceModel(m_pFileModel);
	m_pSortProxy->setDynamicSortFilter(true);

	//ui.treeFiles->setItemDelegate(theGUI->GetItemDelegate());

	ui.treeFiles->setModel(m_pSortProxy);
	((CSortFilterProxyModel*)m_pSortProxy)->setView(ui.treeFiles);

	ui.treeFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.treeFiles->setSortingEnabled(true);
	//ui.treeFiles->setUniformRowHeights(true);

	ui.gridLayout->addWidget(new CFinder(m_pSortProxy, this, true), 3, 0, 1, 5);
	ui.finder->deleteLater(); // remove place holder

	//connect(ui.treeFiles, SIGNAL(clicked(const QModelIndex&)), this, SLOT(UpdateSnapshot(const QModelIndex&)));
	//connect(ui.treeFiles->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(UpdateSnapshot(const QModelIndex&)));
	//connect(ui.treeFiles, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnSelectSnapshot()));

	connect(ui.btnAddFolder, SIGNAL(clicked(bool)), this, SLOT(OnAddFolder()));
	connect(ui.chkShowAll, SIGNAL(clicked(bool)), this, SLOT(FindFiles()));
	connect(ui.btnRefresh, SIGNAL(clicked(bool)), this, SLOT(FindFiles()));
	connect(ui.btnRecover, SIGNAL(clicked(bool)), this, SLOT(OnRecover()));
	connect(ui.cmbRecover, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTargetChanged()));
	connect(ui.btnDeleteAll, SIGNAL(clicked(bool)), this, SLOT(OnDeleteAll()));
	connect(ui.btnClose, SIGNAL(clicked(bool)), this, SLOT(close()));

	QMenu* pRecMenu = new QMenu(ui.btnRecover);
	m_pRemember = pRecMenu->addAction(tr("Remember target selection"));
	m_pRemember->setCheckable(true);
	ui.btnRecover->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnRecover->setMenu(pRecMenu);


	restoreGeometry(theConf->GetBlob("RecoveryWindow/Window_Geometry"));

	QByteArray Columns = theConf->GetBlob("RecoveryWindow/TreeView_Columns");
	if (!Columns.isEmpty())
		ui.treeFiles->header()->restoreState(Columns);

	for (int i = 0; i < m_pFileModel->columnCount(); i++)
		m_pFileModel->SetColumnEnabled(i, true);


	foreach(const QString& NtFolder, m_pBox->GetTextList("RecoverFolder", true, true)) 
	{
		bool bOk;
		QString Folder = theAPI->Nt2DosPath(NtFolder, &bOk);
		if(bOk)
			m_RecoveryFolders.append(Folder);
	}

	ui.cmbRecover->addItem(tr("Original location"), 0);
	ui.cmbRecover->addItem(tr("Browse for location"), 1);
	ui.cmbRecover->addItem(tr("Clear folder list"), -1);

	QStringList RecoverTargets = theAPI->GetUserSettings()->GetTextList("SbieCtrl_RecoverTarget", true);
	ui.cmbRecover->insertItems(ui.cmbRecover->count()-1, RecoverTargets);

	m_LastTargetIndex = theConf->GetInt("RecoveryWindow/LastTarget", -1);
	m_pRemember->setChecked(m_LastTargetIndex != -1);
	if (m_LastTargetIndex == -1)
		m_LastTargetIndex = 0;
	ui.cmbRecover->setCurrentIndex(m_LastTargetIndex);
}

CRecoveryWindow::~CRecoveryWindow()
{
	theConf->SetBlob("RecoveryWindow/Window_Geometry",saveGeometry());

	theConf->SetBlob("RecoveryWindow/TreeView_Columns", ui.treeFiles->header()->saveState());
}

int	CRecoveryWindow::exec()
{
	//QDialog::setWindowModality(Qt::WindowModal);
	ui.btnDeleteAll->setVisible(true);
	return QDialog::exec();
}

void CRecoveryWindow::closeEvent(QCloseEvent *e)
{
	emit Closed();
	this->deleteLater();
}

void CRecoveryWindow::OnAddFolder()
{
	QString Folder = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");;
	if (Folder.isEmpty())
		return;

	if (m_RecoveryFolders.contains(Folder))
		return;

	m_RecoveryFolders.append(Folder);
	m_pBox->AppendText("RecoverFolder", Folder);

	FindFiles(Folder);

	m_pFileModel->Sync(m_FileMap);
	ui.treeFiles->expandAll();
}

void CRecoveryWindow::OnTargetChanged()
{
	int op = ui.cmbRecover->currentData().toInt();
	if (op == 1)
	{
		QString Folder = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
		if (Folder.isEmpty()) {
			ui.cmbRecover->setCurrentIndex(m_LastTargetIndex);
			return;
		}
		m_LastTargetIndex = ui.cmbRecover->count() - 1;
		ui.cmbRecover->insertItem(m_LastTargetIndex, Folder);
		ui.cmbRecover->setCurrentIndex(m_LastTargetIndex);
		m_bTargetsChanged = true;
	}
	else if (op == -1)
	{
		while (ui.cmbRecover->count() > 3)
			ui.cmbRecover->removeItem(2);
		ui.cmbRecover->setCurrentIndex(0);
		m_bTargetsChanged = true;
	}
	else {
		m_LastTargetIndex = ui.cmbRecover->currentIndex();
	}
}

void CRecoveryWindow::OnRecover()
{ 
	if (m_bTargetsChanged) {
		QStringList RecoverTargets;
		for (int i = 2; i < ui.cmbRecover->count() - 1; i++)
			RecoverTargets.append(ui.cmbRecover->itemText(i));
		theAPI->GetUserSettings()->UpdateTextList("SbieCtrl_RecoverTarget", RecoverTargets, true);
	}

	theConf->SetValue("RecoveryWindow/LastTarget", m_pRemember->isChecked() ? m_LastTargetIndex : -1);

	QString RecoveryFolder;
	if (ui.cmbRecover->currentIndex() > 0)
		RecoveryFolder = ui.cmbRecover->currentText();

	RecoverFiles(false, RecoveryFolder); 
}

void CRecoveryWindow::OnDeleteAll()
{
	this->setResult(1);
	this->close();
}

void CRecoveryWindow::AddFile(const QString& FilePath, const QString& BoxPath)
{
	ui.chkShowAll->setTristate(true);
	if (m_FileMap.isEmpty()) {
		ui.chkShowAll->setCheckState(Qt::PartiallyChecked);

		QMenu* pCloseMenu = new QMenu(ui.btnClose);
		pCloseMenu->addAction(tr("Close until all programs stop in this box"), this, SLOT(OnCloseUntil()));
		ui.btnClose->setPopupMode(QToolButton::MenuButtonPopup);
		ui.btnClose->setMenu(pCloseMenu);
	}

	m_NewFiles.insert(FilePath);

	if (m_FileMap.isEmpty())
		FindFiles();
	else if (!m_bReloadPending)
	{
		m_bReloadPending = true;
		QTimer::singleShot(500, this, SLOT(FindFiles()));
	}
}

int CRecoveryWindow::FindFiles()
{
	m_bReloadPending = false;
	if (!m_NewFiles.isEmpty()) {
		ui.lblInfo->setText(tr("There are %1 new files available to recover.").arg(m_NewFiles.count()));
	}
	else if (m_pCounter == NULL) {
		m_pCounter = new CRecoveryCounter(m_pBox->GetFileRoot(), this);
		connect(m_pCounter, SIGNAL(Count(quint32, quint32, quint64)), this, SLOT(OnCount(quint32, quint32, quint64)));
	}

	m_FileMap.clear();
	int Count = 0;

	if (ui.chkShowAll->checkState() == Qt::Checked)
	{
		//for(char drive = 'A'; drive <= 'Z'; drive++)
		QDir Dir(m_pBox->GetFileRoot() + "\\drive\\");
		foreach(const QFileInfo & Info, Dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
			Count += FindBoxFiles("\\drive\\" + Info.fileName());

		if (m_pBox->GetBool("SeparateUserFolders", true)) {
			Count += FindBoxFiles("\\user\\current");
			Count += FindBoxFiles("\\user\\all");
			Count += FindBoxFiles("\\user\\public");
		}

		//Count += FindBoxFiles("\\share");
		QDir DirSvr(m_pBox->GetFileRoot() + "\\share\\");
		foreach(const QFileInfo & InfoSrv, DirSvr.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
			QDir DirPub(m_pBox->GetFileRoot() + "\\share\\" + InfoSrv.fileName());
			foreach(const QFileInfo & InfoPub, DirPub.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
				Count += FindBoxFiles("\\share\\" + InfoSrv.fileName() + "\\" + InfoPub.fileName());
		}
	}
	else
	{
		foreach(const QString & Folder, m_RecoveryFolders)
			Count += FindFiles(Folder);
	}

	m_pFileModel->Sync(m_FileMap);
	ui.treeFiles->expandAll();
	return Count;
}

int CRecoveryWindow::FindFiles(const QString& Folder)
{
	//int Count = 0;
	//foreach(const QString & Path, theAPI->GetBoxedPath(m_pBox, Folder))
	//	Count += FindFiles(Folder, Path, Folder);
	//return Count;
	return FindFiles(theAPI->GetBoxedPath(m_pBox, Folder), Folder, Folder).first;
}

int CRecoveryWindow::FindBoxFiles(const QString& Folder)
{
	QString RealFolder = theAPI->GetRealPath(m_pBox, m_pBox->GetFileRoot() + Folder);
	if (RealFolder.isEmpty())
		return 0;
	return FindFiles(m_pBox->GetFileRoot() + Folder, RealFolder, RealFolder).first;
}

QPair<int, quint64>	CRecoveryWindow::FindFiles(const QString& BoxedFolder, const QString& RealFolder, const QString& Name, const QString& ParentID)
{
	int Count = 0;
	quint64 Size = 0;

	QDir Dir(BoxedFolder);
	foreach(const QFileInfo& Info, Dir.entryInfoList(QDir::AllEntries))
	{
		QString Name = Info.fileName();
		if (Name == "." || Name == "..")
			continue;
		QString Path = Info.filePath().replace("/", "\\");

		if (Info.isFile())
		{
			QString RealPath = RealFolder + Path.mid(BoxedFolder.length());

			if (!m_NewFiles.contains(RealPath) && ui.chkShowAll->checkState() == Qt::PartiallyChecked)
				continue;

			Count++;
			Size += Info.size();

			QVariantMap RecFile;
			RecFile["ID"] = RealPath;
			RecFile["ParentID"] = RealFolder;
			RecFile["FileName"] = Name;
			RecFile["FileSize"] = FormatSize(Info.size());
			RecFile["DiskPath"] = RealPath;
			RecFile["BoxPath"] = Path;
			RecFile["Icon"] = m_IconProvider.icon(Info);
			RecFile["IsDir"] = false;
			m_FileMap.insert(RealPath, RecFile);
		}
		else
		{
			auto CountSize = FindFiles(Path, RealFolder + "\\" + Name, Name, RealFolder);
			Count += CountSize.first;
			Size += CountSize.second;
		}
	}

	if (Count > 0) 
	{
		QVariantMap RecFolder;
		RecFolder["ID"] = RealFolder;
		RecFolder["ParentID"] = ParentID;
		RecFolder["FileName"] = Name;
		RecFolder["FileSize"] = FormatSize(Size);
		RecFolder["DiskPath"] = RealFolder;
		RecFolder["BoxPath"] = BoxedFolder;
		RecFolder["Icon"] = m_IconProvider.icon(QFileIconProvider::Folder);
		RecFolder["IsDir"] = true;
		m_FileMap.insert(RealFolder, RecFolder);
	}

	return qMakePair(Count, Size);
}

void CRecoveryWindow::RecoverFiles(bool bBrowse, QString RecoveryFolder)
{
	//bool HasShare = false;
	QMap<QString, QString> FileMap;
	foreach(const QModelIndex& Index, ui.treeFiles->selectionModel()->selectedIndexes())
	{
		QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
		QVariant ID = m_pFileModel->GetItemID(ModelIndex);
		//QVariant ID = m_pFileModel->GetItemID(Index);

		QVariantMap File = m_FileMap.value(ID);
		if (File.isEmpty())
			continue;

		if (File["IsDir"].toBool() == false)
		{
			//if (File["DiskPath"].toString().indexOf("\\device\\mup", 0, Qt::CaseInsensitive) == 0)
			//	HasShare = true;
			FileMap[File["BoxPath"].toString()] = File["DiskPath"].toString();
		}
		else
		{
			QList<QModelIndex> Folders;
			Folders.append(ModelIndex);
			do
			{
				QModelIndex CurIndex = Folders.takeFirst();
				for (int i = 0; i < m_pFileModel->rowCount(CurIndex); i++)
				{
					QModelIndex ChildIndex = m_pFileModel->index(i, 0, CurIndex);

					QVariant ChildID = m_pFileModel->GetItemID(ChildIndex);
					QVariantMap File = m_FileMap.value(ChildID);
					if (File.isEmpty())
						continue;

					if (File["IsDir"].toBool() == false) 
					{
						//if (File["DiskPath"].toString().indexOf("\\device\\mup") == 0)
						//	HasShare = true;
						FileMap[File["BoxPath"].toString()] = File["DiskPath"].toString();
					}
					else
						Folders.append(ChildIndex);
				}
			} while (!Folders.isEmpty());
		}
	}


	/*if (HasShare && !bBrowse) {
		QMessageBox::warning(this, "Sandboxie-Plus", tr("One or more selected files are located on a network share, and must be recovered to a local drive, please select a folder to recover all selected files to."));
		bBrowse = true;
	}*/


	if (bBrowse && RecoveryFolder.isEmpty()) {
		RecoveryFolder = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
		if (RecoveryFolder.isEmpty())
			return;
		
		QStringList RecoverTargets = theAPI->GetUserSettings()->GetTextList("SbieCtrl_RecoverTarget", true);
		if(!RecoverTargets.contains(RecoveryFolder))
			theAPI->GetUserSettings()->UpdateTextList("SbieCtrl_RecoverTarget", RecoverTargets, true);
	}


	QList<QPair<QString, QString>> FileList;
	for(QMap<QString, QString>::const_iterator I = FileMap.begin(); I != FileMap.end(); ++I)
	{
		QString BoxedFilePath = I.key();
		QString RecoveryPath = I.value();
		if (!RecoveryFolder.isEmpty())
		{
			QString FileName = RecoveryPath.mid(RecoveryPath.lastIndexOf("\\") + 1);
			RecoveryPath = RecoveryFolder + "\\" + FileName;
		}

		FileList.append(qMakePair(BoxedFilePath, RecoveryPath));
	}


	SB_PROGRESS Status = theGUI->RecoverFiles(FileList);
	if (Status.GetStatus() == OP_ASYNC)
	{
		connect(Status.GetValue().data(), SIGNAL(Finished()), this, SLOT(FindFiles()));
		theGUI->AddAsyncOp(Status.GetValue());
	}
}

void CRecoveryWindow::OnCount(quint32 fileCount, quint32 folderCount, quint64 totalSize)
{
	ui.lblInfo->setText(tr("There are %1 files and %2 folders in the sandbox, occupying %3 of disk space.").arg(fileCount).arg(folderCount).arg(FormatSize(totalSize)));
}

void CRecoveryWindow::OnCloseUntil()
{
	m_pBox.objectCast<CSandBoxPlus>()->SetSuspendRecovery();
	close();
}

void CRecoveryCounter::run()
{
	quint32 fileCount = 0;
	quint32 folderCount = 0;
	quint64 totalSize = 0;

	QStringList Folders;
	Folders.append(m_BoxPath);
	do {
		if (!m_run) break;

		QDir Dir(Folders.takeFirst());
		foreach(const QFileInfo & Info, Dir.entryInfoList(QDir::AllEntries))
		{
			if (!m_run) break;

			QString Name = Info.fileName();
			if (Name == "." || Name == "..")
				continue;
			QString Path = Info.filePath().replace("/", "\\");
			if (Info.isFile())
			{
				fileCount++;
				totalSize += Info.size();
			}
			else
			{
				Folders.append(Path);

				folderCount++;
			}
		}

		emit Count(fileCount, folderCount, totalSize);

	} while (!Folders.isEmpty());
}