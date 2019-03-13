#include <stdio.h>
#include <xmmintrin.h>
#include <atomic>
#include "annot.h"

//different cache line placement analysis

void vfence(){
	std::atomic_thread_fence(std::memory_order_release);
}

void pfence(){
	_mm_sfence();
}
void clflush(void const *p){
	_mm_clflush(p);
}

void assign(int* a, int val){
	*a = val;
}


struct ParentA{
	int a;
};

class ChildB{
	ParentA a;
	int b;
};

struct LogEntry{
	pdcl("LogEntry.valid") int data;
	pcheck() int valid;

	LogEntry():data(0), valid(0){}

	void setData(int data_){
		data = 1;
	}

	void setValid(int valid_){
		valid = 1;
	}

	void flushData(){
		clflush(&data);
	}

	void flushValid(){
		clflush(&valid);
	}

	void persistent_code good(){
		data = 1;
		clflush(&data);
		pfence();
		valid = 1;
		clflush(&valid);
		pfence();
	}
};


void persistent_code good(){
	LogEntry entry;
	entry.setData(1);
	entry.flushData();
	pfence();
	entry.setValid(1);
	entry.flushValid();
	pfence();
}

void megaman(){
	LogEntry entry;
	entry.setData(1);
	entry.flushData();
	pfence();
	entry.setValid(1);
	entry.flushValid();
	pfence();
}