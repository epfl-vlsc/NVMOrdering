
#include <cstdlib>
#define MAX_THREADS 10
void FLUSH(const void* ptr) {}
bool CAS(const void* ptr1, const void* ptr2, const void* ptr3) {
  return rand() > 10 ? true : false;
}

enum Operation { ENQ, DEQ };

struct Node {
  int value;
  Node* next;
  LogEntry* logInsert;
  LogEntry* logRemove;
  Node(int val)
      : value(val), next(nullptr), logInsert(nullptr), logRemove(nullptr) {}
};

struct LogEntry {
  int operationNum;
  Operation operation;
  bool status;
  Node* node;
  LogEntry(bool s, Node* n, Operation a, int opNum)
      : operationNum(opNum), operation(a), status(s), node(n) {}
};
struct LogQueue {
  Node* head;
  Node* tail;
  LogEntry* logs[MAX_THREADS];
  LogQueue() {
    Node* sentinel = new Node(0);
    FLUSH(sentinel);
    head = sentinel;
    FLUSH(&head);
    tail = sentinel;
    FLUSH(&tail);
    for (int i = 0; i < MAX_THREADS; ++i) {
      logs[i] = nullptr; // for every thread
      FLUSH(&logs[i]);   // for every thread
    }
  }

  void enq(int value, int threadID, int operationNumber) {
    LogEntry* log =
        new LogEntry(false, nullptr, Operation::ENQ, operationNumber);
    Node* node = new Node(value);
    log->node = node;
    node->logInsert = log;
    FLUSH(node);
    FLUSH(log);
    logs[threadID] = log;
    FLUSH(&logs[threadID]);
    while (true) {
      Node* last = tail;
      Node* next = last->next;
      if (last == tail) {
        if (next == nullptr) {
          if (CAS(&last->next, next, node)) {
            FLUSH(&last->next);
            CAS(&tail, last, node);
            return;
          }

        } else {
          FLUSH(&last->next);
          CAS(&this->tail, last, next);
        }
      }
    }
  }

  void deq(int threadID, int operationNumber) {
    LogEntry* log = new LogEntry(false, NULL, Operation::DEQ, operationNumber);
    FLUSH(log);
    logs[threadID] = log;
    FLUSH(&logs[threadID]);
    while (true) {
      Node* first = this->head;
      Node* last = this->tail;
      Node* next = first->next;
      if (first == this->head) {
        if (first == last) {
          if (next == NULL) {
            logs[threadID]->status = true;
            FLUSH(&(logs[threadID]->status));
            return;
          }
          FLUSH(&last->next);
          CAS(&tail, last, next);

        } else {
          if (CAS(&next->logRemove, NULL, log)) {
            FLUSH(&first->next->logRemove);
            next->logRemove->node = first->next;
            FLUSH(&first->next->logRemove->node);
            CAS(&head, first, next);
            return;

          } else {
            if (head == first) { // same context
              FLUSH(&first->next->logRemove);
              next->logRemove->node = first->next;
              FLUSH(&next->logRemove->node);
              CAS(&head, first, next);
            }
          }
        }
      }
    }
  }
};
