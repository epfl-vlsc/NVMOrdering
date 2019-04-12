#pragma once
#include "Common.h"
#include "identify/OrderFncs.h"
#include "state_machine/OrderVars.h"
#include "walkers/OrderWalker.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace clang::ento::nvm {

class WriteWalker
    : public TUDWalker<WriteWalker, BaseInfo, OrderVars, OrderFncs> {

  void addBasedOnAnnot(const ValueDecl* dataVD, const AnnotateAttr* dataAA) {
    StringRef dataAnnotation = dataAA->getAnnotation();
    auto [annotInfo, textInfo] = dataAnnotation.split(SEP);
    if (annotInfo.contains(CHECK)) {
      // is check, single variable
      orderVars.addUsedVar(dataVD, new CheckInfo(dataVD));
    } else {
      // not check, pair variables
      std::string dataName = dataVD->getQualifiedNameAsString();
      const ValueDecl* checkVD = getCheckVD(textInfo);
      std::string checkName = "";
      if (checkVD) {
        checkName = checkVD->getQualifiedNameAsString();
      }
      BaseInfo* BI = nullptr;

      if (annotInfo.contains(DCL)) {
        if (!dataName.empty() && maskedVars.count(dataName)) {
          // masked vars
          if (!textInfo.empty()) {
            // if transitively has a validator
            BI = new DclMaskToValidInfo(dataVD, checkVD, dataAA);
            orderVars.addUsedVar(dataVD, BI);
            orderVars.addUsedVar(dataAA, BI);
            llvm::report_fatal_error("disable masking");
          } else {
            // does not have a validator
            BI = new DclMaskToValidInfo(dataVD, nullptr, nullptr);
            orderVars.addUsedVar(dataVD, BI);
            llvm::report_fatal_error("disable masking");
          }
        } else {
          if (!checkName.empty() && maskedVars.count(checkName)) {
            // masked valid
            BI = new DclDataToMaskInfo(dataVD, checkVD);
            orderVars.addUsedVar(dataVD, BI);
            llvm::report_fatal_error("disable masking");
          } else {
            // normal dcl
            BI = new DclInfo(dataVD, checkVD);
            orderVars.addUsedVar(dataVD, BI);
          }
        }
      } else if (annotInfo.contains(SCL)) {
        if (!dataName.empty() && maskedVars.count(dataName)) {
          // masked vars
          if (!textInfo.empty()) {
            // if transitively has a validator
            BI = new SclMaskToValidInfo(dataVD, checkVD, dataAA);
            orderVars.addUsedVar(dataVD, BI);
            orderVars.addUsedVar(dataAA, BI);
            llvm::report_fatal_error("disable masking");
          } else {
            // does not have a validator
            BI = new SclMaskToValidInfo(dataVD, nullptr, nullptr);
            orderVars.addUsedVar(dataVD, BI);
            llvm::report_fatal_error("disable masking");
          }
        } else {
          if (!checkName.empty() && maskedVars.count(checkName)) {
            // masked valid
            BI = new SclDataToMaskInfo(dataVD, checkVD);
            orderVars.addUsedVar(dataVD, BI);
            llvm::report_fatal_error("disable masking");
          } else {
            // normal dcl
            BI = new SclInfo(dataVD, checkVD);
            orderVars.addUsedVar(dataVD, BI);
          }
        }
      } else {
        llvm::report_fatal_error("annotation not found - not possible");
      }

      // also subscribe valids
      if (checkVD != nullptr && BI != nullptr) {
        orderVars.addUsedVar(checkVD, BI);
      }
    }
  }

public:
  WriteWalker(OrderVars& orderVars_, OrderFncs& orderFncs_)
      : TUDWalker(orderVars_, orderFncs_) {}
};

} // namespace clang::ento::nvm