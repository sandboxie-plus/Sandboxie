
class CTrayTreeWidget : public QTreeWidget 
{	
public:
	using QTreeWidget::QTreeWidget;

	static bool IsTrayStatusTipModifierActive()
	{
		Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
		QString modifier = theConf ? theConf->GetString("Options/TrayStatusTipModifier", "Ctrl") : QString("Ctrl");
		if (modifier.compare("Shift", Qt::CaseInsensitive) == 0)
			return (mods & Qt::ShiftModifier) != 0;
		return (mods & Qt::ControlModifier) != 0;
	}

	static int GetTrayStatusTipMode()
	{
		int tipMode = theConf ? theConf->GetInt("Options/TrayStatusTip", 1) : 1; // 0=never, 1=Ctrl/Shift key only, 2=always
		if (tipMode < 0 || tipMode > 2)
			tipMode = 1;
		return tipMode;
	}

	static int GetTrayAliasMaxChars(bool bTrayUseAlias)
	{
		if (!bTrayUseAlias)
			return 32;
		int iTrayAliasMaxChars = theConf ? theConf->GetInt("Options/TrayAliasMaxChars", 64) : 64;
		if (iTrayAliasMaxChars < 32 || iTrayAliasMaxChars > 256)
			iTrayAliasMaxChars = 64;
		return iTrayAliasMaxChars;
	}

	static QString MakeTrayDisplayText(const QString& sourceText, int maxChars, bool* pTruncated = nullptr)
	{
		bool truncated = sourceText.length() > maxChars;
		if (pTruncated)
			*pTruncated = truncated;
		if (!truncated)
			return sourceText;
		return sourceText.left(maxChars) + "…";
	}

protected:
	bool viewportEvent(QEvent* e) override {
		int tipMode = GetTrayStatusTipMode();

		if (e->type() == QEvent::ToolTip) {
			QHelpEvent* helpEvent = static_cast<QHelpEvent*>(e);
			QTreeWidgetItem* item = itemAt(helpEvent->pos());
			if (!item) {
				QToolTip::hideText();
				e->ignore();
				return true;
			}

			QString statusTip = item->toolTip(0);
			QString fallbackTip = item->data(0, Qt::UserRole + 1).toString();
			bool modifierActive = IsTrayStatusTipModifierActive();
			QString tip;
			if (!statusTip.isEmpty() && (tipMode == 2 || (tipMode == 1 && modifierActive)))
				tip = statusTip;
			else if (!fallbackTip.isEmpty() && (tipMode == 0 || tipMode == 1))
				tip = fallbackTip;

			if (tip.isEmpty()) {
				QToolTip::hideText();
				e->ignore();
				return true;
			}

			QToolTip::showText(helpEvent->globalPos(), tip, viewport());
			return true;
		}

		return QTreeWidget::viewportEvent(e);
	}

	bool event(QEvent* e) override {
		int tipMode = GetTrayStatusTipMode();

		if ((e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease) && tipMode == 1) {
			QPoint localPos = viewport()->mapFromGlobal(QCursor::pos());
			if (!viewport()->rect().contains(localPos)) {
				QToolTip::hideText();
				return QTreeWidget::event(e);
			}

			QTreeWidgetItem* item = itemAt(localPos);
			if (!item) {
				QToolTip::hideText();
				return QTreeWidget::event(e);
			}

			QString statusTip = item->toolTip(0);
			QString fallbackTip = item->data(0, Qt::UserRole + 1).toString();
			QString tip;
			if (!statusTip.isEmpty() && IsTrayStatusTipModifierActive())
				tip = statusTip;
			else
				tip = fallbackTip;

			if (!tip.isEmpty())
				QToolTip::showText(QCursor::pos(), tip, viewport());
			else
				QToolTip::hideText();
		}

		return QTreeWidget::event(e);
	}

	void mousePressEvent(QMouseEvent* event) override {
		if (event->button() == Qt::RightButton) {
			auto item = itemAt(event->pos());
			if (item)
				setCurrentItem(item);
			emit customContextMenuRequested(event->pos());
		} else {
			QTreeWidget::mousePressEvent(event);
		}
	}
};


bool CTrayBoxesItemDelegate::m_Hold = false;

void CSandMan::CreateTrayIcon()
{
	m_pTrayIcon = new QSystemTrayIcon(GetTrayIcon(), this);
	m_pTrayIcon->setToolTip(GetTrayText());
	connect(m_pTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(OnSysTray(QSystemTrayIcon::ActivationReason)));
	m_bIconEmpty = true;
	m_iIconDisabled = -1;
	m_bIconBusy = false;
	m_iDeletingContent = 0;

	CreateTrayMenu();

	bool bAutoRun = QApplication::arguments().contains("-autorun");

	if(g_PendingMessage.isEmpty()){
	m_pTrayIcon->show(); // Note: qt bug; hide does not work if not showing first :/
	if(!bAutoRun && theConf->GetInt("Options/SysTrayIcon", 1) == 0)
		m_pTrayIcon->hide();
	}
}

