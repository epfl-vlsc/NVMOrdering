#pragma once

//helpers
#define MACRO_PARAM_STRINGIFY(x) #x
#define TOSTRING(x) MACRO_PARAM_STRINGIFY(x)

//transactional checker
#define analyze_tx __attribute((annotate("TxCode")))
#define TX_BEGIN(pop) pmemobj_tx_begin(pop, NULL, TX_LOCK_NONE);
#define TX_ONABORT
#define TX_ONCOMMIT
#define TX_FINALLY
#define TX_END pmemobj_tx_end();

//log checker
#define analyze_logging __attribute((annotate("LogCode")))
#define logger __attribute((annotate("LogFnc")))
#define persist __attribute((annotate("persist")))

//main checker
#define analyze_writes __attribute((annotate("PersistentCode")))
#define sentinelp(CHECKER) __attribute((annotate("pair-" TOSTRING(CHECKER))))