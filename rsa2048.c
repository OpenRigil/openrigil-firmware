#include "bn.h"
#include "mmm.h"

#include <string.h>

// from canokey-crypto
#include "rsa.h"

typedef union {
  uint32_t as_u32[64];
  uint8_t as_u8[256];
} BE_BN2048;

#define BE_BN2048_ZERO(x)    do { memset((x)->as_u8, 0x00, 256); } while(0)
#define BE_BN2048_ONE(x)    do { BE_BN2048_ZERO(x); (x)->as_u8[255] = 0x01; } while(0)
#define BE_BN2048_COPY(r, x)    do { memcpy((r)->as_u8, (x)->as_u8, 256); } while(0)
#define BE_BN2048_ALL_ONE(x) do { memset((x)->as_u8, 0xFF, 256); } while(0)

typedef union {
  uint32_t as_u32[32];
  uint8_t as_u8[128];
} BE_BN1024;

#define BE_BN1024_ZERO(x)    do { memset((x)->as_u8, 0x00, 128); } while(0)
#define BE_BN1024_ONE(x)    do { BE_BN2048_ZERO(x); (x)->as_u8[127] = 0x01; } while(0)
#define BE_BN1024_COPY(r, x)    do { memcpy((r)->as_u8, (x)->as_u8, 128); } while(0)
#define BE_BN1024_ALL_ONE(x) do { memset((x)->as_u8, 0xFF, 128); } while(0)

#define BE_BN2048_HIGH(x) (x->as_u8)
#define BE_BN2048_LOW(x) (x->as_u8 + 128)

#define MM_MUL_BE(block, r, prime, x, y) do { mm_mul_be(block, (r)->as_u32, prime->as_u32, (x)->as_u32, (y)->as_u32) ; } while(0)

// note that the key is big endian
int rsa2048_get_public_key(rsa_key_t *key, uint8_t *n) {
    // P = 2^2048 - 1
    // montgomery actually just asked for gcd(P, R) = 1
    BE_BN2048 _P, *P = &_P;
    BE_BN2048_ALL_ONE(P);
    // R=2^2048 \equiv 1 mod P, R2=1, Rinv = 1
    // so n = p * q % P = p * q % Rinv = p \otimes q % P

    // key->p and key->q are BE_BN10124, so we need buffer
    // store p in lower bits of n
    BE_BN2048 *N = (BE_BN2048 *)n;
    BE_BN2048_ZERO(N);
    BE_BN1024_COPY((BE_BN1024 *)BE_BN2048_LOW(N), (BE_BN1024 *)key->p);

    // store q in lower bits of tmp buffer T
    BE_BN2048 _T, *T = &_T;
    BE_BN2048_ZERO(T);
    BE_BN1024_COPY((BE_BN1024 *)BE_BN2048_LOW(T), (BE_BN1024 *)key->q);

    // in-place mm mul
    MM_MUL_BE(64, N, P, N, T);

    return 0;
}
