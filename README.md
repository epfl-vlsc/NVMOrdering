## How to build for the first time ##

    ./run.sh build
       
## How to run ##
    ./run.sh <report type> <test file> <checker type> 

    ./run.sh scan simple_masstree write

* look at `output/index.html` for results

* \<report type>
    * run - generate command line report
    * scan - generate visual bug report
    * ast - generate ast of code to examine
* \<test file> dcl, scl, dscl, mask, simple_masstree, rmask, rec, txm, txp
* \<checker type>
    * write - main code checker
    * read - recover code checker
    * txm - transaction checker for mnemosyne
    * txp - transaction checker for pmdk
    * exp - experimental features
    * mc - multi source compile tests



Tests:
check: 5
dcl: 8
scl: 5
dcltoscl: 7
scltodcl: 7
twocheck: 8