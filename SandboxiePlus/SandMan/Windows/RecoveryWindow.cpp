#include "stdafx.h"
#include "RecoveryWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/TreeItemModel.h"
#include "../MiscHelpers/Common/Common.h"
#include <QFileIconProvider>


CRecoveryWindow::CRecoveryWindow(const CSandBoxPtr& pBox, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	this->setWindowTitle(tr("%1 - File Recovery").arg(pBox->GetName()));

	m_pBox = pBox;

#ifdef WIN32
	QStyle* pStyle = QStyleFactory::create("windows");
	ui.treeFiles->setStyle(pStyle);
#endif

	ui.btnDeleteAll->setVisible(false);

	m_pFileModel = new CSimpleTreeModel();
	m_pFileModel->SetUseIcons(true);
	m_pFileModel->AddColumn(tr("File Name"), "FileName");
	m_pFileModel->AddColumn(tr("File Size"), "FileSize");
	m_pFileModel->AddColumn(tr("Full Path"), "DiskPath");

	/*m_pSortProxy = new CSortFilterProxyModel(false, this);
	m_pSortProxy->setSortRole(Qt::EditRole);
	m_pSortProxy->setSourceModel(m_pSnapshotModel);
	m_pSortProxy->setDynamicSortFilter(true);*/

	//ui.treeSnapshots->setItemDelegate(theGUI->GetItemDelegate());

	//ui.treeSnapshots->setModel(m_pSortProxy);
	ui.treeFiles->setModel(m_pFileModel);
	ui.treeFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);

	//connect(ui.treeFiles, SIGNAL(clicked(const QModelIndex&)), this, SLOT(UpdateSnapshot(const QModelIndex&)));
	//connect(ui.treeFiles->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(UpdateSnapshot(const QModelIndex&)));
	//connect(ui.treeFiles, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnSelectSnapshot()));

	connect(ui.btnAddFolder, SIGNAL(pressed()), this, SLOT(OnAddFolder()));
	connect(ui.btnRefresh, SIGNAL(pressed()), this, SLOT(FindFiles()));
	connect(ui.btnRecover, SIGNAL(pressed()), this, SLOT(OnRecover()));
	connect(ui.btnRecoverTo, SIGNAL(pressed()), this, SLOT(OnRecoverTo()));
	connect(ui.btnDeleteAll, SIGNAL(pressed()), this, SLOT(OnDeleteAll()));
	connect(ui.btnClose, SIGNAL(pressed()), this, SLOT(close()));

	restoreGeometry(theConf->GetBlob("RecoveryWindow/Window_Geometry"));

	QByteArray Columns = theConf->GetBlob("RecoveryWindow/FileTree_Columns");
	if (!Columns.isEmpty())
		ui.treeFiles->header()->restoreState(Columns);

	for (int i = 0; i < m_pFileModel->columnCount(); i++)
		m_pFileModel->SetColumnEnabled(i, true);


	foreach(const QString& NtFolder, m_pBox->GetTextList("RecoverFolder", true, true)) 
	{
		QString Folder = theAPI->Nt2DosPath(NtFolder);
		m_RecoveryFolders.insert(theAPI->GetBoxedPath(m_pBox, Folder), Folder);
	}
}

CRecoveryWindow::~CRecoveryWindow()
{
	theConf->SetBlob("RecoveryWindow/Window_Geometry",saveGeometry());

	theConf->SetBlob("RecoveryWindow/FileTree_Columns", ui.treeFiles->header()->saveState());
}

int	CRecoveryWindow::exec()
{
	ui.btnDeleteAll->setVisible(true);
	return QDialog::exec();
}

void CRecoveryWindow::closeEvent(QCloseEvent *e)
{
	this->deleteLater();
}

