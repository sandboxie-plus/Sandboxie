#include "stdafx.h"
#include "NewBoxWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"


CNewBoxWindow::CNewBoxWindow(QWidget *parent)
	: QDialog(parent)
{
	this->setWindowTitle(tr("Sandboxie-Plus - Create New Box"));

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

	ui.setupUi(this);

	connect(ui.buttonBox, SIGNAL(accepted()), SLOT(CreateBox()));
	connect(ui.buttonBox, SIGNAL(rejected()), SLOT(reject()));

	QMap<QString, CSandBoxPtr> Boxes = theAPI->GetAllBoxes();

	for (int i=0;; i++) {
		QString NewName = tr("New Box");
		if (i > 0) NewName.append(" " + QString::number(i));
		if (Boxes.contains(NewName.toLower().replace(" ", "_")))
			continue;
		ui.txtName->setText(NewName);
		break;
	}

	ui.cmbTemplates->addItem(tr("Hardened"));
	ui.cmbTemplates->addItem(tr("Default"));
	ui.cmbTemplates->setCurrentIndex(eDefault);
	ui.cmbTemplates->addItem(tr("Legacy Sandboxie Behaviour"));
	// leniant
	// open

	foreach(const CSandBoxPtr& pBox, Boxes)
		ui.cmbBoxes->addItem(pBox->GetName());

	connect(ui.radTemplate, SIGNAL(toggled(bool)), this, SLOT(OnPreset()));
	connect(ui.radCopy, SIGNAL(toggled(bool)), this, SLOT(OnPreset()));
	ui.radTemplate->setChecked(true);

	ui.txtName->setFocus();

	//restoreGeometry(theConf->GetBlob("NewBoxWindow/Window_Geometry"));
}

CNewBoxWindow::~CNewBoxWindow()
{
	//theConf->SetBlob("NewBoxWindow/Window_Geometry", saveGeometry());
}

void CNewBoxWindow::OnPreset()
{
	ui.cmbTemplates->setEnabled(ui.radTemplate->isChecked());
	ui.cmbBoxes->setEnabled(ui.radCopy->isChecked());
}

void CNewBoxWindow::CreateBox()
{
	m_Name = ui.txtName->text();
	m_Name.replace(" ", "_");

	SB_STATUS Status = theAPI->CreateBox(m_Name);

	if (!Status.IsError())
	{
		CSandBoxPtr pBox = theAPI->GetBoxByName(m_Name);

		if (ui.radCopy->isChecked())
		{
			QList<QPair<QString, QString>> Settings;
			CSandBoxPtr pSrcBox = theAPI->GetBoxByName(ui.cmbBoxes->currentText());			
			qint32 status = 0;
			if(!pSrcBox.isNull()) Settings = pSrcBox->GetIniSection(&status);
			if (Settings.isEmpty())
				Status = SB_ERR(SB_FailedCopyConf, QVariantList() << ui.cmbBoxes->currentText() << (quint32)status);
			else
			{
				for (QList<QPair<QString, QString>>::iterator I = Settings.begin(); I != Settings.end(); ++I)
				{
					Status = theAPI->SbieIniSet(m_Name, I->first, I->second, CSbieAPI::eIniInsert);
					if (Status.IsError())
						break;
				}
			}
		}
		else switch (ui.cmbTemplates->currentIndex())
		{
			case eHardened:
				pBox.objectCast<CSandBoxPlus>()->SetBool("DropAdminRights", true);
				//pBox.objectCast<CSandBoxPlus>()->SetBool("FakeAdminRights", true); // Note: making the app think it has admin rights has no security downsides, but it can help with compatibility
				pBox.objectCast<CSandBoxPlus>()->SetBool("ClosePrintSpooler", true);
				break;
			case eLegacy:
				pBox.objectCast<CSandBoxPlus>()->SetBool("UnrestrictedSCM", true);
				//pBox.objectCast<CSandBoxPlus>()->SetBool("ExposeBoxedSystem", true); 
				//pBox.objectCast<CSandBoxPlus>()->SetBool("RunServicesAsSystem", true); // legacy behaviour, but there should be no normal use cases which require this
				pBox.objectCast<CSandBoxPlus>()->SetBool("OpenPrintSpooler", true);
				pBox.objectCast<CSandBoxPlus>()->InsertText("Template", "OpenSmartCard");
			default:
				pBox.objectCast<CSandBoxPlus>()->InsertText("Template", "OpenBluetooth"); // most Unity games needs that, besides most modern games are Unity based
				break;
		}
	}

	if(Status.IsError())
		CSandMan::CheckResults(QList<SB_STATUS>() << Status);
	else
		accept();
}