void CSandMan::CreateTrayMenu()
{
	m_pTrayMenu = new QMenu();
	m_pTrayMenu->setToolTipsVisible(true);
	QAction* pShowHide = m_pTrayMenu->addAction(GetIcon("IconFull", 2), tr("Show/Hide"), this, SLOT(OnShowHide()));
	QFont f = pShowHide->font();
	f.setBold(true);
	pShowHide->setFont(f);
	m_pTrayMenu->addSeparator();

	m_iTrayPos = m_pTrayMenu->actions().count();

	if (!theConf->GetBool("Options/CompactTray", false))
	{
		m_pTrayBoxes = NULL;
		connect(m_pTrayMenu, SIGNAL(hovered(QAction*)), this, SLOT(OnBoxMenuHover(QAction*)));
	}
	else
	{
		m_pTrayList = new QWidgetAction(m_pTrayMenu);

		QWidget* pWidget = new CActionWidget();
		pWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		QHBoxLayout* pLayout = new QHBoxLayout();
		pLayout->setContentsMargins(0,0,0,0);
		pLayout->setSpacing(0);
		pWidget->setLayout(pLayout);

		m_pTrayBoxes = new CTrayTreeWidget();

		m_pTrayBoxes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		m_pTrayBoxes->setRootIsDecorated(false);
		//m_pTrayBoxes->setHeaderLabels(tr("         Sandbox").split("|"));
		m_pTrayBoxes->setHeaderHidden(true);
		m_pTrayBoxes->setSelectionMode(QAbstractItemView::NoSelection);
		m_pTrayBoxes->setTextElideMode(Qt::ElideRight);
		//m_pTrayBoxes->setSelectionMode(QAbstractItemView::ExtendedSelection);
		//m_pTrayBoxes->setStyleSheet("QTreeView::item:hover{background-color:#FFFF00;}");
		m_pTrayBoxes->setItemDelegate(new CTrayBoxesItemDelegate());

		m_pTrayBoxes->setStyle(QStyleFactory::create(m_DefaultStyle));

		pLayout->insertSpacing(0, 0);// keep left edge aligned with menu text

		//QFrame* vFrame = new QFrame;
		//vFrame->setFixedWidth(1);
		//vFrame->setFrameShape(QFrame::VLine);
		//vFrame->setFrameShadow(QFrame::Raised);
		//pLayout->addWidget(vFrame);

		pLayout->addWidget(m_pTrayBoxes);

		m_pTrayList->setDefaultWidget(pWidget);
		m_pTrayMenu->addAction(m_pTrayList);

		m_pTrayBoxes->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_pTrayBoxes, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnBoxMenu(const QPoint&)));
		connect(m_pTrayBoxes, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnBoxDblClick(QTreeWidgetItem*)));
		//m_pBoxMenu
	}

	m_pTrayMenu->addSeparator();
	m_pTrayMenu->addAction(m_pEmptyAll);
	m_pTrayMenu->addAction(m_pLockAll);
	m_pTrayMenu->addSeparator();
	m_pTrayMenu->addAction(m_pDisableForce2);
	if(m_pDisableRecovery) m_pTrayMenu->addAction(m_pDisableRecovery);
	if(m_pDisableMessages) m_pTrayMenu->addAction(m_pDisableMessages);
	m_pDismissUpdate = m_pTrayMenu->addAction(tr("Dismiss Update Notification"), this, SLOT(OnDismissUpdate()));
	m_pDismissUpdate->setCheckable(true);
	m_pDismissUpdate->setVisible(false);
	m_pTrayMenu->addSeparator();

	/*QWidgetAction* pBoxWidget = new QWidgetAction(m_pTrayMenu);

	QWidget* pWidget = new QWidget();
	pWidget->setMaximumHeight(200);
	QGridLayout* pLayout = new QGridLayout();
	pLayout->addWidget(pBar, 0, 0);
	pWidget->setLayout(pLayout);
	pBoxWidget->setDefaultWidget(pWidget);*/

	/*QLabel* pLabel = new QLabel("test");
	pLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	pLabel->setAlignment(Qt::AlignCenter);
	pBoxWidget->setDefaultWidget(pLabel);*/

	//m_pTrayMenu->addAction(pBoxWidget);
	//m_pTrayMenu->addSeparator();

	m_pTrayMenu->addAction(m_pExit);
}

QIcon CSandMan::GetTrayIcon(bool isConnected, bool bSun)
{
	bool bClassic = (theConf->GetInt("Options/SysTrayIcon", 1) == 2);

	QString IconFile;
	if (isConnected) {
		if (m_bIconEmpty)
			IconFile = "IconEmpty";
		else
			IconFile = "IconFull";
	} else 
		IconFile = "IconOff";
	if (bClassic) IconFile += "C";

	QSize size = QSize(16, 16);
	QPixmap result(size);
	result.fill(Qt::transparent); // force alpha channel
	QPainter painter(&result);
	QPixmap base = GetIcon(IconFile, 0).pixmap(size);
	QPixmap overlay;

	if (m_bIconBusy) {
		IconFile = "IconBusy";
		if (bClassic) { // classic has a different icon instead of an overlay
			IconFile += "C";
			base = GetIcon(IconFile, 0).pixmap(size);
		}
		else
			overlay = GetIcon(IconFile, 0).pixmap(size);
	}
	else if(bSun)
		overlay = GetIcon("IconSun", 0).pixmap(size);

	painter.drawPixmap(0, 0, base);
	if(!overlay.isNull()) painter.drawPixmap(0, 0, overlay);

	if (m_iIconDisabled == 1) {
		IconFile = "IconDFP";
		if (bClassic) IconFile += "C";
		overlay = GetIcon(IconFile, 0).pixmap(size);
		painter.drawPixmap(0, 0, overlay);
	}

	return QIcon(result);
}

