#pragma once
#include <QThread>
#include <QJSEngine>
#include "../../QSbieAPI/SbieAPI.h"

#include "JSEngineExt.h"

class CBoxEngine : public QThread
{
	Q_OBJECT
public:
	CBoxEngine(QObject* parent = NULL);
	~CBoxEngine();

	bool				RunScript(const QString& Script, const QString& Name, const QVariantMap& Params = QVariantMap());

	static void			StopAll();

	enum EState {
		eUnknown,

		eRunning,
		eRunningAsync,
		eReady,
		eQuery,

		eCompleted,
		eSuccess,
		eFailed,
		eError,
		eCanceled
	};

	EState				GetState() const { return m_State; }

	bool				IsRunning() const { return m_State == eRunning || m_State == eRunningAsync; }
	bool				Continue() { return Continue(false); }

	bool				HasFailed() const { return m_State == eFailed; }
	bool				HasError() const { return m_State == eError; }
	bool				WasSuccessfull() const { return m_State == eSuccess; }

	bool				IsReady() const { return m_State == eReady; }
	bool				HasQuery() const { return m_State == eQuery; }
	QVariantMap			GetQuery() const { QMutexLocker Locker(&m_Mutex); return m_State == eQuery ? m_Data : QVariantMap(); }
	bool				SetResult(const QVariantMap& Result);
	QVariant			GetResult() const { return m_Result; }

	Q_INVOKABLE void	AppendLog(const QString& Line);
	//QString				GetLog() const { QMutexLocker Locker(&m_Mutex); return m_Log; }

	CJSEngineExt*		GetEngine() { return m_pEngine; }

	QObject*			GetDebuggerBackend();

signals:
	
	void				StateChanged(int state, const QString& Text = "");

	void				BoxUsed(const CSandBoxPtr& pBox);

	void				LogMessage(const QString& Line);

protected:
	friend class JSysObject;
	friend class JSbieObject;
	friend class JIniObject;
	friend class JConfObject;
	friend class JBoxObject;
	friend class JBoxWObject;

	virtual void		init();
	virtual void		run();
	virtual void		Stop();
	//virtual bool		Wait();
	virtual bool		Continue(bool bLocked, EState State = eRunning);

	virtual void		SetState(EState state, const QString& Text = "");
	bool				TestRunning();
	bool				WaitLocked();

	CJSEngineExt*		m_pEngine;
	QObject*			m_pDebuggerBackend;
	QString				m_Script;
	QString				m_Name;
	QVariantMap			m_Params;
	QVariant			m_Result;

	mutable QMutex		m_Mutex;
	QWaitCondition		m_Wait;
	QVariantMap			m_Data;
	EState				m_State;

	//QString				m_Log;

	static QSet<CBoxEngine*> m_Instances;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CWizardEngine
// 

class CWizardEngine : public CBoxEngine
{
	Q_OBJECT
public:
	CWizardEngine(QObject* parent = NULL);
	~CWizardEngine();

	QVariantMap GetReport() { return m_Report; }

	bool		ApplyShadowChanges();

	QSharedPointer<CSbieIni> MakeShadow(const QSharedPointer<CSbieIni>& pIni);
	void		AddShadow(const QSharedPointer<CSbieIni>& pIni);
	void		SetApplyShadow(const QString& OriginalName, bool bApply = true);
	bool		IsNoAppliedShadow(const QString& OriginalName);

	static int	GetInstanceCount() { return m_InstanceCount; }

	Q_INVOKABLE void OpenSettings(const QString& page);
	Q_INVOKABLE void OpenOptions(const QString& box, const QString& page);

protected:
	friend class JWizardObject;

	virtual void		init();

	virtual void		SetState(EState state, const QString& Text = "");

	QVariantMap m_Report;

	struct SBoxShadow {
		SBoxShadow() : iApplyChanges(0) {}
		QSharedPointer<CSbieIni> pShadow;
		QSharedPointer<CSbieIni> pOriginal;
		int iApplyChanges;
	};

	QMap<QString, SBoxShadow> m_Shadows;

	static int			m_InstanceCount;
};

