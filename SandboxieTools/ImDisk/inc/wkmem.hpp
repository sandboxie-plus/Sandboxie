#pragma once

inline void *operator_new(size_t Size, UCHAR FillByte)
{
    void * result = ExAllocatePoolWithTag(NonPagedPool, Size, POOL_TAG);

    if (result != NULL)
    {
        RtlFillMemory(result, Size, FillByte);
    }

    return result;
}

inline void operator_delete(void *Ptr)
{
    if (Ptr != NULL)
    {
        ExFreePoolWithTag(Ptr, POOL_TAG);
    }
}

void * __CRTDECL operator new(size_t Size);

void * __CRTDECL operator new[](size_t Size);

void * __CRTDECL operator new(size_t Size, UCHAR FillByte);

void __CRTDECL operator delete(void * Ptr);

void __CRTDECL operator delete(void * Ptr, size_t);

void __CRTDECL operator delete[](void * Ptr);

template<typename T, POOL_TYPE pool_type> class WPoolMem
{
protected:
    T *ptr;
    SIZE_T bytecount;

    explicit WPoolMem(T *pBlk, SIZE_T AllocationSize)
        : ptr(pBlk),
        bytecount(pBlk != NULL ? AllocationSize : 0) { }

public:
    operator bool()
    {
        return ptr != NULL;
    }

    bool operator!()
    {
        return ptr == NULL;
    }

    operator T*()
    {
        return ptr;
    }

    T* operator ->()
    {
        return ptr;
    }

    T* operator+(int i)
    {
        return ptr + i;
    }

    T* operator-(int i)
    {
        return ptr - i;
    }

    T* operator =(T *pBlk)
    {
        Free();
        return ptr = pBlk;
    }

    T* GetPtr()
    {
        return ptr;
    }

    T* GetPtr(LONG_PTR offset)
    {
        return ptr + offset;
    }

    template<typename TC> TC* GetPtr()
    {
        return (TC*)(void*)ptr;
    }

    template<typename TC> TC* GetPtr(LONG_PTR offset)
    {
        return ((TC*)(void*)ptr) + offset;
    }

    SIZE_T Count() const
    {
        return GetSize() / sizeof(T);
    }

    SIZE_T GetSize() const
    {
        return ptr != NULL ? bytecount : 0;
    }

    void Free()
    {
        if (ptr != NULL)
        {
            ExFreePoolWithTag(ptr, POOL_TAG);
            ptr = NULL;
        }
    }

    void Clear()
    {
        if ((ptr != NULL) && (bytecount > 0))
        {
            RtlZeroMemory(ptr, bytecount);
        }
    }

    T* Abandon()
    {
        T* ab_ptr = ptr;
        ptr = NULL;
        bytecount = 0;
        return ab_ptr;
    }

    ~WPoolMem()
    {
        Free();
    }

    void Initialize(SIZE_T AllocateSize)
    {
        ptr = (T*)ExAllocatePoolWithTag(pool_type, AllocateSize, POOL_TAG);
        bytecount = AllocateSize;
    }

public:
    WPoolMem() :
        ptr(NULL),
        bytecount(0) { }

    explicit WPoolMem(SIZE_T AllocateSize)
    {
        Initialize(AllocateSize);
    }

    T* Alloc(SIZE_T AllocateSize)
    {
        Free();
        Initialize(AllocateSize);
        return ptr;
    }
};

template<POOL_TYPE pool_type> struct WUnicodeString : public UNICODE_STRING
{
    operator bool()
    {
        return (Buffer != NULL) && (MaximumLength != 0);
    }

    operator !()
    {
        return (Buffer == NULL) || (MaximumLength == 0);
    }

    WUnicodeString(USHORT max_length)
    {
        Length = 0;
        Buffer = (PWCHAR)ExAllocatePoolWithTag(pool_type, max_length, MP_TAG_GENERAL);
        MaximumLength = Buffer != NULL ? max_length : 0;
    }

    WUnicodeString(PWCHAR buffer, USHORT length)
    {
        Buffer = buffer;
        Length = MaximumLength = length;
    }

    WUnicodeString(PWCHAR buffer, USHORT length, USHORT max_length)
    {
        Buffer = buffer;
        Length = length;
        MaximumLength = max_length;
    }

    ~WUnicodeString()
    {
        if (Buffer != NULL)
        {
            ExFreePoolWithTag(Buffer, MP_TAG_GENERAL);
        }
    }
};

template<POOL_TYPE pool_type> struct WAnsiString : public ANSI_STRING
{
    operator bool()
    {
        return (Buffer != NULL) && (MaximumLength != 0);
    }

    operator !()
    {
        return (Buffer == NULL) || (MaximumLength == 0);
    }

    WAnsiString(USHORT max_length)
    {
        Length = 0;
        Buffer = (PCHAR)ExAllocatePoolWithTag(pool_type, max_length, MP_TAG_GENERAL);
        MaximumLength = Buffer != NULL ? max_length : 0;
    }

    WAnsiString(PCHAR buffer, USHORT length)
    {
        Buffer = buffer;
        Length = MaximumLength = length;
    }

    WAnsiString(PCHAR buffer, USHORT length, USHORT max_length)
    {
        Buffer = buffer;
        Length = length;
        MaximumLength = max_length;
    }

    ~WAnsiString()
    {
        if (Buffer != NULL)
        {
            ExFreePoolWithTag(Buffer, MP_TAG_GENERAL);
        }
    }
};

template<POOL_TYPE pool_type> struct WOemString : public OEM_STRING
{
    operator bool()
    {
        return (Buffer != NULL) && (MaximumLength != 0);
    }

    operator !()
    {
        return (Buffer == NULL) || (MaximumLength == 0);
    }

    WOemString(USHORT max_length)
    {
        Length = 0;
        Buffer = (PCHAR)ExAllocatePoolWithTag(pool_type, max_length, MP_TAG_GENERAL);
        MaximumLength = Buffer != NULL ? max_length : 0;
    }

    WOemString(PCHAR buffer, USHORT length)
    {
        Buffer = buffer;
        Length = MaximumLength = length;
    }

    WOemString(PCHAR buffer, USHORT length, USHORT max_length)
    {
        Buffer = buffer;
        Length = length;
        MaximumLength = max_length;
    }

    ~WOemString()
    {
        if (Buffer != NULL)
        {
            ExFreePoolWithTag(Buffer, MP_TAG_GENERAL);
        }
    }
};

class WHandle
{
private:
    HANDLE h;

public:
    operator bool()
    {
        return h != NULL;
    }

    bool operator !()
    {
        return h == NULL;
    }

    operator HANDLE()
    {
        return h;
    }

    void Close()
    {
        if (h != NULL)
        {
            ZwClose(h);
            h = NULL;
        }
    }

    WHandle() :
        h(NULL) { }

    explicit WHandle(HANDLE h) :
        h(h) { }

    ~WHandle()
    {
        Close();
    }
};

