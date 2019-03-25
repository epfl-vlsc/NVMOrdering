mode=$1 #run, scan, ast
test_name=$2 #dcl, scl, dscl, mask, simple_masstree
tool_name=$3 #ordering, recovery, transaction
if [ -z "$test_name" ]
  then
	test_name=dcl
fi

if [ -z "$tool_name" ]
  then
	tool_name=multicompile
fi

test_file=../test/$test_name.cpp


compile_multi(){
    #todo -analyzer-opt-analyze-headers
    cd build
    OUT_DIR=../output
    #rm -r $OUT_DIR/*
    scan-build -load-plugin lib/libmulticompilechecker.so -o $OUT_DIR \
        -disable-checker alpha -disable-checker apiModeling \
        -disable-checker valist -disable-checker cplusplus \
        -disable-checker deadcode -disable-checker debug \
        -disable-checker llvm -disable-checker nullability  \
        -disable-checker optin -disable-checker security  \
        -disable-checker osx -disable-checker unix -disable-checker core \
        clang++ \
        -fsyntax-only  \
        -Xclang -analyzer-max-loop -Xclang 2 \
        $test_file -Xclang -analyzer-display-progress
    cd ..
}

create_build(){
	mkdir -p build
	cd build
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=True \
        -DLLVM_DIR=~/llvm_compiler4/llvm_build/lib/cmake/llvm/ \
        -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
    cd ..
}

run_make(){
	cd build
	make -j$(nproc)
	cd ..
}

run_tool(){
    cd build
    clang++ \
        -fsyntax-only -fplugin=lib/lib${tool_name}checker.so -Xclang -analyzer-max-loop -Xclang 2 \
        -Xclang -analyze -Xclang -analyzer-checker=nvm.${tool_name}checker \
        $test_file -Xclang -analyzer-display-progress
    cd ..
}

run_tool2(){
    cd build
    clang++ \
        -fsyntax-only -fplugin=lib/lib${tool_name}checker.so -Xclang -analyzer-max-loop -Xclang 2 \
        -Xclang -analyze -Xclang -analyzer-checker=nvm.${tool_name}checker \
        $test_file -Xclang -analyzer-display-progress
    cd ..
}

run_scanbuild(){
    #todo -analyzer-opt-analyze-headers
    cd build
    OUT_DIR=../output
    #rm -r $OUT_DIR/*
    scan-build -o $OUT_DIR \
        -disable-checker alpha -disable-checker apiModeling \
        -disable-checker valist -disable-checker cplusplus \
        -disable-checker deadcode -disable-checker debug \
        -disable-checker llvm -disable-checker nullability  \
        -disable-checker optin -disable-checker security  \
        -disable-checker osx -disable-checker unix -disable-checker core \
        clang++ \
        -fsyntax-only -fplugin=lib/lib${tool_name}checker.so \
        -Xclang -analyzer-max-loop -Xclang 2 \
        -Xclang -analyze -Xclang -analyzer-checker=nvm.${tool_name}checker \
        $test_file -Xclang -analyzer-display-progress
    cd ..
}

dump_ast(){
    cd build
    clang++ -Xclang -ast-dump -fsyntax-only $test_file > ../test/ast.txt
    cd ..
}

#fsyntax do not create object file
#fplugin load plugin
#display progress

if [ "$mode" == "run" ] ;then
	run_make
	run_tool
elif [ "$mode" == "scan" ] ;then
	run_make
	run_scanbuild
elif [ "$mode" == "make" ] ;then
	run_make
elif [ "$mode" == "build" ] ;then
    rm -r build
	create_build
    run_make
elif [ "$mode" == "ast" ] ;then 
    dump_ast
elif [ "$mode" == "multi" ] ;then 
    run_make
    compile_multi
else
	echo "run, build, ast"
fi