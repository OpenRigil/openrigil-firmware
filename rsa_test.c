#include <stdint.h>
#include <stdio.h>

#include "rsa.h"

uint8_t prsa[] = {
  0xCB, 0xEC, 0x4F, 0x65, 0x78, 0x4D, 0xA9, 0xA9, 0x3D, 0xFB, 0xC3, 0x69, 0xA9, 0x64, 0x1C, 0x82,
  0x5F, 0xDD, 0xC4, 0x55, 0x65, 0xD5, 0x0C, 0xA7, 0x21, 0x38, 0xAF, 0xD5, 0x5A, 0x8A, 0xFD, 0x1C,
  0xC5, 0x65, 0xAB, 0xEF, 0xB4, 0xF0, 0x72, 0x66, 0x8C, 0xCC, 0xDD, 0x97, 0x44, 0x94, 0xF4, 0x4B,
  0xCE, 0x83, 0xF9, 0x66, 0xB4, 0x50, 0xF2, 0xB9, 0x1A, 0xC9, 0x27, 0x22, 0x57, 0x44, 0x6D, 0x6F,
  0xBB, 0xC8, 0x4A, 0x63, 0x55, 0x99, 0x83, 0xA7, 0x98, 0x38, 0x13, 0xBD, 0x44, 0x1A, 0xF4, 0xB0,
  0x1A, 0xE7, 0xDD, 0x49, 0xDC, 0xF0, 0x42, 0x15, 0xA2, 0x4F, 0xB7, 0xED, 0x66, 0xAF, 0xC1, 0x4D,
  0x23, 0x36, 0xFF, 0x9A, 0xE0, 0x37, 0x03, 0x5F, 0xAB, 0x1D, 0xF4, 0x0C, 0x1D, 0xF4, 0x83, 0x80,
  0xF7, 0x3C, 0xE3, 0xE8, 0x6C, 0x15, 0xFA, 0x2D, 0x55, 0xC0, 0x63, 0x5E, 0x75, 0x85, 0x8B, 0x4B
};
uint8_t qrsa[] = {
  0xF0, 0xD6, 0x29, 0x51, 0xAA, 0xF4, 0xEA, 0x2D, 0x5F, 0x0D, 0xA1, 0x7F, 0x0A, 0xD5, 0x08, 0xA6,
  0xA1, 0x79, 0x7D, 0x2A, 0xE0, 0x39, 0x66, 0x12, 0xF4, 0x1A, 0x48, 0x99, 0x83, 0x4F, 0xB2, 0xF3,
  0x65, 0xC4, 0x38, 0x59, 0x18, 0xCF, 0xFA, 0x89, 0xDD, 0xFA, 0x10, 0xA7, 0xC5, 0x84, 0xDE, 0x9C,
  0xD9, 0xE0, 0xF8, 0x8C, 0x96, 0xFA, 0x5C, 0x65, 0x67, 0x5C, 0xD4, 0x15, 0x35, 0xBF, 0xED, 0x94,
  0xD3, 0x1B, 0x57, 0x78, 0xB6, 0xA9, 0x61, 0xED, 0xD9, 0xCF, 0x40, 0xF2, 0x53, 0x85, 0x3B, 0xC2,
  0x25, 0x22, 0xC6, 0x78, 0xBC, 0x8A, 0x45, 0xBB, 0xE8, 0xC6, 0xA2, 0x6D, 0x01, 0xD1, 0xCA, 0x49,
  0x76, 0xC6, 0xDB, 0xE4, 0x88, 0x49, 0x28, 0xDF, 0x18, 0x96, 0x86, 0xB7, 0xD7, 0x26, 0x26, 0x82,
  0x20, 0x1F, 0xB8, 0x59, 0x34, 0xB0, 0x81, 0xCF, 0x96, 0x4F, 0x42, 0x2F, 0xA7, 0xCB, 0x46, 0xDD
};
uint8_t e[] = {0, 1, 0, 1};

