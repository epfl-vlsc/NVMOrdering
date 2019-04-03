#pragma once
#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

// different cache line placement analysis

void vfence();
void pfence();
void clflush(void const* p);
