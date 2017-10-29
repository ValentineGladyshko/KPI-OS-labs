#include <iostream>
#include <map>
#include <vector>

using std::map;
using std::vector;

typedef long align; //Align by 32bits

#define ROUNDALIGN(a, b) ((a) / (b) + (((a) % (b)) ? 1 : 0))  //a - size to round, b - base size

union MemoryHeader
{
    struct MemHeader
	{
            MemoryHeader* prev;
            MemoryHeader* next;
            size_t sz;
            bool free;
    } data;
    align x[ROUNDALIGN(sizeof(MemHeader), sizeof(align))]; // Align union size by `align` type size  (32 or 64 or 96... bits)
};

class Allocator
{
    char* mempool;
    size_t memsize;
    map<size_t, vector<void*>> freeBlocks; // free blocks vectors by size 

private:
    void _mergeWithNext(MemoryHeader* block);

public:
    Allocator();
    Allocator(size_t size);

    void init(size_t size);
    void* allocate(size_t size);
    void* reallocate(void* mem, size_t new_size);
    void free(void* mem);
    void mem_dump();
};