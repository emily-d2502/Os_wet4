#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <ctime>

/* Macros */
#define MAX_ORDER (10)
#define MEM_BLK_COUNT (32)

/* Typename declarations */
struct MallocMetadata {
    int32_t cookie;
    int order;
    size_t size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
};

size_t _mem_blk_size(int order) {
    return (128 << (order));
}
size_t _mem_blk_usable_size(int order) {
    return (128 << (order)) - sizeof(MallocMetadata);
}
MallocMetadata *_mem_blk_meta_data(void *ptr) {
    return (MallocMetadata *)((intptr_t)ptr - sizeof(MallocMetadata));
}

/* Security methods */
int32_t _cookie;
void _generate_cookie() {
    std::srand(std::time(nullptr));
    _cookie = rand();
}

MallocMetadata *_validate_cookie(MallocMetadata *blk) {
    if (!blk) {
        return nullptr;
    }
    if (blk->cookie != _cookie) {
        exit(0xdeadbeef);
    }
    return blk;
}
#define SECURE_ACCESS(ptr) _validate_cookie(ptr)

/* Our data structure and methods for it (interface) */
class list {
public:
    list();
    bool empty() const;
    void print() const;
    // MallocMetadata *pop_back();
    MallocMetadata *pop_front();
    void remove(MallocMetadata *blk);
    // void push_back(MallocMetadata *blk);
    void push_front(MallocMetadata *blk);
    size_t size;
private:

    MallocMetadata *back;
    MallocMetadata *front;
};

// Data structures to maintain free block of memory
list free_blks[MAX_ORDER + 1];
list mapped_blks;

// Functions to maintain our data structures
bool _allocate_first_32_blks();
void _split_blk(MallocMetadata *blk);
void *_allocate_large_mem(size_t size);
MallocMetadata *_find_blk(size_t size);
MallocMetadata *_find_buddy(MallocMetadata *blk);
MallocMetadata *_merge_buddy(MallocMetadata *blk);

/* Stats */
static size_t _num_alloc_blocks = 0;
static size_t _num_alloc_bytes = 0;

/* Implementation of malloc */
void *smalloc(size_t size) {
    static bool called = _allocate_first_32_blks();
    if (size == 0 && called) {
        return nullptr;
    }
    if (size > _mem_blk_usable_size(MAX_ORDER)) {
        return _allocate_large_mem(size);
    }

    MallocMetadata *blk = SECURE_ACCESS(_find_blk(size));
    if (!blk) {
        return nullptr;
    }
    while (_mem_blk_usable_size(blk->order - 1) >= size && blk->order >= 1) {
        _split_blk(blk);
    }

    blk->is_free = false;
    return (void *)(blk + 1);
}

void sfree(void *ptr) {
    if (!ptr) {
        return;
    }
    MallocMetadata *blk = SECURE_ACCESS(_mem_blk_meta_data(ptr));
    if (blk->size > _mem_blk_size(MAX_ORDER)) {
        --_num_alloc_blocks;
        _num_alloc_bytes -= blk->size;
        mapped_blks.remove(blk);
        munmap(ptr, blk->size);
        return;
    }
    if (blk->is_free) {
        return;
    }

    MallocMetadata *buddy = SECURE_ACCESS(_find_buddy(blk));
    blk->is_free = true;
    while (buddy->is_free && blk->order < MAX_ORDER) {
        free_blks[buddy->order].remove(buddy);
        blk = _merge_buddy(blk);
        buddy = _find_buddy(blk);
    }
    free_blks[blk->order].push_front(blk);
}

/* Our data structure and methods for it (implementation) */
list::list():
    size(0),
    back(nullptr),
    front(nullptr) {}

bool list::empty() const {
    return ((front == nullptr) && (back == nullptr));
}

MallocMetadata *list::pop_front() {
    if (empty()) {
        return nullptr;
    }
    MallocMetadata *ret = SECURE_ACCESS(front);
    front = SECURE_ACCESS(front->prev);
    --size;
    if (front) {
        front->next = nullptr;
    } else {
        back = nullptr;
    }
    ret->prev = nullptr;
    return ret;
}

