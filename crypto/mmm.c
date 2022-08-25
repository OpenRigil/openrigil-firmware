#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mmio.h"

#include "bn.h"
#include "mmm.h"

#ifndef MMM_BLOCK
#error "Must define MMM_BLOCK, consult your config"
#endif

#define MMM_STATUS  0x2000
#define MMM_CONTROL 0x2004
#define MMM_PPRIME  0x2008
#define MMM_P       0x200C
#define MMM_A       0x2010
#define MMM_B       0x2014
#define MMM_IW      0x2018
#define MMM_OUT     0x201C

// CAUTION: this whole lib not constant-time

void print_num(const char* name, int block, const uint32_t *bn) {
    printf("%s:", name);
    for(int i = 0; i != block; ++i) {
        printf("%08lX ", bn[block - 1 - i]);
    }
    printf("\n");
}
void print_arg(int block, const uint32_t *ab, const uint32_t *p, const uint32_t *a, const uint32_t *b) {
    print_num("p", block, p);
    print_num("a", block, a);
    print_num("b", block, b);
    print_num("ab", block, ab);
    printf("\n");
}

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
 * mmm could be done in-place (i.e. ab == a or ab == b or ab == p)
 */
int mm_mul(uint32_t block, uint32_t *ab, const uint32_t *p, const uint32_t *a, const uint32_t *b) {
    // num in bits but -1
    uint16_t input_width = block * 32 - 1;

    if (block > MMM_BLOCK) {
        return -1;
    }

    reg_write16(MMM_IW, input_width);
    reg_write8(MMM_PPRIME, 1);
    reg_write32(MMM_P, p[0]);
    reg_write32(MMM_A, a[0]);
    reg_write32(MMM_B, b[0]);
    // send ready signal
    reg_write8(MMM_CONTROL, 2);
    for(uint32_t i = 1; i < block; i++) {
        reg_write32(MMM_P, p[i]);
        reg_write32(MMM_A, a[i]);
        reg_write32(MMM_B, b[i]);
    }
    
    // wait for peripheral to complete
    while ((reg_read8(MMM_STATUS) & 0x1) == 0) ;

    for(uint32_t i = 0; i < block; i++) {
        ab[i] = reg_read32(MMM_OUT);
    }

    // clear peripheral
    reg_write8(MMM_CONTROL, 0);
    //printf("mul\n");
    //print_arg(block, ab, p, a, b);
    return 0;
}

// same as above except ab, p, a, b in big endian
// i.e. a[block-1] >> 24 is the least significant byte
int mm_mul_be(uint32_t block, uint32_t *ab, const uint32_t *p, const uint32_t *a, const uint32_t *b) {
    // num in bits but -1
    uint16_t input_width = block * 32 - 1;

    if (block > MMM_BLOCK) {
        return -1;
    }

    reg_write16(MMM_IW, input_width);
    reg_write8(MMM_PPRIME, 1);
    reg_write32(MMM_P, BSWAP32(p[block - 1]));
    reg_write32(MMM_A, BSWAP32(a[block - 1]));
    reg_write32(MMM_B, BSWAP32(b[block - 1]));
    // send ready signal
    reg_write8(MMM_CONTROL, 2);
    for(uint32_t i = 1; i < block; i++) {
        reg_write32(MMM_P, BSWAP32(p[block - 1 - i]));
        reg_write32(MMM_A, BSWAP32(a[block - 1 - i]));
        reg_write32(MMM_B, BSWAP32(b[block - 1 - i]));
    }

    // wait for peripheral to complete
    while ((reg_read8(MMM_STATUS) & 0x1) == 0) ;

    for(uint32_t i = 0; i < block; i++) {
        // can not directly BSWAP(reg_read32)
        // see the defintion of BSWAP32
        uint32_t out = reg_read32(MMM_OUT);
        ab[block - 1 - i] = BSWAP32(out);
    }

    // clear peripheral
    reg_write8(MMM_CONTROL, 0);
    //printf("mul\n");
    //print_arg(block, ab, p, a, b);
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
void mm_add(uint32_t block, uint32_t *ab, const uint32_t *p, const uint32_t *a, const uint32_t *b) {
    uint32_t carry = bn_add(block, ab, a, b);
    // now ab/carry is a + b

    // compare ab and p
    int cmp = bn_cmp(block, ab, p);
    if (carry != 0 || cmp >= 0) {
        // p <= ab < 2p
        bn_sub(block, ab, ab, p);
    }
    //printf("add\n");
    //print_arg(block, ab, p, a, b);
    // FIXME: not constant time
}

void mm_add_be(uint32_t block, uint32_t *ab, const uint32_t *p, const uint32_t *a, const uint32_t *b) {
    uint32_t carry = bn_add_be(block, ab, a, b);
    // now ab/carry is a + b

    // compare ab and p
    int cmp = bn_cmp_be(block, ab, p);
    if (carry != 0 || cmp >= 0) {
        // p <= ab < 2p
        bn_sub_be(block, ab, ab, p);
    }
    //printf("add be\n");
    //print_arg(block, ab, p, a, b);
    // FIXME: not constant time
}

/**
 * Montgomery Modular Subsctration
 *
 * block: measured in num of uint32_t (e.g. for 256 bit mmm it is 8)
 * a, b, p: stored in little endian manner (e.g. p[0] for least significant 32 bit)
 *   note that RISC-V machine we use is little endian
 *   so p[0] & 0xFF is the least significant byte 
 * ab: result of a - b % p
 * user should ensure a, b < p
 * so that a + b < 2p
 * could be done in-place for ab == b
 * must ensure that ab != a
 */
void mm_sub(uint32_t block, uint32_t *ab, const uint32_t *p, const uint32_t *a, const uint32_t *b) {
    /* used ab as tmp buf */
    bn_sub(block, ab, p, b); // tmp = p - b
    mm_add(block, ab, p, a, ab); // ab = (a + tmp) % p = (a + p - b) % p = (a - b) & p 
    //printf("sub\n");
    //print_arg(block, ab, p, a, b);
}

void mm_sub_be(uint32_t block, uint32_t *ab, const uint32_t *p, const uint32_t *a, const uint32_t *b) {
    /* used ab as tmp buf */
    bn_sub_be(block, ab, p, b); // tmp = p - b
    mm_add_be(block, ab, p, a, ab); // ab = (a + tmp) % p = (a + p - b) % p = (a - b) & p
    //printf("sub be\n");
    //print_arg(block, ab, p, a, b);
}
