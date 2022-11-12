// Common/Defs.h

#ifndef __COMMON_DEFS_H
#define __COMMON_DEFS_H

template <class T> inline T MyMin(T a, T b)
  {  return a < b ? a : b; }
template <class T> inline T MyMax(T a, T b)
  {  return a > b ? a : b; }

template <class T> inline int MyCompare(T a, T b)
  {  return a < b ? -1 : (a == b ? 0 : 1); }

inline int BoolToInt(bool value)
  { return (value ? 1: 0); }

inline bool IntToBool(int value)
  { return (value != 0); }

#endif
