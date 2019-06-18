#pragma once
#include "Common.h"

namespace clang::ento::nvm {

template <typename Globals> class LogBugReporter {
  using LatVar = typename Globals::LatVar;

  struct BugData {
    enum BugType { DoubleLog, OutsideTx, NotLogged };
    const Stmt* bugStmt;
    const Stmt* logStmt;
    LatVar currentVar;
    BugType bugType;

    BugData(const Stmt* bugStmt_, const Stmt* logStmt_, LatVar currentVar_,
            BugType bugType_)
        : bugStmt(bugStmt_), logStmt(logStmt_), currentVar(currentVar_),
          bugType(bugType_) {}

    static BugData getDoubleLogBug(const Stmt* bugStmt_, const Stmt* logStmt_,
                                   LatVar currentVar_) {
      return BugData(bugStmt_, logStmt_, currentVar_, DoubleLog);
    }

    static BugData getOutsideTx(const Stmt* bugStmt_, LatVar currentVar_) {
      return BugData(bugStmt_, nullptr, currentVar_, OutsideTx);
    }

    static BugData getNotLogged(const Stmt* bugStmt_, LatVar currentVar_) {
      return BugData(bugStmt_, nullptr, currentVar_, NotLogged);
    }

    bool isDoubleLog() const { return bugType == DoubleLog; }

    bool isOutsideTx() const { return bugType == OutsideTx; }

    bool isNotLogged() const { return bugType == NotLogged; }

    std::string getMsg() const {
      SmallString<100> buf;
      llvm::raw_svector_ostream os(buf);

      switch (bugType) {
      case DoubleLog:
        os << "Double log " << currentVar->getQualifiedNameAsString() << "\n";
        break;
      case OutsideTx:
        os << "Access to " << currentVar->getQualifiedNameAsString()
           << " outside transaction\n";
        break;
      case NotLogged:
        os << currentVar->getQualifiedNameAsString() << " is not logged\n";
        break;
      default: {
        llvm::report_fatal_error("");
        break;
      }
      }

      return os.str().str();
    }

    std::string getLogStmtMsg() const {
      SmallString<100> buf;
      llvm::raw_svector_ostream os(buf);
      os << currentVar->getQualifiedNameAsString() << " is logged here first\n";
      return os.str().str();
    }
  };

  using BugDataList = std::vector<BugData>;
  using LastLocationMap = std::map<LatVar, const Stmt*>;
  using BuggedVars = std::set<LatVar>;

  // general data structures
  std::unique_ptr<BugType> DoubleLogBug;
  std::unique_ptr<BugType> OutsideTxBug;
  std::unique_ptr<BugType> NotLoggedBug;

  Globals& globals;
  AnalysisManager& Mgr;
  BugReporter& BR;

  BugDataList* bugDataList;
  LastLocationMap* lastLocationMap;
  BuggedVars* buggedVars;

public:
  LogBugReporter(Globals& globals_, AnalysisManager& Mgr_, BugReporter& BR_,
                 const CheckerBase* CB_)
      : globals(globals_), Mgr(Mgr_), BR(BR_) {
    DoubleLogBug.reset(new BugType(CB_, "Double log", ""));
    OutsideTxBug.reset(new BugType(CB_, "Outside tx access", ""));
    NotLoggedBug.reset(new BugType(CB_, "Not logged", ""));
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

  auto& getBugPtr(const BugData& bugData) const {
    if (bugData.isDoubleLog()) {
      return DoubleLogBug;
    } else if (bugData.isOutsideTx()) {
      return OutsideTxBug;
    } else if (bugData.isNotLogged()) {
      return NotLoggedBug;
    } else {
      llvm::report_fatal_error("not a valid configuration");
      return NotLoggedBug;
    }
  }

  void reportBugs() const {
    ASTContext& ACtx = Mgr.getASTContext();
    const TranslationUnitDecl* TUD = ACtx.getTranslationUnitDecl();
    SourceManager& SM = ACtx.getSourceManager();
    AnalysisDeclContext* ADC = Mgr.getAnalysisDeclContext(TUD);

    for (auto& bugData : *bugDataList) {
      auto& [bugStmt, logStmt, currentVar, bugType] = bugData;

      auto msg = bugData.getMsg();
      auto& bugPtr = getBugPtr(bugData);

      auto L = PathDiagnosticLocation::createBegin(bugStmt, SM, ADC);
      auto R = llvm::make_unique<BugReport>(*bugPtr, msg, L);

      if (bugData.isDoubleLog()) {
        auto logStmtMsg = bugData.getLogStmtMsg();
        auto L2 = PathDiagnosticLocation::createBegin(logStmt, SM, ADC);
        R->addNote(logStmtMsg, L2, logStmt->getSourceRange());
      }

      llvm::errs() << R->getDescription() << "\n";
      BR.emitReport(std::move(R));
    }
  }

  template <typename AbstractState>
  void checkOutsideTxBug(LatVar currentVar, const Stmt* bugStmt,
                         AbstractState& state) {
    auto tx = globals.getTxFunction();
    auto& lv = state[tx];

    if (!lv.isInTx()) {
      printMsg("OutsideTx bug here");
      buggedVars->insert(currentVar);
      auto bugData = BugData::getOutsideTx(bugStmt, currentVar);
      bugDataList->push_back(bugData);
    }
  }

  template <typename AbstractState>
  void checkDoubleLogBug(LatVar currentVar, const Stmt* bugStmt,
                         AbstractState& state) {
    if (buggedVars->count(currentVar)) {
      return;
    }

    checkOutsideTxBug(currentVar, bugStmt, state);

    auto& lv = state[currentVar];
    if (lv.isLogged()) {
      printMsg("DoubleLog bug here");
      buggedVars->insert(currentVar);
      const Stmt* logStmt = (*lastLocationMap)[currentVar];
      auto bugData = BugData::getDoubleLogBug(bugStmt, logStmt, currentVar);
      bugDataList->push_back(bugData);
    }
  }

  template <typename AbstractState>
  void checkNotLoggedBug(LatVar currentVar, const Stmt* bugStmt,
                         AbstractState& state) {
    if (buggedVars->count(currentVar)) {
      return;
    }

    checkOutsideTxBug(currentVar, bugStmt, state);

    auto& lv = state[currentVar];
    if (!lv.isLogged()) {
      printMsg("NotLogged bug here");
      buggedVars->insert(currentVar);
      auto bugData = BugData::getNotLogged(bugStmt, currentVar);
      bugDataList->push_back(bugData);
    }
  }
};

} // namespace clang::ento::nvm
