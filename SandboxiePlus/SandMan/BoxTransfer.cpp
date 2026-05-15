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
// QFileXProgress helper for archive operations with progress reporting
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
		m_pBoxTree->setHeaderLabels(QStringList() << tr("Archive Box Name") << tr("Import As") << tr("Source") << tr("Conflict"));
		m_pBoxTree->setColumnCount(4);
	}
	m_pBoxTree->header()->setStretchLastSection(true);
	m_pBoxTree->setRootIsDecorated(false);
	connect(m_pBoxTree, &QTreeWidget::itemChanged, this, &CBoxTransferDialog::OnItemChanged);
	pLayout->addWidget(m_pBoxTree);

	pLayout->addWidget(new CFinder(this, this, 0));

	m_pGlobalConfig = new QCheckBox(mode == eExport ? tr("Export Global Configuration") : tr("Import Global Configuration"), this);
	pLayout->addWidget(m_pGlobalConfig);

	m_pSeparateFiles = nullptr;
	if (mode == eExport) {
		m_pSeparateFiles = new QCheckBox(tr("Export each box to its own file"), this);
		pLayout->addWidget(m_pSeparateFiles);
	}

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

void CBoxTransferDialog::LoadMultiArchiveContents(const QList<SImportBoxInfo>& boxes, bool hasGlobalConfig)
{
	m_pGlobalConfig->setEnabled(hasGlobalConfig);
	if (hasGlobalConfig)
		m_pGlobalConfig->setChecked(true);
	else
		m_pGlobalConfig->setChecked(false);

	for (const SImportBoxInfo& box : boxes) {
		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
		pItem->setCheckState(0, Qt::Checked);
		pItem->setText(0, box.ArchiveName);
		pItem->setText(1, box.ArchiveName); // default "Import As" = same name
		pItem->setData(0, Qt::UserRole, box.ArchiveName); // original archive name
		pItem->setData(0, Qt::UserRole + 1, box.SourceFile); // source file path
		pItem->setData(0, Qt::UserRole + 2, box.Password); // password for this archive

		// Show source file name (not full path)
		QString sourceFileName = Split2(box.SourceFile, "/", true).second;
		if (sourceFileName.isEmpty())
			sourceFileName = Split2(box.SourceFile, "\\", true).second;
		pItem->setText(2, sourceFileName);

		// Try to set icon if a box with this name already exists
		CSandBoxPtr pExisting = theAPI->GetBoxByName(box.ArchiveName);
		if (!pExisting.isNull()) {
			auto pBoxEx = pExisting.objectCast<CSandBoxPlus>();
			if (pBoxEx)
				pItem->setIcon(0, theGUI->GetBoxIcon(pBoxEx->GetType(), false));
		}

		m_pBoxTree->addTopLevelItem(pItem);
	}
	m_pBoxTree->resizeColumnToContents(0);
	m_pBoxTree->resizeColumnToContents(2);

	UpdateConflictIndicators();
}

void CBoxTransferDialog::UpdateConflictIndicators()
{
	if (m_Mode != eImport) return;

	// First pass: collect all import names that are checked to detect duplicates
	QMap<QString, int> importNameCounts;
	for (int i = 0; i < m_pBoxTree->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = m_pBoxTree->topLevelItem(i);
		if (pItem->checkState(0) == Qt::Checked) {
			QString importName = pItem->text(1);
			importNameCounts[importName]++;
		}
	}

	// Second pass: set conflict indicators
	for (int i = 0; i < m_pBoxTree->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = m_pBoxTree->topLevelItem(i);
		QString importName = pItem->text(1);

		QStringList conflicts;

		// Check if box already exists on the system
		CSandBoxPtr pExisting = theAPI->GetBoxByName(importName);
		if (!pExisting.isNull())
			conflicts.append(tr("Exists!"));

		// Check if there are duplicate import names among checked items
		if (pItem->checkState(0) == Qt::Checked && importNameCounts.value(importName) > 1)
			conflicts.append(tr("Duplicate!"));

		pItem->setText(3, conflicts.join(" "));
	}
}

