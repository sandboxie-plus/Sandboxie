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

#ifndef MAP_H
#define MAP_H

#ifdef __cplusplus
extern "C" {
#endif

struct map_node_t;
typedef struct map_node_t map_node_t;

struct map_base_t;
typedef struct map_base_t map_base_t;

struct map_base_t {
  map_node_t **buckets;
  unsigned int nbuckets, nnodes;

  void* mem_pool;
  void*(*func_malloc)(void* pool, size_t size);
  void(*func_free)(void* pool, void* ptr);

  int(*func_key_size)(const void* key);
  unsigned int (*func_hash_key)(const void* key, size_t size);
  BOOLEAN (*func_match_key)(const void* key1, const void* key2);
};

typedef map_base_t HASH_MAP;

int map_wcssize(const void* key);
BOOLEAN map_wcsimatch(const void* key1, const void* key2);

void map_init(map_base_t *m, void* pool);
BOOLEAN map_resize(map_base_t* m, int nbuckets);
void* map_insert(map_base_t *m, const void* key, void* vdata, size_t vsize);
void* map_get(map_base_t *m, const void* key);
void* map_remove(map_base_t *m, const void* key);
void map_clear(map_base_t *m);

typedef struct {
  unsigned int bucketidx;
  map_node_t *node;
  void* key;
  void* value;
} map_iter_t;

map_iter_t map_iter();
BOOLEAN map_next(map_base_t *m, map_iter_t *iter);

//void map_dump(map_base_t *m);

#ifdef __cplusplus
}
#endif

#endif
