#pragma once
#include "Common.h"
#include "analyzer/LogAnalyzer.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.logchecker";

namespace clang::ento::nvm {

class LogChecker : public Checker<check::EndOfTranslationUnit> {

public:
  LogChecker() {}

  void checkEndOfTranslationUnit(const TranslationUnitDecl* TUD,
                                 AnalysisManager& Mgr, BugReporter& BR) const;
};

} // namespace clang::ento::nvm
