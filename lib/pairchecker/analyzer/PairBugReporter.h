#pragma once
#include "Common.h"

namespace clang::ento::nvm {

template <typename Globals> class PairBugReporter {
  using LatVar = typename Globals::LatVar;

  struct BugData {
    const Stmt* bugStmt;
    const Stmt* pairStmt;
    LatVar currentVar;
    LatVar pairVar;

    BugData(const Stmt* bugStmt_, const Stmt* pairStmt_, LatVar currentVar_,
            LatVar pairVar_)
        : bugStmt(bugStmt_), pairStmt(pairStmt_), currentVar(currentVar_),
          pairVar(pairVar_) {}
  };

  using PairSet = std::set<LatVar>;
  using VarToPair = std::map<LatVar, PairSet>;
  using LastLocationMap = std::map<LatVar, const Stmt*>;
  using BuggedVars = std::set<LatVar>;
  using BugDataList = std::vector<BugData>;

  // general data structures
  std::unique_ptr<BugType> CommitBug;
  Globals& globals;
  AnalysisManager& Mgr;
  BugReporter& BR;

  BugDataList* bugDataList;
  VarToPair* varToPair;
  LastLocationMap* lastLocationMap;
  BuggedVars* buggedVars;
public:
  PairBugReporter(Globals& globals_, AnalysisManager& Mgr_, BugReporter& BR_,
                  const CheckerBase* CB_)
      : globals(globals_), Mgr(Mgr_), BR(BR_) {
    CommitBug.reset(new BugType(CB_, "Not committed", ""));
    bugDataList = new BugDataList();
    varToPair = nullptr;
    lastLocationMap = nullptr;
    buggedVars = nullptr;
  }

  void initUnit() {
    // to find pairs quickly
    if (varToPair) {
      delete varToPair;
    }
    varToPair = new VarToPair;

    if (lastLocationMap) {
      delete lastLocationMap;
    }
    lastLocationMap = new LastLocationMap();

    if (buggedVars) {
      delete buggedVars;
    }
    buggedVars = new BuggedVars();    
  }

  PairSet& getPairSet(LatVar ND) {
    auto& pairSet = (*varToPair)[ND];

    auto& PISet = globals.getPairInfoSet(ND);
    for (auto& PI : PISet) {
      LatVar pairND = PI->getPairND(ND);
      if (globals.isUsedVarUnit(pairND)) {
        pairSet.insert(pairND);
      }
    }
    return pairSet;
  }

  void updateLastLocation(LatVar var, const Stmt* S) {
    (*lastLocationMap)[var] = S;
  }

  template <typename AbstractState>
  bool updateLastLocation(LatVar var, const Stmt* S,
                          AbstractState& state) {
    updateLastLocation(var, S);
    checkBug(var, S, state);
    return true;
  }

  void reportBugs() const {
    ASTContext& ACtx = Mgr.getASTContext();
    const TranslationUnitDecl* TUD = ACtx.getTranslationUnitDecl();
    SourceManager& SM = ACtx.getSourceManager();
    AnalysisDeclContext* ADC = Mgr.getAnalysisDeclContext(TUD);

    for (auto& [bugStmt, pairStmt, currentVar, pairVar] : *bugDataList) {
      SmallString<100> buf;
      llvm::raw_svector_ostream os(buf);
      os << "Error writing " << currentVar->getQualifiedNameAsString()
         << " commit " << pairVar->getQualifiedNameAsString() << "\n";

      SmallString<100> buf2;
      llvm::raw_svector_ostream os2(buf2);
      os2 << "Commit " << pairVar->getQualifiedNameAsString() << "\n";

      auto L = PathDiagnosticLocation::createBegin(bugStmt, SM, ADC);
      auto R = llvm::make_unique<BugReport>(*CommitBug, os.str(), L);

      auto L2 = PathDiagnosticLocation::createBegin(pairStmt, SM, ADC);
      R->addNote(os2.str(), L2, pairStmt->getSourceRange());

      BR.emitReport(std::move(R));
    }
  }

  void addAsBug(const Stmt* bugStmt, const Stmt* pairStmt,
                LatVar currentVar, LatVar pairVar) {
    buggedVars->insert(currentVar);
    buggedVars->insert(pairVar);

    bugDataList->emplace_back(bugStmt, pairStmt, currentVar, pairVar);
  }

  template <typename AbstractState>
  void checkBug(LatVar currentVar, const Stmt* bugStmt,
                AbstractState& state) {
    if (buggedVars->count(currentVar)) {
      return;
    }

    auto& pairSet = getPairSet(currentVar);
    for (auto& pairVar : pairSet) {
      auto& pairLattice = state[pairVar];
      if (pairLattice.isWriteFlush()) {
        printMsg("bug here");
        assert(lastLocationMap && lastLocationMap->count(pairVar));
        const Stmt* pairStmt = (*lastLocationMap)[pairVar];
        addAsBug(bugStmt, pairStmt, currentVar, pairVar);
      }
    }
  }
};

} // namespace clang::ento::nvm