bool CBoxTransferDialog::ExportGlobalConfig() const
{
	return m_pGlobalConfig->isChecked();
}

bool CBoxTransferDialog::ExportSeparateFiles() const
{
	return m_pSeparateFiles && m_pSeparateFiles->isChecked();
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

QList<CBoxTransferDialog::SImportEntry> CBoxTransferDialog::GetImportEntries() const
{
	QList<SImportEntry> entries;
	for (int i = 0; i < m_pBoxTree->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = m_pBoxTree->topLevelItem(i);
		if (pItem->checkState(0) == Qt::Checked) {
			SImportEntry entry;
			entry.ArchiveName = pItem->data(0, Qt::UserRole).toString();
			entry.ImportName = pItem->text(1);
			entry.SourceFile = pItem->data(0, Qt::UserRole + 1).toString();
			entry.Password = pItem->data(0, Qt::UserRole + 2).toString();
			entries.append(entry);
		}
	}
	return entries;
}

void CBoxTransferDialog::OnSelectAll()
{
	for (int i = 0; i < m_pBoxTree->topLevelItemCount(); i++)
		m_pBoxTree->topLevelItem(i)->setCheckState(0, Qt::Checked);
	UpdateConflictIndicators();
}

void CBoxTransferDialog::OnSelectNone()
{
	for (int i = 0; i < m_pBoxTree->topLevelItemCount(); i++)
		m_pBoxTree->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
	UpdateConflictIndicators();
}

void CBoxTransferDialog::OnItemChanged(QTreeWidgetItem* item, int column)
{
	if (m_Mode == eImport && (column == 0 || column == 1))
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

		// Write box config as BoxName.ini with [BoxName] section header
		QTemporaryFile* pConfigFile = new QTemporaryFile();
		pConfigFile->open();
		pConfigFile->write(("[" + boxName + "]\n").toUtf8());
		pConfigFile->write(configSection.toUtf8());
		pConfigFile->close();

		int ConfigIndex = Archive.AddFile(boxName + ".ini");
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

void ExportMultiBoxes(QWidget* parent, const QList<CSandBoxPtr>& SandBoxes)
{
	if (!CArchive::IsInit()) {
		QMessageBox::critical(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("7-Zip library is not available."));
		return;
	}

	QStringList selectedBoxes;
	bool exportGlobalConfig = false;
	bool exportSeparateFiles = false;
	bool mustEncrypt = false;

	// For single box, skip selection dialog
	if (SandBoxes.size() == 1) {
		auto pBoxEx = SandBoxes.first().objectCast<CSandBoxPlus>();
		if (!pBoxEx) return;

		if (theAPI->HasProcesses(pBoxEx->GetName())) {
			QMessageBox::warning(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("Cannot export: Sandbox has running processes."));
			return;
		}

		if (!pBoxEx->IsInitialized()) {
			QMessageBox::warning(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("Cannot export: Sandbox is empty."));
			return;
		}

		selectedBoxes.append(pBoxEx->GetName());
		mustEncrypt = pBoxEx->UseImageFile();
	}
	else {
		// 1. Show selection dialog for multiple boxes
		CBoxTransferDialog dlg(CBoxTransferDialog::eExport, parent);
		dlg.PopulateExportList(SandBoxes);
		if (theGUI->SafeExec(&dlg) != QDialog::Accepted)
			return;

		selectedBoxes = dlg.GetSelectedBoxes();
		exportGlobalConfig = dlg.ExportGlobalConfig();
		exportSeparateFiles = dlg.ExportSeparateFiles();

		if (selectedBoxes.isEmpty() && !exportGlobalConfig) {
			QMessageBox::information(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("Nothing selected for export."));
			return;
		}

		if (exportSeparateFiles && selectedBoxes.isEmpty()) {
			QMessageBox::information(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("No boxes selected for separate file export."));
			return;
		}

		// Check if any selected box requires encryption
		for (const QString& boxName : selectedBoxes) {
			CSandBoxPtr pBox = theAPI->GetBoxByName(boxName);
			if (pBox) {
				auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
				if (pBoxEx && pBoxEx->UseImageFile()) {
					mustEncrypt = true;
					break;
				}
			}
		}
	}

	// 2. Compression options
	CCompressDialog compDlg(parent);
	if (mustEncrypt)
		compDlg.SetMustEncrypt();
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

	// 4. Pre-collect data on main thread
	QStringList validBoxes;
	QMap<QString, QString> boxRoots;
	QMap<QString, QString> boxConfigs;

	for (const QString& boxName : selectedBoxes) {
		// Check running processes (skip for single box - already checked above)
		if (SandBoxes.size() > 1 && theAPI->HasProcesses(boxName)) {
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

	// 5. File/folder dialog and export
	QVariantMap vParams;
	if (!Password.isEmpty())
		vParams["password"] = Password;
	vParams["level"] = compDlg.GetLevel();
	vParams["solid"] = compDlg.MakeSolid();

	if (exportSeparateFiles) {
		// Folder picker for separate files export
		QString FolderPath = QFileDialog::getExistingDirectory(parent, CBoxTransferDialog::tr("Select Export Folder"));
		if (FolderPath.isEmpty())
			return;

		// Export each box to its own file
		for (const QString& boxName : validBoxes) {
			QString boxPath = FolderPath + "/" + boxName + ".7z";

			QStringList singleBox;
			singleBox.append(boxName);

			QMap<QString, QString> singleBoxRoots;
			singleBoxRoots.insert(boxName, boxRoots.value(boxName));

			QMap<QString, QString> singleBoxConfigs;
			singleBoxConfigs.insert(boxName, boxConfigs.value(boxName));

			CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
			QtConcurrent::run(ExportMultiBoxesAsync, pProgress, boxPath, singleBox, singleBoxRoots, singleBoxConfigs, false, QString(), vParams);
			theGUI->AddAsyncOp(pProgress, false, CBoxTransferDialog::tr("Exporting: %1").arg(boxPath));
		}

		// Export global config separately if requested
		if (exportGlobalConfig && !globalConfig.isEmpty()) {
			QString globalPath = FolderPath + "/GlobalConfig.7z";
			QStringList emptyBoxes;

			CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
			QtConcurrent::run(ExportMultiBoxesAsync, pProgress, globalPath, emptyBoxes, QMap<QString, QString>(), QMap<QString, QString>(), true, globalConfig, vParams);
			theGUI->AddAsyncOp(pProgress, false, CBoxTransferDialog::tr("Exporting: %1").arg(globalPath));
		}
	} else {
		// Single archive export
		QString defaultName = (validBoxes.size() == 1) ? validBoxes.first() : "SandboxExport";
		QString Path = QFileDialog::getSaveFileName(parent, CBoxTransferDialog::tr("Export Sandbox"),
			defaultName + compDlg.GetFormat(),
			CBoxTransferDialog::tr("7-Zip Archive (*.7z);;Zip Archive (*.zip)"));
		if (Path.isEmpty())
			return;

		CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
		QtConcurrent::run(ExportMultiBoxesAsync, pProgress, Path, validBoxes, boxRoots, boxConfigs, exportGlobalConfig, globalConfig, vParams);
		theGUI->AddAsyncOp(pProgress, false, CBoxTransferDialog::tr("Exporting: %1").arg(Path));
	}
}


///////////////////////////////////////////////////////////////////////////////
// Import
//

// Helper function to extract section content from INI data
// Returns the content of the specified section without the [section] header
// If no section header is found, returns the entire content (for old format compatibility)
static QString ExtractIniSection(const QString& iniContent, const QString& sectionName)
{
	QString content = iniContent;

	// Look for the section header [sectionName]
	QRegularExpression sectionRegex("^\\s*\\[" + QRegularExpression::escape(sectionName) + "\\]\\s*$",
		QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch match = sectionRegex.match(content);

	if (!match.hasMatch()) {
		// No section header found - check if content starts with any section header
		QRegularExpression anySectionRegex("^\\s*\\[.*\\]\\s*$", QRegularExpression::MultilineOption);
		if (!anySectionRegex.match(content).hasMatch()) {
			// No section headers at all - return content as-is (old format)
			return content;
		}
		// Has section headers but not the one we want - return empty
		return QString();
	}

	// Found the section - extract content after it until next section or end
	int sectionStart = match.capturedEnd();

	// Find the next section header (if any)
	QRegularExpression nextSectionRegex("^\\s*\\[.*\\]\\s*$", QRegularExpression::MultilineOption);
	QRegularExpressionMatch nextMatch = nextSectionRegex.match(content, sectionStart);

	int sectionEnd = nextMatch.hasMatch() ? nextMatch.capturedStart() : content.length();

	QString sectionContent = content.mid(sectionStart, sectionEnd - sectionStart).trimmed();

	// Add trailing newline if content is not empty
	if (!sectionContent.isEmpty() && !sectionContent.endsWith('\n'))
		sectionContent += '\n';

	return sectionContent;
}

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
		QString ArchiveBoxName;  // Original box name in archive (for section extraction)
		QString TargetBoxName;   // Target box name for import
		QString TempPath;
	};
	QList<SConfigEntry> configEntries;
	int globalConfigIndex = -1;
	QString globalConfigTempPath;

	// First pass: detect if this is a single-box archive (has root-level BoxConfig.ini)
	bool isSingleBoxArchive = false;
	for (int i = 0; i < Archive.FileCount(); i++) {
		int ArcIndex = Archive.FindByIndex(i);
		QString FilePath = Archive.FileProperty(ArcIndex, "Path").toString();
		FilePath.replace("\\", "/");
		if (FilePath == "BoxConfig.ini") {
			isSingleBoxArchive = true;
			break;
		}
	}

	// For single-box archives, get the target box name from the mapping
	QString singleBoxArchiveName;
	QString singleBoxTargetName;
	QString singleBoxRoot;
	if (isSingleBoxArchive && boxNameMapping.size() == 1) {
		singleBoxArchiveName = boxNameMapping.firstKey();
		singleBoxTargetName = boxNameMapping.first();
		singleBoxRoot = boxRoots.value(singleBoxTargetName);
	}

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

		if (isSingleBoxArchive && !singleBoxRoot.isEmpty()) {
			// Single-box archive: files are at root level
			if (FilePath == "BoxConfig.ini") {
				// Extract config to temp, apply later (old format - no section header)
				QString TempPath = QDir::tempPath() + "/SbiePlus_BoxConfig_" + singleBoxTargetName + "_" + QString::number(QCoreApplication::applicationPid()) + ".ini";
				SConfigEntry entry;
				entry.ArcIndex = ArcIndex;
				entry.ArchiveBoxName = singleBoxArchiveName;
				entry.TargetBoxName = singleBoxTargetName;
				entry.TempPath = TempPath;
				configEntries.append(entry);
				Files.insert(ArcIndex, new QFileXProgress(TempPath, pProgress, &Archive));
			} else {
				// Regular file - extract to box root
				Files.insert(ArcIndex, new QFileXProgress(CArchive::PrepareExtraction(FilePath, singleBoxRoot + "\\"), pProgress, &Archive));
			}
		} else {
			// Multi-box archive: check for new format (BoxName.ini) or old format (BoxName/...)
			int sep = FilePath.indexOf('/');

			// Check for new format: BoxName.ini at root level
			if (sep <= 0 && FilePath.endsWith(".ini", Qt::CaseInsensitive)) {
				QString archiveBoxName = FilePath.left(FilePath.length() - 4); // Remove .ini
				if (!boxNameMapping.contains(archiveBoxName))
					continue;

				QString newBoxName = boxNameMapping.value(archiveBoxName);
				QString TempPath = QDir::tempPath() + "/SbiePlus_BoxConfig_" + newBoxName + "_" + QString::number(QCoreApplication::applicationPid()) + ".ini";
				SConfigEntry entry;
				entry.ArcIndex = ArcIndex;
				entry.ArchiveBoxName = archiveBoxName;
				entry.TargetBoxName = newBoxName;
				entry.TempPath = TempPath;
				configEntries.append(entry);
				Files.insert(ArcIndex, new QFileXProgress(TempPath, pProgress, &Archive));
				continue;
			}

			if (sep <= 0) continue;

			QString archiveBoxName = FilePath.left(sep);
			QString relPath = FilePath.mid(sep + 1);

			if (!boxNameMapping.contains(archiveBoxName))
				continue;

			QString newBoxName = boxNameMapping.value(archiveBoxName);
			QString boxRoot = boxRoots.value(newBoxName);
			if (boxRoot.isEmpty()) continue;

			if (relPath == "BoxConfig.ini") {
				// Old format: Extract config to temp, apply later
				QString TempPath = QDir::tempPath() + "/SbiePlus_BoxConfig_" + newBoxName + "_" + QString::number(QCoreApplication::applicationPid()) + ".ini";
				SConfigEntry entry;
				entry.ArcIndex = ArcIndex;
				entry.ArchiveBoxName = archiveBoxName;
				entry.TargetBoxName = newBoxName;
				entry.TempPath = TempPath;
				configEntries.append(entry);
				Files.insert(ArcIndex, new QFileXProgress(TempPath, pProgress, &Archive));
			} else {
				// Regular file - extract to box root
				Files.insert(ArcIndex, new QFileXProgress(CArchive::PrepareExtraction(relPath, boxRoot + "\\"), pProgress, &Archive));
			}
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
				QString rawContent = QString::fromUtf8(File.readAll());
				File.close();

				// Extract section content - handles both old format (no section header)
				// and new format (with [BoxName] section header)
				QString configContent = ExtractIniSection(rawContent, entry.ArchiveBoxName);

				// Remove FileRootPath= entries as the path should be determined by the new box location
				configContent.remove(QRegularExpression("(?m)^FileRootPath=.*$\\n?"));

				QMetaObject::invokeMethod(theAPI, "SbieIniSetSection", Qt::BlockingQueuedConnection,
					Q_ARG(QString, entry.TargetBoxName),
					Q_ARG(QString, configContent)
				);
			}
			File.remove();
		}

		// Update imported boxes on main thread
		for (const QString& newBoxName : boxNameMapping.values()) {
			QMetaObject::invokeMethod(theGUI, [newBoxName]() {
				CSandBoxPtr pBox = theAPI->GetBoxByName(newBoxName);
				if (pBox) {
					auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
					if (pBoxEx) {
						pBoxEx->UpdateSize();
						if (theConf->GetBool("Options/ScanStartMenu", true))
							pBoxEx->ScanStartMenu();
					}
				}
			}, Qt::QueuedConnection);
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

// Helper structure for scanning archive contents
struct SArchiveInfo {
	QString Path;
	QString Password;
	QStringList BoxNames;
	bool HasGlobalConfig;
	bool IsSingleBox;
};

static bool ScanArchive(QWidget* parent, const QString& path, SArchiveInfo& info)
{
	info.Path = path;
	info.Password.clear();
	info.BoxNames.clear();
	info.HasGlobalConfig = false;
	info.IsSingleBox = false;

	CArchive Archive(path);
	int Ret = Archive.Open();

	if (Ret == ERR_7Z_PASSWORD_REQUIRED) {
		for (;;) {
			CBoxImageWindow window(CBoxImageWindow::eImport, parent);
			window.setWindowTitle(CBoxTransferDialog::tr("Password for %1").arg(Split2(path, "/", true).second));
			if (theGUI->SafeExec(&window) != QDialog::Accepted)
				return false;
			Archive.SetPassword(window.GetPassword());
			Ret = Archive.Open();
			if (Ret != ERR_7Z_OK) {
				QMessageBox::critical(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("Failed to open archive, wrong password?"));
				continue;
			}
			info.Password = window.GetPassword();
			break;
		}
	}

	if (Ret != ERR_7Z_OK) {
		QMessageBox::critical(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("Failed to open archive (%1): %2").arg(path).arg(Ret));
		return false;
	}

	QSet<QString> boxNamesSet;
	bool hasSingleBoxConfig = false;

	for (int i = 0; i < Archive.FileCount(); i++) {
		int ArcIndex = Archive.FindByIndex(i);
		QString FilePath = Archive.FileProperty(ArcIndex, "Path").toString();
		FilePath.replace("\\", "/");

		if (FilePath == "GlobalConfig.ini") {
			info.HasGlobalConfig = true;
			continue;
		}

		if (FilePath == "BoxConfig.ini") {
			hasSingleBoxConfig = true;
			continue;
		}

		int sep = FilePath.indexOf('/');
		if (sep > 0) {
			// Old format: BoxName/... - extract box name from folder
			boxNamesSet.insert(FilePath.left(sep));
		} else if (FilePath.endsWith(".ini", Qt::CaseInsensitive) && FilePath != "GlobalConfig.ini") {
			// New format: BoxName.ini at root level - extract box name from filename
			QString boxName = FilePath.left(FilePath.length() - 4);
			boxNamesSet.insert(boxName);
		}
	}
	Archive.Close();

	if (hasSingleBoxConfig) {
		// Single-box archive (legacy format): derive box name from archive file name
		info.IsSingleBox = true;
		StrPair PathName = Split2(path, "/", true);
		if (PathName.second.isEmpty())
			PathName = Split2(path, "\\", true);
		StrPair NameEx = Split2(PathName.second, ".", true);
		info.BoxNames.append(NameEx.first);
	} else {
		info.BoxNames = boxNamesSet.values();
		info.BoxNames.sort(Qt::CaseInsensitive);
	}

	return true;
}

QStringList ImportMultiBoxes(QWidget* parent)
{
	if (!CArchive::IsInit()) {
		QMessageBox::critical(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("7-Zip library is not available."));
		return QStringList();
	}

	// 1. Pick archives (multiple selection)
	QStringList Paths = QFileDialog::getOpenFileNames(parent, CBoxTransferDialog::tr("Select Archives"),
		"", CBoxTransferDialog::tr("7-Zip Archive (*.7z);;Zip Archive (*.zip);;All Archives (*.7z *.zip)"));
	if (Paths.isEmpty())
		return QStringList();

	// 2. Scan all archives and collect box info
	QList<SArchiveInfo> archiveInfos;
	QList<CBoxTransferDialog::SImportBoxInfo> allBoxes;
	bool hasAnyGlobalConfig = false;
	QString globalConfigSourceFile;
	QString globalConfigPassword;

	for (const QString& path : Paths) {
		SArchiveInfo info;
		if (!ScanArchive(parent, path, info))
			continue; // User cancelled password or archive failed to open

		archiveInfos.append(info);

		// Track global config (only use from first archive that has it)
		if (info.HasGlobalConfig && !hasAnyGlobalConfig) {
			hasAnyGlobalConfig = true;
			globalConfigSourceFile = info.Path;
			globalConfigPassword = info.Password;
		}

		// Add boxes to the list
		for (const QString& boxName : info.BoxNames) {
			CBoxTransferDialog::SImportBoxInfo boxInfo;
			boxInfo.ArchiveName = boxName;
			boxInfo.SourceFile = info.Path;
			boxInfo.Password = info.Password;
			allBoxes.append(boxInfo);
		}
	}

	if (allBoxes.isEmpty() && !hasAnyGlobalConfig) {
		QMessageBox::information(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("The selected archives do not contain any sandbox data."));
		return QStringList();
	}

	// 3. Show selection dialog with all boxes from all files
	CBoxTransferDialog dlg(CBoxTransferDialog::eImport, parent);
	dlg.LoadMultiArchiveContents(allBoxes, hasAnyGlobalConfig);
	if (theGUI->SafeExec(&dlg) != QDialog::Accepted)
		return QStringList();

	QList<CBoxTransferDialog::SImportEntry> importEntries = dlg.GetImportEntries();
	bool importGlobalConfig = dlg.ExportGlobalConfig();

	if (importEntries.isEmpty() && !importGlobalConfig) {
		QMessageBox::information(parent, "Sandboxie-Plus", CBoxTransferDialog::tr("Nothing selected for import."));
		return QStringList();
	}

	// 4. Check for duplicate import names
	QMap<QString, int> importNameCounts;
	for (const auto& entry : importEntries)
		importNameCounts[entry.ImportName]++;

	for (auto it = importNameCounts.constBegin(); it != importNameCounts.constEnd(); ++it) {
		if (it.value() > 1) {
			QMessageBox::warning(parent, "Sandboxie-Plus",
				CBoxTransferDialog::tr("Cannot import multiple boxes with the same name '%1'. Please rename them first.").arg(it.key()));
			return QStringList();
		}
	}

	// 5. Create boxes on main thread
	QMap<QString, QString> boxRoots;
	QList<SB_STATUS> Results;
	QList<CBoxTransferDialog::SImportEntry> validEntries;

	for (const auto& entry : importEntries) {
		// Check if box already exists
		CSandBoxPtr pExisting = theAPI->GetBoxByName(entry.ImportName);
		if (!pExisting.isNull()) {
			int ret = QMessageBox::warning(parent, "Sandboxie-Plus",
				CBoxTransferDialog::tr("Sandbox '%1' already exists. Its configuration will be overwritten. Continue?").arg(entry.ImportName),
				QMessageBox::Yes | QMessageBox::No);
			if (ret != QMessageBox::Yes)
				continue;
			boxRoots.insert(entry.ImportName, pExisting->GetFileRoot());
			validEntries.append(entry);
			continue;
		}

		SB_STATUS Status = theAPI->CreateBox(entry.ImportName);
		if (Status.IsError()) {
			Results.append(Status);
			continue;
		}

		CSandBoxPtr pBox = theAPI->GetBoxByName(entry.ImportName);
		if (pBox)
			boxRoots.insert(entry.ImportName, pBox->GetFileRoot());
		validEntries.append(entry);
	}

	if (!Results.isEmpty())
		theGUI->CheckResults(Results, parent);

	if (validEntries.isEmpty() && !importGlobalConfig)
		return QStringList();

	// 6. Group entries by source file and launch async imports
	QMap<QString, QList<CBoxTransferDialog::SImportEntry>> entriesByFile;
	for (const auto& entry : validEntries)
		entriesByFile[entry.SourceFile].append(entry);

	QStringList importedNames;
	for (auto it = entriesByFile.constBegin(); it != entriesByFile.constEnd(); ++it) {
		QString sourceFile = it.key();
		const QList<CBoxTransferDialog::SImportEntry>& entries = it.value();

		// Build mapping for this file
		QMap<QString, QString> boxNameMapping;
		QString password;
		for (const auto& entry : entries) {
			boxNameMapping.insert(entry.ArchiveName, entry.ImportName);
			password = entry.Password; // Same password for all entries from same file
			importedNames.append(entry.ImportName);
		}

		// Check if this file contains global config and should import it
		bool importGlobalForThisFile = importGlobalConfig && (sourceFile == globalConfigSourceFile);

		CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
		QtConcurrent::run(ImportMultiBoxesAsync, pProgress, sourceFile, password, boxNameMapping, boxRoots, importGlobalForThisFile);
		theGUI->AddAsyncOp(pProgress, false, CBoxTransferDialog::tr("Importing: %1").arg(sourceFile));
	}

	// Handle global config from a file with no boxes selected
	if (importGlobalConfig && !entriesByFile.contains(globalConfigSourceFile)) {
		QMap<QString, QString> emptyMapping;
		CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());
		QtConcurrent::run(ImportMultiBoxesAsync, pProgress, globalConfigSourceFile, globalConfigPassword, emptyMapping, QMap<QString, QString>(), true);
		theGUI->AddAsyncOp(pProgress, false, CBoxTransferDialog::tr("Importing: %1").arg(globalConfigSourceFile));
	}

	return importedNames;
}
