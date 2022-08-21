#ifndef BN_H
#define BN_H

#include <stdint.h>

int bn_cmp(uint32_t block, const uint32_t *a, const uint32_t *b);
uint32_t bn_add(uint32_t block, uint32_t *ab, const uint32_t *a, const uint32_t *b);
uint32_t bn_sub(uint32_t block, uint32_t *ab, const uint32_t *a, const uint32_t *b);
int is_bn_zero(uint32_t block, const uint32_t *bn);
void bn_shift_right_by_1(uint32_t block, uint32_t *bn);

#endif // BN_H
