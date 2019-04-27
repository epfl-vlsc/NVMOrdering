#pragma once
#include "Common.h"

namespace clang::ento::nvm {

bool inAlloc(const IdentifierInfo* II) {
  return II->isStr("pmalloc") || II->isStr("pmemobj_tx_alloc") ||
         II->isStr("pmemobj_tx_zalloc") || II->isStr("pmemobj_tx_xalloc") ||
         II->isStr("pmemobj_tx_realloc") || II->isStr("pmemobj_tx_zrealloc") ||
         II->isStr("pmemobj_tx_strdup") || II->isStr("pmemobj_tx_wcsdup");
}

bool inFree(const IdentifierInfo* II) {
  return II->isStr("pfree") || II->isStr("pmemobj_tx_free");
}

bool inDirect(const IdentifierInfo* II) { return II->isStr("pmemobj_direct"); }

bool inAddRange(const IdentifierInfo* II) {
  return II->isStr("pmemobj_tx_add_range");
}

bool inAddRangeDirect(const IdentifierInfo* II) {
  return II->isStr("pmemobj_tx_add_range_direct");
}

bool inTxBeg(const IdentifierInfo* II) {
  return II->isStr("tx_begin") || II->isStr("pmemobj_tx_begin");
}

bool inTxEnd(const IdentifierInfo* II) {
  return II->isStr("tx_end") || II->isStr("pmemobj_tx_end");
}

bool inMisc(const IdentifierInfo* II) {
  return II->isStr("pmemobj_pool_by_oid");
}

} // namespace clang::ento::nvm