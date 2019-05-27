#include "annot.h"

template <typename V> class plist {
public:
  struct node {
    pptr struct node* next;
    V val;
  };

  void notFlushed() {
    node* n = new node;
    node* z = new node;
    n->next = z;
  }
};

void notFlushed() {
  auto* l = new plist<const char*>;
  l->notFlushed();
}
