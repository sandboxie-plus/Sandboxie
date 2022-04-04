#ifndef __MY_NETFW_H
#define __MY_NETFW_H


#include "common/pool.h"

typedef struct _NETFW_RULE NETFW_RULE;


typedef struct _IP_ADDRESS
{
	//UCHAR Type;
	union {
		UCHAR Data[16];
		ULONG Data32[4];
	};
} IP_ADDRESS;


NETFW_RULE* NetFw_AllocRule(POOL* pool, int MatchLevel);

void NetFw_AddRule(LIST* list, NETFW_RULE* rule);

BOOLEAN NetFw_BlockTraffic(LIST* list, IP_ADDRESS* Ip, USHORT Port, int Protocol);

BOOLEAN NetFw_ParseRule(NETFW_RULE* rule, const WCHAR* RuleStr);

void NetFw_FreeRule(NETFW_RULE* rule);

#endif