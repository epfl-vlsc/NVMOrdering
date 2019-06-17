#pragma once
#include "Common.h"
#include "dataflow_util/AbstractProgram.h"

namespace clang::ento::nvm {

template <typename AbstractState, typename Vars, typename Funcs>
class PairBugReporter {
  struct BugData {
    const Stmt* bugStmt;
    const Stmt* pairStmt;
    const NamedDecl* currentVar;
    const NamedDecl* pairVar;

    BugData(const Stmt* bugStmt_, const Stmt* pairStmt_,
            const NamedDecl* currentVar_, const NamedDecl* pairVar_)
        : bugStmt(bugStmt_), pairStmt(pairStmt_), currentVar(currentVar_),
          pairVar(pairVar_) {}
  };

  using FunctionInfo = typename Funcs::FunctionInfo;
  using PairSet = std::set<const NamedDecl*>;
  using VarToPair = std::map<const NamedDecl*, PairSet>;
  using LastLocationMap = std::map<const NamedDecl*, const AbstractLocation*>;
  using BuggedVars = std::set<const NamedDecl*>;
  using BugDataList = std::vector<BugData>;

  // general data structures
  std::unique_ptr<BugType> CommitBug;
  BugReporter* BR;
  AnalysisManager* Mgr;

  Vars* vars;
  Funcs* funcs;
  FunctionInfo* activeUnitInfo;

  VarToPair* varToPair;
  LastLocationMap* lastLocationMap;
  BuggedVars* buggedVars;
  BugDataList* bugDataList;

public:
  void initAll(const CheckerBase* CB_, BugReporter* BR_, AnalysisManager* Mgr_,
               Vars* vars_, Funcs* funcs_) {
    CommitBug.reset(new BugType(CB_, "Not committed", ""));
    BR = BR_;
    Mgr = Mgr_;
    vars = vars_;
    funcs = funcs_;
    varToPair = nullptr;
    lastLocationMap = nullptr;
    buggedVars = nullptr;
    bugDataList = nullptr;
  }

  void initUnit(FunctionInfo* activeUnitInfo_) {
    activeUnitInfo = activeUnitInfo_;
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

    if (bugDataList) {
      delete bugDataList;
    }
    bugDataList = new BugDataList();
  }

  PairSet& getPairSet(const NamedDecl* ND) {
    auto& pairSet = (*varToPair)[ND];

    auto& PISet = vars->getPairInfoSet(ND);
    for (auto& PI : PISet) {
      const NamedDecl* pairND = PI->getPairND(ND);
      if (activeUnitInfo->isUsedVar(pairND)) {
        pairSet.insert(pairND);
      }
    }
    return pairSet;
  }

  void updateLastLocation(const NamedDecl* currentVar,
                          const AbstractLocation* absLocation) {
    (*lastLocationMap)[currentVar] = absLocation;
  }

  bool updateLastLocation(const NamedDecl* currentVar,
                          const AbstractStmt* absStmt, AbstractState& state) {
    updateLastLocation(currentVar, absStmt);
    checkBug(currentVar, absStmt, state);
    return true;
  }

  void reportBugs() const {
    ASTContext& ACtx = Mgr->getASTContext();
    const TranslationUnitDecl* TUD = ACtx.getTranslationUnitDecl();
    SourceManager& SM = ACtx.getSourceManager();
    AnalysisDeclContext* ADC = Mgr->getAnalysisDeclContext(TUD);

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

      BR->emitReport(std::move(R));
    }
  }

  void addAsBug(const Stmt* bugStmt, const Stmt* pairStmt,
                const NamedDecl* currentVar, const NamedDecl* pairVar) {
    buggedVars->insert(currentVar);
    buggedVars->insert(pairVar);

    bugDataList->emplace_back(bugStmt, pairStmt, currentVar, pairVar);
  }

  void checkBug(const NamedDecl* currentVar, const AbstractStmt* absStmt,
                AbstractState& state) {
    if (buggedVars->count(currentVar)) {
      return;
    }

    auto& pairSet = getPairSet(currentVar);
    for (auto& pairVar : pairSet) {
      auto& pairLattice = state[pairVar];
      if (pairLattice.isWriteFlush()) {
        printMsg("bug here");
        assert(lastLocationMap);
        const AbstractLocation* absLocation = (*lastLocationMap)[pairVar];
        assert(absLocation);
        absLocation->fullDump(Mgr);
        printMsg("");
        absStmt->fullDump(Mgr);
        printMsg("");

        const AbstractStmt* pairAbsStmt = absLocation->castAs<AbstractStmt>();
        const Stmt* bugStmt = absStmt->getStmt();
        const Stmt* pairStmt = pairAbsStmt->getStmt();

        addAsBug(bugStmt, pairStmt, currentVar, pairVar);
      }
    }
  }
};

} // namespace clang::ento::nvm
