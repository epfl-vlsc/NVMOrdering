## How to build for the first time ##

    ./run.sh build
       
## How to run ##
	
	./run.sh run

    ./run.sh <report type> <test file> <checker type> 

* <report type>: scan, run
* <test file>: dcl, dscl, mask, rec, rmask, scl, simple_masstree, tx
* <checker type>: ordering, recovery, transaction


-analyzer-display-progress
clang -cc1 -analyze -analyzer-checker=core  f2.c -analyzer-display-progress

the most optimal solution to finding which functions to analyze would be to modify 
lib/Frontend/AnalysisConsumer.cpp, for now we will implement the features on the checker