QString CSandMan::GetTrayText(bool isConnected)
{
	QString Text = "Sandboxie-Plus";

	if(!isConnected)
		Text +=  tr(" - Driver/Service NOT Running!");
	else if(m_iDeletingContent)
		Text += tr(" - Deleting Sandbox Content");

	return Text;
}

static QString CSandMan__GetTrayEmptyText(int iSysTrayFilter)
{
	if (iSysTrayFilter == 2)
		return CSandMan::tr("No pinned sandboxes to show.");
	if (iSysTrayFilter == 1)
		return CSandMan::tr("No active or pinned sandboxes to show.");
	return CSandMan::tr("No sandboxes to show.");
}

void CSandMan::OnShowHide()
{
	if (isVisible()) {
		StoreState();
		hide();
	} else
		show();
}

double CSandMan__GetBoxOrder(const QMap<QString, QStringList>& Groups, const QString& Name, double value = 0.0, int Depth = 0);

static int CSandMan__GetTraySortMode()
{
	if (!theConf || !theGUI)
		return 1;

	if (theConf->GetBool("MainWindow/BoxTree_UseOrder", false))
		return 0; // manual

	auto pBoxView = theGUI->GetBoxView();
	auto pTree = pBoxView ? pBoxView->GetTree() : nullptr;
	auto pHeader = pTree ? pTree->header() : nullptr;
	if (pHeader && pHeader->isSortIndicatorShown() && pHeader->sortIndicatorSection() == 0) {
		if (pHeader->sortIndicatorOrder() == Qt::DescendingOrder)
			return -1; // descending
	}

	return 1; // ascending (default)
}

static bool CSandMan__IsBeforeBySortMode(const QString& left, const QString& right, int sortMode, const QMap<QString, QStringList>& Groups)
{
	if (sortMode == 0) {
		double l = CSandMan__GetBoxOrder(Groups, left);
		double r = CSandMan__GetBoxOrder(Groups, right);
		if (l != r)
			return l < r;
	}

	int cmp = QString::localeAwareCompare(left, right);
	if (sortMode < 0)
		return cmp > 0;
	return cmp < 0;
}

static QAction* CSandMan__InsertGroupMenuSorted(QMenu* pTargetMenu, QAction* pStopAction, QMenu* pGroupMenu, const QString& groupName, int sortMode, const QMap<QString, QStringList>& Groups)
{
	if (!pTargetMenu || !pGroupMenu)
		return nullptr;

	QAction* pBeforeAction = nullptr;
	for (QAction* pAction : pTargetMenu->actions()) {
		if (pStopAction && pAction == pStopAction)
			break;

		if (!pAction->data().toString().startsWith("group:"))
			continue;

		if (CSandMan__IsBeforeBySortMode(groupName, pAction->text(), sortMode, Groups)) {
			pBeforeAction = pAction;
			break;
		}
	}

	if (pBeforeAction)
		return pTargetMenu->insertMenu(pBeforeAction, pGroupMenu);
	if (pStopAction)
		return pTargetMenu->insertMenu(pStopAction, pGroupMenu);
	return pTargetMenu->addMenu(pGroupMenu);
}

static QTreeWidgetItem* CSandMan__InsertGroupItemSorted(QTreeWidget* pTree, QTreeWidgetItem* pParentItem, QTreeWidgetItem* pGroupItem, const QString& groupName, int sortMode, const QMap<QString, QStringList>& Groups)
{
	if (!pTree || !pGroupItem)
		return pGroupItem;

	int insertIndex = pParentItem ? pParentItem->childCount() : pTree->topLevelItemCount();
	int count = pParentItem ? pParentItem->childCount() : pTree->topLevelItemCount();

	for (int i = 0; i < count; ++i) {
		QTreeWidgetItem* pExisting = pParentItem ? pParentItem->child(i) : pTree->topLevelItem(i);
		if (!pExisting)
			continue;

		QString existingName = pExisting->data(0, Qt::UserRole).toString();
		if (!Groups.contains(existingName))
			continue;

		if (CSandMan__IsBeforeBySortMode(groupName, existingName, sortMode, Groups)) {
			insertIndex = i;
			break;
		}
	}

	if (pParentItem)
		pParentItem->insertChild(insertIndex, pGroupItem);
	else
		pTree->insertTopLevelItem(insertIndex, pGroupItem);

	return pGroupItem;
}

static QTreeWidgetItem* CSandMan__GetBoxParentTree(const QMap<QString, QStringList>& Groups, QMap<QString, QTreeWidgetItem*>& GroupItems, QTreeWidget* pTree, const QString& Name, int sortMode, int Depth = 0)
{
	if (!pTree)
		return nullptr;

	if (Depth > 100)
		return nullptr;

	for (auto I = Groups.constBegin(); I != Groups.constEnd(); ++I) {
		if (!I->contains(Name))
			continue;

		if (I.key().isEmpty())
			return nullptr;

		QTreeWidgetItem*& pParent = GroupItems[I.key()];
		if (!pParent) {
			pParent = new QTreeWidgetItem();
			pParent->setText(0, I.key());
			pParent->setData(0, Qt::UserRole, I.key());
			QFont fnt = pParent->font(0);
			fnt.setBold(true);
			pParent->setFont(0, fnt);

			if (QTreeWidgetItem* pParent2 = CSandMan__GetBoxParentTree(Groups, GroupItems, pTree, I.key(), sortMode, Depth + 1))
				CSandMan__InsertGroupItemSorted(pTree, pParent2, pParent, I.key(), sortMode, Groups);
			else
				CSandMan__InsertGroupItemSorted(pTree, nullptr, pParent, I.key(), sortMode, Groups);
		}

		return pParent;
	}

	return nullptr;
}

