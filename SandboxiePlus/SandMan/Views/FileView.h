#pragma once


//#include "../../MiscHelpers/Common/PanelView.h"
#include "../../MiscHelpers/Common/TreeviewEx.h"
#include "../../MiscHelpers/Common/Finder.h"
#include "SbiePlusAPI.h"
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QThread>

////////////////////////////////////////////////////////////////////////////////////////
// CFileSearchThread

class CFileSearchThread : public QThread
{
	Q_OBJECT

public:
	CFileSearchThread(const QString& rootPath, const QRegularExpression& pattern, QObject* parent = nullptr);

	void cancel() { m_bCancelled = true; }

protected:
	void run() override;

private:
	void searchDirectory(const QString& dirPath, double baseProgress, double availableProgress);

signals:
	void pathFound(const QString& path);
	void progressUpdate(int current, int total);

private:
	QString m_RootPath;
	QRegularExpression m_SearchPattern;
	bool m_bCancelled = false;
};

////////////////////////////////////////////////////////////////////////////////////////
// CFileFilterProxyModel

class CFileFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	CFileFilterProxyModel(QObject* parent = nullptr);

	void SetSearchPattern(const QRegularExpression& pattern);
	QRegularExpression GetSearchPattern() const { return m_SearchPattern; }
	void ClearFilter();
	void ClearPathFilter() { m_PathFilter.clear(); }
	void SetRootPath(const QString& path);
	bool IsFilterActive() const { return m_bFilterActive; }

public slots:
	void AddPathFilter(const QString& path);

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
	QRegularExpression m_SearchPattern;
	bool m_bFilterActive;
	QString m_RootPath;

	QSet<QString> m_PathFilter;
	bool m_bUpdatePending = false;
};

////////////////////////////////////////////////////////////////////////////////////////
// CFileView

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

	void				OnAboutToBeModified();
	void				OnSetFilter(const QRegularExpression& Exp, int iOptions, int Column);

	void				DoSearch();

protected:
	CSandBoxPtr			m_pBox;

private:
	QGridLayout*		m_pMainLayout;
	QTreeViewEx*		m_pTreeView;
	QFileSystemModel*	m_pFileModel;
	CFileFilterProxyModel* m_pProxyModel;
	CFinder*			m_pFinder;
	QString				m_RootPath;
	bool				m_bSearchPending = false;
	CFileSearchThread*	m_pSearchThread = nullptr;
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
