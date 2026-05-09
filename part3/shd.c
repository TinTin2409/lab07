/*
 * Pretty Secure System
 * Joseph Ravichandran
 * UIUC Senior Thesis Spring 2021
 * MIT Secure Hardware Design Spring 2023
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

#include "utils.h"
#include "shd.h"
#include "serial.h"
#include "defines.h"
#include "exception.h"

#ifndef PART3_RUN_EXCEPTION_TEST
#define PART3_RUN_EXCEPTION_TEST 0
#endif

extern void exec_custom_insn(uint32_t insn);
extern void fuzz_exec_tail_init(void);
extern void run_user_x10_fuzz(void);

#ifndef PART3_CUSTOM0_PROGRESS_INTERVAL
#define PART3_CUSTOM0_PROGRESS_INTERVAL 256
#endif

#ifndef PART3_WARMUP_PROGRESS_TRIALS
/* Print every trial for secrets [0,N) so stalls right after banner are noticeable. */
#define PART3_WARMUP_PROGRESS_TRIALS 4
#endif

/* Optional sanity check from exception_test.s; never returns after pass/fail paths. */
extern void exception_test(void);

/* Writable opcode used by fuzz (also read from user-mode assembly). */
uint32_t g_backdoor_opcode;

__attribute__((noreturn)) void fuzz_report_success(uint32_t unlock_x10) {
    printf("Hidden instruction opcode: 0x%08X\n", g_backdoor_opcode);
    printf("Backdoor unlock x10: 0x%08X\n", unlock_x10);
    for (;;) {
        /* halted */
    }
}

__attribute__((noreturn)) void fuzz_report_failure(void) {
    printf("No x10 in [0, 65535] unlocked privileged CSR access.\n");
    for (;;) {
    }
}

#define OP_CUSTOM0 0x0BU

/*
 * Encode a custom-0 instruction with upper immediate secret in bits [31:12].
 */
static uint32_t encode_instruction(uint32_t secret_hi20) {
    return OP_CUSTOM0 | (secret_hi20 << 12);
}

/*
 * shd_main()
 * Entrypoint for all MIT secure hardware design lab code.
 * Once we have reached this point, software-independent bringup
 * is fully complete and the CPU is ready to run the lab code.
 */
void shd_main(void) {
    printf("+------------------+\n");
    printf("| MIT SHD Fuzz Lab |\n");
    printf("|      Part 3      |\n");
    printf("+------------------+\n");

#if PART3_RUN_EXCEPTION_TEST
    printf("Sanity-check: exception_test (will not return).\n");
    exception_test();
#endif

    exception_handler_set_verbose(0);

    printf(
        "Scanning custom-0 encodings (secret 0..65535). Verilator-style sims are slow; "
        "this phase often takes tens of minutes to hours.\n"
    );

    printf("[part3] sweep start — if nothing appears below, simulator is stuck before first insn.\n");

    fuzz_exec_tail_init();

    uint32_t found_encoding = 0;
    uint32_t found_secret = 0;
    uint32_t backdoor_found = 0;

    for (uint32_t s = 0; s <= 65535u; s++) {
        if (s == 0) {
            printf("[part3] running first custom-0 probe (slow on Verilator/WSL).\n");
        }

        asm volatile("" ::: "memory");
        g_illegal_inst_trap_occurred = 0;
        g_last_trap_mcause = 0;

        asm volatile("" ::: "memory");
        exec_custom_insn(encode_instruction(s));
        asm volatile("" ::: "memory");

        /* Progress after completing trial `s`; first interval was unreachable while s!=0 guarded s=0 */
        int show = 0;
        if (s < (uint32_t)PART3_WARMUP_PROGRESS_TRIALS) {
            show = 1;
        } else if (PART3_CUSTOM0_PROGRESS_INTERVAL > 0 &&
                   (s % (uint32_t)PART3_CUSTOM0_PROGRESS_INTERVAL) == 0) {
            show = 1;
        }
        if (show) {
            printf(
                "[part3] trial done secret=0x%X (%u/65536) trap=%u last_mcause_if_trap=0x%X\n",
                s,
                s + 1u,
                g_illegal_inst_trap_occurred,
                g_last_trap_mcause
            );
        }

        if (!g_illegal_inst_trap_occurred) {
            found_encoding = encode_instruction(s);
            found_secret = s;
            backdoor_found = 1;
            printf(
                "No illegal-instruction trap (hidden custom-0): full word 0x%08X (secret 0x%X)\n",
                found_encoding, found_secret
            );
            break;
        }
    }

    if (!backdoor_found) {
        printf("No undocumented custom-0 instruction found for secrets 0..65535.\n");
        fuzz_report_failure();
    }

    g_backdoor_opcode = found_encoding;

    printf(
        "Searching x10 unlock in user mode (privileged CSR probe). Same note: "
        "up to 65536 iterations, can take a long time.\n"
    );
    exception_handler_set_verbose(0);

    asm volatile("" ::: "memory");
    run_user_x10_fuzz();
}