QMenu* CSandMan__GetBoxParent(const QMap<QString, QStringList>& Groups, QMap<QString, QMenu*>& GroupItems, const QIcon& Icon, int iNoIcons, QMenu* pMenu, QAction* pPos, const QString& Name, int sortMode, int Depth = 0)
{
	if (!pMenu)
		return NULL;

	if (Depth > 100)
		return NULL;
	for (auto I = Groups.constBegin(); I != Groups.constEnd(); ++I) {
		if (I->contains(Name)) {
			if (I.key().isEmpty())
				return NULL; // global group
			QMenu*& pParent = GroupItems[I.key()];
			if (!pParent) {
				pParent = new QMenu(I.key());
				pParent->setToolTipsVisible(true);
				if(!iNoIcons) pParent->setIcon(Icon);
				QAction* pMenuAction = NULL;
				if (QMenu* pParent2 = CSandMan__GetBoxParent(Groups, GroupItems, Icon, iNoIcons, pMenu, pPos, I.key(), sortMode, Depth + 1))
					pMenuAction = CSandMan__InsertGroupMenuSorted(pParent2, nullptr, pParent, I.key(), sortMode, Groups);
				else
					pMenuAction = CSandMan__InsertGroupMenuSorted(pMenu, pPos, pParent, I.key(), sortMode, Groups);
				if (pMenuAction) {
					pMenuAction->setData("group:" + I.key());
					QFont fnt = pMenuAction->font();
					fnt.setBold(true);
					pMenuAction->setFont(fnt);
				}
			}
			return pParent;
		}
	}
	return NULL;
}

double CSandMan__GetBoxOrder(const QMap<QString, QStringList>& Groups, const QString& Name, double value, int Depth) 
{
	if (Depth > 100)
		return 1000000000;
	for (auto I = Groups.constBegin(); I != Groups.constEnd(); ++I) {
		int Pos = I->indexOf(Name);
		if (Pos != -1) {
			value = double(Pos) + value / 10.0;
			if (I.key().isEmpty())
				return value;
			return CSandMan__GetBoxOrder(Groups, I.key(), value, Depth + 1);
		}
	}
	return 1000000000;
}

// Builds the rich status tooltip shown on sandbox items — matches the format used in the main sandbox tree view.
static QString CSandMan__BuildBoxTooltip(const CSandBoxPlus* pBoxEx)
{
	if (!pBoxEx)
		return QString();

	QString boxName = pBoxEx->GetName();
	QString boxAlias = pBoxEx->GetText("BoxAlias").trimmed();
	QString boxAliasDisabled = pBoxEx->GetText("BoxAliasDisabled");
	bool aliasEnabled = !boxAlias.isEmpty() && boxAliasDisabled.isEmpty();
	QString tip = boxName + "\n";
	if (aliasEnabled && boxAlias.compare(boxName, Qt::CaseSensitive) != 0)
		tip += CSandMan::tr("    Alias: %1\n").arg(boxAlias);
	tip += CSandMan::tr("    File root: %1\n").arg(pBoxEx->GetFileRoot());
	tip += CSandMan::tr("    Registry root: %1\n").arg(pBoxEx->GetRegRoot());
	tip += CSandMan::tr("    IPC root: %1\n").arg(pBoxEx->GetIpcRoot());
	if (!pBoxEx->GetMountRoot().isEmpty())
		tip += CSandMan::tr("    Disk root: %1\n").arg(pBoxEx->GetMountRoot());
	tip += CSandMan::tr("Options:\n    ");
	tip += pBoxEx->GetStatusStr().replace(", ", "\n    ");
	return tip;
}

// Returns a cached custom icon (BoxIcon / DblClickAction). Falls back to null QIcon if none found.
// Only disk-loaded icons are cached. Color/type icons are cheap and always generated fresh.
static QIcon CSandMan__GetCachedCustomIcon(const QString& boxName, const QString& boxIconStr,
	const QString& dblClickAction, CSandBoxPlus* pBoxEx,
	QFileIconProvider& IconProvider, QMap<QString, QPair<QString,QIcon>>& cache)
{
	QString configKey = boxIconStr + "|" + dblClickAction;
	auto it = cache.find(boxName);
	if (it != cache.end() && it->first == configKey)
		return it->second; // cache hit (may be null QIcon if no custom icon exists)

	QIcon Icon;
	if (!boxIconStr.isEmpty()) {
		int comma = boxIconStr.lastIndexOf(',');
		bool ok = false;
		if (comma > 0) {
			int idx = boxIconStr.mid(comma + 1).toInt(&ok);
			if (ok) Icon = QIcon(LoadWindowsIcon(boxIconStr.left(comma), idx));
		}
		if (Icon.isNull()) Icon = QIcon(QPixmap(boxIconStr));
	}
	if (Icon.isNull() && pBoxEx && !dblClickAction.isEmpty() && dblClickAction.left(1) != "!") {
		QString cmdFile = pBoxEx->GetCommandFile(dblClickAction);
		if (!cmdFile.isEmpty())
			Icon = IconProvider.icon(QFileInfo(cmdFile));
	}

	cache[boxName] = qMakePair(configKey, Icon);
	return Icon;
}

