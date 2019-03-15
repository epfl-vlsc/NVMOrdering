#pragma once
#include "Common.h"
#include <set>

namespace clang::ento::nvm
{

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

bool isTopFunction(CheckerContext &C)
{
    return C.inTopFrame();
}

class BaseFunction
{
  protected:
    std::set<const FunctionDecl *> functions;

    virtual bool checkName(const IdentifierInfo *II) const{
        llvm::report_fatal_error("Override one check name in derived");
        return false;
    }
    virtual bool checkName(const AnnotateAttr *AA) const{
        llvm::report_fatal_error("Override one check name in derived");
        return false;
    }

    virtual bool checkName(const FunctionDecl *D) const{
        return checkName(D->getIdentifier());
    }

  public:
    virtual void dump() const
    {
        for (auto *D : functions)
        {
            llvm::outs() << D->getQualifiedNameAsString() << "\n";
        }
    }

    bool inFunctions(const FunctionDecl *D) const
    {
        return functions.count(D);
    }

    //for annotation
    virtual void insertIfKnown(const FunctionDecl *D) {
        if(checkName(D)){
            functions.insert(D);
        }
    }
};

class FlushFunction : public BaseFunction
{
  public:
bool checkName(const IdentifierInfo *II) const{
        return II && II->isStr("clflush");
    }
};

class PFenceFunction : public BaseFunction
{
  public:
    bool checkName(const IdentifierInfo *II) const{
        return II && II->isStr("pfence");
    }
};

class VFenceFunction : public BaseFunction
{
  public:
    bool checkName(const IdentifierInfo *II) const{
        return II && II->isStr("vfence");
    }
};

class AnnotFunction : public BaseFunction
{
  public:
    bool checkName(const AnnotateAttr *AA) const{
        return AA && AA->getAnnotation() == "PersistentCode";
    }

    void insertIfKnown(const FunctionDecl *D)
    {
        //if has attribute for analysis, add function to set
        for (const auto *Ann : D->specific_attrs<AnnotateAttr>())
        {
            if (checkName(Ann))
            {
                functions.insert(D);
            }
        }
    }
};

class NVMFunctionInfo
{
    AnnotFunction annotFnc;
    VFenceFunction vfenceFnc;
    PFenceFunction pfenceFnc;
    FlushFunction flushFnc;

  public:
    void checkFunction(const FunctionDecl *D)
    {
        annotFnc.insertIfKnown(D);
        vfenceFnc.insertIfKnown(D);
        pfenceFnc.insertIfKnown(D);
        flushFnc.insertIfKnown(D);
    }

    bool isAnnotatedFunction(CheckerContext &C) const
    {
        const LocationContext *LCtx = C.getLocationContext();
        if (const FunctionDecl *D = dyn_cast_or_null<FunctionDecl>(LCtx->getDecl()))
        {
            return annotFnc.inFunctions(D);
        }
        return false;
    }

    bool isFlushFunction(const CallEvent &Call) const
    {
        const FunctionDecl* D = getFuncDecl(Call);
        return flushFnc.inFunctions(D);
    }

    bool isPFenceFunction(const CallEvent &Call) const
    {
        const FunctionDecl* D = getFuncDecl(Call);
        return pfenceFnc.inFunctions(D);
    }

    bool isVFenceFunction(const CallEvent &Call) const
    {
        const FunctionDecl* D = getFuncDecl(Call);
        return vfenceFnc.inFunctions(D);
    }

    void dump() const
    {
        annotFnc.dump();
        vfenceFnc.dump();
        vfenceFnc.dump();
        flushFnc.dump();
    }
};

} // namespace clang::ento::nvm