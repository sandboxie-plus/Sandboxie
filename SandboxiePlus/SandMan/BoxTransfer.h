#pragma once

#include <QDialog>
#include <QTreeWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QRegularExpression>
#include "SbiePlusAPI.h"

class CBoxTransferDialog : public QDialog
{
	Q_OBJECT
public:
	enum EMode { eExport, eImport };

	CBoxTransferDialog(EMode mode, QWidget* parent = nullptr);

	void PopulateExportList(const QList<CSandBoxPtr>& SandBoxes = QList<CSandBoxPtr>());

	// For eImport mode: populate from archive contents
	void LoadArchiveContents(const QStringList& boxNames, bool hasGlobalConfig);

	// Import box info structure for multi-file import
	struct SImportBoxInfo {
		QString ArchiveName;    // Original name in archive
		QString SourceFile;     // Source archive file path
		QString Password;       // Password for this archive (if encrypted)
	};
	void LoadMultiArchiveContents(const QList<SImportBoxInfo>& boxes, bool hasGlobalConfig);

	// Results
	bool ExportGlobalConfig() const;
	bool ExportSeparateFiles() const;
	QStringList GetSelectedBoxes() const;
	// For import: returns map of originalName -> newName (for renames)
	QMap<QString, QString> GetBoxNameMapping() const;

	// For multi-file import: returns list of selected boxes with full info
	struct SImportEntry {
		QString ArchiveName;    // Original name in archive
		QString ImportName;     // Name to import as
		QString SourceFile;     // Source archive file path
		QString Password;       // Password for this archive
	};
	QList<SImportEntry> GetImportEntries() const;

public slots:
	void SetFilter(const QRegularExpression& Exp, int iOptions = 0, int Column = -1);

private slots:
	void OnSelectAll();
	void OnSelectNone();
	void OnItemChanged(QTreeWidgetItem* item, int column);

private:
	void UpdateConflictIndicators();

	EMode m_Mode;
	QTreeWidget* m_pBoxTree;
	QCheckBox* m_pGlobalConfig;
	QCheckBox* m_pSeparateFiles;
	QPushButton* m_pSelectAll;
	QPushButton* m_pSelectNone;
};

void ExportMultiBoxes(QWidget* parent, const QList<CSandBoxPtr>& SandBoxes = QList<CSandBoxPtr>());
QStringList ImportMultiBoxes(QWidget* parent);
