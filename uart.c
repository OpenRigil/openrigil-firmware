#include "mmio.h"
#include <sys/types.h>
#include <sys/stat.h>

/* MMIO register */
#define UART_BASE   0x6000
#define UART_TXFIFO (UART_BASE + 0x00)
#define UART_TXCTRL (UART_BASE + 0x08)

/* TXCTRL register */
#define UART_TXEN   0x1

void uart_init() {
    reg_write32(UART_TXCTRL, UART_TXEN);
}

static inline void uart_putc(char c) {
    volatile uint32_t *tx = (uint32_t*)UART_TXFIFO;
    while ((int32_t)(*tx) < 0);
    // manually insert \r before \n
    if (c == '\n') {
        *tx = '\r';
        while ((int32_t)(*tx) < 0);
    }
    *tx = c;
}

/* Implement syscall from newlib */
int _open(const char *name, int flags, int mode) {
    return 0;
}

int _close(int file) {
    return 0;
}

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _lseek(int file, int offset, int whence) {
    return 0;
}

ssize_t _read(int file, void *ptr, size_t len)
{
    return 0;
}

extern uint8_t __heap_start;

void *_sbrk(int incr) {
    static unsigned char *heap = &__heap_start;
    unsigned char *prev_heap;
    prev_heap = heap;
    heap += incr;
    return prev_heap;
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++) {
        uart_putc(*ptr++);
    }
    return len;
}
