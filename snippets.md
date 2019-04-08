``` cpp
//how to just analyze persistent function
//stack frame if top level, check function attribute if persistent, then analyze, else generate sink node

//how to check transactions?
//check BlockInCriticalSectionChecker


//REGISTER_TRAIT_WITH_PROGRAMSTATE(MutexCounter, unsigned)
//think about it like unsigned MutexCounter;

//how to find ordering bugs
//check mallocchecker

//iterate over types
//while iterating, check annotate attr

//to track mem locations use bind

//seperate bug report from checker, state

//for identifier initializatio
//for types look at
//boolassignmentchecker

//for annot identify either get it from bind or check statement

//use sval symbolic information and get the type from there



// bind, precall,

//symbol ref
//if field name is annotated type, and this is same, then do stuff
```

``` cpp

SVal Node = Call.getArgSVal(0);
  SVal Offset = Call.getArgSVal(1);
  llvm::errs() << "Range" << "\n";
  Node.dump();
  llvm::errs() << "\n";
  Offset.dump();
  llvm::errs() << "\n";
  SourceRange SR = Call.getSourceRange();
  SourceLocation SL = SR.getBegin();
  SL.dump(C.getSourceManager());
  llvm::errs() << "\n";

void checkEndFunction (CheckerContext & C ){
    ProgramStateRef State = C . getState ();

const Expr * E = /* some AST expression you are interested in */;
const LocationContext * LC = C . getLocationContext ();
SVal Val = State - > getSVal (E , LC );

//if expr is pointer to get the location, getasregion
const Expr * E = /* an pointer expression */;
const MemRegion * Reg = State - > getSVal (E , LC ). getAsRegion ();

const VarDecl * D = /* a declaration of a variable */;
const MemRegion * Reg = State - > getLValue (D , LC ). getAsRegion ();

SVal Val = State - > getSVal ( Reg );

class Callback : public StoreManager :: BindingsHandler {
public :
bool HandleBinding ( StoreManager & SM , Store St ,
const MemRegion * Region , SVal Val ) {
/* ... */
}
};

Callback CB ;
StoreManager & SM = C . getStoreManager ();
SM . iterBindings ( State - > getStore () , CB );

SVal Val = /* a certain symbolic value */;
Optional < DefinedOrUnknownSVal > DVal = Val . getAs < DefinedOrUnknownSVal >();
if (! DVal )
return ;
if ( State - > assume (* DVal , true )) {
/* things to do if Val can possibly be true */
}
if ( State - > assume (* DVal , false )) {
/* things to do if Val can possibly be false */
}

SVal Val = /* a certain symbolic value */;
Optional < DefinedOrUnknownSVal > DVal = Val . getAs < DefinedOrUnknownSVal >();
if (! DVal )
return ;
if ( State - > assume (* DVal , true )) {
/* things to do if Val can possibly be true */
}
if ( State - > assume (* DVal , false )) {
/* things to do if Val can possibly be false */
}

SVal A = /* a certain symbolic value */;
SVal B = /* the other symbolic value */;
ASTContext & ACtx = C . getASTContext ();
SValBuilder & SVB = C . getSValBuilder ();
SVal C = SVB . evalBinOp ( State , BO_GT , A , B , ACtx . BoolTy );


ProgramStateRef State = C . getState ();
SVal Loc = /* Obtain a location */;
SVal Val = /* Obtain a value */;
State = State - > bindLoc ( Loc , Val );
C . addTransition ( State );

//to use generic data map
//override profile method

mutable std :: unique_ptr < BugType > BT ;
if (! BT )
24 BT . reset (new BugType (this , " Call to main ", " Example checker " ));
25 ExplodedNode * N = C . generateErrorNode ();
26 auto Report = llvm :: make_unique < BugReport >(* BT , BT - > getName () , N );
27 C . emitReport ( std :: move ( Report ));

auto R = llvm::make_unique<BugReport>(*DoubleCloseBugType,
      "Closing a previously closed file stream", ErrNode);
  R->addRange(Call.getSourceRange());
  R->markInteresting(FileDescSym);
  C.emitReport(std::move(R));



check::EndOfTranslationUnit
check::ASTCodeBody
check::ASTDecl<T>

void WalkAST :: VisitCallExpr ( const CallExpr * CE ) {
if ( const FunctionDecl * FD = CE - > getDirectCallee ())
if ( const IdentifierInfo * II = FD -> getIdentifier ())
if ( II - > isStr (" main ")) {
SourceRange R = CE - > getSourceRange ();
PathDiagnosticLocation ELoc =
PathDiagnosticLocation :: createBegin ( CE , BR . getSourceManager () , ADC );
BR . EmitBasicReport ( ADC -> getDecl () , " Call to main ", " Example checker ",
" Call to main ", ELoc , R );
}
VisitChildren ( CE );
}

void MainCallCheckerAST :: checkASTCodeBody ( const Decl *D , AnalysisManager & AM ,
BugReporter & BR ) const {
WalkAST Walker ( BR , AM . getAnalysisDeclContext ( D ));
Walker . Visit (D - > getBody ());
}

namespace {
class Callback : public MatchFinder :: MatchCallback {
BugReporter & BR ;
AnalysisDeclContext * ADC ;
public :
void run ( const MatchFinder :: MatchResult & Result );
Callback ( BugReporter & Reporter , AnalysisDeclContext * Context )
: BR ( Reporter ) , ADC ( Context ) {}
};
}

void Callback :: run ( const MatchFinder :: MatchResult & Result ) {
const CallExpr * CE = Result . Nodes . getStmtAs < CallExpr >(" call ");
assert ( CE );
SourceRange R = CE - > getSourceRange ();
PathDiagnosticLocation ELoc =
PathDiagnosticLocation :: createBegin ( CE , BR . getSourceManager () , ADC );
BR . EmitBasicReport ( ADC -> getDecl () , " Call to main ", " Example checker ",
" Call to main ", ELoc , R );
}

FnCheck evalFunction = llvm::StringSwitch<FnCheck>(Name)
    .Case("scanf", &GenericTaintChecker::postScanf)
    // TODO: Add support for vfscanf & family.
    .Case("getchar", &GenericTaintChecker::postRetTaint)
    .Case("getchar_unlocked", &GenericTaintChecker::postRetTaint)
    .Case("getenv", &GenericTaintChecker::postRetTaint)
    .Case("fopen", &GenericTaintChecker::postRetTaint)
    .Case("fdopen", &GenericTaintChecker::postRetTaint)
    .Case("freopen", &GenericTaintChecker::postRetTaint)
    .Case("getch", &GenericTaintChecker::postRetTaint)
    .Case("wgetch", &GenericTaintChecker::postRetTaint)
    .Case("socket", &GenericTaintChecker::postSocket)
    .Default(nullptr);

```
