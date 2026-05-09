/*
 * Pretty Secure System
 * Joseph Ravichandran
 * UIUC Senior Thesis Spring 2021
 *
 * MIT License
 * Copyright (c) 2021-2023 Joseph Ravichandran
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "exception.h"
#include "defines.h"
#include "types.h"
#include "utils.h"

/* Communication from the exception handler back to fuzz / test harness */
volatile uint32_t g_illegal_inst_trap_occurred;
volatile uint32_t g_illegal_csr_access_trap;
volatile uint32_t g_last_trap_mcause;

static volatile int g_exception_handler_verbose = 1;

void exception_handler_set_verbose(int verbose) {
    g_exception_handler_verbose = verbose ? 1 : 0;
}

/*
 * exception_handler(mcause, saved_regs)
 * This is a C method that can be called by
 * exception_handler_entry to handle your exception state.
 *
 * mcause: The exception cause (value of the mcause CSR)
 * saved_regs: A pointer to the saved_regs_t representing this exception context.
 *
 * We assume that the caller of this function will read from saved_regs
 * to restore the CPU architectural state before returning from the exception with mret.
 */
void exception_handler(uint32_t mcause, saved_regs_t *saved_regs) {
    asm volatile("" ::: "memory");

    g_last_trap_mcause = mcause;

#ifdef PART3_TRAP_TRACE
    {
        static unsigned trace_count;
        if (trace_count < 32u) {
            printf("[trap] #%u mcause=0x%X\n", trace_count, mcause);
            trace_count++;
        }
    }
#endif

    /*
     * Only PSP external / local IPI are expected to use the mcause interrupt form for
     * real delivery. Anything else—even if RTL sets the interrupt MSB wrongly—still has
     * to advance mepc past a faulting insn or the fuzz loop spins forever.
     *
     * EXTERNAL / IPI use the canonical full-encoding mcause constants from defines.h.
     */
    if (mcause == EXCEPTION_CAUSE_EXTERNAL || mcause == EXCEPTION_CAUSE_IPI) {
        goto after_pc_patch;
    }

    {
        uint32_t eff = mcause & 0x7fffffffu;
        switch (eff) {
            case EXCEPTION_CAUSE_INVALID_INST:
                g_illegal_inst_trap_occurred = 1;
                saved_regs->mepc += 4;
                break;
            case EXCEPTION_CAUSE_ILLEGAL_ACCESS:
                g_illegal_csr_access_trap = 1;
                saved_regs->mepc += 4;
                break;
            default:
                /* Unknown synchronous (or miscoded) faults: skip insn; mark probes. */
                saved_regs->mepc += 4;
                g_illegal_inst_trap_occurred = 1;
                g_illegal_csr_access_trap = 1;
                break;
        }
    }

after_pc_patch:

    if (g_exception_handler_verbose) {
        printf("=============================\n");
        printf("Exception! mcause=0x%X\n", mcause);
        dump_regs(saved_regs);
    }

    asm volatile("" ::: "memory");
}


/*
 * dump_regs(saved_regs)
 * Prints all saved registers to the serial port.
 * Can be useful for debugging your exception handler!
 */
void dump_regs(saved_regs_t *saved_regs) {
    if (!saved_regs) return;

    printf("pc:  0x%0X\n", saved_regs->mepc);
    printf("x1:  0x%0X    ", saved_regs->x1);
    printf("x2:  0x%0X    ", saved_regs->x2);
    printf("x3:  0x%0X    ", saved_regs->x3);
    printf("x4:  0x%0X\n", saved_regs->x4);
    printf("x5:  0x%0X    ", saved_regs->x5);
    printf("x6:  0x%0X    ", saved_regs->x6);
    printf("x7:  0x%0X    ", saved_regs->x7);
    printf("x8:  0x%0X\n", saved_regs->x8);
    printf("x9:  0x%0X    ", saved_regs->x9);
    printf("x10: 0x%0X    ", saved_regs->x10);
    printf("x11: 0x%0X    ", saved_regs->x11);
    printf("x12: 0x%0X\n", saved_regs->x12);
    printf("x13: 0x%0X    ", saved_regs->x13);
    printf("x14: 0x%0X    ", saved_regs->x14);
    printf("x15: 0x%0X    ", saved_regs->x15);
    printf("x16: 0x%0X\n", saved_regs->x16);
    printf("x17: 0x%0X    ", saved_regs->x17);
    printf("x18: 0x%0X    ", saved_regs->x18);
    printf("x19: 0x%0X    ", saved_regs->x19);
    printf("x20: 0x%0X\n", saved_regs->x20);
    printf("x21: 0x%0X    ", saved_regs->x21);
    printf("x22: 0x%0X    ", saved_regs->x22);
    printf("x23: 0x%0X    ", saved_regs->x23);
    printf("x24: 0x%0X\n", saved_regs->x24);
    printf("x25: 0x%0X    ", saved_regs->x25);
    printf("x26: 0x%0X    ", saved_regs->x26);
    printf("x27: 0x%0X    ", saved_regs->x27);
    printf("x28: 0x%0X\n", saved_regs->x28);
    printf("x29: 0x%0X    ", saved_regs->x29);
    printf("x30: 0x%0X    ", saved_regs->x30);
    printf("x31: 0x%0X    \n", saved_regs->x31);
}
