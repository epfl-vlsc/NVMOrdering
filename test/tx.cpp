#include "annot.h"
#include <cstdlib>
#include <cstdio>

void* pmalloc(size_t size){
    return malloc(size);
}

void pfree(void* ptr){
    free(ptr);
}

void assign(int* ptr, int data){
    *ptr = data;
}

void correctAccess(){
    tx_beg
    int *a = (int*)pmalloc(sizeof(int));
    assign(a, 2);
    pfree(a);
    tx_end
}

void allocateOutsideTransaction(){
    int *a = (int*)pmalloc(sizeof(int));
    tx_beg
    assign(a, 2);
    pfree(a);
    tx_end
}

void writeOutsideTransaction(){
    tx_beg
    int *a = (int*)pmalloc(sizeof(int));
    tx_end
    assign(a, 2);
    pfree(a);
}

void deleteOutsideTransaction(){
    tx_beg
    int *a = (int*)pmalloc(sizeof(int));
    assign(a, 2);
    tx_end
    pfree(a);
}