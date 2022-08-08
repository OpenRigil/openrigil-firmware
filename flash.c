#include <stddef.h>
#include "mmio.h"

#include "flash.h"

int flash_init() {
    unsigned long fmt = reg_read32(SPI_FMT);
    /* Set protocol */
    fmt &= ~SPI_PROTO_MASK;
    fmt |= SPI_PROTO_SINGLE;

    /* Always populate receive FIFO */
    fmt &= ~SPI_DISABLE_RX;

    /* Set frame length */
    if ((fmt & SPI_FRAME_LEN_MASK) != (8 << SPI_FRAME_LEN_SHIFT)) {
        fmt &= ~SPI_FRAME_LEN_MASK;
        fmt |= (8 << SPI_FRAME_LEN_SHIFT);
    }
    /* make it effective */
    reg_write32(SPI_FMT, fmt);

    /* Set CS line */
    reg_write32(SPI_CSID, 0);

    /* Toggle off memory-mapped SPI flash mode, toggle on programmable IO mode
     * It seems that with this line uncommented, the debugger cannot have access
     * to the chip at all because it assumes the chip is in memory-mapped mode.
     * I have to compile the code with this line commented and launch gdb,
     * reset cores, reset $pc, set *((int *) 0x20004060) = 0, (set the flash
     * interface control register to programmable I/O mode) and then continue
     * Alternative, comment out the "flash" line in openocd.cfg */
    reg_write32(SPI_FCTRL, SPI_CONTROL_IO);

    return 0;
}

int spi_transfer_raw(const uint8_t* tx_buf, uint8_t* rx_buf, int len) {
    /* Declare time_t variables to break out of infinite while loop */
    //time_t endwait;
    for (int i = 0; i < len; i++) {
        while (reg_read32(SPI_TXDATA) & SPI_TXDATA_FULL)
            ;

        if (tx_buf) {
            reg_write32(SPI_TXDATA, tx_buf[i]);
        } else {
            reg_write32(SPI_TXDATA, 0);
        }

        //endwait = metal_time() + METAL_SPI_RXDATA_TIMEOUT;

        unsigned long rxdata;
        while ((rxdata = reg_read32(SPI_RXDATA)) &
               SPI_RXDATA_EMPTY) {
            //if (metal_time() > endwait) {
            //    METAL_SPI_REGW(METAL_SIFIVE_SPI0_CSMODE) &=
            //        ~(METAL_SPI_CSMODE_MASK);

            //    return 1;
            //}
        }

        if (rx_buf) {
            rx_buf[i] = (uint8_t)(rxdata & SPI_TXRXDATA_MASK);
        }
    }
    return 0;
}

int spi_transfer(const uint8_t* tx_buf, uint8_t* rx_buf, int len) {
    /* Hold the chip select line for all len transferred */
    reg_write32(SPI_CSMODE, ~(SPI_CSMODE_MASK));
    reg_write32(SPI_CSMODE, SPI_CSMODE_HOLD);

    spi_transfer_raw(tx_buf, rx_buf, len);

    /* On the last byte, set CSMODE to auto so that the chip select transitions
     * back to high The reason that CS pin is not deasserted after transmitting
     * out the byte buffer is timing. The code on the host side likely executes
     * faster than the ability of FIFO to send out bytes. After the host
     * iterates through the array, fifo is likely not cleared yet. If host
     * deasserts the CS pin immediately, the following bytes in the output FIFO
     * will not be sent consecutively.
     * There needs to be a better way to handle this. */
    reg_write32(SPI_CSMODE, ~(SPI_CSMODE_MASK));

    return 0;
}

int spi_transfer_cmd_addr_with_payload(const uint8_t* cmd_addr_buf, const uint8_t* tx_payload_buf, uint8_t* rx_payload_buf, int len) {
    /* Hold the chip select line for all len transferred */
    reg_write32(SPI_CSMODE, ~(SPI_CSMODE_MASK));
    reg_write32(SPI_CSMODE, SPI_CSMODE_HOLD);

    spi_transfer_raw(cmd_addr_buf, NULL, 4);
    spi_transfer_raw(tx_payload_buf, rx_payload_buf, len);

    reg_write32(SPI_CSMODE, ~(SPI_CSMODE_MASK));

    return 0;
}
