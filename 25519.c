// this is originally implemented by dangfan <https://dang.fan/>
// we rewrite it into our accelerator here
#include "mmm.h"

/* store BN in little endian manner (e.g. as_u32[0] for least significant 32 bit)
 * note that RISC-V machine we use is little endian
 * so as_u8[0] is the least significant byte 
 */
typedef union {
  uint32_t as_u32[8];
  uint8_t as_u8[32];
} BN256;

typedef struct _para {
  BN256 _PRIME;
  BN256 _ZERO;
  BN256 _ONE;
  BN256 _R2;
  uint32_t _block;
} para_t;

#define PRIME para->_PRIME
#define ONE para->_ONE
#define ZERO para->_ZERO
#define R2 para->_R2
#define BLOCK para->_block

void init_para_25519(para_t *para) {
  BLOCK = 8; // 256-bit data

  // P=2^255-19
  memset(PRIME.as_u8, 0xFF, 32);
  PRIME.as_u8[0] = 0xED;
  PRIME.as_u8[31] = 0x7F;

  memset(ZERO.as_u8, 0x00, 32);

  memset(ONE.as_u8, 0x00, 32);
  ONE.as_u8[0] = 0x01;

  // R=2^256
  // R^2 % P = 0x05A4
  memset(R2.as_u8, 0x00, 32);
  R2.as_u8[0] = 0xA4;
  R2.as_u8[1] = 0x05;
}

#define INIT_PARA (para_t para; init_para_25519(&para);)

#define MM_MUL(ab, a, b) do { mm_mul(BLOCK, (r).as_u32, (x).as_u32, (y).as_u32, PRIME) ; } while(0)
#define MM_SQR(r, x) MM_MUL(r, x, x)

void mm_inv(para_t *para, BN256* r, BN256 *x) {
	BN256 x2;	// 43
	BN256 x4;	// 41
	BN256 x8;	// 40
	BN256 x9;	// 42
	MM_SQR(x2, *x);
	MM_SQR(x4, x2);
	MM_SQR(x8, x4);
	MM_MUL(x9, x8, *x);
  MM_MUL(x9, x8, *x);
  MM_MUL(x10, x9, x2);
	BN256 *x16 = &x4; // 41
  MM_SQR(*x16, x8);
	// r as tmp buffer for x25
  MM_MUL(*r, *x16, x9);
	BN256 *x50 = x16; // 41
  MM_SQR(*x50, *r);
	BN256 *tmp = &x9; // 42
	BN256 *x800 = x50; // 41
  for (uint8_t i = 0; i < 2; ++i) {
    MM_SQR(*tmp, *x50);
    MM_SQR(*x800, *tmp);
  }
	BN256 *x825 = tmp; // 42
  MM_MUL(*x825, *x800, *r);
	BN256 *x1650 = tmp; // 41
  MM_SQR(*x1650, *x825);
	tmp = &x2; // 43
	BN *x422400 = x1650; // 41
  for (uint8_t i = 0; i < 4; ++i) {
    MM_SQR(*tmp, *x1650);
    MM_SQR(*x422400, *tmp);
  }
	// r = x844800
  MM_SQR(*r, *x422400);
	BN *x845625 = tmp; // 43
  MM_MUL(*x845625, *r, *x825);
	BN *x1691250 = x422400; // 41
  MM_SQR(*x1691250, *x845625);
	BN *x1773404160000 = x1691250; // 41
  for (uint8_t i = 1; i < 10; i++) {
    // r as tmp buf
    MM_SQR(*r, *x1691250);
    MM_SQR(*x1773404160000, *r);
  }
  // r = x3546808320000
  MM_SQR(*r, *x1773404160000);
	BN *x3546809165625 = x1773404160000; // 41
  MM_MUL(*x3546809165625, *r, *x845625);
	BN *x3631932585600000 = x3546809165625; // 41
  for (uint8_t i = 0; i < 5; ++i) {
    MM_SQR(*r, *x3546809165625);
    MM_SQR(*x3631932585600000, *r);
  }
	BN *x3631932585600825 = x845625; // 43
  MM_MUL(*x3631932585600825, *x3631932585600000, *x825);
	BN *x7263865171201650 = x3631932585600000; // 41
  MM_SQR(*x7263865171201650, *x3631932585600825);
	BN *x8178385119573318768063519129600 = x7263865171201650; // 41
  for (uint8_t i = 1; i < 25; i++) {
    MM_SQR(*r, *x7263865171201650);
    MM_SQR(*x8178385119573318768063519129600, *r);
  }
  // r = x16356770239146637536127038259200
  MM_SQR(*r, *x8178385119573318768063519129600);
	BN *x16356770239146641168059623860025 = x8178385119573318768063519129600; // 41
  MM_MUL(*x16356770239146641168059623860025, *r, *x3631932585600825);
	// r = x32713540478293282336119247720050
  MM_SQR(*r, *x16356770239146641168059623860025);
  for (uint8_t i = 1; i < 50; i++) {
		// use x825 as tmp buffer
    MM_SQR(*x825, *r);
    MM_SQR(*r, *x825);
  }
	// r = x41469339222898958093550015788532188587876218953168614902988800
	BN *x82938678445797916187100031577064377175752437906337229805977600 = x825; // 42
  MM_SQR(*x82938678445797916187100031577064377175752437906337229805977600, *r);
	// r = x82938678445797916187100031577080733945991584547505289429837625
  sm2MonMul(*r, *x16356770239146641168059623860025, *x82938678445797916187100031577064377175752437906337229805977600);
  for (uint8_t i = 0; i < 25; i++) {
		// use x16356770239146641168059623860025 as tmp buf
    monSqr(*x16356770239146641168059623860025, *r);
    monSqr(*r, *x16356770239146641168059623860025);
  }
	// r = x82938678445797916187100031577080733945991584547505289429837625
	BN *x93380650335774220916763827294471207111809010479334470841314076215223334528825 = x16356770239146641168059623860025; // 41
  MM_MUL(x93380650335774220916763827294471207111809010479334470841314076215223334528825, *r, *x3631932585600825);
  monSqr(42, 41);
  monSqr(41, 42);
  monSqr(42, 41);
  monSqr(41, 42);
  monSqr(42, 41);
  MM_MUL(r, 42, 40);
}

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

