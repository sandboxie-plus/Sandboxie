#include "stdafx.h"
#include "BoxTransfer.h"
#include "SandMan.h"
#include "..\MiscHelpers\Common\Common.h"
#include "..\MiscHelpers\Common\OtherFunctions.h"
#include "../MiscHelpers/Archive/Archive.h"
#include "Windows/CompressDialog.h"
#include "Windows/BoxImageWindow.h"
#include "Windows/ExtractDialog.h"
#include "../MiscHelpers/Common/Finder.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QtConcurrent>

#include <windows.h>


///////////////////////////////////////////////////////////////////////////////
// QFileX helper - same as in SbiePlusAPI.cpp
//
class QFileXProgress : public QFile {
public:
	QFileXProgress(const QString& path, const CSbieProgressPtr& pProgress, CArchive* pArchive) : QFile(path)
	{
		m_pProgress = pProgress;
		m_pArchive = pArchive;
	}

	bool open(OpenMode flags) override
	{
		if (m_pProgress->IsCanceled())
			return false;
		m_pProgress->ShowMessage(Split2(fileName(), "/", true).second);
		m_pProgress->SetProgress((int)(m_pArchive->GetProgress() * 100.0));
		return QFile::open(flags);
	}

	qint64 size() const override
	{
		qint64 Size = QFile::size();
		if (QFileInfo(fileName()).isShortcut())
		{
			QFile File(fileName());
			if (File.open(QFile::ReadOnly))
				Size = File.size();
			File.close();
		}
		return Size;
	}

protected:
	CSbieProgressPtr m_pProgress;
	CArchive* m_pArchive;
};


///////////////////////////////////////////////////////////////////////////////
// CBoxTransferDialog
//

