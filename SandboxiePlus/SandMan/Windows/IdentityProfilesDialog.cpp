#include "IdentityProfilesDialog.h"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>

//---------------------------------------------------------------------------
// CIdentityProfileEditDialog
//---------------------------------------------------------------------------

CIdentityProfileEditDialog::CIdentityProfileEditDialog(CIdentityProfileStore& Store, const CIdentityProfile& Profile, QWidget* parent)
	: QDialog(parent), m_Store(Store), m_Profile(Profile)
{
	setWindowTitle(tr("Identity Profile"));
	resize(520, 360);

	QVBoxLayout* pLayout = new QVBoxLayout(this);

	QFormLayout* pForm = new QFormLayout();
	m_pName = new QLineEdit(m_Profile.Name);
	pForm->addRow(tr("Name:"), m_pName);
	pForm->addRow(tr("Id:"), new QLabel(m_Profile.Id));
	pForm->addRow(tr("Revision:"), new QLabel(QString::number(m_Profile.Revision)));
	pLayout->addLayout(pForm);

	m_pVolumes = new QTableWidget(0, 2);
	m_pVolumes->setHorizontalHeaderLabels(QStringList() << tr("Volume device") << tr("Volume serial"));
	m_pVolumes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	m_pVolumes->setSelectionBehavior(QAbstractItemView::SelectRows);
	foreach(const SIdentityVolume& Volume, m_Profile.Volumes)
		AddVolume(Volume.Device, Volume.Serial);
	pLayout->addWidget(m_pVolumes);

	QHBoxLayout* pButtons = new QHBoxLayout();
	QPushButton* pAdd = new QPushButton(tr("Add Volume"));
	QPushButton* pRemove = new QPushButton(tr("Remove Volume"));
	QPushButton* pRegenerate = new QPushButton(tr("Regenerate Serials"));
	pRegenerate->setToolTip(tr("Replaces every serial in this draft with a new random value. Nothing changes until you save, and bound sandboxes must be re-applied while stopped."));
	pButtons->addWidget(pAdd);
	pButtons->addWidget(pRemove);
	pButtons->addStretch();
	pButtons->addWidget(pRegenerate);
	pLayout->addLayout(pButtons);

	m_pInfo = new QLabel(CIdentityProfileBinding::CoverageText());
	m_pInfo->setWordWrap(true);
	pLayout->addWidget(m_pInfo);

	QDialogButtonBox* pBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
	pLayout->addWidget(pBox);

	connect(pAdd, SIGNAL(clicked(bool)), this, SLOT(OnAdd()));
	connect(pRemove, SIGNAL(clicked(bool)), this, SLOT(OnRemove()));
	connect(pRegenerate, SIGNAL(clicked(bool)), this, SLOT(OnRegenerate()));
	connect(pBox, SIGNAL(accepted()), this, SLOT(OnSave()));
	connect(pBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void CIdentityProfileEditDialog::AddVolume(const QString& Device, const QString& Serial)
{
	int Row = m_pVolumes->rowCount();
	m_pVolumes->insertRow(Row);
	m_pVolumes->setItem(Row, 0, new QTableWidgetItem(Device));
	m_pVolumes->setItem(Row, 1, new QTableWidgetItem(Serial));
}

void CIdentityProfileEditDialog::RemoveSelectedVolume()
{
	int Row = m_pVolumes->currentRow();
	if (Row >= 0)
		m_pVolumes->removeRow(Row);
}

void CIdentityProfileEditDialog::RegenerateSerials()
{
	for (int Row = 0; Row < m_pVolumes->rowCount(); Row++)
		m_pVolumes->item(Row, 1)->setText(CIdentityProfile::RandomSerial());
}

QStringList CIdentityProfileEditDialog::GetVolumesText() const
{
	QStringList Values;
	for (int Row = 0; Row < m_pVolumes->rowCount(); Row++)
		Values.append(m_pVolumes->item(Row, 0)->text() + "," + m_pVolumes->item(Row, 1)->text());
	return Values;
}

bool CIdentityProfileEditDialog::CollectProfile(CIdentityProfile& Profile, QString* pError)
{
	Profile = m_Profile;
	Profile.Name = m_pName->text().trimmed();
	Profile.Volumes.clear();
	for (int Row = 0; Row < m_pVolumes->rowCount(); Row++) {
		QString Error;
		SIdentityVolume Volume;
		Volume.Device = CIdentityProfile::NormalizeDevice(m_pVolumes->item(Row, 0)->text(), &Error);
		if (Volume.Device.isEmpty()) { if (pError) *pError = Error; return false; }
		Volume.Serial = CIdentityProfile::NormalizeSerial(m_pVolumes->item(Row, 1)->text(), &Error);
		if (Volume.Serial.isEmpty()) { if (pError) *pError = Error; return false; }
		Profile.Volumes.append(Volume);
	}
	QStringList Errors = Profile.Validate();
	if (!Errors.isEmpty()) {
		if (pError) *pError = Errors.join("; ");
		return false;
	}
	return true;
}

bool CIdentityProfileEditDialog::Save()
{
	CIdentityProfile Profile;
	if (!CollectProfile(Profile, &m_LastError))
		return false;
	if (!m_Store.Save(Profile, &m_LastError))
		return false;
	m_Profile = Profile;
	m_LastError.clear();
	return true;
}

void CIdentityProfileEditDialog::OnAdd()
{
	AddVolume("HarddiskVolume" + QString::number(m_pVolumes->rowCount() + 1), CIdentityProfile::RandomSerial());
}

void CIdentityProfileEditDialog::OnRemove()
{
	RemoveSelectedVolume();
}

void CIdentityProfileEditDialog::OnRegenerate()
{
	RegenerateSerials();
}

void CIdentityProfileEditDialog::OnSave()
{
	if (!Save()) {
		QMessageBox::warning(this, "Sandboxie-Plus", tr("The identity profile was not saved: %1").arg(m_LastError));
		return;
	}
	accept();
}

//---------------------------------------------------------------------------
// CIdentityProfilesDialog
//---------------------------------------------------------------------------

CIdentityProfilesDialog::CIdentityProfilesDialog(CIdentityProfileStore& Store, UsersFunc Users, QWidget* parent)
	: QDialog(parent), m_Store(Store), m_Users(Users)
{
	setWindowTitle(tr("Identity Profiles"));
	resize(640, 400);

	QVBoxLayout* pLayout = new QVBoxLayout(this);

	m_pList = new QTreeWidget();
	m_pList->setHeaderLabels(QStringList() << tr("Name") << tr("Revision") << tr("Volumes") << tr("Used by") << tr("Id"));
	m_pList->setRootIsDecorated(false);
	pLayout->addWidget(m_pList);

	QHBoxLayout* pButtons = new QHBoxLayout();
	QPushButton* pNew = new QPushButton(tr("New"));
	m_pEdit = new QPushButton(tr("Edit"));
	m_pClone = new QPushButton(tr("Clone"));
	m_pRegenerate = new QPushButton(tr("Regenerate"));
	QPushButton* pImport = new QPushButton(tr("Import"));
	m_pExport = new QPushButton(tr("Export"));
	m_pDelete = new QPushButton(tr("Delete"));
	foreach(QPushButton* pButton, QList<QPushButton*>() << pNew << m_pEdit << m_pClone << m_pRegenerate << pImport << m_pExport << m_pDelete)
		pButtons->addWidget(pButton);
	pButtons->addStretch();
	pLayout->addLayout(pButtons);

	m_pInfo = new QLabel(CIdentityProfileBinding::CoverageText());
	m_pInfo->setWordWrap(true);
	pLayout->addWidget(m_pInfo);

	QDialogButtonBox* pBox = new QDialogButtonBox(QDialogButtonBox::Close);
	pLayout->addWidget(pBox);

	connect(pNew, SIGNAL(clicked(bool)), this, SLOT(OnNew()));
	connect(m_pEdit, SIGNAL(clicked(bool)), this, SLOT(OnEdit()));
	connect(m_pClone, SIGNAL(clicked(bool)), this, SLOT(OnClone()));
	connect(m_pRegenerate, SIGNAL(clicked(bool)), this, SLOT(OnRegenerate()));
	connect(pImport, SIGNAL(clicked(bool)), this, SLOT(OnImport()));
	connect(m_pExport, SIGNAL(clicked(bool)), this, SLOT(OnExport()));
	connect(m_pDelete, SIGNAL(clicked(bool)), this, SLOT(OnDelete()));
	connect(m_pList, SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectionChanged()));
	connect(m_pList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnEdit()));
	connect(pBox, SIGNAL(rejected()), this, SLOT(reject()));

	Reload();
}

