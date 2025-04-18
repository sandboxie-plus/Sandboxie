
#include <QStyledItemDelegate>
class CTrayBoxesItemDelegate : public QStyledItemDelegate
{
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QStyleOptionViewItem opt(option);
		if ((opt.state & QStyle::State_MouseOver) != 0)
			opt.state |= QStyle::State_Selected;
		else if ((opt.state & QStyle::State_HasFocus) != 0 && m_Hold)
			opt.state |= QStyle::State_Selected;
		opt.state &= ~QStyle::State_HasFocus;
		QStyledItemDelegate::paint(painter, opt, index);
	}
public:
	static bool m_Hold;
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
		QHBoxLayout* pLayout = new QHBoxLayout();
		pLayout->setContentsMargins(0,0,0,0);
		pWidget->setLayout(pLayout);

		m_pTrayBoxes = new QTreeWidget();

		m_pTrayBoxes->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
		m_pTrayBoxes->setRootIsDecorated(false);
		//m_pTrayBoxes->setHeaderLabels(tr("         Sandbox").split("|"));
		m_pTrayBoxes->setHeaderHidden(true);
		m_pTrayBoxes->setSelectionMode(QAbstractItemView::NoSelection);
		//m_pTrayBoxes->setSelectionMode(QAbstractItemView::ExtendedSelection);
		//m_pTrayBoxes->setStyleSheet("QTreeView::item:hover{background-color:#FFFF00;}");
		m_pTrayBoxes->setItemDelegate(new CTrayBoxesItemDelegate());

		m_pTrayBoxes->setStyle(QStyleFactory::create(m_DefaultStyle));

		pLayout->insertSpacing(0, 1);// 32);

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

void CSandMan::OnShowHide()
{
	if (isVisible()) {
		StoreState();
		hide();
	} else
		show();
}

QMenu* CSandMan__GetBoxParent(const QMap<QString, QStringList>& Groups, QMap<QString, QMenu*>& GroupItems, const QIcon& Icon, int iNoIcons, QMenu* pMenu, QAction* pPos, const QString& Name, int Depth = 0)
{
	if (Depth > 100)
		return NULL;
	for (auto I = Groups.constBegin(); I != Groups.constEnd(); ++I) {
		if (I->contains(Name)) {
			if (I.key().isEmpty())
				return NULL; // global group
			QMenu*& pParent = GroupItems[I.key()];
			if (!pParent) {
				pParent = new QMenu(I.key());
				if(!iNoIcons) pParent->setIcon(Icon);
				QAction* pMenuAction = NULL;
				if (QMenu* pParent2 = CSandMan__GetBoxParent(Groups, GroupItems, Icon, iNoIcons, pMenu, pPos, I.key(), ++Depth))
					pMenuAction = pParent2->addMenu(pParent);
				else
					pMenuAction = pMenu->insertMenu(pPos, pParent);
				pMenuAction->setData("group:" + I.key());
				QFont fnt = pMenuAction->font();
				fnt.setBold(true);
				pMenuAction->setFont(fnt);
			}
			return pParent;
		}
	}
	return NULL;
}

double CSandMan__GetBoxOrder(const QMap<QString, QStringList>& Groups, const QString& Name, double value = 0.0, int Depth = 0) 
{
	if (Depth > 100)
		return 1000000000;
	for (auto I = Groups.constBegin(); I != Groups.constEnd(); ++I) {
		int Pos = I->indexOf(Name);
		if (Pos != -1) {
			value = double(Pos) + value / 10.0;
			if (I.key().isEmpty())
				return value;
			return CSandMan__GetBoxOrder(Groups, I.key(), value, ++Depth);
		}
	}
	return 1000000000;
}

QAction* CSandMan__MakeBoxEntry(QMenu* pMenu, CSandBoxPlus* pBoxEx, QFileIconProvider& IconProvider, int iNoIcons, bool ColorIcons)
{
	static QMenu* pEmptyMenu = new QMenu();

	QAction* pBoxAction = new QAction(pBoxEx->GetDisplayName());
	if (!iNoIcons) {
		QIcon Icon;
		QString Action = pBoxEx->GetText("DblClickAction");
		if (!Action.isEmpty() && Action.left(1) != "!")
			Icon = IconProvider.icon(QFileInfo(pBoxEx->GetCommandFile(Action)));
		else if (ColorIcons)
			Icon = theGUI->GetColorIcon(pBoxEx->GetColor(), pBoxEx->GetActiveProcessCount());
		else
			Icon = theGUI->GetBoxIcon(pBoxEx->GetType(), pBoxEx->GetActiveProcessCount() != 0);
		pBoxAction->setIcon(Icon);
	}
	pBoxAction->setData("box:" + pBoxEx->GetName());
	pBoxAction->setMenu(pEmptyMenu);
	//pBoxAction->setIcon
	//connect(pBoxAction, SIGNAL(triggered()), this, SLOT(OnBoxMenu()));
	return pBoxAction;
}

