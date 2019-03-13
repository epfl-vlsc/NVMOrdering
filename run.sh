mode=$1

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
        ../test/dcl.cpp -Xclang -analyzer-display-progress
    cd ..
}

#fsyntax do not create object file
#fplugin load plugin
#display progress

if [ "$mode" == "run" ] ;then
	run_make
	run_tool
elif [ "$mode" == "build" ] ;then
    rm -r build
	create_build
    run_make
else
	echo "help, run, build, ir, test"
fi