#include <stdint.h>
#include <stdbool.h>

/*------------------------------------------------------------------*/
/* MMIO
 *------------------------------------------------------------------*/

#define USB_BASE        0x5000
#define USB_ADDR        (USB_BASE + 0x00)
#define USB_STALL       (USB_BASE + 0x01)
#define USB_PULLUP      (USB_BASE + 0x02)
#define USB_FRAME       (USB_BASE + 0x04)

#define USB_INTERRUPT   (USB_BASE + 0x08)
#define USB_INTERRUPT_RESET_BIT_MASK   (1 << 0)
#define USB_INTERRUPT_RX_BIT_MASK      (1 << 1)
#define USB_INTERRUPT_TX_BASE_BIT_MASK (1 << 2)
#define USB_INTERRUPT_TX_BIT_MASK(ep)  (USB_INTERRUPT_TX_BASE_BIT_MASK << (ep))

#define USB_RX_FIFO     (USB_BASE + 0x10)
#define USB_RX_EMPTY    (USB_BASE + 0x11)
#define USB_RX_SETUP    (USB_BASE + 0x12)
#define USB_RX_PID      (USB_BASE + 0x13)
#define USB_RX_ENDP     (USB_BASE + 0x14)
#define USB_RX_CONSUMED (USB_BASE + 0x15)

#define USB_TX_BASE     (USB_BASE + 0x20)
#define USB_TX_OFFSET_FIFO     0x0
#define USB_TX_OFFSET_STATUS   0x1
#define USB_TX_OFFSET_PRODUCED 0x2
#define USB_TX_OFFSET_TOGGLE   0x3
#define USB_TX_OFFSET_EP       0x4

#define USB_TX_FIFO(ep)     (USB_TX_BASE + ((ep) * USB_TX_OFFSET_EP) + USB_TX_OFFSET_FIFO)
#define USB_TX_STATUS(ep)   (USB_TX_BASE + ((ep) * USB_TX_OFFSET_EP) + USB_TX_OFFSET_STATUS)
#define USB_TX_PRODUCED(ep) (USB_TX_BASE + ((ep) * USB_TX_OFFSET_EP) + USB_TX_OFFSET_PRODUCED)
#define USB_TX_TOGGLE(ep)   (USB_TX_BASE + ((ep) * USB_TX_OFFSET_EP) + USB_TX_OFFSET_TOGGLE)

#define USB_TX_STATUS_NAKED_BIT_MASK (1 << 0)
#define USB_TX_STATUS_ACKED_BIT_MASK (1 << 1)

/*------------------------------------------------------------------*/
/* Controller API
 *------------------------------------------------------------------*/

#define EP_MAX 4
#define EP_SIZE 64

#define EP_DIR_IN 1
#define EP_DIR_OUT 0
#define EP_IN(e) (e | 0x80)
#define EP_OUT(e) (e & 0x7F)
#define EP_DIR(e) ((e & 0x80) ? EP_DIR_IN : EP_DIR_OUT)
#define EP_NUM(e) (e & 0x7F)


/*------------------------------------------------------------------*/
/* Controller API
 *------------------------------------------------------------------*/

void dcd_init();
void dcd_set_address(uint8_t dev_addr);
void dcd_remote_wakeup();
void dcd_connect();
void dcd_disconnect();

/*------------------------------------------------------------------*/
/* DCD Endpoint port
 *------------------------------------------------------------------*/

bool dcd_edpt_open(uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps);
void dcd_edpt_close_all(void);
void dcd_edpt_xfer(uint8_t ep_addr, const uint8_t *buffer, uint16_t total_bytes);
uint32_t dcd_edpt_get_rx_size(uint8_t ep_addr);
void dcd_edpt_prepare(uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes);

void dcd_edpt_stall(uint8_t ep_addr);
void dcd_edpt_clear_stall(uint8_t ep_addr);

/*------------------------------------------------------------------*/
/* Interrupt and event into upper layer
 *------------------------------------------------------------------*/

void dcd_handle_interrupt(void);

void dcd_event_setup_received(uint8_t* setup);
void dcd_event_data_out(uint8_t ep, uint8_t* buffer);
void dcd_event_data_in(uint8_t ep, uint8_t* buffer);
void dcd_event_bus_reset(void);

