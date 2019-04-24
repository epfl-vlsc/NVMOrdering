#include "btree_map.h"
#include "annot.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>

TOID_DECLARE(struct tree_map_node, BTREE_MAP_TYPE_OFFSET + 1);

#define BTREE_ORDER 8                     /* can't be odd */
#define BTREE_MIN ((BTREE_ORDER / 2) - 1) /* min number of keys per node */

struct tree_map_node_item {
  uint64_t key;
  PMEMoid value;
};

struct tree_map_node {
  int n; /* number of occupied slots */
  struct tree_map_node_item items[BTREE_ORDER - 1];
  TOID(struct tree_map_node) slots[BTREE_ORDER];
};

struct btree_map {
  TOID(struct tree_map_node) root;
};

void create(PMEMobjpool* pop, TOID(struct btree_map) * map,
                     void* arg) {

  TX_BEGIN(pop) {
    pmemobj_tx_add_range_direct(map, sizeof(*map));
    *map = TX_ZNEW(struct btree_map);
  }
  TX_END
}


static void insert(TOID(struct tree_map_node) node) {
  D_RW(node)->n += 1;
}

void clear(TOID(struct btree_map) map) {

    TX_ADD_FIELD(map, root);
    D_RW(map)->root = TOID_NULL(struct tree_map_node);

	insert(D_RO(map)->root);

}
