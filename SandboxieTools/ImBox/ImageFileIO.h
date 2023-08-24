#pragma once
#include "AbstractIO.h"

class CImageFileIO : public CAbstractIO
{
public:
	CImageFileIO(std::wstring& FilePath, ULONG64 uSize = 0);
	virtual ~CImageFileIO();

	virtual ULONG64 GetDiskSize() const;
	virtual ULONG64 GetAllocSize() const;
	virtual bool CanBeFormated() const;

	virtual int Init();
	virtual void PrepViewOfFile(BYTE*) {}

	virtual bool DiskWrite(void* buf, int size, __int64 offset);
	virtual bool DiskRead(void* buf, int size, __int64 offset);
	virtual void TrimProcess(DEVICE_DATA_SET_RANGE* range, int n);

protected:
	struct SImageFileIO* m;
};

