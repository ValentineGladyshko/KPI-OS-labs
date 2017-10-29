#include "allocator.h"
#include <cstring>
#include <algorithm>

using std::cin;
using std::cout;
using std::endl;

Allocator::Allocator(size_t size)
{
    init(size);

}

void Allocator::init(size_t size)
{
    mempool = new char[size + sizeof(MemoryHeader)];
    memsize = size;

    MemoryHeader* head = new (mempool) MemoryHeader();

    head->data.next = NULL;
    head->data.prev = NULL;
    head->data.sz = memsize;
    head->data.free = true;

    freeBlocks[memsize].push_back(mempool);
}

void *Allocator::allocate(size_t size)
{
	cout << "Allocate" << endl;
    size = sizeof(MemoryHeader) + sizeof(align) * ROUNDALIGN(size, sizeof(align));

    auto it = freeBlocks.lower_bound(size);

    if(it == freeBlocks.end())
        return NULL;

    MemoryHeader *bl = (MemoryHeader*)it->second.back(); 
    it->second.pop_back();

    if(it->second.empty()){
        freeBlocks.erase(it);
    }

    MemoryHeader *next_block = bl->data.next;
    size_t memleft = bl->data.sz - size;
    
    bl->data.sz = size;
    bl->data.free = false;
    
    if(memleft >= sizeof(MemoryHeader) + sizeof(align)) //Split block
	{
        MemoryHeader* newBlock = new ((char*)bl + bl->data.sz) MemoryHeader();

        newBlock->data.sz = memleft;
        newBlock->data.prev = bl;
        newBlock->data.next = next_block;
        newBlock->data.free = true;

        bl->data.next = newBlock;

        freeBlocks[memleft].push_back((void*)newBlock);        
    } 

    return (void*)((char*)bl + sizeof(MemoryHeader));
}

void *Allocator::reallocate(void* mem, size_t new_size)
{
	cout << "Reallocate" << endl;
    if(!mem) return NULL;

    MemoryHeader* MemHeader = (MemoryHeader*) ((char*)mem - sizeof(MemoryHeader));

    if(MemHeader->data.free) return NULL;

    size_t aligned_size = sizeof(MemoryHeader) + sizeof(align) * ROUNDALIGN(new_size, sizeof(align)); //if new size is smaller

    if(aligned_size <= MemHeader->data.sz)
	{
        size_t memleft = MemHeader->data.sz - aligned_size;
        MemHeader->data.sz = aligned_size;

        if(memleft >= sizeof(MemoryHeader) + sizeof(align)) //Split block  
		{  
            MemoryHeader* newBlock = new ((char*)MemHeader + MemHeader->data.sz) MemoryHeader();

            newBlock->data.sz = memleft;
            newBlock->data.prev = MemHeader;
            newBlock->data.next = MemHeader->data.next;
            newBlock->data.free = true;

            MemHeader->data.next->data.prev = newBlock;            
            MemHeader->data.next = newBlock;

            freeBlocks[memleft].push_back((void*)newBlock);  
            
            if(newBlock->data.next &&
				newBlock->data.next->data.free) // if next block is free, merge with current unused memory
			{
                _mergeWithNext(newBlock);
            }      
        }

        return (void*)((char*)MemHeader + sizeof(MemoryHeader));        
    }

    if(MemHeader->data.next && MemHeader->data.next->data.free &&
		MemHeader->data.sz + MemHeader->data.next->data.sz >= aligned_size)  //if next block is free and has enough memory to reallocate current
	{
        _mergeWithNext(MemHeader);

        size_t memleft = MemHeader->data.sz - aligned_size;

        MemHeader->data.sz = aligned_size;

        if(memleft >= sizeof(MemoryHeader) + sizeof(align)) //Split block 
		{
            MemoryHeader* newBlock = new ((char*)MemHeader + MemHeader->data.sz) MemoryHeader();

            newBlock->data.sz = memleft;
            newBlock->data.prev = MemHeader;
            newBlock->data.next = MemHeader->data.next;
            newBlock->data.free = true;

            MemHeader->data.next->data.prev = newBlock;
            MemHeader->data.next = newBlock;

            freeBlocks[memleft].push_back((void*)newBlock);        
        }

        return (void*)((char*)MemHeader + sizeof(MemoryHeader));
    }

    void* new_mem = allocate(new_size);

    if(new_mem)
	{
        memcpy(new_mem, mem, std::min(new_size, MemHeader->data.sz)); 
        free(mem);
        return new_mem;
    }

    return NULL;

}

void Allocator::free(void* mem)
{
	cout << "Deallocate" << endl;
    if(!mem) return;

    MemoryHeader* MemHeader = (MemoryHeader*) ((char*)mem - sizeof(MemoryHeader));

    MemHeader->data.free = true;

    freeBlocks[MemHeader->data.sz].push_back(MemHeader);

    if(MemHeader->data.prev && MemHeader->data.prev->data.free)
	{
        MemHeader = MemHeader->data.prev;        
        _mergeWithNext(MemHeader);
    }

    if(MemHeader->data.next && MemHeader->data.next->data.free)
	{
        _mergeWithNext(MemHeader);
    }
}

void Allocator::_mergeWithNext(MemoryHeader* block)
{
    MemoryHeader* nxt = block->data.next;

    for(auto it = freeBlocks[nxt->data.sz].begin(); it!=freeBlocks[nxt->data.sz].end(); ++it)
	{
        if(*it == (void*)nxt)
		{
            freeBlocks[nxt->data.sz].erase(it);

            if(freeBlocks[nxt->data.sz].empty())
			{
                freeBlocks.erase(nxt->data.sz);
            }
            break;
        }
    }

    for(auto it = freeBlocks[block->data.sz].begin(); it!=freeBlocks[block->data.sz].end(); ++it)
	{
        if((void*)block == (void*)(*it))
		{
            freeBlocks[block->data.sz].erase(it);

            if(freeBlocks[block->data.sz].empty())
			{
                freeBlocks.erase(block->data.sz);
            }
            break;
        }
    }
    block->data.sz += nxt->data.sz;
    block->data.next = nxt->data.next;

    if(nxt->data.next)
	{
        nxt->data.next->data.prev = block;
    }

    freeBlocks[block->data.sz].push_back(block);    
}
void Allocator::mem_dump()
{
    MemoryHeader* MemHeader = (MemoryHeader*) mempool;

	cout << "\nMemory info: " << endl;
    while(MemHeader != NULL)
	{
        cout << "Addr: " << MemHeader << ", Size: " << MemHeader->data.sz;
		cout << ", Free: " << (MemHeader->data.free ? "1 (true)" : "0 (false)") << endl;
        MemHeader = MemHeader->data.next;
    }
	cout << endl;
}