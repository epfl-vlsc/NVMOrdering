#pragma once
#include "Common.h"

constexpr const char* CHECKER_PLUGIN_NAME = "nvm.expchecker";

namespace clang::ento::nvm {

class ExpChecker : public Checker<check::EndOfTranslationUnit> {

public:
  ExpChecker() {}

  void checkEndOfTranslationUnit(const TranslationUnitDecl* TU,
                                 AnalysisManager& Mgr, BugReporter& BR) const;

private:
};

} // namespace clang::ento::nvm
