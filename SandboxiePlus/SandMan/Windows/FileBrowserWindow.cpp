#include "stdafx.h"
#include "FileBrowserWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/TreeItemModel.h"
#include "../MiscHelpers/Common/OtherFunctions.h"
#include "../QSbieAPI/SbieUtils.h"

CFileBrowserWindow::CFileBrowserWindow(const CSandBoxPtr& pBox, QWidget *parent)
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
	this->setWindowTitle(tr("%1 - Files").arg(pBox->GetName()));

	m_pBox = pBox;

    m_pFileModel = new QFileSystemModel(this);
    m_pFileModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::System);
    m_pFileModel->setRootPath(pBox->GetFileRoot());
    ui.treeFiles->setModel(m_pFileModel);
    ui.treeFiles->setRootIndex(m_pFileModel->setRootPath(pBox->GetFileRoot()));
    ui.treeFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);

#ifdef WIN32
	QStyle* pStyle = QStyleFactory::create("windows");
	ui.treeFiles->setStyle(pStyle);
#endif
	ui.treeFiles->setExpandsOnDoubleClick(false);

	ui.treeFiles->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.treeFiles, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(OnFileMenu(const QPoint &)));
	connect(ui.treeFiles, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OnFileDblClick(const QModelIndex &)));

	//statusBar();

	restoreGeometry(theConf->GetBlob("FileBrowserWindow/Window_Geometry"));
    QByteArray Columns = theConf->GetBlob("FileBrowserWindow/FileTree_Columns");
	if (!Columns.isEmpty())
		ui.treeFiles->header()->restoreState(Columns);
}

CFileBrowserWindow::~CFileBrowserWindow()
{
	theConf->SetBlob("FileBrowserWindow/Window_Geometry",saveGeometry());
    theConf->SetBlob("FileBrowserWindow/FileTree_Columns", ui.treeFiles->header()->saveState());
}

void CFileBrowserWindow::closeEvent(QCloseEvent *e)
{
	emit Closed();
	this->deleteLater();
}


#include <windows.h>
#include <Shlobj.h>
#include <atlbase.h>

#define MENU_RECOVER            1
#define MENU_RECOVER_TO_ANY     2
#define MENU_CREATE_SHORTCUT    3

void addSeparatorToShellContextMenu(HMENU hMenu)
{
    MENUITEMINFO menu_item_info;
    memset(&menu_item_info, 0, sizeof(MENUITEMINFO));
    menu_item_info.cbSize = sizeof(MENUITEMINFO);
    menu_item_info.fType = MFT_SEPARATOR;
    menu_item_info.wID = 0;
    InsertMenuItem(hMenu, 0, TRUE, &menu_item_info);
}

void addItemToShellContextMenu(HMENU hMenu, const wchar_t *name, int ID)
{
    MENUITEMINFO menu_item_info;
    memset(&menu_item_info, 0, sizeof(MENUITEMINFO));
    menu_item_info.cbSize = sizeof(MENUITEMINFO);
    menu_item_info.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA;
    menu_item_info.wID = 0xF000 + ID;
    menu_item_info.dwTypeData = (wchar_t*)name;
    InsertMenuItem(hMenu, 0, TRUE, &menu_item_info);
}

int openShellContextMenu(const QStringList& Files, void * parentWindow)
{
    std::list<CComHeapPtr<ITEMIDLIST_ABSOLUTE>> items;
    items.resize(Files.count());
    auto IT = items.begin();
    foreach(QString File, Files) {
        CComPtr<IShellItem> item;
        SHCreateItemFromParsingName(File.toStdWString().c_str(), NULL, IID_PPV_ARGS(&item));
        CComQIPtr<IPersistIDList> idl(item);
        idl->GetIDList(&*IT++);
    }

    std::vector<LPCITEMIDLIST> list;
    list.resize(items.size());
    LPCITEMIDLIST* listPtr = &list.front();
    for (auto I = items.begin(); I != items.end(); I++)
        *listPtr++ = *I;
    CComPtr<IShellItemArray> array;
    SHCreateShellItemArrayFromIDLists(list.size(), &list.front(), &array);

    
    CComPtr<IContextMenu> menu;
    array->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARGS(&menu));
    if (!menu) 
        return 0;

    HMENU hMenu = CreatePopupMenu();
    if (!hMenu)
        return 0;
    if (SUCCEEDED(menu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_NORMAL)))
    {
        addSeparatorToShellContextMenu(hMenu);

        std::wstring Str1 = CFileBrowserWindow::tr("Create Shortcut").toStdWString();
        if (Files.count() == 1) {
            addItemToShellContextMenu(hMenu, Str1.c_str(), MENU_CREATE_SHORTCUT);
            addSeparatorToShellContextMenu(hMenu);
        }

        std::wstring Str2 = CFileBrowserWindow::tr("Recover to Any Folder").toStdWString();
        addItemToShellContextMenu(hMenu, Str2.c_str(), MENU_RECOVER_TO_ANY);
        std::wstring Str3 = CFileBrowserWindow::tr("Recover to Same Folder").toStdWString();
        addItemToShellContextMenu(hMenu, Str3.c_str(), MENU_RECOVER);
        
        POINT point;
        GetCursorPos(&point);
        int iCmd = TrackPopupMenuEx(hMenu, TPM_RETURNCMD, point.x, point.y, (HWND)parentWindow, NULL);
        if (iCmd > 0)
        {
            if ((iCmd & 0xF000) == 0xF000) {
                DestroyMenu(hMenu);
                return iCmd & 0x0FFF;
            }
            
            CMINVOKECOMMANDINFOEX info = { 0 };
            info.cbSize = sizeof(info);
            info.fMask = CMIC_MASK_UNICODE;
            info.hwnd = (HWND)parentWindow;
            info.lpVerb = MAKEINTRESOURCEA(iCmd - 1);
            info.lpVerbW = MAKEINTRESOURCEW(iCmd - 1);
            info.nShow = SW_SHOWNORMAL;
            menu->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
        }
    }
    DestroyMenu(hMenu);

    return 0;
}


