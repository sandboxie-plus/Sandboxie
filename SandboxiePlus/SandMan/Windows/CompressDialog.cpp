#include "stdafx.h"
#include "CompressDialog.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"


CCompressDialog::CCompressDialog(QWidget *parent)
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
	this->setWindowTitle(tr("Sandboxie-Plus - Sandbox Export"));

	ui.cmbCompression->addItem(tr("Store"), 0);
	ui.cmbCompression->addItem(tr("Fastest"), 1);
	ui.cmbCompression->addItem(tr("Fast"), 3);
	ui.cmbCompression->addItem(tr("Normal"), 5);
	ui.cmbCompression->addItem(tr("Maximum"), 7);
	ui.cmbCompression->addItem(tr("Ultra"), 9);
	ui.cmbCompression->setCurrentIndex(ui.cmbCompression->findData(theConf->GetInt("Options/ExportCompression", 3)));

	connect(ui.buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(ui.buttonBox, SIGNAL(rejected()), SLOT(reject()));

	//restoreGeometry(theConf->GetBlob("CompressDialog/Window_Geometry"));
}

CCompressDialog::~CCompressDialog()
{
	//theConf->SetBlob("CompressDialog/Window_Geometry", saveGeometry());
}

int CCompressDialog::GetLevel()
{
	return ui.cmbCompression->currentData().toInt();
}

bool CCompressDialog::MakeSolid()
{
	return ui.chkSolid->isChecked();
}

void CCompressDialog::SetMustEncrypt()
{
	ui.chkEncrypt->setChecked(true);
	ui.chkEncrypt->setEnabled(false);
}

bool CCompressDialog::UseEncryption()
{
	return ui.chkEncrypt->isChecked();
}