#pragma once

#define ERROR_OK (1)
#define OP_ASYNC (2)
#define OP_CONFIRM (3)
#define OP_CANCELED (4)

#include "qsbieapi_global.h"

QString QSBIEAPI_EXPORT CSbieAPI__FormatNtStatus(long nsCode);

enum ESbieMsgCodes
{
	SB_Generic = 0,
	SB_Message,
	SB_NeedAdmin,
	SB_ExecFail,
	SB_DriverFail,
	SB_ServiceFail,
	SB_Incompatible,
	SB_PathFail,
	SB_FailedCopyConf,
	SB_AlreadyExists,
	SB_DeleteFailed,
	SB_NameLenLimit,
	SB_BadNameDev,
	SB_BadNameChar,
	SB_FailedKillAll,
	SB_DeleteProtect,
	SB_DeleteNotEmpty,
	SB_DeleteError,
	//SB_RemNotEmpty,
	SB_DelNotEmpty,
	SB_FailedMoveDir,
	SB_SnapMkDirFail,
	SB_SnapCopyDatFail,
	SB_SnapNotFound,
	SB_SnapMergeFail,
	SB_SnapRmDirFail,
	SB_SnapIsShared,
	SB_SnapIsRunning,
	SB_SnapDelDatFail,
	SB_NotAuthorized,
	SB_ConfigFailed,
	SB_SnapIsEmpty,
	SB_NameExists,
	SB_PasswordBad,
	SB_Canceled,
	SB_LastError
};

class CSbieStatus
{
public:
	CSbieStatus()
	{
		m = NULL;
	}
	CSbieStatus(ESbieMsgCodes MsgCode, const QVariantList& Args = QVariantList(), long Status = 0xC0000001 /*STATUS_UNSUCCESSFUL*/) : CSbieStatus()
	{
		SFlexError* p = new SFlexError();
		p->MsgCode = MsgCode;
		p->Args = Args;
		p->Status = Status;
		Attach(p);
	}
	CSbieStatus(ESbieMsgCodes MsgCode, long Status) : CSbieStatus(MsgCode, QVariantList(), Status)
	{
	}
	CSbieStatus(long Status) : CSbieStatus(SB_Generic, QVariantList() << CSbieAPI__FormatNtStatus(Status), Status)
	{
	}
	CSbieStatus(const CSbieStatus& other) : CSbieStatus()
	{
		if(other.m != NULL)
			Attach((SFlexError*)other.m);
	}
	~CSbieStatus()
	{
		Detach();
	}

	CSbieStatus& operator=(const CSbieStatus& other)
	{
		Attach(&other);
		return *this; 
	}

	__inline bool IsError() const { return m != NULL; }
	__inline long GetStatus() const { return m ? m->Status : 0; }
	__inline long GetMsgCode() const { return m ? m->MsgCode : 0; }
	__inline QVariantList GetArgs() const { return m ? m->Args : QVariantList(); }
	//__inline QString GetText() const { return m ? m->Text: ""; }

	operator bool() const				{return !IsError();}

protected:
	struct SFlexError
	{
		ESbieMsgCodes MsgCode;
		QVariantList Args;
		long Status;

		mutable std::atomic<int> aRefCnt;
	} *m;

	void Attach(const CSbieStatus* p)
	{
		Attach(p->m);
	}

	void Attach(SFlexError* p)
	{
		Detach();

		if (p != NULL)
		{
			p->aRefCnt.fetch_add(1);
			m = p;
		}
	}

	void Detach()
	{
		if (m != NULL)
		{
			if (m->aRefCnt.fetch_sub(1) == 1)
				delete m;
			m = NULL;
		}
	}
};


typedef CSbieStatus SB_STATUS;
#define SB_OK SB_STATUS()
#define SB_ERR SB_STATUS

template <class T>
class CSbieResult : public CSbieStatus
{
public:
	CSbieResult(const T& value = T()) : CSbieStatus()
	{
		v = value;
	}
	CSbieResult(long Status, const T& value = T()) : CSbieStatus(Status)
	{
		v = value;
	}
	CSbieResult(const CSbieStatus& other) : CSbieResult()
	{
		Attach(&other);
	}
	CSbieResult(const CSbieResult& other) : CSbieStatus(other)
	{
		v = other.v;
	}

	__inline T GetValue() const { return v; }

private:
	T v;
};

#define SB_RESULT(x) CSbieResult<x>
#define SB_RETURN(x,y) CSbieResult<x>(y)


#include "qsbieapi_global.h"


class QSBIEAPI_EXPORT CSbieProgress : public QObject
{
	Q_OBJECT
public:
	CSbieProgress() : m_Status(OP_ASYNC), m_Canceled(false) {}

	void Cancel() { m_Canceled = true; emit Canceled(); }
	bool IsCanceled() { return m_Canceled; }

	void ShowMessage(const QString& text) { emit Message(text);}
	void SetProgress(int value) { emit Progress(value); }
	void Finish(SB_STATUS status) { m_Status = m_Canceled ? SB_ERR(OP_CANCELED) : status; emit Finished(); }

	SB_STATUS GetStatus() { return m_Status; }
	bool IsFinished() { return m_Status.GetStatus() != OP_ASYNC; }

signals:
	//void Progress(int procent);
	void Message(const QString& text);
	void Progress(int value);
	void Canceled();
	void Finished();

protected:
	volatile bool m_Canceled;
	SB_STATUS m_Status;
};

typedef QSharedPointer<CSbieProgress> CSbieProgressPtr;

#define SB_PROGRESS CSbieResult<CSbieProgressPtr>