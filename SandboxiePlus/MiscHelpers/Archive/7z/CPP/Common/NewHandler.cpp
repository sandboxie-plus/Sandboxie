// NewHandler.cpp
 
#include "StdAfx.h"

#include <stdlib.h>

#include "NewHandler.h"

// #define DEBUG_MEMORY_LEAK

#ifndef DEBUG_MEMORY_LEAK

#ifdef _WIN32
void *
#ifdef _MSC_VER
__cdecl
#endif
operator new(size_t size)
{
  // void *p = ::HeapAlloc(::GetProcessHeap(), 0, size);
  void *p = ::malloc(size);
  if (p == 0)
    throw CNewException();
  return p;
}

void
#ifdef _MSC_VER
__cdecl
#endif
operator delete(void *p) throw()
{
  /*
  if (p == 0)
    return;
  ::HeapFree(::GetProcessHeap(), 0, p);
  */
  ::free(p);
}
#endif

#else

#pragma init_seg(lib)
const int kDebugSize = 1000000;
static void *a[kDebugSize];
static int index = 0;

static int numAllocs = 0;
void * __cdecl operator new(size_t size)
{
  numAllocs++;
  void *p = HeapAlloc(GetProcessHeap(), 0, size);
  if (index == 40)
  {
    int t = 1;
  }
  if (index < kDebugSize)
  {
    a[index] = p;
    index++;
  }
  if (p == 0)
    throw CNewException();
  printf("Alloc %6d, size = %8d\n", numAllocs, size);
  return p;
}

class CC
{
public:
  CC()
  {
    for (int i = 0; i < kDebugSize; i++)
      a[i] = 0;
  }
  ~CC()
  {
    for (int i = 0; i < kDebugSize; i++)
      if (a[i] != 0)
        return;
  }
} g_CC;


void __cdecl operator delete(void *p)
{
  if (p == 0)
    return;
  /*
  for (int i = 0; i < index; i++)
    if (a[i] == p)
      a[i] = 0;
  */
  HeapFree(GetProcessHeap(), 0, p);
  numAllocs--;
  printf("Free %d\n", numAllocs);
}

#endif

/*
int MemErrorVC(size_t)
{
  throw CNewException();
  // return 1;
}
CNewHandlerSetter::CNewHandlerSetter()
{
  // MemErrorOldVCFunction = _set_new_handler(MemErrorVC);
}
CNewHandlerSetter::~CNewHandlerSetter()
{
  // _set_new_handler(MemErrorOldVCFunction);
}
*/
