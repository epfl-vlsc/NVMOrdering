#pragma once

#define DBGSTATE
#ifdef DBGSTATE
#define DBG(P) llvm::errs() << P << "\n";
#else
#define DBG(P)
#endif
