#include "annot.h"
#include <xmmintrin.h>
#include <atomic>
#include <cstdlib>
#include <set>

std::set<uint64_t> failedEpoch;
uint64_t globalEpoch;
uint64_t currExecEpoch; // first epoch in current execution

//The flow of statements are important not full implementation

void vfence(){
	std::atomic_thread_fence(std::memory_order_release);
}

void pfence(){
	_mm_sfence();
}
void clflush(void const *p){
	_mm_clflush(p);
}

uint64_t higher(uint64_t h){
	return h;
}

uint64_t lower(uint64_t l){
	return l;
}

class leafnode;
struct Log{
	struct LogEntry{
		pdcl(Log.current) leafnode* data;

		void log(leafnode* data_){
			data = data_;
			clflush(&data);
			pfence();
		}
	};

	LogEntry buf[100];
	pcheck() int current;

	bool logNode(leafnode* data_){
		buf[current].log(data_);

		current++;
		clflush(&current);
		pfence();
		return true;
	}

};

class basenode{};

struct ValInCLL{
	friend class leafnode;
	static const int INVALIDIDX=-1;

    enum EPOCH{
		epoch_mask 			= 4,
		non_epoch_mask 		= ~epoch_mask,
	};

	pcheck(epoch_mask) uint64_t incll;
	
	/*
	long idx:4;
	long ptr:44; // 48 bits minus 4 least significant bits
	long lowNodeEpoch:16; // last 16 bits of the epoch;
	*/

	ValInCLL(void *ptr, int idx){
		(void)ptr;
		incll = (incll & non_epoch_mask) | idx;
		incll = (incll & non_epoch_mask) | (uint64_t)(uintptr_t)ptr;
	}

	ValInCLL(){
		init(3);
	}

	void init(int data){
		incll = (incll & non_epoch_mask) | data;
	}

	int idx(){
		return (incll & non_epoch_mask);
	}

	void setEpoch(int epoch){
		incll = (incll & epoch_mask) | epoch;
	}

	uint64_t epoch(){
		return (incll & epoch_mask);
	}

	uint64_t* ptr(){
		return (uint64_t*)(uintptr_t)(incll & non_epoch_mask);
	}

};

Log log;
class leafnode : public basenode{
	basenode *parent;
	basenode* prev;
	basenode *next;
	pcheck() uint64_t nodeEpoch;
	pscl(leafnode.nodeEpoch) bool logged;
	pscl(leafnode.nodeEpoch) bool InsAllowed;
	pscl(leafnode.nodeEpoch) uint64_t permutationInCLL;
	uint64_t permutation;
	uint64_t keys[14];
	uint64_t padding[3];
	pscl(ValInCLL.incll) ValInCLL InCLL1;
	uint64_t* vals[14];
	pscl(ValInCLL.incll) ValInCLL InCLL2;

	void remove_idx(uint64_t* permutation, int idx){
		(void)permutation;
		(void)idx;
	}
	int insert_idx(uint64_t* permutation){
		(void)permutation;
		return rand()%14;
	}

	int find_idx(int key){
		return (rand()+key)%14;
	}
public:

	void persistent_code setInCLL(bool InCLLallowed, uint64_t permInCLL, ValInCLL valInCLL1, ValInCLL valInCLL2){
		if(globalEpoch != nodeEpoch){
			InsAllowed = true;
			logged = false;
			if(higher(globalEpoch) != higher(nodeEpoch))
				logged = log.logNode(this);
			if(!logged){
				permutationInCLL = permInCLL;
				InCLL1 = valInCLL1;
				InCLL2 = valInCLL2;
				// order writes
				vfence();
			}
			nodeEpoch = globalEpoch;
			InCLL1.setEpoch(lower(nodeEpoch));
			InCLL2.setEpoch(lower(nodeEpoch));
		}
		else if(!logged && !InCLLallowed)
			logged = log.logNode(this);
		vfence();
	}

	void pad(){
		(void)padding;
		(void)parent;
		(void)prev;
		(void)next;
	}

	void remove(uint64_t key){
		int idx = find_idx(key);
		setInCLL(true, permutation, ValInCLL(), ValInCLL());
		InsAllowed=false;
		remove_idx(&permutation,idx);
	}

	void insert(uint64_t key, uint64_t* val){
		int idx = insert_idx(&permutation);
		setInCLL(InsAllowed, permutation, ValInCLL(), ValInCLL());
		keys[idx] = key;
		vals[idx] = val;
	}

	void update(int idx, uint64_t* val){
		if(idx<=6){
			ValInCLL& incll = InCLL1;
			bool InCLLallowed = (incll.idx() == idx || incll.idx() == ValInCLL::INVALIDIDX);
			ValInCLL vc1{vals[idx], idx};
			ValInCLL vc2{nullptr, ValInCLL::INVALIDIDX};
			setInCLL(InCLLallowed, permutation, vc1, vc2);
			vals[idx] = val;
		}else{
			ValInCLL& incll = InCLL2;
			bool InCLLallowed = (incll.idx() == idx || incll.idx() == ValInCLL::INVALIDIDX);
			ValInCLL vc2{vals[idx], idx};
			ValInCLL vc1{nullptr, ValInCLL::INVALIDIDX};
			setInCLL(InCLLallowed, permutation, vc2, vc1);
			vals[idx] = val;
		}
	}

	// before first access to durable Masstree
	void recovery(){
		//log procedure
	}
	// before first access to a leaf node
	void lazyNodeRecovery(){
		if(nodeEpoch<currExecEpoch){
			if(nodeEpoch<currExecEpoch){
				nodeRecovery();
			}

		}
	}

	void recovery_code nodeRecovery(){
		// InCLLp
		if(failedEpoch.count(nodeEpoch))
			permutation = permutationInCLL;
		nodeEpoch = currExecEpoch;
		// InCLL1
		uint64_t epoch = higher(nodeEpoch) | InCLL1.epoch();
		if(failedEpoch.count(epoch))
			vals[InCLL1.idx()] = InCLL1.ptr();
		// InCLL2
		epoch = higher(nodeEpoch) | InCLL2.epoch();
		if(failedEpoch.count(epoch))
			vals[InCLL2.idx()] = InCLL2.ptr();

		InCLL1 = ValInCLL(nullptr, ValInCLL::INVALIDIDX);
		InCLL2 = ValInCLL(nullptr, ValInCLL::INVALIDIDX);
		InCLL1.setEpoch(lower(currExecEpoch));
		InCLL2.setEpoch(lower(currExecEpoch));
	}

};


void use(){
	leafnode ln;
	uint64_t key = 2;
	uint64_t* val = new uint64_t(7);
	uint64_t* val2 = new uint64_t(8);
	ln.insert(key, val);
	ln.update(key, val2);
	ln.remove(key);
	ln.pad();

	delete val;
	delete val2;
}
