#include "annot.h"

void log(void* ptr) {}
void tx_begin() {}
void tx_end() {}

struct Log {
  log_field int data;

  void analyze_logging correct() {
    tx_begin();
    log(&data);
    data = 6;
    tx_end();
  }

  void analyze_logging notLogged() {
    tx_begin();
    data = 5;
    tx_end();
  }

  void analyze_logging outsideTx() { data = 5; }
};
