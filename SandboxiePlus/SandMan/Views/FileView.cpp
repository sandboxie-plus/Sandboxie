#include "stdafx.h"
#include "FileView.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/TreeItemModel.h"
#include "../MiscHelpers/Common/OtherFunctions.h"
#include "../QSbieAPI/SbieUtils.h"
#include "../Windows/SettingsWindow.h"

CFileView::CFileView(QWidget *parent)
	: QWidget(parent)
{
    m_pMainLayout = new QGridLayout();
	m_pMainLayout->setContentsMargins(0,0,0,0);
	this->setLayout(m_pMainLayout);

    m_pTreeView = new QTreeViewEx();
    m_pTreeView->setColumnFixed(0, true);
    m_pTreeView->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));
    m_pMainLayout->addWidget(m_pTreeView, 0, 0);

    m_pFileModel = NULL;

    m_pTreeView->setSortingEnabled(true);
    m_pTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QStyle* pStyle = QStyleFactory::create("windows");
	m_pTreeView->setStyle(pStyle);
    m_pTreeView->setItemDelegate(new CTreeItemDelegate());

	m_pTreeView->setExpandsOnDoubleClick(false);

	m_pTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pTreeView, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(OnFileMenu(const QPoint &)));
	connect(m_pTreeView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OnFileDblClick(const QModelIndex &)));
}

CFileView::~CFileView()
{
    SaveState();
}

void CFileView::SaveState()
{
    if(m_pFileModel)
        theConf->SetBlob("MainWindow/FileTree_Columns", m_pTreeView->header()->saveState());
}

void CFileView::SetBox(const CSandBoxPtr& pBox)
{
    if (!m_pBox.isNull()) disconnect(m_pBox.data(), SIGNAL(AboutToBeModified()), this, SLOT(OnAboutToBeModified()));

	m_pBox = pBox;

    if (!m_pBox.isNull()) connect(m_pBox.data(), SIGNAL(AboutToBeModified()), this, SLOT(OnAboutToBeModified()));

    QString Root;
    if (!pBox.isNull() && QFile::exists(pBox->GetFileRoot()))
        Root = pBox->GetFileRoot();
    //if (Root.isEmpty()) {
    //    //Root = theAPI->GetSbiePath();
    //    m_pTreeView->setEnabled(false);
    //}
    //else
    //    m_pTreeView->setEnabled(true);

    if (m_pFileModel) {
        SaveState();

        delete m_pFileModel;
        m_pFileModel = NULL;
    }
    if (!Root.isEmpty()) {
        m_pFileModel = new QFileSystemModel(this);
        m_pFileModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::System);

        QByteArray Columns = theConf->GetBlob("MainWindow/FileTree_Columns");
	    if (!Columns.isEmpty())
		    m_pTreeView->header()->restoreState(Columns);
    }
    m_pTreeView->setModel(m_pFileModel);

    if (!Root.isEmpty()) 
    {
        m_pTreeView->setRootIndex(m_pFileModel->setRootPath(Root));

        m_pTreeView->expand(m_pFileModel->index(Root + "/drive"));
        m_pTreeView->expand(m_pFileModel->index(Root + "/share"));
        m_pTreeView->expand(m_pFileModel->index(Root + "/user"));
        //m_pTreeView->expand(m_pFileModel->index(Root + "/user/all"));
        //m_pTreeView->expand(m_pFileModel->index(Root + "/user/current"));
    }
}

void CFileView::OnAboutToBeModified()
{
    if (sender() == m_pBox.data())
        SetBox(CSandBoxPtr());
}

#include <windows.h>
#include <Shlobj.h>
#include <atlbase.h>

#define MENU_RECOVER            1
#define MENU_RECOVER_TO_ANY     2
#define MENU_CREATE_SHORTCUT    3
#define MENU_CHECK_FILE         4
#define MENU_PIN_FILE           5

void addSeparatorToShellContextMenu(HMENU hMenu)
{
    MENUITEMINFO menu_item_info;
    memset(&menu_item_info, 0, sizeof(MENUITEMINFO));
    menu_item_info.cbSize = sizeof(MENUITEMINFO);
    menu_item_info.fType = MFT_SEPARATOR;
    menu_item_info.wID = 0;
    InsertMenuItem(hMenu, 0, TRUE, &menu_item_info);
}

