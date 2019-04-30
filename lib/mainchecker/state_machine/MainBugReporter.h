#pragma once
#include "Common.h"
#include "reporting/BaseReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

namespace clang::ento::nvm {

const std::string WriteError = "NVM Write Error";

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

class FFBugs {
public:
  BugPtr DataAlreadyPersisted;
  BugPtr DataNotPersisted;

protected:
  FFBugs(const CheckerBase& CB) {
    DataAlreadyPersisted.reset(
        new BugType(&CB, "Data is already persisted", WriteError));
    DataNotPersisted.reset(
        new BugType(&CB, "Data is not persisted", WriteError));
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