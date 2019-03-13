#pragma once
#include "Common.h"
#include <set>

namespace clang::ento::nvm
{

bool isTopFunction(CheckerContext &C){
    return C.inTopFrame();
}

class FlushFunction
{

  public:
    void dump() const
    {
    }
};

class FenceFunction
{

  public:
    void dump() const
    {
    }
};

class AnalyzeFunction
{
    static constexpr const char *ANALYSIS_ANNOT = "PersistentCode";
    std::set<const FunctionDecl*> analysisFunctions;

  public:
    void insertIfAnnotated(const FunctionDecl *D)
    {
        //if has attribute for analysis, add function to set
        for (const auto *Ann : D->specific_attrs<AnnotateAttr>())
        {
            if (Ann->getAnnotation() == ANALYSIS_ANNOT)
            {
                analysisFunctions.insert(D);
            }
        }
    }

    bool isAnnotatedFunction(const FunctionDecl *D) const{
        if(analysisFunctions.count(D)){
            return true;
        }
        return false;
    }

    void dump() const
    {
        llvm::outs() << "Annotated functions\n";
        for (auto* D : analysisFunctions)
        {
            llvm::outs() << D->getQualifiedNameAsString() << "\n";
        }
    }
};

class NVMFunctionInfo
{
    AnalyzeFunction analyzeFnc;
    FenceFunction fenceFnc;
    FlushFunction flushFnc;

  public:
    void analyzeIfAnnotated(const FunctionDecl *D)
    {
        analyzeFnc.insertIfAnnotated(D);
    }

    bool isAnnotatedFunction(CheckerContext &C) const{
        const LocationContext* LCtx = C.getLocationContext();
        if(const FunctionDecl* D = dyn_cast_or_null<FunctionDecl>(LCtx->getDecl())){
            return analyzeFnc.isAnnotatedFunction(D);
        }
        return false;
    }

    void dump() const
    {
        analyzeFnc.dump();
        fenceFnc.dump();
        flushFnc.dump();
    }
};



} // namespace clang::ento::nvm