uint8_t expected1[256] = {
  0xBF, 0xD8, 0x22, 0xE3, 0x66, 0x45, 0xC3, 0xD9, 0x31, 0xC3, 0x1F, 0xBB, 0x13, 0xE1, 0x0D, 0x9F,
  0x37, 0x76, 0x2A, 0x2F, 0x60, 0xBC, 0x1B, 0x2D, 0x9B, 0x8B, 0xFC, 0x8A, 0x74, 0xA7, 0x31, 0xC7,
  0x06, 0xA4, 0x87, 0xE9, 0x47, 0xB3, 0x56, 0x6F, 0xFD, 0xCA, 0x10, 0x45, 0xA4, 0xE4, 0xBE, 0x42,
  0x03, 0xDB, 0x04, 0x99, 0xD9, 0x2F, 0xC9, 0xEE, 0x86, 0xA1, 0x26, 0x10, 0x6A, 0x25, 0x59, 0x76,
  0xA7, 0xB8, 0x09, 0x0A, 0x3C, 0xFF, 0x51, 0x8C, 0x33, 0x38, 0x0D, 0x8F, 0x73, 0x4A, 0x3C, 0xF1,
  0xB6, 0x46, 0xE4, 0xC9, 0x17, 0x72, 0x71, 0x57, 0x4D, 0xFE, 0x60, 0x23, 0x09, 0xE3, 0x7D, 0x63,
  0xA3, 0xBE, 0x7C, 0x33, 0x59, 0xB7, 0x38, 0x10, 0xEA, 0x7A, 0xBC, 0x02, 0x40, 0x7A, 0xF6, 0x99,
  0xAA, 0x74, 0xA4, 0xA8, 0x2D, 0xC3, 0xAA, 0x42, 0x33, 0x56, 0x74, 0x19, 0xAE, 0x6A, 0x96, 0xE6,
  0x02, 0x57, 0xB6, 0x3F, 0x09, 0x66, 0x52, 0xFF, 0xB1, 0x6A, 0xE8, 0x3D, 0x5E, 0x56, 0x1B, 0x9C,
  0xD2, 0xC9, 0xBD, 0x0E, 0x8E, 0xEE, 0xC7, 0xE1, 0x56, 0xD6, 0xBA, 0x42, 0xE4, 0x99, 0xD1, 0xAE,
  0xE1, 0xEC, 0x0D, 0x5A, 0xFB, 0xEA, 0xBB, 0x69, 0xB5, 0x5C, 0xF6, 0x96, 0x43, 0x1C, 0xFA, 0x50,
  0x67, 0xA5, 0x9A, 0x9C, 0xCB, 0xD2, 0xBB, 0x9B, 0x85, 0xCA, 0xC3, 0xEA, 0x73, 0xF4, 0x9C, 0xA4,
  0x2E, 0xF5, 0xE0, 0x96, 0xF4, 0xF7, 0x76, 0xF9, 0xB8, 0x1F, 0x32, 0xF2, 0x0B, 0x9C, 0x15, 0xD3,
  0x0C, 0xDE, 0xF5, 0x32, 0xB7, 0x15, 0xC1, 0x34, 0xF9, 0xDC, 0x59, 0x97, 0xF8, 0xC4, 0x84, 0x14,
  0x38, 0xBE, 0x93, 0x29, 0xE3, 0x89, 0x85, 0xAC, 0x99, 0x2C, 0xB4, 0x59, 0x82, 0x10, 0xF5, 0x19,
  0x93, 0x8A, 0x5B, 0xA1, 0xF2, 0xF4, 0x68, 0x13, 0xFC, 0x65, 0xD4, 0x36, 0x59, 0xD8, 0xC1, 0xBF
};


void test_rsa2048_get_public() {
  uint8_t buf[256];
  rsa_key_t key;
  key.nbits = 2048;
  memcpy(key.p, prsa, sizeof(prsa));
  memcpy(key.q, qrsa, sizeof(qrsa));
  memcpy(key.e, e, sizeof(e));

  uint32_t prev = device_get_tick();

  rsa2048_get_public_key(&key, buf);

  uint32_t complete = device_get_tick();
  printf("rsa public end %ld (%ld -> %ld)\n", complete - prev, prev, complete);

  for (int i = 0; i != 256; ++i) {
    if(buf[i] != expected1[i]) {
      printf("Error at %d %X\n", i, buf[i]);
	}
  }
}

