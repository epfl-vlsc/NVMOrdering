#pragma once
#include "Common.h"
//#include "analyzer/FlushedAnalyzer.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.flushedchecker";

namespace clang::ento::nvm {

class FlushedChecker : public Checker<check::EndOfTranslationUnit> {

public:
  FlushedChecker() {}

  void checkEndOfTranslationUnit(const TranslationUnitDecl* TUD,
                                 AnalysisManager& Mgr, BugReporter& BR) const;
};

} // namespace clang::ento::nvm
