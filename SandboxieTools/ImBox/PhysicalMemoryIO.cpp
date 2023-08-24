#include "framework.h"
#include "ImDiskIO.h"
#include "ImBox.h"
#include "PhysicalMemoryIO.h"
#include "..\Common\helpers.h"
#include "..\ImDisk\inc\imdproxy.h"
#include "..\ImDisk\inc\imdisk.h"

struct SPhysicalMemory
{
	ULONG64 uSize;

	int mem_block_size, mem_block_size_mask, mem_block_size_shift; 

	size_t table_size;
	void *virtual_mem_ptr = NULL;
	bool *allocated_block = NULL;
	HANDLE current_process;
	ULONG_PTR n_pages, *pfn = NULL;

	volatile size_t n_block = 0;
};

CPhysicalMemoryIO::CPhysicalMemoryIO(ULONG64 uSize, int BlockSize)
{
	m = new SPhysicalMemory;
	memset(m, 0, sizeof SPhysicalMemory);
	m->uSize = uSize;

	m->mem_block_size_shift = BlockSize;
	if (m->mem_block_size_shift < 12) m->mem_block_size_shift = 12;
	if (m->mem_block_size_shift > 30) m->mem_block_size_shift = 30;
	m->mem_block_size = 1 << m->mem_block_size_shift;
	m->mem_block_size_mask = m->mem_block_size - 1;
}

CPhysicalMemoryIO::~CPhysicalMemoryIO()
{
	delete m;
}

ULONG64 CPhysicalMemoryIO::GetDiskSize() const
{
	return m->uSize;
}

ULONG64 CPhysicalMemoryIO::GetAllocSize() const
{ 
	return (ULONG64)m->n_block * m->mem_block_size; 
}

