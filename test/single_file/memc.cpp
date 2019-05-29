#include <cstring>

void x() {
  int x = 5;
  int y = 0;
  memcpy(&y, &x, sizeof(int));
  int z = x;
  x = 6;
  memcpy(&x, &y, sizeof(int));
  z = x;
}