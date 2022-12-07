#include "stdafx.h"
#include "NewBoxWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "Views/SbieView.h"


CNewBoxWindow::CNewBoxWindow(QWidget *parent)
	: QDialog(parent)
{
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
	this->setWindowTitle(tr("Sandboxie-Plus - Create New Box"));

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

	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eHardenedPlus), tr("Hardened Sandbox with Data Protection"), (int)CSandBoxPlus::eHardenedPlus);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eHardened), tr("Security Hardened Sandbox"), (int)CSandBoxPlus::eHardened);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eDefaultPlus), tr("Sandbox with Data Protection"), (int)CSandBoxPlus::eDefaultPlus);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eDefault), tr("Standard Isolation Sandbox (Default)"), (int)CSandBoxPlus::eDefault);
	//ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eInsecure), tr("INSECURE Configuration (please change)"), (int)CSandBoxPlus::eInsecure);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eAppBoxPlus), tr("Application Compartment with Data Protection"), (int)CSandBoxPlus::eAppBoxPlus);
	ui.cmbBoxType->addItem(theGUI->GetBoxIcon(CSandBoxPlus::eAppBox), tr("Application Compartment (NO Isolation)"), (int)CSandBoxPlus::eAppBox);

	connect(ui.cmbBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnBoxTypChanged()));
	ui.cmbBoxType->setCurrentIndex(3); // default

	ui.txtName->setFocus();

	//restoreGeometry(theConf->GetBlob("NewBoxWindow/Window_Geometry"));
}

CNewBoxWindow::~CNewBoxWindow()
{
	//theConf->SetBlob("NewBoxWindow/Window_Geometry", saveGeometry());
}

void CNewBoxWindow::OnBoxTypChanged()
{
	int BoxType = ui.cmbBoxType->currentData().toInt();

	ui.lblBoxInfo->setText(theGUI->GetBoxDescription(BoxType));

	if(BoxType != CSandBoxPlus::eDefault)
		theGUI->CheckCertificate(this);
}

void CNewBoxWindow::CreateBox()
{
	m_Name = ui.txtName->text();
	int BoxType = ui.cmbBoxType->currentData().toInt();

	if (!theGUI->GetBoxView()->TestNameAndWarn(m_Name))
		return;

	m_Name.replace(" ", "_");

	SB_STATUS Status = theAPI->CreateBox(m_Name, true);

	if (!Status.IsError())
	{
		CSandBoxPtr pBox = theAPI->GetBoxByName(m_Name);

		pBox->SetBool("AutoRecover", true);

		switch (BoxType)
		{
			case CSandBoxPlus::eHardenedPlus:
			case CSandBoxPlus::eHardened:
				//pBox->SetBool("NoSecurityIsolation", false);
				pBox->SetBool("UseSecurityMode", true);
				//pBox->SetBool("DropAdminRights", true);
				//pBox->SetBool("MsiInstallerExemptions", false);
				pBox->SetBool("UsePrivacyMode", BoxType == CSandBoxPlus::eHardenedPlus);
				break;
			case CSandBoxPlus::eDefaultPlus:
			case CSandBoxPlus::eDefault:
				//pBox->SetBool("NoSecurityIsolation", false);
				pBox->SetBool("UseSecurityMode", false);
				//pBox->SetBool("DropAdminRights", false);
				//pBox->SetBool("MsiInstallerExemptions", false);
				//pBox->SetBool("RunServicesAsSystem", false);
				pBox->SetBool("UsePrivacyMode", BoxType == CSandBoxPlus::eDefaultPlus);
				break;
			case CSandBoxPlus::eAppBoxPlus:
			case CSandBoxPlus::eAppBox:
				//pBox->SetBool("UseSecurityMode", false);
				pBox->SetBool("NoSecurityIsolation", true);
				//pBox->SetBool("RunServicesAsSystem", true);
				pBox->SetBool("UsePrivacyMode", BoxType == CSandBoxPlus::eAppBoxPlus);
				//pBox->InsertText("Template", "NoUACProxy"); // proxy is always needed for exes in the box
				pBox->InsertText("Template", "RpcPortBindingsExt");
				break;
		}

		QRgb rgb = theGUI->GetBoxColor(BoxType);
		pBox->SetText("BorderColor", QString("#%1%2%3").arg(qBlue(rgb), 2, 16, QChar('0')).arg(qGreen(rgb), 2, 16, QChar('0')).arg(qRed(rgb), 2, 16, QChar('0')) + ",ttl");
	}

	if(Status.IsError())
		CSandMan::CheckResults(QList<SB_STATUS>() << Status);
	else
		accept();
}
