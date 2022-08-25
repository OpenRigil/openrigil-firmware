// this is originally implemented by dangfan <https://dang.fan/>
// we rewrite it into our accelerator here
#include "bn.h"
#include "mmm.h"

// canokey-crypto
#include "sha.h"

#include <string.h>

/* store BN in little endian manner (e.g. as_u32[0] for least significant 32 bit)
 * note that RISC-V machine we use is little endian
 * so as_u8[0] is the least significant byte 
 */
typedef union {
  uint32_t as_u32[8];
  uint8_t as_u8[32];
} BN256;

// 256-bit BN represented as 8 uint32_t
#define BLOCK 8

#define BN_ZERO(x)    do { memset((x)->as_u8, 0x00, 32); } while(0)
#define BN_ONE(x)    do { BN_ZERO(x); (x)->as_u8[0] = 0x01; } while(0)
#define BN_COPY(r, x)    do { memmove((r)->as_u8, (x)->as_u8, 32); } while(0)
#define BN_ALL_ONE(x) do { memset((x)->as_u8, 0xFF, 32); } while(0)

typedef struct _para {
  BN256 _PRIME;
  BN256 _TWO;
  BN256 _R2;
} para_t;

#define PRIME (&(para->_PRIME))
#define R2 (&(para->_R2))
#define TWO (&(para->_TWO))

void init_para_25519(para_t *para) {
  // P=2^255-19
  BN_ALL_ONE(PRIME);
  PRIME->as_u8[0] = 0xED;
  PRIME->as_u8[31] = 0x7F;

  // R=2^256
  // R^2 % P = 0x05A4
  BN_ZERO(R2);
  R2->as_u8[0] = 0xA4;
  R2->as_u8[1] = 0x05;

  // 2*R % p = 0x4C
  BN_ZERO(TWO);
  TWO->as_u8[0] = 0x4C;
}

#define MM_MUL(r, x, y) do { mm_mul(BLOCK, (r)->as_u32, PRIME->as_u32, (x)->as_u32, (y)->as_u32) ; } while(0)
#define MM_ADD(r, x, y) do { mm_add(BLOCK, (r)->as_u32, PRIME->as_u32, (x)->as_u32, (y)->as_u32) ; } while(0)
#define MM_SUB(r, x, y) do { mm_sub(BLOCK, (r)->as_u32, PRIME->as_u32, (x)->as_u32, (y)->as_u32) ; } while(0)
#define MM_SQR(r, x) MM_MUL(r, x, x)

void mm_inv_25519(para_t *para, BN256 *r, BN256 *x) {
  // compute r = x^{p-2} where p = 2**255 - 19
  // r != x
  BN256 _x0, _x1, _x2, _x3;
  BN256 *x0 = &_x0;
  BN256 *x1 = &_x1;
  BN256 *x2 = &_x2;
  BN256 *x3 = &_x3;
  MM_SQR(x3, x);
  MM_SQR(x1, x3);
  MM_SQR(x0, x1);
  MM_MUL(x2, x0, x);
  MM_MUL(x0, x2, x3);
  MM_SQR(x1, x0);
  MM_MUL(r, x1, x2);
  MM_SQR(x1, r);
  for (uint8_t i = 0; i < 2; ++i) {
    MM_SQR(x2, x1);
    MM_SQR(x1, x2);
  }
  MM_MUL(x2, x1, r);
  MM_SQR(x1, x2);
  for (uint8_t i = 0; i < 4; ++i) {
    MM_SQR(x3, x1);
    MM_SQR(x1, x3);
  }
  MM_SQR(r, x1);
  MM_MUL(x3, r, x2);
  MM_SQR(x1, x3);
  for (uint8_t i = 1; i < 10; i++) {
    MM_SQR(r, x1);
    MM_SQR(x1, r);
  }
  MM_SQR(r, x1);
  MM_MUL(x1, r, x3);
  for (uint8_t i = 0; i < 5; ++i) {
    MM_SQR(r, x1);
    MM_SQR(x1, r);
  }
  MM_MUL(x3, x1, x2);
  MM_SQR(x1, x3);
  for (uint8_t i = 1; i < 25; i++) {
    MM_SQR(r, x1);
    MM_SQR(x1, r);
  }
  MM_SQR(r, x1);
  MM_MUL(x1, r, x3);
  MM_SQR(r, x1);
  for (uint8_t i = 1; i < 50; i++) {
    MM_SQR(x2, r);
    MM_SQR(r, x2);
  }
  MM_SQR(x2, r);
  MM_MUL(r, x1, x2);
  for (uint8_t i = 0; i < 25; i++) {
    MM_SQR(x1, r);
    MM_SQR(r, x1);
  }
  MM_MUL(x1, r, x3);
  MM_SQR(x2, x1);
  MM_SQR(x1, x2);
  MM_SQR(x2, x1);
  MM_SQR(x1, x2);
  MM_SQR(x2, x1);
  MM_MUL(r, x2, x0);
}

