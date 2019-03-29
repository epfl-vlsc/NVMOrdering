#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class BaseBugReporter {
protected:
  // todo report model bug, valid bug
  const std::string WriteError = "NVM Write Error";

  std::unique_ptr<BugType> DataAlreadyWritten;
  std::unique_ptr<BugType> DataNotWritten;

  std::string getErrorMessage(CheckerContext& C, const ValueDecl* VD,
                              const char* msg) const {
    const FunctionDecl* FD = getTopFunction(C);
    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "At: " << FD->getName() << " " << msg << " "
            << VD->getNameAsString();
    return ErrorOs.str();
  }

  void reportDirect(SVal Loc, const std::string& ErrMsg,
                    const ExplodedNode* const ExplNode,
                    BugReporter& BReporter) const {
    auto Report =
        llvm::make_unique<BugReport>(*DataAlreadyWritten, ErrMsg, ExplNode);
    Report->markInteresting(Loc);
    BReporter.emitReport(std::move(Report));
  }

public:
  BaseBugReporter(const CheckerBase& CB) {
    DataAlreadyWritten.reset(
        new BugType(&CB, "Data is already written", WriteError));
    DataNotWritten.reset(new BugType(&CB, "Data is not written", WriteError));
  }

  void reportDataAlreadyWritten(SVal Loc, CheckerContext& C,
                                const ValueDecl* VD,
                                const ExplodedNode* const ExplNode,
                                BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "already written");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }

  void reportDataNotWritten(SVal Loc, CheckerContext& C, const ValueDecl* VD,
                            const ExplodedNode* const ExplNode,
                            BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "not written");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }
};

class FenceCheckBugReporter : public BaseBugReporter {
  std::unique_ptr<BugType> DataAlreadyFenced;
  std::unique_ptr<BugType> DataNotFenced;
  std::unique_ptr<BugType> CheckAlreadyWritten;
  std::unique_ptr<BugType> CheckNotWritten;

public:
  FenceCheckBugReporter(const CheckerBase& CB) : BaseBugReporter(CB) {
    DataAlreadyFenced.reset(
        new BugType(&CB, "Data is already fenced", WriteError));
    DataNotFenced.reset(new BugType(&CB, "Data is not fenced", WriteError));
    CheckAlreadyWritten.reset(
        new BugType(&CB, "Check is already written", WriteError));
    CheckNotWritten.reset(new BugType(&CB, "Check is not written", WriteError));
  }

  void reportDataAlreadyFenced(SVal Loc, CheckerContext& C, const ValueDecl* VD,
                               const ExplodedNode* const ExplNode,
                               BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "already fenced");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }

  void reportDataNotFenced(SVal Loc, CheckerContext& C, const ValueDecl* VD,
                           const ExplodedNode* const ExplNode,
                           BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "not fenced");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }

  void reportCheckAlreadyWritten(SVal Loc, CheckerContext& C,
                                 const ValueDecl* VD,
                                 const ExplodedNode* const ExplNode,
                                 BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "already written");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }

  void reportCheckNotWritten(SVal Loc, CheckerContext& C, const ValueDecl* VD,
                             const ExplodedNode* const ExplNode,
                             BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "not written");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }
};

class WriteBugReporter : public FenceCheckBugReporter {
  std::unique_ptr<BugType> DataAlreadyFlushed;
  std::unique_ptr<BugType> DataNotFlushed;
  std::unique_ptr<BugType> DataNotPersisted;

public:
  WriteBugReporter(const CheckerBase& CB) : FenceCheckBugReporter(CB) {
    DataAlreadyFlushed.reset(
        new BugType(&CB, "Data is already flushed", WriteError));
    DataNotFlushed.reset(new BugType(&CB, "Data is not flushed", WriteError));
    DataNotPersisted.reset(
        new BugType(&CB, "Data is not persisted", WriteError));
  }

  void reportDataAlreadyFlushed(SVal Loc, CheckerContext& C,
                                const ValueDecl* VD,
                                const ExplodedNode* const ExplNode,
                                BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "already flushed");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }

  void reportDataNotFlushed(SVal Loc, CheckerContext& C, const ValueDecl* VD,
                            const ExplodedNode* const ExplNode,
                            BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "not flushed");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }

  void reportDataNotPersisted(SVal Loc, CheckerContext& C, const ValueDecl* VD,
                              const ExplodedNode* const ExplNode,
                              BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "not persisted");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }
};

} // namespace clang::ento::nvm