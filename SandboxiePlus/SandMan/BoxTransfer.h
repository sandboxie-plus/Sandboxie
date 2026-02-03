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

	// Results
	bool ExportGlobalConfig() const;
	QStringList GetSelectedBoxes() const;
	// For import: returns map of originalName -> newName (for renames)
	QMap<QString, QString> GetBoxNameMapping() const;

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
	QPushButton* m_pSelectAll;
	QPushButton* m_pSelectNone;
};

void ExportMultiBoxes(QWidget* parent, const QList<CSandBoxPtr>& SandBoxes = QList<CSandBoxPtr>());
QStringList ImportMultiBoxes(QWidget* parent);
