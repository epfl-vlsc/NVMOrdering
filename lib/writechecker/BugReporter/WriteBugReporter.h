#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

class BaseBugReporter {
protected:
  // todo report model bug, valid bug
  const std::string WriteError = "NVM Write Error";

  std::unique_ptr<BugType> DataAlreadyWritten;
  std::unique_ptr<BugType> WrongModel;

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
                      BugReporter& BReporter) const{
    auto Report =
        llvm::make_unique<BugReport>(*DataAlreadyWritten, ErrMsg, ExplNode);
    Report->markInteresting(Loc);
    BReporter.emitReport(std::move(Report));
  }

public:
  BaseBugReporter(const CheckerBase& CB) {
    DataAlreadyWritten.reset(
        new BugType(&CB, "Already wrote data", WriteError));
    WrongModel.reset(new BugType(&CB, "Wrong model use", WriteError));
  }

  void reportDataAlreadyWritten(SVal Loc, CheckerContext& C,
                                const ValueDecl* VD,
                                const ExplodedNode* const ExplNode,
                                BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "already written");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }
};

class WriteBugReporter : public BaseBugReporter {
  std::unique_ptr<BugType> DataAlreadyFlushed;
  std::unique_ptr<BugType> DataAlreadyFenced;
  std::unique_ptr<BugType> DataNotFlushed;
  std::unique_ptr<BugType> DataNotFenced;
  std::unique_ptr<BugType> CheckAlreadyWritten;

public:
  WriteBugReporter(const CheckerBase& CB) : BaseBugReporter(CB) {
    DataAlreadyFlushed.reset(
        new BugType(&CB, "Already flushed data", WriteError));
    DataAlreadyFenced.reset(
        new BugType(&CB, "Already fenced data", WriteError));
    DataNotFlushed.reset(new BugType(&CB, "Data is not flushed", WriteError));
    DataNotFenced.reset(new BugType(&CB, "Data is not fenced", WriteError));
    CheckAlreadyWritten.reset(
        new BugType(&CB, "Already wrote valid", WriteError));
  }

  void reportDataAlreadyFlushed(SVal Loc, CheckerContext& C,
                                const ValueDecl* VD,
                                const ExplodedNode* const ExplNode,
                                BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "already flushed");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }

  void reportDataAlreadyFenced(SVal Loc, CheckerContext& C, const ValueDecl* VD,
                               const ExplodedNode* const ExplNode,
                               BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "already fenced");
    reportDirect(Loc, ErrMsg, ExplNode, BReporter);
  }

  void reportDataNotFlushed(SVal Loc, CheckerContext& C, const ValueDecl* VD,
                            const ExplodedNode* const ExplNode,
                            BugReporter& BReporter) const {
    std::string ErrMsg = getErrorMessage(C, VD, "not flushed");
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
};

} // namespace clang::ento::nvm