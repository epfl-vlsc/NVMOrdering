#include "annot.h"

struct Dcl {
  sentinelp(dcl-Dcl::valid) int data;
  int valid;

  void do_stuff(){
    clflushopt(&data);
    pfence();
  }

  void correct(bool use_nvm) {
    data = 1;
    if(use_nvm){
      do_stuff();
    }
    valid = 1;
    if(use_nvm){
      return;
    }else{
      return;
    }
  }
};