void CIdentityProfilesDialog::Reload()
{
	QString Selected = GetSelectedId();
	m_pList->clear();
	QStringList Problems;
	foreach(const CIdentityProfile& Profile, m_Store.List(&Problems)) {
		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		pItem->setText(0, Profile.Name);
		pItem->setText(1, QString::number(Profile.Revision));
		pItem->setText(2, QString::number(Profile.Volumes.size()));
		pItem->setText(3, m_Users ? m_Users(Profile.Id).join(", ") : QString());
		pItem->setText(4, Profile.Id);
		m_pList->addTopLevelItem(pItem);
	}
	for (int i = 0; i < m_pList->columnCount(); i++)
		m_pList->resizeColumnToContents(i);
	if (!Problems.isEmpty())
		Report(tr("Some profile files were skipped:\n%1").arg(Problems.join("\n")));
	SelectId(Selected);
	OnSelectionChanged();
}

QString CIdentityProfilesDialog::GetSelectedId() const
{
	QTreeWidgetItem* pItem = m_pList->currentItem();
	return pItem ? pItem->text(4) : QString();
}

void CIdentityProfilesDialog::SelectId(const QString& Id)
{
	for (int i = 0; i < m_pList->topLevelItemCount(); i++) {
		if (m_pList->topLevelItem(i)->text(4) == Id) {
			m_pList->setCurrentItem(m_pList->topLevelItem(i));
			return;
		}
	}
}

