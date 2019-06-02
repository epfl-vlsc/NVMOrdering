#pragma once
#include "Common.h"
#include "main_util/Parser.h"
#include "dataflow/Dataflow.h"

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
  mutable MainFncs mainFncs;
  mutable MainVars mainVars;
};

} // namespace clang::ento::nvm
