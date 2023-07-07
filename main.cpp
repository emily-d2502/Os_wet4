#include "malloc_3.cpp"

#include <iostream>
using namespace std;

void list::print() const {
    if (empty()) {
        printf("EMPTY\n");
        return;
    }
    MallocMetadata *blk = back;
    while (blk) {
        printf("%lu -> ", (intptr_t) blk);
        blk = blk->next;
    }
    printf("\n");
}

int main() {
    int i = 4;
    printf("i = %d\n", i);
    printf("i >> 1 = %d\n", i >> 1);
    printf("i << 1 = %d\n\n", i << 1);

    int order = 1;
    printf("size of meta data = %lu\n", sizeof(MallocMetadata));
    printf("size of blk of order = %d is %lu bytes\n", order, _mem_blk_size(order));
    printf("usable size of blk of order = %d is %lu bytes\n", order, _mem_blk_usable_size(order));

    printf("\nPrinting lists:\n");
    for (int i = 0; i <= MAX_ORDER; ++i) {
        printf("list #%d ", i);
        free_blks[i].print();
    }

    char *bytes = (char *) smalloc(_mem_blk_usable_size(9) - 1);

    printf("\nPrinting lists:\n");
    for (int i = 0; i <= MAX_ORDER; ++i) {
        printf("list #%d ", i);
        free_blks[i].print();
    }
    printf("addr = %lu\n", (intptr_t) bytes);
    printf("addr = %lu\n", (intptr_t)_find_buddy(_mem_blk_meta_data(bytes)));
    printf("addr = %lu\n", (intptr_t)_find_buddy(_find_buddy(_mem_blk_meta_data(bytes))));

    sfree(bytes);
    printf("\nPrinting lists:\n");
    for (int i = 0; i <= MAX_ORDER; ++i) {
        printf("list #%d ", i);
        free_blks[i].print();
    }
    return 0;
}