void addItemToShellContextMenu(HMENU hMenu, const wchar_t *name, int ID, bool bChecked = false)
{
    MENUITEMINFO menu_item_info;
    memset(&menu_item_info, 0, sizeof(MENUITEMINFO));
    menu_item_info.cbSize = sizeof(MENUITEMINFO);
    menu_item_info.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA;
    if (bChecked) {
        menu_item_info.fMask |= MIIM_STATE;
        menu_item_info.fState |= MFS_CHECKED;
    }
    menu_item_info.wID = 0xF000 + ID;
    menu_item_info.dwTypeData = (wchar_t*)name;
    InsertMenuItem(hMenu, 0, TRUE, &menu_item_info);
}

int openShellContextMenu(const QStringList& Files, void* parentWindow, const CSandBoxPtr& pBox, QString* pPin = NULL)
{
    CComPtr<IShellFolder> pDesktop;
    if (!SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
        return 0;

    std::list<CComHeapPtr<ITEMIDLIST_ABSOLUTE>> items;
    items.resize(Files.count());
    auto IT = items.begin();
    foreach(QString File, Files) {
        //CComPtr<IShellItem> item;
        //SHCreateItemFromParsingName(File.toStdWString().c_str(), NULL, IID_PPV_ARGS(&item));
        //CComQIPtr<IPersistIDList> idl(item);
        //idl->GetIDList(&*IT++);
        pDesktop->ParseDisplayName((HWND)parentWindow, NULL, (wchar_t*)File.toStdWString().c_str(), NULL, &*IT++, NULL);
    }

    std::vector<LPCITEMIDLIST> list;
    list.resize(items.size());
    LPCITEMIDLIST* listPtr = &list.front();
    for (auto I = items.begin(); I != items.end(); I++)
        *listPtr++ = *I;

    CComPtr<IContextMenu> pContextMenu;
    //CComPtr<IShellItemArray> array;
    //SHCreateShellItemArrayFromIDLists(list.size(), (LPCITEMIDLIST*)&list.front(), &array);
    //array->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARGS(&pContextMenu)); // note: this fails when the files have different parent folders
    //if (!pContextMenu) // note: with the below approach properties and delete does not work, so a custom handler would be needed
    //    pDesktop->GetUIObjectOf((HWND)parentWindow, list.size(), (LPCITEMIDLIST*)&list.front(), IID_IContextMenu, NULL, (void**)&pContextMenu); 
    {
        DEFCONTEXTMENU details = { 0 };
        details.hwnd = (HWND)parentWindow;
        //details.pcmcb = pContextMenuCB;
        //details.pidlFolder = NULL;
        details.psf = pDesktop;
        details.cidl = list.size();
        details.apidl = reinterpret_cast<PCUITEMID_CHILD_ARRAY>(&list.front());
        //details.punkAssociationInfo = NULL;
        //details.cKeys = 0;
        //details.aKeys = NULL;
        SHCreateDefaultContextMenu(&details, IID_IContextMenu, reinterpret_cast<LPVOID*>(&pContextMenu));
    }
    if (!pContextMenu)
        return 0;

    HMENU hMenu = CreatePopupMenu();
    if (!hMenu)
        return 0;
    if (SUCCEEDED(pContextMenu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_NORMAL)))
    {
        addSeparatorToShellContextMenu(hMenu);

        std::wstring Str1 = CFileView::tr("Create Shortcut").toStdWString();
        if (Files.count() == 1) 
        {
            addItemToShellContextMenu(hMenu, Str1.c_str(), MENU_CREATE_SHORTCUT);

            if (pPin)  
            {
                auto pBoxPlus = pBox.objectCast<CSandBoxPlus>();
                QStringList RunOptions = pBox->GetTextList("RunCommand", true);

                QString FoundPin;
                QString FileName = Files.first();
                foreach(const QString & RunOption, RunOptions) {
		            QVariantMap Entry = GetRunEntry(RunOption);
		            QString CmdFile = pBoxPlus->GetCommandFile(Entry["Command"].toString());
		            if(CmdFile.compare(FileName, Qt::CaseInsensitive) == 0) {
                        FoundPin = RunOption;
                        break;
                    }
                }

                *pPin = FoundPin;

                std::wstring Str5 = CFileView::tr("Pin to Box Run Menu").toStdWString();
                addItemToShellContextMenu(hMenu, Str5.c_str(), MENU_PIN_FILE, !FoundPin.isEmpty());
            }

            addSeparatorToShellContextMenu(hMenu);
        }

        std::wstring Str2 = CFileView::tr("Recover to Any Folder").toStdWString();
        addItemToShellContextMenu(hMenu, Str2.c_str(), MENU_RECOVER_TO_ANY);
        std::wstring Str3 = CFileView::tr("Recover to Same Folder").toStdWString();
        addItemToShellContextMenu(hMenu, Str3.c_str(), MENU_RECOVER);
        
        if (!CSandMan::GetFileCheckers(pBox).isEmpty()) {
            std::wstring Str4 = CFileView::tr("Run Recovery Checks").toStdWString();
            addItemToShellContextMenu(hMenu, Str4.c_str(), MENU_CHECK_FILE);
        }

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
            pContextMenu->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
        }
    }
    DestroyMenu(hMenu);

    return 0;
}


