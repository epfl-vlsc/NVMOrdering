#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class BaseBugReporter {
protected:
  using BugPtr = std::unique_ptr<BugType>;
  const std::string WriteError = "NVM Write Error";

  std::string getErrorMessage(CheckerContext& C, char* D,
                              const char* msg) const {
    const FunctionDecl* FD = getTopFunction(C);
    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "At: " << FD->getName() << " " << msg;

    const Decl* BD = (const Decl*)D;
    if (const ValueDecl* VD = dyn_cast_or_null<FunctionDecl>(BD)) {
      ErrorOs << " " << VD->getNameAsString();
    }

    return ErrorOs.str();
  }

  void reportDirect(SVal Loc, const std::string& ErrMsg,
                    const ExplodedNode* const ExplNode, const BugPtr& bugPtr,
                    BugReporter& BReporter) const {
    auto Report = llvm::make_unique<BugReport>(*bugPtr, ErrMsg, ExplNode);
    Report->markInteresting(Loc);
    BReporter.emitReport(std::move(Report));
  }

public:
  void report(CheckerContext& C, char* D, const char* msg, SVal Loc,
              const ExplodedNode* const EN, const BugPtr& bugPtr) const {
    BugReporter& BReporter = C.getBugReporter();
    std::string ErrMsg = getErrorMessage(C, D, msg);
    reportDirect(Loc, ErrMsg, EN, bugPtr, BReporter);
  }
};

class DataBugReporter : public BaseBugReporter {
public:
  BugPtr DataAlreadyWritten;
  BugPtr DataNotWritten;

protected:
  DataBugReporter(const CheckerBase& CB) {
    DataAlreadyWritten.reset(
        new BugType(&CB, "Data is already written", WriteError));
    DataNotWritten.reset(new BugType(&CB, "Data is not written", WriteError));
  }
};

class FenceCheckBugReporter : public DataBugReporter {
public:
  BugPtr DataAlreadyFenced;
  BugPtr DataNotFenced;
  BugPtr CheckAlreadyWritten;
  BugPtr CheckNotWritten;

protected:
  FenceCheckBugReporter(const CheckerBase& CB) : DataBugReporter(CB) {
    DataAlreadyFenced.reset(
        new BugType(&CB, "Data is already fenced", WriteError));
    DataNotFenced.reset(new BugType(&CB, "Data is not fenced", WriteError));
    CheckAlreadyWritten.reset(
        new BugType(&CB, "Check is already written", WriteError));
    CheckNotWritten.reset(new BugType(&CB, "Check is not written", WriteError));
  }
};

class WriteBugReporter : public FenceCheckBugReporter {
public:
  BugPtr DataAlreadyFlushed;
  BugPtr DataNotFlushed;
  BugPtr DataNotPersisted;

public:
  WriteBugReporter(const CheckerBase& CB) : FenceCheckBugReporter(CB) {
    DataAlreadyFlushed.reset(
        new BugType(&CB, "Data is already flushed", WriteError));
    DataNotFlushed.reset(new BugType(&CB, "Data is not flushed", WriteError));
    DataNotPersisted.reset(
        new BugType(&CB, "Data is not persisted", WriteError));
  }
};

} // namespace clang::ento::nvm