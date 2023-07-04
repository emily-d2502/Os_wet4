#include <cstring>
#include "malloc_2.h"
using namespace std;

struct MallocMetadata;
MallocMetadata *sector_list;

struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
};

// Searches for a free block with at least ‘size’ bytes or allocates
// (sbrk()) one if none are found
void* smalloc(size_t size) {

    if (size == 0 || size > 1e8) {
        return nullptr;
    }

    /* Find a free block that's big enough */
    MallocMetadata *sector = sector_list;
    while (sector) {
        if (sector->is_free && (sector->size >= size)) {
            sector->is_free = false;
            return (void *)(sector + 1);
        }
    }

    /* No block big enough was free, allocate new block */
    MallocMetadata* meta_data_place
        = (MallocMetadata *) sbrk(sizeof(MallocMetadata));
    if (meta_data_place == (void *)(-1)) {
        return nullptr;
    }

    void *ret = sbrk(size);
    if (ret == (void *)(-1)) {
        sbrk( -sizeof(MallocMetadata));
        return nullptr;
    }

    meta_data_place->size = size;
    meta_data_place->is_free = false;
    meta_data_place->next = nullptr;
    return ret;
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

    size_t ret = 0;
    MallocMetadata *tmp = sector_list;
    while (tmp) {
        if (tmp->is_free) {
            ++ret;
        }
        tmp = tmp->next;
    }
    return ret;
}

// Returns the number of bytes in all allocated blocks in the heap that are currently free,
// excluding the bytes used by the meta-data structs
size_t _num_free_bytes() {

    size_t ret = 0;
    MallocMetadata *tmp = sector_list;
    while (tmp) {
        if (tmp->is_free) {
            ret += tmp->size;
        }
        tmp = tmp->next;
    }
    return ret;
}

// Returns the overall (free and used) number of allocated blocks in the heap.
size_t _num_allocated_blocks() {

    size_t ret = 0;
    MallocMetadata *tmp = sector_list;
    while (tmp) {
        ++tmp;
        tmp = tmp->next;
    }
    return ret;
}

// Returns the overall number (free and used) of allocated bytes in the heap, excluding
// the bytes used by the meta-data structs.
size_t _num_allocated_bytes() {

    size_t ret = 0;
    MallocMetadata *tmp = sector_list;
    while (tmp) {
        ret += tmp->size;
        tmp = tmp->next;
    }
    return ret;
}

// Returns the overall number of meta-data bytes currently in the heap.
size_t _num_meta_data_bytes() {
    return _num_allocated_blocks() * sizeof(MallocMetadata);
}

// Returns the number of bytes of a single meta-data structure in your system.
size_t _size_meta_data() {
    return sizeof(MallocMetadata);
}




