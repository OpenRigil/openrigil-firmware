#include "dcd_zenithal.h"
#include "usbd_def.h"
#include "mmio.h"

#include <stdio.h>

typedef struct {
    uint8_t *buffer;
    uint16_t total_len;
    uint16_t queued_len;
    uint8_t toggle;
    uint8_t stall;
    bool short_packet;
} xfer_ctl_t;

static uint8_t setup_buf[8];

static xfer_ctl_t xfer_status[EP_MAX][2];

/*------------------------------------------------------------------*/
/* Controller API
 *------------------------------------------------------------------*/
void dcd_init()
{
}

void dcd_set_address(uint8_t dev_addr)
{
  reg_write8(USB_ADDR, dev_addr);
}

// connect by enabling internal pull-up resistor on D+/D-
void dcd_connect()
{
    reg_write8(USB_PULLUP, 1);
}

// disconnect by disabling internal pull-up resistor on D+/D-
void dcd_disconnect()
{
    reg_write8(USB_PULLUP, 2);
}

/*------------------------------------------------------------------*/
/* DCD Endpoint port
 *------------------------------------------------------------------*/

bool dcd_edpt_open(uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
    xfer_ctl_t* p = &xfer_status[EP_NUM(ep_addr)][EP_DIR_IN];
    // ctrl xfer starts with pid data1
    // other xfer starts with pid data0
    if (EP_NUM(ep_addr) == 0) {
        p->toggle = 1;
    } else {
        p->toggle = 0;
    }
    return true;
}

void dcd_edpt_close_all()
{
}

void dcd_edpt_xfer(uint8_t ep_addr, const uint8_t *buffer, uint16_t total_bytes)
{
    xfer_ctl_t* p = &xfer_status[EP_NUM(ep_addr)][EP_DIR_IN];
    p->buffer = (uint8_t*)buffer;
    p->total_len = total_bytes;
    p->queued_len = 0; // clear sent bytes

    // so only send data if (short_packet || queued_len != total_len)
    // MPS multiple ZLP is handled by upper layer
    p->short_packet = p->total_len == 0;

    // enable TX interrupt for this ep
    uint32_t interrupt_enable = reg_read32(USB_INTERRUPT_ENABLE);
    interrupt_enable |= USB_INTERRUPT_ENABLE_TX_BIT_MASK(ep_addr);
    reg_write32(USB_INTERRUPT_ENABLE, interrupt_enable);
    // actually send data in IRQ handler
}

void dcd_edpt_prepare(uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes)
{
    xfer_ctl_t* p = &xfer_status[EP_NUM(ep_addr)][EP_DIR_OUT];
    p->buffer = buffer;
    p->total_len = total_bytes;
    reg_write8(USB_RX_CONSUMED, 1);
}

uint32_t dcd_edpt_get_rx_size(uint8_t ep_addr)
{
    return xfer_status[EP_NUM(ep_addr)][EP_DIR_OUT].queued_len;
}

void dcd_edpt_stall(uint8_t ep_addr)
{
    uint32_t interrupt_enable = reg_read32(USB_INTERRUPT_ENABLE);
    // enable TX interrupt for this ep in case we have disabled it
    interrupt_enable |= USB_INTERRUPT_ENABLE_TX_BIT_MASK(EP_NUM(ep_addr));
    // enable RX interrupt for this ep in case we have disabled it
    interrupt_enable |= USB_INTERRUPT_ENABLE_RX_BIT_MASK;
    reg_write32(USB_INTERRUPT_ENABLE, interrupt_enable);
    // stall for one transaction
    reg_write8(USB_STALL, 1);
}

void dcd_edpt_clear_stall(uint8_t ep_addr)
{
}

/*------------------------------------------------------------------*/

static uint16_t receive_packet(uint8_t* buf, size_t len) {
    uint16_t i = 0;
    while (!reg_read8(USB_RX_EMPTY)) {
        int t = reg_read8(USB_RX_FIFO);
        if (i < len) {
            buf[i] = t;
        }
        ++i;
    }
    // discard CRC16
    return i - 2;
}

static void dcd_handle_rx() {
    int setup = reg_read8(USB_RX_SETUP);
    // TODO: check toggle sequence
    //int pid = reg_read8(USB_RX_PID);
    int ep = reg_read8(USB_RX_ENDP);
    if (ep == 0) {
        if (setup) {
            // setup
            receive_packet(setup_buf, 8);
            xfer_ctl_t* p = &xfer_status[ep][EP_DIR_IN];
            p->toggle = 1; // reset toggle for ctrl ep
            dcd_event_setup_received(setup_buf);
            uint16_t length = setup_buf[6] | ((uint16_t)setup_buf[7] << 8);
            if ((setup_buf[0] & 0x80) == 0 && length != 0) {
                // for ctrl out with payload,
                // dcd_event_setup_received must call dcd_edpt_prepare
                // which will clear RX_CONSUMED
                //reg_write8(USB_RX_CONSUMED, 1);
            } else {
                // ready for status for ctrl in
                // clear the interrupt for ctrl out w/o payload
                dcd_edpt_prepare(0, NULL, 0);
            }
        } else {
            xfer_ctl_t* p = &xfer_status[ep][EP_DIR_OUT];
            p->queued_len = receive_packet(p->buffer, p->total_len);
            dcd_event_data_out(ep, p->buffer);
            if (p->buffer == NULL) {
                // DARK MAGIC: 'ctrl in status stage'
                // the usb stack above wont call edpt_prepare for 'ctrl in status'
                // so we do it here
                dcd_edpt_prepare(0, NULL, 0);
            }
        }
    } else {
        xfer_ctl_t* p = &xfer_status[ep][EP_DIR_OUT];
        p->queued_len = receive_packet(p->buffer, p->total_len);
        dcd_event_data_out(ep, p->buffer);
        // do so in edpt_prepare in dcd_event_data_out
        //reg_write8(USB_RX_CONSUMED, 1);
    }
}

