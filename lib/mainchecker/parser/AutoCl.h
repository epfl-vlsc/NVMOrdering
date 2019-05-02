#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct ClNos {
  static const constexpr int INVALID_CL = -1;
  int beg;
  int end;

  ClNos() : beg(INVALID_CL), end(INVALID_CL) {}
  ClNos(int beg_, int end_) : beg(beg_), end(end_) {}

  bool isValid() const { return (beg != INVALID_CL) && (end != INVALID_CL); }
};

bool operator==(const ClNos& lhs, const ClNos& rhs) {
  bool beg_eq = (lhs.beg == rhs.beg);
  bool end_eq = (lhs.end == rhs.end);
  bool lhs_val = lhs.isValid();
  bool rhs_val = rhs.isValid();
  return beg_eq && end_eq && lhs_val && rhs_val;
}

class AutoCl {
  using Fields = std::map<const NamedDecl*, ClNos>;
  using Records = std::map<const RecordDecl*, Fields>;

  static const constexpr unsigned CACHE_LINE_SIZE = 64;

  const ASTContext& ASTC;
  Records records;

  bool recordExists(const RecordDecl* RD) { return records.count(RD); }

  bool isRecordValid(const RecordDecl* RD) {
    if (records.count(RD)) {
      return !records[RD].empty();
    }
    return false;
  }

  void createRD(const RecordDecl* RD) {
    int current = 0;

    int begCl = 0;
    int endCl = 0;

    if (records.count(RD)) {
      return;
    }

    // create record entry, add obj to 0
    auto& fields = records[RD];
    fields[RD] = ClNos(begCl, endCl);

    // add fields
    for (const FieldDecl* FD : RD->fields()) {
      QualType QT = FD->getType();
      const Type* type = QT.getTypePtr();
      if (type->isConstantSizeType()) {
        uint64_t fieldSize = ASTC.getTypeSizeInChars(type).getQuantity();
        current += fieldSize;
        begCl = endCl;
        endCl = current / CACHE_LINE_SIZE;

        fields[FD] = ClNos(begCl, endCl);
      } else {
        // fallback to dcl because of unknown size field
        fields.clear();
        return;
      }
    }
  }

  ClNos getClNos(const RecordDecl* RD, const NamedDecl* ND) {
    auto& fields = records[RD];

    // valid record
    if (fields.count(ND)) {
      return fields[ND];
    } else {
      llvm::report_fatal_error("field not tracked");
    }
  }

public:
  AutoCl(const ASTContext& ASTC_) : ASTC(ASTC_) {}

  bool isScl(const NamedDecl* ND1, const NamedDecl* ND2) {
    if (!isFieldOrObj(ND1) || !isFieldOrObj(ND2)) {
      return false;
    }

    const FieldDecl* FD = getFieldFromNDs(ND1, ND2);
    const RecordDecl* RD = FD->getParent();
    createRD(RD);

    if (isRecordValid(RD)) {
      ClNos clND1 = getClNos(RD, ND1);
      ClNos clND2 = getClNos(RD, ND2);
      return clND1 == clND2;
    }
    return false;
  }
}; // namespace clang::ento::nvm

} // namespace clang::ento::nvm
