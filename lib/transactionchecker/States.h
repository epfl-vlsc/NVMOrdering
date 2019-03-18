#pragma once
#include "Common.h"

REGISTER_TRAIT_WITH_PROGRAMSTATE(TxCounter, unsigned)
REGISTER_MAP_WITH_PROGRAMSTATE(PMap, const clang::ento::MemRegion*, bool)