#define MM_INV_25519(r, x) do { mm_inv_25519(para, r, x); } while(0)

////////////////////////////////////////
/// Ed25519
////////////////////////////////////////

/**
 * Arithmetic on the twisted Edwards curve -x^2 + y^2 = 1 + dx^2y^2
 * d = -(121665/121666) = 0x52036cee2b6ffe738cc740797779e89800700a4d4141d8ab75eb4dca135978a3
 *
 * order: 0x1000000000000000000000000000000014DEF9DEA2F79CD65812631A5CF5D3ED
 * Identity element: (0,1)
 * Gx: 0x216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A
 * Gy: 0x6666666666666666666666666666666666666666666666666666666666666658
 */

// little endian
static const uint8_t L_U8[] = {0xED, 0xD3, 0xF5, 0x5C, 0x1A, 0x63, 0x12, 0x58, 0xD6, 0x9C, 0xF7,
                               0xA2, 0xDE, 0xF9, 0xDE, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10};
static const BN256 *L = (const BN256 *)L_U8;

#define MM_MUL_L(r, x, y) do { mm_mul(BLOCK, (r)->as_u32, L->as_u32, (x)->as_u32, (y)->as_u32) ; } while(0)
#define MM_ADD_L(r, x, y) do { mm_add(BLOCK, (r)->as_u32, L->as_u32, (x)->as_u32, (y)->as_u32) ; } while(0)

static const uint8_t R2L_U8[] = {0x01, 0x0F, 0x9C, 0x44, 0xE3, 0x11, 0x06, 0xA4, 0x47, 0x93, 0x85,
                                 0x68, 0xA7, 0x1B, 0x0E, 0xD0, 0x65, 0xBE, 0xF5, 0x17, 0xD2, 0x73,
                                 0xEC, 0xCE, 0x3D, 0x9A, 0x30, 0x7C, 0x1B, 0x41, 0x99, 0x03};
static const BN256 *R2L = (const BN256 *)R2L_U8;

// The following constants are in Montgomery field
static const uint8_t COEFFICIENT2D_MON_U8[] = {0xF4, 0xD3, 0x8F, 0xBE, 0xFD, 0x17, 0xDB, 0x01, 0xE7, 0x52, 0x8C,
                                               0x5F, 0xEF, 0x0E, 0x43, 0x21, 0x20, 0x0D, 0x31, 0x78, 0x0F, 0x24,
                                               0x27, 0xCB, 0x4D, 0x8A, 0x3F, 0xE5, 0xB4, 0x56, 0x04, 0x59};
static const BN256 *COEFFICIENT_2D = (const BN256 *)&COEFFICIENT2D_MON_U8;

static const uint8_t GX_MON_U8[] = {0x87, 0xA2, 0x9D, 0x3F, 0x55, 0xBC, 0xCA, 0xE2, 0x89, 0xE4, 0x96,
                                    0x23, 0x56, 0x98, 0xA5, 0x9C, 0xB7, 0xB5, 0xE4, 0xAD, 0x6B, 0x93,
                                    0x79, 0x98, 0xD0, 0x77, 0x60, 0x7E, 0x70, 0x23, 0x9E, 0x75};
static const BN256 *GX = (const BN256 *)GX_MON_U8;

static const uint8_t GY_MON_U8[] = {0x4A, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
                                    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
                                    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33};
static const BN256 *GY = (const BN256 *)GY_MON_U8;

// CL = 2^256 % L = 0x0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEC6EF5BF4737DCF70D6EC31748D98951D
// CL_MON = (CL * R) % L = (R ^ 2) % L = R2L
static const BN256 *CL = (const BN256 *)R2L_U8;

typedef struct {
  BN256 x;
  BN256 y;
  BN256 z;
  BN256 t;
} ehc; // extended homogeneous coordinates

