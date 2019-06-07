#pragma once
#include "Common.h"
#include "analyzer/PairAnalyzer.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.pairchecker";

namespace clang::ento::nvm {

class PairChecker
    : public Checker<check::ASTDecl<TranslationUnitDecl>> {

public:
  PairChecker() {}

  void checkASTDecl(const TranslationUnitDecl* CTUD, AnalysisManager& Mgr,
                    BugReporter& BR) const;

private:
  mutable PairAnalyzer analyzer;
};

} // namespace clang::ento::nvm
