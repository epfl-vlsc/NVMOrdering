#include "annot.h"

struct BitField {
  sentinelp(BitField::valid) int data : 30;
  int valid : 2;
};
