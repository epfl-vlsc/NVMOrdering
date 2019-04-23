#pragma once
#include "AnnotVar.h"
namespace clang::ento::nvm{

class LogDecls{
    using FunctionMap = std::map<std::string, const FunctionDecl*>;
    
    AnnotVar logVar;
    FunctionMap fncMap;

    public:

    LogDecls():logVar("logging"){}
};




}//clang ento nvm