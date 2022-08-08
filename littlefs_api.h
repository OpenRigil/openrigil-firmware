#ifndef LITTLE_FS_API_H
#define LITTLE_FS_API_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define LFS_NO_ASSERT
#define LFS_NO_MALLOC
#define LFS_NO_DEBUG
#define LFS_NO_WARN
#define LFS_NO_ERROR

#define LOOKAHEAD_SIZE 32
#define CACHE_SIZE 512
#define WRITE_SIZE 1
#define READ_SIZE 1

// Logging functions
#ifdef LFS_YES_TRACE
#define LFS_TRACE(fmt, ...) printf("lfs_trace:%d: " fmt "\n", __LINE__, ##__VA_ARGS__)
#else
#define LFS_TRACE(fmt, ...)
#endif

#ifndef LFS_NO_DEBUG
#define LFS_DEBUG(fmt, ...) printf("lfs_debug:%d: " fmt "\n", __LINE__, ##__VA_ARGS__)
#else
#define LFS_DEBUG(fmt, ...)
#endif

#ifndef LFS_NO_WARN
#define LFS_WARN(fmt, ...) printf("lfs_warn:%d: " fmt "\n", __LINE__, ##__VA_ARGS__)
#else
#define LFS_WARN(fmt, ...)
#endif

#ifndef LFS_NO_ERROR
#define LFS_ERROR(fmt, ...) printf("lfs_error:%d: " fmt "\n", __LINE__, ##__VA_ARGS__)
#else
#define LFS_ERROR(fmt, ...)
#endif

// Runtime assertions
#ifndef LFS_NO_ASSERT
#define LFS_ASSERT(test) assert(test)
#else
#define LFS_ASSERT(test)
#endif

// Min/max functions for unsigned 32-bit numbers
static inline uint32_t lfs_max(uint32_t a, uint32_t b) { return (a > b) ? a : b; }

static inline uint32_t lfs_min(uint32_t a, uint32_t b) { return (a < b) ? a : b; }

// Align to nearest multiple of a size
static inline uint32_t lfs_aligndown(uint32_t a, uint32_t alignment) { return a - (a % alignment); }

static inline uint32_t lfs_alignup(uint32_t a, uint32_t alignment) {
  return lfs_aligndown(a + alignment - 1, alignment);
}

// Find the next smallest power of 2 less than or equal to a
static inline uint32_t lfs_npw2(uint32_t a) { return 32 - __builtin_clz(a - 1); }

// Count the number of trailing binary zeros in a
static inline uint32_t lfs_ctz(uint32_t a) { return __builtin_ctz(a); }

// Count the number of binary ones in a
static inline uint32_t lfs_popc(uint32_t a) { return __builtin_popcount(a); }

// Find the sequence comparison of a and b, this is the distance
// between a and b ignoring overflow
static inline int lfs_scmp(uint32_t a, uint32_t b) { return (int)(unsigned)(a - b); }

// Convert between 32-bit little-endian and native order
static inline uint32_t lfs_fromle32(uint32_t a) { return __builtin_bswap32(a); }

static inline uint32_t lfs_tole32(uint32_t a) { return __builtin_bswap32(a); }

// Convert between 32-bit big-endian and native order
static inline uint32_t lfs_frombe32(uint32_t a) { return a; }

static inline uint32_t lfs_tobe32(uint32_t a) { return a; }

// Calculate CRC-32 with polynomial = 0x04c11db7
uint32_t lfs_crc(uint32_t crc, const void *buffer, size_t size);

extern uint8_t file_buffer[CACHE_SIZE];

// Allocate memory, only used if buffers are not provided to littlefs
// Note, memory must be 64-bit aligned
static inline void *lfs_malloc(size_t size) {
#ifndef LFS_NO_MALLOC
  return malloc(size);
#else
  (void)size;
  return file_buffer;
#endif
}

// Deallocate memory, only used if buffers are not provided to littlefs
static inline void lfs_free(void *p) {
#ifndef LFS_NO_MALLOC
  free(p);
#else
  (void)p;
#endif
}

void littlefs_init(void);

#endif // LITTLE_FS_API_H
