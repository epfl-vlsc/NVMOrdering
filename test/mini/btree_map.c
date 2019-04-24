#include "btree_map.h"
#include "annot.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>

TOID_DECLARE(struct tnode, BTREE_MAP_TYPE_OFFSET + 1);

#define BTREE_ORDER 8                     /* can't be odd */
#define BTREE_MIN ((BTREE_ORDER / 2) - 1) /* min number of keys per node */

struct tnode {
  int n;
};

struct btree {
  TOID(struct tnode) root;
};

void create(PMEMobjpool* pop, TOID(struct btree) * map) {
  TX_BEGIN(pop) {
    pmemobj_tx_add_range_direct(map, sizeof(*map));
    *map = TX_ZNEW(struct btree);
  }
  TX_END
}

static void insert(TOID(struct tnode) node) {
  D_RW(node)->n += 1;
  //memmove(&D_RW(node)->n, &D_RW(node)->n, sizeof(int));
}

void clear(TOID(struct btree) map) {
  insert(D_RO(map)->root);

  TX_ADD_FIELD(map, root);
  D_RW(map)->root = TOID_NULL(struct tnode);
}
