//===-- ExpChecker.cpp -----------------------------------------*
// ensure main handle functions only add one state

#include "ExpChecker.h"

namespace clang::ento::nvm {

void ExpChecker::checkASTDecl(const RecordDecl* RD, AnalysisManager& Mgr,
                              BugReporter& BR) const {
  ASTContext& ASTC = Mgr.getASTContext();
  llvm::outs() << "record: " << RD->getNameAsString() << "\n";
  for (const FieldDecl* FD : RD->fields()) {
    llvm::outs() << "field: " << FD->getNameAsString() << "\n";
    QualType QT = FD->getType();
    QT.dump();
    const Type* type = QT.getTypePtr();
    if(type->isConstantSizeType()){
      uint64_t fieldSize = ASTC.getTypeSizeInChars(type).getQuantity();
      llvm::outs() << "size: " << fieldSize << "\n";
    }
  }
}

void ExpChecker::checkASTDecl(const TranslationUnitDecl* CTUD,
                              AnalysisManager& Mgr, BugReporter& BR) const {}

void ExpChecker::checkBeginFunction(CheckerContext& C) const {}

void ExpChecker::checkEndFunction(CheckerContext& C) const {}

void ExpChecker::checkBind(SVal Loc, SVal Val, const Stmt* S,
                           CheckerContext& C) const {}

void ExpChecker::checkPreCall(const CallEvent& Call, CheckerContext& C) const {
  const Decl* BD = Call.getDecl();
  if (const FunctionDecl* FD = dyn_cast_or_null<FunctionDecl>(BD)) {
    llvm::outs() << FD->getNameAsString() << "\n";
  }
}

} // namespace clang::ento::nvm

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;

extern "C" void clang_registerCheckers(clang::ento::CheckerRegistry& registry) {
  registry.addChecker<clang::ento::nvm::ExpChecker>(
      CHECKER_PLUGIN_NAME, "Checks cache line pair usage");
}