static QIcon CSandMan__ApplyTrayBoxOverlay(QIcon Icon, CSandBoxPlus* pBoxEx)
{
	if (!theGUI || Icon.isNull())
		return Icon;
	if (pBoxEx->IsForceDisabled())
		return theGUI->IconAddOverlay(Icon, ":/IconDFP");
	if (pBoxEx->UseRamDisk())
		return theGUI->IconAddOverlay(Icon, ":/Actions/RamDisk.png");
	if (pBoxEx->UseImageFile())
		return theGUI->IconAddOverlay(Icon, pBoxEx->GetMountRoot().isEmpty() ? ":/Actions/LockClosed.png" : ":/Actions/LockOpen.png");
	if (pBoxEx->IsAutoDelete())
		return theGUI->IconAddOverlay(Icon, ":/Boxes/AutoDel");
	return Icon;
}

QAction* CSandMan__MakeBoxEntry(QMenu* pMenu, CSandBoxPlus* pBoxEx, QFileIconProvider& IconProvider, int iNoIcons, bool ColorIcons)
{
	static QMenu* pEmptyMenu = new QMenu();
	if (!pBoxEx) return nullptr;

	bool bTrayUseAlias = theConf ? theConf->GetBool("Options/TrayUseAlias", true) : true;
	QString displayNameRaw = bTrayUseAlias ? pBoxEx->GetDisplayName() : pBoxEx->GetName();
	bool truncated = false;
	QString displayName = CTrayTreeWidget::MakeTrayDisplayText(displayNameRaw, CTrayTreeWidget::GetTrayAliasMaxChars(bTrayUseAlias), &truncated);
	QAction* pBoxAction = new QAction(displayName);
	if (!iNoIcons) {
		QIcon Icon;
		bool bTrayIcons = theConf ? theConf->GetBool("Options/TrayIcons", true) : true;
		if (bTrayIcons) {
			QString boxIconStr   = pBoxEx->GetText("BoxIcon");
			QString dblClickAct  = pBoxEx->GetText("DblClickAction");
			if (theGUI)
				Icon = CSandMan__GetCachedCustomIcon(pBoxEx->GetName(), boxIconStr, dblClickAct, pBoxEx, IconProvider, theGUI->m_TrayIconCache);
		}
		if (Icon.isNull()) {
			if (!theGUI)
				Icon = QIcon();
			else if (ColorIcons)
				Icon = theGUI->GetColorIcon(pBoxEx->GetColor(), pBoxEx->GetActiveProcessCount());
			else
				Icon = theGUI->GetBoxIcon(pBoxEx->GetType(), pBoxEx->GetActiveProcessCount() != 0);
		}
		if (theConf ? theConf->GetBool("Options/TrayOverlayIcons", true) : true)
			Icon = CSandMan__ApplyTrayBoxOverlay(Icon, pBoxEx);
		pBoxAction->setIcon(Icon);
	}
	pBoxAction->setData("box:" + pBoxEx->GetName());
	pBoxAction->setMenu(pEmptyMenu);
	// Always store rich tooltip; OnBoxMenuHover decides whether to display it.
	// When status tooltip mode is disabled, keep full-name tooltip only for truncated labels.
	QString fallbackTip = truncated ? displayNameRaw : QString();
	pBoxAction->setProperty("tray_fallback_tip", fallbackTip);
	pBoxAction->setToolTip(CSandMan__BuildBoxTooltip(pBoxEx));
	//connect(pBoxAction, SIGNAL(triggered()), this, SLOT(OnBoxMenu()));
	return pBoxAction;
}

void CSandMan::CreateBoxMenu(QMenu* pMenu, int iOffset, int iSysTrayFilter)
{
	if (!pMenu || !theConf || !theAPI || !theGUI)
		return;
	auto pBoxView = theGUI->GetBoxView();
	if (!pBoxView)
		return;

	QList<QAction*> actions = pMenu->actions();
	if (iOffset < 0 || iOffset >= actions.count())
		return;

	int iNoIcons = theConf->GetInt("Options/NoIcons", 2);
	if (iNoIcons == 2)
		iNoIcons = theConf->GetInt("Options/ViewMode", 1) == 2 ? 1 : 0;
	QFileIconProvider IconProvider;
	bool ColorIcons = theConf->GetBool("Options/ColorBoxIcons", false);

	while (iOffset < pMenu->actions().count()) {
		QAction* pAction = pMenu->actions().at(iOffset);
		if (!pAction)
			break;
		if (!pAction->data().toString().isEmpty())
			pMenu->removeAction(pAction);
		else
			break;
	}

	if (iOffset >= pMenu->actions().count())
		return;

	QAction* pPos = pMenu->actions().at(iOffset);

	bool bPlus = (theAPI->GetFeatureFlags() & CSbieAPI::eSbieFeatureCert) != 0;
	QIcon Icon = QIcon(bPlus ? ":/Boxes/Group2" : ":/Boxes/Group"); // theGUI->GetBoxIcon(CSandBoxPlus::eDefault, false);

	QList<CSandBoxPtr> Boxes = theAPI->GetAllBoxes().values(); // map is sorted by key (box name)
	QMap<QString, QStringList> Groups = pBoxView->GetGroups();
	int sortMode = CSandMan__GetTraySortMode(); // 0=manual, 1=asc, -1=desc

	if (sortMode == 0) {
		QMultiMap<double, CSandBoxPtr> Boxes2;
		foreach(const CSandBoxPtr &pBox, Boxes) {
			Boxes2.insertMulti(CSandMan__GetBoxOrder(Groups, pBox->GetName()), pBox);
		}
		Boxes = Boxes2.values();
	}
	else if (sortMode < 0) {
		QList<CSandBoxPtr> Boxes2;
		for (int i = Boxes.count() - 1; i >= 0; --i)
			Boxes2.append(Boxes.at(i));
		Boxes = Boxes2;
	}

	QMap<QString, QMenu*> GroupItems;
	bool bAnyBoxAdded = false;
	foreach(const CSandBoxPtr &pBox, Boxes) 
	{
		if (!pBox->IsEnabled())
			continue;

		auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
		if (!pBoxEx) continue;

		if (iSysTrayFilter == 2) { // pinned only
			if (!pBox->GetBool("PinToTray", false))
				continue;
		}
		else if (iSysTrayFilter == 1) { // active + pinned
			if (pBoxEx->GetActiveProcessCount() == 0 && !pBox->GetBool("PinToTray", false))
				continue;
		}

		QMenu* pSubMenu = CSandMan__GetBoxParent(Groups, GroupItems, Icon, iNoIcons, pMenu, pPos, pBox->GetName(), sortMode);
		
		QAction* pBoxAction = CSandMan__MakeBoxEntry(pMenu, pBoxEx.data(), IconProvider, iNoIcons, ColorIcons);
		if (!pBoxAction) continue;
		if (pSubMenu)
			pSubMenu->addAction(pBoxAction);
		else
			pMenu->insertAction(pPos, pBoxAction);
		bAnyBoxAdded = true;
	}

	if (!bAnyBoxAdded) {
		QAction* pEmptyAction = new QAction(CSandMan__GetTrayEmptyText(iSysTrayFilter), pMenu);
		pEmptyAction->setEnabled(false);
		pEmptyAction->setData("empty:");
		pMenu->insertAction(pPos, pEmptyAction);
	}
}

