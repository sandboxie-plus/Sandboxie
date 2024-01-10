#pragma once

class CAbstractIO
{
public:
	virtual ~CAbstractIO() {}

	virtual ULONG64 GetAllocSize() const = 0;
	virtual ULONG64 GetDiskSize() const = 0;
	virtual bool CanBeFormated() const = 0;

	virtual int Init() = 0;
	virtual void PrepViewOfFile(BYTE*) = 0;

	virtual bool DiskWrite(void* buf, int size, __int64 offset) = 0;
	virtual bool DiskRead(void* buf, int size, __int64 offset) = 0;
	virtual void TrimProcess(DEVICE_DATA_SET_RANGE* range, int n) = 0;
};
