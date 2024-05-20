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


int _wntoi(const WCHAR* str, ULONG max);
int _inet_pton(int af, const wchar_t* src, void* dst);
int _inet_aton(const wchar_t* from, struct in_addr* in);
int _inet_xton(const WCHAR* src, ULONG max, IP_ADDRESS* dst, USHORT* type);
BOOLEAN is_localhost(const struct sockaddr* name);
BOOLEAN is_inet(const struct sockaddr* name);

#endif