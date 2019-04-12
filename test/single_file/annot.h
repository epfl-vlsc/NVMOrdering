#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define sentinelp(CHECKER) __attribute((annotate("pair-" TOSTRING(CHECKER))))
#define sentinel __attribute((annotate("sent")))

#define analyze_writes __attribute((annotate("PersistentCode")))
#define analyze_recovery __attribute((annotate("RecoveryCode")))
#define analyze_safe __attribute((annotate("SafeCode")))

#define plog __attribute((annotate("log")))
#define log_ptr __attribute((annotate("LogPtr")))
#define log_code __attribute((annotate("LogCode")))


void tx_begin(){}
void tx_end(){}
void tx_add(void* ptr){}

#define tx_beg tx_begin();
#define tx_end tx_end();
#define tx_add(P) tx_add(P)
