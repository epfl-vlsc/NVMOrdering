#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class LogBugReporter {
  using BugPtr = std::unique_ptr<BugType>;
  const std::string LogError = "NVM Log Error";

  // path-sensitive bug types
  BugPtr WriteWithoutLogging;
  BugPtr AlreadyWritten;
  BugPtr AlreadyLogged;

  std::string getErrorMessage(const ValueDecl* VD, const char* msg,
                              CheckerContext& C) const {
    const FunctionDecl* FD = getTopFunction(C);
    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "At: " << FD->getQualifiedNameAsString() << " " << msg
            << ": " + VD->getQualifiedNameAsString() << "\n";
    return ErrorOs.str();
  }

  void report(const ValueDecl* VD, const char* msg, const BugPtr& bugPtr,
              CheckerContext& C) const {
    ExplodedNode* ErrNode = C.generateErrorNode();
    if (!ErrNode)
      return;

    BugReporter& BReporter = C.getBugReporter();
    std::string ErrMsg = getErrorMessage(VD, msg, C);
    auto Report = llvm::make_unique<BugReport>(*bugPtr, ErrMsg, ErrNode);

    BReporter.emitReport(std::move(Report));
  }

public:
  LogBugReporter(const CheckerBase& CB) {
    WriteWithoutLogging.reset(
        new BugType(&CB, "Writing without logging", LogError));
    AlreadyWritten.reset(new BugType(&CB, "Already written", LogError));
    AlreadyLogged.reset(new BugType(&CB, "Already logged", LogError));
  }

  void reportWriteWithoutLogging(const ValueDecl* VD, CheckerContext& C) const {
    report(VD, "Writing without logging", WriteWithoutLogging, C);
  }

  void reportAlreadyWritten(const ValueDecl* VD, CheckerContext& C) const {
    report(VD, "Already written", AlreadyWritten, C);
  }

  void reportAlreadyLogged(const ValueDecl* VD, CheckerContext& C) const {
    report(VD, "Already logged", AlreadyLogged, C);
  }
};

} // namespace clang::ento::nvm