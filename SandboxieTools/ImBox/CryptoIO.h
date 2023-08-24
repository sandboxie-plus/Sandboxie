#pragma once
#include "AbstractIO.h"

class CCryptoIO : public CAbstractIO
{
public:
	CCryptoIO(CAbstractIO* pIO, const WCHAR* pKey, const std::wstring& Cipher = std::wstring());
	virtual ~CCryptoIO();

	virtual ULONG64 GetAllocSize() const { return m_pIO->GetAllocSize(); }
	virtual ULONG64 GetDiskSize() const;
	virtual bool CanBeFormated() const;

	virtual int Init();
	virtual void PrepViewOfFile(BYTE* p) { m_pIO->PrepViewOfFile(p); }
	virtual int ChangePassword(const WCHAR* pNewKey);

	virtual bool DiskWrite(void* buf, int size, __int64 offset);
	virtual bool DiskRead(void* buf, int size, __int64 offset);
	virtual void TrimProcess(DEVICE_DATA_SET_RANGE* range, int n);

	static int BackupHeader(CAbstractIO* pIO, const std::wstring& Path);
	static int RestoreHeader(CAbstractIO* pIO, const std::wstring& Path);

protected:
	virtual int InitCrypto();
	virtual int WriteHeader(struct _dc_header* header);

	struct SCryptoIO* m;

public:
	CAbstractIO* m_pIO;
};

