#ifndef _DEFINES_H_
#define _DEFINES_H_

#ifdef IS_DRIVER
 #include <ntifs.h>
#endif

#ifdef _UEFI
 #include <Uefi.h>
#endif

#if !defined(IS_DRIVER) && !defined(BOOT_LDR) && !defined(_UEFI)
 #include <windows.h>
 #include <stdio.h>
#endif

#ifndef _WCHAR_T_DEFINED 
 typedef short wchar_t;
#endif

typedef unsigned __int64 u64;
typedef unsigned long    u32;
typedef unsigned short   u16;
typedef unsigned char    u8;

typedef __int64 s64;
typedef long    s32;
typedef short   s16;
typedef char    s8;

//#ifndef size_t
//#define size_t UINTN
//#endif

#define d8(_x)  ((u8)(_x))
#define d16(_x) ((u16)(_x))
#define d32(_x) ((u32)(_x))
#define d64(_x) ((u64)(_x))
#define dSZ(_x) ((size_t)(_x))

typedef void (*callback)(void*);
typedef void (*callback_ex)(void*,void*);

#define BE16(x) _byteswap_ushort(x)
#define BE32(x) _byteswap_ulong(x)
#define BE64(x) _byteswap_uint64(x)

#define ROR64(x,y)     (_rotr64((x),(y)))
#define ROL64(x,y)     (_rotl64((x),(y)))
#define ROL32(x,y)     (_rotl((x), (y)))
#define ROR32(x,y)     (_rotr((x), (y)))
#define bsf(x,y)       (_BitScanForward((x),(y)))
#define bsr(x,y)       (_BitScanReverse((x),(y)))

#define align16  __declspec(align(16))
#define naked    __declspec(naked)

#define p8(_x)   ((u8*)(_x))
#define p16(_x)  ((u16*)(_x))
#define p32(_x)  ((u32*)(_x))
#define p64(_x)  ((u64*)(_x))
#define pv(_x)   ((void*)(_x))
#define ppv(_x)  ((void**)(_x)) 

#define in_reg(a,base,size)     ( (a >= base) && (a < base+size)  )
#define is_intersect(start1, size1, start2, size2) ( max(start1, start2) < min(start1 + size1, start2 + size2) )
#define addof(a,o)              ( pv(p8(a)+o) )
#ifndef offsetof
#define offsetof(type,field)    ( d32(&(((type *)0)->field)) )
#endif

#ifdef BOOT_LDR
 #pragma warning(disable:4142)
 typedef unsigned long size_t;
 #pragma warning(default:4142)
#endif

#ifndef max
 #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
 #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _align
 #define _align(size, align) (((size) + ((align) - 1)) & ~((align) - 1))
#endif

#ifndef PAGE_SIZE
 #define PAGE_SIZE 0x1000
#endif

#ifndef bittest
#ifdef _M_IX86 
 #define bittest(a,b) ( _bittest(p32(&a),b) )
#else
 #define bittest(a,b) ( sizeof(a) == sizeof(u32) ? _bittest(p32(&a),b):_bittest64(p64(&a),b) )   
#endif /* _M_IX86 */
#endif /* bittest */

#ifndef NULL
 #define NULL pv(0)
#endif

#ifndef MAX_PATH
 #define MAX_PATH 260
#endif

#define sizeof_w(x)  ( sizeof(x) / sizeof(wchar_t) ) /* return number of wide characters in array */
#define array_num(x) ( sizeof(x) / sizeof((x)[0]) )  /* return number of elements in array */

#if 0
#define zeromem(m,s) memset(m, 0, s)

/* size optimized intrinsics */
#define mincpy(a,b,c) __movsb(pv(a), pv(b), (size_t)(c))
#define memset(a,b,c) __stosb(pv(a), (char)(b), (size_t)(c))

/* zeromem for 4byte aligned blocks */
#define zerofast(m,s) __stosd(pv(m),0,(size_t)(s) / 4)

/* fast intrinsics for memory copying and zeroing */
#ifdef _M_IX86 
 #define fastcpy(a,b,c) __movsd(pv(a), pv(b), (size_t)(c) / 4)

 #define autocpy(a,b,c) { \
    if (!((c) % 4)) { __movsd(pv(a), pv(b), (size_t)(c) / 4); } else \
    if (!((c) % 2)) { __movsw(pv(a), pv(b), (size_t)(c) / 2); } else \
    { __movsb(pv(a), pv(b), (size_t)(c)); } }
  
 #define zeroauto(m,s) { \
    if (!((s) % 4)) { __stosd(pv(m), 0, (size_t)(s) / 4); } else \
    if (!((s) % 2)) { __stosw(pv(m), 0, (size_t)(s) / 2); } else \
	{ __stosb(pv(m), 0, (size_t)(s)); } }

 #define _disable() { __asm { cli }; }
 #define _enable()  { __asm { sti }; }
#else
 #define fastcpy(a,b,c) __movsq(pv(a), pv(b), (size_t)(c) / 8)
 
 #define autocpy(a,b,c) { \
    if (!((c) % 8)) { __movsq(pv(a), pv(b), (size_t)(c) / 8); } else \
    if (!((c) % 4)) { __movsd(pv(a), pv(b), (size_t)(c) / 4); } else \
    if (!((c) % 2)) { __movsw(pv(a), pv(b), (size_t)(c) / 2); } else \
    { __movsb(pv(a), pv(b), (size_t)(c)); } }

 #define zeroauto(m,s) { \
    if (!((s) % 8)) { __stosq(pv(m), 0, (size_t)(s) / 8); } else \
    if (!((s) % 4)) { __stosd(pv(m), 0, (size_t)(s) / 4); } else \
    if (!((s) % 2)) { __stosw(pv(m), 0, (size_t)(s) / 2); } else \
	{ __stosb(pv(m), 0, (size_t)(s)); } }
#endif
#endif

#define lock_inc(_x)          ( _InterlockedIncrement(_x) )
#define lock_dec(_x)          ( _InterlockedDecrement(_x) )
#define lock_xchg(_p, _v)     ( _InterlockedExchange(_p, _v) )
#define lock_xchg_add(_p, _v) ( _InterlockedExchangeAdd(_p, _v) )

#ifndef _UEFI
#pragma warning(disable:4995)
#pragma intrinsic(memcpy,memset,memcmp)
#pragma intrinsic(strcpy,strcmp,strlen)
#pragma intrinsic(strcat)
#pragma warning(default:4995)
#endif

#endif
