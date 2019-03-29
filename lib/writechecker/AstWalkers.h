#pragma once
#include "Common.h"
#include "DataInfos.h"
#include "FunctionInfos.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

class TUDWalker : public RecursiveASTVisitor<TUDWalker> {
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

  static constexpr const std::array<const char*, 3> ANNOTS = {DCL, SCL, CHECK};

  StringMap varMap;
  ValueSet annotatedVars;
  VarInfos& varInfos;
  FunctionInfos& fncInfos;

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

  void addBasedOnAnnot(const ValueDecl* dataVD, const AnnotateAttr* dataAA) {
    StringRef dataAnnotation = dataAA->getAnnotation();
    auto [annotInfo, textInfo] = dataAnnotation.split(SEP);
    if (annotInfo.contains(CHECK)) {
      // is check, single variable
      varInfos.addUsedVar(dataVD, new CheckInfo(dataVD));
    } else {
      // not check, pair variables
      const ValueDecl* checkVD = getCheckVD(textInfo);
      BaseInfo* BI = nullptr;
      if (annotInfo.contains(DCLM)) {
        // checkVD is either some other VD or nullptr
        BI = new DclMaskToValidInfo(dataVD, checkVD, dataAA);
        varInfos.addUsedVar(dataVD, BI);
      } else if (annotInfo.contains(SCLM)) {
        // checkVD is either some other VD or nullptr
        BI = new SclMaskToValidInfo(dataVD, checkVD, dataAA);
        varInfos.addUsedVar(dataVD, BI);
      } else if (annotInfo.contains(DCL)) {
        const AnnotateAttr* checkAA = getAnnotation(checkVD);
        if (checkAA) {
          StringRef checkAnnotation = checkAA->getAnnotation();
          if (checkAnnotation.contains(MASK)) {
            // valid is mask
            BI = new DclDataToMaskInfo(dataVD, checkVD);
            varInfos.addUsedVar(dataVD, BI);
          } else {
            // valid is normal
            BI = new DclInfo(dataVD, checkVD);
            varInfos.addUsedVar(dataVD, BI);
          }
        } else {
          BI = new DclInfo(dataVD, checkVD);
          varInfos.addUsedVar(dataVD, BI);
        }
      } else if (annotInfo.contains(SCL)) {
        const AnnotateAttr* checkAA = getAnnotation(checkVD);
        if (checkAA) {
          StringRef checkAnnotation = checkAA->getAnnotation();
          if (checkAnnotation.contains(MASK)) {
            // valid is mask
            BI = new SclDataToMaskInfo(dataVD, checkVD);
            varInfos.addUsedVar(dataVD, BI);
          } else {
            // valid is normal
            BI = new SclInfo(dataVD, checkVD);
            varInfos.addUsedVar(dataVD, BI);
          }
        } else {
          BI = new SclInfo(dataVD, checkVD);
          varInfos.addUsedVar(dataVD, BI);
        }
      } else {
        llvm::report_fatal_error("annotation not found - not possible");
      }

      // also subscribe valids
      if (BI) {
        varInfos.addUsedVar(checkVD, BI);
      }
    }
  }

public:
  TUDWalker(VarInfos& varInfos_, FunctionInfos& fncInfos_)
      : varInfos(varInfos_), fncInfos(fncInfos_) {}

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
    fncInfos.insertIfKnown(FD);

    // continue traversal
    return true;
  }

  void createUsedVars() {
    for (auto VD : annotatedVars) {
      createUsedVar(VD);
    }
  }

}; // namespace clang::ento::nvm

} // namespace clang::ento::nvm