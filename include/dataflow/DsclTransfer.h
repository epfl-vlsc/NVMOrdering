#pragma once
#include "Common.h"
#include "DsclState.h"

namespace clang::ento::nvm {

class DclTransfer {
  static void write();
  static void flush();
  static void pfence();
};

class SclTransfer {
  static void write();
  static void fence();
};

class DsclTransfer {
  static void write();
  static void flush();
  static void pfence();
  static void vfence();

public:
  static void handleStmt(const Stmt* stmt, AnalysisManager& mgr){
      printStmt(stmt, mgr, "s");
      printStmt(stmt, "s");
  }
};

} // namespace clang::ento::nvm