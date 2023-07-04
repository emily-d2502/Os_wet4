#include <unistd.h>

/* ``````````` main functions ``````````` */

/*
 * size             - Number of bytes to allocate.
 * return value		- Success: returns pointer to the first byte in the allocated block.
 *                  - Failure:
 *                      - If size is 0 returns NULL
 *                      - If size is more than 1e8, return NULL.
 *                      - If sbrk fails in allocating the needed space, return NULL.
 */
void* smalloc(size_t size);

/*
 * num              - Number of elemnts to allocate and initialize.
 * size             - Size of each elemnt (bytes).
 * return value		- Success: returns pointer to the first byte in the allocated block.
 *                  - Failure:
 *                      - If size is 0 returns NULL
 *                      - If size is more than 1e8, return NULL.
 *                      - If sbrk fails in allocating the needed space, return NULL.
 */
void* scalloc(size_t num, size_t size);

/*
 * oldp             - Old pointer to re-allocate.
 * size             - Number of bytes to allocate.
 * return value		- Success: returns pointer to the first byte in the (newly) allocated block.
 *                  - Failure:
 *                      - If size is 0 returns NULL
 *                      - If size is more than 1e8, return NULL.
 *                      - If sbrk fails in allocating the needed space, return NULL.
 */
void* srealloc(void* oldp, size_t size);

/*
 * ptr              - Pointer to the first byte in the block we want to free.
 */
void sfree(void* ptr);



/*  ``````````` stats functions ``````````` */

/*
 * Returns the number of allocated blocks in the heap that are currently free.
 */
size_t _num_free_blocks();

/*
 * Returns the number of bytes in all allocated blocks in the heap that are currently free,
 * excluding the bytes used by the meta-data structs;
 */
size_t _num_free_bytes();

/*
 * Returns the overall (free and used) number of allocated blocks in the heap.
 */
size_t _num_allocated_blocks();

/*
 * Returns the overall number (free and used) of allocated bytes in the heap, excluding
 * the bytes used by the meta-data structs.
 */
size_t _num_allocated_bytes();

/*
 * Returns the overall number of meta-data bytes currently in the heap.
 */
size_t _num_meta_data_bytes();

/*
 * Returns the number of bytes of a single meta-data structure in your system.
 */
size_t _size_meta_data();