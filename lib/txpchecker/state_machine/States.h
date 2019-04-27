#pragma once
#include "Common.h"

REGISTER_TRAIT_WITH_PROGRAMSTATE(TxCounter, unsigned)
REGISTER_MAP_WITH_PROGRAMSTATE(LogMap, const clang::NamedDecl*, bool)
REGISTER_MAP_WITH_PROGRAMSTATE(IpMap, const clang::NamedDecl*,
                               const clang::NamedDecl*)