/*
 * Copyright 2021 David Xanatos, xanasoft.com
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "map.h"

struct map_node_t {
  unsigned int hash;
  void *value;
  map_node_t *next;
  char key[0];
  /* char value[]; */
};


#ifdef WITHOUT_POOL
VOID* map_alloc(void* pool, size_t size)
{
    return malloc(size);
}

VOID map_free(void* pool, void* ptr)
{
    free(ptr);
}
#else
VOID* map_alloc(void* pool, size_t size)
{
    if (!pool) return NULL;
#ifdef KERNEL_MODE
	size_t* base = Mem_Alloc(pool, sizeof(size_t) + size);
#else
    size_t* base = (size_t*)Pool_Alloc((struct POOL*)pool, sizeof(size_t) + size);
#endif
    *base++ = size;
    return base;
}

VOID map_free(void* pool, void* ptr)
{
    size_t* base = (size_t*)ptr;
    size_t size = *--base;
#ifdef KERNEL_MODE
    Mem_Free(base, size);
#else
	Pool_Free(base, size);
#endif
}
#endif


static unsigned int map_hash(const void* key, size_t size) 
{
    /*const int MULTIPLIER = 37;
    unsigned int h = 0;
    for (unsigned char* p = (unsigned char*)key; p < ((unsigned char*)key) + size; p++)
        h = MULTIPLIER * h + *p;
    return h;*/
    unsigned int hash = 5381;
    for (unsigned char* ptr = (unsigned char*)key; ptr < ((unsigned char*)key) + size; ptr++)
        hash = ((hash << 5) + hash) ^ *ptr;
    return hash;
}


int map_wcssize(const void* key)
{
    return (wcslen((const wchar_t*)key) + 1) * sizeof(WCHAR);
}


BOOLEAN map_wcsimatch(const void* key1, const void* key2)
{
    return (_wcsicmp((const wchar_t*)key1, (const wchar_t*)key2) == 0);
}


void map_init(map_base_t* m, void* pool) 
{
    memset(m, 0, sizeof(map_base_t));
    m->mem_pool = pool;
    m->func_malloc = &map_alloc;
    m->func_free = &map_free;
    m->func_hash_key = &map_hash;
    m->func_key_size = NULL; // use key by value not by reference
    m->func_match_key = NULL; // use memcmp by default
}


static int map_bucket_idx(map_base_t* m, unsigned int hash)
{
    // Note: this works only with bucket sizes which are a power of 2
    //          this can be generalized by using % instead of &
    return hash & (m->nbuckets - 1);
}


static void map_add_node(map_base_t* m, map_node_t* node, BOOLEAN append) 
{
    int n = map_bucket_idx(m, node->hash);
	map_node_t** ptr = &m->buckets[n];
	if(append)
		while(*ptr) ptr = &(*ptr)->next;
	node->next = *ptr;
	*ptr = node;
}


BOOLEAN map_resize(map_base_t* m, int nbuckets) 
{
    map_node_t** buckets = (map_node_t**)m->func_malloc(m->mem_pool, sizeof(*m->buckets) * nbuckets);
    if (!buckets) return FALSE;
    memset(buckets, 0, sizeof(*m->buckets) * nbuckets);

    map_node_t* nodes = NULL;
    // store all old entries
    for (unsigned int i = m->nbuckets; i--; ) {
        map_node_t* node = (m->buckets)[i];
        while (node) {
            map_node_t* next = node->next;
            node->next = nodes;
            nodes = node;
            node = next;
        }
    }

    // swap bucket array
    if (m->buckets) m->func_free(m->mem_pool, m->buckets);
    m->buckets = buckets;
    m->nbuckets = nbuckets;

    // re insert all entries from our temporary list back into our buckets
    map_node_t* node = nodes;
    while (node) {
        map_node_t* next = node->next;
        map_add_node(m, node, FALSE);
        node = next;
    }
    
    return TRUE;
}


static map_node_t* map_new_node(map_base_t* m, const void* _key, void* vdata, size_t vsize) 
{
    const void* key;
    int ksize;
    if (m->func_key_size) {
        key = _key;
        ksize = m->func_key_size(key);
    }
    else {
        key = &_key;
        ksize = sizeof(void*);
    }
    unsigned int hash = m->func_hash_key(key, ksize);

    int voffset = ksize + ((sizeof(void*) - ksize) % sizeof(void*));
    map_node_t* node = (map_node_t*)m->func_malloc(m->mem_pool, sizeof(*node) + voffset + vsize);
    if (!node) return NULL;

    node->hash = hash;
    memcpy(node->key, key, ksize);
    node->next = NULL;
    if (vsize) {
        node->value = node->key + voffset;
        if (vdata) 
            memcpy(node->value, vdata, vsize);
        else
            memset(node->value, 0, vsize);
    }
    else
        node->value = vdata;
    return node;
}


void* map_add(map_base_t* m, const void* key, void* vdata, size_t vsize, BOOLEAN append)
{
    // create a new node and fill inn all the blanks
    map_node_t* node = map_new_node(m, key, vdata, vsize);
    if(!node)  goto fail;

	{
    // check and if we need to grow our bucker array
    //const int reduce_buckets = 0; // average 1.5 nodes/bucket and 49% of buckets used
    const int reduce_buckets = 1; // average 1.9 nodes/bucket and 75% of buckets used
    //const int reduce_buckets = 2; // average 3.2 nodes/bucket and 91% of buckets used
    //const int reduce_buckets = 3; // average 5.9 nodes/bucket and 98% of buckets used
    //const int reduce_buckets = 4; // average 11.8 nodes/bucket and 100% of buckets used
    if ((m->nnodes >> reduce_buckets) >= m->nbuckets) {
        int nbuckets = (m->nbuckets > 0) ? (m->nbuckets << 1) : 1; // *2
        if (!map_resize(m, nbuckets)) goto fail;
    }
	}

    // add new entry to the right bucket
    map_add_node(m, node, append);
    m->nnodes++;

    return node->value;
fail:
    if (node) m->func_free(m->mem_pool, node);
    return NULL;
}


