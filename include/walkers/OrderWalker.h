#pragma once
#include "Common.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

template <typename Derived, typename BaseInfo, typename OrderVars,
          typename OrderFncs>
class TUDWalker : public RecursiveASTVisitor<Derived> {
protected:
  using ValueSet = std::set<const ValueDecl*>;
  using StringSet = std::set<std::string>;
  using StringMap = std::map<std::string, const ValueDecl*>;
  using BIVec = std::vector<BaseInfo*>;
  using ValueMap = std::map<const ValueDecl*, BIVec>;

  static constexpr const char* SEP = "-";
  static constexpr const char* CHECK = "check";
  static constexpr const char* DCL = "dcl";
  static constexpr const char* SCL = "scl";
  static constexpr const char* DCLM = "dclm";
  static constexpr const char* SCLM = "sclm";
  static constexpr const char* MASK = "mask";
  static constexpr const char* MASK_ANN = "MASK";

  static constexpr const std::array<const char*, 3> ANNOTS = {DCL, SCL, CHECK};

  StringSet maskedVars;
  StringMap varMap;
  ValueSet annotatedVars;
  OrderVars& orderVars;
  OrderFncs& orderFncs;

  virtual void addBasedOnAnnot(const ValueDecl* dataVD,
                               const AnnotateAttr* dataAA) = 0;

  void addIfAnnotated(const FieldDecl* FD, StringRef annotation) {
    if (auto [annotInfo, textInfo] = annotation.split(SEP);
        !annotInfo.empty()) {
      // if any annotation exists
      if (std::any_of(ANNOTS.begin(), ANNOTS.end(),
                      [&annotation](const char* a) {
                        return annotation.contains(a);
                      })) {
        // var is annotated
        annotatedVars.insert(FD);
      }
    }
  }

  void createUsedVar(const ValueDecl* VD) {
    for (const auto* Ann : VD->specific_attrs<AnnotateAttr>()) {
      // add to annotated vars
      addBasedOnAnnot(VD, Ann);
    }
  }

  const ValueDecl* getCheckVD(const StringRef& checkNameRef) {
    if (checkNameRef.empty()) {
      // empty body masked dcl, scl
      return nullptr;
    }

    auto checkName = checkNameRef.str();
    dbg_assert(varMap, checkName, "check not tracked correctly");
    return varMap[checkName];
  }

  bool hasMask(const Stmt* S) {
    MaskWalker maskWalker(false);
    maskWalker.Visit(S);
    return maskWalker.hasMask();
  }

public:
  TUDWalker(OrderVars& orderVars_, OrderFncs& orderFncs_)
      : orderVars(orderVars_), orderFncs(orderFncs_) {}

  // todo add a visitor to support local var declaration
  bool VisitFieldDecl(const FieldDecl* FD) {
    for (const auto* Ann : FD->specific_attrs<AnnotateAttr>()) {
      StringRef annotation = Ann->getAnnotation();
      // add to annotated vars
      addIfAnnotated(FD, annotation);
    }

    // add to known fields
    std::string varName = FD->getQualifiedNameAsString();
    varMap[varName] = FD;

    // continue traversal
    return true;
  }

  bool VisitFunctionDecl(const FunctionDecl* FD) {
    orderFncs.insertIfKnown(FD);

    // continue traversal
    return true;
  }

  bool VisitBinaryOperator(const BinaryOperator* BO) {
    if (BO->isAssignmentOp() && hasMask(BO)) {
      Expr* E = BO->getLHS();
      if (const MemberExpr* ME = dyn_cast_or_null<MemberExpr>(E)) {
        ValueDecl* VD = ME->getMemberDecl();
        std::string fqName = VD->getQualifiedNameAsString();
        maskedVars.insert(fqName);
      }
    }

    // continue traversal
    return true;
  }

  void createUsedVars() {
    for (auto VD : annotatedVars) {
      createUsedVar(VD);
    }
  }
};

} // namespace clang::ento::nvm