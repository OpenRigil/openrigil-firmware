#include <stdint.h>
#include <stdio.h>

void test_ed25519_public() {
  uint32_t prev = device_get_tick();
  //printf("ed25519 public start (%ld)\n", prev);
  //prev = device_get_tick();

  uint8_t in[64] = {0x4c, 0xcd, 0x08, 0x9b, 0x28, 0xff, 0x96, 0xda, 0x9d, 0xb6, 0xc3, 0x46, 0xec, 0x11, 0x4e, 0x0f,
                    0x5b, 0x8a, 0x31, 0x9f, 0x35, 0xab, 0xa6, 0x24, 0xda, 0x8c, 0xf6, 0xed, 0x4f, 0xb8, 0xa6, 0xfb};
  uint8_t out[32];
  uint8_t expected[] = {0x3d, 0x40, 0x17, 0xc3, 0xe8, 0x43, 0x89, 0x5a, 0x92, 0xb7, 0x0a, 0xa7, 0x4d, 0x1b, 0x7e, 0xbc,
                        0x9c, 0x98, 0x2c, 0xcf, 0x2e, 0xc4, 0x96, 0x8c, 0xc0, 0xcd, 0x55, 0xf1, 0x2a, 0xf4, 0x66, 0x0c};

  ed25519_publickey(in, out);

  for (int i = 0; i != 32; ++i) {
    if(out[i] != expected[i]) {
      printf("Error at %d %X\n", i, out[i]);
	}
  }

  uint32_t complete = device_get_tick();
  printf("ed25519 public end %ld (%ld -> %ld)\n", complete - prev, prev, complete);
}