static map_node_t** map_getmatch(map_base_t* m, map_node_t** next, unsigned int hash, const void* key, int ksize) 
{
	while (*next) {
        if ((*next)->hash == hash && (m->func_match_key
            ? m->func_match_key((*next)->key, key)
            : (memcmp((*next)->key, key, ksize) == 0)
            )) {
            return next;
        }
        next = &(*next)->next;
    }
    return NULL;
}


static map_node_t** map_getref(map_base_t* m, const void* _key) 
{
    if (!m->buckets) return NULL;

    const void* key;
    int ksize;
    if (m->func_key_size) {
        key = _key;
        ksize = m->func_key_size(key);
    }
    else {
        key = &_key;
        ksize = sizeof(void*);
    }
    unsigned int hash = m->func_hash_key(key, ksize);

    map_node_t** next = &m->buckets[map_bucket_idx(m, hash)];
	return map_getmatch(m, next, hash, key, ksize);
}


void* map_get(map_base_t* m, const void* key) 
{
    map_node_t** node = map_getref(m, key);
    return node ? (*node)->value : NULL;
}


BOOLEAN map_take(map_base_t* m, const void* key, void* vdata, size_t vsize)
{
    map_node_t** next = map_getref(m, key);
    if (next) {
        map_node_t* node = *next;
        *next = (*next)->next;
        if (vdata) {
            if (vsize) // by value
                memcpy(vdata, node->value, vsize);
            else // by extern pointer
                *((UINT_PTR*)vdata) = (UINT_PTR)node->value;
        }
        m->func_free(m->mem_pool, node);
        m->nnodes--;
        return TRUE;
    }
    if(vdata && !vsize)
        *((UINT_PTR*)vdata) = 0;
    return FALSE;
}


void map_clear(map_base_t* m) 
{
    for (unsigned int i = m->nbuckets; i--; ) {
        map_node_t* node = m->buckets[i];
        while (node) {
            map_node_t* next = node->next;
            m->func_free(m->mem_pool, node);
            node = next;
        }
    }
    if (m->buckets) {
        m->func_free(m->mem_pool, m->buckets);
        m->buckets = NULL;
    }
    m->nnodes = m->nbuckets = 0;
}


map_iter_t map_iter()
{
    map_iter_t iter;
    memset(&iter, 0, sizeof(map_iter_t));
    iter.bucketIdx = -1;
    return iter;
}


map_iter_t map_key_iter(map_base_t *m, const void* key)
{
    map_iter_t iter;
    memset(&iter, 0, sizeof(map_iter_t));
    iter.bucketIdx = -1;
	if(key) {
		iter.ksize = m->func_key_size ? m->func_key_size(key) : sizeof(void*);
		iter.key = key;
	}
    return iter;
}


BOOLEAN map_next(map_base_t* m, map_iter_t* iter) 
{
    if (iter->node) {
		map_node_t** next = &iter->node->next;
		if(iter->ksize){
			// note: here key must always be pointer to key value
			next = map_getmatch(m, next, iter->node->hash, iter->key, iter->ksize); 
			if(!next) return FALSE;
		}
        iter->node = *next;
        if (iter->node == NULL) goto nextBucket;
    }
    else { // first tun
		if(iter->ksize){
			map_node_t** node = map_getref(m, iter->key);
			if(!node) return FALSE;
			iter->node = *node;
		} else
    nextBucket:
        do {
            if (++iter->bucketIdx >= m->nbuckets) 
                return FALSE;
            iter->node = m->buckets[iter->bucketIdx];
        } while (iter->node == NULL);
    }
	iter->key = iter->node->key;
    iter->value = iter->node->value;
    return TRUE;
}



/*void map_dump(map_base_t *m)
{
	int used = 0;
	int empty = 0;
	int bucketIdx  = -1;
	//printf("start\r\n");
	while(++bucketIdx < (int)m->nbuckets) {
		map_node_t* bucket = m->buckets[bucketIdx];
		if (bucket) {
			used++;
			//printf("bucket %d\r\n", bucketIdx);
			//for(map_node_t* node = bucket; node != NULL; node = node->next) {
			//	printf("node %d: %d\r\n", node->hash, (int)node->value);
			//}
		} else {
			empty++;
			//printf("EMPTY bucket %d\r\n", bucketIdx);
		}
	}
#ifdef KERNEL_MODE
    DbgPrint("usage %d/%d (%d%%)\r\n", used, used+empty, 100*used/(used+empty));
#else
	printf("\r\n\r\nBucket usage %d/%d (%d%%), average nodes per used bucket %.2f\r\n", used, used+empty, 100*used/(used+empty), (double)m->nnodes/used);
#endif
}*/

BOOLEAN str_map_match(const void* key1, const void* key2) {
	const wchar_t** str1 = (const wchar_t**)key1;
	const wchar_t** str2 = (const wchar_t**)key2;
	return _wcsicmp(*str1, *str2) == 0;
}

unsigned int str_map_hash(const void* key, size_t size) {
	const wchar_t** str = (const wchar_t**)key;
	unsigned int hash = 5381;
	for (unsigned short* ptr = (unsigned short*)*str; *ptr != 0; ptr++)
		hash = ((hash << 5) + hash) ^ *ptr;
	return hash;
}