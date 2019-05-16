MODE=$1 #run, scan, ast, multi
TOOL_NAME=$2 #low level(write, read), high level(txm, txp, log)
TEST_NAME=$3 #any *.cpp file under test directory
PATCH_NO=$4 #pmdk patch

#initialize info---------------------------------------------
if [ -z "$TEST_NAME" ] ; then
	TEST_NAME=dcl
fi

if [ -z "$TOOL_NAME" ] ; then
	TOOL_NAME=mc
fi

BASE_DIR=$(dirname $(realpath "$0"))
OUT_DIR="${BASE_DIR}/output"
BUILD_DIR_NAME="build"
BUILD_DIR="${BASE_DIR}/${BUILD_DIR_NAME}"
PLUGIN_DIR="${BUILD_DIR}/lib"
TEST_DIR="${BASE_DIR}/test"
SINGLE_FILE_REPO=${TEST_DIR}/single_file
TEST_FILE=${SINGLE_FILE_REPO}/$TEST_NAME.cpp

PATCH_DIR=${TEST_DIR}

if [ "$MODE" == "run" ] || [ "$MODE" == "mini" ] ;then
    SBFLAGS="-fsyntax-only -Xclang -analyzer-max-loop -Xclang 2
-Xclang -analyzer-display-progress -Xclang -analyzer-config -Xclang c++-allocator-inlining=true"
    PLUGIN="-fplugin=${PLUGIN_DIR}/lib${TOOL_NAME}checker.so \
    -Xclang -analyze -Xclang -analyzer-checker=nvm.${TOOL_NAME}checker"
elif [ "$MODE" == "scan" ] || [ "$MODE" == "multi" ] ;then
    SBFLAGS="--force-analyze-debug-code -v -stats -maxloop 2 -o ${OUT_DIR}"
    PLUGIN="-load-plugin ${PLUGIN_DIR}/lib${TOOL_NAME}checker.so \
    -enable-checker nvm.${TOOL_NAME}checker"
    DISPLUGIN="-disable-checker alpha,apiModeling,valist,\
cplusplus,deadcode,debug,nullability,optin,security,osx,core,unix"
elif [ "$MODE" == "ast" ] ;then
    SBFLAGS="-Xclang -ast-dump -fsyntax-only"
fi

#initialize info---------------------------------------------

#functions---------------------------------------------------
run_multi(){
    # do not use -analyze-headers
    cd ${TEST_DIR}/project
    make clean
    scan-build ${SBFLAGS} ${PLUGIN} ${DISPLUGIN} make
}

create_build(){
	mkdir -p ${BUILD_DIR_NAME}
	cd ${BUILD_DIR}
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=True \
        -DLLVM_DIR=~/llvm_compiler4/llvm_build/lib/cmake/llvm/ \
        -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
    cd ${BASE_DIR}
}

run_make(){
	cd ${BUILD_DIR}
	make -j$(nproc)
	cd ${BASE_DIR}
}

run_tool(){
    cd ${BUILD_DIR}
    clang++ ${SBFLAGS} ${PLUGIN} ${TEST_FILE}
    cd ${BASE_DIR}
}

run_mini(){
    PMDK_DIR=${TEST_DIR}/mini
    cd ${BUILD_DIR}
    clang ${SBFLAGS} ${PLUGIN} -c -std=gnu99 -ggdb -Wall -Werror -fPIC \
    -I$PMDK_DIR/include -I$PMDK_DIR $PMDK_DIR/btree_map.c
    cd ${BASE_DIR}
}

run_scanbuild(){
    #todo -analyzer-opt-analyze-headers
    cd ${BUILD_DIR}
    scan-build ${SBFLAGS} ${DISPLUGIN} ${PLUGIN} \
        clang++ -fsyntax-only ${TEST_FILE}
    cd ${BASE_DIR}
}

dump_ast(){
    clang++ ${SBFLAGS} ${TEST_FILE} > ./test/ast.txt
}
#functions---------------------------------------------------


#commands----------------------------------------------------
if [ "$MODE" == "run" ] ;then
	run_make
	run_tool
elif [ "$MODE" == "scan" ] ;then
	run_make
	run_scanbuild
elif [ "$MODE" == "make" ] ;then
	run_make
elif [ "$MODE" == "build" ] ;then
    rm -r build
	create_build
    run_make
elif [ "$MODE" == "ast" ] ;then 
    dump_ast
elif [ "$MODE" == "multi" ] ;then 
    run_make
    run_multi
elif [ "$MODE" == "mini" ] ;then 
    PMDK_DIR=${TEST_DIR}/mini
    run_make
    run_mini
else
	echo "run, build, ast"
fi
#commands----------------------------------------------------