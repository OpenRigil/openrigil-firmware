/* Big Number lib */
#include "bn.h"

/*
 * Big Number Compare
 * 
 * block: should <= 128
 * endianness: little endian for both word and bn
 * return 1 if a > b, -1 if a < b, 0 if a == b
 */
int bn_cmp(uint32_t block, const uint32_t *a, const uint32_t *b) {
    uint64_t res1 = 0;
    uint64_t res2 = 0;
    for (int i = (block / 2) - 1; i >= 0; --i) {
      // compare by uint64_t since it may happen that block == 64 or 128
      res1 = (res1 << 1) | (((const uint64_t *)a)[i] > ((const uint64_t *)b)[i]);
      res2 = (res2 << 1) | (((const uint64_t *)a)[i] < ((const uint64_t *)b)[i]);
    }
    if (res1 > res2) {
        // if a > b, return positive value
        return 1;
    } else if (res1 < res2) {
        return -1;
    } else {
        return 0;
    }
    return 0;
}

/*
 * Big Number Add
 *
 * constant time
 * could be done in-place
 * return carry
 */
uint32_t bn_add(uint32_t block, uint32_t *ab, const uint32_t *a, const uint32_t *b) {
    uint64_t carry = 0;
    for (uint32_t i = 0; i != block; ++i) {
        carry += ((uint64_t)a[i]) + b[i];
        ab[i] = carry & 0xFFFFFFFF;
        carry >>= 32;
    }
    return carry;
}

/*
 * Big Number Sub
 *
 * constant time
 * could be done in-place
 * return 0 if borrowed 1 from higher bits
 * return 1 if did not used the 1 from higher bits
 */
uint32_t bn_sub(uint32_t block, uint32_t *ab, const uint32_t *a, const uint32_t *b) {
    uint64_t carry = 1;
    for (uint32_t i = 0; i != block; ++i) {
      carry += (uint64_t)0xFFFFFFFF + a[i] - b[i];
      ab[i] = carry & 0xFFFFFFFF;
      carry >>= 32;
    }
	return carry;
}

/*
 * Big Number Sub
 *
 * constant time
 * could be done in-place
 * return 0 if borrowed 1 from higher bits
 * return 1 if did not used the 1 from higher bits
 */
int is_bn_zero(uint32_t block, const uint32_t *bn) {
    for (uint32_t i = 0; i != block; ++i) {
        if (bn[i] != 0) {
            return 0;
        }
    }
    return 1;
}

/*
 * Big Number right shift by 1
 *
 * constant time
 * done in-place
 */
void bn_shift_right_by_1(uint32_t block, uint32_t *bn) {
    for (uint32_t i = 0; i <= block - 2; ++i) {
        bn[i] = ((bn[i + 1] & 1) << 31) | (bn[i] >> 1);
    }
    bn[block - 1] = bn[block - 1] >> 1;
}
