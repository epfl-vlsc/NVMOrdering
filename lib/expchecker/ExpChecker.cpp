//===-- ExpChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "ExpChecker.h"

namespace clang::ento::nvm {

void ExpChecker::checkASTDecl(const RecordDecl* RD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  /*
ASTContext& ASTC = Mgr.getASTContext();
llvm::outs() << "record: " << RD->getNameAsString() << "\n";

const unsigned CACHE_LINE_SIZE = 64;
unsigned current = 0;
unsigned CL = 0;
for (const FieldDecl* FD : RD->fields()) {
llvm::outs() << "field: " << FD->getNameAsString() << "\n";
QualType QT = FD->getType();
QT.dump();
const Type* type = QT.getTypePtr();
if (type->isConstantSizeType()) {
uint64_t fieldSize = ASTC.getTypeSizeInChars(type).getQuantity();
llvm::outs() << "size: " << fieldSize << "\n";
current += fieldSize;
CL = current / CACHE_LINE_SIZE;
llvm::outs() << "currently at CL: " << CL << "\n";
} else {
// fallback to dcl
}
}
*/
}

void ExpChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                              AnalysisManager& Mgr, BugReporter& BR) const {}

void ExpChecker::checkBeginFunction(CheckerContext& C) const {
  const LocationContext* LC = C.getLocationContext();
  const Decl* BD = LC->getDecl();
  if (const FunctionDecl* FD = dyn_cast_or_null<FunctionDecl>(BD)) {
    auto fncName = FD->getNameAsString();
    if (C.inTopFrame() && fncName == "insert") {
      ExplodedNode* ErrNode = C.generateErrorNode();
      if (!ErrNode)
        return;
    }
  }
}

void ExpChecker::checkEndFunction(CheckerContext& C) const {}

void ExpChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {

  llvm::errs() << "bind:";
  Loc.dump();
  llvm::errs() << "\n";
}

void printArg(const CallEvent& Call, CheckerContext& C) {
  SVal Loc = Call.getArgSVal(0);
  unsigned kind = Loc.getRawKind();
  llvm::errs() << "arg:";
  Loc.dump();
  llvm::errs() << " " << kind;
  llvm::errs() << "\n";

  if (kind == 11) {
    nonloc::LazyCompoundVal LCV = Loc.castAs<nonloc::LazyCompoundVal>();
    const TypedValueRegion* TVR = LCV.getRegion();
    llvm::errs() << "tvr:";
    TVR->dump();
    llvm::errs() << "\n";

    const MemRegion* MR = TVR->getSuperRegion();
    if (MR) {
      llvm::errs() << "mr:";
      MR->dump();
      llvm::errs() << "\n";
    }
  }
}

void printReturn(const CallEvent& Call, CheckerContext& C) {
  SVal Loc = Call.getReturnValue();
  unsigned kind = Loc.getRawKind();
  llvm::errs() << "return:";
  Loc.dump();
  llvm::errs() << " " << kind;
  llvm::errs() << "\n";
}

void ExpChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  const Decl* BD = Call.getDecl();
  if (const FunctionDecl* FD = dyn_cast_or_null<FunctionDecl>(BD)) {
    std::string fncName = FD->getNameAsString();
    llvm::errs() << "function:";
    llvm::errs() << fncName << "\n";

    if (fncName == "pmemobj_tx_add_range_direct" ||
        fncName == "pmemobj_tx_add_range" || fncName == "pmemobj_direct") {
      printArg(Call, C);
    }

    if (fncName == "pmemobj_direct") {
      printReturn(Call, C);
    }
  }
}

bool ExpChecker::evalCall(const CallExpr* CE, CheckerContext& C) const {
  const FunctionDecl* FD = C.getCalleeDecl(CE);
  if (!FD)
    return false;

  const IdentifierInfo* II = FD->getIdentifier();
  if (!II || !II->isStr("pmemobj_direct")) {
    return false;
  }

  llvm::errs() << "eval:\n";
  ProgramStateRef State = C.getState();
  const LocationContext* LC = C.getLocationContext();
  const Expr* Arg0 = CE->getArg(0);
  SVal Loc = State->getSVal(Arg0, LC);
  unsigned kind = Loc.getRawKind();
  if (kind == 11) {
    nonloc::LazyCompoundVal LCV = Loc.castAs<nonloc::LazyCompoundVal>();
    const TypedValueRegion* TVR = LCV.getRegion();

    const MemRegion* MR = TVR->getBaseRegion();

    SVal RetVal = State->getSVal(MR);
    State = State->BindExpr(CE, LC, RetVal);
    C.addTransition(State);
  }

  return true;
}

void ExpChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ExpChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