static inline void send_data_complete(uint8_t ep) {
    xfer_ctl_t* p = &xfer_status[ep][EP_DIR_IN];
    // complete then update xfer_ctl_t
    int remain = p->total_len - p->queued_len;
    int sent = remain > EP_SIZE ? EP_SIZE : remain;
    p->queued_len += sent;
    // FIXME: isoc has no data toggle
    p->toggle = 1 - p->toggle;
    // it is safe to clear this flag no matter we have sent short packet or not
    p->short_packet = false;

    // enable RX if we have sent all data in xfer_status
    // we are ready for next RX
    // TODO: only for this ep
    // NOTE: this also works for short_packet
    if (p->queued_len == p->total_len) {
        uint32_t interrupt_enable = reg_read32(USB_INTERRUPT_ENABLE);
        interrupt_enable |= USB_INTERRUPT_ENABLE_RX_BIT_MASK;
        reg_write32(USB_INTERRUPT_ENABLE, interrupt_enable);
    }
}

static inline void send_data(uint8_t ep, xfer_ctl_t* p) {
    // possibly data lost so retransmission will call this function again
    // make it idempotent for xfer_ctl_t
    // update xfer_ctl_t when we received ack
    reg_write8(USB_TX_TOGGLE(ep), p->toggle);
    int remain = p->total_len - p->queued_len;
    int to_send = remain > EP_SIZE ? EP_SIZE : remain;
    for (int i = 0; i != to_send; ++i) {
        reg_write8(USB_TX_FIFO(ep), p->buffer[p->queued_len + i]);
    }
    reg_write8(USB_TX_PRODUCED(ep), 1);
}

static inline void send_packet(uint8_t ep) {
    xfer_ctl_t* p = &xfer_status[ep][EP_DIR_IN];
    if (p->short_packet || p->queued_len != p->total_len) {
        send_data(ep, p);
    } else {
        uint32_t interrupt_enable = reg_read32(USB_INTERRUPT_ENABLE);
        // disable this tx interrupt until main_loop calls edpt_xfer
        // this reduces the cost of IRQ handling
        interrupt_enable &= ~(USB_INTERRUPT_ENABLE_TX_BIT_MASK(ep));
        // HACK: disable the rx interrupt until we have sent all data in edpt_xfer
        // otherwise the next rx could override data in internal buffer like ccid buffer
        // FIXME: hardware should enable rx interrupt for setup packet
        // FIXME: interrupt ep should not disable rx
        interrupt_enable &= ~(USB_INTERRUPT_ENABLE_RX_BIT_MASK);
        // make it effective
        reg_write32(USB_INTERRUPT_ENABLE, interrupt_enable);
        // only clear interrupt bit but not send data
        // NOTE: already NAKed in hardware
        // only ask USBD to produce data (may not)
        dcd_event_data_in(ep, p->buffer);
    }
}

static void dcd_handle_tx(uint8_t ep) {
    uintptr_t reg = USB_TX_STATUS(ep);
    uint8_t status = reg_read8(reg);
    if (status & USB_TX_STATUS_ACKED_BIT_MASK) {
        send_data_complete(ep);
        reg_write8(reg, USB_TX_STATUS_ACKED_BIT_MASK);
    }

    if (status & USB_TX_STATUS_NAKED_BIT_MASK) {
        send_packet(ep);
        // NOTE: must write PRODUCED before clear NAKED
        // to ensure that NAK interrupt does not happen again
        // after PRODUCED and before next transaction
        //
        // Actually we should clear txNaked in Interrupt Pending reg
        // when we received PRODUCED in hardware, but we are doing so in software
        // (doing so in hardware is quite racing)
        //
        // Note again: it is possible that after PRODUCED,
        // we send the data in the following transaction,
        // then NAKed the next transaction (should send NAK interrupt)
        // but we only clear the NAKed IP until now
        // so the NAK interrupt is lost
        // this case is harmless
        // as the host would IN again
        reg_write8(reg, USB_TX_STATUS_NAKED_BIT_MASK);
    }
}

void dcd_handle_interrupt()
{
    // should check both ip and ie
    uint32_t ip = reg_read32(USB_INTERRUPT);
    uint32_t ie = reg_read32(USB_INTERRUPT_ENABLE);
    if ((ip & USB_INTERRUPT_RESET_BIT_MASK) && (ie & USB_INTERRUPT_ENABLE_RESET_BIT_MASK)) {
        dcd_event_bus_reset();
        reg_write32(USB_INTERRUPT, USB_INTERRUPT_RESET_BIT_MASK);
    }

    if ((ip & USB_INTERRUPT_RX_BIT_MASK) && (ie & USB_INTERRUPT_ENABLE_RX_BIT_MASK)) {
        dcd_handle_rx();
        reg_write32(USB_INTERRUPT, USB_INTERRUPT_RX_BIT_MASK);
    }

    for (uint8_t ep = 0; ep != EP_MAX; ++ep) {
        if ((ip & USB_INTERRUPT_TX_BIT_MASK(ep)) && (ie & USB_INTERRUPT_ENABLE_TX_BIT_MASK(ep))) {
            dcd_handle_tx(ep);
            reg_write32(USB_INTERRUPT, USB_INTERRUPT_TX_BIT_MASK(ep));
        }
    }
}
