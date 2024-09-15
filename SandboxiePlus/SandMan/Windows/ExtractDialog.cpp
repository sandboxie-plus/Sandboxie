#include "stdafx.h"
#include "ExtractDialog.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"


CExtractDialog::CExtractDialog(const QString& Name, QWidget *parent)
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
	this->setWindowTitle(tr("Sandboxie-Plus - Sandbox Import"));

	ui.txtName->setText(Name);

	QString Location = theAPI->GetGlobalSettings()->GetText("FileRootPath", "\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%");
    ui.cmbRoot->addItem(Location/*.replace("%SANDBOX%", field("boxName").toString())*/);
    QStringList StdLocations = QStringList() 
        << "\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%" 
        << "\\??\\%SystemDrive%\\Sandbox\\%SANDBOX%" 
        << "\\??\\%SystemDrive%\\Users\\%USER%\\Sandbox\\%SANDBOX%";
    foreach(auto StdLocation, StdLocations) {
        if (StdLocation != Location)
            ui.cmbRoot->addItem(StdLocation);
    }

	connect(ui.btnRoot, &QPushButton::clicked, [&]() {
        QString FilePath = QFileDialog::getExistingDirectory(this, tr("Select Directory"));
	    if (!FilePath.isEmpty())
		    ui.cmbRoot->setCurrentText(FilePath.replace("/", "\\"));
    });

	connect(ui.buttonBox, SIGNAL(accepted()), SLOT(OnAccept()));
	connect(ui.buttonBox, SIGNAL(rejected()), SLOT(reject()));

	//restoreGeometry(theConf->GetBlob("ExtractDialog/Window_Geometry"));
}

CExtractDialog::~CExtractDialog()
{
	//theConf->SetBlob("ExtractDialog/Window_Geometry", saveGeometry());
}

void CExtractDialog::OnAccept()
{
	CSandBoxPtr pBox = theAPI->GetBoxByName(ui.txtName->text());
	if (!pBox.isNull()) {
		QMessageBox::warning(this, "Sandboxie-Plus", tr("This name is already in use, please select an alternative box name"));
		return;
	}

	accept();
}

QString CExtractDialog::GetRoot() const
{
	QString Location = theAPI->GetGlobalSettings()->GetText("FileRootPath", "\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%");
	if (Location == ui.cmbRoot->currentText())
		return "";
	return ui.cmbRoot->currentText();
}