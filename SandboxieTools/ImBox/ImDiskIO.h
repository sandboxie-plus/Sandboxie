#pragma once
#include "AbstractIO.h"

#define DEF_BUFFER_SIZE (1 << 20)
#define MINIMAL_MEM (100 << 20)

class CImDiskIO
{
public:
	CImDiskIO(CAbstractIO* pIO, const std::wstring& Mount, const std::wstring& Format, const std::wstring& Params = L"");
	~CImDiskIO();

	void		SetProxyName(const std::wstring& Name);
	void		SetMountEvent(HANDLE hEvent);
	void		SetMountSection(HANDLE hMapping, WCHAR* pSection);

	int			DoComm();

protected:
	struct SImDiskIO* m;

	CAbstractIO* m_pIO;
	HANDLE m_hThread;
};

void disp_err_mem();

extern bool (*data_search)(void *ptr, int size);