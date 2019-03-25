#include <cstdlib>

void loop() {
  int i = 0;
  int n = 0;
  while (i < 10) {
    if (i == 6) {
      n = 1;
    }
    ++i;
  }
}

void* heapmem() {
  void* ptr = malloc(sizeof(int));
  loop();
  return ptr;
}