void CSandMan::CreateBoxMenu(QMenu* pMenu, int iOffset, int iSysTrayFilter)
{
	int iNoIcons = theConf->GetInt("Options/NoIcons", 2);
	if (iNoIcons == 2)
		iNoIcons = theConf->GetInt("Options/ViewMode", 1) == 2 ? 1 : 0;
	QFileIconProvider IconProvider;
	bool ColorIcons = theConf->GetBool("Options/ColorBoxIcons", false);

	while (!pMenu->actions().at(iOffset)->data().toString().isEmpty())
		pMenu->removeAction(pMenu->actions().at(iOffset));

	QAction* pPos = pMenu->actions().at(iOffset);

	bool bPlus = (theAPI->GetFeatureFlags() & CSbieAPI::eSbieFeatureCert) != 0;
	QIcon Icon = QIcon(bPlus ? ":/Boxes/Group2" : ":/Boxes/Group"); // theGUI->GetBoxIcon(CSandBoxPlus::eDefault, false);

	QList<CSandBoxPtr> Boxes = theAPI->GetAllBoxes().values(); // map is sorted by key (box name)
	QMap<QString, QStringList> Groups = theGUI->GetBoxView()->GetGroups();

	if (theConf->GetBool("MainWindow/BoxTree_UseOrder", false)) {
		QMultiMap<double, CSandBoxPtr> Boxes2;
		foreach(const CSandBoxPtr &pBox, Boxes) {
			Boxes2.insertMulti(CSandMan__GetBoxOrder(Groups, pBox->GetName()), pBox);
		}
		Boxes = Boxes2.values();
	}

	QMap<QString, QMenu*> GroupItems;
	foreach(const CSandBoxPtr &pBox, Boxes) 
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

		QMenu* pSubMenu = CSandMan__GetBoxParent(Groups, GroupItems, Icon, iNoIcons, pMenu, pPos, pBox->GetName());
		
		QAction* pBoxAction = CSandMan__MakeBoxEntry(pMenu, pBoxEx.data(), IconProvider, iNoIcons, ColorIcons);
		if (pSubMenu)
			pSubMenu->addAction(pBoxAction);
		else
			pMenu->insertAction(pPos, pBoxAction);
	}
}

void CSandMan::OnBoxMenuHover(QAction* action)
{
	if (action->data().type() != QVariant::String)
		return;
	QString Str = action->data().toString();
	if (Str.left(4) != "box:")
		return;

	QString Name = Str.mid(4);
	static QPointer<QAction> pPrev = NULL;
	if (pPrev.data() != action) {
		if (!pPrev.isNull()) {
			pPrev->menu()->close();
			pPrev->setMenu(new QMenu());
		}
		pPrev = action;
		QMenu* pMenu = theGUI->GetBoxView()->GetMenu(Name);
		action->setMenu(pMenu);
	}
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
			int iSysTrayFilter = theConf->GetInt("Options/SysTrayFilter", 0);

			if(!m_pTrayBoxes)
				CreateBoxMenu(m_pTrayMenu, m_iTrayPos, iSysTrayFilter);
			else
			{
				QFileIconProvider IconProvider;
				bool ColorIcons = theConf->GetBool("Options/ColorBoxIcons", false);

				/**/
				m_pTrayBoxes->clear();

				QList<CSandBoxPtr> Boxes = theAPI->GetAllBoxes().values(); // map is sorted by key (box name)
				QMap<QString, QStringList> Groups = theGUI->GetBoxView()->GetGroups();

				if (theConf->GetBool("MainWindow/BoxTree_UseOrder", false)) {
					QMultiMap<double, CSandBoxPtr> Boxes2;
					foreach(const CSandBoxPtr &pBox, Boxes) {
						Boxes2.insertMulti(CBoxPicker::GetBoxOrder(Groups, pBox->GetName()), pBox);
					}
					Boxes = Boxes2.values();
				}

				QMap<QString, QTreeWidgetItem*> GroupItems;
				foreach(const CSandBoxPtr &pBox, Boxes) 
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

					QTreeWidgetItem* pParent = CBoxPicker::GetBoxParent(Groups, GroupItems, m_pTrayBoxes, pBox->GetName());

					QTreeWidgetItem* pItem = new QTreeWidgetItem();
					pItem->setText(0, pBoxEx->GetDisplayName());
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
						m_pTrayBoxes->addTopLevelItem(pItem);
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

					m_pTrayBoxes->setFixedHeight(Height);

					m_pTrayMenu->removeAction(m_pTrayList);
					m_pTrayMenu->insertAction(m_pTrayMenu->actions().at(m_iTrayPos), m_pTrayList);

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
				
				if (theAPI->GetGlobalSettings()->GetBool("ForgetPassword", false))
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
			m_pPopUpWindow->Poke();
			break;
	}
}

void CSandMan::OnBoxMenu(const QPoint& point)
{
	QTreeWidgetItem* pItem = m_pTrayBoxes->currentItem();
	if (!pItem)
		return;
	CTrayBoxesItemDelegate::m_Hold = true;
	m_pBoxView->PopUpMenu(pItem->data(0, Qt::UserRole).toString());
	CTrayBoxesItemDelegate::m_Hold = false;
}
