#include "bn.h"
#include "mmm.h"
// RSA4096 is implemented with MMM2048 using xRSA algorithm
// see https://dang.fan/publication/icpads21-xrsa/ for more detail

#include <string.h>

// from canokey-crypto
#include "rsa.h"

typedef union {
  uint32_t as_u32[128];
  uint8_t as_u8[512];
} BE_BN4096;

#define BE_BN4096_ZERO(x)    do { memset((x)->as_u8, 0x00, 512); } while(0)
#define BE_BN4096_ONE(x)    do { BE_BN4096_ZERO(x); (x)->as_u8[511] = 0x01; } while(0)
#define BE_BN4096_COPY(r, x)    do { memmove((r)->as_u8, (x)->as_u8, 512); } while(0)
#define BE_BN4096_ALL_ONE(x) do { memset((x)->as_u8, 0xFF, 512); } while(0)
#define BE_BN4096_SET_BIT(x, i) do { x->as_u8[511 - (i >> 3)] |= 1 << (i & 0x7); } while(0)

typedef union {
  uint32_t as_u32[64];
  uint8_t as_u8[256];
} BE_BN2048;

#define BLOCK_2048 64

#define BE_BN2048_ZERO(x)    do { memset((x)->as_u8, 0x00, 256); } while(0)
#define BE_BN2048_ONE(x)    do { BE_BN2048_ZERO(x); (x)->as_u8[255] = 0x01; } while(0)
#define BE_BN2048_COPY(r, x)    do { memmove((r)->as_u8, (x)->as_u8, 256); } while(0)
#define BE_BN2048_ALL_ONE(x) do { memset((x)->as_u8, 0xFF, 256); } while(0)
#define BE_BN2048_SET_BIT(x, i) do { x->as_u8[255 - (i >> 3)] |= 1 << (i & 0x7); } while(0)

#define BE_BN4096_HI(x) ((x)->as_u8)
#define BE_BN4096_LO(x) ((x)->as_u8 + 256)

typedef union {
  uint32_t as_u32[32];
  uint8_t as_u8[128];
} BE_BN1024;

#define BE_BN2048_HI(x) ((x)->as_u8)
#define BE_BN2048_LO(x) ((x)->as_u8 + 128)

#define BE_BN1024_COPY(r, x)    do { memmove((r)->as_u8, (x)->as_u8, 128); } while(0)

#define MM_MUL_BE(block, r, prime, x, y) do { mm_mul_be(block, (r)->as_u32, prime->as_u32, (x)->as_u32, (y)->as_u32) ; } while(0)
#define MM_ADD_BE(block, r, prime, x, y) do { mm_add_be(block, (r)->as_u32, prime->as_u32, (x)->as_u32, (y)->as_u32) ; } while(0)
#define MM_SUB_BE(block, r, prime, x, y) do { mm_sub_be(block, (r)->as_u32, prime->as_u32, (x)->as_u32, (y)->as_u32) ; } while(0)

