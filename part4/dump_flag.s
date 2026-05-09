/*
 * Machine-mode shellcode: dump flag bytes from CSRs 0xFC0..0xFD9 (csr_file.mem words 4033..4058).
 * Serial TX: csrw 0x202, byte (see defines_asm.h CSR_SERIAL_IO_OUT).
 */
.equ CSR_SERIAL_IO_OUT, 0x202

.section .text
.global _start
_start:
.macro dump1 n
	csrr a0, \n
	andi a0, a0, 0xff
	beqz a0, skip\@
	csrw CSR_SERIAL_IO_OUT, a0
skip\@:
.endm

	dump1 0xfc0
	dump1 0xfc1
	dump1 0xfc2
	dump1 0xfc3
	dump1 0xfc4
	dump1 0xfc5
	dump1 0xfc6
	dump1 0xfc7
	dump1 0xfc8
	dump1 0xfc9
	dump1 0xfca
	dump1 0xfcb
	dump1 0xfcc
	dump1 0xfcd
	dump1 0xfce
	dump1 0xfcf
	dump1 0xfd0
	dump1 0xfd1
	dump1 0xfd2
	dump1 0xfd3
	dump1 0xfd4
	dump1 0xfd5
	dump1 0xfd6
	dump1 0xfd7
	dump1 0xfd8
	dump1 0xfd9
endi:
	j endi
