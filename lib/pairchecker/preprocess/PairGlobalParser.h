#pragma once
#include "Common.h"
#include "PairInfo.h"
#include "parser_util/AutoCl.h"

namespace clang::ento::nvm {

template <typename Globals>
class PairGlobalParser : public RecursiveASTVisitor<PairGlobalParser<Globals>> {
public:
  using LatVar = typename Globals::LatVar;

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

  bool usesScl(LatVar dataND, LatVar checkND, ClType clType) {
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
  using StrToND = std::map<std::string, LatVar>;
  using MESet = std::set<const MemberExpr*>;

  // whole program data structures
  Globals& globals;
  ASTContext& AC;

  // temporary structures
  PSIList psiList;
  StrToND strToND;
  AutoCl autoCl;
  MESet memberExprs;

  void addAnnotated(LatVar ND) {
    std::string dataName = ND->getQualifiedNameAsString();
    strToND[dataName] = ND;

    for (const auto* Ann : ND->template specific_attrs<AnnotateAttr>()) {
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

  LatVar getNDFromStr(const std::string& str) {
    // ensures valid ND
    if (!strToND.count(str)) {
      llvm::errs() << str << "\n";
      llvm::report_fatal_error("not tracked correctly");
    }

    LatVar ND = strToND[str];

    if (!ND) {
      llvm::report_fatal_error("ND not found");
    }

    return ND;
  }

  void fillMainVars() {
    for (auto& [data, check, clType] : psiList) {
      LatVar dataND = getNDFromStr(data);
      LatVar checkND = getNDFromStr(check);

      bool isScl = usesScl(dataND, checkND, clType);

      // create Pair info
      PairInfo* pi = new PairInfo(dataND, checkND, isScl);
      globals.insertVariable(dataND, pi);
      globals.insertVariable(checkND, pi);
    }
  }

  void autoFindFunctions() {
    for (const MemberExpr* ME : memberExprs) {
      const ValueDecl* VD = ME->getMemberDecl();
      if (!globals.isUsedVar(VD)) {
        continue;
      }

      const FunctionDecl* FD = getFuncDecl(ME, AC);
      if (!FD) {
        continue;
      }

      // add function to the analysis list
      globals.insertAnalyzeFunction(FD);
    }

    // remove all skip from analyze
    globals.removeSkipFromAnalyze();
  }

  void trackStmt(const FunctionDecl* FD) {
    if (FD->hasBody() && !globals.isSkipFunction(FD)) {
      const Stmt* functionBody = FD->getBody();
      trackStmt(functionBody);
    }
  }

  void trackStmt(const Stmt* S) {
    if (const CallExpr* CE = dyn_cast<CallExpr>(S)) {
      const FunctionDecl* FD = CE->getDirectCallee();
      if (FD->hasBody() && !globals.isSkipFunction(FD)) {
        trackStmt(FD);
        globals.insertFunctionToActiveUnit(FD);
      } else {
        return;
      }
    } else if (const MemberExpr* ME = dyn_cast<MemberExpr>(S)) {
      const ValueDecl* VD = ME->getMemberDecl();
      globals.insertVariableToActiveUnit(VD);
    }

    trackChildren(S);
  }

  void trackChildren(const Stmt* S) {
    for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
         I != E; ++I) {
      if (const Stmt* Child = *I) {
        trackStmt(Child);
      }
    }
  }

  void fillTrackMap() {
    for (const FunctionDecl* FD : globals.getAnalyzedFunctions()) {
      globals.initActiveUnit(FD);
      trackStmt(FD);
    }
  }

public:
  PairGlobalParser(Globals& globals, ASTContext& AC_)
      : globals(globals), AC(AC_), autoCl(AC_) {}

  bool VisitFunctionDecl(const FunctionDecl* FD) {
    globals.insertFunction(FD);

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

  void fillStructures() {
    fillMainVars();
    autoFindFunctions();
    fillTrackMap();
  }
};

} // namespace clang::ento::nvm