int CPhysicalMemoryIO::Init()
{
	TOKEN_PRIVILEGES tok_priv;
	tok_priv.PrivilegeCount = 1;
	tok_priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	m->current_process = GetCurrentProcess();
	HANDLE token;
	if (!OpenProcessToken(m->current_process, TOKEN_ADJUST_PRIVILEGES, &token) ||
		!LookupPrivilegeValueA(NULL, "SeLockMemoryPrivilege", &tok_priv.Privileges[0].Luid) ||
		!AdjustTokenPrivileges(token, FALSE, &tok_priv, 0, NULL, NULL) ||
		GetLastError() != ERROR_SUCCESS)
			return ERR_PRIVILEGE;
	NtClose(token);


	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	if (!(m->n_pages = m->mem_block_size / sys.dwPageSize))
		return ERR_INTERNAL;

	m->table_size = (m->uSize + m->mem_block_size_mask) / m->mem_block_size;

	SIZE_T alloc_size = m->n_pages * m->table_size * sizeof(size_t);
	NtAllocateVirtualMemory(NtCurrentProcess(), (void**)&m->pfn, 0, &alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	alloc_size = m->table_size * sizeof(bool);
	NtAllocateVirtualMemory(NtCurrentProcess(), (void**)&m->allocated_block, 0, &alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	alloc_size = m->mem_block_size;
	NtAllocateVirtualMemory(NtCurrentProcess(), &m->virtual_mem_ptr, 0, &alloc_size, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE);

	return ERR_OK;
}

void CPhysicalMemoryIO::Expand(ULONG64 uSize)
{
	if (uSize <= m->uSize)
		return;
	m->uSize = uSize;

	SIZE_T old_size = m->table_size;
	ULONG_PTR* old_pfn = m->pfn;
	bool* old_block = m->allocated_block;

	m->table_size = (m->uSize + m->mem_block_size_mask) / m->mem_block_size;
	
	m->pfn = NULL;
	SIZE_T alloc_size = m->n_pages * m->table_size * sizeof(size_t);
	NtAllocateVirtualMemory(NtCurrentProcess(), (void**)&m->pfn, 0, &alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	m->allocated_block = NULL;
	alloc_size = m->table_size * sizeof(bool);
	NtAllocateVirtualMemory(NtCurrentProcess(), (void**)&m->allocated_block, 0, &alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	memcpy(m->pfn, old_pfn, m->n_pages * old_size * sizeof(size_t));
	memcpy(m->allocated_block, old_block, old_size * sizeof(bool));

	old_size = 0;
	NtFreeVirtualMemory(NtCurrentProcess(), (void**)&old_pfn, &old_size, MEM_RELEASE);
	old_size = 0;
	NtFreeVirtualMemory(NtCurrentProcess(), (void**)&old_block, &old_size, MEM_RELEASE);

	DbgPrint(L"Physical RAM Disk resized\n");
}

void CPhysicalMemoryIO::PrepViewOfFile(BYTE* shm_view)
{
	SIZE_T min_working_set, max_working_set;

	GetProcessWorkingSetSize(m->current_process, &min_working_set, &max_working_set);
	min_working_set += DEF_BUFFER_SIZE + IMDPROXY_HEADER_SIZE;
	max_working_set += DEF_BUFFER_SIZE + IMDPROXY_HEADER_SIZE;
	SetProcessWorkingSetSize(m->current_process, min_working_set, max_working_set);
	VirtualLock(shm_view, DEF_BUFFER_SIZE + IMDPROXY_HEADER_SIZE);
}

bool CPhysicalMemoryIO::DiskWrite(void* buf, int size, __int64 offset)
{
	bool ret = true;
	size_t index = offset >> m->mem_block_size_shift;
	int current_size;
	int block_offset = offset & m->mem_block_size_mask;
	bool data;
	ULONG_PTR allocated_pages, *pfn_ptr;
	MEMORYSTATUSEX mem_stat;

	mem_stat.dwLength = sizeof mem_stat;
	do {
		if (index >= m->table_size)
			Expand(offset + size);
		current_size = min(size + block_offset, m->mem_block_size) - block_offset;
		data = data_search(buf, current_size);
		if (m->allocated_block[index]) {
			MapUserPhysicalPages(m->virtual_mem_ptr, m->n_pages, m->pfn + index * m->n_pages);
			if (data)
				memcpy((BYTE*)m->virtual_mem_ptr + block_offset, buf, current_size);
			else if (data_search(m->virtual_mem_ptr, block_offset) || data_search((BYTE*)m->virtual_mem_ptr + block_offset + current_size, m->mem_block_size - block_offset - current_size))
				ZeroMemory((BYTE*)m->virtual_mem_ptr + block_offset, current_size);
			else {
				allocated_pages = m->n_pages;
				FreeUserPhysicalPages(m->current_process, &allocated_pages, m->pfn + index * m->n_pages);
				m->allocated_block[index] = FALSE;
				m->n_block--;
			}
		}
		else if (data) {
			GlobalMemoryStatusEx(&mem_stat);
			if (mem_stat.ullAvailPageFile < MINIMAL_MEM) {
				ret = false;
			}
			else {
				allocated_pages = m->n_pages;
				pfn_ptr = m->pfn + index * m->n_pages;
				if (!AllocateUserPhysicalPages(m->current_process, &allocated_pages, pfn_ptr)) {
					ret = false;
				}
				else if (allocated_pages != m->n_pages) {
					FreeUserPhysicalPages(m->current_process, &allocated_pages, pfn_ptr);
					ret = false;
				} else {
					MapUserPhysicalPages(m->virtual_mem_ptr, m->n_pages, pfn_ptr);
					memcpy((BYTE*)m->virtual_mem_ptr + block_offset, buf, current_size);
					m->allocated_block[index] = TRUE;
					m->n_block++;
				}
			}
		}
		block_offset = 0;
		buf = (BYTE*)buf + current_size;
		index++;
		size -= current_size;
	} while (size > 0);

	return ret;
}

bool CPhysicalMemoryIO::DiskRead(void* buf, int size, __int64 offset)
{
	size_t index = offset >> m->mem_block_size_shift;
	int current_size;
	int block_offset = offset & m->mem_block_size_mask;

	do {
		if (index >= m->table_size)
			Expand(offset + size);
		current_size = min(size + block_offset, m->mem_block_size) - block_offset;
		if (m->allocated_block[index]) {
			MapUserPhysicalPages(m->virtual_mem_ptr, m->n_pages, m->pfn + index * m->n_pages);
			memcpy(buf, (BYTE*)m->virtual_mem_ptr + block_offset, current_size);
		} else
			ZeroMemory(buf, current_size);
		block_offset = 0;
		buf = (BYTE*)buf + current_size;
		index++;
		size -= current_size;
	} while (size > 0);

	return true;
}

void CPhysicalMemoryIO::TrimProcess(DEVICE_DATA_SET_RANGE* range, int n)
{
	size_t index;
	int current_size, block_offset;
	__int64 size;
	ULONG_PTR allocated_pages;

	while (n) {
		index = range->StartingOffset >> m->mem_block_size_shift;
		block_offset = range->StartingOffset & m->mem_block_size_mask;
		for (size = range->LengthInBytes; size > 0; size -= current_size) {
			if (index >= m->table_size)
				break;
			current_size = min(size + block_offset, (__int64)m->mem_block_size) - block_offset;
			if (m->allocated_block[index]) {
				MapUserPhysicalPages(m->virtual_mem_ptr, m->n_pages, m->pfn + index * m->n_pages);
				if (data_search(m->virtual_mem_ptr, block_offset) || data_search((BYTE*)m->virtual_mem_ptr + block_offset + current_size, m->mem_block_size - block_offset - current_size))
					ZeroMemory((BYTE*)m->virtual_mem_ptr + block_offset, current_size);
				else {
					allocated_pages = m->n_pages;
					FreeUserPhysicalPages(m->current_process, &allocated_pages, m->pfn + index * m->n_pages);
					m->allocated_block[index] = FALSE;
					m->n_block--;
				}
			}
			block_offset = 0;
			index++;
		}
		range++;
		n--;
	}
}