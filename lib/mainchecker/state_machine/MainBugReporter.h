#pragma once
#include "Common.h"
#include "reporting/BaseReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

const std::string WriteError = "NVM Write Error";

class WDBugs {
public:
  BugPtr DataAlreadyCommitted;
  BugPtr DataNotCommitted;
  BugPtr DataNotWritten;

protected:
  WDBugs(const CheckerBase& CB) {
    DataAlreadyCommitted.reset(
        new BugType(&CB, "Data is already committed", WriteError));
    DataNotCommitted.reset(
        new BugType(&CB, "Data is not committed", WriteError));
    DataNotWritten.reset(new BugType(&CB, "Data is not written", WriteError));
  }
};

class WCBugs {
public:
  BugPtr CheckAlreadyCommitted;
  BugPtr CheckNotCommitted;
  BugPtr CheckNotWritten;

protected:
  WCBugs(const CheckerBase& CB) {
    CheckAlreadyCommitted.reset(
        new BugType(&CB, "Check is already committed", WriteError));
    CheckNotCommitted.reset(
        new BugType(&CB, "Check is not committed", WriteError));
    CheckNotWritten.reset(new BugType(&CB, "Check is not written", WriteError));
  }
};

class FFBugs {
public:
  BugPtr NotPossible;

protected:
  FFBugs(const CheckerBase& CB) {
    NotPossible.reset(new BugType(&CB, "Not possible", WriteError));
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

class MainBugReporter : public BaseReporter,
                        public WDBugs,
                        public WCBugs,
                        public FFBugs,
                        public ModelBug {

public:
  MainBugReporter(const CheckerBase& CB)
      : WDBugs(CB), WCBugs(CB), FFBugs(CB), ModelBug(CB) {}
};

} // namespace clang::ento::nvm