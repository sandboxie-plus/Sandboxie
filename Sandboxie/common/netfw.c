
#include "common/netfw.h"
#include "common/defines.h"
#include "common/rbtree.h"
#include "common/my_wsa.h"
#include "common/str_util.h"

struct _NETFW_RULE 
{
    LIST_ELEM list_elem;

	POOL* pool;

	BOOLEAN action_block;

	int proc_match_level;

	rbtree_t port_map;
	rbtree_t ip_map;

	int protocol;
};

static int NetFw_PortCmp(const void * l, const void * r)
{
	if (*((USHORT*)l) > *((USHORT*)r))
		return 1;
	if (*((USHORT*)l) < *((USHORT*)r))
		return -1;
	return 0;
}

static int NetFw_IpCmp(const void * l, const void * r)
{
	IP_ADDRESS* L = (IP_ADDRESS*)l;
	IP_ADDRESS* R = (IP_ADDRESS*)r;
	/*if (L->Type != R->Type)
		return L->Type > R->Type ? 1 : -1;
	return memcmp(L->Data, R->Data, L->Type == AF_INET6 ? 16: 4);*/
	return memcmp(L->Data, R->Data, 16);
}

#define NETFW_MATCH_NONE	0
#define NETFW_MATCH_GLOBAL	1
#define NETFW_MATCH_NOT		2
#define NETFW_MATCH_RANGE	3
#define NETFW_MATCH_EXACT	4

NETFW_RULE* NetFw_AllocRule(POOL* pool, int MatchLevel)
{
#ifdef KERNEL_MODE
	NETFW_RULE* rule = ExAllocatePoolWithTag(NonPagedPool, sizeof(NETFW_RULE), tzuk);
#else
    NETFW_RULE* rule = Pool_Alloc(pool, sizeof(NETFW_RULE));
#endif
	if (rule == NULL)
		return NULL;

    memzero(&rule->list_elem, sizeof(LIST_ELEM));
	rule->pool = pool;

	rule->action_block = FALSE;

	//rule->proc_match_level = MatchLevel;
    // convert levels, todo: unify levels
    switch (MatchLevel) {
    case 0: rule->proc_match_level = NETFW_MATCH_EXACT; break;
    case 1: rule->proc_match_level = NETFW_MATCH_NOT; break;
    case 2: rule->proc_match_level = NETFW_MATCH_GLOBAL; break;
    default: rule->proc_match_level = NETFW_MATCH_NONE; break;
    }

	rbtree_init(&rule->port_map, NetFw_PortCmp);
	rbtree_init(&rule->ip_map, NetFw_IpCmp);
	rule->protocol = IPPROTO_ANY;

	return rule;
}

void NetFw_RuleSetBlockAction(NETFW_RULE* rule, BOOLEAN BlockAction)
{
	rule->action_block = BlockAction;
}

// Port ranges

typedef struct _NETFW_PORTS
{
    rbnode_t tree_elem;

	USHORT RangeBegin;
	USHORT RangeEnd;
} NETFW_PORTS;

void NetFw_RuleAddPortRange(rbtree_t* tree, USHORT PortBegin, USHORT PortEnd, POOL* pool)
{
#ifdef KERNEL_MODE
	NETFW_PORTS* node = ExAllocatePoolWithTag(NonPagedPool, sizeof(NETFW_PORTS), tzuk);
#else
	NETFW_PORTS* node = Pool_Alloc(pool, sizeof(NETFW_PORTS));
#endif
	node->tree_elem.key = &node->RangeBegin;
	node->RangeBegin = PortBegin;
	node->RangeEnd = PortEnd;
	rbtree_insert(tree, (rbnode_t*)node);
}

static void NetFw_FreePort(rbnode_t* node, void* arg) 
{
#ifdef KERNEL_MODE
	ExFreePoolWithTag(node, tzuk);
#else
	Pool_Free(node, sizeof(NETFW_PORTS));
#endif
}

