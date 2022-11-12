// IStream.h

#ifndef __ISTREAM_H
#define __ISTREAM_H

#include "../Common/MyUnknown.h"
#include "../Common/Types.h"

#include "IDecl.h"

#define STREAM_INTERFACE_SUB(i, base, x) DECL_INTERFACE_SUB(i, base, 3, x)
#define STREAM_INTERFACE(i, x) STREAM_INTERFACE_SUB(i, IUnknown, x)

STREAM_INTERFACE(ISequentialInStream, 0x01)
{
  STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize) PURE;
  /*
  Out: if size != 0, return_value = S_OK and (*processedSize == 0),
    then there are no more bytes in stream.
  if (size > 0) && there are bytes in stream,
  this function must read at least 1 byte.
  This function is allowed to read less than number of remaining bytes in stream.
  You must call Read function in loop, if you need exact amount of data
  */
};

STREAM_INTERFACE(ISequentialOutStream, 0x02)
{
  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize) PURE;
  /*
  if (size > 0) this function must write at least 1 byte.
  This function is allowed to write less than "size".
  You must call Write function in loop, if you need to write exact amount of data
  */
};

STREAM_INTERFACE_SUB(IInStream, ISequentialInStream, 0x03)
{
  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) PURE;
};

STREAM_INTERFACE_SUB(IOutStream, ISequentialOutStream, 0x04)
{
  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) PURE;
  STDMETHOD(SetSize)(UInt64 newSize) PURE;
};

STREAM_INTERFACE(IStreamGetSize, 0x06)
{
  STDMETHOD(GetSize)(UInt64 *size) PURE;
};

STREAM_INTERFACE(IOutStreamFlush, 0x07)
{
  STDMETHOD(Flush)() PURE;
};

#endif
