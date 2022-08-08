#ifndef SPI_H
#define SPI_H

// SPI related functions
#define SPI_BASE 0x7000

#define SPI_FMT (SPI_BASE + 0x40)
#define SPI_PROTO_MASK 3
#define SPI_PROTO_SINGLE 0
#define SPI_PROTO_DUAL 1
#define SPI_PROTO_QUAD 2

#define SPI_DISABLE_RX 8

#define SPI_FRAME_LEN_SHIFT 16
#define SPI_FRAME_LEN_MASK (0xF << SPI_FRAME_LEN_SHIFT)

#define SPI_CSID (SPI_BASE + 0x10)

#define SPI_CSMODE (SPI_BASE + 0x18)
#define SPI_CSMODE_MASK 3
#define SPI_CSMODE_AUTO 0
#define SPI_CSMODE_HOLD 2
#define SPI_CSMODE_OFF  3

#define SPI_TXDATA (SPI_BASE + 0x48)
#define SPI_TXDATA_FULL (1 << 31)

#define SPI_RXDATA (SPI_BASE + 0x4c)
#define SPI_RXDATA_EMPTY (1 << 31)

#define SPI_TXRXDATA_MASK (0xFF)

#define SPI_FCTRL (SPI_BASE + 0x60)
#define SPI_CONTROL_IO 0
#define SPI_CONTROL_MAPPED 1

int flash_init();
int spi_transfer(const uint8_t* tx_buf, uint8_t* rx_buf, int len);
int spi_transfer_cmd_addr_with_payload(const uint8_t* cmd_addr_buf, const uint8_t* tx_payload_buf, uint8_t* rx_payload_buf, int len);

#endif // SPI_H
