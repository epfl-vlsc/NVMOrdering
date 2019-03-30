#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

using BugPtr = std::unique_ptr<BugType>;
const std::string WriteError = "NVM Write Error";

class BaseReporter {
protected:
  std::string getErrorMessage(CheckerContext& C, const char* D,
                              const char* msg) const {
    const FunctionDecl* FD = getTopFunction(C);
    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "At: " << FD->getName() << " " << msg;

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

class WDBugs {
public:
  BugPtr DataAlreadyWritten;
  BugPtr DataNotWritten;

protected:
  WDBugs(const CheckerBase& CB) {
    DataAlreadyWritten.reset(
        new BugType(&CB, "Data is already written", WriteError));
    DataNotWritten.reset(new BugType(&CB, "Data is not written", WriteError));
  }
};

class WCBugs {
public:
  BugPtr CheckAlreadyWritten;
  BugPtr CheckNotWritten;

protected:
  WCBugs(const CheckerBase& CB) {
    CheckAlreadyWritten.reset(
        new BugType(&CB, "Check is already written", WriteError));
    CheckNotWritten.reset(new BugType(&CB, "Check is not written", WriteError));
  }
};

class FDBugs {
public:
  BugPtr DataAlreadyFlushed;
  BugPtr DataNotFlushed;
  BugPtr DataNotPersisted;
  BugPtr DataNotFenced;

protected:
  FDBugs(const CheckerBase& CB) {
    DataAlreadyFlushed.reset(
        new BugType(&CB, "Data is already flushed", WriteError));
    DataNotFlushed.reset(new BugType(&CB, "Data is not flushed", WriteError));
    DataNotPersisted.reset(
        new BugType(&CB, "Data is not persisted", WriteError));
    DataNotFenced.reset(new BugType(&CB, "Data is not fenced", WriteError));
  }
};

class ModelBug {
public:
  BugPtr WrongModel;

protected:
  ModelBug(const CheckerBase& CB) {
    WrongModel.reset(new BugType(&CB, "Wrong model", WriteError));
  }
};

class WriteBugReporter : public BaseReporter,
                         public WDBugs,
                         public WCBugs,
                         public FDBugs,
                         public ModelBug {

public:
  WriteBugReporter(const CheckerBase& CB)
      : WDBugs(CB), WCBugs(CB), FDBugs(CB), ModelBug(CB) {}
};

} // namespace clang::ento::nvm