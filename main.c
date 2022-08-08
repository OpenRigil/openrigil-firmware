#include "mmio.h"
#include "dcd_zenithal.h"
#include "usbd_def.h"
#include "interrupt.h"
#include "uart.h"
#include "flash.h"

// canokey-core
#include "common.h"
#include "apdu.h"
#include "applets.h"
#include "device.h"
#include "usb_device.h"

int main(void)
{
    uart_init();
    DBG_MSG("UART Init\n");

    flash_init();
    DBG_MSG("FLASH Init\n");

    littlefs_init();
    DBG_MSG("littlefs Init\n");

    applets_install();
    init_apdu_buffer();
    DBG_MSG("Applets Init\n");

    // avoid touch
    set_nfc_state(1);

    usb_device_init();
    enable_interrupt();
    DBG_MSG("USB Init\n");

    uint8_t has_touch = 0;
    do {
        device_loop(has_touch);
    } while (1);

    return 0;
}

int strong_user_presence_test(void) {
    DBG_MSG("Strong user-presence test is skipped.\n");
    return 0;
}

#define TICK 48000

static uint64_t read_cycles(void) {
    uint32_t cycles;
    uint32_t cycles_h, cycles_h2;
    do {
        asm volatile ("csrr %0, mcycleh": "=r" (cycles_h));
        asm volatile ("csrr %0, mcycle" : "=r" (cycles));
        asm volatile ("csrr %0, mcycleh": "=r" (cycles_h2));
    } while (cycles_h != cycles_h2);
    return (((uint64_t)cycles_h) << 32 | cycles);
}


void device_delay(int tick) {
    uint64_t prev = read_cycles();
    while (1) {
        uint64_t next = read_cycles();
        if (next - prev > tick * TICK) {
            break;
        }
    }
    return;
}
uint32_t device_get_tick(void) {
    return read_cycles() / TICK;
}
void device_disable_irq(void) {}
void device_enable_irq(void) {}
void device_set_timeout(void (*callback)(void), uint16_t timeout) {}

int device_atomic_compare_and_swap(volatile uint32_t *var, uint32_t expect, uint32_t update) {
    if (*var == expect) {
        *var = update;
        return 0;
    } else {
        return -1;
    }
}

int device_spinlock_lock(volatile uint32_t *lock, uint32_t blocking) {
  // Not really working, for test only
    while (*lock) {
        if (!blocking) return -1;
    }
    *lock = 1;
    return 0;
}
void device_spinlock_unlock(volatile uint32_t *lock) { *lock = 0; }
void led_on(void) {}
void led_off(void) {}

void usb_resources_alloc(void) {
  memset(&IFACE_TABLE, 0xFF, sizeof(IFACE_TABLE));
  memset(&EP_TABLE, 0xFF, sizeof(EP_TABLE));
  memset(&EP_SIZE_TABLE, 0xFF, sizeof(EP_SIZE_TABLE));

  IFACE_TABLE.ctap_hid = 0;
  EP_TABLE.ctap_hid = 2;
  EP_SIZE_TABLE.ctap_hid = 64;

  IFACE_TABLE.webusb = 1;

  IFACE_TABLE.ccid = 2;
  EP_TABLE.ccid = 3;
  EP_SIZE_TABLE.ccid = 64;
}
