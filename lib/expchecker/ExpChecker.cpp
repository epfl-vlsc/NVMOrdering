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

void ExpChecker::checkBeginFunction(CheckerContext& C) const {}

void ExpChecker::checkEndFunction(CheckerContext& C) const {}

bool ExpChecker::evalCall(const CallExpr* CE, CheckerContext& C) const {
  llvm::errs() << "eval:";
  CE->dumpPretty(C.getASTContext());
  llvm::errs() << "\n";
  return false;
}

void ExpChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {

  llvm::errs() << "bind:";
  Loc.dump();
  llvm::errs() << "\n";
}

void ExpChecker::checkPostCall(const CallEvent& Call, CheckerContext& C) const {
  const Decl* BD = Call.getDecl();
  if (const FunctionDecl* FD = dyn_cast_or_null<FunctionDecl>(BD)) {
    llvm::errs() << "function:";
    llvm::errs() << FD->getNameAsString() << "\n";
  }
}

void ExpChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ExpChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