void CIdentityProfilesDialog::OnSelectionChanged()
{
	bool Has = !GetSelectedId().isEmpty();
	m_pEdit->setEnabled(Has);
	m_pClone->setEnabled(Has);
	m_pRegenerate->setEnabled(Has);
	m_pExport->setEnabled(Has);
	m_pDelete->setEnabled(Has);
}

void CIdentityProfilesDialog::Report(const QString& Text)
{
	m_LastError = Text;
	if (isVisible())
		QMessageBox::warning(this, "Sandboxie-Plus", Text);
}

void CIdentityProfilesDialog::OnNew()
{
	CIdentityProfile Profile;
	Profile.Id = CIdentityProfile::NewId();
	Profile.Name = tr("New Profile");
	SIdentityVolume Volume;
	Volume.Device = "HarddiskVolume1";
	Volume.Serial = CIdentityProfile::RandomSerial();
	Profile.Volumes.append(Volume);
	CIdentityProfileEditDialog Dialog(m_Store, Profile, this);
	if (Dialog.exec() == QDialog::Accepted) {
		Reload();
		SelectId(Dialog.GetProfile().Id);
	}
}

void CIdentityProfilesDialog::OnEdit()
{
	CIdentityProfile Profile;
	QString Error;
	if (!m_Store.Load(GetSelectedId(), Profile, &Error)) {
		Report(Error);
		return;
	}
	CIdentityProfileEditDialog Dialog(m_Store, Profile, this);
	if (Dialog.exec() == QDialog::Accepted)
		Reload();
}

bool CIdentityProfilesDialog::CloneSelected(const QString& NewName)
{
	CIdentityProfile Profile;
	if (!m_Store.Load(GetSelectedId(), Profile, &m_LastError))
		return false;
	CIdentityProfile Copy = Profile.Clone(NewName);
	if (!m_Store.Save(Copy, &m_LastError))
		return false;
	Reload();
	SelectId(Copy.Id);
	return true;
}

