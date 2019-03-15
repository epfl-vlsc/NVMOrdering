#include <stdio.h>
#include <xmmintrin.h>
#include <atomic>
#include "annot.h"

//different cache line placement analysis

void vfence()
{
    std::atomic_thread_fence(std::memory_order_release);
}

void pfence()
{
    _mm_sfence();
}
void clflush(void const *p)
{
    _mm_clflush(p);
}

void assign(int *a, int val)
{
    *a = val;
}

struct ParentA
{
    int a;
};

class ChildB
{
    ParentA a;
    int b;
};

struct LogEntry
{
    enum
    {
        MASK = 7,
    };

    pscl(LogEntry.valid) int data;
    pcheck() int valid;
    pscl(LogEntry.chunk) int cdata;
    pcheck(MASK) int chunk;

    LogEntry()
    {
        data = 0;
        cdata = 0;
        chunk = (chunk & ~MASK) | 0;
        vfence();
        chunk = (chunk & MASK) | 0;
        valid=0;
    }

    void setData(int data_)
    {
        data = 1;
        cdata  = 1;
        chunk = (chunk & ~MASK) | 1;
    }

    void setValid(int valid_)
    {
        valid=1;
        chunk = (chunk & MASK) | valid_;
    }

    void persistent_code usingPfence()
    {
        cdata = 1;
        chunk = (chunk & ~MASK) | 1;
        pfence();
        chunk = (chunk & MASK) | 1;
    }

    void persistent_code fenceMissing()
    {
        chunk = (chunk & ~MASK) | 1;
        chunk = (chunk & MASK) | 1;
    }
};

void persistent_code correctModelFunction()
{
    LogEntry *entry = new LogEntry;
    entry->setData(1);
    vfence();
    entry->setValid(1);
}

void persistent_code writeToValid()
{
    LogEntry *entry = new LogEntry;
    entry->setValid(1);
}
