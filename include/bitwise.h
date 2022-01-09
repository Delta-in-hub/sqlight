#if !defined(__SQLIGHT_BITWISE__)
#define __SQLIGHT_BITWISE__
#include "fmt/format.h"
#include <cassert>
#include <cstdint>
#include <cstring>

/**
 * @brief  print anything in hex format to stdout.
 * @param arr pointer to array
 * @param len length in bytes
 */
void printHex(const void *arr, size_t len = 1);

/**
 * @brief return integer ceil(a/b)
 */
constexpr unsigned ceil(const unsigned a, const unsigned b)
{
    return (a + b - 1) / b;
}

/**
 * @brief 1 Bytes == 8 bits
 */
constexpr unsigned BYTEINBITS = 8;

constexpr uint8_t MSB = 0b10000000; // most significant bit

/**
 * @brief Variable-length bits array.
 * There are a variety of ways to keep track of free record slots on a given page. One efficient method is to use a
 * bitmap: If each data page can hold n records, then you can store an n-bit bitmap in the page header indicating which
 * slots currently contain valid records and which slots are available.
 */
class BitMap
{
  private:
    uint8_t *_data;
    const unsigned _sizeInBytes;
    int _whichByte(int pos) const
    {
        return pos / BYTEINBITS;
    }
    uint8_t _getMask(int pos) const
    {
        return MSB >> (pos % BYTEINBITS);
    }

  public:
    /**
     * @brief Construct a new Bit Map object
     *
     * @param data pointer to data
     * @param size length in bytes
     */
    BitMap(void *data, unsigned size) : _data(static_cast<uint8_t *>(data)), _sizeInBytes(size)
    {
        ;
    }
    /**
     * @brief get a certain bit.
     * @param pos  index from 0
     * @return true for 1b
     * @return false for 0b
     */
    bool get(int pos) const
    {
        assert(pos >= 0 and pos < _sizeInBytes * BYTEINBITS);
        return (_data[_whichByte(pos)] & _getMask(pos)) != 0;
    }

    void set(int pos)
    {
        assert(pos >= 0 and pos < _sizeInBytes * BYTEINBITS);
        _data[_whichByte(pos)] |= _getMask(pos);
    }

    void reset(int pos)
    {
        assert(pos >= 0 and pos < _sizeInBytes * BYTEINBITS);
        _data[_whichByte(pos)] &= (~_getMask(pos));
    }

    void resetAll()
    {
        memset(_data, 0, _sizeInBytes);
    }

    void setAll()
    {
        memset(_data, 0xff, _sizeInBytes);
    }

    /**
     * @brief Get the Length of bitmap in bits
     * @return bits length
     */
    unsigned getLength() const
    {
        return _sizeInBytes * BYTEINBITS;
    }
};

#endif // __SQLIGHT_BITWISE__
