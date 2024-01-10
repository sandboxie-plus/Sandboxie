#include <WINDOWS.H>

/*
WPreserveLastError

An object of this class preserves value of Win32 GetLastError(). Constructor
calls GetLastError() and saves returned value. Destructor calls SetLastError()
with saved value.
*/
class WPreserveLastError
{
public:
    DWORD Value;

    WPreserveLastError()
    {
        Value = GetLastError();
    }

    ~WPreserveLastError()
    {
        SetLastError(Value);
    }
};

struct WSystemInfo : public SYSTEM_INFO
{
    WSystemInfo()
    {
        GetSystemInfo(this);
    }
};

#if _WIN32_WINNT >= 0x0501
struct WNativeSystemInfo : public SYSTEM_INFO
{
    WNativeSystemInfo()
    {
        GetNativeSystemInfo(this);
    }
};
#endif

struct WOSVersionInfo : public OSVERSIONINFO
{
    WOSVersionInfo()
    {
        dwOSVersionInfoSize = sizeof(*this);

#if _MSC_VER >= 1500
#pragma warning(suppress: 4996)
#endif
        GetVersionEx(this);
    }
};

struct WOSVersionInfoEx : public OSVERSIONINFOEX
{
    WOSVersionInfoEx()
    {
        dwOSVersionInfoSize = sizeof(*this);

#if _MSC_VER >= 1500
#pragma warning(suppress: 4996)
#endif
        GetVersionEx((LPOSVERSIONINFO)this);
    }
};

template<typename T> class WMemHolder
{
protected:
    T *ptr;

public:
    operator bool() const
    {
        return ptr != NULL;
    }

    bool operator!() const
    {
        return ptr == NULL;
    }

    operator T*() const
    {
        return ptr;
    }

    T* operator ->() const
    {
        return ptr;
    }

    T* operator+(int i) const
    {
        return ptr + i;
    }

    T* operator-(int i) const
    {
        return ptr - i;
    }

    T* operator =(T *pBlk)
    {
        Free();
        return ptr = pBlk;
    }

    T* Abandon()
    {
        T* ab_ptr = ptr;
        ptr = NULL;
        return ab_ptr;
    }

    WMemHolder()
        : ptr(NULL) { }

    explicit WMemHolder(T *pBlk)
        : ptr(pBlk)
    {
    }
};

class WEnvironmentStrings : public WMemHolder<TCHAR>
{
public:
    WEnvironmentStrings()
        : WMemHolder<TCHAR>(GetEnvironmentStrings())
    {
    }

    BOOL Free()
    {
        if (ptr != NULL)
        {
            return FreeEnvironmentStrings(ptr);
        }
        else
        {
            return TRUE;
        }
    }

    ~WEnvironmentStrings()
    {
        Free();
    }
};

template<typename T> class WMem : public WMemHolder<T>
{
public:
    T* operator =(T *pBlk)
    {
        Free();
        return ptr = pBlk;
    }

    DWORD_PTR Count() const
    {
        return GetSize() / sizeof(T);
    }

    DWORD_PTR GetSize() const
    {
        if (ptr == NULL)
            return 0;
        else
            return LocalSize(ptr);
    }

    /* WMem::ReAlloc()
    *
    * Note that this function uses LocalReAlloc() which makes it lose the
    * data if the block must be moved to increase.
    */
    T* ReAlloc(DWORD dwAllocSize)
    {
        T *newblock = (T*)LocalReAlloc(ptr, dwAllocSize, LMEM_ZEROINIT);
        if (newblock != NULL)
            return ptr = newblock;
        else
            return NULL;
    }

    T* Free()
    {
        if (ptr == NULL)
            return NULL;
        else
            return ptr = (T*)LocalFree(ptr);
    }

    WMem()
    {
    }

    explicit WMem(DWORD dwAllocSize)
        : WMemHolder<T>(LocalAlloc(LPTR, dwAllocSize))
    {
    }

    explicit WMem(T *pBlk)
        : WMemHolder<T>(pBlk)
    {
    }

    ~WMem()
    {
        Free();
    }
};

#ifdef _INC_MALLOC
template<typename T> class WCRTMem : public WMemHolder<T>
{
public:
    T* operator =(T *pBlk)
    {
        Free();
        return ptr = pBlk;
    }

    size_t Count() const
    {
        return GetSize() / sizeof(T);
    }

    size_t GetSize() const
    {
        if (ptr == NULL)
            return 0;
        else
            return _msize(ptr);
    }

    /* WHeapMem::ReAlloc()
    *
    * This function uses realloc() which makes it preserve the data if the
    * block must be moved to increase.
    */
    T* ReAlloc(size_t dwAllocSize)
    {
        T *newblock = (T*)realloc(ptr, dwAllocSize);
        if (newblock != NULL)
            return ptr = newblock;
        else
            return NULL;
    }

    void Free()
    {
        if (ptr != NULL)
        {
            free(ptr);
            ptr = NULL;
        }
    }

    WCRTMem()
    {
    }

    explicit WCRTMem(size_t dwAllocSize)
        : WMemHolder<T>((T*)malloc(dwAllocSize)) { }

    explicit WCRTMem(T *pBlk)
        : WMemHolder<T>(pBlk) { }

    ~WCRTMem()
    {
        Free();
    }
};
#endif

template<typename T> class WHeapMem :public WMemHolder<T>
{
public:
    T* operator =(T *pBlk)
    {
        Free();
        return ptr = pBlk;
    }

    WHeapMem<T> & operator =(WHeapMem<T> &o)
    {
        Free();
        ptr = o.ptr;
        o.ptr = NULL;
        return *this;
    }

    SIZE_T Count() const
    {
        return GetSize() / sizeof(T);
    }

    SIZE_T GetSize(DWORD dwFlags = 0) const
    {
        if (ptr == NULL)
            return 0;
        else
            return HeapSize(GetProcessHeap(), dwFlags, ptr);
    }

    /* WHeapMem::ReAlloc()
    *
    * This function uses HeapReAlloc() which makes it preserve the data if the
    * block must be moved to increase.
    */
    T* ReAlloc(SIZE_T AllocSize, DWORD dwFlags = 0)
    {
        if (ptr == NULL)
        {
            return ptr =
                (T*)HeapAlloc(GetProcessHeap(), dwFlags, AllocSize);
        }

        T *newblock = (T*)HeapReAlloc(GetProcessHeap(), dwFlags, ptr, AllocSize);

        if (newblock != NULL)
            return ptr = newblock;
        else
            return NULL;
    }

    T *Free(DWORD dwFlags = 0)
    {
        if ((this == NULL) || (ptr == NULL))
            return NULL;
        else if (HeapFree(GetProcessHeap(), dwFlags, ptr))
            return ptr = NULL;
        else
#pragma warning(suppress: 6001)
            return ptr;
    }

    WHeapMem()
    {
    }

    explicit WHeapMem(SIZE_T dwAllocSize, DWORD dwFlags = 0)
        : WMemHolder<T>((T*)HeapAlloc(GetProcessHeap(), dwFlags, dwAllocSize))
    {
    }

    explicit WHeapMem(T *pBlk)
        : WMemHolder<T>(pBlk)
    {
    }

    ~WHeapMem()
    {
        Free();
    }
};

