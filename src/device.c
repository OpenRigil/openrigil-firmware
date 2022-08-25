// canokey-core
#include "common.h"
#include "apdu.h"
#include "applets.h"
#include "device.h"
#include "usb_device.h"

// constants for vendor
#include "git-rev.h"

#ifndef HW_VARIANT_NAME
#define HW_VARIANT_NAME "OpenRigil"
#endif

#ifndef HW_SN
#define HW_SN "Zenithal"
#endif

int admin_vendor_version(const CAPDU *capdu, RAPDU *rapdu) {
  LL = strlen(GIT_REV);
  memcpy(RDATA, GIT_REV, LL);
  if (LL > LE) LL = LE;

  return 0;
}

int admin_vendor_hw_variant(const CAPDU *capdu, RAPDU *rapdu) {
  UNUSED(capdu);

  static const char *const hw_variant_str = HW_VARIANT_NAME;
  size_t len = strlen(hw_variant_str);
  memcpy(RDATA, hw_variant_str, len);
  LL = len;
  if (LL > LE) LL = LE;

  return 0;
}

int admin_vendor_hw_sn(const CAPDU *capdu, RAPDU *rapdu) {
  UNUSED(capdu);

  static const char *const hw_sn = HW_SN;
  size_t len = strlen(hw_sn);
  memcpy(RDATA, hw_sn, len);
  LL = len;
  if (LL > LE) LL = LE;

  return 0;
}

int strong_user_presence_test(void) {
    DBG_MSG("Strong user-presence test is skipped.\n");
    return 0;
}

// 60MHz
#define TICK 60000

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
