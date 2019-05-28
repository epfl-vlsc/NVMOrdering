#include "annot.h"

unsigned int flush_range(const void *addr, const size_t size){return 0;}

#define PM_EQU(a, b) a = b;
#define kp_flush_range(addr, size, use_nvm) do {  \
	if (use_nvm) {                                \
		flush_range(addr, size);                  \
	}                                             \
} while (0)

struct kp_kvstore_struct {
	int id;
	uint64_t pairs_count;      //NOTE: check that this is reliable before using...
	pthread_rwlock_t *rwlock;  //kv store-wide lock
	bool is_master;            //is this the master store?
	bool use_nvm;              //is this kvstore stored in non-volatile memory?
	bool detect_conflicts;
	uint64_t global_snapshot;        //global snapshot number - master only!!!
	                                 //this number should only ever be incremented,
									 //never written to!!

	pthread_mutex_t *snapshot_lock;  //lock for the snapshot number
	int state;
}sentinelp(dcl-kp_kvstore_struct::state);


int kp_kvstore_create(kp_kvstore_struct** kv, bool is_master, bool use_nvm){
	int ret;
	void (*hash_freer)(void *) = NULL;    //function pointer

	
	PM_EQU(((*kv)->id), (1)); // persistent
	PM_EQU(((*kv)->is_master), (is_master)); // persistent
	PM_EQU(((*kv)->use_nvm), (use_nvm));	// persistent 
	kp_flush_range((void *)*kv, sizeof(kp_kvstore_struct) - sizeof(int), (*kv)->use_nvm);
	PM_EQU(((*kv)->state), (1)); // persist
	kp_flush_range((void *)&((*kv)->state), sizeof(int), (*kv)->use_nvm);

	return 0;
}