void CIdentityProfilesDialog::OnClone()
{
	CIdentityProfile Profile;
	if (!m_Store.Load(GetSelectedId(), Profile, &m_LastError)) {
		Report(m_LastError);
		return;
	}
	bool Ok = false;
	QString Name = QInputDialog::getText(this, "Sandboxie-Plus", tr("Name of the copy (same serials, new profile id):"), QLineEdit::Normal, Profile.Name + " (copy)", &Ok);
	if (!Ok || Name.trimmed().isEmpty())
		return;
	if (!CloneSelected(Name.trimmed()))
		Report(m_LastError);
}

bool CIdentityProfilesDialog::RegenerateSelected()
{
	CIdentityProfile Profile;
	if (!m_Store.Load(GetSelectedId(), Profile, &m_LastError))
		return false;
	Profile.Regenerate();
	if (!m_Store.Save(Profile, &m_LastError))
		return false;
	Reload();
	return true;
}

void CIdentityProfilesDialog::OnRegenerate()
{
	QStringList Users = m_Users ? m_Users(GetSelectedId()) : QStringList();
	QString Text = tr("Replace every serial of this profile with new random values?");
	if (!Users.isEmpty())
		Text += "\n\n" + tr("Sandboxes using it (%1) keep their current configuration until the profile is re-applied while they are stopped.").arg(Users.join(", "));
	if (QMessageBox::question(this, "Sandboxie-Plus", Text, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
		return;
	if (!RegenerateSelected())
		Report(m_LastError);
}

bool CIdentityProfilesDialog::DeleteSelected(bool Force)
{
	QString Id = GetSelectedId();
	QStringList Users = m_Users ? m_Users(Id) : QStringList();
	if (!Users.isEmpty() && !Force) {
		m_LastError = tr("The profile is still used by: %1").arg(Users.join(", "));
		return false;
	}
	if (!m_Store.Remove(Id, &m_LastError))
		return false;
	Reload();
	return true;
}

void CIdentityProfilesDialog::OnDelete()
{
	QStringList Users = m_Users ? m_Users(GetSelectedId()) : QStringList();
	if (!Users.isEmpty()) {
		Report(tr("The profile is still used by: %1\nRemove it from those sandboxes first.").arg(Users.join(", ")));
		return;
	}
	if (QMessageBox::question(this, "Sandboxie-Plus", tr("Delete the selected identity profile?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
		return;
	if (!DeleteSelected(false))
		Report(m_LastError);
}

bool CIdentityProfilesDialog::ImportFile(const QString& Path, QString* pNewId)
{
	CIdentityProfile Profile;
	if (!m_Store.Import(Path, Profile, &m_LastError))
		return false;
	if (!m_Store.Save(Profile, &m_LastError))
		return false;
	if (pNewId) *pNewId = Profile.Id;
	Reload();
	SelectId(Profile.Id);
	return true;
}

void CIdentityProfilesDialog::OnImport()
{
	QString Path = QFileDialog::getOpenFileName(this, tr("Import identity profile"), QString(), tr("Identity profile (*.json)"));
	if (Path.isEmpty())
		return;
	if (!ImportFile(Path))
		Report(m_LastError);
}

bool CIdentityProfilesDialog::ExportSelected(const QString& Path)
{
	CIdentityProfile Profile;
	if (!m_Store.Load(GetSelectedId(), Profile, &m_LastError))
		return false;
	return m_Store.Export(Profile, Path, &m_LastError);
}

void CIdentityProfilesDialog::OnExport()
{
	QString Path = QFileDialog::getSaveFileName(this, tr("Export identity profile"), GetSelectedId() + ".json", tr("Identity profile (*.json)"));
	if (Path.isEmpty())
		return;
	if (!ExportSelected(Path))
		Report(m_LastError);
}
