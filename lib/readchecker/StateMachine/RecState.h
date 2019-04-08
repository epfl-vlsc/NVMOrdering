#pragma once
#include "Common.h"

namespace clang::ento::nvm {

struct RecState {
private:
  enum Kind { RC, RD } K;
  static constexpr const char* Str[] = {"ReadCheck", "ReadData"};
  static constexpr const char* Exp[] = {"check is read", "data is read"};

  RecState(Kind kind) : K(kind) {}

public:
  bool isReadCheck() const { return K == RC; }
  bool isReadData() const { return K == RD; }

  bool isFinal() const { return K == RD; }
  static RecState getInit() { return RecState(RC); }

  static RecState getReadCheck() { return RecState(RC); }
  static RecState getReadData() { return RecState(RD); }

  int getStateKind() const { return static_cast<int>(K); }

  bool operator==(const RecState& X) const { return K == X.K; }

  const char* getStateName() const { return Str[K]; }
  const char* getExplanation() const { return Exp[K]; }

  void Profile(llvm::FoldingSetNodeID& ID) const {
    ID.AddInteger(K);
    ID.AddPointer(Str);
    ID.AddPointer(Exp);
  }
};

} // namespace clang::ento::nvm

REGISTER_MAP_WITH_PROGRAMSTATE(RecMap, const char*,
                               clang::ento::nvm::RecState)
