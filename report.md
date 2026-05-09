## 1-1

**Answer.**

In `part1`:

- **`mret`** jumps into **`boot_main`** in `boot.c`. The target address comes from **`mepc`** (CSR **`0x341`**).
- **`mtvec`** is set in `bringup.s` so traps go to **`exception_handler_entry`** in **`exception_entry.s`** (the C side is **`exception_handler()`** in `exception.c`).
- Stack: in **`linker.ld`** it sits under **`.data`**, with **4096 bytes** (**`0x1000`**) reserved. The symbol **`_kernel_anonymous_stack`** marks the high end of the stack (stack growth is downward).
- **`saved_regs_t`** is defined in **`exception.h`**. **`asm_offsets.h`** is generated after `make`. **`mepc`**’s offset inside the struct is **124 bytes** (**`0x7C`**).
- RISC-V ABI: **`ra`** / **`x1`** holds the return address; **`sp`** / **`x2`** is the stack pointer.
- After **`mret`**, execution is still in **machine mode**, because **`CSR_MPP`** was set earlier to **`PSP_PRIV_MACHINE`** (**3**).

## 1-3

**Question.**

Based on the printf result, record what are the inputs (eg. values of mindistance and cost[nextnode][i]) that cause the addition instruction to return an incorrect sum? Does the operation fail for all inputs or just specific ones?

**Answer.**

A **`printf`** is inserted right before **`if (mindistance + cost[nextnode][i] < distance[i])`** in `dijkstra.c` to print those variables plus two notions of addition: **`cpu_sum`** (the value from the CPU **`+`** / add instruction) and **`bitwise_ref_sum`** (a bit-wide reference addition that does not trust the hardware adder). Printed values appear in **hex** via **`%x`**.

![Screenshot of `./run.sh part1` after adding debug prints](report-assets/part1-terminal.png)

Compared to the screenshot:

| Log line | What it shows |
|----------|----------------|
| `nextnode=1`, `cpu_sum=500`, `bitwise_ref_sum=500` | `0x400 + 0x100` matches on both sides, fine. |
| `nextnode=1`, both sides `2400` | `0x400 + 0x2000`, still fine. |
| `nextnode=2`, `i=3`: **`1337`** vs **`600`** | This is where it breaks. It should be `0x500 + 0x100 = 0x600`, but the CPU reports **`0x1337`**, so **`cpu_sum < distance[3]`** is false (**`distance[3]` is `0x1000`**) → Dijkstra does **not** relax the path through node **2**. |

The three **Distance / Path** lines at the bottom of the image match the faulty outcome: node **3** stays **`1000`**, path **`3<-0`**.

The incorrect sum appears for **`mindistance = 0x500`** and **`cost[nextnode][i] = 0x100`** (with **`nextnode = 2`**, **`i = 3`** in the log): the correct sum is **`0x600`**, but the CPU gives **`0x1337`**. The operation does **not** fail for all inputs—the first two log lines match on CPU vs reference; failures depend on the operand pair.

---

## 2-1

**Question.**

What is your approach for finding the operands that can trigger the bug with the add instruction? Briefly describe it.

**Answer.**

The fuzzer exhaustively tries every pair **`(a, b)`** with **`a`** and **`b`** in **`[0x100, 0x1FF]`** (inclusive), i.e. **256×256** combinations as required. For each pair it computes a **reference sum** using only **XOR, AND, and shifts** (carry propagation loop — same idea as Part 1’s bitwise add): this never uses the untrusted register–register **`add`** on the two operands. It compares that reference to **`a + b`**, which the compiler lowers to **`add`**. Any mismatch is printed in the required form **`0x%X+0x%X=0x%X,0x%X`**.

When **`./run.sh part2`** finishes, the shell returns to the prompt after printing every mismatch; **13** lines indicates all failing pairs in-range (six symmetric pairs × two orderings, plus **`0x1FC+0x1FC`** once).

![Screenshot of `./run.sh part2` (Part 2 fuzz output)](report-assets/part2-terminal.png)

## 2-3 (optional)

**Question.**

Did you encounter any bugs while implementing the fuzzer? How did you handle the possibility of add instructions being incorrect during control flow instructions (eg. loop condition checks)?

**Answer.**

No tooling bugs were hit for Part 2 beyond normal simulator slowness over **65k** pairs. **Loop indices** use **`for (a = …; a <= hi; a++)`** — **`a++`** / **`b++`** compile to **`addi`**, not register–register **`add`**, so iteration and address arithmetic for **`printf`** stay independent of the faulty **`add`**. The **only** deliberate use of **`add`** on the fuzz operands is **`cpu_sum = a + b`**, which is exactly the instruction under test. Comparisons use **`!=`** on the two **already computed** 32-bit words (typically **`xor`** / branch), not **`add`** on **`a`** and **`b`** again for control flow.

**Remaining report sections:** **3-x** and **4-x** (including flags **4-3** / **4-5**) require finishing Part 3 (`exception.c`, test harness in **`shd_main`**) and Part 4 (`solve_part4.py`, optional **`dump_flag`** shellcode, remote). Those are still mostly TODO in this repo.

## 3-2

**Question.**

Why does the exception handler restore x2 after all the other registers?

**Answer.**

## 3-3

**Question.**

Describe your design decisions for the exception handler. What does it do while trying to find the undocumented instruction? How does your exception handler communicate with the test code?

**Answer.**

## 3-5

**Question.**

Include the hidden backdoor instruction found by your code/script in the report.

**Answer.**

## 3-6

**Question.**

Describe your approach for finding the correct x10 value. Does the exception handler behave differently when searching for the correct x10 value of that instruction compared to searching for hidden instructions?

**Answer.**

## 3-8

**Question.**

Include the correct x10 value found by your code/script in the report.

**Answer.**

## 3-10 (optional)

**Question.**

Did you encounter any challenges while building the code? How did you overcome the challenges of the add instruction occasionally producing an incorrect result? Did you try anything that failed?

**Answer.**

## 4-3

**Question.**

Include this flag in your report.

**Answer.**

## 4-5

**Question.**

Include the flag leaked from the remote CPU in your report.

**Answer.**
