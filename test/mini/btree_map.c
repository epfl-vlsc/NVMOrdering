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

#define EMPTY_ITEM ((struct tree_map_node_item){0, OID_NULL})

struct tree_map_node {
  int n; /* number of occupied slots */
  struct tree_map_node_item items[BTREE_ORDER - 1];
  TOID(struct tree_map_node) slots[BTREE_ORDER];
};

struct btree_map {
  TOID(struct tree_map_node) root;
};

/*
static void CCC(TOID(struct tree_map_node) lsb, TOID(struct tree_map_node) node,
                TOID(struct tree_map_node) parent, int p) {

  TX_ADD_FIELD(parent, items[p - 1]);
  D_RW(parent)->items[p - 1] = D_RO(lsb)->items[D_RO(lsb)->n - 1];

  TX_ADD(node);

  TX_ADD_FIELD(lsb, n);
  D_RW(lsb)->n -= 1;
}

static PMEMoid BBB(TOID(struct btree_map) map, TOID(struct tree_map_node) node,
                   TOID(struct tree_map_node) parent, uint64_t key, int p) {
  while (1) {
    CCC(D_RO(map)->root, node, parent, p);
  }
  return OID_NULL;
}

PMEMoid analyze_tx AAA(PMEMobjpool* pop, TOID(struct btree_map) map,
                       uint64_t key) {
  PMEMoid ret = OID_NULL;
  TX_BEGIN(pop) {
    BBB(map, D_RW(map)->root, TOID_NULL(struct tree_map_node), key, 0);
  }
  TX_END

  return ret;
}*/

#define NODE_CONTAINS_ITEM(_n, _i, _k)                                         \
  (_i != D_RO(_n)->n && D_RO(_n)->items[_i].key == _k)

#define NODE_CHILD_CAN_CONTAIN_ITEM(_n, _i, _k)                                \
  (_i == D_RO(_n)->n || D_RO(_n)->items[_i].key > _k) &&                       \
      !TOID_IS_NULL(D_RO(_n)->slots[_i])


static void btree_map_insert_item(TOID(struct tree_map_node) node, int p,
                                  struct tree_map_node_item item) {
  TX_ADD(node);
  if (D_RO(node)->items[p].key != 0) {
    memmove(&D_RW(node)->items[p + 1], &D_RW(node)->items[p],
            sizeof(struct tree_map_node_item) * ((BTREE_ORDER - 2 - p)));
  }
}

static void btree_map_rotate_left(TOID(struct tree_map_node) lsb,
                                  TOID(struct tree_map_node) node,
                                  TOID(struct tree_map_node) parent, int p) {
  /* move the separator from parent to the deficient node */
  struct tree_map_node_item sep = D_RO(parent)->items[p - 1];
  btree_map_insert_item(node, 0, sep);

  /* the last element of the left sibling is the new separator */
  TX_ADD_FIELD(parent, items[p - 1]);
  D_RW(parent)->items[p - 1] = D_RO(lsb)->items[D_RO(lsb)->n - 1];

  TX_ADD(node);
  /* rotate the node children */
  memmove(D_RW(node)->slots + 1, D_RO(node)->slots,
          sizeof(TOID(struct tree_map_node)) * (D_RO(node)->n));

  /* the nodes are not necessarily leafs, so copy also the slot */
  D_RW(node)->slots[0] = D_RO(lsb)->slots[D_RO(lsb)->n];

  TX_ADD_FIELD(lsb, n);
  D_RW(lsb)->n -= 1; /* it loses one element, but still > min */
}

static void btree_map_rebalance(TOID(struct btree_map) map,
                                TOID(struct tree_map_node) node,
                                TOID(struct tree_map_node) parent, int p) {
  TOID(struct tree_map_node)
  lsb = p == 0 ? TOID_NULL(struct tree_map_node) : D_RO(parent)->slots[p - 1];

  btree_map_rotate_left(lsb, node, parent, p);
}


static PMEMoid btree_map_remove_item(TOID(struct btree_map) map,
                                     TOID(struct tree_map_node) node,
                                     TOID(struct tree_map_node) parent,
                                     uint64_t key, int p) {
  PMEMoid ret = OID_NULL;
  btree_map_rebalance(map, node, parent, p);

  return ret;
}

/*
 * btree_map_remove -- removes key-value pair from the map
 */
PMEMoid analyze_tx btree_map_remove(PMEMobjpool* pop,
                                    TOID(struct btree_map) map, uint64_t key) {
  PMEMoid ret = OID_NULL;
  TX_BEGIN(pop) {
    ret = btree_map_remove_item(map, D_RW(map)->root,
                                TOID_NULL(struct tree_map_node), key, 0);
  }
  TX_END

  return ret;
}