void CSandMan::OnBoxMenuHover(QAction* action)
{
	if (!action) {
		QToolTip::hideText();
		return;
	}
	if (!theGUI) {
		QToolTip::hideText();
		return;
	}
	auto pBoxView = theGUI->GetBoxView();
	if (!pBoxView) {
		QToolTip::hideText();
		return;
	}

	int tipMode = CTrayTreeWidget::GetTrayStatusTipMode();

	if (action->data().type() != QVariant::String) {
		QToolTip::hideText();
		return;
	}
	QString Str = action->data().toString();
	if (Str.left(4) != "box:") {
		QToolTip::hideText();
		return;
	}

	QString Name = Str.mid(4);
	static QPointer<QAction> pPrev = NULL;
	if (pPrev.data() != action) {
		if (!pPrev.isNull()) {
			if (pPrev->menu())
				pPrev->menu()->close();
			pPrev->setMenu(new QMenu());
		}
		pPrev = action;
		QMenu* pMenu = pBoxView->GetMenu(Name);
		action->setMenu(pMenu);
	}

	// Show rich tooltip manually — QAction+submenu combos don't auto-show Qt tooltips
	QString statusTip = action->toolTip();
	QString fallbackTip = action->property("tray_fallback_tip").toString();
	QString tipToShow;
	if (!statusTip.isEmpty() && (tipMode == 2 || (tipMode == 1 && CTrayTreeWidget::IsTrayStatusTipModifierActive())))
		tipToShow = statusTip;
	else if (tipMode == 0 || tipMode == 1)
		tipToShow = fallbackTip;

	if (!tipToShow.isEmpty())
		QToolTip::showText(QCursor::pos() + QPoint(20, 0), tipToShow);
	else
		QToolTip::hideText();
}

