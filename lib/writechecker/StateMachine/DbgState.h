#pragma once

//#define DBGSTATE
#ifdef DBGSTATE
#define DBG(P) llvm::outs() << P << "\n";
#else
#define DBG(P)
#endif
