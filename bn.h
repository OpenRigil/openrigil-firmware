#ifndef BN_H
#define BN_H

#include <stdint.h>

int bn_cmp(uint32_t block, const uint32_t *a, const uint32_t *b);
uint32_t bn_add(uint32_t block, uint32_t *ab, const uint32_t *a, const uint32_t *b);
uint32_t bn_sub(uint32_t block, uint32_t *ab, const uint32_t *a, const uint32_t *b);
int is_bn_zero(uint32_t block, const uint32_t *bn);
void bn_shift_right_by_1(uint32_t block, uint32_t *bn);

int bn_cmp_be(uint32_t block, const uint32_t *a, const uint32_t *b);
uint32_t bn_add_be(uint32_t block, uint32_t *ab, const uint32_t *a, const uint32_t *b);
uint32_t bn_sub_be(uint32_t block, uint32_t *ab, const uint32_t *a, const uint32_t *b);

#define BSWAP32(x) (((x >> 24) & 0xFF) | ((x << 8) & 0xFF0000) | ((x >> 8) & 0xFF00) | ((x << 24) & 0xFF000000))
#define BSWAP64(x) (__builtin_bswap64(x))

#endif // BN_H
