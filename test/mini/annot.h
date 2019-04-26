#pragma once

#define analyze_tx __attribute((annotate("TxCode")))
#define TXBEG(pop) pmemobj_tx_begin(pop, NULL, TX_LOCK_NONE);
#define TXEND pmemobj_tx_end();