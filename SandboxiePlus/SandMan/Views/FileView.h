#pragma once


//#include "../../MiscHelpers/Common/PanelView.h"
#include "../../MiscHelpers/Common/TreeviewEx.h"
#include "SbiePlusAPI.h"
#include <QFileSystemModel>

class CFileView : public QWidget
{
	Q_OBJECT

public:
	CFileView(QWidget *parent = Q_NULLPTR);
	~CFileView();

	virtual void				SaveState();

	void SetBox(const CSandBoxPtr& pBox);
	const CSandBoxPtr& GetBox() const { return m_pBox; }
	
	virtual QTreeView*		GetTree() { return m_pTreeView; }

private slots:
	void				OnFileMenu(const QPoint &);
	void				OnFileDblClick(const QModelIndex &);

	void				OnAboutToBeCleaned();

protected:
	CSandBoxPtr m_pBox;

private:
	QGridLayout*		m_pMainLayout;
	QTreeView*			m_pTreeView;
	QFileSystemModel*	m_pFileModel;
};


////////////////////////////////////////////////////////////////////////////////////////
// CFileBrowserWindow

class CFileBrowserWindow : public QDialog
{
	Q_OBJECT

public:
	CFileBrowserWindow(const CSandBoxPtr& pBox, QWidget *parent = Q_NULLPTR);
	~CFileBrowserWindow();

	virtual void accept() {}
	virtual void reject() { this->close(); }

signals:
	void Closed();

protected:
	void closeEvent(QCloseEvent *e);

private:

	QGridLayout* m_pMainLayout;
	CFileView* m_FileView;
};
