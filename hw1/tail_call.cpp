#include <iostream>
#include <cstdint>

int foo(int a, int b) {
    if (a == 0)
        return b;

    return foo(a - 1, b);
}

int main() {
    return foo(42, 42);
}
