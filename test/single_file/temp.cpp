#include "annot.h"

struct alignas(128) X{
  sentinelp(X::valid) int data : 30;
  int valid : 2;
};

struct alignas(128) BitField {
  int data : 30;
  int valid : 2;
  sentinelp(BitField::valid) X x;
} sentinelp(BitField::valid);
