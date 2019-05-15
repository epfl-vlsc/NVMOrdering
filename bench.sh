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

PMGD_SRCS=(FixedAllocator FixSizeAllocator AvlTree \
            PropertyList VariableAllocator)
array_contains PMGD_SRCS ${TEST_NAME}
res=$?
if [ "$res" == "1" ] ;then
    CC="clang++"
    CFLAGS="--std=c++11  -O3 -fomit-frame-pointer -funit-at-a-time -fno-strict-aliasing \
-fno-threadsafe-statics -fnon-call-exceptions -fno-aligned-allocation -fPIC -Wall \
-Wno-parentheses -Wno-conversion -Wno-sign-compare -DPM -DPMFLUSH=clflushopt -MP -MMD"
    BENCH_DIR="${TEST_DIR}/pmgd"
    TEST_FILE=${BENCH_DIR}/src/${TEST_NAME}.cc
    INCLUDES="-I${BENCH_DIR}/include -I${BENCH_DIR}/.."
fi

#pmgd--------------------------------------------------------

#echo--------------------------------------------------------

ECHO_SRCS=(kp_kv_master hash kp_kv_local vector kp_kvstore)
array_contains ECHO_SRCS ${TEST_NAME}
res=$?

if [ "$res" == "1" ] ;then
    CC="clang"
    CFLAGS="-ggdb -O0 -std=c99 -D_ENABLE_FTRACE -D_GNU_SOURCE -Wall -fPIC -c"
    BENCH_DIR="${TEST_DIR}/echo"
    SRC_DIR="${BENCH_DIR}/src"
    if [ "$TEST_NAME" == "hash" ] ;then
        SRC_DIR=${SRC_DIR}/hash_table
    elif [ "$TEST_NAME" == "vector" ] ;then
        SRC_DIR=${SRC_DIR}/vector-cdds
    fi
    TEST_FILE=${SRC_DIR}/${TEST_NAME}.c
    INCLUDES="-I${BENCH_DIR}/.. -I${SRC_DIR}/vector-cdds/ \
-I${SRC_DIR}/hash_table/ -I${SRC_DIR}/threadpool/ -I${BENCH_DIR}/include"
fi

#echo--------------------------------------------------------

#nstore------------------------------------------------------  
NSTORE_SRCS=(plist_user)
array_contains NSTORE_SRCS ${TEST_NAME}
res=$?

if [ "$res" == "1" ] ;then
    CC="clang++"
    CFLAGS="-DHAVE_CONFIG_H -fsized-deallocation -Wall -Wextra \
-Werror -ggdb -Wno-pointer-arith -O3 -D_ENABLE_FTRACE -std=c++11 \
-fsized-deallocation -MT -MD -MP -Wno-unused-variable \
-Wno-unused-parameter"
    BENCH_DIR="${TEST_DIR}/nstore"
    SRC_DIR="${BENCH_DIR}/src"
    TEST_FILE=${SRC_DIR}/${TEST_NAME}.cpp
    INCLUDES="-I${BENCH_DIR}/.. -I${SRC_DIR} -I${SRC_DIR}/.. \
-I${SRC_DIR}/common"
fi

#nstore------------------------------------------------------

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
    ${CC} ${SBFLAGS} ${PLUGIN} ${CFLAGS} ${INCLUDES} ${TEST_FILE}
    cd ${BASE_DIR}

    rem_patch
}

run_scan(){
    add_patch

    #todo -analyzer-opt-analyze-headers
    cd ${BENCH_DIR}
    scan-build ${SBFLAGS} ${DISPLUGIN} ${PLUGIN} \
        ${CC} -fsyntax-only ${CFLAGS} ${INCLUDES} ${TEST_FILE}
    cd ${BASE_DIR}

    rem_patch
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