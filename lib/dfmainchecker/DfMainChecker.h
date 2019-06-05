#pragma once
#include "Common.h"
#include "dataflow/Dataflow.h"
#include "flow/MainLattice.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.dfmainchecker";

namespace clang::ento::nvm {

class DfMainChecker
    : public Checker<check::ASTCodeBody, check::ASTDecl<TranslationUnitDecl>> {

public:
  DfMainChecker() {}

  void checkASTDecl(const TranslationUnitDecl* CTUD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

  void checkASTCodeBody(const Decl* D, AnalysisManager& mgr,
                        BugReporter& BR) const;

private:
  mutable MainLattice lattice;
};

} // namespace clang::ento::nvm