uint8_t dp[] = {
  0x99, 0xC2, 0x21, 0x8F, 0x89, 0xE9, 0x4D, 0x1E, 0xEE, 0x3C, 0x91, 0xF8, 0x6E, 0x9F, 0xAC, 0x9B,
  0x65, 0x3B, 0x6A, 0xD9, 0x55, 0x42, 0x12, 0x1B, 0x46, 0x82, 0x5E, 0x9A, 0x49, 0x7D, 0xF9, 0x21,
  0xA3, 0x79, 0x68, 0xEE, 0x02, 0x09, 0x36, 0xC9, 0x3A, 0x2B, 0x12, 0x7E, 0x07, 0x59, 0x24, 0xE6,
  0xAF, 0xDD, 0x74, 0xC4, 0xCA, 0xFB, 0x2A, 0x39, 0x06, 0x08, 0x5B, 0xF8, 0x99, 0xC2, 0x5F, 0x9D,
  0xDB, 0x02, 0x47, 0x8C, 0xA4, 0xB3, 0x6E, 0xC0, 0x84, 0x96, 0xEB, 0xD3, 0x4E, 0xB1, 0xE0, 0x83,
  0x94, 0x95, 0xF5, 0x95, 0xC7, 0x00, 0x65, 0x63, 0xB8, 0x91, 0xD1, 0x46, 0x30, 0xA2, 0x1E, 0xE7,
  0x51, 0xC1, 0x1B, 0x73, 0xA4, 0xD1, 0xF6, 0x3A, 0x41, 0x65, 0x01, 0x69, 0xCD, 0x80, 0x5B, 0x53,
  0x77, 0x98, 0x32, 0xE4, 0x41, 0xF3, 0x44, 0x42, 0xC0, 0xA0, 0xA2, 0x8C, 0x4B, 0xA2, 0x99, 0x07
};
uint8_t dq[] = {
  0x4C, 0x25, 0x5A, 0x61, 0x10, 0xED, 0x68, 0xD1, 0x39, 0x99, 0x84, 0xB3, 0x37, 0x48, 0x8D, 0xF8,
  0xA9, 0x5B, 0x51, 0x20, 0xEA, 0x4E, 0x1B, 0xA0, 0xE8, 0x7F, 0x1A, 0xEF, 0x36, 0x9E, 0x4F, 0x55,
  0x37, 0x9A, 0xE7, 0xB9, 0xFB, 0xE7, 0x5E, 0x2E, 0x92, 0xF9, 0x8A, 0xA4, 0x91, 0x05, 0xE6, 0xA0,
  0xAF, 0x1E, 0xBB, 0x57, 0x30, 0x31, 0x2E, 0x5F, 0x7D, 0x63, 0xD3, 0x46, 0x75, 0x7D, 0x1E, 0x70,
  0x0C, 0xA3, 0x3B, 0x6F, 0xD6, 0x49, 0x0D, 0xC6, 0x77, 0x34, 0x59, 0x33, 0x85, 0x06, 0xBE, 0x13,
  0x2F, 0x4C, 0xA4, 0x75, 0xEE, 0x1E, 0xC5, 0xD7, 0x98, 0xCC, 0x9C, 0xF4, 0x5C, 0x3B, 0xE9, 0x7A,
  0x7A, 0xCC, 0x7B, 0x1F, 0xF1, 0x9B, 0x38, 0x09, 0xF9, 0xAB, 0x36, 0x23, 0x10, 0x2A, 0x66, 0xC8,
  0x83, 0xC1, 0xA3, 0xB6, 0xC8, 0xBA, 0xFA, 0x00, 0x19, 0x65, 0x2B, 0xE7, 0x07, 0x65, 0x75, 0x1D
};
uint8_t qinv[] = {
  0x95, 0xB7, 0x7B, 0x17, 0xAE, 0x1A, 0x92, 0xF9, 0x74, 0x54, 0xCF, 0xF6, 0xDC, 0xBC, 0x82, 0x42,
  0xEF, 0x17, 0x3C, 0x6E, 0xF9, 0x1C, 0xDF, 0xB0, 0xF1, 0xE2, 0xD6, 0x64, 0x5C, 0xDD, 0x45, 0x0D,
  0x14, 0xB6, 0x58, 0xBF, 0xDE, 0x4E, 0x02, 0x4F, 0x8C, 0x92, 0x89, 0x2E, 0x62, 0x31, 0x6D, 0x9D,
  0x06, 0xC5, 0x12, 0x22, 0x1F, 0x2A, 0x7B, 0x78, 0xF4, 0xE1, 0xF1, 0x68, 0x50, 0x59, 0x79, 0xD8,
  0x65, 0x3B, 0x94, 0xD2, 0x55, 0x7F, 0x41, 0x62, 0xA2, 0x9A, 0x03, 0x75, 0x7F, 0x70, 0x07, 0xC0,
  0x8C, 0xA1, 0xB1, 0xB3, 0x0F, 0xB3, 0xEC, 0x28, 0x66, 0x64, 0x2B, 0x03, 0x9D, 0x49, 0xD0, 0xDA,
  0x39, 0x01, 0xD7, 0x75, 0xE7, 0x7C, 0x73, 0x35, 0x49, 0xA2, 0x0E, 0xC4, 0xCC, 0x4A, 0x26, 0x99,
  0xF5, 0x79, 0x1D, 0xEB, 0x7F, 0x5D, 0x7B, 0x94, 0x18, 0xA9, 0xA7, 0x4C, 0xD2, 0x7A, 0x6E, 0xBE
};
uint8_t cipher_text[] = {
  0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x30, 0x31, 0x30,
  0x0D, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20,
  0x3D, 0x64, 0x32, 0xC7, 0x19, 0x28, 0xF8, 0xC6, 0x8F, 0x86, 0x88, 0x21, 0xED, 0x08, 0xDE, 0xB9,
  0x3D, 0x1D, 0xFD, 0x38, 0xB1, 0xA5, 0x8D, 0x7A, 0x62, 0xA4, 0x28, 0xC4, 0x20, 0xC5, 0xFB, 0xCF
};
uint8_t expected2[] = {
  0x5B, 0xA8, 0xB5, 0xBB, 0x4F, 0xB6, 0x87, 0xEE, 0x59, 0x0C, 0xE0, 0x72, 0x1F, 0xAE, 0x53, 0xC5,
  0x25, 0x10, 0xA7, 0x9F, 0x7A, 0xB5, 0x16, 0x03, 0x77, 0xA7, 0x04, 0x15, 0x75, 0x47, 0xA3, 0xCC,
  0xB8, 0xD9, 0x9F, 0x7C, 0x86, 0x44, 0xB5, 0x60, 0xFB, 0xF0, 0x64, 0x62, 0x2E, 0xC0, 0x85, 0xA0,
  0x02, 0x51, 0x67, 0x09, 0xDD, 0x00, 0x87, 0x99, 0x33, 0x54, 0x29, 0x30, 0xF2, 0x12, 0xE8, 0x7B,
  0x86, 0x4B, 0x60, 0x10, 0x40, 0xC1, 0x03, 0x4F, 0x47, 0x82, 0xF3, 0x24, 0xA6, 0xCD, 0x55, 0xDB,
  0xA6, 0x52, 0xC4, 0xB4, 0x8E, 0x1C, 0xF1, 0x2B, 0x06, 0xC3, 0x96, 0x8C, 0x9F, 0x6A, 0xD1, 0xEA,
  0xCA, 0xAC, 0x16, 0x99, 0x3F, 0x57, 0xD6, 0xC1, 0x60, 0xCF, 0x76, 0x6A, 0x5C, 0x02, 0x6F, 0x18,
  0x85, 0xAE, 0x04, 0xF9, 0x29, 0x35, 0x84, 0xA1, 0xE1, 0x61, 0xD2, 0xA8, 0x7A, 0x61, 0xD8, 0x13,
  0x35, 0x64, 0x54, 0x76, 0xB8, 0x5D, 0x4D, 0xE6, 0x56, 0x94, 0x47, 0x68, 0x53, 0x1D, 0x51, 0x05,
  0x73, 0xD0, 0xA7, 0xBD, 0x74, 0xE8, 0xE3, 0x58, 0xCF, 0x04, 0xFE, 0x2B, 0x5A, 0xF3, 0x8A, 0x9C,
  0xB8, 0xC5, 0x39, 0xE1, 0x0A, 0x3A, 0xAB, 0xA8, 0xC1, 0xC9, 0xDA, 0x43, 0x9A, 0x07, 0x05, 0x21,
  0x50, 0xFD, 0xB7, 0x66, 0xA2, 0x4E, 0xD6, 0x74, 0x38, 0xED, 0xC0, 0xD7, 0xC8, 0xD5, 0x83, 0x32,
  0xD3, 0xC1, 0x5F, 0xA2, 0x4C, 0xC0, 0xA4, 0x6A, 0xF5, 0xB2, 0xE4, 0xEA, 0x0C, 0xBF, 0x9D, 0x34,
  0x13, 0x0C, 0xB7, 0x68, 0x16, 0xE7, 0x10, 0xF5, 0x6C, 0xD0, 0x20, 0xCA, 0x7D, 0x9A, 0x42, 0x61,
  0xC6, 0x70, 0xC8, 0x31, 0x00, 0xE6, 0x2C, 0x95, 0xAE, 0xB2, 0x20, 0xB3, 0x7B, 0x6E, 0x51, 0x5A,
  0x95, 0x38, 0xDE, 0xF6, 0xA2, 0x7E, 0x0F, 0x32, 0x30, 0x5B, 0x8B, 0xE0, 0x32, 0x75, 0xE1, 0x14
};

void test_rsa2048_decrypt() {
  uint8_t buf[256];
  rsa_key_t key;
  key.nbits = 2048;
  memcpy(key.p, prsa, sizeof(prsa));
  memcpy(key.q, qrsa, sizeof(qrsa));
  memcpy(key.dp, dp, sizeof(dp));
  memcpy(key.dq, dq, sizeof(dq));
  memcpy(key.qinv, qinv, sizeof(qinv));
  memcpy(key.e, e, sizeof(e));

  uint32_t prev = device_get_tick();

  rsa2048_private(&key, cipher_text, buf);

  uint32_t complete = device_get_tick();
  printf("rsa private end %ld (%ld -> %ld)\n", complete - prev, prev, complete);

  for (int i = 0; i != 256; ++i) {
    if(buf[i] != expected2[i]) {
      printf("Error at %d %X\n", i, buf[i]);
	}
  }
}
