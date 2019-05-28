#pragma once
#include "AutoCl.h"
#include "Common.h"
#include "MainVars.h"
#include "PairInfo.h"
#include "identify/MainFncs.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

class MainWalker : public RecursiveASTVisitor<MainWalker> {
  static constexpr const char* PAIR = "pair";
  static constexpr const char KEY_SEP = '-';
  static constexpr const char* SCL_STR = "scl";
  static constexpr const char* DCL_STR = "dcl";

  enum class ClType { SCL, DCL, UNK };
  struct PairStrInfo {
    const std::string dataName;
    const std::string checkName;
    const ClType clType;

    PairStrInfo(const std::string& dataName_, const std::string& checkName_,
                ClType clType_)
        : dataName(dataName_), checkName(checkName_), clType(clType_) {}
  };

  ClType getClType(const StringRef& pairStrRef) {
    if (pairStrRef.contains(SCL_STR)) {
      return ClType::SCL;
    } else if (pairStrRef.contains(DCL_STR)) {
      return ClType::DCL;
    }

    return ClType::UNK;
  }

  bool usesScl(const NamedDecl* dataND, const NamedDecl* checkND,
               ClType clType) {
    switch (clType) {
    case ClType::SCL:
      return true;
    case ClType::DCL:
      return false;
    default:
      if (autoCl.isScl(dataND, checkND)) {
        return true;
      }
      return false;
    }
  }

  using PSIList = std::vector<PairStrInfo>;
  using StrToND = std::map<std::string, const NamedDecl*>;
  using MESet = std::set<const MemberExpr*>;

  MainVars& mainVars;
  MainFncs& mainFncs;
  ASTContext& AC;

  PSIList psiList;
  StrToND strToND;
  AutoCl autoCl;
  MESet memberExprs;

  void addAnnotated(const NamedDecl* ND) {
    std::string dataName = ND->getQualifiedNameAsString();
    strToND[dataName] = ND;

    for (const auto* Ann : ND->specific_attrs<AnnotateAttr>()) {
      StringRef annotation = Ann->getAnnotation();
      if (annotation.contains(PAIR)) {
        auto [pairStrRef, checkNameRef] = annotation.rsplit(KEY_SEP);
        if (checkNameRef.empty()) {
          llvm::report_fatal_error("check cannot be empty");
        }

        ClType clType = getClType(pairStrRef);

        std::string checkName = checkNameRef.str();
        psiList.emplace_back(dataName, checkName, clType);
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
    for (auto& [data, check, clType] : psiList) {
      const NamedDecl* dataND = getNDFromStr(data);
      const NamedDecl* checkND = getNDFromStr(check);

      bool isScl = usesScl(dataND, checkND, clType);

      // create Pair info
      PairInfo* pi = new PairInfo(dataND, checkND, isScl);
      mainVars.addUsedVar(dataND, pi);
      mainVars.addUsedVar(checkND, pi);
    }
  }

  void fillAnalysisFunctions() {
    for (const MemberExpr* ME : memberExprs) {
      const ValueDecl* VD = ME->getMemberDecl();
      if (!mainVars.isUsedVar(VD)) {
        continue;
      }

      const FunctionDecl* FD = getFuncDecl(ME, AC);
      if (!FD) {
        continue;
      }

      mainFncs.insertAnalyze(FD);
    }
  }

public:
  MainWalker(MainVars& mainVars_, MainFncs& mainFncs_, ASTContext& AC_)
      : mainVars(mainVars_), mainFncs(mainFncs_), AC(AC_), autoCl(AC_) {}

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

  bool VisitMemberExpr(const MemberExpr* ME) {
    memberExprs.insert(ME);

    // continue traversal
    return true;
  }

  bool shouldVisitTemplateInstantiations() const { return true; }

  void finalize() {
    fillMainVars();
    fillAnalysisFunctions();
  }
};

} // namespace clang::ento::nvm