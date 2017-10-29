#include "allocator.h"
#include <iostream>

#define ALLOC_SIZE 1048576 //1MiB
Allocator allocator(ALLOC_SIZE);

using std::cin;
using std::cout;
using std::endl;

void* mem_alloc(size_t size)
{
    return allocator.allocate(size);
}

void *mem_realloc(void *addr, size_t size)
{
    return allocator.reallocate(addr, size);
}

void mem_free(void *addr)
{
    allocator.free(addr);
}

void mem_dump()
{
    allocator.mem_dump();
}

int main()
{ 
	void* mem = mem_alloc(sizeof(int));
	void* mem1 = mem_alloc(sizeof(int) * 100);
    mem_dump();

    mem_free(mem);
    mem_dump();

    mem1 = mem_realloc(mem1, sizeof(int) * 10);
	void* mem2 = mem_alloc(sizeof(int) * 5);
    mem_dump();

	mem_free(mem1);
	mem_dump();

	system("pause");
}