void CSandMan::OnSysTray(QSystemTrayIcon::ActivationReason Reason)
{
	static bool TriggerSet = false;
	static bool NullifyTrigger = false;

	if (theConf->GetBool("Options/TraySingleClick", false) && Reason == QSystemTrayIcon::Trigger)
		Reason = QSystemTrayIcon::DoubleClick;

	switch(Reason)
	{
		case QSystemTrayIcon::Context:
		{
			if (!theConf || !theAPI || !theGUI || !m_pTrayMenu)
				break;

			int iSysTrayFilter = theConf->GetInt("Options/SysTrayFilter", 0);

			if(!m_pTrayBoxes)
				CreateBoxMenu(m_pTrayMenu, m_iTrayPos, iSysTrayFilter);
			else
			{
				auto pBoxView = theGUI->GetBoxView();
				if (!pBoxView)
					break;

				QFileIconProvider IconProvider;
				bool ColorIcons = theConf->GetBool("Options/ColorBoxIcons", false);
				bool bTrayIcons = theConf->GetBool("Options/TrayIcons", true);
				bool bTrayUseAlias = theConf->GetBool("Options/TrayUseAlias", true);
				int iTrayAliasMaxChars = CTrayTreeWidget::GetTrayAliasMaxChars(bTrayUseAlias);
				m_pTrayBoxes->clear();

				QList<CSandBoxPtr> Boxes = theAPI->GetAllBoxes().values(); // map is sorted by key (box name)
				QMap<QString, QStringList> Groups = pBoxView->GetGroups();
				int sortMode = CSandMan__GetTraySortMode(); // 0=manual, 1=asc, -1=desc

				if (sortMode == 0) {
					QMultiMap<double, CSandBoxPtr> Boxes2;
					foreach(const CSandBoxPtr &pBox, Boxes) {
						Boxes2.insertMulti(CBoxPicker::GetBoxOrder(Groups, pBox->GetName()), pBox);
					}
					Boxes = Boxes2.values();
				}
				else if (sortMode < 0) {
					QList<CSandBoxPtr> Boxes2;
					for (int i = Boxes.count() - 1; i >= 0; --i)
						Boxes2.append(Boxes.at(i));
					Boxes = Boxes2;
				}

				QMap<QString, QTreeWidgetItem*> GroupItems;
				bool bHasVisibleBoxes = false;
				foreach(const CSandBoxPtr &pBox, Boxes) 
				{
					if (!pBox->IsEnabled())
						continue;

					auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
					if (!pBoxEx) continue;

					if (iSysTrayFilter == 2) { // pinned only
						if (!pBox->GetBool("PinToTray", false))
							continue;
					}
					else if (iSysTrayFilter == 1) { // active + pinned
						if (pBoxEx->GetActiveProcessCount() == 0 && !pBox->GetBool("PinToTray", false))
							continue;
					}

					QTreeWidgetItem* pParent = CSandMan__GetBoxParentTree(Groups, GroupItems, m_pTrayBoxes, pBox->GetName(), sortMode);

					QTreeWidgetItem* pItem = new QTreeWidgetItem();
					QString displayNameRaw = bTrayUseAlias ? pBoxEx->GetDisplayName() : pBoxEx->GetName();
					bool truncated = false;
					QString displayName = CTrayTreeWidget::MakeTrayDisplayText(displayNameRaw, iTrayAliasMaxChars, &truncated);
					pItem->setText(0, displayName);
					pItem->setData(0, Qt::UserRole, pBox->GetName());
					QIcon Icon;
					if (bTrayIcons) {
						QString boxIconStr  = pBox->GetText("BoxIcon");
						QString dblClickAct = pBox->GetText("DblClickAction");
						Icon = CSandMan__GetCachedCustomIcon(pBox->GetName(), boxIconStr, dblClickAct, pBoxEx.data(), IconProvider, m_TrayIconCache);
					}
					if (Icon.isNull()) {
						if(ColorIcons)
							Icon = theGUI->GetColorIcon(pBoxEx->GetColor(), pBox->GetActiveProcessCount());
						else
							Icon = theGUI->GetBoxIcon(pBoxEx->GetType(), pBox->GetActiveProcessCount() != 0);
					}
					if (theConf->GetBool("Options/TrayOverlayIcons", true))
						Icon = CSandMan__ApplyTrayBoxOverlay(Icon, pBoxEx.data());
					pItem->setData(0, Qt::DecorationRole, Icon);
					// Store both status and truncation tooltips; view logic picks one by mode/modifier
					pItem->setToolTip(0, CSandMan__BuildBoxTooltip(pBoxEx.data()));
					pItem->setData(0, Qt::UserRole + 1, truncated ? displayNameRaw : QString());
					if (pParent)
						pParent->addChild(pItem);
					else
						m_pTrayBoxes->addTopLevelItem(pItem);

					bHasVisibleBoxes = true;
				}

				if (!bHasVisibleBoxes) {
					QTreeWidgetItem* pEmptyItem = new QTreeWidgetItem();
					pEmptyItem->setText(0, CSandMan__GetTrayEmptyText(iSysTrayFilter));
					pEmptyItem->setFlags(Qt::NoItemFlags);
					m_pTrayBoxes->addTopLevelItem(pEmptyItem);
				}

				m_pTrayBoxes->expandAll();
				/**/

				/*/
				QMap<QString, CSandBoxPtr> Boxes = theAPI->GetAllBoxes();

				bool bAdded = false;
				if (m_pTrayBoxes->topLevelItemCount() == 0)
					bAdded = true; // trigger size refresh

				QMap<QString, QTreeWidgetItem*> OldBoxes;
				for (int i = 0; i < m_pTrayBoxes->topLevelItemCount(); ++i)
				{
					QTreeWidgetItem* pItem = m_pTrayBoxes->topLevelItem(i);
					QString Name = pItem->data(0, Qt::UserRole).toString();
					OldBoxes.insert(Name, pItem);
				}

				foreach(const CSandBoxPtr & pBox, Boxes)
				{
					if (!pBox->IsEnabled())
						continue;

					auto pBoxEx = pBox.objectCast<CSandBoxPlus>();

					if (iSysTrayFilter == 2) { // pinned only
						if (!pBox->GetBool("PinToTray", false))
							continue;
					}
					else if (iSysTrayFilter == 1) { // active + pinned
						if (pBoxEx->GetActiveProcessCount() == 0 && !pBox->GetBool("PinToTray", false))
							continue;
					}

					QTreeWidgetItem* pItem = OldBoxes.take(pBox->GetName());
					if (!pItem)
					{
						pItem = new QTreeWidgetItem();
						pItem->setData(0, Qt::UserRole, pBox->GetName());
						pItem->setText(0, "  " + pBoxEx->GetDisplayName());
						m_pTrayBoxes->addTopLevelItem(pItem);

						bAdded = true;
					}

					QIcon Icon;
					QString Action = pBox->GetText("DblClickAction");
					if (!Action.isEmpty() && Action.left(1) != "!")
						Icon = IconProvider.icon(QFileInfo(pBoxEx->GetCommandFile(Action)));
					else if (ColorIcons)
						Icon = theGUI->GetColorIcon(pBoxEx->GetColor(), pBox->GetActiveProcessCount());
					else
						Icon = theGUI->GetBoxIcon(pBoxEx->GetType(), pBox->GetActiveProcessCount() != 0);
					pItem->setData(0, Qt::DecorationRole, Icon);
				}

				foreach(QTreeWidgetItem * pItem, OldBoxes)
					delete pItem;
				/*/

				//if (!OldBoxes.isEmpty() || bAdded)
				{
					auto palette = m_pTrayBoxes->palette();
					palette.setColor(QPalette::Base, m_pTrayMenu->palette().color(m_DarkTheme ? QPalette::Base : QPalette::Window));
					m_pTrayBoxes->setPalette(palette);
					m_pTrayBoxes->setFrameShape(QFrame::NoFrame);

					//const int FrameWidth = m_pTrayBoxes->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
					int Height = 0; //m_pTrayBoxes->header()->height() + (2 * FrameWidth);

					for (QTreeWidgetItemIterator AllIterator(m_pTrayBoxes, QTreeWidgetItemIterator::All); *AllIterator; ++AllIterator)
						Height += m_pTrayBoxes->visualItemRect(*AllIterator).height();

					QRect scrRect = this->screen()->availableGeometry();
					int MaxHeight = scrRect.height() / 3;
					if (Height > MaxHeight) {
						Height = MaxHeight;
						if (Height < 64)
							Height = 64;
					}

					bool bNeedsVScroll = Height >= MaxHeight;
					m_pTrayBoxes->setFixedHeight(Height);

					m_pTrayBoxes->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
					m_pTrayBoxes->setVerticalScrollBarPolicy(bNeedsVScroll ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);

					// Measure each item's text to get exact required width
					QFontMetrics fm(m_pTrayBoxes->font());
					// DPI-aware scaling: logicalDpiX() returns actual screen DPI (e.g. 192 at 200% scaling)
					const qreal dpiScale = m_pTrayBoxes->logicalDpiX() / 96.0;
					int iconSize = m_pTrayBoxes->style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, m_pTrayBoxes);
					// Ensure the tree widget paints icons at the same size we measure
					m_pTrayBoxes->setIconSize(QSize(iconSize, iconSize));
					int indent = m_pTrayBoxes->indentation();
					// Gap between icon and text, and right margin — scale proportionally with DPI
					int spacing = qRound(4 * dpiScale);
					int maxItemWidth = 0;
					for (QTreeWidgetItemIterator it(m_pTrayBoxes, QTreeWidgetItemIterator::All); *it; ++it) {
						QTreeWidgetItem* twi = *it;
						int depth = 0;
						for (QTreeWidgetItem* p = twi->parent(); p; p = p->parent()) depth++;
						int itemWidth = depth * indent + iconSize + spacing + fm.horizontalAdvance(twi->text(0)) + spacing;
						if (itemWidth > maxItemWidth) maxItemWidth = itemWidth;
					}
					int scrollBarWidth = bNeedsVScroll ? m_pTrayBoxes->style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr, m_pTrayBoxes) : 0;
					int Width = maxItemWidth + scrollBarWidth + qRound(4 * dpiScale);
					int MaxWidth = scrRect.width() / 3;
					if (Width > MaxWidth) Width = MaxWidth;
					m_pTrayBoxes->setMinimumWidth(Width);

					m_pTrayMenu->removeAction(m_pTrayList);
					QAction* pInsertPos = (m_iTrayPos >= 0 && m_iTrayPos < m_pTrayMenu->actions().count())
						? m_pTrayMenu->actions().at(m_iTrayPos)
						: nullptr;
					if (pInsertPos)
						m_pTrayMenu->insertAction(pInsertPos, m_pTrayList);
					else
						m_pTrayMenu->addAction(m_pTrayList);

					m_pTrayBoxes->setFocus();
				}
			}

			m_pTrayMenu->popup(QCursor::pos());	
			break;
		}
		case QSystemTrayIcon::DoubleClick:
			if (isVisible())
			{
				if(TriggerSet)
					NullifyTrigger = true;
				
				StoreState();
				hide();
				
				if (theAPI && theAPI->GetGlobalSettings() && theAPI->GetGlobalSettings()->GetBool("ForgetPassword", false))
					theAPI->ClearPassword();

				break;
			}
			CheckSupport();
			show();
		case QSystemTrayIcon::Trigger:
			if (isVisible() && !TriggerSet)
			{
				TriggerSet = true;
				QTimer::singleShot(100, [this]() { 
					TriggerSet = false;
					if (NullifyTrigger) {
						NullifyTrigger = false;
						return;
					}
					this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
					SetForegroundWindow(MainWndHandle);
				} );
			}
			if (m_pPopUpWindow)
				m_pPopUpWindow->Poke();
			break;
	}
}

void CSandMan::OnBoxMenu(const QPoint& point)
{
	Q_UNUSED(point);
	if (!m_pTrayBoxes || !m_pBoxView)
		return;
	QTreeWidgetItem* pItem = m_pTrayBoxes->currentItem();
	if (!pItem || !m_pBoxView)
		return;
	QString Name = pItem->data(0, Qt::UserRole).toString();
	if (Name.isEmpty() || !theAPI || theAPI->GetBoxByName(Name).isNull())
		return;
	CTrayBoxesItemDelegate::m_Hold = true;
	m_pBoxView->PopUpMenu(Name);
	CTrayBoxesItemDelegate::m_Hold = false;
}
