#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mmio.h"

#define MMM_STATUS  0x2000
#define MMM_CONTROL 0x2004
#define MMM_PPRIME  0x2008
#define MMM_P       0x200C
#define MMM_A       0x2010
#define MMM_B       0x2014
#define MMM_IW      0x2018
#define MMM_IW2     0x201C
#define MMM_OUT     0x2020

// CAUTION: this whole lib not constant-time

/**
 * Montgomery Modular Multiplication
 *
 * block: measured in num of uint32_t (e.g. for 256 bit mmm it is 8)
 * a, b, p: stored in little endian manner (e.g. p[0] for least significant 32 bit)
 *   note that RISC-V machine we use is little endian
 *   so p[0] & 0xFF is the least significant byte 
 * ab: result of a * b * Rinv % p where R = 2 ^ (block * 32)
 * user should ensure a, b < p
 * hardware ensure that ab < p
 * hardware is not constant time
 * mmm could be done in-place (i.e. ab == a or ab == b)
 */
int mm_mul(uint32_t block, uint32_t *ab, uint32_t *p, uint32_t *a, uint32_t *b) {
  uint32_t result;
  uint32_t input_width1 = block * 2 - 1;
  uint32_t input_width2 = 15;

  // only accept 8 for now
  if (block != 8) {
    return -1;
  }

  volatile int *m = MMM_OUT;
  reg_write32(MMM_IW, input_width1);
  reg_write32(MMM_IW2, input_width2);
  reg_write8(MMM_PPRIME, 1);
  reg_write32(MMM_P, p[0]);
  reg_write32(MMM_A, a[0]);
  reg_write32(MMM_B, b[0]);
  // send ready signal
  reg_write8(MMM_CONTROL, 2);
  for(uint32_t i = 1; i < block; i++) 
  {
    reg_write32(MMM_P, p[i]);
    reg_write32(MMM_A, a[i]);
    reg_write32(MMM_B, b[i]);
  }

  // wait for peripheral to complete
  while ((reg_read8(MMM_STATUS) & 0x1) == 0) ;
  for(uint32_t i = 0; i < block; i++)
  {
    ab[i] = reg_read32(m);
  }
  reg_write8(MMM_CONTROL, 0);

  return 0;
}

/**
 * Montgomery Modular Addition
 *
 * block: measured in num of uint32_t (e.g. for 256 bit mmm it is 8)
 * a, b, p: stored in little endian manner (e.g. p[0] for least significant 32 bit)
 *   note that RISC-V machine we use is little endian
 *   so p[0] & 0xFF is the least significant byte 
 * ab: result of a + b % p
 * user should ensure a, b < p
 * so that a + b < 2p
 * mma could be done in-place (i.e. ab == a or ab == b)
 */
int mm_add(uint32_t block, uint32_t *ab, uint32_t *p, uint32_t *a, uint32_t *b) {
    uint32_t carry = bn_add(block, ab, a, b);
    // now ab/carry is a + b

    // compare ab and p
    int cmp = bn_cmp(ab, p);
    if (carry != 0 || cmp >= 0) {
        // p < ab < 2p
		bn_sub(block, ab, ab, p);
    }
	// FIXME: not constant time
}

/* Big Number lib */

/*
 * Big Number Compare
 *
 * block: should <= 128
 * endianness: same as above
 * return 1 if a > b, -1 if a < b, 0 if a == b
 */
int bn_cmp(uint32_t block, uint32_t *a, uint32_t *b) {
    uint64_t res1 = 0;
    uint64_t res2 = 0;
    for (i = (blocks / 2) - 1; i >= 0; --i) {
      // compare by uint64_t since it may happen that block == 64 or 128
      res1 = (res1 << 1) | (((uint64_t *)a)[i] > ((uint64_t *)b)[i]);
      res2 = (res2 << 1) | (((uint64_t *)a)[i] < ((uint64_t *)b)[i]);
    }
    if (res1 > res2) {
        // if a > b, return positive value
        return 1;
    } else if (res1 < res2) {
        return -1;
    } else {
        return 0;
    }
}

/*
 * Big Number Add
 *
 * constant time
 * could be done in-place
 * return carry
 */
uint32_t bn_add(uint32_t block, uint32_t *ab, uint32_t *a, uint32_t *b) {
    uint32_t carry = 0;
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
int bn_sub(uint32_t block, uint32_t *ab, uint32_t *a, uint32_t *b) {
    uint64_t carry = 1;
    for (i = 0; i != block; ++i) {
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
static int is_bn_zero(uint32_t block, uint32_t *bn) {
    for (int i = 0; i != block; ++i) {
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
static void bn_shift_right_by_1(uint32_t block, uint32_t *bn) {
    for (int i = 0; i <= block - 2; ++i) {
        bn[i] = ((bn[i + 1] & 1) << 31) | (bn[i] >> 1);
    }
    bn[block - 1] = bn[block - 1] >> 1;
}
