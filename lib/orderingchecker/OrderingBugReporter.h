#pragma once
#include "Common.h"
#include "States.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

std::string getPairStr(const DeclaratorDecl* DD, const DataInfo* DI) {
  std::string sbuf;
  llvm::raw_string_ostream PairOs(sbuf);
  PairOs << "<" << DD->getNameAsString() << "," << DI->getCheckName()
         << ">:" << DI->getClStr();
  return PairOs.str();
}

struct BugInfo {
  const DeclaratorDecl* DD;
  const DataInfo* DI;
  const char* stateStr;
  int state;
  bool dcl;

public:
  BugInfo(const DeclaratorDecl* decl, const DataInfo* dataInfo,
          const char* stateS, int stateK, bool cl)
      : DD(decl), DI(dataInfo), stateStr(stateS), state(stateK), dcl(cl) {}
};

class WrongModelWalker : public BugReporterVisitorImpl<WrongModelWalker> {
  const DeclaratorDecl* DD;
  const DataInfo* DI;
  const char* stateStr;
  int state;
  bool dcl;
  bool skip;

  std::shared_ptr<PathDiagnosticPiece> reportStateDcl(const ExplodedNode* N,
                                                      const ExplodedNode* PrevN,
                                                      BugReporterContext& BRC,
                                                      BugReport& BR) {
    ProgramStateRef PrevSt = PrevN->getState();
    ProgramStateRef CurrSt = N->getState();

    const DclState* PGS = PrevSt->get<DclMap>(DD);
    const DclState* CGS = CurrSt->get<DclMap>(DD);

    const Stmt* S = PathDiagnosticLocation::getStmt(N);
    if (!S) {
      return nullptr;
    }
    std::string sbuf;
    llvm::raw_string_ostream MsgOs(sbuf);
    MsgOs << getPairStr(DD, CGS->getDataInfo()) << " " << CGS->getStateName()
          << "\n";

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

  std::shared_ptr<PathDiagnosticPiece> reportStateScl(const ExplodedNode* N,
                                                      const ExplodedNode* PrevN,
                                                      BugReporterContext& BRC,
                                                      BugReport& BR) {

    ProgramStateRef PrevSt = PrevN->getState();
    ProgramStateRef CurrSt = N->getState();

    const SclState* PGS = PrevSt->get<SclMap>(DD);
    const SclState* CGS = CurrSt->get<SclMap>(DD);

    const Stmt* S = PathDiagnosticLocation::getStmt(N);
    if (!S) {
      return nullptr;
    }
    std::string sbuf;
    llvm::raw_string_ostream MsgOs(sbuf);
    MsgOs << getPairStr(DD, CGS->getDataInfo()) << " " << CGS->getStateName()
          << "\n";

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
      : DD(BI.DD), DI(BI.DI), stateStr(BI.stateStr), state(BI.state),
        dcl(BI.dcl), skip(false) {}
  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddPointer(DD);
    ID.AddPointer(DI);
    ID.AddPointer(stateStr);
    ID.AddInteger(state);
    ID.AddBoolean(dcl);
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

    if (dcl) {
      return reportStateDcl(N, PrevN, BRC, BR);
    } else {
      return reportStateScl(N, PrevN, BRC, BR);
    }
  }
};

class OrderingBugReporter {
  const std::string OrderingError = "NVM Ordering Error";

  // path-sensitive bug types
  std::unique_ptr<BugType> WrongWriteBugType;
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
  OrderingBugReporter(const CheckerBase& CB) {
    WrongWriteBugType.reset(new BugType(&CB, "Wrong write", OrderingError));
    WrongModelBugType.reset(new BugType(&CB, "Wrong model", OrderingError));
  }

  void reportWriteBug(SVal Loc, CheckerContext& C, const DeclaratorDecl* D,
                      const ExplodedNode* const ExplNode,
                      BugReporter& BReporter) const {

    const FunctionDecl* FD = getTopFunction(C);

    std::string sbuf;
    llvm::raw_string_ostream ErrorOs(sbuf);
    ErrorOs << "Wrong write usage at " + FD->getName();

    auto Report = llvm::make_unique<BugReport>(*WrongWriteBugType,
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
            << " for pair: " << getPairStr(BI.DD, BI.DI)
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