void list::push_front(MallocMetadata *blk) {
    SECURE_ACCESS(blk);
    ++size;
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

void list::remove(MallocMetadata *blk) {
    SECURE_ACCESS(blk);
    --size;
    if (!blk->prev && !blk->next) {
        back = nullptr;
        front = nullptr;
        return;
    }
    if (!blk->prev) {
        back = SECURE_ACCESS(blk->next);
        (blk->next)->prev = nullptr;
        return;
    }
    if (!blk->next) {
        front = SECURE_ACCESS(blk->prev);
        (blk->prev)->next = nullptr;
        return;
    }
    (blk->prev)->next = SECURE_ACCESS(blk->next);
    (blk->next)->prev = SECURE_ACCESS(blk->prev);
}

MallocMetadata *_find_blk(size_t size) {
    for (int order = 0; order <= MAX_ORDER; ++order) {
        if (!free_blks[order].empty() && (_mem_blk_usable_size(order) >= size)) {
            return SECURE_ACCESS(free_blks[order].pop_front());
        }
    }
    return nullptr;
}

MallocMetadata *_find_buddy(MallocMetadata *blk) {
    return (MallocMetadata *)((intptr_t)SECURE_ACCESS(blk) xor _mem_blk_size(blk->order));
}

MallocMetadata *_merge_buddy(MallocMetadata *blk) {
    MallocMetadata *base = SECURE_ACCESS(_find_buddy(SECURE_ACCESS(blk)));
    if ((intptr_t)blk < (intptr_t)base) {
        base = blk;
    }
    ++base->order;
    blk->size = _mem_blk_size(blk->order);
    --_num_alloc_blocks;
    _num_alloc_bytes += sizeof(MallocMetadata);
    return base;
}

bool _allocate_first_32_blks() {
    _generate_cookie();
    _num_alloc_blocks = MEM_BLK_COUNT;
    _num_alloc_bytes = MEM_BLK_COUNT * _mem_blk_usable_size(MAX_ORDER);
    intptr_t addr = (intptr_t)sbrk(0);
    int max_size = MEM_BLK_COUNT * _mem_blk_size(MAX_ORDER);
    int diff = max_size - (addr % max_size);
    addr = (intptr_t)sbrk(max_size + diff) + diff;

    for (int i = 0; i < MEM_BLK_COUNT; ++i) {
        MallocMetadata *blk = (MallocMetadata *)(addr + i * _mem_blk_size(MAX_ORDER));

        blk->cookie = _cookie;
        blk->is_free = true;
        blk->order = MAX_ORDER;
        blk->size = _mem_blk_size(blk->order);

        free_blks[MAX_ORDER].push_front(blk);
    }
    return true;
}

void _split_blk(MallocMetadata *blk) {
    --blk->order;
    blk->size = _mem_blk_size(blk->order);
    MallocMetadata *buddy = _find_buddy(SECURE_ACCESS(blk));

    buddy->is_free = true;
    buddy->cookie = _cookie;
    buddy->order = blk->order;
    buddy->size = _mem_blk_size(buddy->order);

    free_blks[buddy->order].push_front(buddy);
    ++_num_alloc_blocks;
    _num_alloc_bytes -= sizeof(MallocMetadata);
}

void *_allocate_large_mem(size_t size) {

    void* ret = mmap(NULL, size + sizeof(MallocMetadata),
        PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ret == (void *)(-1)) {
        return nullptr;
    }

    ++_num_alloc_blocks;
    _num_alloc_bytes += size;
    MallocMetadata *blk = (MallocMetadata *)ret;

    blk->size = size;
    blk->is_free = false;
    blk->cookie = _cookie;

    mapped_blks.push_front(blk);
    return (void *)(blk + 1);
}

// Returns the number of allocated blocks in the heap that are currently free.
size_t _num_free_blocks() {
    size_t ret = 0;
    for (int i = 0; i <= MAX_ORDER; ++i) {
        ret += free_blks[i].size;
    }
    return ret;
}

// Returns the number of bytes in all allocated blocks in the heap that are currently free,
// excluding the bytes used by the meta-data structs
size_t _num_free_bytes() {
    size_t ret = 0;
    for (int i = 0; i <= MAX_ORDER; ++i) {
        ret += free_blks[i].size * _mem_blk_usable_size(i);
    }
    return ret;
}

// Returns the overall (free and used) number of allocated blocks in the heap.
size_t _num_allocated_blocks() {
    return _num_alloc_blocks;
}

// Returns the overall number (free and used) of allocated bytes in the heap, excluding
// the bytes used by the meta-data structs.
size_t _num_allocated_bytes() {
    return _num_alloc_bytes;
}

// Returns the overall number of meta-data bytes currently in the heap.
size_t _num_meta_data_bytes() {
    return _num_alloc_blocks * sizeof(MallocMetadata);
}

// Returns the number of bytes of a single meta-data structure in your system.
size_t _size_meta_data() {
    return sizeof(MallocMetadata);
}

// Searches for a free block of at least ‘num’ elements, each ‘size’ bytes
// that are all set to 0 or allocates if none are found. In other words,
// find/allocate size * num bytes and set all bytes to 0.
void* scalloc(size_t num, size_t size) {
    size_t req_size = num * size;
    void *mem = smalloc(req_size);
    if (!mem) {
        return nullptr;
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
    if (size == 0) {
        return nullptr;
    }

    MallocMetadata *blk = SECURE_ACCESS(_mem_blk_meta_data(oldp));
    if (size <= blk->size) {
        return oldp;
    }
    if (blk->size > _mem_blk_usable_size(MAX_ORDER)) {
        void *newp = smalloc(size);
        memmove(newp, oldp, (blk->size < size) ? blk->size : size);
        sfree(oldp);
        return newp;
    }

    bool merge = false;
    int order = blk->order;
    MallocMetadata *buddy = _find_buddy(blk);
    while (order < MAX_ORDER) {
        if (buddy->is_free && size <= _mem_blk_usable_size(order)) {
            merge = true;
            break;
        }
        buddy = (MallocMetadata *)((intptr_t)(blk) xor _mem_blk_size(++order));
    }
    if (merge) {
        buddy = _find_buddy(blk);
        while (size > _mem_blk_usable_size(blk->order) && buddy->is_free) {
            free_blks[buddy->order].remove(buddy);
            blk = _merge_buddy(blk);
            buddy = _find_buddy(blk);
        }
        return (void *)(blk + 1);
    }
    void *newp = smalloc(size);
    memmove(newp, oldp, (blk->size < size) ? blk->size : size);
    sfree(oldp);
    return newp;
}