CBoxTransferDialog::CBoxTransferDialog(EMode mode, QWidget* parent)
	: QDialog(parent), m_Mode(mode)
{
	if (mode == eExport)
		setWindowTitle(tr("Export Sandboxes"));
	else
		setWindowTitle(tr("Import Sandboxes"));
	setMinimumSize(500, 400);

	QVBoxLayout* pLayout = new QVBoxLayout(this);

	m_pBoxTree = new QTreeWidget(this);
	if (mode == eExport) {
		m_pBoxTree->setHeaderLabels(QStringList() << tr("Box Name") << tr("Status"));
		m_pBoxTree->setColumnCount(2);
	} else {
		m_pBoxTree->setHeaderLabels(QStringList() << tr("Archive Box Name") << tr("Import As") << tr("Conflict"));
		m_pBoxTree->setColumnCount(3);
	}
	m_pBoxTree->header()->setStretchLastSection(true);
	m_pBoxTree->setRootIsDecorated(false);
	connect(m_pBoxTree, &QTreeWidget::itemChanged, this, &CBoxTransferDialog::OnItemChanged);
	pLayout->addWidget(m_pBoxTree);

	pLayout->addWidget(new CFinder(this, this, 0));

	m_pGlobalConfig = new QCheckBox(mode == eExport ? tr("Export Global Configuration") : tr("Import Global Configuration"), this);
	pLayout->addWidget(m_pGlobalConfig);

	QHBoxLayout* pButtonRow = new QHBoxLayout();
	m_pSelectAll = new QPushButton(tr("Select All"), this);
	connect(m_pSelectAll, &QPushButton::clicked, this, &CBoxTransferDialog::OnSelectAll);
	pButtonRow->addWidget(m_pSelectAll);

	m_pSelectNone = new QPushButton(tr("Select None"), this);
	connect(m_pSelectNone, &QPushButton::clicked, this, &CBoxTransferDialog::OnSelectNone);
	pButtonRow->addWidget(m_pSelectNone);
	pButtonRow->addStretch();
	pLayout->addLayout(pButtonRow);

	QDialogButtonBox* pButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(pButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(pButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	pLayout->addWidget(pButtons);
}

void CBoxTransferDialog::PopulateExportList(const QList<CSandBoxPtr>& SandBoxes)
{
	QMap<QString, CSandBoxPtr> Boxes = theAPI->GetAllBoxes();
	for (auto I = Boxes.begin(); I != Boxes.end(); ++I) {
		auto pBoxEx = I.value().objectCast<CSandBoxPlus>();
		if (!pBoxEx) continue;

		bool inUse = theAPI->HasProcesses(pBoxEx->GetName());

		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable);
		pItem->setCheckState(0, Qt::Unchecked);
		pItem->setIcon(0, theGUI->GetBoxIcon(pBoxEx->GetType(), inUse));
		pItem->setText(0, pBoxEx->GetName());

		QStringList Status;
		if (inUse)
			Status.append(tr("Running"));
		if (pBoxEx->UseImageFile())
			Status.append(tr("Encrypted"));
		if (!pBoxEx->IsInitialized())
			Status.append(tr("Empty"));
		pItem->setText(1, Status.join(", "));
		pItem->setData(0, Qt::UserRole, pBoxEx->GetName());

		if(!SandBoxes.isEmpty() && SandBoxes.contains(I.value()))
			pItem->setCheckState(0, Qt::Checked);

		m_pBoxTree->addTopLevelItem(pItem);
	}
	m_pBoxTree->resizeColumnToContents(0);
}

void CBoxTransferDialog::LoadArchiveContents(const QStringList& boxNames, bool hasGlobalConfig)
{
	m_pGlobalConfig->setEnabled(hasGlobalConfig);
	if (hasGlobalConfig)
		m_pGlobalConfig->setChecked(true);
	else
		m_pGlobalConfig->setChecked(false);

	for (const QString& name : boxNames) {
		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
		pItem->setCheckState(0, Qt::Checked);
		pItem->setText(0, name);
		pItem->setText(1, name); // default "Import As" = same name
		pItem->setData(0, Qt::UserRole, name); // original archive name

		// Try to set icon if a box with this name already exists
		CSandBoxPtr pExisting = theAPI->GetBoxByName(name);
		if (!pExisting.isNull()) {
			auto pBoxEx = pExisting.objectCast<CSandBoxPlus>();
			if (pBoxEx)
				pItem->setIcon(0, theGUI->GetBoxIcon(pBoxEx->GetType(), false));
		}

		m_pBoxTree->addTopLevelItem(pItem);
	}
	m_pBoxTree->resizeColumnToContents(0);

	UpdateConflictIndicators();
}

void CBoxTransferDialog::UpdateConflictIndicators()
{
	if (m_Mode != eImport) return;

	for (int i = 0; i < m_pBoxTree->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = m_pBoxTree->topLevelItem(i);
		QString importName = pItem->text(1);
		CSandBoxPtr pExisting = theAPI->GetBoxByName(importName);
		if (!pExisting.isNull())
			pItem->setText(2, tr("Exists!"));
		else
			pItem->setText(2, "");
	}
}

bool CBoxTransferDialog::ExportGlobalConfig() const
{
	return m_pGlobalConfig->isChecked();
}

QStringList CBoxTransferDialog::GetSelectedBoxes() const
{
	QStringList boxes;
	for (int i = 0; i < m_pBoxTree->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = m_pBoxTree->topLevelItem(i);
		if (pItem->checkState(0) == Qt::Checked)
			boxes.append(pItem->data(0, Qt::UserRole).toString());
	}
	return boxes;
}

QMap<QString, QString> CBoxTransferDialog::GetBoxNameMapping() const
{
	QMap<QString, QString> mapping;
	for (int i = 0; i < m_pBoxTree->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = m_pBoxTree->topLevelItem(i);
		if (pItem->checkState(0) == Qt::Checked) {
			QString originalName = pItem->data(0, Qt::UserRole).toString();
			QString importName = pItem->text(1);
			mapping.insert(originalName, importName);
		}
	}
	return mapping;
}

void CBoxTransferDialog::OnSelectAll()
{
	for (int i = 0; i < m_pBoxTree->topLevelItemCount(); i++)
		m_pBoxTree->topLevelItem(i)->setCheckState(0, Qt::Checked);
}

void CBoxTransferDialog::OnSelectNone()
{
	for (int i = 0; i < m_pBoxTree->topLevelItemCount(); i++)
		m_pBoxTree->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
}

void CBoxTransferDialog::OnItemChanged(QTreeWidgetItem* item, int column)
{
	if (m_Mode == eImport && column == 1)
		UpdateConflictIndicators();
}

void CBoxTransferDialog::SetFilter(const QRegularExpression& Exp, int iOptions, int Column)
{
	for (int i = 0; i < m_pBoxTree->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = m_pBoxTree->topLevelItem(i);
		if (!Exp.isValid() || Exp.pattern().isEmpty())
			pItem->setHidden(false);
		else
			pItem->setHidden(!Exp.match(pItem->text(0)).hasMatch());
	}
}


///////////////////////////////////////////////////////////////////////////////
// Export
//

static void ExportMultiBoxesAsync(const CSbieProgressPtr& pProgress, const QString& exportPath,
	const QStringList& boxNames, const QMap<QString, QString>& boxRoots, const QMap<QString, QString>& boxConfigs,
	bool exportGlobalConfig, const QString& globalConfig, const QVariantMap& params)
{
	CArchive Archive(exportPath);

	QMap<int, QIODevice*> Files;
	QMap<int, quint32> Attributes;

	// Global config
	if (exportGlobalConfig && !globalConfig.isEmpty()) {
		QTemporaryFile* pTempFile = new QTemporaryFile();
		pTempFile->open();
		pTempFile->write(globalConfig.toUtf8());
		pTempFile->close();

		int ArcIndex = Archive.AddFile("GlobalConfig.ini");
		if (ArcIndex != -1)
			Files.insert(ArcIndex, pTempFile);
		else
			delete pTempFile;
	}

	// Per-box files
	for (const QString& boxName : boxNames) {
		if (pProgress->IsCanceled()) break;

		QString rootPath = boxRoots.value(boxName);
		QString configSection = boxConfigs.value(boxName);

		// Write BoxConfig.ini to box root temporarily, then add to archive
		QTemporaryFile* pConfigFile = new QTemporaryFile();
		pConfigFile->open();
		pConfigFile->write(configSection.toUtf8());
		pConfigFile->close();

		int ConfigIndex = Archive.AddFile(boxName + "/BoxConfig.ini");
		if (ConfigIndex != -1)
			Files.insert(ConfigIndex, pConfigFile);
		else
			delete pConfigFile;

		// Add all box files
		QStringList FileList = ListDir(rootPath + "\\");
		for (const QString& File : FileList) {
			if (pProgress->IsCanceled()) break;

			StrPair RootName = Split2(File, "\\", true);
			if (RootName.second.isEmpty()) {
				RootName.second = RootName.first;
				RootName.first = "";
			}
			int ArcIndex = Archive.AddFile(boxName + "/" + RootName.second);
			if (ArcIndex != -1) {
				QString FileName = (RootName.first.isEmpty() ? rootPath + "\\" : RootName.first) + RootName.second;
				Files.insert(ArcIndex, new QFileXProgress(FileName, pProgress, &Archive));
				Attributes.insert(ArcIndex, GetFileAttributesW(QString(FileName).replace("/", "\\").toStdWString().c_str()));
			}
		}
	}

	if (params.contains("password"))
		Archive.SetPassword(params["password"].toString());

	SArcInfo Info = GetArcInfo(exportPath);

	SCompressParams CompressParams;
	CompressParams.iLevel = params["level"].toInt();
	CompressParams.bSolid = params["solid"].toBool();
	CompressParams.b7z = Info.ArchiveExt != "zip";

	SB_STATUS Status = SB_OK;
	if (!pProgress->IsCanceled()) {
		if (!Archive.Update(&Files, true, &CompressParams, &Attributes))
			Status = SB_ERR((ESbieMsgCodes)SBX_7zCreateFailed);
	}

	pProgress->Finish(Status);
}

static void ExportSingleBox(QWidget* parent, const CSandBoxPtr& pBox)
{
	auto pBoxEx = pBox.objectCast<CSandBoxPlus>();

	CCompressDialog optWnd(parent);
	if (pBoxEx->UseImageFile())
		optWnd.SetMustEncrypt();
	if (theGUI->SafeExec(&optWnd) != QDialog::Accepted)
		return;

	QString Password;
	if (optWnd.UseEncryption()) {
		CBoxImageWindow pwWnd(CBoxImageWindow::eExport, parent);
		if (theGUI->SafeExec(&pwWnd) != QDialog::Accepted)
			return;
		Password = pwWnd.GetPassword();
	}

	QString Path = QFileDialog::getSaveFileName(parent, CBoxTransferDialog::tr("Select file name"), pBox->GetName() + optWnd.GetFormat(), CBoxTransferDialog::tr("7-Zip Archive (*.7z);;Zip Archive (*.zip)"));
	if (Path.isEmpty())
		return;

	SB_PROGRESS Status = pBoxEx->ExportBox(Path, Password, optWnd.GetLevel(), optWnd.MakeSolid());
	if (Status.GetStatus() == OP_ASYNC)
		theGUI->AddAsyncOp(Status.GetValue(), false, CBoxTransferDialog::tr("Exporting: %1").arg(Path));
}

void ExportMultiBoxes(QWidget* parent, const QList<CSandBoxPtr>& SandBoxes)
{
	if (!CArchive::IsInit()) {
		QMessageBox::critical(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("7-Zip library is not available."));
		return;
	}

	if (SandBoxes.size() == 1) {
		ExportSingleBox(parent, SandBoxes.first());
		return;
	}

	// 1. Show selection dialog
	CBoxTransferDialog dlg(CBoxTransferDialog::eExport, parent);
	dlg.PopulateExportList(SandBoxes);
	if (theGUI->SafeExec(&dlg) != QDialog::Accepted)
		return;

	QStringList selectedBoxes = dlg.GetSelectedBoxes();
	bool exportGlobalConfig = dlg.ExportGlobalConfig();

	if (selectedBoxes.isEmpty() && !exportGlobalConfig) {
		QMessageBox::information(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("Nothing selected for export."));
		return;
	}

	// 2. Compression options
	CCompressDialog compDlg(parent);
	if (theGUI->SafeExec(&compDlg) != QDialog::Accepted)
		return;

	// 3. Password if encryption selected
	QString Password;
	if (compDlg.UseEncryption()) {
		CBoxImageWindow pwWnd(CBoxImageWindow::eExport, parent);
		if (theGUI->SafeExec(&pwWnd) != QDialog::Accepted)
			return;
		Password = pwWnd.GetPassword();
	}

	// 4. Save file dialog
	QString Path = QFileDialog::getSaveFileName(parent, CBoxTransferDialog::tr("Export Sandboxes"),
		"SandboxExport" + compDlg.GetFormat(),
		CBoxTransferDialog::tr("7-Zip Archive (*.7z);;Zip Archive (*.zip)"));
	if (Path.isEmpty())
		return;

	// 5. Pre-collect data on main thread
	QStringList validBoxes;
	QMap<QString, QString> boxRoots;
	QMap<QString, QString> boxConfigs;

	for (const QString& boxName : selectedBoxes) {
		// Check running processes
		if (theAPI->HasProcesses(boxName)) {
			int ret = QMessageBox::warning(parent, "Sandboxie-Plus",
				CBoxTransferDialog::tr("Sandbox '%1' has running processes. Skip it?").arg(boxName),
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			if (ret == QMessageBox::Cancel) return;
			if (ret == QMessageBox::Yes) continue;
		}

		CSandBoxPtr pBox = theAPI->GetBoxByName(boxName);
		if (!pBox) continue;

		auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
		if (!pBoxEx) continue;

		if (!pBoxEx->IsInitialized()) {
			// Empty box - still export the config but warn
		}

		boxRoots.insert(boxName, pBoxEx->GetFileRoot());
		boxConfigs.insert(boxName, pBoxEx->SbieIniGetEx(boxName, ""));
		validBoxes.append(boxName);
	}

	QString globalConfig;
	if (exportGlobalConfig)
		globalConfig = theAPI->SbieIniGetEx("GlobalSettings", "");

	// 6. Launch async
	QVariantMap vParams;
	if (!Password.isEmpty())
		vParams["password"] = Password;
	vParams["level"] = compDlg.GetLevel();
	vParams["solid"] = compDlg.MakeSolid();

	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
	QtConcurrent::run(ExportMultiBoxesAsync, pProgress, Path, validBoxes, boxRoots, boxConfigs, exportGlobalConfig, globalConfig, vParams);
	theGUI->AddAsyncOp(pProgress, false, CBoxTransferDialog::tr("Exporting: %1").arg(Path));
}


///////////////////////////////////////////////////////////////////////////////
// Import
//

static void ImportMultiBoxesAsync(const CSbieProgressPtr& pProgress, const QString& importPath, const QString& password,
	const QMap<QString, QString>& boxNameMapping, const QMap<QString, QString>& boxRoots,
	bool importGlobalConfig)
{
	CArchive Archive(importPath);

	if (!password.isEmpty())
		Archive.SetPassword(password);

	if (Archive.Open() != ERR_7Z_OK) {
		pProgress->Finish(SB_ERR((ESbieMsgCodes)SBX_7zOpenFailed));
		return;
	}

	QMap<int, QIODevice*> Files;

	// Collect config data to apply after extraction
	struct SConfigEntry {
		int ArcIndex;
		QString BoxName;
		QString TempPath;
	};
	QList<SConfigEntry> configEntries;
	int globalConfigIndex = -1;
	QString globalConfigTempPath;

	for (int i = 0; i < Archive.FileCount(); i++) {
		int ArcIndex = Archive.FindByIndex(i);
		if (Archive.FileProperty(ArcIndex, "IsDir").toBool())
			continue;

		QString FilePath = Archive.FileProperty(ArcIndex, "Path").toString();
		FilePath.replace("\\", "/");

		// Check for root-level GlobalConfig.ini
		if (FilePath == "GlobalConfig.ini" && importGlobalConfig) {
			QString TempPath = QDir::tempPath() + "/SbiePlus_GlobalConfig_" + QString::number(QCoreApplication::applicationPid()) + ".ini";
			globalConfigIndex = ArcIndex;
			globalConfigTempPath = TempPath;
			Files.insert(ArcIndex, new QFileXProgress(TempPath, pProgress, &Archive));
			continue;
		}

		// Parse boxName/relative path
		int sep = FilePath.indexOf('/');
		if (sep <= 0) continue;

		QString archiveBoxName = FilePath.left(sep);
		QString relPath = FilePath.mid(sep + 1);

		if (!boxNameMapping.contains(archiveBoxName))
			continue;

		QString newBoxName = boxNameMapping.value(archiveBoxName);
		QString boxRoot = boxRoots.value(newBoxName);
		if (boxRoot.isEmpty()) continue;

		if (relPath == "BoxConfig.ini") {
			// Extract config to temp, apply later
			QString TempPath = QDir::tempPath() + "/SbiePlus_BoxConfig_" + newBoxName + "_" + QString::number(QCoreApplication::applicationPid()) + ".ini";
			SConfigEntry entry;
			entry.ArcIndex = ArcIndex;
			entry.BoxName = newBoxName;
			entry.TempPath = TempPath;
			configEntries.append(entry);
			Files.insert(ArcIndex, new QFileXProgress(TempPath, pProgress, &Archive));
		} else {
			// Regular file - extract to box root
			Files.insert(ArcIndex, new QFileXProgress(CArchive::PrepareExtraction(relPath, boxRoot + "\\"), pProgress, &Archive));
		}
	}

	SB_STATUS Status = SB_OK;
	if (!Archive.Extract(&Files))
		Status = SB_ERR((ESbieMsgCodes)SBX_7zExtractFailed);

	if (!Status.IsError() && !pProgress->IsCanceled()) {

		// Apply global config
		if (globalConfigIndex != -1 && !globalConfigTempPath.isEmpty()) {
			QFile File(globalConfigTempPath);
			if (File.open(QFile::ReadOnly)) {
				QMetaObject::invokeMethod(theAPI, "SbieIniSetSection", Qt::BlockingQueuedConnection,
					Q_ARG(QString, QString("GlobalSettings")),
					Q_ARG(QString, QString::fromUtf8(File.readAll()))
				);
				File.close();
			}
			File.remove();
		}

		// Apply box configs
		for (const SConfigEntry& entry : configEntries) {
			QFile File(entry.TempPath);
			if (File.open(QFile::ReadOnly)) {
				QString configContent = QString::fromUtf8(File.readAll());
				File.close();

				// Replace box name in section header if renamed
				// The section header is typically [BoxName] on the first line
				// We need to handle the case where the archive box name differs from the new name
				// SbieIniSetSection takes the section name separately, so we just need to pass the content

				QMetaObject::invokeMethod(theAPI, "SbieIniSetSection", Qt::BlockingQueuedConnection,
					Q_ARG(QString, entry.BoxName),
					Q_ARG(QString, configContent)
				);
			}
			File.remove();
		}
	} else {
		// Clean up temp files on error
		if (!globalConfigTempPath.isEmpty())
			QFile::remove(globalConfigTempPath);
		for (const SConfigEntry& entry : configEntries)
			QFile::remove(entry.TempPath);
	}

	pProgress->Finish(Status);
}

static void ImportSingleBox(QWidget* parent, const QString& Path, const QString& Password, quint64 ImageSize)
{
	StrPair PathName = Split2(Path, "/", true);
	StrPair NameEx = Split2(PathName.second, ".", true);
	QString Name = NameEx.first;

	CExtractDialog optWnd(Name, parent);
	if (!Password.isEmpty())
		optWnd.ShowNoCrypt();
	if (theGUI->SafeExec(&optWnd) != QDialog::Accepted)
		return;
	Name = optWnd.GetName();
	QString BoxRoot = optWnd.GetRoot();

	CSandBoxPtr pBox;
	SB_PROGRESS Status = theAPI->CreateBox(Name);
	if (!Status.IsError()) {
		pBox = theAPI->GetBoxByName(Name);
		if (pBox) {
			auto pBoxEx = pBox.objectCast<CSandBoxPlus>();

			if (!BoxRoot.isEmpty())
				pBox->SetFileRoot(BoxRoot);

			if (!Password.isEmpty() && !optWnd.IsNoCrypt()) {
				Status = pBoxEx->ImBoxCreate(ImageSize / 1024, Password);
				if (!Status.IsError())
					Status = pBoxEx->ImBoxMount(Password, true, true);
			}

			if (!Status.IsError())
				Status = pBoxEx->ImportBox(Path, Password);

			// always overwrite restored FileRootPath
			pBox->SetText("FileRootPath", BoxRoot);
		}
	}

	if (Status.GetStatus() == OP_ASYNC) {
		Status = theGUI->AddAsyncOp(Status.GetValue(), true, CBoxTransferDialog::tr("Importing: %1").arg(Path));
		if (Status.IsError()) {
			theGUI->DeleteBoxContent(pBox, CSandMan::eForDelete);
			pBox->RemoveBox();
		}
	}
	else
		theGUI->CheckResults(QList<SB_STATUS>() << Status, parent);
}

QStringList ImportMultiBoxes(QWidget* parent)
{
	if (!CArchive::IsInit()) {
		QMessageBox::critical(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("7-Zip library is not available."));
		return QStringList();
	}

	// 1. Pick archive
	QString Path = QFileDialog::getOpenFileName(parent, CBoxTransferDialog::tr("Select file name"),
		"", CBoxTransferDialog::tr("7-Zip Archive (*.7z);;Zip Archive (*.zip)"));
	if (Path.isEmpty())
		return QStringList();

	// 2. Open and handle password
	QString Password;
	quint64 ImageSize = 0;
	CArchive Archive(Path);
	int Ret = Archive.Open();
	if (Ret == ERR_7Z_PASSWORD_REQUIRED) {
		for (;;) {
			CBoxImageWindow window(CBoxImageWindow::eImport, parent);
			if (theGUI->SafeExec(&window) != QDialog::Accepted)
				return QStringList();
			Archive.SetPassword(window.GetPassword());
			Ret = Archive.Open();
			if (Ret != ERR_7Z_OK) {
				QMessageBox::critical(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("Failed to open archive, wrong password?"));
				continue;
			}
			Password = window.GetPassword();
			ImageSize = window.GetImageSize();
			break;
		}
	}
	if (Ret != ERR_7Z_OK) {
		QMessageBox::critical(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("Failed to open archive (%1)!").arg(Ret));
		return QStringList();
	}

	// 3. Scan archive index to detect format
	QSet<QString> boxNamesSet;
	bool hasGlobalConfig = false;
	bool hasSingleBoxConfig = false;
	for (int i = 0; i < Archive.FileCount(); i++) {
		int ArcIndex = Archive.FindByIndex(i);
		QString FilePath = Archive.FileProperty(ArcIndex, "Path").toString();
		FilePath.replace("\\", "/");

		if (FilePath == "GlobalConfig.ini") {
			hasGlobalConfig = true;
			continue;
		}

		if (FilePath == "BoxConfig.ini") {
			hasSingleBoxConfig = true;
			continue;
		}

		int sep = FilePath.indexOf('/');
		if (sep > 0) {
			boxNamesSet.insert(FilePath.left(sep));
		}
	}
	Archive.Close();

	// 4. Detect format: root-level BoxConfig.ini means single-box archive
	//    Multi-box exports place configs under boxName/BoxConfig.ini, never at root
	if (hasSingleBoxConfig) {
		ImportSingleBox(parent, Path, Password, ImageSize);
		return QStringList();
	}

	QStringList boxNames = boxNamesSet.values();
	boxNames.sort(Qt::CaseInsensitive);

	if (boxNames.isEmpty() && !hasGlobalConfig) {
		QMessageBox::information(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("The archive does not contain any sandbox data."));
		return QStringList();
	}

	// 5. Show selection dialog (multi-box path)
	CBoxTransferDialog dlg(CBoxTransferDialog::eImport, parent);
	dlg.LoadArchiveContents(boxNames, hasGlobalConfig);
	if (theGUI->SafeExec(&dlg) != QDialog::Accepted)
		return QStringList();

	QMap<QString, QString> boxNameMapping = dlg.GetBoxNameMapping();
	bool importGlobalConfig = dlg.ExportGlobalConfig(); // same getter works for import

	if (boxNameMapping.isEmpty() && !importGlobalConfig) {
		QMessageBox::information(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("Nothing selected for import."));
		return QStringList();
	}

	// 6. Create boxes on main thread
	QMap<QString, QString> boxRoots;
	QList<SB_STATUS> Results;
	QStringList keysToRemove;

	for (auto I = boxNameMapping.constBegin(); I != boxNameMapping.constEnd(); ++I) {
		QString newName = I.value();

		// Check if box already exists
		CSandBoxPtr pExisting = theAPI->GetBoxByName(newName);
		if (!pExisting.isNull()) {
			int ret = QMessageBox::warning(parent, "Sandboxie-Plus",
				CBoxTransferDialog::tr("Sandbox '%1' already exists. Its configuration will be overwritten. Continue?").arg(newName),
				QMessageBox::Yes | QMessageBox::No);
			if (ret != QMessageBox::Yes) {
				keysToRemove.append(I.key());
				continue;
			}
			boxRoots.insert(newName, pExisting->GetFileRoot());
			continue;
		}

		SB_STATUS Status = theAPI->CreateBox(newName);
		if (Status.IsError()) {
			Results.append(Status);
			keysToRemove.append(I.key());
			continue;
		}

		CSandBoxPtr pBox = theAPI->GetBoxByName(newName);
		if (pBox)
			boxRoots.insert(newName, pBox->GetFileRoot());
	}

	for (const QString& key : keysToRemove)
		boxNameMapping.remove(key);

	if (!Results.isEmpty())
		theGUI->CheckResults(Results, parent);

	if (boxNameMapping.isEmpty() && !importGlobalConfig)
		return QStringList();

	// 7. Launch async import
	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
	QtConcurrent::run(ImportMultiBoxesAsync, pProgress, Path, Password, boxNameMapping, boxRoots, importGlobalConfig);
	theGUI->AddAsyncOp(pProgress, false, CBoxTransferDialog::tr("Importing: %1").arg(Path));

	return boxNameMapping.values();
}
