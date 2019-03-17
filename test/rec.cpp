#include "annot.h"
#include <atomic>
#include <stdio.h>
#include <xmmintrin.h>

struct LogEntry {
  pdcl(LogEntry.valid) int data;
  pcheck() int valid;

  int getData() { return data; }

  int getValid() { return valid; }

  int recovery_code correctModelMethod() {  
    if(valid){
        return data;
    }
    return 0;
  }

  int recovery_code wrongDataAccess() {  
    return data;
  }
};
