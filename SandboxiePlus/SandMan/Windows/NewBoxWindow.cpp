#include "stdafx.h"
#include "NewBoxWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"


CNewBoxWindow::CNewBoxWindow(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.buttonBox, SIGNAL(accepted()), SLOT(CreateBox()));
	connect(ui.buttonBox, SIGNAL(rejected()), SLOT(reject()));

	ui.txtName->setText(tr("New Box"));

	ui.cmbTemplates->addItem(tr("Hardened"));
	ui.cmbTemplates->addItem(tr("Default"));
	ui.cmbTemplates->setCurrentIndex(eDefault);
	ui.cmbTemplates->addItem(tr("Legacy (old sbie behaviour)"));

	ui.cmbBoxes->addItems(theAPI->GetAllBoxes().keys());

	connect(ui.radTemplate, SIGNAL(toggled(bool)), this, SLOT(OnPreset()));
	connect(ui.radCopy, SIGNAL(toggled(bool)), this, SLOT(OnPreset()));
	ui.radTemplate->setChecked(true);

	restoreGeometry(theConf->GetBlob("NewBoxWindow/Window_Geometry"));
}

CNewBoxWindow::~CNewBoxWindow()
{
	theConf->SetBlob("NewBoxWindow/Window_Geometry", saveGeometry());
}

void CNewBoxWindow::OnPreset()
{
	ui.cmbTemplates->setEnabled(ui.radTemplate->isChecked());
	ui.cmbBoxes->setEnabled(ui.radCopy->isChecked());
}

void CNewBoxWindow::CreateBox()
{
	QString Name = ui.txtName->text();
	Name.replace(" ", "_");

	SB_STATUS Status = theAPI->CreateBox(Name);

	if (!Status.IsError())
	{
		CSandBoxPtr pBox = theAPI->GetBoxByName(Name);

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
					Status = theAPI->SbieIniSet(Name, I->first, I->second, CSbieAPI::eIniInsert);
					if (Status.IsError())
						break;
				}
			}
		}
		else switch (ui.cmbTemplates->currentIndex())
		{
			case eHardened:
				pBox.objectCast<CSandBoxPlus>()->SetBool("DropAdminRights", true);
				pBox.objectCast<CSandBoxPlus>()->SetBool("ProtectRpcSs", true);
				break;
			case eLegacy:
				pBox.objectCast<CSandBoxPlus>()->SetBool("UnrestrictedSCM", true);
				pBox.objectCast<CSandBoxPlus>()->SetBool("ExposeBoxedSystem", true);
				break;
		}
	}

	if(Status.IsError())
		CSandMan::CheckResults(QList<SB_STATUS>() << Status);
	else
		accept();
}
