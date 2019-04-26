#pragma once
#include <libpmemobj.h>
#include <stdlib.h>

#ifndef BTREE_MAP_TYPE_OFFSET
#define BTREE_MAP_TYPE_OFFSET 1012
#endif

struct btree;
struct vbtree;
TOID_DECLARE(struct btree, BTREE_MAP_TYPE_OFFSET + 0);

void create(PMEMobjpool* pop, TOID(struct btree) * map);

void clear(PMEMobjpool* pop, TOID(struct btree) map);

/*
void *pmemobj_direct(PMEMoid oid);
*/