BOOLEAN NetFw_MatchPortMaps(rbtree_t* l_tree, rbtree_t* r_tree)
{
	NETFW_PORTS* l_node = (NETFW_PORTS*)rbtree_first(l_tree);
	NETFW_PORTS* r_node = (NETFW_PORTS*)rbtree_first(r_tree);
	while (1) {
		if ((((rbnode_t*)l_node) != RBTREE_NULL) != (((rbnode_t*)r_node) != RBTREE_NULL))
			break; // no match
		if ((((rbnode_t*)l_node) == RBTREE_NULL) && (((rbnode_t*)r_node) == RBTREE_NULL))
			return TRUE;
		if (NetFw_PortCmp(&l_node->RangeBegin, &r_node->RangeBegin) != 0 || NetFw_PortCmp(&l_node->RangeEnd, &r_node->RangeEnd) != 0)
			break; // no match

		l_node = (NETFW_PORTS*)rbtree_next(((rbnode_t*)l_node));
		r_node = (NETFW_PORTS*)rbtree_next(((rbnode_t*)r_node));
	}
	return FALSE;
}

BOOLEAN NetFw_MergePortMaps(rbtree_t* dst, rbtree_t* src, POOL* pool)
{
	//
	// search for overlaps, and if found abort
	// we merge only non overlapping ranges as single entries vs ranges have a different priority
	//

	for (NETFW_PORTS* src_node = (NETFW_PORTS*)rbtree_first(src); ((rbnode_t*)src_node) != RBTREE_NULL; src_node = (NETFW_PORTS*)rbtree_next((rbnode_t*)src_node)) {
		
		NETFW_PORTS* dst_node = NULL;
		rbtree_find_less_equal(dst, &src_node->RangeBegin, (rbnode_t**)&dst_node);
		if(dst_node && NetFw_PortCmp(&dst_node->RangeEnd, &src_node->RangeEnd) >= 0) // found overlap
			return FALSE;
	}

	for (NETFW_PORTS* src_node = (NETFW_PORTS*)rbtree_first(src); ((rbnode_t*)src_node) != RBTREE_NULL; src_node = (NETFW_PORTS*)rbtree_next((rbnode_t*)src_node)) {
		
		NetFw_RuleAddPortRange(dst, src_node->RangeBegin, src_node->RangeEnd, pool);
	}
	return TRUE;
}

ULONG NetFw_MatchPort(rbtree_t* port_map, USHORT port)
{
	if (port_map->count == 0)
		return NETFW_MATCH_GLOBAL;

	NETFW_PORTS* node = NULL;
	rbtree_find_less_equal(port_map, &port, (rbnode_t**)&node);
	if (node == NULL)
		return NETFW_MATCH_NONE;
	if (NetFw_PortCmp(&port, &node->RangeBegin) < 0 || NetFw_PortCmp(&node->RangeEnd, &port) < 0)
		return NETFW_MATCH_NONE;
	return NetFw_PortCmp(&node->RangeBegin, &node->RangeEnd) == 0 ? NETFW_MATCH_EXACT : NETFW_MATCH_RANGE;
}

//

// IP ranges

typedef struct _NETFW_IPS
{
    rbnode_t tree_elem;

	IP_ADDRESS RangeBegin;
	IP_ADDRESS RangeEnd;
} NETFW_IPS;

void NetFw_RuleAddIpRange(rbtree_t* tree, IP_ADDRESS* IpBegin, IP_ADDRESS* IpEnd, POOL* pool)
{
#ifdef KERNEL_MODE
	NETFW_IPS* node = ExAllocatePoolWithTag(NonPagedPool, sizeof(NETFW_IPS), tzuk);
#else
	NETFW_IPS* node = Pool_Alloc(pool, sizeof(NETFW_IPS));
#endif
	node->tree_elem.key = &node->RangeBegin;
	node->RangeBegin = *IpBegin;
	node->RangeEnd = *IpEnd;
	rbtree_insert(tree, (rbnode_t*)node);
}

