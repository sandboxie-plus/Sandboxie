#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SnapshotsWindow.h"
#include "SbiePlusAPI.h"
class CSimpleTreeModel;

class CSnapshotsWindow : public QDialog
{
	Q_OBJECT

public:
	CSnapshotsWindow(const CSandBoxPtr& pBox, QWidget *parent = Q_NULLPTR);
	~CSnapshotsWindow();

	virtual void accept() {}
	virtual void reject() { this->close(); }

signals:
	void Closed();

private slots:
	void UpdateSnapshots(bool AndSelect = false);
	void UpdateSnapshot(const QModelIndex& Index);

	void SaveInfo();

	void OnTakeSnapshot();
	void OnSelectSnapshot();
	void OnSelectEmpty();
	void OnChangeDefault();
	void OnRemoveSnapshot();

	void OnSaveInfo();

protected:
	void closeEvent(QCloseEvent *e);

	void SelectSnapshot(const QString& ID);
	void HandleResult(SB_PROGRESS Status);

	CSandBoxPtr m_pBox;
	QString						m_CurSnapshot;
	QString						m_DefaultSnapshot;
	QMap<QVariant, QVariantMap> m_SnapshotMap;

	QVariant	m_SellectedID;
	int			m_SaveInfoPending;

private:
	Ui::SnapshotsWindow ui;
	CSimpleTreeModel* m_pSnapshotModel;
};
