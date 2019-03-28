#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define pdclm(CHECKER) __attribute((annotate("dclmask-" TOSTRING(CHECKER))))
#define psclm(CHECKER) __attribute((annotate("sclmask-" TOSTRING(CHECKER))))
#define pdcl(CHECKER) __attribute((annotate("dcl-" TOSTRING(CHECKER))))
#define pscl(CHECKER) __attribute((annotate("scl-" TOSTRING(CHECKER))))
#define pcheck __attribute((annotate("check")))

#define recovery_code __attribute((annotate("RecoveryCode")))
#define persistent_code __attribute((annotate("PersistentCode")))

void tx_begin(){}
void tx_end(){}
void tx_add(void* ptr){}

#define tx_beg tx_begin();
#define tx_end tx_end();
#define tx_add(P) tx_add(P)