static void mul(BE_BN4096 *AB, const BE_BN2048 *A, const BE_BN2048 *B) {
    // P = 2^2048 - 1
    // montgomery actually just asked for gcd(P, R) = 1
    BE_BN2048 _P, *P = &_P;
    BE_BN2048_ALL_ONE(P);
    // R = 2^2048 \equiv 1 mod P, R2=1, Rinv = 1

    // A = a * 2^1024 + b
    // B = c * 2^1024 + d
    const BE_BN1024 *a = (const BE_BN1024 *)BE_BN2048_HI(A);
    const BE_BN1024 *b = (const BE_BN1024 *)BE_BN2048_LO(A);
    const BE_BN1024 *c = (const BE_BN1024 *)BE_BN2048_HI(B);
    const BE_BN1024 *d = (const BE_BN1024 *)BE_BN2048_LO(B);

    // a * c, a * d, b * c, b * d
    BE_BN2048 _AC, *AC = &_AC;
    BE_BN2048 _AD, *AD = &_AD;
    BE_BN2048 _BC, *BC = &_BC;
    BE_BN2048 _BD, *BD = &_BD;

    BE_BN2048 _TMP1, *TMP1 = &_TMP1;
    BE_BN2048 _TMP2, *TMP2 = &_TMP2;
    // clear high bits
    BE_BN2048_ZERO(TMP1);
    BE_BN2048_ZERO(TMP2);

    BE_BN1024 *TMP1LO = (BE_BN1024 *)BE_BN2048_LO(TMP1);
    BE_BN1024 *TMP2LO = (BE_BN1024 *)BE_BN2048_LO(TMP2);

    BE_BN1024_COPY(TMP1LO, a);
    BE_BN1024_COPY(TMP2LO, c);
    MM_MUL_BE(BLOCK_2048, AC, P, TMP1, TMP2);

    BE_BN1024_COPY(TMP1LO, a);
    BE_BN1024_COPY(TMP2LO, d);
    MM_MUL_BE(BLOCK_2048, AD, P, TMP1, TMP2);

    BE_BN1024_COPY(TMP1LO, b);
    BE_BN1024_COPY(TMP2LO, c);
    MM_MUL_BE(BLOCK_2048, BC, P, TMP1, TMP2);

    BE_BN1024_COPY(TMP1LO, b);
    BE_BN1024_COPY(TMP2LO, d);
    MM_MUL_BE(BLOCK_2048, BD, P, TMP1, TMP2);

    // 1024   1024   1024   1024
    // HI(ac) LO(ac)
    //        HI(ad) LO(ad)
    //        HI(bc) LO(bc)
    //               HI(bd) LO(bd)
    // LO(BD)
    BE_BN1024_COPY((BE_BN1024 *)BE_BN2048_LO((BE_BN2048 *)BE_BN4096_LO(AB)), (BE_BN1024 *)BE_BN2048_LO(BD));
    // LO(LO(ad) + LO(bc) + HI(bd))
    BE_BN1024_COPY(TMP1LO, (BE_BN1024 *)BE_BN2048_HI(BD)); // tmp1 = HI(bd)
    BE_BN1024_COPY(TMP2LO, (BE_BN1024 *)BE_BN2048_LO(BC)); // tmp2 = LO(bc)
    MM_ADD_BE(BLOCK_2048, TMP1, P, TMP1, TMP2); // tmp1 = HI(bd) + LO(bc)
    BE_BN1024_COPY(TMP2LO, (BE_BN1024 *)BE_BN2048_LO(AD)); // tmp2 = LO(ad)
    MM_ADD_BE(BLOCK_2048, TMP1, P, TMP1, TMP2); // tmp1 = LO(ad) + LO(bc) + HI(bd)
    BE_BN1024_COPY((BE_BN1024 *)BE_BN2048_HI((BE_BN2048 *)BE_BN4096_LO(AB)), (BE_BN1024 *)BE_BN2048_LO(TMP1));
    // HI(AB)
    BE_BN1024_COPY(TMP2LO, (BE_BN1024 *)BE_BN2048_HI(TMP1)); // tmp2 = HI(tmp1) = HI(LO(ad) + LO(bc) + HI(bd)) = carry
    BE_BN2048_ZERO(TMP1); // TMP1 is polluted
    BE_BN1024_COPY(TMP1LO, (BE_BN1024 *)BE_BN2048_HI(BC)); // tmp1 = HI(bc)
    MM_ADD_BE(BLOCK_2048, TMP1, P, TMP1, TMP2); // tmp1 = HI(bc) + carry
    BE_BN1024_COPY(TMP2LO, (BE_BN1024 *)BE_BN2048_HI(AD)); // tmp2 = HI(ad)
    MM_ADD_BE(BLOCK_2048, TMP1, P, TMP1, TMP2); // tmp1 = HI(ad) + HI(bc) + carry
    MM_ADD_BE(BLOCK_2048, (BE_BN2048 *)BE_BN4096_HI(AB), P, AC, TMP1); // HI(AB) = AC + tmp1
}

// note that the key is big endian
int rsa4096_get_public_key(rsa_key_t *key, uint8_t *n) {
    mul((BE_BN4096 *)n, (const BE_BN2048 *)key->p, (const BE_BN2048 *)key->q);
    return 0;
}

