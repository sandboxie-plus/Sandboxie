#pragma once


#include "../../MiscHelpers/Common/PanelView.h"
#include "../../MiscHelpers/Common/TreeviewEx.h"
#include "../Models/NtObjectModel.h"

class CNtObjectView : public CPanelWidgetTmpl<QTreeViewEx>
{
	Q_OBJECT

public:
	CNtObjectView(QWidget *parent = Q_NULLPTR);
	~CNtObjectView();

	virtual void				SaveState();

	void SetBox(const CSandBoxPtr& pBox);
	const CSandBoxPtr& GetBox() const { return m_pBox; }

private slots:
	void				OnFileMenu(const QPoint &);
	void				OnFileDblClick(const QModelIndex &);

protected:
	CSandBoxPtr m_pBox;

private:
	CNtObjectModel*		m_pNtObjectModel;
	CSortFilterProxyModel* m_pSortProxy;
};


////////////////////////////////////////////////////////////////////////////////////////
// CNtObjectBrowserWindow

class CNtObjectBrowserWindow : public QDialog
{
	Q_OBJECT

public:
	CNtObjectBrowserWindow(const CSandBoxPtr& pBox, QWidget *parent = Q_NULLPTR);
	~CNtObjectBrowserWindow();

	virtual void accept() {}
	virtual void reject() { this->close(); }

signals:
	void Closed();

protected:
	void closeEvent(QCloseEvent *e);

private:

	QGridLayout* m_pMainLayout;
	CNtObjectView* m_FileView;
};
