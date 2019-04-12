#pragma once
#include "Common.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "reporting/BaseReporter.h"

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

class FDBugs {
public:
  BugPtr DataAlreadyFlushed;
  BugPtr DataNotFlushed;
  BugPtr DataNotPersisted;
  BugPtr DataNotFenced;

protected:
  FDBugs(const CheckerBase& CB) {
    DataAlreadyFlushed.reset(
        new BugType(&CB, "Data is already flushed", WriteError));
    DataNotFlushed.reset(new BugType(&CB, "Data is not flushed", WriteError));
    DataNotPersisted.reset(
        new BugType(&CB, "Data is not persisted", WriteError));
    DataNotFenced.reset(new BugType(&CB, "Data is not fenced", WriteError));
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

class WriteBugReporter : public BaseReporter,
                         public WDBugs,
                         public WCBugs,
                         public FDBugs,
                         public ModelBug {

public:
  WriteBugReporter(const CheckerBase& CB)
      : WDBugs(CB), WCBugs(CB), FDBugs(CB), ModelBug(CB) {}
};

} // namespace clang::ento::nvm