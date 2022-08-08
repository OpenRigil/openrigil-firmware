#include "mmio.h"
#include "plic.h"
#include "interrupt.h"

static inline void csr_write_mie(uint32_t value) {
    __asm__ volatile ("csrw    mie, %0" 
                      : /* output: none */ 
                      : "r" (value) /* input : from register */
                      : /* clobbers: none */);
}
/* Register CSR bit set and clear instructions */
static inline void csr_set_bits_mie(uint32_t mask) {
    __asm__ volatile ("csrrs    zero, mie, %0"  
                      : /* output: none */ 
                      : "r" (mask)  /* input : register */
                      : /* clobbers: none */);
}
static inline void csr_clr_bits_mie(uint32_t mask) {
    __asm__ volatile ("csrrc    zero, mie, %0"  
                      : /* output: none */ 
                      : "r" (mask)  /* input : register */
                      : /* clobbers: none */);
}

/* Register CSR bit set and clear instructions */
static inline void csr_set_bits_mstatus(uint32_t mask) {
    __asm__ volatile ("csrrs    zero, mstatus, %0"  
                      : /* output: none */ 
                      : "r" (mask)  /* input : register */
                      : /* clobbers: none */);
}
static inline void csr_clr_bits_mstatus(uint32_t mask) {
    __asm__ volatile ("csrrc    zero, mstatus, %0"  
                      : /* output: none */ 
                      : "r" (mask)  /* input : register */
                      : /* clobbers: none */);
}
static inline void csr_write_mtvec(uint32_t value) {
    __asm__ volatile ("csrw    mtvec, %0" 
                      : /* output: none */ 
                      : "r" (value) /* input : from register */
                      : /* clobbers: none */);
}

#define MSTATUS_MIE_BIT_MASK     0x8
#define MIE_MEI_BIT_MASK     0x800

#define RISCV_MTVEC_MODE_VECTORED 1

void enable_interrupt() {
    // Global interrupt disable
    csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);
    csr_write_mie(0);

    // enable plic usb
    reg_write32(PLIC_INT_ENABLE, 1 << PLIC_USB_ID);
    reg_write32(PLIC_PRIORITY + 4 * PLIC_USB_ID, 7);
    reg_write32(PLIC_THRESHOLD, 6);

    // Setup the IRQ handler entry point, set the mode to vectored
    csr_write_mtvec((uint_xlen_t) riscv_mtvec_table | RISCV_MTVEC_MODE_VECTORED);
    //csr_write_mtvec((uint_xlen_t) riscv_mtvec_table);

    // Enable MIE.MEI
    csr_set_bits_mie(MIE_MEI_BIT_MASK);

    // Global interrupt enable 
    csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
}
