#include "stdafx.h"
#include "BoxImageWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"


CBoxImageWindow::CBoxImageWindow(EAction Action, QWidget *parent)
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
	this->setWindowTitle(tr("Sandboxie-Plus - Password Entry"));

	m_Action = Action;

	connect(ui.chkShow, SIGNAL(clicked(bool)), this, SLOT(OnShowPassword()));

	connect(ui.txtImageSize, SIGNAL(textChanged(const QString&)), this, SLOT(OnImageSize()));

	connect(ui.buttonBox, SIGNAL(accepted()), SLOT(CheckPassword()));
	connect(ui.buttonBox, SIGNAL(rejected()), SLOT(reject()));

	switch (m_Action)
	{
	case eNew:
		ui.lblInfo->setText(tr("Creating new box image, please enter a secure password, and choose a disk image size."));
		ui.lblIcon->setPixmap(QPixmap::fromImage(QImage(":/Actions/LockClosed.png")));
		break;
	case eMount:
		ui.lblInfo->setText(tr("Enter Box Image password:"));
		ui.lblIcon->setPixmap(QPixmap::fromImage(QImage(":/Actions/LockOpen.png")));
		break;
	case eChange:
		ui.lblInfo->setText(tr("Enter Box Image passwords:"));
		ui.lblIcon->setPixmap(QPixmap::fromImage(QImage(":/Actions/LockClosed.png")));
		break;
	case eExport:
		ui.lblInfo->setText(tr("Enter Encryption passwords for archive export:"));
		ui.lblIcon->setPixmap(QPixmap::fromImage(QImage(":/Actions/LockClosed.png")));
		break;
	case eImport:
		ui.lblInfo->setText(tr("Enter Encryption passwords for archive import:"));
		ui.lblIcon->setPixmap(QPixmap::fromImage(QImage(":/Actions/LockOpen.png")));
		break;
	}


	if (m_Action == eNew || m_Action == eExport)
		ui.txtNewPassword->setFocus();
	else 
		ui.txtPassword->setFocus();

	if (m_Action == eNew || m_Action == eImport)
		ui.txtImageSize->setText(QString::number(2 * 1024 * 1024)); // suggest 2GB
	else {
		ui.lblImageSize->setVisible(false);
		ui.txtImageSize->setVisible(false);
		ui.lblImageSizeKb->setVisible(false);
	}

	if (m_Action == eNew || m_Action == eExport) {
		ui.lblPassword->setVisible(false);
		ui.txtPassword->setVisible(false);
	}

	if (m_Action == eMount || m_Action == eImport){
		ui.lblNewPassword->setVisible(false);
		ui.txtNewPassword->setVisible(false);
		ui.lblRepeatPassword->setVisible(false);
		ui.txtRepeatPassword->setVisible(false);
	}

	//if (!bNew) {
		ui.lblCipher->setVisible(false);
		ui.cmbCipher->setVisible(false);
	//}
	ui.cmbCipher->addItem("AES", 0);
	ui.cmbCipher->addItem("Twofish", 1);
	ui.cmbCipher->addItem("Serpent", 2);
	ui.cmbCipher->addItem("AES-Twofish", 3);
	ui.cmbCipher->addItem("Twofish-Serpent", 4);
	ui.cmbCipher->addItem("Serpent-AES", 5);
	ui.cmbCipher->addItem("AES-Twofish-Serpent", 6);

	if (m_Action != eMount)
		ui.chkProtect->setVisible(false);

	//restoreGeometry(theConf->GetBlob("BoxImageWindow/Window_Geometry"));
}

CBoxImageWindow::~CBoxImageWindow()
{
	//theConf->SetBlob("BoxImageWindow/Window_Geometry", saveGeometry());
}

void CBoxImageWindow::OnShowPassword()
{
	ui.txtPassword->setEchoMode(ui.chkShow->isChecked() ? QLineEdit::Normal : QLineEdit::Password);
	ui.txtNewPassword->setEchoMode(ui.chkShow->isChecked() ? QLineEdit::Normal : QLineEdit::Password);
	ui.txtRepeatPassword->setEchoMode(ui.chkShow->isChecked() ? QLineEdit::Normal : QLineEdit::Password);
}

void CBoxImageWindow::OnImageSize()
{
	ui.lblImageSizeKb->setText(tr("kilobytes (%1)").arg(FormatSize(GetImageSize())));
}

void CBoxImageWindow::CheckPassword()
{
	if (m_Action == eMount || m_Action == eImport) {
		m_Password = ui.txtPassword->text();
	}
	else {

		if (ui.txtNewPassword->text() != ui.txtRepeatPassword->text()) {
			QMessageBox::critical(this, "Sandboxie-Plus", tr("Passwords don't match!!!"));
			return;
		}
		if (ui.txtNewPassword->text().length() < 20) {
			if (QMessageBox::warning(this, "Sandboxie-Plus", tr("WARNING: Short passwords are easy to crack using brute force techniques!\n\n"
				"It is recommended to choose a password consisting of 20 or more characters. Are you sure you want to use a short password?")
				, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
				return;
		}

		if (m_Action == eNew || m_Action == eExport)
			m_Password = ui.txtNewPassword->text();
		else if (m_Action == eChange) {
			m_Password = ui.txtPassword->text();
			m_NewPassword = ui.txtNewPassword->text();
		}
	}
	
	if (m_Action == eNew || m_Action == eImport) {
		if (GetImageSize() < 128 * 1024 * 1024) { // ask for 256 mb but silently accept >= 128 mb
			QMessageBox::critical(this, "Sandboxie-Plus", tr("The Box Disk Image must be at least 256 MB in size, 2GB are recommended."));
			SetImageSize(256 * 1024 * 1024);
			return;
		}
	}

	accept();
}