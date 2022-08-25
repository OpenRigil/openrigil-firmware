#include "plic.h"
#include "dcd_zenithal.h"
#include "mmio.h"

#include "interrupt.h"

// Default "NOP" implementations
static void riscv_nop_machine(void)    __attribute__ ((interrupt ("machine")));

#define NOT_WEAK

// Weak alias to the "NOP" implementations.
void riscv_mtvec_exception(void) __attribute__ ((interrupt ("machine") , alias("riscv_nop_machine"), __used__));
void riscv_mtvec_msi(void) __attribute__ ((interrupt ("machine")     , alias("riscv_nop_machine"), __used__));
void riscv_mtvec_mti(void) __attribute__ ((interrupt ("machine")     , alias("riscv_nop_machine"), __used__));
void riscv_mtvec_mei(void) __attribute__ ((interrupt ("machine"), __used__));

// rocket chip mtvec is 128byte aligned for rv32 and 256byte aligned for rv64
// used prevent LTO from removing it
void riscv_mtvec_table(void)  __attribute__ ((naked, section(".text.mtvec_table") ,aligned(256), __used__));

#pragma GCC push_options

void riscv_mtvec_table(void) {
    __asm__ volatile (
        ".org  riscv_mtvec_table + 0*4;"
        "jal   zero,riscv_mtvec_exception;"  /* 0  */   
        ".org  riscv_mtvec_table + 3*4;"
        "jal   zero,riscv_mtvec_msi;"  /* 3  */   
        ".org  riscv_mtvec_table + 7*4;"
        "jal   zero,riscv_mtvec_mti;"  /* 7  */   
        ".org  riscv_mtvec_table + 11*4;"
        "jal   zero,riscv_mtvec_mei;"  /* 11 */   
        : /* output: none */                    
        : /* input : immediate */               
        : /* clobbers: none */
        );
}

// Ensure all ISR functions are aligned.
#pragma GCC optimize ("align-functions=4")

static void riscv_nop_machine(void)  {
    // Nop machine mode interrupt.
}

void riscv_mtvec_mei(void) {
    uint32_t claim = reg_read32(PLIC_CLAIM);
    if (claim == PLIC_USB_ID) {
        dcd_handle_interrupt();
    }
    reg_write32(PLIC_COMPLETE, claim);
}

#pragma GCC pop_options

/////////////

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

void plic_enable(int id, int priority) {
    reg_write32(PLIC_INT_ENABLE, 1 << id);
    reg_write32(PLIC_PRIORITY + 4 * id, priority & 7);
}

void enable_interrupt(void)
{
    // Global interrupt disable
    csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);
    csr_write_mie(0);

    // enable plic usb
    plic_enable(PLIC_USB_ID, 7);
    reg_write32(PLIC_THRESHOLD, 6);

    // Setup the IRQ handler entry point, set the mode to vectored
    csr_write_mtvec((uint32_t) riscv_mtvec_table | RISCV_MTVEC_MODE_VECTORED);

    // Enable MIE.MEI
    csr_set_bits_mie(MIE_MEI_BIT_MASK);

    // Global interrupt enable 
    csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
}
