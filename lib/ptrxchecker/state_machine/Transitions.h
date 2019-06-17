#pragma once
#include "Common.h"
#include "State.h"

namespace clang::ento::nvm::Transitions {

ProgramStateRef writePtr(ProgramStateRef State, SymbolRef Sym);
ProgramStateRef writePtr(ProgramStateRef State, const MemRegion* R);
ProgramStateRef writePartialPtr(ProgramStateRef State, SymbolRef ParentSym,
                                const SubRegion* SubRegion);
ProgramStateRef writePtr(ProgramStateRef State, SVal V);
ProgramStateRef writePtr(ProgramStateRef State, const Stmt* S,
                         const LocationContext* LCtx);

ProgramStateRef flushPtr(ProgramStateRef State, SymbolRef Sym);
ProgramStateRef flushPtr(ProgramStateRef State, const MemRegion* R);
ProgramStateRef flushPartialPtr(ProgramStateRef State, SymbolRef ParentSym,
                                const SubRegion* SubRegion);
ProgramStateRef flushPtr(ProgramStateRef State, SVal V);
ProgramStateRef flushPtr(ProgramStateRef State, const Stmt* S,
                         const LocationContext* LCtx);

bool isPtrWritten(ProgramStateRef State, const Stmt* S,
                  const LocationContext* LCtx);
bool isPtrWritten(ProgramStateRef State, SVal V);
bool isPtrWritten(ProgramStateRef State, SymbolRef Sym);
bool isPtrWritten(ProgramStateRef State, const MemRegion* Reg);

void dumpPtr(ProgramStateRef State) {
  PtrSetTy TM = State->get<PtrSet>();

  if (!TM.isEmpty())
    llvm::errs() << "Ptr symbols:\n";

  for (auto I : TM) {
    llvm::errs() << I << "\n";
  }
}

ProgramStateRef writePtr(ProgramStateRef State, SymbolRef Sym) {
  // If this is a symbol cast, remove the cast before adding the taint. Taint
  // is cast agnostic.
  while (const SymbolCast* SC = dyn_cast<SymbolCast>(Sym))
    Sym = SC->getOperand();

  ProgramStateRef NewState = State->add<PtrSet>(Sym);
  assert(NewState);
  return NewState;
}

ProgramStateRef writePtr(ProgramStateRef State, const MemRegion* R) {
  if (const SymbolicRegion* SR = dyn_cast_or_null<SymbolicRegion>(R))
    return writePtr(State, SR->getSymbol());
  return State;
}

ProgramStateRef writePartialPtr(ProgramStateRef State, SymbolRef ParentSym,
                                const SubRegion* SubRegion) {
  // Ignore partial taint if the entire parent symbol is already tainted.
  if (State->contains<PtrSet>(ParentSym))
    return State;

  // Partial taint applies if only a portion of the symbol is tainted.
  if (SubRegion == SubRegion->getBaseRegion())
    return writePtr(State, ParentSym);

  const PtrSubRegions* SavedRegs = State->get<DerivedSymPtr>(ParentSym);
  PtrSubRegions::Factory& F = State->get_context<PtrSubRegions>();
  PtrSubRegions Regs = SavedRegs ? *SavedRegs : F.getEmptySet();

  Regs = F.add(Regs, SubRegion);
  ProgramStateRef NewState = State->set<DerivedSymPtr>(ParentSym, Regs);
  assert(NewState);
  return NewState;
}

ProgramStateRef writePtr(ProgramStateRef State, SVal V) {
  SymbolRef Sym = V.getAsSymbol();
  if (Sym)
    return writePtr(State, Sym);

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
    if (Optional<SVal> binding =
            State->getStateManager().getStoreManager().getDefaultBinding(
                *LCV)) {
      if (SymbolRef Sym = binding->getAsSymbol())
        return writePartialPtr(State, Sym, LCV->getRegion());
    }
  }

  const MemRegion* R = V.getAsRegion();
  return writePtr(State, R);
}

ProgramStateRef writePtr(ProgramStateRef State, const Stmt* S,
                         const LocationContext* LCtx) {
  return writePtr(State, State->getSVal(S, LCtx));
}

ProgramStateRef flushPtr(ProgramStateRef State, SymbolRef Sym) {
  // If this is a symbol cast, remove the cast before adding the taint. Taint
  // is cast agnostic.
  while (const SymbolCast* SC = dyn_cast<SymbolCast>(Sym))
    Sym = SC->getOperand();

  ProgramStateRef NewState = State->remove<PtrSet>(Sym);
  assert(NewState);
  return NewState;
}

ProgramStateRef flushPtr(ProgramStateRef State, const MemRegion* R) {
  if (const SymbolicRegion* SR = dyn_cast_or_null<SymbolicRegion>(R))
    return flushPtr(State, SR->getSymbol());
  return State;
}

