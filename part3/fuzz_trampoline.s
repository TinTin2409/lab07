/*
 * Trampoline to execute one 32-bit instruction from a writable buffer, then return.
 * Used for custom-0 fuzzing and for the user-mode x10 search.
 */
.include "defines_asm.h"

.section .text
.global fuzz_exec_tail_init
.global exec_custom_insn
.global insn_buffer

/*
 * Writes the fixed jalr return slot once (saves writes on every fuzz iteration).
 */
fuzz_exec_tail_init:
    la t0, insn_buffer
    li t1, 0x00008067        /* jalr x0, ra, 0 */
    sw t1, 4(t0)
    ret

/*
 * void exec_custom_insn(uint32_t insn)
 * Places insn at insn_buffer[0]; slot at +4 must already hold jalr x0, ra, 0
 * (call fuzz_exec_tail_init once at boot).
 */
exec_custom_insn:
    la t0, insn_buffer
    sw a0, 0(t0)
    la ra, 1f
    jalr x0, t0, 0
1:
    ret

.section .bss
.align 4
insn_buffer:
    .space 8