typedef struct _field_25519 {
  BN256 _TWO;
} field_25519_t;

#define TWO field_25519->_TWO

// little endian
static const uint8_t L[] = {0xED, 0xD3, 0xF5, 0x5C, 0x1A, 0x63, 0x12, 0x58, 0xD6, 0x9C, 0xF7,
                            0xA2, 0xDE, 0xF9, 0xDE, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10};

static const uint8_t R2L[] = {0x01, 0x0F, 0x9C, 0x44, 0xE3, 0x11, 0x06, 0xA4, 0x47, 0x93, 0x85,
                              0x68, 0xA7, 0x1B, 0x0E, 0xD0, 0x65, 0xBE, 0xF5, 0x17, 0xD2, 0x73,
                              0xEC, 0xCE, 0x3D, 0x9A, 0x30, 0x7C, 0x1B, 0x41, 0x99, 0x03};

// The following constants are in Montgomery field
static const uint8_t COEFFICIENT2D_MON[] = {0xF4, 0xD3, 0x8F, 0xBE, 0xFD, 0x17, 0xDB, 0x01, 0xE7, 0x52, 0x8C,
                                            0x5F, 0xEF, 0x0E, 0x43, 0x21, 0x20, 0x0D, 0x31, 0x78, 0x0F, 0x24,
                                            0x27, 0xCB, 0x4D, 0x8A, 0x3F, 0xE5, 0xB4, 0x56, 0x04, 0x59};

static const uint8_t GX_MON[] = {0x87, 0xA2, 0x9D, 0x3F, 0x55, 0xBC, 0xCA, 0xE2, 0x89, 0xE4, 0x96,
                                 0x23, 0x56, 0x98, 0xA5, 0x9C, 0xB7, 0xB5, 0xE4, 0xAD, 0x6B, 0x93,
                                 0x79, 0x98, 0xD0, 0x77, 0x60, 0x7E, 0x70, 0x23, 0x9E, 0x75};

static const uint8_t GY_MON[] = {0x4A, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
                                 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
                                 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33};

// CL = 2^256 % L = 0x0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEC6EF5BF4737DCF70D6EC31748D98951D
static const uint8_t CL_MON[] = {0x01, 0x0F, 0x9C, 0x44, 0xE3, 0x11, 0x06, 0xA4, 0x47, 0x93, 0x85,
                                 0x68, 0xA7, 0x1B, 0x0E, 0xD0, 0x65, 0xBE, 0xF5, 0x17, 0xD2, 0x73,
                                 0xEC, 0xCE, 0x3D, 0x9A, 0x30, 0x7C, 0x1B, 0x41, 0x99, 0x03};

static void init_field_25519(field_25519_t *field_25519) {
  memset(TWO.as_u8, 0, 32);
  TWO.as_u8[0] = 0x4C;
  eccPKEWriteBuf(TWO, tmp.as_u32, 8);
}

#define INIT_FIELD_25519 (field_25519_t field_25519; init_field_25519(&field_25519);)

typedef struct {
  BN256 x;
  BN256 y;
  BN256 z;
  BN256 t;
} ehc; // extended homogeneous coordinates

