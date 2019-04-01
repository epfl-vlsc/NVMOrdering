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
* \<test file> all files under test folder
* \<checker type>
    * write - main code checker
    * read - recover code checker
    * txm - transaction checker for mnemosyne
    * txp - transaction checker for pmdk
    * log - check log ordering


Tests:
write
check: 4
dcl: 7
scl: 4
dcltoscl: 7
scltodcl: 7
twocheck: 8
dclm: 3
dclmt: 7
sclm: 3
sclmt: 7
simple_masstree: 0

txm
txm: 4

txp
txp: 1

log
log: 4