static void NetFw_FreeIp(rbnode_t* node, void* arg) 
{
#ifdef KERNEL_MODE
	ExFreePoolWithTag(node, tzuk);
#else
	Pool_Free(node, sizeof(NETFW_IPS));
#endif
}

BOOLEAN NetFw_MatchIPMaps(rbtree_t* l_tree, rbtree_t* r_tree)
{
	NETFW_IPS* l_node = (NETFW_IPS*)rbtree_first(l_tree);
	NETFW_IPS* r_node = (NETFW_IPS*)rbtree_first(r_tree);
	while (1) {
		if ((((rbnode_t*)l_node) != RBTREE_NULL) != (((rbnode_t*)r_node) != RBTREE_NULL))
			break; // no match
		if ((((rbnode_t*)l_node) == RBTREE_NULL) && (((rbnode_t*)r_node) == RBTREE_NULL))
			return TRUE;
		if (NetFw_IpCmp(&l_node->RangeBegin, &r_node->RangeBegin) != 0 || NetFw_IpCmp(&l_node->RangeEnd, &r_node->RangeEnd) != 0)
			break; // no match

		l_node = (NETFW_IPS*)rbtree_next((rbnode_t*)l_node);
		r_node = (NETFW_IPS*)rbtree_next((rbnode_t*)r_node);
	}
	return FALSE;
}

BOOLEAN NetFw_MergeIPMaps(rbtree_t* dst, rbtree_t* src, POOL* pool)
{
	//
	// search for overlaps, and if found abort
	// we merge only non overlapping ranges as single entries vs ranges have a different priority
	//

	for (NETFW_IPS* src_node = (NETFW_IPS*)rbtree_first(src); ((rbnode_t*)src_node) != RBTREE_NULL; src_node = (NETFW_IPS*)rbtree_next((rbnode_t*)src_node)) {
		
		NETFW_IPS* dst_node = NULL;
		rbtree_find_less_equal(dst, &src_node->RangeBegin, (rbnode_t**)&dst_node);
		if(dst_node && NetFw_IpCmp(&dst_node->RangeEnd, &src_node->RangeEnd) >= 0) // found overlap
			return FALSE;
	}

	for (NETFW_IPS* src_node = (NETFW_IPS*)rbtree_first(src); ((rbnode_t*)src_node) != RBTREE_NULL; src_node = (NETFW_IPS*)rbtree_next((rbnode_t*)src_node)) {
		
		NetFw_RuleAddIpRange(dst, &src_node->RangeBegin, &src_node->RangeEnd, pool);
	}
	return TRUE;
}

ULONG NetFw_MatchAddress(rbtree_t* ip_map, IP_ADDRESS* ip)
{
	if (ip_map->count == 0)
		return NETFW_MATCH_GLOBAL;

	NETFW_IPS* node = NULL;
	rbtree_find_less_equal(ip_map, ip, (rbnode_t**)&node);
	if (node == NULL)
		return NETFW_MATCH_NONE;
	if (NetFw_IpCmp(ip, &node->RangeBegin) < 0 || NetFw_IpCmp(&node->RangeEnd, ip) < 0)
		return NETFW_MATCH_NONE;
	return NetFw_IpCmp(&node->RangeBegin, &node->RangeEnd) == 0 ? NETFW_MATCH_EXACT : NETFW_MATCH_RANGE;
}

//

void NetFw_RuleSetProtocol(NETFW_RULE* rule, int Protocol)
{
	rule->protocol = Protocol;
}

ULONG NetFw_MatchProtocol(int protocol, int to_test)
{
	if (protocol == IPPROTO_ANY)
		return NETFW_MATCH_GLOBAL;

	if (protocol == to_test)
		return NETFW_MATCH_EXACT;
	return NETFW_MATCH_NONE;
}

