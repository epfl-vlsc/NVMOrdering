#pragma once
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>

namespace clang::ento::nvm
{

const FunctionDecl* getFuncDecl(const Decl* BD);

const FunctionDecl* getFuncDecl(const CallEvent &Call);

const FunctionDecl* getFuncDecl(const LocationContext *LC);

const DeclaratorDecl *getDeclaratorDecl(const Decl *BD);

bool isTopFunction(CheckerContext &C);

} // namespace clang::ento::nvm