ProgramStateRef flushPartialPtr(ProgramStateRef State, SymbolRef ParentSym,
                                const SubRegion* SubRegion) {
  // Ignore partial taint if the entire parent symbol is already tainted.
  if (State->contains<PtrSet>(ParentSym))
    return flushPtr(State, ParentSym);

  // Partial taint applies if only a portion of the symbol is tainted.
  if (SubRegion == SubRegion->getBaseRegion())
    return flushPtr(State, ParentSym);

  const PtrSubRegions* SavedRegs = State->get<DerivedSymPtr>(ParentSym);
  PtrSubRegions::Factory& F = State->get_context<PtrSubRegions>();

  if (SavedRegs) {
    PtrSubRegions Regs = *SavedRegs;
    Regs = F.remove(Regs, SubRegion);
    ProgramStateRef NewState = State->set<DerivedSymPtr>(ParentSym, Regs);
    assert(NewState);
    return NewState;
  }

  return State;
}

ProgramStateRef flushPtr(ProgramStateRef State, SVal V) {
  SymbolRef Sym = V.getAsSymbol();
  if (Sym)
    return flushPtr(State, Sym);

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
    if (Optional<SVal> binding =
            State->getStateManager().getStoreManager().getDefaultBinding(
                *LCV)) {
      if (SymbolRef Sym = binding->getAsSymbol())
        return flushPartialPtr(State, Sym, LCV->getRegion());
    }
  }

  const MemRegion* R = V.getAsRegion();
  return flushPtr(State, R);
}

ProgramStateRef flushPtr(ProgramStateRef State, const Stmt* S,
                         const LocationContext* LCtx) {
  return flushPtr(State, State->getSVal(S, LCtx));
}

bool isPtrWritten(ProgramStateRef State, const MemRegion* Reg) {
  if (!Reg)
    return false;

  // Element region (array element) is tainted if either the base or the offset
  // are tainted.
  if (const ElementRegion* ER = dyn_cast<ElementRegion>(Reg))
    return isPtrWritten(State, ER->getSuperRegion()) ||
           isPtrWritten(State, ER->getIndex());

  if (const SymbolicRegion* SR = dyn_cast<SymbolicRegion>(Reg))
    return isPtrWritten(State, SR->getSymbol());

  if (const SubRegion* ER = dyn_cast<SubRegion>(Reg))
    return isPtrWritten(State, ER->getSuperRegion());

  return false;
}

bool isPtrWritten(ProgramStateRef State, SymbolRef Sym) {
  if (!Sym)
    return false;

  // Traverse all the symbols this symbol depends on to see if any are tainted.
  for (SymExpr::symbol_iterator SI = Sym->symbol_begin(),
                                SE = Sym->symbol_end();
       SI != SE; ++SI) {
    if (!isa<SymbolData>(*SI))
      continue;

    if (State->contains<PtrSet>(*SI)) {
      return true;
    }

    if (const auto* SD = dyn_cast<SymbolDerived>(*SI)) {
      // If this is a SymbolDerived with a tainted parent, it's also tainted.
      if (isPtrWritten(State, SD->getParentSymbol()))
        return true;

      // If this is a SymbolDerived with the same parent symbol as another
      // tainted SymbolDerived and a region that's a sub-region of that tainted
      // symbol, it's also tainted.
      if (const PtrSubRegions* Regs =
              State->get<DerivedSymPtr>(SD->getParentSymbol())) {
        const TypedValueRegion* R = SD->getRegion();
        for (auto I : *Regs) {
          // FIXME: The logic to identify tainted regions could be more
          // complete. For example, this would not currently identify
          // overlapping fields in a union as tainted. To identify this we can
          // check for overlapping/nested byte offsets.
          if (R->isSubRegionOf(I))
            return true;
        }
      }
    }

    // If memory region is tainted, data is also tainted.
    if (const auto* SRV = dyn_cast<SymbolRegionValue>(*SI)) {
      if (isPtrWritten(State, SRV->getRegion()))
        return true;
    }

    // If this is a SymbolCast from a tainted value, it's also tainted.
    if (const auto* SC = dyn_cast<SymbolCast>(*SI)) {
      if (isPtrWritten(State, SC->getOperand()))
        return true;
    }
  }

  return false;
}

bool isPtrWritten(ProgramStateRef State, SVal V) {
  if (const SymExpr* Sym = V.getAsSymExpr())
    return isPtrWritten(State, Sym);
  if (const MemRegion* Reg = V.getAsRegion())
    return isPtrWritten(State, Reg);
  return false;
}

bool isPtrWritten(ProgramStateRef State, const Stmt* S,
                  const LocationContext* LCtx) {
  SVal val = State->getSVal(S, LCtx);
  return isPtrWritten(State, val);
}

} // namespace clang::ento::nvm::Transitions