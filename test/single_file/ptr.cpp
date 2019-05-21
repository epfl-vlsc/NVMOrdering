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
    clflush(ptr);
    ptr->data = 5;
    this->next = ptr;
  }

  void analyze_writes notFlushed2() {
    Ptr* ptr = new Ptr;
    this->next = ptr;
  }

  void analyze_writes correct3() {
    Ptr* ptr = (Ptr*)malloc(sizeof(Ptr));
    ptr->data = 5;
    clflush(ptr);
    this->next = ptr;
    ptr->next = ptr;
  }

  void unanalyzed() {
    Ptr* ptr = new Ptr;
    this->next = ptr;
  }    
};
