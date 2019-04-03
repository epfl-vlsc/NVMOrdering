#pragma once
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define pdcl(CHECKER) __attribute((annotate("dcl-" TOSTRING(CHECKER))))
#define pscl(CHECKER) __attribute((annotate("scl-" TOSTRING(CHECKER))))
#define pcheck __attribute((annotate("check-")))

#define recovery_code __attribute((annotate("RecoveryCode")))
#define persistent_code __attribute((annotate("PersistentCode")))
#define end_code __attribute((annotate("EndCode")))

#define plog __attribute((annotate("log")))
#define log_ptr __attribute((annotate("LogPtr")))
#define log_code __attribute((annotate("LogCode")))