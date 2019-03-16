mode=$1
test_name=$2
if [ -z "$test_name" ]
  then
	test_name=dcl
fi

test_file=../test/$test_name.cpp

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
        -fsyntax-only -fplugin=lib/liborderingchecker.so -Xclang -analyzer-max-loop -Xclang 2 \
        -Xclang -analyze -Xclang -analyzer-checker=nvm.orderingchecker \
        $test_file -Xclang -analyzer-display-progress
    cd ..
}

run_tool2(){
    cd build
    clang++ \
        -fsyntax-only -fplugin=lib/liborderingchecker.so -Xclang -analyzer-max-loop -Xclang 2 \
        -Xclang -analyze -Xclang -analyzer-checker=nvm.orderingchecker \
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
        -fsyntax-only -fplugin=lib/liborderingchecker.so \
        -Xclang -analyzer-max-loop -Xclang 2 \
        -Xclang -analyze -Xclang -analyzer-checker=nvm.orderingchecker \
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
elif [ "$mode" == "build" ] ;then
    rm -r build
	create_build
    run_make
elif [ "$mode" == "ast" ] ;then 
    dump_ast
else
	echo "run, build, ast"
fi