#pragma once
#include "Common.h"

namespace clang::ento::nvm {

template <typename Globals> class FlushedBugReporter {
  using LatVar = typename Globals::LatVar;

  struct BugData {
    const Stmt* fieldStmt;
    const Stmt* trackedStmt;
    LatVar fieldVar;
    LatVar trackedVar;

    BugData(const Stmt* fieldStmt_, const Stmt* trackedStmt_, LatVar fieldVar_,
            LatVar trackedVar_)
        : fieldStmt(fieldStmt_), trackedStmt(trackedStmt_), fieldVar(fieldVar_),
          trackedVar(trackedVar_) {}
  };

  using LastLocationMap = std::map<LatVar, const Stmt*>;
  using BuggedVars = std::set<LatVar>;
  using BugDataList = std::vector<BugData>;

  // general data structures
  std::unique_ptr<BugType> FlushedBug;
  Globals& globals;
  AnalysisManager& Mgr;
  BugReporter& BR;

  BugDataList* bugDataList;
  LastLocationMap* lastLocationMap;
  BuggedVars* buggedVars;

public:
  FlushedBugReporter(Globals& globals_, AnalysisManager& Mgr_, BugReporter& BR_,
                     const CheckerBase* CB_)
      : globals(globals_), Mgr(Mgr_), BR(BR_) {
    FlushedBug.reset(new BugType(CB_, "Not committed", ""));
    bugDataList = new BugDataList();
    lastLocationMap = nullptr;
    buggedVars = nullptr;
  }

  void initUnit() {
    if (lastLocationMap) {
      delete lastLocationMap;
    }
    lastLocationMap = new LastLocationMap();

    if (buggedVars) {
      delete buggedVars;
    }
    buggedVars = new BuggedVars();
  }

  void updateLastLocation(LatVar var, const Stmt* S) {
    (*lastLocationMap)[var] = S;
  }

  void reportBugs() const {
    ASTContext& ACtx = Mgr.getASTContext();
    const TranslationUnitDecl* TUD = ACtx.getTranslationUnitDecl();
    SourceManager& SM = ACtx.getSourceManager();
    AnalysisDeclContext* ADC = Mgr.getAnalysisDeclContext(TUD);

    for (auto& [fieldStmt, trackedStmt, fieldVar, trackedVar] : *bugDataList) {
      SmallString<100> buf;
      llvm::raw_svector_ostream os(buf);
      os << "Error writing " << fieldVar->getQualifiedNameAsString()
         << " commit " << trackedVar->getQualifiedNameAsString() << "\n";

      auto L = PathDiagnosticLocation::createBegin(fieldStmt, SM, ADC);
      auto R = llvm::make_unique<BugReport>(*FlushedBug, os.str(), L);

      if (trackedStmt) {
        SmallString<100> buf2;
        llvm::raw_svector_ostream os2(buf2);
        os2 << "Commit " << trackedVar->getQualifiedNameAsString() << "\n";

        auto L2 = PathDiagnosticLocation::createBegin(trackedStmt, SM, ADC);
        R->addNote(os2.str(), L2, trackedStmt->getSourceRange());
      };

      BR.emitReport(std::move(R));
    }
  }

  void addAsBug(const Stmt* fieldStmt, const Stmt* trackedStmt, LatVar fieldVar,
                LatVar trackedVar) {
    bugDataList->emplace_back(fieldStmt, trackedStmt, fieldVar, trackedVar);
  }

  const Stmt* getLastLocation(LatVar var) const {
    if (lastLocationMap->count(var)) {
      return (*lastLocationMap)[var];
    }
    return nullptr;
  }

  template <typename AbstractState>
  void checkBug(LatVar fieldVar, LatVar trackedVar, const Stmt* fieldStmt,
                AbstractState& state) {
    if (buggedVars->count(fieldVar)) {
      return;
    }

    auto& lv = state[trackedVar];
    if (!lv.isPfence()) {
      const Stmt* trackedStmt = getLastLocation(trackedVar);
      printMsg("bug here");
      addAsBug(fieldStmt, trackedStmt, fieldVar, trackedVar);
    }
  }
};

} // namespace clang::ento::nvm
