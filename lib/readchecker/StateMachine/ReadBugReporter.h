#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

using BugPtr = std::unique_ptr<BugType>;
const std::string ReadError = "NVM Read Error";

class BaseReporter {
protected:
  std::string getErrorMessage(CheckerContext& C, const char* D,
                              const char* msg) const {
    const FunctionDecl* FD = getTopFunction(C);
    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "At: " << FD->getQualifiedNameAsString() << " " << msg;

    const Decl* BD = (const Decl*)D;
    if (const ValueDecl* VD = dyn_cast_or_null<ValueDecl>(BD)) {
      ErrorOs << " " << VD->getNameAsString();
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
  void report(CheckerContext& C, const char* D, const char* msg, SVal* Loc,
              const ExplodedNode* const EN, const BugPtr& bugPtr) const {
    BugReporter& BReporter = C.getBugReporter();
    std::string ErrMsg = getErrorMessage(C, D, msg);
    reportDirect(Loc, ErrMsg, EN, bugPtr, BReporter);
  }
};

class CheckBugs {
public:
  BugPtr CheckNotRead;

protected:
  CheckBugs(const CheckerBase& CB) {
    CheckNotRead.reset(new BugType(&CB, "Check is not read", ReadError));
  }
};

class ReadBugReporter : public BaseReporter, public CheckBugs {

public:
  ReadBugReporter(const CheckerBase& CB) : CheckBugs(CB) {}
};

} // namespace clang::ento::nvm