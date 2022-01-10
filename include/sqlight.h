#if !defined(__SQLIGHT__)
#define __SQLIGHT__
#include <ciso646>

#define SQLIGHT_VERSION "0.01"

constexpr unsigned BYTEINBITS = 8;

constexpr unsigned PAGESIZE = 4096; // A page is 4096 bytes.

constexpr unsigned CACHESIZE = 4096 * 2; // Cache could contains CACHESIZE pages for maximum.
#endif                                   // __SQLIGHT__
