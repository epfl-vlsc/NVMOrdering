#pragma once

#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define sentinelp(CHECKER) __attribute((annotate("pair-" TOSTRING(CHECKER))))
#define sentinel __attribute((annotate("sent")))

//pointer checker
#define pptr __attribute((annotate("persistent_ptr")))

#define analyze_writes __attribute((annotate("PersistentCode")))
#define analyze_recovery __attribute((annotate("RecoveryCode")))
#define analyze_safe __attribute((annotate("SafeCode")))

#define log_field __attribute((annotate("log")))
#define analyze_logging __attribute((annotate("LogCode")))

void vfence() { std::atomic_thread_fence(std::memory_order_release); }
void pfence() { _mm_sfence(); }
void clflushopt(void const* p) { }
void clflush(void const* p) { }
