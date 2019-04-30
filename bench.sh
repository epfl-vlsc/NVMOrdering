MODE=$1 #run, scan, ast, multi
TOOL_NAME=$2 #low level(write, read), high level(txm, txp, log)
TEST_NAME=$3 #any *.cpp file under test directory
PATCH_NO=$4 #pmdk patch

#initialize info---------------------------------------------
if [ -z "$TOOL_NAME" ] ; then
	TOOL_NAME=txp
fi

if [ -z "$TEST_NAME" ] ; then
	TEST_NAME=btree_map
fi

BASE_DIR=$(dirname $(realpath "$0"))
OUT_DIR="${BASE_DIR}/output"
BUILD_DIR="${BASE_DIR}/build"
PLUGIN_DIR="${BUILD_DIR}/lib"
TEST_DIR="${BASE_DIR}/test"

# -Xclang -analyzer-display-progress"
if [ "$MODE" == "run" ] ;then
    SBFLAGS="-fsyntax-only -Xclang -analyzer-max-loop -Xclang 2"
    PLUGIN="-fplugin=${PLUGIN_DIR}/lib${TOOL_NAME}checker.so \
-Xclang -analyze -Xclang -analyzer-checker=nvm.${TOOL_NAME}checker"
elif [ "$MODE" == "scan" ] ;then
    SBFLAGS="--force-analyze-debug-code -v -stats -maxloop 2 -o ${OUT_DIR}"
    PLUGIN="-load-plugin ${PLUGIN_DIR}/lib${TOOL_NAME}checker.so \
-enable-checker nvm.${TOOL_NAME}checker"
    DISPLUGIN="-disable-checker alpha,apiModeling,valist,\
cplusplus,deadcode,debug,llvm,nullability,optin,security,osx,core,unix"
fi

array_contains () { 
    local array="$1[@]"
    local seeking=$2
    local found=0
    for element in "${!array}"; do
        if [[ $element == $seeking ]]; then
            found=1
            break
        fi
    done
    return $found
}

#nvml--------------------------------------------------------

NVML_SRCS=(btree_map ctree_map rbtree_map hashmap_tx)
array_contains NVML_SRCS ${TEST_NAME}
res=$?
if [ "$res" == "1" ] ;then
    CC="clang"
    CFLAGS="-c -std=gnu99 -ggdb -Wall -Werror -fPIC"
    BENCH_DIR="${TEST_DIR}/pmdk"
    TEST_FILE=${BENCH_DIR}/${TEST_NAME}.c
    INCLUDES="-I${BENCH_DIR}/include -I${BENCH_DIR} -I${BENCH_DIR}/.."
fi

#nvml--------------------------------------------------------

#pmgd--------------------------------------------------------

PMGD_SRCS=(FixedAllocator FixSizeAllocator PropertyList VariableAllocator)
array_contains PMGD_SRCS ${TEST_NAME}
res=$?
if [ "$res" == "1" ] ;then
    CC="clang++"
    CFLAGS="--std=c++11  -O3 -fomit-frame-pointer -funit-at-a-time -fno-strict-aliasing \
-fno-threadsafe-statics -fnon-call-exceptions -fPIC -Wall -Wno-parentheses -Wno-conversion \
-Wno-sign-compare -DPM -DPMFLUSH=clflushopt -MP -MMD"
    BENCH_DIR="${TEST_DIR}/pmgd"
    TEST_FILE=${BENCH_DIR}/src/${TEST_NAME}.cc
    INCLUDES="-I${BENCH_DIR}/include -I${BENCH_DIR}/.."
fi

#pmgd--------------------------------------------------------

PATCH_DIR="${TEST_DIR}/patch"
PATCH_FILE="${PATCH_DIR}/${TOOL_NAME}_${PATCH_NO}_${TEST_NAME}.txt"
#initialize info---------------------------------------------

#functions---------------------------------------------------

create_patch(){
    git diff > ${PATCH_FILE}
    git reset --hard
}

add_patch(){
    if [ "$PATCH_NO" ] ; then
	    git apply ${PATCH_FILE}
    fi
}

rem_patch(){
    if [ "$PATCH_NO" ] ; then
	    git apply -R ${PATCH_FILE}
    fi
}

run_make(){
	cd ${BUILD_DIR}
	make -j$(nproc)
	cd ${BASE_DIR}
}

run_run(){
    add_patch

    cd ${BENCH_DIR}
    #echo "clang ${SBFLAGS} ${PLUGIN} ${CFLAGS} ${INCLUDES} ${TEST_FILE}"
    clang ${SBFLAGS} ${PLUGIN} ${CFLAGS} ${INCLUDES} ${TEST_FILE}
    cd ${BASE_DIR}

    rem_patch
}

run_scan(){
    clang ${SBFLAGS} ${PLUGIN} ${CFLAGS} ${INCLUDES} ${TEST_FILE}
    cd ${BENCH_DIR}

    #todo -analyzer-opt-analyze-headers
    cd ${BUILD_DIR}
    scan-build ${SBFLAGS} ${DISPLUGIN} ${PLUGIN} \
        clang++ -fsyntax-only ${TEST_FILE}
    cd ${BASE_DIR}
}
#functions---------------------------------------------------

#commands----------------------------------------------------
if [ "$MODE" == "run" ] ;then
	run_make
	run_run
elif [ "$MODE" == "scan" ] ;then
	run_make
	run_scan
elif [ "$MODE" == "build" ] ;then
    ./run.sh build
elif [ "$MODE" == "patch" ] ;then
    create_patch
else
	echo "run, scan, build, patch"
fi
#commands----------------------------------------------------