#include <iostream>
#include <stdint.h>
#include <unistd.h>
// #include "malloc_2.h"
using namespace std;

struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
};

// void  sfree(void* p);
// void* smalloc(size_t size);
// void* scalloc(size_t num, size_t size);

int main() {
    /*
    intptr_t initial_program_break = (intptr_t) sbrk(0);
    printf("initial program break = %lu\n", initial_program_break);

    int n = 10;
    int *arr = (int *) smalloc(n * sizeof(int));
    for (int i = 0; i < n; ++i) {
        if (arr[i]) {
            printf("ERROR\n");
        }
    }

    intptr_t final_program_break = (intptr_t) sbrk(0);
    printf("final program break = %lu\n", final_program_break);
    printf("total bytes alloc and not freed = %lu\n",
                                final_program_break - initial_program_break);
     */

    printf("%lu\n", sizeof(MallocMetadata));
    return 0;
}