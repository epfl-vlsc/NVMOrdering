#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct ClNos {
  static const constexpr int INVALID_CL = -1;
  int beg;
  int end;

  ClNos(): beg(INVALID_CL), end(INVALID_CL) {}
  ClNos(int beg_, int end_) : beg(beg_), end(end_) {}

  bool isDcl() { return beg != end || beg == INVALID_CL || end == INVALID_CL; }
};

bool operator==(const ClNos& lhs, const ClNos& rhs) {
  return (lhs.beg == rhs.beg) && (lhs.end == rhs.end);
}

class ClPlace {
  using FieldCl = std::map<const std::string, ClNos>;
  using RecordCl = std::map<const RecordDecl*, FieldCl*>;

  static const constexpr unsigned CACHE_LINE_SIZE = 64;

  const ASTContext& ASTC;
  RecordCl recordCl;

  bool recordExists(const RecordDecl* RD) { return recordCl.count(RD); }

  bool isRecordValid(const RecordDecl* RD) {
    if (recordCl.count(RD)) {
      return recordCl[RD] != nullptr;
    }
    return false;
  }

  void createRD(const RecordDecl* RD) {
    int current = 0;

    int begCL = 0;
    int endCL = 0;

    if (!recordCl.count(RD)) {
      recordCl[RD] = new FieldCl();
    }

    for (const FieldDecl* FD : RD->fields()) {
      QualType QT = FD->getType();
      const Type* type = QT.getTypePtr();
      if (type->isConstantSizeType()) {
        uint64_t fieldSize = ASTC.getTypeSizeInChars(type).getQuantity();
        current += fieldSize;
        begCL = endCL;
        endCL = current / CACHE_LINE_SIZE;
        std::string fieldName = FD->getQualifiedNameAsString();

        FieldCl* fcl = recordCl[RD];
        (*fcl)[fieldName] = ClNos(begCL, endCL);
      } else {
        // fallback to dcl because of unknown size field
        recordCl[RD] = nullptr;
        return;
      }
    }
  }

  ClNos getClNos(const RecordDecl* RD, const std::string& field) {
    FieldCl* fcl = recordCl[RD];
    if (fcl) {
      // valid record
      if (fcl->count(field)) {
        return (*fcl)[field];
      } else {
        llvm::report_fatal_error("field not tracked");
      }
    } else {
      // invalid record
      return ClNos();
    }
  }

  bool isScl(const RecordDecl* RD, const std::string& field1,
             const std::string& field2) {
    if (field1 == field2) {
      // mask
      return true;
    } else {
      if (!recordExists(RD)) {
        createRD(RD);
      }

      if (isRecordValid(RD)) {
        ClNos field1ClNos = getClNos(RD, field1);
        ClNos field2ClNos = getClNos(RD, field2);
        return !field1ClNos.isDcl() && !field2ClNos.isDcl() &&
               (field1ClNos == field2ClNos);
      }
      return false;
    }
  }

public:
  ClPlace(const ASTContext& ASTC_) : ASTC(ASTC_) {}

  bool isScl(const FieldDecl* FD, const std::string& field2) {
    const RecordDecl* RD = FD->getParent();
    auto field1 = FD->getQualifiedNameAsString();
    return isScl(RD, field1, field2);
  }
};

} // namespace clang::ento::nvm
