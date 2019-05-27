#include "annot.h"

struct Ptr {
  int data;
  pptr Ptr* next;

  void correct() {
    Ptr* ptr = new Ptr;
    ptr->data = 5;
    clflush(ptr);
    this->next = ptr;
  }

  void correct2() {
    Ptr* ptr = new Ptr;
    clflush(ptr);
    this->next = ptr;
  }

  void notFlushed() {
    Ptr* ptr = new Ptr;
    clflush(ptr);
    ptr->data = 5;
    this->next = ptr;
  }

  void notFlushed2() {
    Ptr* ptr = new Ptr;
    this->next = ptr;
  }

  void correct3() {
    Ptr* ptr = (Ptr*)malloc(sizeof(Ptr));
    ptr->data = 5;
    clflush(ptr);
    this->next = ptr;
    ptr->next = ptr;
  }

  void writeParam(Ptr* param, Ptr* param2){
    param->next = param2;    
  }

  void writeParam2(Ptr* param, Ptr* param2){
    clflush(param2);
    param->next = param2;
    this->next = param;
  }

  void correctParam(Ptr* param, Ptr* param2){
    clflush(param2);
    param->next = param2;
    clflush(param);
    this->next = param;
  }

  void skip_fnc unanalyzed() {
    Ptr* ptr = new Ptr;
    this->next = ptr;
  }   
};
