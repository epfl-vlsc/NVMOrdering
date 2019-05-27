#include "annot.h"

struct Cons {
  sentinelp(dcl-Cons::valid) int data;
  int valid;

  Cons() {
    data = 1;
    valid = 1;
  }
};

void x(){
    Cons cons;
}