void CFileBrowserWindow::OnFileMenu(const QPoint&)
{
    QStringList Files;
    foreach(const QModelIndex & Index, ui.treeFiles->selectionModel()->selectedIndexes()) {
        QString BoxedPath = m_pFileModel->fileInfo(Index).absoluteFilePath().replace("/", "\\");
        if (m_pFileModel->fileInfo(Index).isDir())
            BoxedPath += "\\";

        bool bFound = false;
        foreach(const QString & File, Files) {
            if (BoxedPath.contains(File))
                bFound = true;
            else if (File.contains(BoxedPath))
                Files.removeOne(File);
        }

        if(!bFound)
            Files.append(BoxedPath);
    }

    if (Files.isEmpty())
        return;

    int iCmd = openShellContextMenu(Files, (void*)this->winId());
    if (iCmd == 0)
        return;

    QString RecoveryFolder;
    switch (iCmd)
    {
        case MENU_RECOVER_TO_ANY:
            RecoveryFolder = QFileDialog::getExistingDirectory(this, CFileBrowserWindow::tr("Select Directory")).replace("/", "\\");
            if (RecoveryFolder.isEmpty())
                break;
        case MENU_RECOVER:
        {
            QStringList AllFiles;
            foreach(const QString& File, Files)
            {
                if (File.right(1) == "\\") {
                    foreach(QString SubFile, ListDir(File))
                        AllFiles.append(File + SubFile.replace("/", "\\"));
                }
                else
                    AllFiles.append(File);
            }

            QList<QPair<QString, QString>> FileList;
            foreach(QString BoxedPath, AllFiles) 
            {
                if (!RecoveryFolder.isEmpty()) {
                    QString FileName = BoxedPath.mid(BoxedPath.lastIndexOf("\\") + 1);
                    FileList.append(qMakePair(BoxedPath, RecoveryFolder + "\\" + FileName));
                }
                else {
                    QString RealPath = theAPI->GetRealPath(m_pBox.data(), BoxedPath);
                    FileList.append(qMakePair(BoxedPath, RealPath));
                }
            }

            SB_PROGRESS Status = theGUI->RecoverFiles(FileList, 0);
            if (Status.GetStatus() == OP_ASYNC)
                theGUI->AddAsyncOp(Status.GetValue());

            break;
        }
        case MENU_CREATE_SHORTCUT:
        {
            QString BoxName = m_pBox->GetName();
            QString LinkPath = Files.first();
            QString LinkName = LinkPath.mid(LinkPath.lastIndexOf("\\") + 1);

			QString Path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).replace("/", "\\");
			//Path = QFileDialog::getExistingDirectory(this, tr("Select Directory to create Shorcut in"), Path).replace("/", "\\");
			//if (Path.isEmpty())
			//	return;

			if (Path.right(1) != "\\")
				Path.append("\\");
			Path += "[" + BoxName + "] " + LinkName;

			Path = QFileDialog::getSaveFileName(this, tr("Create Shortcut to sandbox %1").arg(BoxName), Path, QString("Shortcut files (*.lnk)")).replace("/", "\\");
			if (Path.isEmpty())
				return;

			CSbieUtils::CreateShortcut(theAPI, Path, LinkName, BoxName, LinkPath, LinkPath);

            break;
        }
    }

}

void CFileBrowserWindow::OnFileDblClick(const QModelIndex &)
{
    QString BoxedPath = m_pFileModel->fileInfo(ui.treeFiles->currentIndex()).absoluteFilePath();

    ShellExecute(NULL, NULL, BoxedPath.toStdWString().c_str(), NULL, m_pBox->GetFileRoot().toStdWString().c_str(), SW_SHOWNORMAL);
}
