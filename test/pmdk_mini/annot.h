#pragma once

#define analyze_tx __attribute((annotate("TxCode")))
#define TX_BEGIN(pop) pmemobj_tx_begin(pop, NULL, TX_LOCK_NONE);
#define TX_ONABORT
#define TX_ONCOMMIT
#define TX_FINALLY
#define TX_END pmemobj_tx_end();