void NetFw_FreeRule(NETFW_RULE* rule)
{
	traverse_postorder(&rule->port_map, NetFw_FreePort, NULL);
	traverse_postorder(&rule->ip_map, NetFw_FreeIp, NULL);
#ifdef KERNEL_MODE
	ExFreePoolWithTag(rule, tzuk);
#else
	Pool_Free(rule, sizeof(NETFW_RULE));
#endif
}

void NetFw_AddRule(LIST* list, NETFW_RULE* new_rule)
{
	NETFW_RULE* rule = List_Head(list);
    while (rule) 
    {
		if (rule->proc_match_level != new_rule->proc_match_level || rule->action_block != new_rule->action_block)
			goto next; // must be same level and same action

		if ((rule->port_map.count != 0) != (new_rule->port_map.count != 0))
			goto next; // booth must, or must not, have Ports
		
		if ((rule->ip_map.count != 0) != (new_rule->ip_map.count != 0))
			goto next; // booth must, or must not, have IPs

		if (rule->protocol != new_rule->protocol)
			goto next; // must be same protocol

		//
		// it seems we might be able to merge these rules
		// now we check the convoluted case when rules havs ip's and port's set
		//

		if ((rule->port_map.count != 0) && (rule->ip_map.count != 0)){
			
			BOOLEAN same_ports = NetFw_MatchPortMaps(&rule->port_map, &new_rule->port_map);
			BOOLEAN same_ips = NetFw_MatchIPMaps(&rule->ip_map, &new_rule->ip_map);
			if (!same_ports && !same_ips) { // if neider Ports nor IP's are same 
				goto next; // we don't merge
			}
			else if (!same_ports) {
				if (!NetFw_MergePortMaps(&rule->port_map, &new_rule->port_map, rule->pool))
					goto next; // merge failed
			}
			else if (!same_ips) {
				if (!NetFw_MergeIPMaps(&rule->ip_map, &new_rule->ip_map, rule->pool))
					goto next; // merge failed
			}
			
		}
		// if we are here it means that booth rules heve eider only ports or only IP's set
		else if (rule->port_map.count != 0) {
			if (!NetFw_MergePortMaps(&rule->port_map, &new_rule->port_map, rule->pool))
				goto next; // merge failed
		}
		else if(rule->ip_map.count != 0) {
			if (!NetFw_MergeIPMaps(&rule->ip_map, &new_rule->ip_map, rule->pool))
				goto next; // merge failed
		}

		//
		// if we are here we eider merged the rules or the rules are identical
		//

		NetFw_FreeRule(new_rule);
		return;

	next:
        rule = List_Next(rule);
    }

	List_Insert_After(list, NULL, new_rule);
}

typedef struct _RULE_MATCH {
	ULONG ByProg;
	ULONG ByPort;
	ULONG ByAddress;
	ULONG ByEndPoint;
	ULONG ByProtocol;
	BOOLEAN BlockAction;
} RULE_MATCH;

BOOLEAN NetFw_MatchRule(NETFW_RULE* rule, USHORT TestPort, IP_ADDRESS* TestAddress, int TestProt, RULE_MATCH* Match) 
{
	Match->ByProg = rule->proc_match_level;

	if (!(Match->ByPort = NetFw_MatchPort(&rule->port_map, TestPort)))
		return FALSE;
	if (!(Match->ByAddress = NetFw_MatchAddress(&rule->ip_map, TestAddress)))
		return FALSE;
	if (!(Match->ByProtocol = NetFw_MatchProtocol(rule->protocol, TestProt)))
		return FALSE;

	if (Match->ByAddress > NETFW_MATCH_GLOBAL && Match->ByPort > NETFW_MATCH_GLOBAL)
		Match->ByEndPoint = Match->ByAddress > Match->ByPort ? Match->ByAddress : Match->ByPort; // max

	Match->BlockAction = rule->action_block;

	return TRUE;
}