// see RFC 8032 5.1.4
static void point_add(para_t *para, ehc *X, ehc *A, ehc *B) {
  // allocate in stack
  BN256 _a, _b, _c, _d, _e, _f, _g, _h;
  BN256 *a = &_a, *b = &_b, *c = &_c, *d = &_d, *e = &_e, *f = &_f, *g = &_g, *h = &_h;
  BN256 *X1 = &(A->x), *Y1 = &(A->y), *Z1 = &(A->z), *T1 = &(A->t);
  BN256 *X2 = &(B->x), *Y2 = &(B->y), *Z2 = &(B->z), *T2 = &(B->t);
  BN256 *X3 = &(X->x), *Y3 = &(X->y), *Z3 = &(X->z), *T3 = &(X->t);
  MM_SUB(c, Y1, X1);
  MM_SUB(d, Y2, X2);
  MM_MUL(a, c, d); // A = (Y1-X1)*(Y2-X2)
  MM_ADD(c, Y1, X1);
  MM_ADD(d, Y2, X2);
  MM_MUL(b, c, d); // B = (Y1+X1)*(Y2+X2)
  MM_MUL(d, T1, T2);
  MM_MUL(c, d, COEFFICIENT_2D); // C = T1*2*d*T2
  MM_MUL(e, Z1, Z2);
  MM_MUL(d, e, TWO); // D = Z1*2*Z2
  MM_SUB(e, b, a);   // E = B-A
  MM_SUB(f, d, c);   // F = D-C
  MM_ADD(g, d, c);   // G = D+C
  MM_ADD(h, b, a);   // H = B+A
  MM_MUL(X3, e, f);  // X3 = E*F
  MM_MUL(Y3, g, h);  // Y3 = G*H
  MM_MUL(T3, e, h);  // T3 = E*H
  MM_MUL(Z3, f, g);  // Z3 = F*G
}

// see RFC 8032 5.1.4
static void point_double(para_t *para, ehc *X, ehc *A) {
  // allocate in stack
  BN256 _a, _b, _c, _d, _e, _f, _g, _h;
  BN256 *a = &_a, *b = &_b, *c = &_c, *d = &_d, *e = &_e, *f = &_f, *g = &_g, *h = &_h;
  BN256 *X1 = &(A->x), *Y1 = &(A->y), *Z1 = &(A->z);
  BN256 *X3 = &(X->x), *Y3 = &(X->y), *Z3 = &(X->z), *T3 = &(X->t);
  MM_SQR(a, X1); // A = X1^2
  MM_SQR(b, Y1); // B = Y1^2
  MM_SQR(d, Z1); // C = 2*Z1^2
  MM_MUL(c, TWO, d);
  MM_ADD(h, a, b);   // H = A+B
  MM_ADD(d, X1, Y1); // E = H-(X1+Y1)^2
  MM_SQR(g, d);
  MM_SUB(e, h, g);
  MM_SUB(g, a, b);  // G = A-B
  MM_ADD(f, c, g);  // F = C+G
  MM_MUL(X3, e, f); // X3 = E*F
  MM_MUL(Y3, g, h); // Y3 = G*H
  MM_MUL(T3, e, h); // T3 = E*H
  MM_MUL(Z3, f, g); // Z3 = F*G
}

static void point_reduce(para_t *para, ehc *P) {
  // allocate in stack
  BN256 _zinv;
  BN256 *zinv = &_zinv;
  BN256 *Px = &(P->x), *Py = &(P->y), *Pz = &(P->z), *Pt = &(P->t);
  MM_INV_25519(zinv, Pz);
  MM_MUL(Px, Px, zinv);
  MM_MUL(Py, Py, zinv);
  MM_MUL(Pz, Pz, zinv);
  MM_MUL(Pt, Px, Py);
}

static void compute_kG(para_t *para, BN256 *x, BN256 *y, BN256 *k) {
  // allocate in stack
  ehc ret;
  ehc tmp;
  // ret = (0, 1, 1, 0)
  BN_ZERO(&ret.x);
  BN_ONE(&ret.y);
  BN_ONE(&ret.z);
  BN_ZERO(&ret.t);
  // tmp = (GX, GY, 1, GX * GY) in Montgomery field
  BN_COPY(&tmp.x, GX);
  BN_COPY(&tmp.y, GY);
  BN256 _ONE, *ONE = &_ONE;
  BN_ONE(ONE);
  MM_MUL(&tmp.z, ONE, R2);
  MM_MUL(&tmp.t, GX, GY);
  while (!is_bn_zero(BLOCK, k->as_u32)) {
    if (k->as_u8[0] & 1) {
      point_add(para, &ret, &ret, &tmp);
    }
    point_double(para, &tmp, &tmp);
    bn_shift_right_by_1(BLOCK, k->as_u32);
  }
  point_reduce(para, &ret);
  MM_MUL(x, &ret.x, ONE);
  MM_MUL(y, &ret.y, ONE);
}


