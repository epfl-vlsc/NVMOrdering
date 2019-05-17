#pragma once
#include <libpmemobj.h>
#include <stdlib.h>

#ifndef BTREE_MAP_TYPE_OFFSET
#define BTREE_MAP_TYPE_OFFSET 1012
#endif

struct btree_map;
TOID_DECLARE(struct btree_map, BTREE_MAP_TYPE_OFFSET + 0);

PMEMoid btree_map_remove(PMEMobjpool* pop, TOID(struct btree_map) map,
                         uint64_t key);

/*
void *pmemobj_direct(PMEMoid oid);
*/
