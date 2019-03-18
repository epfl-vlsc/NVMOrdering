#pragma once

#include "Common.h"
#include <set>

namespace clang::ento::nvm {

class LabeledInfo {
  bool isCheck_;

public:
  LabeledInfo(bool check) : isCheck_(check) {}

  bool isCheck() { return isCheck_; }
};

class DataInfo : public LabeledInfo {
  static constexpr const char* DCL = "dcl";
  static constexpr const char* SCL = "scl";
  
  // todo if you have time optimize string comparison
  StringRef checkName_;
  bool isDcl_;

public:
  DataInfo(bool check, bool dcl, const StringRef& cName)
      : LabeledInfo(check), checkName_(cName), isDcl_(dcl) {}

  bool isDcl() const { return isDcl_; }

  bool isSameCheckName(const StringRef& otherCheckName) const{
    return checkName_.equals(otherCheckName);
  }

  //todo make stringref
  std::string getCheckName() const{
    return checkName_.str();
  }

  const char* getClStr() const{
    return (isDcl_) ? DCL : SCL;
  }
};

class CheckInfo : public LabeledInfo {
  bool hasMask_;
  StringRef maskStr_;

public:
  CheckInfo(bool check, bool msk, StringRef maskVal)
      : LabeledInfo(check), hasMask_(msk), maskStr_(maskVal) {}

  bool hasMask() const { return hasMask_; }

  StringRef getMask() const { return maskStr_; }
};

class CheckDataInfo : public CheckInfo {
  StringRef checkName_;
  bool isDcl_;

public:
  CheckDataInfo(bool check, bool dcl, const StringRef& cName, bool msk,
                StringRef maskVal)
      : CheckInfo(check, msk, maskVal), checkName_(cName), isDcl_(dcl) {}

  bool isDcl() const { return isDcl_; }

  bool isSameCheckName(const StringRef& otherCheckName) const {
    return checkName_.equals(otherCheckName);
  }

  StringRef getCheckName() const{
    return checkName_;
  }

  // todo use smart pointer
  DataInfo* getDI() const{ return new DataInfo(false, false, checkName_); }
};

class NVMTypeInfo {
  static constexpr const char* DCL = "dcl";
  static constexpr const char* SCL = "scl";
  static constexpr const char* CHECK = "check";
  static constexpr const char* SEP = "-";
  static constexpr const char* DOT_SEP = ".";
  static constexpr const StringRef emptySR;
  std::map<const DeclaratorDecl*, LabeledInfo*> labels;

public:
  ~NVMTypeInfo() {
    for (auto& [_, LD] : labels) {
      delete LD;
    }
  }

  void analyzeMemLabel(const DeclaratorDecl* D) {
    for (const auto* Ann : D->specific_attrs<AnnotateAttr>()) {
      StringRef annot = Ann->getAnnotation();
      if (auto [annotInfo, textInfo] = annot.split(SEP);
          annotInfo.contains(DCL)) {
        // todo use fully qualified name
        auto [_, fieldName] = annot.split(DOT_SEP);
        labels[D] = new DataInfo(false, true, fieldName);
        // llvm::outs() << "dcl " << annot << " " << fieldName << "\n";
      } else if (annotInfo.contains(SCL)) {
        // todo use fully qualified name
        auto [_, fieldName] = annot.split(DOT_SEP);
        labels[D] = new DataInfo(false, false, fieldName);
        // llvm::outs() << "scl " << annot << "\n";
      } else if (annotInfo.contains(CHECK)) {
        if (textInfo.empty()) {
          labels[D] = new CheckInfo(true, false, emptySR);
          // llvm::outs() << "check " << annot << "\n";
        } else {
          StringRef fieldName = D->getName();
          labels[D] = new CheckDataInfo(true, false, fieldName, true, textInfo);
          /*
          llvm::outs() << "m: " << textInfo << " f: " << fieldName
                       << " " << annot << "\n";
          */
        }
      }
    }
  }

  bool inLabels(const DeclaratorDecl* D) const { return labels.count(D); }

  LabeledInfo* getDeclaratorInfo(const DeclaratorDecl* D) const {
    assert(labels.count(D));
    return labels.find(D)->second;
  }
};

std::string getPairStr(const DeclaratorDecl* DD, const DataInfo* DI) {
  std::string sbuf;
  llvm::raw_string_ostream PairOs(sbuf);
  PairOs << "<" << DD->getNameAsString() << "," << DI->getCheckName()
         << ">:" << DI->getClStr();
  return PairOs.str();
}

} // namespace clang::ento::nvm