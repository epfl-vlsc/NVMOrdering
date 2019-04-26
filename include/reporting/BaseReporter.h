#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

using BugPtr = std::unique_ptr<BugType>;

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

  void reportDirect(SVal* Loc, const std::string& ErrMsg,
                    const ExplodedNode* const EN, const BugPtr& bugPtr,
                    BugReporter& BReporter) const {
    auto Report = llvm::make_unique<BugReport>(*bugPtr, ErrMsg, EN);
    if (Loc) {
      Report->markInteresting(*Loc);
    }
    BReporter.emitReport(std::move(Report));
  }

public:
  void report(CheckerContext& C, const ValueDecl* VD, const char* msg,
              SVal* Loc, const ExplodedNode* const EN,
              const BugPtr& bugPtr) const {
    BugReporter& BReporter = C.getBugReporter();
    std::string ErrMsg = getErrorMessage(C, VD, msg);
    reportDirect(Loc, ErrMsg, EN, bugPtr, BReporter);
  }

  void report(CheckerContext& C, const NamedDecl* ND1, const NamedDecl* ND2,
              const char* msg, SVal* Loc, const ExplodedNode* const EN,
              const BugPtr& bugPtr) const {
    const NamedDecl* ND = (ND1) ? ND1 : ND2;
    BugReporter& BReporter = C.getBugReporter();
    std::string ErrMsg = getErrorMessage(C, ND, msg);
    reportDirect(Loc, ErrMsg, EN, bugPtr, BReporter);
  }
};

} // namespace clang::ento::nvm