#define COMPARE_AND_RETURN(x, y) if(x != y) return x > y

BOOLEAN NetFw_IsBetterMatch(RULE_MATCH* MyMatch, RULE_MATCH* OtherMatch)
{
	// 1. A rule for a specified program trumps a rule for all programs except a given one, trumps a rule for all programs
	COMPARE_AND_RETURN(MyMatch->ByProg, OtherMatch->ByProg);
		
	// 2. a rule with a Port or IP trumps a rule without
	// 2a. a rule with ip and port trums a rule with ip or port only
	// 2b. a rule with one ip trumps a rule with an ip range that is besides that on the same level
	COMPARE_AND_RETURN(MyMatch->ByEndPoint, OtherMatch->ByEndPoint);
	COMPARE_AND_RETURN(MyMatch->ByPort, OtherMatch->ByPort);
	COMPARE_AND_RETURN(MyMatch->ByAddress, OtherMatch->ByAddress);

	// 3. block rules trump allow rules
	if(MyMatch->BlockAction == TRUE && OtherMatch->BlockAction != TRUE)
		return TRUE;
		
	// 4-> a rule without a protocol means all protocols, a rule with a protocol trumps a rule without if its the only difference
	COMPARE_AND_RETURN(MyMatch->ByProtocol, OtherMatch->ByProtocol);

	return FALSE;
}

BOOLEAN NetFw_BlockTraffic(LIST* list, IP_ADDRESS* Ip, USHORT Port, int Protocol)
{
	NETFW_RULE* best_rule = NULL;
	RULE_MATCH best_match = { 0 };

	NETFW_RULE* rule = List_Head(list);
    while (rule) 
    {
		RULE_MATCH match = { 0 };
		if (NetFw_MatchRule(rule, Port, Ip, Protocol, &match))
		{
			if (!best_rule || NetFw_IsBetterMatch(&match, &best_match)) {
				best_rule = rule;
				best_match = match;
			}
		}

        rule = List_Next(rule);
    }

	if (best_rule && best_rule->action_block)
		return TRUE;
	return FALSE;
}

// text helpers

const WCHAR* wcsnchr(const WCHAR* str, size_t max, WCHAR ch)
{
    for (;*str != 0 && max > 0;str++, max--) {
        if (*str == ch)
            return str;
    }
    return NULL;
}

int _inet_pton(int af, const wchar_t* src, void* dst);

int _inet_xton(const WCHAR* src, ULONG max, IP_ADDRESS *dst)
{
	WCHAR tmp[46]; // INET6_ADDRSTRLEN 
	wmemcpy(tmp, src, max);
	
    //dst->Type = AF_INET;
    //if (wcschr(src, L':') != NULL)
    //    dst->Type = AF_INET6;

    int ret = _inet_pton(wcschr(src, L':') != NULL ? AF_INET6 : AF_INET, tmp, dst->Data);
    return ret;
}

int _wntoi(const WCHAR* str, ULONG max) 
{
	WCHAR tmp[12];
	wmemcpy(tmp, str, max);
    return _wtoi(tmp);
}

