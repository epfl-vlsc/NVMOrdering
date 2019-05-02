#pragma once
#include "Common.h"
#include "MainFncs.h"
#include "MainVars.h"
#include "PairInfo.h"
#include "walkers/OrderWalker.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

class MainWalker : public RecursiveASTVisitor<MainWalker> {
  using PairStrInfo = std::pair<std::string, std::string>;
  using PSIList = std::vector<PairStrInfo>;
  using StrToND = std::map<std::string, const NamedDecl*>;

  static constexpr const char* PAIR = "pair";
  static constexpr const char* KEY_SEP = "-";

  MainVars& mainVars;
  MainFncs& mainFncs;

  PSIList psiList;
  StrToND strToND;

  void addAnnotated(const NamedDecl* ND) {
    std::string data = ND->getQualifiedNameAsString();
    strToND[data] = ND;

    for (const auto* Ann : ND->specific_attrs<AnnotateAttr>()) {
      StringRef annotation = Ann->getAnnotation();
      if (annotation.contains(PAIR)) {
        auto [pairStrRef, checkNameRef] = annotation.split(KEY_SEP);
        if (checkNameRef.empty()) {
          llvm::report_fatal_error("check cannot be empty");
        }

        std::string check = checkNameRef.str();
        auto pairStrInfo = std::pair(data, check);
        psiList.push_back(pairStrInfo);
      }
    }
  }

  const NamedDecl* getNDFromStr(const std::string& str) {
    // ensures valid ND
    if (!strToND.count(str)) {
      llvm::errs() << str << "\n";
      llvm::report_fatal_error("not tracked correctly");
    }

    const NamedDecl* ND = strToND[str];

    if (!ND) {
      llvm::report_fatal_error("ND not found");
    }

    return ND;
  }

  void fillMainVars() {
    for (auto& [data, check] : psiList) {
      const NamedDecl* dataND = getNDFromStr(data);
      const NamedDecl* checkND = getNDFromStr(check);

      //todo fix leak later
      PairInfo* pi = new PairInfo(dataND, checkND);
      mainVars.addUsedVar(dataND, pi);
      mainVars.addUsedVar(checkND, pi);
    }
  }

public:
  MainWalker(MainVars& mainVars_, MainFncs& mainFncs_)
      : mainVars(mainVars_), mainFncs(mainFncs_) {}

  bool VisitFunctionDecl(const FunctionDecl* FD) {
    mainFncs.insertIfKnown(FD);

    // continue traversal
    return true;
  }

  bool VisitFieldDecl(const FieldDecl* FD) {
    addAnnotated(FD);

    // continue traversal
    return true;
  }

  bool VisitRecordDecl(const RecordDecl* RD) {
    addAnnotated(RD);

    // continue traversal
    return true;
  }

  bool VisitVarDecl(const VarDecl* VD) {
    printND(VD, "vd");

    // continue traversal
    return true;
  }

  void finalize() { fillMainVars(); }
};

} // namespace clang::ento::nvm