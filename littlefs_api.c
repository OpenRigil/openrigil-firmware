// SPDX-License-Identifier: Apache-2.0
#include "common.h"
#include <fs.h>
#include <memzero.h>
#include <stdalign.h>
#include <string.h>

#include "flash.h"
#include "littlefs_api.h"

// start from 4M
#define FLASH_ADDR_BASE (4 * 1024 * 1024)
// 12M
#define FLASH_SIZE (12 * 1024 * 1024)
// 64K
#define FLASH_BLOCK_SIZE (64 * 1024)
#define FLASH_ADDR(b, o) (FLASH_ADDR_BASE + (b) * FLASH_BLOCK_SIZE + (o))

static struct lfs_config config;
alignas(8) uint8_t file_buffer[CACHE_SIZE];
static uint8_t read_buffer[CACHE_SIZE];
static uint8_t prog_buffer[CACHE_SIZE];
static alignas(4) uint8_t lookahead_buffer[LOOKAHEAD_SIZE];

static void fill_addr(uint8_t *buf, lfs_block_t block, lfs_off_t off) {
    uint32_t addr = FLASH_ADDR(block, off);
    buf[0] = (uint8_t)((addr >> 16) & 0xFF);
    buf[1] = (uint8_t)((addr >>  8) & 0xFF);
    buf[2] = (uint8_t)((addr      ) & 0xFF);
}

int littlefs_api_read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    //DBG_MSG("littlefs read %d+%d with size %d\n", block, off, size);
    uint8_t read[4] = {0x03};
    fill_addr(read + 1, block, off);
    spi_transfer_cmd_addr_with_payload(read, NULL, buffer, size);
    return 0;
}

int littlefs_api_prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    //DBG_MSG("littlefs prog %d+%d with size %d\n", block, off, size);
    // wren
    uint8_t wren[1] = {0x06};
    spi_transfer(wren, NULL, 1);
    // pp
    uint8_t pp[4] = {0x02};
    fill_addr(pp + 1, block, off);
    spi_transfer_cmd_addr_with_payload(pp, buffer, NULL, size);
    // check wip in sr1
    // rdsr1
    uint8_t rdsr1[2] = {0x05, 0x00};
    uint8_t sr1[2] = {0x00, 0x00};
    do {
        spi_transfer(rdsr1, sr1, 2);
    } while ((sr1[1] & 0x1) == 1); // WIP is high
    // wrdi
    uint8_t wrdi[1] = {0x04};
    spi_transfer(wrdi, NULL, 1);
    return 0;
}

int littlefs_api_erase(const struct lfs_config *c, lfs_block_t block) {
    //DBG_MSG("littlefs erase %d\n", block);
    // wren
    uint8_t wren[1] = {0x06};
    spi_transfer(wren, NULL, 1);
    // se
    uint8_t se[4] = {0xD8, 0x00, 0x00, 0x00};
    fill_addr(se + 1, block, 0);
    spi_transfer(se, NULL, 4);
    // check wip in sr1
    // rdsr1
    uint8_t rdsr1[2] = {0x05, 0x00};
    uint8_t sr1[2] = {0x00, 0x00};
    do {
        spi_transfer(rdsr1, sr1, 2);
    } while ((sr1[1] & 0x1) == 1); // WIP is high
    // wrdi
    uint8_t wrdi[1] = {0x04};
    spi_transfer(wrdi, NULL, 1);
    return 0;
}

int littlefs_api_sync(const struct lfs_config *c) {
    (void)c;
    return 0;
}

void littlefs_init() {
    memzero(&config, sizeof(config));
    config.read = littlefs_api_read;
    config.prog = littlefs_api_prog;
    config.erase = littlefs_api_erase;
    config.sync = littlefs_api_sync;
    config.read_size = READ_SIZE;
    config.prog_size = WRITE_SIZE;
    config.block_size = FLASH_BLOCK_SIZE;
    config.block_count = FLASH_SIZE / FLASH_BLOCK_SIZE;
    config.block_cycles = 100000;
    config.cache_size = CACHE_SIZE;
    config.lookahead_size = LOOKAHEAD_SIZE;
    config.read_buffer = read_buffer;
    config.prog_buffer = prog_buffer;
    config.lookahead_buffer = lookahead_buffer;
    DBG_MSG("Flash %u blocks (%u bytes)\r\n", config.block_count, FLASH_BLOCK_SIZE);

    int err;
    for (int retry = 0; retry < 3; retry++) {
        err = fs_mount(&config);
        if (!err) return;
    }
    // should happen for the first boot
    DBG_MSG("Formating data area...\r\n");
    fs_format(&config);
    err = fs_mount(&config);
    if (err) {
        ERR_MSG("Failed to mount FS after formating\r\n");
        for (;;)
            ;
    }
}

uint32_t lfs_crc(uint32_t crc, const void *buffer, size_t size) {
  static const uint32_t rtable[16] = {
      0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
      0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
  };

  const uint8_t *data = buffer;

  for (size_t i = 0; i < size; i++) {
    crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 0)) & 0xf];
    crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 4)) & 0xf];
  }

  return crc;
}