// compute R2P, could not be done in place
static void R2P_2048(BE_BN2048 *R, const BE_BN2048 *P) {
    // first set R to 2 ^ 1023
    // for rsa it is ensured that P > 2 ^ 1023
    // so that R < P
    BE_BN2048_ZERO(R);
    BE_BN2048_SET_BIT(R, 1023);

    // now R = 2 ^ 2048 % P
    MM_ADD_BE(BLOCK_2048, R, P, R, R);

    // produce A1 = 2^1 * R % P
    MM_ADD_BE(BLOCK_2048, R, P, R, R);

    int bits = 2048;
    // 11 round double
    // 1: A2 = A1 * A1 * Rinv % p = 2^2 * R % p
    // 2: A4 = A2 * A2 * Rinv % p = 2^4 * R % p
    // 3: A8 = A4 * A4 * Rinv % p = 2^8 * R % p
    // ...
    // 10: A2048 = A512 * A512 * Rinv % p = 2^2048 * R % p = R^2 % p
    while (bits >>= 1) {
        MM_MUL_BE(BLOCK_2048, R, P, R, R);
    }
}

// note that the key is big endian
// can be done in-place: i.e. input == output
int rsa4096_private(rsa_key_t *key, const uint8_t *input, uint8_t *output) {
    const BE_BN2048 *P = (const BE_BN2048 *)key->p;
    const BE_BN2048 *Q = (const BE_BN2048 *)key->q;
    const BE_BN2048 *DP = (const BE_BN2048 *)key->dp;
    const BE_BN2048 *DQ = (const BE_BN2048 *)key->dq;
    const BE_BN2048 *QINV = (const BE_BN2048 *)key->qinv;
    const BE_BN4096 *M = (const BE_BN4096 *)input;
    const BE_BN2048 *M1 = (const BE_BN2048 *)BE_BN4096_HI(M);
    const BE_BN2048 *M2 = (const BE_BN2048 *)BE_BN4096_LO(M);

    BE_BN4096 *S = (BE_BN4096 *)output;

    BE_BN4096 _R2, _TMP1, _TMP2, _SQ, _SP;
    BE_BN4096 *R2L = &_R2, *TMP1L = &_TMP1, *TMP2L = &_TMP2, *SQL = &_SQ, *SPL = &_SP;
    // clear HI part
    BE_BN4096_ZERO(R2L);
    BE_BN4096_ZERO(TMP1L);
    BE_BN4096_ZERO(TMP2L);
    BE_BN4096_ZERO(SQL);
    BE_BN4096_ZERO(SPL);

    BE_BN2048 *R2 = (BE_BN2048 *)BE_BN4096_LO(R2L);
    BE_BN2048 *TMP1 = (BE_BN2048 *)BE_BN4096_LO(TMP1L);
    BE_BN2048 *TMP2 = (BE_BN2048 *)BE_BN4096_LO(TMP2L);
    BE_BN2048 *SQ = (BE_BN2048 *)BE_BN4096_LO(SQL);
    BE_BN2048 *SP = (BE_BN2048 *)BE_BN4096_LO(SPL);

    {
    // step 1: SQ = M^DQ mod Q
    // 1.1 compute R2Q = R^2 % Q
    BE_BN2048 *R2Q = R2; // use R2 for Q now, afterwards it will be assigned to R2P
    R2P_2048(R2Q, Q);
    // 1.2 M mod Q = (M1 * R + M2) mod Q = M1 \otimes R2Q + M2 mod Q
    MM_MUL_BE(BLOCK_2048, TMP1, Q, M1, R2Q);   // TMP1 = M1 \otimes R2Q
    MM_ADD_BE(BLOCK_2048, TMP1, Q, TMP1, M2);  // TMP1 = M1 \otimes R2Q + M2 mod Q = M mod Q
    MM_MUL_BE(BLOCK_2048, TMP1, Q, TMP1, R2Q); // TMP1 = M \otimes R2 = MR mod Q (in mont field) = M'
    BE_BN2048 *POW = TMP1;
    // 1.3 SQ' = (M')^{DQ} mod Q
    BE_BN2048_ONE(SQ);
    MM_MUL_BE(BLOCK_2048, SQ, Q, SQ, R2Q); // init value: 1 \otimes R2 = R % Q = 1'
    // 128 * 8 bits = 2048 bits
    for (int i = 255; i >= 0; --i) {
      uint8_t byte = DQ->as_u8[i];
      for (int j = 0; j < 8; ++j) {
        MM_MUL_BE(BLOCK_2048, TMP2, Q, SQ, POW);
        if (byte & 1) {
          BE_BN2048_COPY(SQ, TMP2);
        } else { // to resist timing side-channel
          BE_BN2048_COPY(TMP2, TMP2);
        }
        MM_MUL_BE(BLOCK_2048, POW, Q, POW, POW);
        byte >>= 1;
      }
    }
    // 1.4 SQ = SQ' \otimes 1
    BE_BN2048_ONE(TMP2);
    MM_MUL_BE(BLOCK_2048, SQ, Q, SQ, TMP2);
    }

    // step 2: SP = M^DP mod P
    // 2.1 compute R2P = R^2 % P
    BE_BN2048 *R2P = R2; // assign R2 to R2P
    R2P_2048(R2P, P);
    { // reuse R2P for step 3
    // 2.2 M mod P = (M1 * R + M2) mod P = M1 \otimes R2P + M2 mod P
    MM_MUL_BE(BLOCK_2048, TMP1, P, M1, R2P);   // TMP1 = M1 \otimes R2P
    MM_ADD_BE(BLOCK_2048, TMP1, P, TMP1, M2);  // TMP1 = M1 \otimes R2P + M2 mod P = M mod P
    MM_MUL_BE(BLOCK_2048, TMP1, P, TMP1, R2P); // TMP1 = M \otimes R2 = MR mod P (in mont field) = M'
    BE_BN2048 *POW = TMP1;
    // 1.3 SP' = (M')^{DP} mod P
    BE_BN2048_ONE(SP);
    MM_MUL_BE(BLOCK_2048, SP, P, SP, R2P); // init value: 1 \otimes R2 = R % P = 1'
    // 128 * 8 bits = 2048 bits
    for (int i = 255; i >= 0; --i) {
      uint8_t byte = DP->as_u8[i];
      for (int j = 0; j < 8; ++j) {
        MM_MUL_BE(BLOCK_2048, TMP2, P, SP, POW);
        if (byte & 1) {
          BE_BN2048_COPY(SP, TMP2);
        } else { // to resist timing side-channel
          BE_BN2048_COPY(TMP2, TMP2);
        }
        MM_MUL_BE(BLOCK_2048, POW, P, POW, POW);
        byte >>= 1;
      }
    }
    }

    // step 3: H = QINV * (SP - SQ) mod P
    MM_MUL_BE(BLOCK_2048, TMP1, P, SQ, R2P);   // TMP1 = SQ' = SQ \otimes R2P = SQ' (note, now in Q's mont field)
    MM_SUB_BE(BLOCK_2048, TMP2, P, SP, TMP1);  // TMP2 = SP' - SQ' mod P
    MM_MUL_BE(BLOCK_2048, TMP1, P, QINV, R2P); // TMP1 = QINV' = QINV \otimes R2P
    BE_BN2048 *H = SP; // reuse _SP for H
    MM_MUL_BE(BLOCK_2048, H, P, TMP1, TMP2); // H' = QINV' \otimes (SP' - SQ')
    BE_BN2048_ONE(TMP1);
    MM_MUL_BE(BLOCK_2048, H, P, H, TMP1); // H = H' \otimes 1

    //// step 4: S = SQ + H * Q mod N
    //// compute N (use S as buffer) (in-place note: now we do not use M, so S can be used)
    //rsa4096_get_public_key(key, S->as_u8);
    //// get R2N
    //R2P_4096(R2L, S);
    //// H4096 bit H, see step 3
    //BE_BN4096 *HL = SPL;
    //// H' = H \otimes R2N
    //MM_MUL_BE(BLOCK_4096, HL, S, HL, R2L);
    //// 4096 bit Q in TMP1L
    //BE_BN2048_COPY(TMP1, Q);
    //BE_BN4096 *QL = TMP1L;
    //// Q' = Q \otimes R2N
    //MM_MUL_BE(BLOCK_4096, QL, S, QL, R2L);
    //// H' * Q', store in QL (i.e. TMP1L)
    //MM_MUL_BE(BLOCK_4096, QL, S, QL, HL);
    //// H * Q in QL (i.e. TMP1L)
    //BE_BN4096_ONE(TMP2L);
    //MM_MUL_BE(BLOCK_4096, QL, S, QL, TMP2L); // H = H' \otimes 1
    //// SQ + H * Q
    //// unfortunately, ab != p for mm_add
    //MM_ADD_BE(BLOCK_4096, TMP2L, S, SQL, QL);
    //BE_BN4096_COPY(S, TMP2L);
    return 0;
}
