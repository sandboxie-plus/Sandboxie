// Interface/IProgress.h

#ifndef __IPROGRESS_H
#define __IPROGRESS_H

#include "../Common/MyUnknown.h"
#include "../Common/Types.h"

#include "IDecl.h"

#define INTERFACE_IProgress(x) \
  STDMETHOD(SetTotal)(UInt64 total) x; \
  STDMETHOD(SetCompleted)(const UInt64 *completeValue) x; \

DECL_INTERFACE(IProgress, 0, 5)
{
  INTERFACE_IProgress(PURE)
};

/*
// {23170F69-40C1-278A-0000-000000050002}
DEFINE_GUID(IID_IProgress2,
0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x02);
MIDL_INTERFACE("23170F69-40C1-278A-0000-000000050002")
IProgress2: public IUnknown
{
public:
  STDMETHOD(SetTotal)(const UInt64 *total) PURE;
  STDMETHOD(SetCompleted)(const UInt64 *completeValue) PURE;
};
*/

#endif