void CFileView::OnFileMenu(const QPoint&)
{
    if (!m_pFileModel) return;

    QStringList Files;
    foreach(const QModelIndex & Index, m_pTreeView->selectionModel()->selectedIndexes()) {
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

    QString FoundPin;
    int iCmd = openShellContextMenu(Files, (void*)this->winId(), m_pBox, &FoundPin);
    if (iCmd == 0)
        return;

    QString RecoveryFolder;
    switch (iCmd)
    {
        case MENU_RECOVER_TO_ANY:
            RecoveryFolder = QFileDialog::getExistingDirectory(this, CFileView::tr("Select Directory")).replace("/", "\\");
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

            SB_PROGRESS Status = theGUI->RecoverFiles(m_pBox->GetName(), FileList, theGUI, 0);
            if (Status.GetStatus() == OP_ASYNC)
                theGUI->AddAsyncOp(Status.GetValue());

            break;
        }
        case MENU_CHECK_FILE:
        {
            SB_PROGRESS Status = theGUI->CheckFiles(m_pBox->GetName(), Files);
            if (Status.GetStatus() == OP_ASYNC)
                theGUI->AddAsyncOp(Status.GetValue());
            break;
        }
        case MENU_PIN_FILE:
        {
            auto pBoxPlus = m_pBox.objectCast<CSandBoxPlus>();
            if (FoundPin.isEmpty())
				pBoxPlus->InsertText("RunCommand", Split2(Files.first(), "\\", true).second + "|\"" + pBoxPlus->MakeBoxCommand(Files.first()) + "\"");
            else
				pBoxPlus->DelValue("RunCommand", FoundPin);
            break;
        }
        case MENU_CREATE_SHORTCUT:
        {
            QString BoxName = m_pBox->GetName();
            QString LinkPath = Files.first();
            QString LinkName = LinkPath.mid(LinkPath.lastIndexOf("\\") + 1);

			QString Path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).replace("/", "\\");
			//Path = QFileDialog::getExistingDirectory(this, tr("Select Directory to create Shortcut in"), Path).replace("/", "\\");
			//if (Path.isEmpty())
			//	return;

			if (Path.right(1) != "\\")
				Path.append("\\");
			Path += "[" + BoxName + "] " + LinkName;

			Path = QFileDialog::getSaveFileName(this, tr("Create Shortcut to sandbox %1").arg(BoxName), Path, QString("Shortcut files (*.lnk)")).replace("/", "\\");
			if (Path.isEmpty())
				return;

            QString StartExe = theAPI->GetSbiePath() + "\\SandMan.exe";
			CSbieUtils::CreateShortcut(StartExe, Path, LinkName, BoxName, LinkPath, LinkPath);

            break;
        }
    }

}

void CFileView::OnFileDblClick(const QModelIndex &)
{
    if (!m_pFileModel) return;

    QString BoxedPath = m_pFileModel->fileInfo(m_pTreeView->currentIndex()).absoluteFilePath();

    ShellExecute(NULL, NULL, BoxedPath.toStdWString().c_str(), NULL, m_pBox->GetFileRoot().toStdWString().c_str(), SW_SHOWNORMAL);
}


////////////////////////////////////////////////////////////////////////////////////////
// CFileBrowserWindow

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

	this->setWindowFlag(Qt::WindowStaysOnTopHint, theGUI->IsAlwaysOnTop());

    m_pMainLayout = new QGridLayout(this);
    m_FileView = new CFileView();
    m_FileView->SetBox(pBox);
    m_pMainLayout->addWidget(m_FileView, 0, 0);

	this->setWindowTitle(tr("%1 - Files").arg(pBox->GetName()));

	
	//statusBar();

	restoreGeometry(theConf->GetBlob("FileBrowserWindow/Window_Geometry"));
}

CFileBrowserWindow::~CFileBrowserWindow()
{
	theConf->SetBlob("FileBrowserWindow/Window_Geometry",saveGeometry());
}

void CFileBrowserWindow::closeEvent(QCloseEvent *e)
{
	emit Closed();
	this->deleteLater();
}
