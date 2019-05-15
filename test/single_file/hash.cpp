#include "annot.h"

#define PM_EQU(X, Y) X = Y
#define kp_flush_range(addr, use_nvm)                                          \
  do {                                                                         \
    if (use_nvm) {                                                             \
      flush_range(addr);                                                       \
    }                                                                          \
  } while (0)

void flush_range(void* ptr) {}

struct hash_table {
  /* The array of buckets starts at BUCKET and extends to BUCKET_LIMIT-1,
     for a possibility of N_BUCKETS.  Among those, N_BUCKETS_USED buckets
     are not empty, there are N_ENTRIES active entries in the table.  */
  struct hash_entry* bucket;
  struct hash_entry const* bucket_limit;
  size_t n_buckets;
  size_t n_buckets_used;
  size_t n_entries;

  bool use_nvm;
  int state;
} sentinelp(dcl-hash_table::state);

struct masters {
  /* The array of buckets starts at BUCKET and extends to BUCKET_LIMIT-1,
     for a possibility of N_BUCKETS.  Among those, N_BUCKETS_USED buckets
     are not empty, there are N_ENTRIES active entries in the table.  */
  struct hash_entry* bucket;
  struct hash_entry const* bucket_limit;
  size_t n_buckets;
  size_t n_buckets_used;
  size_t n_entries;

  bool use_nvm;
  int state;
} sentinelp(dcl-masters::state);

struct pairs {
  /* Why not a "use_nvm" flag here? Because these are always stored in a
   * commit record, which does have use_nvm.
   * But actually, just makes it simpler to store it in here anyway. */
  bool use_nvm;
  const char* key;
  const void* value;
  size_t size;
  int state;
} sentinelp(dcl-pairs::state);

int analyze_writes hash_initialize(hash_table** table, masters** master,
                                   pairs** pair, bool use_nvm) {

  /* "CDDS": flush, set state, and flush again. */
  PM_EQU(((*master)->bucket), (NULL)); // persistent
  kp_flush_range(*master, use_nvm);
  PM_EQU(((*master)->state), (1)); // persistent
  kp_flush_range(&((*master)->state), use_nvm);

  PM_EQU(((*table)->bucket), (NULL)); // persistent
  kp_flush_range((void*)*table, use_nvm);
  PM_EQU(((*table)->state), (1));
  kp_flush_range((void*)&((*table)->state), use_nvm);

  PM_EQU(((*pair)->size), (NULL)); // persistent
  kp_flush_range((void*)*pair, use_nvm);
  PM_EQU(((*pair)->state), (1));
  kp_flush_range((void*)&((*pair)->state), use_nvm);

  return 0;
}