#include <iostream>
#include <stdint.h>
#include <unistd.h>

using namespace std;

#define MAX_ORDER (10)
#define MEM_BLK(order) (128 << (order))

int *mem[MAX_ORDER];

int goo();

void foo() {
    static int called = goo();
}

int goo() {
    printf("Function entered\n");
    return 1;
}


int main() {

    foo();
    foo();
    foo();
    foo();
    return 0;
}