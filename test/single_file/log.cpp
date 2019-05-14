#include "annot.h"

void log(void* ptr) {}

struct Log {
  log_field int data;

  void analyze_logging correct() {
    log(&data);
    data = 6;
  }

  void analyze_logging notLogged() { data = 5; }
};
