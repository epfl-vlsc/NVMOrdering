#include <iostream>
#include <vector>

#include "annot.h"

#define PM_EQU(x, y) x = y
#define pmalloc malloc

void pmemalloc_activate(void* ptr){}
void pmem_persist(void* ptr, size_t s, int b){}

class plist {
 public:
  struct node {
    pptr struct node* next;
    const char* val;
  };

  sentinelp(plist::_size) struct node** head;
  sentinelp(plist::_size) struct node** tail;
  bool activate;
  off_t _size = 0;

  plist()
      : head(nullptr),
        tail(nullptr),
        activate(true) { // Why would not want to activate ?
    head = (struct node**) pmalloc(sizeof(struct node*));//new (struct node*);
    tail = (struct node**) pmalloc(sizeof(struct node*));//new (struct node*);
    PM_EQU(((*head)), (nullptr));
    PM_EQU(((*tail)), (nullptr));
  }


  struct node* analyze_writes init(const char* val) {
    struct node* np = (struct node*) pmalloc(sizeof(struct node));//new struct node;

    PM_EQU((np->next), ((*head)));
    PM_EQU((np->val), (val));

    PM_EQU(((*head)), (np));
    PM_EQU(((*tail)), (np));

    if (activate)
      pmemalloc_activate(np);

    PM_EQU((_size), (_size+1));
    return np;
  }

  off_t push_back(const char* val) {
    off_t index = -1;

    if ((*head) == nullptr) {
      if (init(val) != nullptr)
        index = 0;
      return index;
    }

    struct node* tailp = nullptr;
    struct node* np = (struct node*) pmalloc(sizeof(struct node));// new struct node;

    // Link it in at the end of the list
    PM_EQU((np->val), (val));
    PM_EQU((np->next), (nullptr));

    tailp = (*tail);
    PM_EQU(((*tail)), (np));

    if (activate)
      pmemalloc_activate(np);

    PM_EQU((tailp->next), (np));
    pmem_persist(&tailp->next, sizeof(*np), 0);

    index = _size;
    PM_EQU((_size), (_size+1));
    return index;
  }
};