BOOLEAN NetFw_ParseRule(NETFW_RULE* rule, const WCHAR* found_value)
{
	//NetworkAccess=explorer.exe,Allow;Port=137,138,139,445;Address=192.168.0.1-192.168.100.255;Protocol=TCP;

    WCHAR* action_value = NULL;
    ULONG action_len = 0;
    const WCHAR* tag_list = SbieDll_GetTagValue(found_value, NULL, &action_value, &action_len, L';');

	NetFw_RuleSetBlockAction(rule, _wcsnicmp(action_value, L"Allow", action_len) != 0);

    const WCHAR* port_value;
    ULONG port_len;
    if (SbieDll_FindTagValuePtr(tag_list, L"Port", &port_value, &port_len, L'=', L';')) {
        for (const WCHAR* port_end = port_value + port_len; port_value < port_end;) {
            const WCHAR* port_str1;
            ULONG port_len1;
            port_value = SbieDll_GetTagValue(port_value, port_end, &port_str1, &port_len1, L',');

            const WCHAR* port_str2 = wcsnchr(port_str1, port_len1, L'-');
            if (port_str2) {
                port_len1 = (ULONG)(port_str2 - port_str1);
                port_str2++; // skip dash
                ULONG port_len2 = (ULONG)(port_value - port_str2);

                USHORT Port1 = (USHORT)_wntoi(port_str1, port_len1);
                USHORT Port2 = (USHORT)_wntoi(port_str2, port_len2);
                NetFw_RuleAddPortRange(&rule->port_map, Port1, Port2, rule->pool);
            }
            else
            {
                USHORT Port = (USHORT)_wntoi(port_str1, port_len1);
                NetFw_RuleAddPortRange(&rule->port_map, Port, Port, rule->pool);
            }
        }
    }

    const WCHAR* ip_value;
    ULONG ip_len;
    if (SbieDll_FindTagValuePtr(tag_list, L"Address", &ip_value, &ip_len, L'=', L';')) {
        for (const WCHAR* ip_end = ip_value + ip_len; ip_value < ip_end;) {
            const WCHAR* ip_str1;
            ULONG ip_len1;
            ip_value = SbieDll_GetTagValue(ip_value, ip_end, &ip_str1, &ip_len1, L',');

            const WCHAR* ip_str2 = wcsnchr(ip_str1, ip_len1, L'-');
            if (ip_str2) {
                ip_len1 = (ULONG)(ip_str2 - ip_str1);
                ip_str2++; // skip dash
                ULONG ip_len2 = (ULONG)(ip_value - ip_str2);

                IP_ADDRESS ip1;
                _inet_xton(ip_str1, ip_len1, &ip1);
                IP_ADDRESS ip2;
                _inet_xton(ip_str2, ip_len2, &ip2);

                NetFw_RuleAddIpRange(&rule->ip_map, &ip1, &ip2, rule->pool);
            }
            else
            {
                IP_ADDRESS ip;
                _inet_xton(ip_str1, ip_len1, &ip);
                NetFw_RuleAddIpRange(&rule->ip_map, &ip, &ip, rule->pool);
            }
        }
    }

    WCHAR* prot_value;
    ULONG prot_len;
    if (SbieDll_FindTagValuePtr(tag_list, L"Protocol", &prot_value, &prot_len, L'=', L';')) {
        if (_wcsnicmp(prot_value, L"TCP", prot_len) == 0)
            NetFw_RuleSetProtocol(rule, IPPROTO_TCP);
        else if (_wcsnicmp(prot_value, L"UDP", prot_len) == 0)
            NetFw_RuleSetProtocol(rule, IPPROTO_UDP);
        else if (_wcsnicmp(prot_value, L"ICMP", prot_len) == 0)
            NetFw_RuleSetProtocol(rule, IPPROTO_ICMP);
    }

	return TRUE;
}



#include <inaddr.h>
#include <in6addr.h>

