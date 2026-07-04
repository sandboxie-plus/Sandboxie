#pragma once

#include <QtWidgets/QMainWindow>
#include <QFileIconProvider>
#include "ui_RecoveryWindow.h"
#include "SbiePlusAPI.h"
class CSimpleTreeModel;

class CRecoveryCounter : public QThread
{
	Q_OBJECT
public:
	CRecoveryCounter(const QString& BoxPath, QWidget* parent = Q_NULLPTR) : QThread(parent) {
		m_BoxPath = BoxPath;
		m_run = true;
		start(QThread::LowPriority);
	}
	~CRecoveryCounter() {
		m_run = false;
		wait(2000);
		terminate();
	}

signals:
	void		Count(quint32 fileCount, quint32 folderCount, quint64 totalSize);

protected:
	void		run();

	QString		m_BoxPath;
	bool		m_run;
};

class CRecoveryWindow : public QDialog
{
	Q_OBJECT

public:
	CRecoveryWindow(const CSandBoxPtr& pBox, bool bImmediate = false, QWidget *parent = Q_NULLPTR);
	~CRecoveryWindow();

	bool		IsDeleteDialog() const;
	bool		IsDeleteSnapshots() { return m_DeleteSnapshots; }
	int			GetUnfilteredFileCount() const { return m_UnfilteredFileCount; }

	static bool	IsFileIgnored(const CSandBoxPtr& pBox, const QString& diskPath, const QString& boxedPath);

	virtual void accept() {}
	virtual void reject() { this->close(); }

signals:
	void		Closed();

public slots:
	int			exec();

	int			FindFiles();
	void		SelectFiles();
	void		AddFile(const QString& FilePath, const QString& BoxPath);

private slots:
	void		OnAddFolder();
	void		OnShowAllChanged(int state);
	void		OnRecover();
	void		OnDelete();
	void		OnTargetChanged();
	void		OnDeleteAll();
	void		OnDeleteEverything();
	void		OnCloseUntil();
	void		OnAutoDisable();
	void		OnCount(quint32 fileCount, quint32 folderCount, quint64 totalSize);

protected:
	void		closeEvent(QCloseEvent *e);

	int			FindFiles(const QString& Folder, int* pUnfilteredCount = NULL);
	int			FindBoxFiles(const QString& Folder, int* pUnfilteredCount = NULL, bool bWildcardOnly = false);
	int			FindWildcardRecoverFiles(int* pUnfilteredCount = NULL);
	QPair<int, quint64> FindFiles(const QString& BoxedFolder, const QString& RealFolder,
		const QString& NtFolder, const QString& Name, const QString& ParentID = QString(),
		int* pUnfilteredCount = NULL, bool bWildcardOnly = false);

	struct SRecItem {
		QString FullPath;
		QString RelPath;
	};

	QMap<QString, SRecItem> GetFiles();

	void		RecoverFiles(bool bBrowse, QString RecoveryFolder = QString());

	CSandBoxPtr m_pBox;

	QMap<QVariant, QVariantMap> m_FileMap;
	QSet<QString> m_NewFiles;
	QMap<QString, QString> m_NewFilePaths;
	QMap<QString, QString> m_NewFileBoxPaths;

	QStringList m_RecoveryFolders;
	QMap<QString, QString> m_RecoveryNtFolders;
	QStringList m_WildcardRecoverFolders;

	CRecoveryCounter* m_pCounter;

	int m_LastTargetIndex;
	bool m_bTargetsChanged;
	bool m_bReloadPending;
	bool m_DeleteSnapshots;
	bool m_bImmediate;
	int m_UnfilteredFileCount;
	int m_IgnoredFileCount;

	bool m_UseIgnoreForQuick;
	struct SIgnorePattern {
		QString Pattern;
		QString NtPattern;
		QString DosPattern;
		QString BoxedPattern;
		bool HasWildcard = false;
		bool FullPath = false;
	};
	QList<SIgnorePattern> m_AutoRecoverIgnorePatterns;
	QVector<quint8> m_IgnoreMatchScratch;
	bool m_IgnorePatternsBuilt;

	static QList<SIgnorePattern> LoadIgnorePatterns(const CSandBoxPtr& pBox);
	static bool MatchIgnorePatterns(const QList<SIgnorePattern>& patterns,
		const QString& diskPath, const QString& ntPath,
		const QString& boxedPath, QVector<quint8>& scratch);
	void ReloadIgnoreSettings();
	void BuildIgnorePatterns();
	void UpdateShowIgnoredState();
	bool ShouldFilterIgnoredFiles() const;
	bool IsInWildcardRecoverFolders(const QString& diskPath, const QString& ntPath,
		const QString& boxedPath);
	bool IsExcludedByIgnorePatterns(const QString& diskPath, const QString& ntPath,
		const QString& boxedPath);

private:
	Ui::RecoveryWindow ui;
	QAction* m_pRemember;
	
	CSimpleTreeModel* m_pFileModel;
	QSortFilterProxyModel*	m_pSortProxy;

	QFileIconProvider m_IconProvider;
};
