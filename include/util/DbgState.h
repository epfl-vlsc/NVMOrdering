#pragma once

#define DBGSTATE
#ifdef DBGSTATE
#define DBG(P) llvm::errs() << P << "\n";
#define DBGS(P, M) printStmt(P, *Mgr, M, true);
#define DBGL(P, M) printLoc(P, M);
#define DBGR(P, M) printReg(P, M);
#define DBGN(P, M) printND(P, M);
#else
#define DBG(P)
#define DBGS(P, M)
#define DBGL(P, M)
#define DBGR(P, M)
#define DBGN(P, M)
#endif