static int isdigit_(int c) { return (c >= '0' && c <= '9'); }
static int isxdigit_(int c) { return (isdigit_(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')); }
#undef isascii
static int isascii(int iChar) { return((iChar <= 127) && (iChar >= 0)); }
static int isalnum_(int c) { return (isdigit_(c) || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')); }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// from BSD sources: http://code.google.com/p/plan9front/source/browse/sys/src/ape/lib/bsd/?r=320990f52487ae84e28961517a4fa0d02d473bac

/*
wchar_t* _inet_ntop(int af, const void *src, wchar_t *dst, int size)
{
	unsigned char *p;
	wchar_t *t;
	int i;

	if(af == AF_INET){
		if(size < INET_ADDRSTRLEN){
			//errno = ENOSPC;
			return 0;
		}
		p = (unsigned char*)&(((struct in_addr*)src)->s_addr);
		wsprintf(dst, L"%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
		return dst;
	}

	if(af != AF_INET6){
		//errno = EAFNOSUPPORT;
		return 0;
	}
	if(size < INET6_ADDRSTRLEN){
		//errno = ENOSPC;
		return 0;
	}

	p = (unsigned char*)((struct in6_addr*)src)->s6_addr;
	t = dst;
	for(i=0; i<16; i += 2){
		unsigned int w;

		if(i > 0)
			*t++ = L':';
		w = p[i]<<8 | p[i+1];
		wsprintf(t, L"%x", w);
		t += strlen(t);
	}
	return dst;
}*/

//////////////////////////////////////////////////////////////////////////////////////

#define CLASS(x)        (x[0]>>6)

int _inet_aton(const wchar_t *from, struct in_addr *in)
{
	unsigned char *to;
	unsigned long x;
	wchar_t *p;
	int i;

	in->s_addr = 0;
	to = (unsigned char*)&in->s_addr;
	if(*from == 0)
		return 0;
	for(i = 0; i < 4 && *from; i++, from = p){
		x = wcstoul(from, &p, 0);
		if(x != (unsigned char)x || p == from)
			return 0;       /* parse error */
		to[i] = (unsigned char)x;
		if(*p == L'.')
			p++;
		else if(*p != 0)
			return 0;       /* parse error */
	}

	switch(CLASS(to)){
	case 0: /* class A - 1 byte net */
	case 1:
		if(i == 3){
			to[3] = to[2];
			to[2] = to[1];
			to[1] = 0;
		} else if (i == 2){
			to[3] = to[1];
			to[1] = 0;
		}
		break;
	case 2: /* class B - 2 byte net */
		if(i == 3){
			to[3] = to[2];
			to[2] = 0;
		}
		break;
	}
	return 1;
}

static int ipcharok(int c)
{
	return c == ':' || isascii(c) && isxdigit_(c);
}

static int delimchar(int c)
{
	if(c == '\0')
		return 1;
	if(c == ':' || isascii(c) && isalnum(c))
		return 0;
	return 1;
}

int _inet_pton(int af, const wchar_t *src, void *dst) // ip is always in network order !!!
{
	int i, ellipsis = 0;
	unsigned char *to;
	unsigned long x;
	const wchar_t *p, *op;

	if (af == AF_INET) {
		struct in_addr temp;
		int ret = _inet_aton(src, &temp);
		// IPv4-mapped IPv6 addresses, eg. ::FFFF:192.168.0.1
		ULONG* Data32 = (ULONG*)dst;
        Data32[0] = 0;
        Data32[1] = 0;
        Data32[2] = 0xFFFF0000;
		Data32[3] = temp.S_un.S_addr;
		return ret;
		//return _inet_aton(src, dst);
	}

	if(af != AF_INET6){
		//errno = EAFNOSUPPORT;
		return -1;
	}

	to = ((struct in6_addr*)dst)->s6_addr;
	memset(to, 0, 16);

	p = src;
	for(i = 0; i < 16 && ipcharok(*p); i+=2){
		op = p;
		x = wcstoul(p, (wchar_t**)&p, 16);

		if(x != (unsigned short)x || *p != L':' && !delimchar(*p))
			return 0;                       /* parse error */

		to[i] = (unsigned char)(x>>8);
		to[i+1] = (unsigned char)x;
		if(*p == L':'){
			if(*++p == L':'){        /* :: is elided zero short(s) */
				if (ellipsis)
					return 0;       /* second :: */
				ellipsis = i+2;
				p++;
			}
		} else if (p == op)             /* strtoul made no progress? */
			break;
	}
	if (p == src || !delimchar(*p))
		return 0;                               /* parse error */
	if(i < 16){
		memmove(&to[ellipsis+16-i], &to[ellipsis], i-ellipsis);
		memset(&to[ellipsis], 0, 16-i);
	}
	return 1;
}