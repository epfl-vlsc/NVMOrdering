#pragma once

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