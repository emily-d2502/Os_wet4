#include <iostream>
#include <cstring>
#include <unistd.h>
#include <stdint.h>
using namespace std;

/* Macros */
#define MAX_ORDER (10)
#define MEM_BLK_COUNT (32)
#define MEM_BLK_SIZE(order) (128 << (order))

#define META_DATA_SIZE() (sizeof(MallocMetadata))

/* Structs declarations */
struct MallocMetadata;
typedef MallocMetadata *list;

/* Global variables */
list free_blks[MAX_ORDER + 1];

struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
};

// Searches for a free block with at least ‘size’ bytes or allocates
// (sbrk()) one if none are found
void* smalloc(size_t size) {
    if (size == 0 || size > 1e8) {
        return nullptr;
    }

    static int called = _allocate_first_32_blks();


}

int _allocate_first_32_blks() {
    intptr_t addr = (intptr_t) sbrk(0);
    int max_size = MEM_BLK_COUNT * MEM_BLK_SIZE(MAX_ORDER);
    int diff = max_size - (addr % max_size);
    addr = (intptr_t) sbrk(max_size + diff) + diff;

    MallocMetadata *node;
    MallocMetadata *prev = nullptr;
    for (int i = 0; i < MEM_BLK_COUNT; ++i) {
        node = (MallocMetadata *) (addr + i * MEM_BLK_SIZE(MAX_ORDER));

        node->is_free = true;
        node->size = MEM_BLK_SIZE(MAX_ORDER);
        node->prev = prev;

        if (prev) {
            prev->next = node;
        }
        prev = node;
    }
    node->next = nullptr;
    free_blks[MAX_ORDER] = (MallocMetadata *) addr;
    return 1;
}

int _mem_blk_size(int order) {
    return (128 << (order));
}

// Releases the usage of the block that starts with the pointer ‘p’.
void sfree(void* p) {
    if (!p) {
        return;
    }

    MallocMetadata *m_data
        = (MallocMetadata *)((intptr_t)p - sizeof(MallocMetadata));
    m_data->is_free = true;
}

// Searches for a free block of at least ‘num’ elements, each ‘size’ bytes
// that are all set to 0 or allocates if none are found. In other words,
// find/allocate size * num bytes and set all bytes to 0.
void* scalloc(size_t num, size_t size) {
    size_t req_size = num * size;
    void *mem = smalloc(req_size);
    return memset(mem, 0, req_size);
}

// If ‘size’ is smaller than or equal to the current block’s size, reuses
// the same block. Otherwise, finds/allocates ‘size’ bytes for a new space,
// copies content of oldp into the new allocated space and frees the oldp.
void* srealloc(void* oldp, size_t size) {
    if (!oldp) {
        return smalloc(size);
    }

    if (size == 0 || size > 1e8) {
        return nullptr;
    }
    MallocMetadata *m_data
        = (MallocMetadata *)((intptr_t)oldp - sizeof(MallocMetadata));
    if (size <= m_data->size) {
        return oldp;
    }

    void *newp = smalloc(size);
    memmove(newp, oldp, m_data->size);
    sfree(oldp);
    return newp;
}

// Returns the number of allocated blocks in the heap that are currently free.
size_t _num_free_blocks() {

}

// Returns the number of bytes in all allocated blocks in the heap that are currently free,
// excluding the bytes used by the meta-data structs
size_t _num_free_bytes() {

}

// Returns the overall (free and used) number of allocated blocks in the heap.
size_t _num_allocated_blocks() {

}

// Returns the overall number (free and used) of allocated bytes in the heap, excluding
// the bytes used by the meta-data structs.
size_t _num_allocated_bytes() {

}

// Returns the overall number of meta-data bytes currently in the heap.
size_t _num_meta_data_bytes() {

}

// Returns the number of bytes of a single meta-data structure in your system.
size_t _size_meta_data() {

}

