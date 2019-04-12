#pragma once
#include "Common.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

class AnnotVarInfo {
public:
  enum ClKind { Check, Scl, Dcl, Unk };

private:
  ClKind clKind;
  std::string checkName;
  bool usesMask;
  StringRef maskName;

public:
  AnnotVarInfo() : clKind(Unk), checkName(), usesMask(false) {}
  AnnotVarInfo(ClKind clKind_)
      : clKind(clKind_), checkName(), usesMask(false) {}
  AnnotVarInfo(ClKind clKind_, std::string checkName_)
      : clKind(clKind_), checkName(checkName_), usesMask(false) {}
  AnnotVarInfo(ClKind clKind_, std::string checkName_, StringRef maskName_)
      : clKind(clKind_), checkName(checkName_), usesMask(true),
        maskName(maskName_) {}

  bool isMask() const { return usesMask; }

  StringRef getMask() const { return maskName; }

  ClKind getClType() const { return clKind; }

  const std::string& getCheckName() const { return checkName; }

  void setDcl(){
    clKind = ClKind::Dcl;
  }

  void setScl(){
    clKind = ClKind::Scl;
  }
};

template <typename Derived, typename BaseInfo, typename OrderVars,
          typename OrderFncs>
class TUDWalker : public RecursiveASTVisitor<Derived> {
protected:
  using AnnotMap = std::map<const ValueDecl*, AnnotVarInfo>;
  using StringMap = std::map<std::string, const ValueDecl*>;
  using BIVec = std::vector<BaseInfo*>;
  using ValueMap = std::map<const ValueDecl*, BIVec>;

  static constexpr const char* KEY_SEP = "-";
  static constexpr const char* PAIR_SEP = "+";
  static constexpr const char* CHECK = "sent";
  static constexpr const char* PAIR = "pair";
  static constexpr const char* DCL = "dcl";
  static constexpr const char* SCL = "scl";

  static constexpr const std::array<const char*, 2> ANNOTS = {CHECK, PAIR};

  StringMap varMap;
  AnnotMap annotatedVars;
  OrderVars& orderVars;
  OrderFncs& orderFncs;

  AnnotVarInfo parseAnnotation(StringRef annotation) {
    auto [annotType, annotInfo] = annotation.split(KEY_SEP);
    if (annotType.contains(CHECK)) {
      return AnnotVarInfo(AnnotVarInfo::Check);
    } else if (annotType.contains(PAIR)) {
      SmallVector<StringRef, 3> annotatedTokens;
      annotInfo.split(annotatedTokens, PAIR_SEP);
      StringRef checkName = annotatedTokens[0];
      if (annotatedTokens.size() == 1) {
        return AnnotVarInfo(AnnotVarInfo::Unk, checkName);
      } else if (annotatedTokens.size() == 2) {
        StringRef token1 = annotatedTokens[1];
        if (token1.contains(DCL)) {
          return AnnotVarInfo(AnnotVarInfo::Dcl, checkName);
        } else if (token1.contains(SCL)) {
          return AnnotVarInfo(AnnotVarInfo::Scl, checkName);
        } else {
          return AnnotVarInfo(AnnotVarInfo::Unk, checkName, token1);
        }
      } else if (annotatedTokens.size() == 3) {
        StringRef token1 = annotatedTokens[1];
        StringRef token2 = annotatedTokens[2];

        if (token1.contains(DCL)) {
          return AnnotVarInfo(AnnotVarInfo::Dcl, checkName, token2);
        } else if (token1.contains(SCL)) {
          return AnnotVarInfo(AnnotVarInfo::Scl, checkName, token2);
        } else if (token2.contains(DCL)) {
          return AnnotVarInfo(AnnotVarInfo::Dcl, checkName, token1);
        } else if (token2.contains(SCL)) {
          return AnnotVarInfo(AnnotVarInfo::Scl, checkName, token1);
        } else {
          llvm::report_fatal_error("wrong annotation");
          return AnnotVarInfo();
        }
      } else {
        llvm::report_fatal_error("wrong annotation");
        return AnnotVarInfo();
      }

    } else {
      llvm::report_fatal_error("wrong annotation");
      return AnnotVarInfo();
    }
  }

  AnnotVarInfo* getAVI(const ValueDecl* VD) {
    if (VD) {
      // empty body masked dcl, scl
      return nullptr;
    }

    dbg_assert(annotatedVars, VD, "check not tracked correctly");
    return &annotatedVars[VD];
  }

  virtual void addAnnotation(const ValueDecl* dataVD,
                             const AnnotateAttr* dataAA,
                             const AnnotVarInfo& AVI) = 0;

  void addIfAnnotated(const FieldDecl* FD, StringRef annotation) {
    if (!annotation.empty()) {
      // if any annotation exists
      if (std::any_of(ANNOTS.begin(), ANNOTS.end(),
                      [&annotation](const char* a) {
                        return annotation.contains(a);
                      })) {
        // var is annotated

        // parse annotation
        AnnotVarInfo AVI = parseAnnotation(annotation);
        if (AVI.getClType() == AnnotVarInfo::Unk) {
          // do auto-determination
          AVI.setDcl();
        }

        annotatedVars[FD] = AVI;
      }
    }
  }

  void createUsedVar(const ValueDecl* VD, const AnnotVarInfo& AVI) {
    for (const auto* Ann : VD->specific_attrs<AnnotateAttr>()) {
      // add to annotated vars
      addAnnotation(VD, Ann, AVI);
    }
  }

  const ValueDecl* getCheckVD(const std::string& checkName) {
    if (checkName.empty()) {
      // empty body masked dcl, scl
      return nullptr;
    }

    if(!varMap.count(checkName)){
      llvm::report_fatal_error("check not found");
    }
    return varMap[checkName];
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

  void createUsedVars() {
    for (auto& [VD, VDInfo] : annotatedVars) {
      createUsedVar(VD, VDInfo);
    }
  }
};

} // namespace clang::ento::nvm