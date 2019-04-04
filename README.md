## How to build for the first time ##

    ./run.sh build
       
## How to run ##
    ./run.sh <report type> <test file> <checker type> 
    ./run.sh scan dcl write

## Simple Masstree ##
    ./run.sh scan simple_masstree write
    ./run.sh scan simple_masstree read

* look at `output/index.html` for results

* \<report type>
    * run - generate command line report
    * scan - generate visual bug report
    * ast - generate ast of code to examine
* \<test file> all files under test folder
* \<checker type>
    * write - main code checker
    * read - recover code checker
    * txm - transaction checker for mnemosyne
    * txp - transaction checker for pmdk
    * log - check log ordering


Tests:
write
check: 4
dcl: 7
scl: 4
dcltoscl: 7
scltodcl: 7
twocheck: 8
dclm: 3
dclmt: 7
sclm: 3
sclmt: 7
simple_masstree: 0

read
rec: 1
recm: 1
recmt: 2
simple_masstree: 0

txm
txm: 4

txp
txp: 1

log
log: 4

git diff > patch/owriteback/pmfs1.patch
 git reset --hard



## how to program using libpmemobj
mmap
* pmemobj_create
* pmemobj_open
* pmemobj_close
* pmemobj_check

access
PMEMoid root = pmemobj_root(pop, sizeof (struct my_root));

root->length = strlen(my_name);
pmemobj_persist(&root->length, sizeof (root->length));
pmemobj_memcpy_persist(root->name, my_name, root->length);


pmemobj_tx_add_range(root, 0, sizeof (struct vector));
or
pmemobj_tx_add_range(root, offsetof(struct vector, z), sizeof(int));
or
pmemobj_tx_add_range_direct(to_modify, sizeof (int));

TX_SET(rect, a, new_a);

#define TX_SET(o, field, value) (\
	TX_ADD_FIELD(o, field),\
	D_RW(o)->field = value)

#define TX_SET_DIRECT(p, field, value) (\
	TX_ADD_FIELD_DIRECT(p, field),\
	(p)->field = value)

TOID(struct rectangle) rect = TX_NEW(struct rectangle);
TX_ADD(root);

POBJ_NEW(pop, &D_RW(root)->rect, struct rectangle, rect_construct, NULL);
int p = perimeter_calc(D_RO(root)->rect);
/* busy work */
POBJ_FREE(&D_RW(root)->rect);

POBJ_FOREACH_TYPE(pop, iter) {
	int p = perimeter_calc(D_RO(iter));
	printf("Perimeter of rectangle = %d", p);
}

The TX_ADD_FIELD() macro saves the current value of given FIELD of the object referenced by a handle o in the undo log. The application is then free to directly modify the specified FIELD. In case of a failure or abort, the saved value will be restored.

The TX_ADD() macro takes a "snapshot" of the entire object referenced by object handle o and saves it in the undo log. The object size is determined from its TYPE. The application is then free to directly modify the object. In case of a failure or abort, all the changes within the object will be rolled back.



synthetic
## backup ##
```cpp
PM_EQU(D_RW(sb)->color, COLOR_BLACK);

//D_RW
//pmemobj_direct
//PM_EQU
//pm_dst = y;

TX_ADD_FIELD(map, root);
/*
#define TX_ADD_FIELD(o, field)\
pmemobj_tx_add_range((o).oid, offsetofp(TOID_TYPEOF(o) *, field),\
		sizeof(D_RO(o)->field))
*/
TX_ADD(rsb);
/*
#define TX_ADD(o)\
pmemobj_tx_add_range((o).oid, 0, sizeof(*(o)._type))
*/

TX_SET(NODE_P(node), slots[NODE_LOCATION(node)], child);
/*
#define TX_SET(o, field, value) (\
TX_ADD_FIELD(o, field),\
D_RW(o)->field = value)
*/

pmemobj_tx_add_range_direct(dest, sizeof(*dest));


```

## incomplete tx ##
remove TX
```cpp
TX_BEGIN(pop) {
map_insert(mapc, D_RW(root)->map, rand(),
        new_store_item().oid);
} TX_END


//pmemobj_tx_begin
//pmemobj_tx_end
```

## ordering ##
```cpp
pmemobj_persist(pop, D_RW(hashmap), sizeof(*D_RW(hashmap)));

/*
#define pmem_persist(addr, len, flags)	\
{									    \
    pmem_flush_cache(addr, len, flags);	\
    PM_FENCE();							\
}

#define pmem_flush_cache(addr, len, flags)				\
{									                    \
    uintptr_t uptr = (uintptr_t) addr & ~(ALIGN - 1);	\
    uintptr_t end = (uintptr_t) addr + len;				\
    for (; uptr < end; uptr += ALIGN) {				    \
                __builtin_ia32_clflush((void *) uptr);	\   
    }                   								\
}

#define PM_FENCE() 
__pm_trace_print

*/
```


## writeback ##
```cpp
pmfs_flush_buffer(le, LOGENTRY_SIZE, false);

//_mm_clwb(buf + i);
//_mm_clflushopt(buf + i);
//_mm_clflush(buf + i);
//asm volatile ("sfence\n" : : ); 
```



