#todo
* break circular dependency

* clang 8.0.0

## How to build for the first time ##

    ./run.sh build
       
## How to run ##

    ./run.sh <report type> <checker type> <test file>
    ./run.sh scan write dcl

## Simple Masstree ##

    ./run.sh scan write simple_masstree
    ./run.sh scan read simple_masstree 

## How to run benchmark ##

    ./bench.sh run txp ctree_map

* look at `output/index.html` for results

* \<report type>
    * run - generate command line report
    * scan - generate visual bug report
    * ast - generate ast of code to examine
* \<checker type>
    * write - main code checker
    * read - recover code checker
    * txm - transaction checker for mnemosyne
    * txp - transaction checker for pmdk
    * log - check log ordering
* \<test file> all files under test folder

# incll #
commit: 0a0da25844f4c162260918db4cc6b4a15d262f58

# mnemosyne #
wrong: 68c1ef43c6597c1f1a426022800bb52c4c17acb4
fix: 60235c796de43268744793af940f51f53436705f

# pmgd #
wrong: 236221b2a6fb7149cd22b408bd3f4d49505937ba
fix: 891b437673ebd947699c1fea2e945ca3bbdc9540 

# whisper #
nvml commit: 54871035ad2a8a59f26dd00f02287aecf19d5b01
pmfs commit: 94323d525a10980fc6b9371234c26195acf0fc9f

# assumptions
* txp
    * use types of variables to recognize pm

# txp
* pmdk
    * handle loops
    * handle obj->field->field access
* log
    * handle obj

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
scltom: 4
dcltom: 9
simple_masstree: 0

read
rec: 1
recm: 1
recmt: 2
simple_masstree: 0

txm
txm: 4

txp
txp: 1

log
log: 4

git diff > patch/owriteback/pmfs1.patch
 git reset --hard


# benchmark details #
## nova ##
    
    make V=1 > make.txt

## nvthreads ##

    make libnvthread.so > nvthreads.txt

    cd $NVthreads/tests/recover/
    make > recover.txt

## pmgd ##

    make PMOPT=PM -n > make.txt


## pmwcas ##

    mkdir build
    cd build
    cmake -DPMEM_BACKEND=Volatile -DCMAKE_BUILD_TYPE=Debug ..
    make VERBOSE=1 > make.txt 

## kv echo ##
 
    make > make.txt

## mnemosyne ##

    scons --build-example=simple --verbose > simple.txt
    scons --build-bench=stamp-kozy --verbose > stamp.txt

## nstore ## 

    make > make.txt

## pmfs

    make V=1 > make.txt

## redis ##

    make USE_NVML=yes STD=-std=gnu99 V=1

## atlas ##

    make VERBOSE=1 > make.txt

## nvml ## 
* delete pfence

    ./build.sh > make.txt
    cd src/examples/libpmemobj
    make > libpmemobj.txt

## redis ## 
    
    make USE_NVML=yes STD=-std=gnu99 V=1 > make.txt



