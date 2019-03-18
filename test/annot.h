#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define pdcl(CHECKER) __attribute((annotate("dcl-" TOSTRING(CHECKER))))
#define pscl(CHECKER) __attribute((annotate("scl-" TOSTRING(CHECKER))))
#define pcheck(VAL) __attribute((annotate("check-" #VAL)))
//#define pcheck __attribute((annotate("check")))

#define recovery_code __attribute((annotate("RecoveryCode")))
#define persistent_code __attribute((annotate("PersistentCode")))

#define tx_beg __attribute((annotate("TX_BEG"))) int transaction_begin=0;
#define tx_end __attribute((annotate("TX_END"))) int transaction_end=0;
