#pragma once
#include "Common.h"
#include "TypeInfos.h"

namespace clang::ento::nvm {

struct RecState {
private:
  enum Kind { RC, RD } K;
  CheckInfo* CI;
  static constexpr const char* Str[] = {"ReadCheck", "ReadData"};

  RecState(Kind K_, CheckInfo* CI_) : K(K_), CI(CI_) {}

public:
  bool isReadCheck() const { return K == RC; }
  bool isReadData() const { return K == RD; }

  static RecState getReadCheck(CheckInfo* CI_) { return RecState(RC, CI_); }
  static RecState getReadData(CheckInfo* CI_) { return RecState(RD, CI_); }

  int getStateKind() const { return static_cast<int>(K); }

  CheckInfo* getCheckInfo() const { return CI; }

  bool operator==(const RecState& X) const { return K == X.K; }

  const char* getStateName() const { return Str[K]; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddInteger(K);
    ID.AddPointer(CI);
    ID.AddPointer(Str);
  }
};

} // namespace clang::ento::nvm

REGISTER_MAP_WITH_PROGRAMSTATE(RecMap, const clang::DeclaratorDecl*,
                               clang::ento::nvm::RecState)