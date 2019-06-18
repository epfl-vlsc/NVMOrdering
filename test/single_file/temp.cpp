#include "annot.h"

struct Dcl {
  sentinelp(dcl-Dcl::valid) int data;
  int valid;

  void x(){
    data = 1;
  }

  void correct(){
    x();
    if(data == 1){
      clflush(&data);
      valid = 1;
    }else{
      valid = 1;
    }
    x();
    
  }

  void skip_fnc do_stuff(){
    clflushopt(&data);
    pfence();
  }

  void skip_fnc wrongButTrue(bool use_nvm) {
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

  void skip_fnc wrong() {
    data = 1;
    valid = 1;
  }
};
