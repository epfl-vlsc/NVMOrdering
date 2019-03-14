#pragma once
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>


namespace clang::ento::nvm{

const FunctionDecl* getFuncDecl(const CallEvent &Call){
    const Decl* BD = Call.getDecl();
    if (const FunctionDecl *D = dyn_cast_or_null<FunctionDecl>(BD))
    {
        return D;
    }
    llvm::report_fatal_error("All calls have function declaration");
    return nullptr;
}

const DeclaratorDecl* getDeclaratorDecl(const Decl* BD){
    if (const DeclaratorDecl *D = dyn_cast_or_null<DeclaratorDecl>(BD))
    {
        return D;
    }
    llvm::report_fatal_error("All calls have function declaration");
    return nullptr;
}

} //namespace nvm