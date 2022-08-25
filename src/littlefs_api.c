// SPDX-License-Identifier: Apache-2.0
#include "common.h"
#include <fs.h>
#include <memzero.h>
#include <stdalign.h>
#include <string.h>

#include "flash.h"

// canokey-core
#include "device.h"

#define LOOKAHEAD_SIZE 32
#define CACHE_SIZE 512
#define WRITE_SIZE 1
#define READ_SIZE 1

// start from 4M
#define FLASH_ADDR_BASE (4 * 1024 * 1024)
// 12M
#define FLASH_SIZE (12 * 1024 * 1024)
// 64K
#define FLASH_BLOCK_SIZE (64 * 1024)
#define FLASH_ADDR(b, o) (FLASH_ADDR_BASE + (b) * FLASH_BLOCK_SIZE + (o))
// 256
#define FLASH_PAGE_SIZE (256)

static struct lfs_config config;
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
    //for (int i = 0; i != size; ++i) printf("%02X ", ((uint8_t *)buffer)[i]); putchar('\n');
    return 0;
}

int littlefs_api_prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    //DBG_MSG("littlefs prog %d+%d with size %d\n", block, off, size);
    lfs_size_t sent = 0;
    do {
        // wren
        uint8_t wren[1] = {0x06};
        spi_transfer(wren, NULL, 1);
        // calculate to_send
        uint32_t addr = FLASH_ADDR(block, off + sent);
        uint32_t addr_aligndown = addr & ~(FLASH_PAGE_SIZE - 1);
        uint32_t addr_alignup = addr_aligndown + FLASH_PAGE_SIZE;
        lfs_size_t to_send = lfs_min(addr_alignup - addr, size - sent);
        // pp
        uint8_t pp[4] = {0x02};
        fill_addr(pp + 1, block, off + sent);
        //DBG_MSG("at %X with len %X\n", addr, to_send);
        spi_transfer_cmd_addr_with_payload(pp, buffer + sent, NULL, to_send);
        sent += to_send;
        // HACK: otherwise the first bits of the next transfer would corrupt
        device_delay(10);
        // check wip in sr1
        // rdsr1
        uint8_t rdsr1[2] = {0x05, 0x00};
        uint8_t sr1[2] = {0x00, 0x00};
        do {
            spi_transfer(rdsr1, sr1, 2);
        } while ((sr1[1] & 0x1) == 1); // WIP is high
        //// wrdi
        //uint8_t wrdi[1] = {0x04};
        //spi_transfer(wrdi, NULL, 1);
    } while (sent != size);
    //for (int i = 0; i != size; ++i) printf("%02X ", ((uint8_t *)buffer)[i]); putchar('\n');
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
    // HACK here
    device_delay(3);
    // check wip in sr1
    // rdsr1
    uint8_t rdsr1[2] = {0x05, 0x00};
    uint8_t sr1[2] = {0x00, 0x00};
    do {
        spi_transfer(rdsr1, sr1, 2);
    } while ((sr1[1] & 0x1) == 1); // WIP is high
    //// wrdi
    //uint8_t wrdi[1] = {0x04};
    //spi_transfer(wrdi, NULL, 1);
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

    //int err;
    //for (int retry = 0; retry < 3; retry++) {
    //    err = fs_mount(&config);
    //    if (!err) return;
    //}
    // should happen for the first boot
    DBG_MSG("Formating data area...\r\n");
    fs_format(&config);
    int err = fs_mount(&config);
    if (err) {
        ERR_MSG("Failed to mount FS after formating\r\n");
        for (;;)
            ;
    }
}
