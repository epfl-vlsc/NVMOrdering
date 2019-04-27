#include "btree_map.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "annot.h"

TOID_DECLARE(struct tnode, BTREE_MAP_TYPE_OFFSET + 1);

#define BTREE_ORDER 8                     /* can't be odd */
#define BTREE_MIN ((BTREE_ORDER / 2) - 1) /* min number of keys per node */

struct tnode {
  int n;
};

struct btree {
  TOID(struct tnode) root;
};

void analyze_tx create(PMEMobjpool* pop, TOID(struct btree) * map) {
  int ret = 0;
  TX_BEGIN(pop) {
    pmemobj_tx_add_range_direct(map, sizeof(*map));
    *map = TX_ZNEW(struct btree);
  }TX_ONABORT {
		ret = 1;
	}
  TX_END
}

static void log(TOID(struct btree) zap) {
  TX_ADD(zap);
}

static void insert(PMEMobjpool* pop, TOID(struct tnode) node, TOID(struct btree) zmap) {
  D_RW(node)->n += 1;
  D_RW(node)->n *= 1;
  D_RW(node)->n = 1;
  int a = 5;
  int *p = &a;
  D_RW(node)->n = *p;
  log(zmap);
}

void analyze_tx clear(PMEMobjpool* pop, TOID(struct btree) map) {
  TX_BEGIN(pop) {
    TX_ADD_FIELD(map, root);
    insert(pop, D_RO(map)->root, map);

    D_RW(map)->root = TOID_NULL(struct tnode);
  }
  TX_END
}