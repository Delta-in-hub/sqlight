#if !defined(__SQLIGHT_BITWISE__)
#define __SQLIGHT_BITWISE__
#include <cstdint>
#include "fmt/format.h"

/**
   * @brief  print anything in hex format to stdout.
   * @param arr pointer to array
   * @param len length in bytes
   */
void printHex(const void *arr, size_t len = 1);

#endif // __SQLIGHT_BITWISE__
