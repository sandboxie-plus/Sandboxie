#include "stdafx.h"
#include "NtObjectView.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/TreeItemModel.h"
#include "../MiscHelpers/Common/OtherFunctions.h"
#include "../QSbieAPI/SbieUtils.h"

CNtObjectView::CNtObjectView(QWidget* parent)
    : CPanelWidgetTmpl<QTreeViewEx>(NULL, parent)
{
    m_pTreeList->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));


    m_pNtObjectModel = new CNtObjectModel(this);
	m_pNtObjectModel->SetUseIcons(true);
	
	m_pSortProxy = new CSortFilterProxyModel(this);
	m_pSortProxy->setSortRole(Qt::EditRole);
    m_pSortProxy->setSourceModel(m_pNtObjectModel);
	m_pSortProxy->setDynamicSortFilter(true);

    m_pTreeList->setModel(m_pSortProxy);

    m_pTreeList->setSortingEnabled(true);
    m_pTreeList->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QStyle* pStyle = QStyleFactory::create("windows");
	m_pTreeList->setStyle(pStyle);
    m_pTreeList->setItemDelegate(new CTreeItemDelegate());

	m_pTreeList->setExpandsOnDoubleClick(false);

    m_pMainLayout->addWidget(CFinder::AddFinder(m_pTreeList, m_pSortProxy));

	m_pTreeList->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pTreeList, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(OnFileMenu(const QPoint &)));
	connect(m_pTreeList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OnFileDblClick(const QModelIndex &)));


    QByteArray Columns = theConf->GetBlob("MainWindow/NtObjectTree_Columns");
	if (!Columns.isEmpty())
		m_pTreeList->header()->restoreState(Columns);
}

CNtObjectView::~CNtObjectView()
{
    SaveState();
}

void CNtObjectView::SaveState()
{
    theConf->SetBlob("MainWindow/NtObjectTree_Columns", m_pTreeList->header()->saveState());
}

SAccessRule CNtObjectView__MkRule(QString Value)
{
    SAccessRule Rule;
    Rule.Pattern = Value;
    if ((Rule.Star = Value.right(1) == "*"))
        Value.truncate(Value.length() - 1);
    QString Exp = QRegularExpression::wildcardToRegularExpression(Value);
    if (Rule.Star)
        Exp.insert(Exp.length() - 2, "(.*)");
    Rule.Exp = QRegularExpression(Exp, QRegularExpression::CaseInsensitiveOption);
    return Rule;
}

void CNtObjectView__LoadRules(const QString& Name, SAccessRules& Rules, bool AddStar = false) 
{
    QSharedPointer<CSbieIni> pSection = QSharedPointer<CSbieIni>(new CSbieIni(Name, theAPI));

    for (int i = 0; i < COptionsWindow::eMaxAccessEntry; i++)
    {
        QPair<COptionsWindow::EAccessType, COptionsWindow::EAccessMode> Type = COptionsWindow::SplitAccessType((COptionsWindow::EAccessEntry)i);
        if (Type.first != COptionsWindow::eFile && Type.first != COptionsWindow::eKey && Type.first != COptionsWindow::eIPC)
            continue;

        bool RemoveBackslashes = Type.first == COptionsWindow::eFile || Type.first == COptionsWindow::eKey;

        foreach(QString Value, pSection->GetTextList(COptionsWindow::AccessTypeToName((COptionsWindow::EAccessEntry)i), true, true, true)) 
        {
            StrPair ProgVal = Split2(Value, ",");

            if(ProgVal.second.isEmpty())
                Value = ProgVal.first;
            else {
                // todo: filter by program ProgVal.first
                Value = ProgVal.second;
            }

            if (RemoveBackslashes && Value.left(1) == "|") {
                Value.remove(0, 1);
                AddStar = false;
            }

            if (AddStar && !Value.contains("*"))
                Value.append("*");

            if (RemoveBackslashes)
                Value = Value.replace(QRegularExpression("\\\\+"), "\\");

            if (Type.second == COptionsWindow::eOpen4All)
                Type.second = COptionsWindow::eOpen;

            Rules.Lists[Type.first].Rules.insert(Type.second, CNtObjectView__MkRule(Value));
        }
    }
}

void CNtObjectView::SetBox(const CSandBoxPtr& pBox)
{
	m_pBox = pBox;
    if (m_pBox.isNull())
        return;

    SAccessRules Rules;

    CNtObjectView__LoadRules(pBox->GetName(), Rules, true);

    CNtObjectView__LoadRules("TemplateDefaultPaths", Rules);
    bool SMode;
    if((SMode = (pBox->GetBool("UseSecurityMode") || pBox->GetBool("RestrictDevices"))))
        CNtObjectView__LoadRules("TemplateSModPaths", Rules);
    bool PMode;
    if((PMode = pBox->GetBool("UsePrivacyMode")))
        CNtObjectView__LoadRules("TemplatePModPaths", Rules);
    if(pBox->GetBool("NoSecurityIsolation"))
        CNtObjectView__LoadRules("TemplateAppCPaths", Rules);

    if (PMode || SMode) {
        // in privacy mode all drive paths are set to "write"
        // else and when in restricted mode we need to set drive paths to "normal"
        QStringList Disks = QStringList() << 
                            "\\Device\\Floppy*\\*" << 
                            "\\Device\\CdRom*\\*" << 
                            "\\Device\\HarddiskVolume*\\*" << 
                            "\\Device\\Harddisk*\\*";
        foreach(const QString & Disk, Disks)
            Rules.Lists[COptionsWindow::eFile].Rules.insert(PMode ? COptionsWindow::eBoxOnly : COptionsWindow::eNormal, CNtObjectView__MkRule(Disk));
    }

    Rules.RuleSpecificity = PMode || SMode || pBox->GetBool("UseRuleSpecificity");
    
    m_pNtObjectModel->deleteLater();
    m_pNtObjectModel = new CNtObjectModel(this);
	m_pNtObjectModel->SetUseIcons(true);
    m_pNtObjectModel->SetRules(Rules);
    m_pSortProxy->setSourceModel(m_pNtObjectModel);
}

void CNtObjectView::OnFileMenu(const QPoint&)
{
    /*if (!m_pFileModel) return;

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
        */
    // todo
}

void CNtObjectView::OnFileDblClick(const QModelIndex &)
{
    /*if (!m_pFileModel) return;

    QString BoxedPath = m_pFileModel->fileInfo(m_pTreeView->currentIndex()).absoluteFilePath();
    */
    // todo
}


////////////////////////////////////////////////////////////////////////////////////////
// CNtObjectBrowserWindow

CNtObjectBrowserWindow::CNtObjectBrowserWindow(const CSandBoxPtr& pBox, QWidget *parent)
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
    m_FileView = new CNtObjectView();
    m_FileView->SetBox(pBox);
    m_pMainLayout->addWidget(m_FileView, 0, 0);

	this->setWindowTitle(tr("%1 - NT Object Namespace").arg(pBox->GetName()));

	
	//statusBar();

	restoreGeometry(theConf->GetBlob("NtObjectBrowserWindow/Window_Geometry"));
}

CNtObjectBrowserWindow::~CNtObjectBrowserWindow()
{
	theConf->SetBlob("NtObjectBrowserWindow/Window_Geometry",saveGeometry());
}

void CNtObjectBrowserWindow::closeEvent(QCloseEvent *e)
{
	emit Closed();
	this->deleteLater();
}
