#pragma once

//#define DBGSTATE
#ifdef DBGSTATE
#define DBG(P) llvm::errs() << P << "\n";
#define DBGS(P, M) printStmt(P, C, M, true);
#define DBGL(P, M) printLoc(P, M);
#else
#define DBG(P)
#define DBGS(P, M)
#define DBGL(P, M)
#endif