void CRecoveryWindow::OnAddFolder()
{
	QString Folder = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");;
	if (Folder.isEmpty())
		return;

	if (m_RecoveryFolders.contains(Folder))
		return;

	m_RecoveryFolders.insert(theAPI->GetBoxedPath(m_pBox, Folder), Folder);
	m_pBox->AppendText("RecoverFolder", Folder);

	FindFiles(theAPI->GetBoxedPath(m_pBox, Folder));

	m_pFileModel->Sync(m_FileMap);
	ui.treeFiles->expandAll();
}

void CRecoveryWindow::OnDeleteAll()
{
	this->setResult(1);
	this->close();
}

int CRecoveryWindow::FindFiles()
{
	m_FileMap.clear();
	int Count = 0;
	foreach(const QString& Folder, m_RecoveryFolders.keys())
		Count += FindFiles(Folder);

	m_pFileModel->Sync(m_FileMap);
	ui.treeFiles->expandAll();
	return Count;
}

int CRecoveryWindow::FindFiles(const QString& Folder)
{
	QFileIconProvider IconProvider;

	int Count = 0;
	quint64 TotalSize = 0;

	QString RecParent = m_RecoveryFolders.value(Folder);

	QStringList Folders;
	Folders.append(Folder);
	do {
		QDir Dir(Folders.takeFirst());
		foreach(const QFileInfo& Info, Dir.entryInfoList(QDir::AllEntries))
		{
			QString Name = Info.fileName();
			if (Name == "." || Name == "..")
				continue;
			QString Path = Info.filePath().replace("/", "\\");
			if (Info.isFile())
			{
				Count++;
				TotalSize += Info.size();

				QString RealPath = RecParent + Path.mid(Folder.length());

				QVariantMap RecFile;
				RecFile["ID"] = RealPath;
				RecFile["ParentID"] = RecParent;
				RecFile["FileName"] = Name;
				RecFile["FileSize"] = FormatSize(Info.size());
				RecFile["DiskPath"] = RealPath;
				RecFile["BoxPath"] = Path;
				RecFile["Icon"] = IconProvider.icon(Info);
				m_FileMap.insert(RealPath, RecFile);
			}
			else
				Folders.append(Path);
		}

	} while (!Folders.isEmpty());

	if (Count > 0)
	{
		QVariantMap RecFolder;
		RecFolder["ID"] = RecParent;
		RecFolder["ParentID"] = QVariant();
		RecFolder["FileName"] = RecParent;
		RecFolder["FileSize"] = FormatSize(TotalSize);
		//RecFolder["DiskPath"];
		//RecFolder["BoxPath"];
		RecFolder["Icon"] = IconProvider.icon(QFileIconProvider::Folder);
		m_FileMap.insert(RecParent, RecFolder);
	}

	return Count;
}

void CRecoveryWindow::RecoverFiles(bool bBrowse)
{
	QString RecoveryFolder;
	if (bBrowse)
	{
		RecoveryFolder = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
		if (RecoveryFolder.isEmpty())
			return;
	}

	QMap<QString, QString> FileMap;
	foreach(const QModelIndex& Index, ui.treeFiles->selectionModel()->selectedIndexes())
	{
		//QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
		//QVariant ID = m_pFileModel->GetItemID(ModelIndex);
		QVariant ID = m_pFileModel->GetItemID(Index);

		QVariantMap File = m_FileMap.value(ID);
		if (File.isEmpty())
			continue;

		if (!File["ParentID"].isNull())
			FileMap[File["BoxPath"].toString()] = File["DiskPath"].toString();
		else
		{
			for (int i = 0; i < m_pFileModel->rowCount(Index); i++)
			{
				QModelIndex ChildIndex = m_pFileModel->index(i, 0, Index);

				QVariant ChildID = m_pFileModel->GetItemID(ChildIndex);
				QVariantMap File = m_FileMap.value(ChildID);
				if (File.isEmpty())
					continue;

				FileMap[File["BoxPath"].toString()] = File["DiskPath"].toString();
			}
		}
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
