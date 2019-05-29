# NVM Ordering
* Helps with finding ordering bugs for NVM


# Requirements
* clang 8.0.0

## How to build for the first time ##
    ./run.sh build
       
## How to run ##
* Check run.sh script

    ./run.sh <report type> <checker type> <test file>
    ./run.sh scan main dcl

## TODO ##
* explain how to run benchmarks

## Unit Test Bugs ##
|main||
|---|---|
|autoscl|3|
|bit|1|
|cons|1|
|dcl|8|
|obj|2|

|ptr||
|---|---|
|ptr|4|

|log||
|---|---|
|log|1|
|logtx|2|


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

    -fno-sized-deallocation
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



