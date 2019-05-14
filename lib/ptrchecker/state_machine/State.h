#pragma once
#include "Common.h"

REGISTER_SET_WITH_PROGRAMSTATE(PtrSet, clang::ento::SymbolRef)

REGISTER_SET_FACTORY_WITH_PROGRAMSTATE(PtrSubRegions,
                                       const clang::ento::SubRegion*)

REGISTER_MAP_WITH_PROGRAMSTATE(DerivedSymPtr, clang::ento::SymbolRef,
                               PtrSubRegions)
