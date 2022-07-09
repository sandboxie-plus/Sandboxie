
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
	m_bIconDisabled = false;
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
	QAction* pShowHide = m_pTrayMenu->addAction(GetIcon("IconFull", false), tr("Show/Hide"), this, SLOT(OnShowHide()));
	QFont f = pShowHide->font();
	f.setBold(true);
	pShowHide->setFont(f);
	m_pTrayMenu->addSeparator();

	m_pTrayList = new QWidgetAction(m_pTrayMenu);

	QWidget* pWidget = new CActionWidget();
    QHBoxLayout* pLayout = new QHBoxLayout();
	pLayout->setMargin(0);
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

	/*QFrame* vFrame = new QFrame;
	vFrame->setFixedWidth(1);
	vFrame->setFrameShape(QFrame::VLine);
	vFrame->setFrameShadow(QFrame::Raised);
	pLayout->addWidget(vFrame);*/
	
	pLayout->addWidget(m_pTrayBoxes);

    m_pTrayList->setDefaultWidget(pWidget);
	m_pTrayMenu->addAction(m_pTrayList);


	m_pTrayBoxes->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pTrayBoxes, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(OnBoxMenu(const QPoint &)));
	connect(m_pTrayBoxes, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnBoxDblClick(QTreeWidgetItem*)));
	//m_pBoxMenu

	m_pTraySeparator = m_pTrayMenu->addSeparator();
	m_pTrayMenu->addAction(m_pEmptyAll);
	m_pDisableForce2 = m_pTrayMenu->addAction(tr("Pause Forcing Programs"), this, SLOT(OnDisableForce2()));
	m_pDisableForce2->setCheckable(true);
	if(m_pDisableRecovery) m_pTrayMenu->addAction(m_pDisableRecovery);
	if(m_pDisableMessages) m_pTrayMenu->addAction(m_pDisableMessages);
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

QIcon CSandMan::GetTrayIcon(bool isConnected)
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
	QPixmap base = GetIcon(IconFile, false).pixmap(size);
	QPixmap overlay;

	if (m_bIconBusy) {
		IconFile = "IconBusy";
		if (bClassic) { // classic has a different icon instead of an overlay
			IconFile += "C";
			base = GetIcon(IconFile, false).pixmap(size);
		}
		else
			overlay = GetIcon(IconFile, false).pixmap(size);
	}

	painter.drawPixmap(0, 0, base);
	if(!overlay.isNull()) painter.drawPixmap(0, 0, overlay);

	if (m_bIconDisabled) {
		IconFile = "IconDFP";
		if (bClassic) IconFile += "C";
		overlay = GetIcon(IconFile, false).pixmap(size);
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

void CSandMan::OnSysTray(QSystemTrayIcon::ActivationReason Reason)
{
	static bool TriggerSet = false;
	static bool NullifyTrigger = false;
	switch(Reason)
	{
		case QSystemTrayIcon::Context:
		{
			QMap<QString, CSandBoxPtr> Boxes = theAPI->GetAllBoxes();

			int iSysTrayFilter = theConf->GetInt("Options/SysTrayFilter", 0);

			bool bAdded = false;
			if (m_pTrayBoxes->topLevelItemCount() == 0)
				bAdded = true; // triger size refresh

			QMap<QString, QTreeWidgetItem*> OldBoxes;
			for(int i = 0; i < m_pTrayBoxes->topLevelItemCount(); ++i) 
			{
				QTreeWidgetItem* pItem = m_pTrayBoxes->topLevelItem(i);
				QString Name = pItem->data(0, Qt::UserRole).toString();
				OldBoxes.insert(Name,pItem);
			}
			
			foreach(const CSandBoxPtr & pBox, Boxes) 
			{
				if (!pBox->IsEnabled())
					continue;

				CSandBoxPlus* pBoxEx = qobject_cast<CSandBoxPlus*>(pBox.data());

				if (iSysTrayFilter == 2) { // pinned only
					if (!pBox->GetBool("PinToTray", false))
						continue;
				}
				else if (iSysTrayFilter == 1) { // active + pinned
					if (pBoxEx->GetActiveProcessCount() == 0 && !pBox->GetBool("PinToTray", false))
						continue;
				}

				QTreeWidgetItem* pItem = OldBoxes.take(pBox->GetName());
				if(!pItem)
				{
					pItem = new QTreeWidgetItem();
					pItem->setData(0, Qt::UserRole, pBox->GetName());
					pItem->setText(0, "  " + pBox->GetName().replace("_", " "));
					m_pTrayBoxes->addTopLevelItem(pItem);

					bAdded = true;
				}

				QIcon Icon = theGUI->GetBoxIcon(pBoxEx->GetType(), pBox->GetActiveProcessCount() != 0);
				pItem->setData(0, Qt::DecorationRole, Icon);
			}

			foreach(QTreeWidgetItem* pItem, OldBoxes)
				delete pItem;

			if (!OldBoxes.isEmpty() || bAdded) 
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
				int MaxHeight = scrRect.height() / 2;
				if (Height > MaxHeight) {
					Height = MaxHeight;
					if (Height < 64)
						Height = 64;
				}

				m_pTrayBoxes->setFixedHeight(Height);

				m_pTrayMenu->removeAction(m_pTrayList);
				m_pTrayMenu->insertAction(m_pTraySeparator, m_pTrayList);

				m_pTrayBoxes->setFocus();
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
