#ifndef MMM_H
#define MMM_H

#include <stdint.h>

int mm_mul(uint32_t block, uint32_t *ab, const uint32_t *p, const uint32_t *a, const uint32_t *b);
void mm_add(uint32_t block, uint32_t *ab, const uint32_t *p, const uint32_t *a, const uint32_t *b);
void mm_sub(uint32_t block, uint32_t *ab, const uint32_t *p, const uint32_t *a, const uint32_t *b);

#endif // MMM_H
