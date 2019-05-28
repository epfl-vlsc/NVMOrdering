TOOL_NAME=$1 #low level(write, read), high level(txm, txp, log)
TEST_NAME=$2 #any *.cpp file under test directory
PATCH_NO=$3 #pmdk patch

#initialize info---------------------------------------------
if [ -z "$TOOL_NAME" ] ; then
	TOOL_NAME=main
fi

if [ -z "$TEST_NAME" ] ; then
	TEST_NAME=echo
fi

BASE_DIR=$(dirname $(realpath "$0"))
OUT_DIR="${BASE_DIR}/output"
BUILD_DIR="${BASE_DIR}/build"
PLUGIN_DIR="${BUILD_DIR}/lib"
TEST_DIR="${BASE_DIR}/../NvmBenchmarks"

#SBFLAGS = -v -stats
SBFLAGS="--force-analyze-debug-code -maxloop 2 -o ${OUT_DIR}"
PLUGIN="-load-plugin ${PLUGIN_DIR}/lib${TOOL_NAME}checker.so \
-enable-checker nvm.${TOOL_NAME}checker"
DISPLUGIN="-disable-checker alpha,apiModeling,valist,\
cplusplus,deadcode,debug,nullability,optin,security,osx,core,unix"


#nvml--------------------------------------------------------

#nvml--------------------------------------------------------

#pmgd--------------------------------------------------------

#pmgd--------------------------------------------------------

#echo--------------------------------------------------------

if [ "$TEST_NAME" == "echo" ] ;then
    BENCH_DIR="${TEST_DIR}/whisper/kv-echo/echo/src"
    MAKE="make"
    CLEAN="make clean"
fi

#echo--------------------------------------------------------

PATCH_DIR="${TEST_DIR}/patch"
PATCH_FILE="${PATCH_DIR}/${TOOL_NAME}_${PATCH_NO}_${TEST_NAME}.txt"
#initialize info---------------------------------------------

#functions---------------------------------------------------

run_make(){
	cd ${BUILD_DIR}
	make -j$(nproc)
	cd ${BASE_DIR}
}

run_scan(){
    #todo -analyzer-opt-analyze-headers
    cd ${BENCH_DIR}
    ${CLEAN}
    scan-build ${SBFLAGS} ${DISPLUGIN} ${PLUGIN} ${MAKE} 
    cd ${BASE_DIR}
}
#functions---------------------------------------------------

#commands----------------------------------------------------
run_make
run_scan
#commands----------------------------------------------------