// see RFC 8032 5.1.4
static void point_add(ehc X, ehc A, ehc B) {
  uint8_t a = 28, b = 29, c = 30, d = 31, e = 32, f = 33, g = 34, h = 35;
  uint8_t X1 = A.x, Y1 = A.y, Z1 = A.z, T1 = A.t;
  uint8_t X2 = B.x, Y2 = B.y, Z2 = B.z, T2 = B.t;
  uint8_t X3 = X.x, Y3 = X.y, Z3 = X.z, T3 = X.t;
  sm2MonSub(c, Y1, X1);
  sm2MonSub(d, Y2, X2);
  sm2MonMul(a, c, d); // A = (Y1-X1)*(Y2-X2)
  sm2MonAdd(c, Y1, X1);
  sm2MonAdd(d, Y2, X2);
  sm2MonMul(b, c, d); // B = (Y1+X1)*(Y2+X2)
  sm2MonMul(d, T1, T2);
  sm2MonMul(c, d, COEFFICIENT_2D); // C = T1*2*d*T2
  sm2MonMul(e, Z1, Z2);
  sm2MonMul(d, e, TWO); // D = Z1*2*Z2
  sm2MonSub(e, b, a);   // E = B-A
  sm2MonSub(f, d, c);   // F = D-C
  sm2MonAdd(g, d, c);   // G = D+C
  sm2MonAdd(h, b, a);   // H = B+A
  sm2MonMul(X3, e, f);  // X3 = E*F
  sm2MonMul(Y3, g, h);  // Y3 = G*H
  sm2MonMul(T3, e, h);  // T3 = E*H
  sm2MonMul(Z3, f, g);  // Z3 = F*G
}

// see RFC 8032 5.1.4
static void point_double(ehc X, ehc A) {
  uint8_t a = 28, b = 29, c = 30, d = 31, e = 32, f = 33, g = 34, h = 35;
  uint8_t X1 = A.x, Y1 = A.y, Z1 = A.z;
  uint8_t X3 = X.x, Y3 = X.y, Z3 = X.z, T3 = X.t;
  monSqr(a, X1); // A = X1^2
  monSqr(b, Y1); // B = Y1^2
  monSqr(d, Z1); // C = 2*Z1^2
  sm2MonMul(c, TWO, d);
  sm2MonAdd(h, a, b);   // H = A+B
  sm2MonAdd(d, X1, Y1); // E = H-(X1+Y1)^2
  monSqr(g, d);
  sm2MonSub(e, h, g);
  sm2MonSub(g, a, b);  // G = A-B
  sm2MonAdd(f, c, g);  // F = C+G
  sm2MonMul(X3, e, f); // X3 = E*F
  sm2MonMul(Y3, g, h); // Y3 = G*H
  sm2MonMul(T3, e, h); // T3 = E*H
  sm2MonMul(Z3, f, g); // Z3 = F*G
}

static void point_reduce(ehc P) {
  uint8_t zinv = 31, tmp = 32;
  monInv(zinv, P.z);
  sm2MonMul(tmp, P.x, zinv);
  sm2MonAdd(P.x, tmp, ZERO);
  sm2MonMul(tmp, P.y, zinv);
  sm2MonAdd(P.y, tmp, ZERO);
  sm2MonMul(tmp, P.z, zinv);
  sm2MonAdd(P.z, tmp, ZERO);
  sm2MonMul(P.t, P.x, P.y);
}

static void compute_kG(context_ed25519 *ctx, field_25519_t *field_25519, BN256 *x, BN256 *y, BN256 *k) {
  // allocate in stack
  ehc ret;
  ehc tmp;
  // ret = (0, 1, 1, 0) in Montgomery field
  mm_add(BLOCK, ret.x, ZERO, ZERO, PRIME);
  mm_add(BLOCK, ret.y, ONE, ZERO, PRIME);
  mm_add(BLOCK, ret.z, ONE, ZERO, PRIME);
  mm_add(BLOCK, ret.t, ZERO, ZERO, PRIME);
  // tmp = (GX, GY, 1, GX * GY) in Montgomery field
  mm_add(BLOCK, tmp.x, GX, ZERO, PRIME);
  mm_add(BLOCK, tmp.y, GY, ZERO, PRIME);
  mm_mul(BLOCK, tmp.z, ONE, R2, PRIME);
  mm_mul(BLOCK, tmp.t, GX, GY, PRIME);
  while (!is_bn_zero(k->as_u32)) {
    // TODO: may not constant time
    if (k->as_u8[0] & 1) {
      point_add(ret, ret, tmp);
    }
    point_double(tmp, tmp);
    bn_shift_right_by_1(k->as_u32);
  }
  point_reduce(ret);
  mm_mul(BLOCK, tmp.x, ret.x, ONE, PRIME);
  mm_mul(BLOCK, tmp.y, ret.y, ONE, PRIME);
  memcpy(x->as_u8, tmp.x.as_u8, 32);
  memcpy(y->as_u8, tmp.y.as_u8, 32);
}


void ed25519_publickey(const uint8_t *sk, uint8_t *pk) {
  // allocate in stack
  INIT_PARA;
  INIT_FIELD_25519;

  uint8_t hash[64];
  sha512_raw(sk, 32, hash);
  hash[0] &= 248;
  hash[31] &= 127;
  hash[31] |= 64;
  // hash[0:31] is a little endian number
  BN256 *s = hash;

  BN256 x, y;
  compute_kG(ctx, field_25519, &x, &y, s);
  y.as_u8[31] ^= ((x.as_u8[0] & 1) << 7); 
  memcpy(pk, y.as_u8, 32);
}

// vim: ts=2:sts=2:sw=2
