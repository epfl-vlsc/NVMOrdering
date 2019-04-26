#pragma once
#include <libpmemobj.h>
#include <stdlib.h>
#include "annot.h"

#ifndef BTREE_MAP_TYPE_OFFSET
#define BTREE_MAP_TYPE_OFFSET 1012
#endif

struct btree;
struct vbtree;
TOID_DECLARE(struct btree, BTREE_MAP_TYPE_OFFSET + 0);

void create(PMEMobjpool* pop, TOID(struct btree) * map);

void clear(TOID(struct btree) map);

/*
TOID(struct btree_map) *map
pmemobj_tx_add_range_direct(map, sizeof(*map));
*map = TX_ZNEW(struct btree_map);
TX_FREE(*map);

TOID(struct btree_map) map
TX_ADD_FIELD(map, root);
D_RW(map)->root = TX_ZNEW(struct tree_map_node);

TOID(struct tree_map_node) node
TX_ADD(node);
TX_ADD_FIELD(node, slots[D_RO(node)->n]);
D_RW(node)->items[pos] = item;
D_RW(node)->n += 1;

TOID(struct tree_map_node) lsb
TX_ADD_FIELD(lsb, n);
D_RW(lsb)->n -= 1;




int pmemobj_tx_add_range(PMEMoid oid, uint64_t off, size_t size);
int pmemobj_tx_add_range_direct(const void *ptr, size_t size);

PMEMoid pmemobj_tx_alloc(size_t size, uint64_t type_num);
PMEMoid pmemobj_tx_zalloc(size_t size, uint64_t type_num);
PMEMoid pmemobj_tx_xalloc(size_t size, uint64_t type_num, uint64_t flags);

void *pmemobj_direct(PMEMoid oid);
*/
