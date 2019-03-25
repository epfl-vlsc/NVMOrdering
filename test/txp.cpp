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

void correctWrite(){
    tx_beg
    int *a = (int*)pmalloc(sizeof(int));
    tx_add(a);
    assign(a, 2);
    pfree(a);
    tx_end
}

void wrongLogBeforeWrite(){
    tx_beg
    int *a = (int*)pmalloc(sizeof(int));
    assign(a, 2);
    pfree(a);
    tx_end
}

