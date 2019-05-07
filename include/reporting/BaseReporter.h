#pragma once
#include "Common.h"
#include "SourceLineStorage.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
namespace clang::ento::nvm {

using BugPtr = std::unique_ptr<BugType>;

struct BugReportData {
  const NamedDecl* ND;
  ProgramStateRef& State;
  CheckerContext& C;
  const ExplodedNode* const EN;
  const char* msg;
  const BugPtr& bugPtr;
};

class BaseReporter {
protected:
  std::string getErrorMessage(CheckerContext& C, const NamedDecl* ND,
                              const char* msg) const {
    const FunctionDecl* FD = getTopFunction(C);
    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "At: " << FD->getQualifiedNameAsString() << " " << msg;

    if (ND) {
      ErrorOs << " " << ND->getNameAsString();
    }

    return ErrorOs.str();
  }

public:
  void report(BugReportData& BRData) const {
    auto& [ND, State, C, EN, msg, bugPtr] = BRData;
    BugReporter& BReporter = C.getBugReporter();
    std::string ErrMsg = getErrorMessage(C, ND, msg);
    auto Report = llvm::make_unique<BugReport>(*bugPtr, ErrMsg, EN);

    TransitionStoreTy LineStore = SlSpace::getSRStore(State);
    if (!LineStore.isEmpty()) {
      for (auto slRange : LineStore) {
        Report->addRange(slRange.getSR());
      }
    }

    BReporter.emitReport(std::move(Report));
  }
};

} // namespace clang::ento::nvm