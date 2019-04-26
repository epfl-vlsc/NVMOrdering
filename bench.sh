MODE=$1 #run or scan
TOOL_NAME=$2 #low level(write, read), high level(txm, txp, log)
TEST_NAME=$3 #btree ctree rbtree hashmaptx
PATCH_NO=$3 #patch no

#initialize info---------------------------------------------
if [ -z "$TOOL_NAME" ] ; then
	TOOL_NAME=txp
fi

if [ -z "$TEST_NAME" ] ; then
	TEST_NAME=btree
fi

BASE_DIR=$(dirname $(realpath "$0"))
OUT_DIR="${BASE_DIR}/output"
BUILD_DIR="${BASE_DIR}/build"
PLUGIN_DIR="${BUILD_DIR}/lib"
BENCH_DIR="${BASE_DIR}/benchmark"

if [ "$MODE" == "run" ] ;then
    SBFLAGS="-fsyntax-only -Xclang -analyzer-max-loop -Xclang 2 -Xclang -analyzer-display-progress"
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
    CFLAGS="-c -std=gnu99 -ggdb -Wall -Werror -fPIC"
    INCLUDES="-I../include -I."
    NVML_DIR=${BENCH_DIR}/nvml
    TEST_FILE=${TEST_NAME}.c

    if [ "${TEST_NAME}" == "btree_map" ] \
        || [ "${TEST_NAME}" == "ctree_map" ] \
        || [ "${TEST_NAME}" == "rbtree_map" ];then
        COMP_DIR=${NVML_DIR}/tree_map
    elif [ "${TEST_NAME}" == "hashmap_tx" ] ;then
        COMP_DIR=${NVML_DIR}/hashmap
    fi
fi

#nvml--------------------------------------------------------

#initialize info---------------------------------------------

#functions---------------------------------------------------
run_make(){
	cd ${BUILD_DIR}
	make -j$(nproc)
	cd ${BASE_DIR}
}

run_bench(){
    cd ${COMP_DIR}
    echo "clang ${SBFLAGS} ${PLUGIN} ${CFLAGS} ${INCLUDES} ${TEST_FILE}"
    clang ${SBFLAGS} ${PLUGIN} ${CFLAGS} ${INCLUDES} ${TEST_FILE}
    cd ${BASE_DIR}
}

scan_bench(){
    clang ${SBFLAGS} ${PLUGIN} ${CFLAGS} ${INCLUDES} ${TEST_FILE}
    cd ${BASE_DIR}

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
	run_bench
elif [ "$MODE" == "scan" ] ;then
	run_make
	scan_bench
elif [ "$MODE" == "build" ] ;then
    ./run.sh build
else
	echo "run, scan, build"
fi
#commands----------------------------------------------------