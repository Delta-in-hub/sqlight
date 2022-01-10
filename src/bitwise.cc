#include "bitwise.h"

void printHex(const void *arr, size_t len)
{
    if (not arr or len == 0)
        return;
    const uint8_t *ptr = static_cast<const uint8_t *>(arr);
    const uint8_t *_end = ptr + len;
    while (ptr < _end)
        fmt::print("{:#04x} ", *ptr++);
    fmt::print("\n");
}
