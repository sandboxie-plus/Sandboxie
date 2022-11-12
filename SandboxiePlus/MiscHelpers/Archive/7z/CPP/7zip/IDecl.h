// IDecl.h

#ifndef __IDECL_H
#define __IDECL_H

#include "../Common/MyUnknown.h"

#define DECL_INTERFACE_SUB(i, base, groupId, subId) \
DEFINE_GUID(IID_ ## i, \
0x23170F69, 0x40C1, 0x278A, 0, 0, 0, (groupId), 0, (subId), 0, 0); \
struct i: public base

#define DECL_INTERFACE(i, groupId, subId) DECL_INTERFACE_SUB(i, IUnknown, groupId, subId)

#endif