void ed25519_publickey(const uint8_t *sk, uint8_t *pk) {
  // allocate in stack
  para_t para;
  init_para_25519(&para);

  uint8_t hash[64];
  sha512_raw(sk, 32, hash);
  hash[0] &= 248;
  hash[31] &= 127;
  hash[31] |= 64;
  // hash[0:31] is a little endian number
  BN256 *s = (BN256 *)hash;

  BN256 x, y;
  compute_kG(&para, &x, &y, s);
  y.as_u8[31] ^= ((x.as_u8[0] & 1) << 7); 
  memcpy(pk, y.as_u8, 32);
}

void ed25519_sign(const uint8_t *m, size_t mlen, const uint8_t *sk, const uint8_t *pk, uint8_t *RS) {
  // allocate in stack
  para_t para;
  init_para_25519(&para);

  BN256 _R, _S, _X, _Y;
  BN256 *R = &_R, *S = &_S, *X = &_X, *Y = &_Y;

  // Step 1: hash the private key
  uint8_t hash[64];
  sha512_raw(sk, 32, hash);
  hash[0] &= 248;
  hash[31] &= 127;
  hash[31] |= 64;
  // store the first half to scalar S as the little endian number
  memcpy(S->as_u8, hash, 32);

  // Step 2: hash the prefix (the second half of the hash) and the message
  sha512_init();
  sha512_update(hash + 32, 32);
  sha512_update(m, mlen);
  sha512_final(hash);

  // Step 3: compute [R]B
  // reduce the 64-bit hash to R
  // prepare hash = a * 2^256 + b
  // also reuse hash buffer
  BN256 *B = (BN256 *)hash;
  BN256 *A = (BN256 *)(hash + 32);
  // compute hash % L = (a * 2^256 + b) % L = (a * CL + b) % L
  // where CL = 2^256 % L = 0x0ffffffffffffffffffffffffffffffec6ef5bf4737dcf70d6ec31748d98951d
  // for mont mul, we use CL = (2^256 * R) % L here
  BN256 _T, *T = &_T;
  // load a
  MM_MUL_L(A, A, R2L);
  // load b
  MM_MUL_L(B, B, R2L);
  // t = a * CL, a = t + b
  MM_MUL_L(T, A, CL);
  MM_ADD_L(A, T, B);
  // unload a to R
  BN_ONE(T);
  MM_MUL_L(R, A, T);
  // backup R
  BN_COPY(T, R);
  // compute [R]B in 25519
  compute_kG(&para, X, Y, R);
  // restore R
  BN_COPY(R, T);
  // encoding the point to first half of RS
  Y->as_u8[31] ^= ((X->as_u8[0] & 1) << 7);
  memcpy(RS, Y->as_u8, 32);

  // Step 4: hash again H(R || pk || m)
  sha512_init();
  sha512_update(Y->as_u8, 32);
  sha512_update(pk, 32);
  sha512_update(m, mlen);
  sha512_final(hash);
  // reduce the 64-bit hash to x
  // compute hash % L = (a * 2^256 + b) % L = (a * CL + b) % L
  // load a
  MM_MUL_L(A, A, R2L);
  // load b
  MM_MUL_L(B, B, R2L);
  // t = a * CL, a = t + b
  MM_MUL_L(T, A, CL);
  MM_ADD_L(A, T, B);

  // Step 5: compute S = (r + k * s) mod L
  BN256 *K = A;
  // load r
  MM_MUL_L(R, R, R2L);
  // load s
  MM_MUL_L(S, S, R2L);
  // t = k * s, s = r + t
  MM_MUL_L(T, K, S);
  MM_ADD_L(S, R, T);
  // unload S
  BN_ONE(T);
  MM_MUL_L(S, S, T);
  memcpy(RS + 32, S->as_u8, 32);
}

// vim: ts=2:sts=2:sw=2
