#include "framework.h"
#include "ImDiskIO.h"
#include "ImBox.h"
#include "VirtualMemoryIO.h"
#include "..\Common\helpers.h"

struct SVirtualMemory
{
	ULONG64 uSize;
	
	int mem_block_size, mem_block_size_mask, mem_block_size_shift; 

	size_t table_size;
	void **ptr_table;

	volatile size_t n_block = 0;
};

CVirtualMemoryIO::CVirtualMemoryIO(ULONG64 uSize, int BlockSize)
{
	m = new SVirtualMemory;
	memset(m, 0, sizeof SVirtualMemory);
	m->uSize = uSize;

	m->mem_block_size_shift = BlockSize;
	if (m->mem_block_size_shift < 12) m->mem_block_size_shift = 12;
	if (m->mem_block_size_shift > 30) m->mem_block_size_shift = 30;
	m->mem_block_size = 1 << m->mem_block_size_shift;
	m->mem_block_size_mask = m->mem_block_size - 1;
}

CVirtualMemoryIO::~CVirtualMemoryIO()
{
	delete m;
}

ULONG64 CVirtualMemoryIO::GetDiskSize() const
{
	return m->uSize;
}

ULONG64 CVirtualMemoryIO::GetAllocSize() const
{ 
	return (ULONG64)m->n_block * m->mem_block_size; 
}

int CVirtualMemoryIO::Init()
{
	m->table_size = (m->uSize + m->mem_block_size_mask) / m->mem_block_size;

	SIZE_T alloc_size = m->table_size * sizeof(size_t);
	NtAllocateVirtualMemory(NtCurrentProcess(), (void**)&m->ptr_table, 0, &alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	return ERR_OK;
}

void CVirtualMemoryIO::Expand(ULONG64 uSize)
{
	if (uSize <= m->uSize)
		return;
	m->uSize = uSize;

	SIZE_T old_size = m->table_size;
	void** old_table = m->ptr_table;

	m->table_size = (m->uSize + m->mem_block_size_mask) / m->mem_block_size;

	m->ptr_table = NULL;
	SIZE_T alloc_size = m->table_size * sizeof(size_t);
	NtAllocateVirtualMemory(NtCurrentProcess(), (void**)&m->ptr_table, 0, &alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	memcpy(m->ptr_table, old_table, old_size * sizeof(size_t));

	old_size = 0;
	NtFreeVirtualMemory(NtCurrentProcess(), (void**)&old_table, &old_size, MEM_RELEASE);

	DbgPrint(L"Virtual RAM Disk resized\n");
}

bool CVirtualMemoryIO::DiskWrite(void* buf, int size, __int64 offset)
{
	bool ret = true;
	size_t index = offset >> m->mem_block_size_shift;
	int current_size;
	int block_offset = offset & m->mem_block_size_mask;
	void *ptr;
	bool data;
	SIZE_T alloc_size;
	MEMORYSTATUSEX mem_stat;

	mem_stat.dwLength = sizeof mem_stat;
	do {
		if (index >= m->table_size)
			Expand(offset + size);
		current_size = min(size + block_offset, m->mem_block_size) - block_offset;
		data = data_search(buf, current_size);
		if ((ptr = m->ptr_table[index])) {
			if (data)
				memcpy((BYTE*)ptr + block_offset, buf, current_size);
			else if (data_search(ptr, block_offset) || data_search((BYTE*)ptr + block_offset + current_size, m->mem_block_size - block_offset - current_size))
				ZeroMemory((BYTE*)ptr + block_offset, current_size);
			else {
				alloc_size = 0;
				NtFreeVirtualMemory(NtCurrentProcess(), &ptr, &alloc_size, MEM_RELEASE);
				m->ptr_table[index] = NULL;
				m->n_block--;
			}
		}
		else if (data) {
			GlobalMemoryStatusEx(&mem_stat);
			alloc_size = m->mem_block_size;
			if (mem_stat.ullAvailPageFile >= MINIMAL_MEM && (NtAllocateVirtualMemory(NtCurrentProcess(), &m->ptr_table[index], 0, &alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE) == STATUS_SUCCESS)) {
				memcpy((BYTE*)m->ptr_table[index] + block_offset, buf, current_size);
				m->n_block++;
			}
			else {
				ret = false;
			}
		}
		block_offset = 0;
		buf = (BYTE*)buf + current_size;
		index++;
		size -= current_size;
	} while (size > 0);

	return ret;
}

bool CVirtualMemoryIO::DiskRead(void* buf, int size, __int64 offset)
{
	size_t index = offset >> m->mem_block_size_shift;
	int current_size;
	int block_offset = offset & m->mem_block_size_mask;

	do {
		if (index >= m->table_size)
			Expand(offset + size);
		current_size = min(size + block_offset, m->mem_block_size) - block_offset;
		if (m->ptr_table[index])
			memcpy(buf, (BYTE*)m->ptr_table[index] + block_offset, current_size);
		else
			ZeroMemory(buf, current_size);
		block_offset = 0;
		buf = (BYTE*)buf + current_size;
		index++;
		size -= current_size;
	} while (size > 0);

	return true;
}

void CVirtualMemoryIO::TrimProcess(DEVICE_DATA_SET_RANGE* range, int n)
{
	size_t index;
	int current_size, block_offset;
	__int64 size;
	void *ptr;
	SIZE_T alloc_size;

	while (n) {
		index = range->StartingOffset >> m->mem_block_size_shift;
		block_offset = range->StartingOffset & m->mem_block_size_mask;
		for (size = range->LengthInBytes; size > 0; size -= current_size) {
			if (index >= m->table_size)
				break;
			current_size = min(size + block_offset, (__int64)m->mem_block_size) - block_offset;
			if ((ptr = m->ptr_table[index])) {
				if (data_search(ptr, block_offset) || data_search((BYTE*)ptr + block_offset + current_size, m->mem_block_size - block_offset - current_size))
					ZeroMemory((BYTE*)ptr + block_offset, current_size);
				else {
					alloc_size = 0;
					NtFreeVirtualMemory(NtCurrentProcess(), &ptr, &alloc_size, MEM_RELEASE);
					m->ptr_table[index] = NULL;
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