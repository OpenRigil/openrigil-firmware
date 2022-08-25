#include "mmio.h"
#include "dcd_zenithal.h"
#include "usbd_def.h"
#include "interrupt.h"
#include "uart.h"
#include "flash.h"
#include "littlefs_api.h"

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
