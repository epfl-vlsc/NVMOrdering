#include "annot.h"
#include <atomic>
#include <cstdlib>
#include <set>
#include <xmmintrin.h>

std::set<uint64_t> failedEpoch;
uint64_t globalEpoch;
uint64_t currExecEpoch; // first epoch in current execution

// The flow of statements are important not full implementation

void vfence() { std::atomic_thread_fence(std::memory_order_release); }

void pfence() { _mm_sfence(); }
void clflush(void const* p) { _mm_clflush(p); }

uint64_t higher(uint64_t h) { return h >> 2; }

uint64_t lower(uint64_t l) { return l << 2; }

class leafnode;
struct Log {
  leafnode* testNode;

  struct LogEntry {
    pdcl(Log::current) leafnode* data;

    void end_code log(leafnode* data_) {
      data = data_;
      clflush(&data);
      pfence();
    }

    leafnode* getEntry(){
      return data;
    }
  };

  static constexpr const int N = 100;
  LogEntry buf[N];
  int current;

  bool persistent_code logNode(leafnode* data_) {
    buf[current].log(data_);

    current++;
    clflush(&current);
    pfence();
    return true;
  }

  void recovery_code recover(){
    int i = 0;
    while(i < current){
      testNode = buf[i].getEntry();
    }
  }
};

class basenode {};

struct ValInCLL {
  friend class leafnode;
  static const int INVALIDIDX = -1;

  enum EPOCH { MASK = 4 };

  //pscl() uint64_t incll;
  uint64_t incll;

  /*
  long idx:4;
  long ptr:44; // 48 bits minus 4 least significant bits
  long lowNodeEpoch:16; // last 16 bits of the epoch;
  */

  ValInCLL(void* ptr, int idx) {
    incll = (incll & MASK) | idx;
    incll = (incll & MASK) | (uint64_t)(uintptr_t)ptr;
  }

  ValInCLL() {
    incll = (incll & MASK) | 0;
  }

  int idx() { return (incll & ~MASK); }

  void setEpoch(int epoch) { incll = (incll & ~MASK) | epoch; }

  uint64_t epoch() { return (incll & MASK); }

  uint64_t* ptr() { return (uint64_t*)(uintptr_t)(incll & ~MASK); }
};

Log log;
class leafnode : public basenode {
  basenode* parent;
  basenode* prev;
  basenode* next;
  uint64_t nodeEpoch;
  bool logged;
  bool InsAllowed;
  pscl(leafnode::nodeEpoch) uint64_t permutationInCLL;
  uint64_t permutation;
  uint64_t keys[14];
  uint64_t padding[3];
  pscl(ValInCLL::incll) ValInCLL InCLL1;
  uint64_t* vals[14];
  pscl(ValInCLL::incll) ValInCLL InCLL2;

  void remove_idx(uint64_t* permutation, int idx) {
    (void)permutation;
    (void)idx;
  }
  int insert_idx(uint64_t* permutation) {
    (void)permutation;
    return rand() % 14;
  }

  int find_idx(int key) { return (rand() + key) % 14; }

public:
  void setInCLL(bool InCLLallowed, uint64_t permInCLL,
                                ValInCLL valInCLL1, ValInCLL valInCLL2) {
    if (globalEpoch != nodeEpoch) {
      InsAllowed = true;
      logged = false;
      if (higher(globalEpoch) != higher(nodeEpoch))
        logged = log.logNode(this);
      if (!logged) {
        permutationInCLL = permInCLL;
        InCLL1 = valInCLL1;
        InCLL2 = valInCLL2;
        // order writes
        vfence();
      }
      nodeEpoch = globalEpoch;
      InCLL1.setEpoch(lower(nodeEpoch));
      InCLL2.setEpoch(lower(nodeEpoch));
    } else if (!logged && !InCLLallowed)
      logged = log.logNode(this);
    vfence();
  }

  void persistent_code remove(uint64_t key) {
  //void persistent_code remove(uint64_t key) {
    int idx = find_idx(key);
    setInCLL(true, permutation, ValInCLL(), ValInCLL());
    InsAllowed = false;
    remove_idx(&permutation, idx);
  }

  void persistent_code insert(uint64_t key, uint64_t* val) {
    int idx = insert_idx(&permutation);
    setInCLL(InsAllowed, permutation, ValInCLL(), ValInCLL());
    keys[idx] = key;
    vals[idx] = val;
  }

  void persistent_code update(int idx, uint64_t* val) {
  //void persistent_code update(int idx, uint64_t* val) {
    if (idx <= 6) {
      ValInCLL& incll = InCLL1;
      bool InCLLallowed =
          (incll.idx() == idx || incll.idx() == ValInCLL::INVALIDIDX);
      ValInCLL vc1{vals[idx], idx};
      ValInCLL vc2{nullptr, ValInCLL::INVALIDIDX};
      setInCLL(InCLLallowed, permutation, vc1, vc2);
      vals[idx] = val;
    } else {
      ValInCLL& incll = InCLL2;
      bool InCLLallowed =
          (incll.idx() == idx || incll.idx() == ValInCLL::INVALIDIDX);
      ValInCLL vc2{vals[idx], idx};
      ValInCLL vc1{nullptr, ValInCLL::INVALIDIDX};
      setInCLL(InCLLallowed, permutation, vc2, vc1);
      vals[idx] = val;
    }
  }

  // before first access to a leaf node
  void recovery_code lazyNodeRecovery() {
      if (nodeEpoch < currExecEpoch) {
        nodeRecovery();
      }
  }

  void nodeRecovery() {
    // InCLLp
    if (failedEpoch.count(nodeEpoch))
      permutation = permutationInCLL;
    nodeEpoch = currExecEpoch;
    // InCLL1
    uint64_t epoch = higher(nodeEpoch) | InCLL1.epoch();
    if (failedEpoch.count(epoch))
      vals[InCLL1.idx()] = InCLL1.ptr();
    // InCLL2
    epoch = higher(nodeEpoch) | InCLL2.epoch();
    if (failedEpoch.count(epoch))
      vals[InCLL2.idx()] = InCLL2.ptr();

    InCLL1 = ValInCLL(nullptr, ValInCLL::INVALIDIDX);
    InCLL2 = ValInCLL(nullptr, ValInCLL::INVALIDIDX);
    InCLL1.setEpoch(lower(currExecEpoch));
    InCLL2.setEpoch(lower(currExecEpoch));
  }
};