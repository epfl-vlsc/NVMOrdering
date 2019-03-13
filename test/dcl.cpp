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

class LogEntry{
	pdcl("LogEntry.valid") int data;
	pcheck() int valid;
public:
	void persistent_code bad(){
		data = 1;
		clflush(&data);
		valid = 1;
		clflush(&valid);
		pfence();
	}

	void persistent_code bad2(){
		data = 1;
		clflush(&data);
		valid = 1;
		if(data == 1){
			clflush(&valid);
			pfence();
		}
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
