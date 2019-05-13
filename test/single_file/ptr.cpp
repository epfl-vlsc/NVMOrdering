#include "annot.h"

struct Ptr {
  int data;
  pptr Ptr* next;

  void analyze_writes correct() {
    Ptr* ptr = new Ptr;
    ptr->data = 5;
    clflush(ptr);
    this->next = ptr;
  }

  void analyze_writes correct2() {
    Ptr* ptr = new Ptr;
    clflush(ptr);
    this->next = ptr;
  }

  void analyze_writes notFlushed() {
    Ptr* ptr = new Ptr;
    this->next = ptr;
  }  
};
