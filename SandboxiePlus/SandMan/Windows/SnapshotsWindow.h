#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SnapshotsWindow.h"
#include "SbiePlusAPI.h"
class CSimpleTreeModel;

class CSnapshotsWindow : public QMainWindow
{
	Q_OBJECT

public:
	CSnapshotsWindow(const CSandBoxPtr& pBox, QWidget *parent = Q_NULLPTR);
	~CSnapshotsWindow();

private slots:
	void UpdateSnapshots();
	void UpdateSnapshot(const QModelIndex& Index);

	void SaveInfo();

	void OnTakeSnapshot();
	void OnSelectSnapshot();
	void OnRemoveSnapshot();

	void OnSaveInfo();

protected:
	void closeEvent(QCloseEvent *e);

	void HandleResult(SB_PROGRESS Status);

	CSandBoxPtr m_pBox;
	QString						m_CurSnapshot;
	QMap<QVariant, QVariantMap> m_SnapshotMap;

	QVariant	m_SellectedID;
	int			m_SaveInfoPending;

private:
	Ui::SnapshotsWindow ui;
	CSimpleTreeModel* m_pSnapshotModel;
};
