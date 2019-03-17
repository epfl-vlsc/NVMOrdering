#pragma once
#include "Common.h"
#include "States.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

struct BugInfo {
  const DeclaratorDecl* DD;
  const CheckInfo* CI;
  const char* stateStr;
  int state;

public:
  BugInfo(const DeclaratorDecl* DD_, const CheckInfo* CI_,
          const char* stateStr_, int state_)
      : DD(DD_), CI(CI_), stateStr(stateStr_), state(state_) {}
};

class WrongModelWalker : public BugReporterVisitorImpl<WrongModelWalker> {
  const DeclaratorDecl* DD;
  const CheckInfo* CI;
  const char* stateStr;
  int state;
  bool skip;

  std::shared_ptr<PathDiagnosticPiece> reportStateRec(const ExplodedNode* N,
                                                      const ExplodedNode* PrevN,
                                                      BugReporterContext& BRC,
                                                      BugReport& BR) {
    ProgramStateRef PrevSt = PrevN->getState();
    ProgramStateRef CurrSt = N->getState();

    const RecState* PGS = PrevSt->get<RecMap>(DD);
    const RecState* CGS = CurrSt->get<RecMap>(DD);

    const Stmt* S = PathDiagnosticLocation::getStmt(N);
    if (!S) {
      return nullptr;
    }
    std::string sbuf;
    llvm::raw_string_ostream MsgOs(sbuf);
    MsgOs << "";

    if (PGS) {
      // the pair exists in previous state
      if (*PGS == *CGS) {
        // if same, not interesting
        return nullptr;
      } else {
        PathDiagnosticLocation Pos(S, BRC.getSourceManager(),
                                   N->getLocationContext());
        return std::make_shared<PathDiagnosticEventPiece>(Pos, MsgOs.str());
      }
    } else {
      // the pair does not exist in previous state
      // todo stop going back
      skip = true;
      PathDiagnosticLocation Pos(S, BRC.getSourceManager(),
                                 N->getLocationContext());
      return std::make_shared<PathDiagnosticEventPiece>(Pos, MsgOs.str());
    }
  }

public:
  WrongModelWalker(const BugInfo& BI)
      : DD(BI.DD), CI(BI.CI), stateStr(BI.stateStr), state(BI.state),
        skip(false) {}
  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddPointer(DD);
    ID.AddPointer(CI);
    ID.AddPointer(stateStr);
    ID.AddInteger(state);
    ID.AddBoolean(skip);
  }

  std::shared_ptr<PathDiagnosticPiece> VisitNode(const ExplodedNode* N,
                                                 const ExplodedNode* PrevN,
                                                 BugReporterContext& BRC,
                                                 BugReport& BR) {
    // todo optimize if possible stop going back
    if (skip) {
      return nullptr;
    }

    return reportStateRec(N, PrevN, BRC, BR);
  }
};

class RecoveryBugReporter {
  const std::string RecoveryError = "NVM Recovery Error";

  // path-sensitive bug types
  std::unique_ptr<BugType> WrongReadBugType;
  std::unique_ptr<BugType> WrongModelBugType;

  const FunctionDecl* getTopFunction(CheckerContext& C) const {
    LocationContext* LC = (LocationContext*)C.getLocationContext();
    while (LC && LC->getParent()) {
      LC = (LocationContext*)LC->getParent();
    }

    const Decl* BD = LC->getDecl();
    if (const FunctionDecl* FD = dyn_cast<FunctionDecl>(BD)) {
      return FD;
    } else {
      llvm::report_fatal_error("always stack function");
      return nullptr;
    }
  }

public:
  RecoveryBugReporter(const CheckerBase& CB) {
    WrongReadBugType.reset(new BugType(&CB, "Wrong read", RecoveryError));
    WrongModelBugType.reset(new BugType(&CB, "Wrong model", RecoveryError));
  }

  void reportReadBug(SVal Loc, CheckerContext& C, const DeclaratorDecl* D,
                      const ExplodedNode* const ExplNode,
                      BugReporter& BReporter) const {

    const FunctionDecl* FD = getTopFunction(C);

    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "Wrong write usage at " + FD->getName();

    auto Report = llvm::make_unique<BugReport>(*WrongReadBugType,
                                               ErrorOs.str(), ExplNode);
    Report->markInteresting(Loc);
    BReporter.emitReport(std::move(Report));
  }

  void reportModelBug(CheckerContext& C, const BugInfo& BI,
                      const ExplodedNode* const ExplNode,
                      BugReporter& BReporter) const {

    const LocationContext* LC = C.getLocationContext();
    const FunctionDecl* FD = getFuncDecl(LC);

    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "Wrong state: " << BI.stateStr
            << " at function: " << FD->getNameAsString();
    auto Report = llvm::make_unique<BugReport>(*WrongModelBugType,
                                               ErrorOs.str(), ExplNode);
    Report->markInteresting(LC);
    Report->addRange(FD->getSourceRange());
    Report->addVisitor(llvm::make_unique<WrongModelWalker>(BI));
    BReporter.emitReport(std::move(Report));
  }
};

} // namespace clang::ento::nvm