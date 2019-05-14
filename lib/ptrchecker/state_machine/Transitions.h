#pragma once
#include "Common.h"
#include "State.h"

namespace clang::ento::nvm {
/*
ProgramStateRef addPtr(ProgramStateRef& State, SymbolRef Sym) {
  // If this is a symbol cast, remove the cast before adding the ptr.
  while (const SymbolCast* SC = dyn_cast<SymbolCast>(Sym))
    Sym = SC->getOperand();

  ProgramStateRef NewState = State->add<PtrSet>(Sym);
  assert(NewState);
  return NewState;
}

ProgramStateRef addPartialPtr(ProgramStateRef& State, SymbolRef ParentSym,
                              const SubRegion* SubRegion) {
  // Ignore partial ptr if the entire ptr symbol is already exists.
  if (State->contains<PtrSet>(ParentSym)) {
    return State;
  }

  // Partial ptr applies if only a portion of the symbol is used.
  if (SubRegion == SubRegion->getBaseRegion())
    return addPtr(State, ParentSym);

  const TaintedSubRegions* SavedRegs = State->get<DerivedSymTaint>(ParentSym);
  PtrSubRegions::Factory& F = State->get_context<PtrSubRegions>();
  PtrSubRegions Regs = SavedRegs ? *SavedRegs : F.getEmptyMap();

  Regs = F.add(Regs, SubRegion, Kind);
  ProgramStateRef NewState = State->set<DerivedSymTaint>(ParentSym, Regs);
  assert(NewState);
  return NewState;
}

ProgramStateRef addPtr(ProgramStateRef& State, const MemRegion* R) {
  if (const SymbolicRegion* SR = dyn_cast_or_null<SymbolicRegion>(R))
    return addPtr(State, SR->getSymbol());
  return State;
}

ProgramStateRef addPtr(ProgramStateRef& State, SVal V) {
  SymbolRef Sym = V.getAsSymbol();
  if (Sym)
    return addPtr(State, Sym);

  // If the SVal represents a structure, try to mass-taint all values within the
  // structure. For now it only works efficiently on lazy compound values that
  // were conjured during a conservative evaluation of a function - either as
  // return values of functions that return structures or arrays by value, or as
  // values of structures or arrays passed into the function by reference,
  // directly or through pointer aliasing. Such lazy compound values are
  // characterized by having exactly one binding in their captured store within
  // their parent region, which is a conjured symbol default-bound to the base
  // region of the parent region.
  if (auto LCV = V.getAs<nonloc::LazyCompoundVal>()) {
    ProgramStateManager& PSM = State->getStateManager();
    StoreManager& SM = PSM.getStoreManager();
    if (Optional<SVal> binding = SM.getDefaultBinding(*LCV)) {
      if (SymbolRef Sym = binding->getAsSymbol())
        return addPartialPtr(State, Sym, LCV->getRegion());
    }
  }

  const MemRegion* R = V.getAsRegion();
  return addPtr(State, R);
}

ProgramStateRef addPtr(ProgramStateRef& State, const Stmt* S,
                       const LocationContext* LCtx) {
  return addPtr(State, State->getSVal(S, LCtx));
}
/*
bool isTainted(ProgramStateRef& State, const Stmt* S,
               const LocationContext* LCtx, TaintTagType Kind) {
  if (const Expr* E = dyn_cast_or_null<Expr>(S))
    S = E->IgnoreParens();

  SVal val = getSVal(S, LCtx);
  return isTainted(val, Kind);
}

bool isTainted(ProgramStateRef& State, SVal V, TaintTagType Kind) {
  if (const SymExpr* Sym = V.getAsSymExpr())
    return isTainted(Sym, Kind);
  if (const MemRegion* Reg = V.getAsRegion())
    return isTainted(Reg, Kind);
  return false;
}

bool isTainted(ProgramStateRef& State, const MemRegion* Reg, TaintTagType K) {
  if (!Reg)
    return false;

  // Element region (array element) is tainted if either the base or the offset
  // are tainted.
  if (const ElementRegion* ER = dyn_cast<ElementRegion>(Reg))
    return isTainted(ER->getSuperRegion(), K) || isTainted(ER->getIndex(), K);

  if (const SymbolicRegion* SR = dyn_cast<SymbolicRegion>(Reg))
    return isTainted(SR->getSymbol(), K);

  if (const SubRegion* ER = dyn_cast<SubRegion>(Reg))
    return isTainted(ER->getSuperRegion(), K);

  return false;
}

bool isTainted(ProgramStateRef& State, SymbolRef Sym, TaintTagType Kind) {
  if (!Sym)
    return false;

  // Traverse all the symbols this symbol depends on to see if any are tainted.
  for (SymExpr::symbol_iterator SI = Sym->symbol_begin(),
                                SE = Sym->symbol_end();
       SI != SE; ++SI) {
    if (!isa<SymbolData>(*SI))
      continue;

    if (const TaintTagType* Tag = get<TaintMap>(*SI)) {
      if (*Tag == Kind)
        return true;
    }

    if (const SymbolDerived* SD = dyn_cast<SymbolDerived>(*SI)) {
      // If this is a SymbolDerived with a tainted parent, it's also tainted.
      if (isTainted(SD->getParentSymbol(), Kind))
        return true;

      // If this is a SymbolDerived with the same parent symbol as another
      // tainted SymbolDerived and a region that's a sub-region of that tainted
      // symbol, it's also tainted.
      if (const TaintedSubRegions* Regs =
              get<DerivedSymTaint>(SD->getParentSymbol())) {
        const TypedValueRegion* R = SD->getRegion();
        for (auto I : *Regs) {
          // FIXME: The logic to identify tainted regions could be more
          // complete. For example, this would not currently identify
          // overlapping fields in a union as tainted. To identify this we can
          // check for overlapping/nested byte offsets.
          if (Kind == I.second && (R == I.first || R->isSubRegionOf(I.first)))
            return true;
        }
      }
    }

    // If memory region is tainted, data is also tainted.
    if (const SymbolRegionValue* SRV = dyn_cast<SymbolRegionValue>(*SI)) {
      if (isTainted(SRV->getRegion(), Kind))
        return true;
    }

    // If this is a SymbolCast from a tainted value, it's also tainted.
    if (const SymbolCast* SC = dyn_cast<SymbolCast>(*SI)) {
      if (isTainted(SC->getOperand(), Kind))
        return true;
    }
  }

  return false;
}
*/
} // namespace clang::ento::nvm