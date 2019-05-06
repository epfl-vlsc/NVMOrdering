#include <atomic>

// forward decl
class LogEntry;
class Node;

using NodeA = std::atomic<Node*>;

struct Node {
  int value;
  NodeA next;
  LogEntry* logInsert;
  LogEntry* logRemove;
  Node(int val) : value(val), next(nullptr), logInsert(nullptr), logRemove(nullptr) {}
};

struct LogEntry {
  int operationNum;
  bool status;
  NodeA node;
  LogEntry() {}
};

struct LogQueue {
  NodeA head;
  NodeA tail;
  LogEntry* logs[3];

  void enq(int value, int threadID, int operationNumber) {
    LogEntry* log = new LogEntry();
    Node* node = new Node(value); 
    log->node = node;
    node->logInsert = log;
    logs[threadID] = log;
    while (true) {
      Node* last = tail.load();
      Node* next = last->next.load();
      if (last == tail) {
        if (next == nullptr) {
          if (last->next.compare_exchange_weak(next, node)) {
            tail.compare_exchange_weak(last, node);
            return;
          }
        } else {
          tail.compare_exchange_weak(last, next);
        }
      }
    }
  }
};