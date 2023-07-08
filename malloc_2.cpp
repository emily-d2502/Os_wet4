#include <unistd.h>
#include <stdint.h>
#include <cstring>

#define MAX_ALLOCATION_SIZE (1e8)

struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata *prev;
};

class list {
public:
    list();
    bool empty() const;
    void print() const;
    void push_front(MallocMetadata *blk);
    MallocMetadata *find_blk(size_t size);

    size_t _num_free_bytes() const;
    size_t _num_free_blocks() const;
    size_t _num_allocated_bytes() const;
    size_t _num_allocated_blocks() const;
private:
    MallocMetadata *back;
    MallocMetadata *front;
};

MallocMetadata *_mem_blk_meta_data(void *ptr) {
    return (MallocMetadata *)((intptr_t)ptr - sizeof(MallocMetadata));
}

/* Global variable to store alocated blocks */
list allocated_blks;

// Searches for a free block with at least ‘size’ bytes or allocates
// (sbrk()) one if none are found
void* smalloc(size_t size) {
    if (size == 0 || size > MAX_ALLOCATION_SIZE) {
        return nullptr;
    }

    /* Find a free block that's big enough */
    MallocMetadata *blk = allocated_blks.find_blk(size);
    if (blk) {
        return (void *)(blk + 1);
    }

    /* No block big enough was free, allocate new block */
    void *ret = sbrk(size + sizeof(MallocMetadata));
    if (ret == (void *)(-1)) {
        return nullptr;
    }
    blk = (MallocMetadata *) ret;
    blk->size = size;
    blk->is_free = false;
    allocated_blks.push_front(blk);
    return (void *)(blk + 1);
}

// Releases the usage of the block that starts with the pointer ‘ptr’.
void sfree(void* ptr) {
    if (!ptr) {
        return;
    }

    MallocMetadata *blk = _mem_blk_meta_data(ptr);
    blk->is_free = true;
}

// Searches for a free block of at least ‘num’ elements, each ‘size’ bytes
// that are all set to 0 or allocates if none are found. In other words,
// find/allocate size * num bytes and set all bytes to 0.
void* scalloc(size_t num, size_t size) {
    size_t req_size = num * size;
    void *mem = smalloc(req_size);
    if (!mem) {
        return mem;
    }
    return memset(mem, 0, req_size);
}

// If ‘size’ is smaller than or equal to the current block’s size, reuses
// the same block. Otherwise, finds/allocates ‘size’ bytes for a new space,
// copies content of oldp into the new allocated space and frees the oldp.
void* srealloc(void* oldp, size_t size) {
    if (!oldp) {
        return smalloc(size);
    }
    if (size == 0 || size > MAX_ALLOCATION_SIZE) {
        return nullptr;
    }

    MallocMetadata *blk = _mem_blk_meta_data(oldp);
    if (size <= blk->size) {
        return oldp;
    }

    void *newp = smalloc(size);
    memmove(newp, oldp, blk->size);
    sfree(oldp);
    return newp;
}

// Returns the number of allocated blocks in the heap that are currently free.
size_t list::_num_free_blocks() const {
    size_t ret = 0;
    MallocMetadata *tmp = back;
    while (tmp) {
        if (tmp->is_free) {
            ++ret;
        }
        tmp = tmp->next;
    }
    return ret;
}

size_t _num_free_blocks() {
    return allocated_blks._num_free_blocks();
}

// Returns the number of bytes in all allocated blocks in the heap that are currently free,
// excluding the bytes used by the meta-data structs
size_t list::_num_free_bytes() const {
    size_t ret = 0;
    MallocMetadata *tmp = back;
    while (tmp) {
        if (tmp->is_free) {
            ret += tmp->size;
        }
        tmp = tmp->next;
    }
    return ret;
}

size_t _num_free_bytes() {
    return allocated_blks._num_free_bytes();
}

// Returns the overall (free and used) number of allocated blocks in the heap.
size_t list::_num_allocated_blocks() const {
    size_t ret = 0;
    MallocMetadata *tmp = back;
    while (tmp) {
        ++ret;
        tmp = tmp->next;
    }
    return ret;
}

size_t _num_allocated_blocks() {
    return allocated_blks._num_allocated_blocks();
}

// Returns the overall number (free and used) of allocated bytes in the heap, excluding
// the bytes used by the meta-data structs.
size_t list::_num_allocated_bytes() const {
    size_t ret = 0;
    MallocMetadata *tmp = back;
    while (tmp) {
        ret += tmp->size;
        tmp = tmp->next;
    }
    return ret;
}

size_t _num_allocated_bytes() {
    return allocated_blks._num_allocated_bytes();
}

// Returns the overall number of meta-data bytes currently in the heap.
size_t _num_meta_data_bytes() {
    return _num_allocated_blocks() * sizeof(MallocMetadata);
}

// Returns the number of bytes of a single meta-data structure in your system.
size_t _size_meta_data() {
    return sizeof(MallocMetadata);
}

/* Our data structure and methods for it (implementation) */
list::list():
    back(nullptr),
    front(nullptr) {}

bool list::empty() const {
    return ((front == nullptr) && (back == nullptr));
}

void list::push_front(MallocMetadata *blk) {
    if (empty()) {
        back = blk;
        front = blk;
        return;
    }
    front->next = blk;
    blk->prev = front;
    blk->next = nullptr;
    front = blk;
}

MallocMetadata *list::find_blk(size_t size) {
    MallocMetadata *blk = back;
    while (blk) {
        if (blk->is_free && (blk->size >= size)) {
            blk->is_free = false;
            return blk;
        }
        blk = blk->next;
    }
    return nullptr;
}

