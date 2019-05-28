#pragma once

#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

#define MACRO_PARAM_STRINGIFY(x) #x
#define MACRO_TOSTRING(x) MACRO_PARAM_STRINGIFY(x)

#define sentinelp(CHECKER) __attribute((annotate("pair-" MACRO_TOSTRING(CHECKER))))
#define sentinel __attribute((annotate("sent")))

//pointer checker
#define pptr __attribute((annotate("persistent_ptr")))

#define skip_fnc __attribute((annotate("SkipCode")))

#define log_field __attribute((annotate("log")))
#define analyze_logging __attribute((annotate("LogCode")))

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflushopt(void const* p) { }
void clflush(void const* p) { }
