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

#include "fuzz.h"
#include "types.h"
#include "utils.h"

/* True 32-bit sum without using the `+` operator (avoids a single buggy `add`). */
static uint32_t reliable_add_u32(uint32_t a, uint32_t b) {
	while (b != (uint32_t)0) {
		uint32_t carry = (a & b) << 1;
		a ^= b;
		b = carry;
	}
	return a;
}

void part2_fuzz_inputs(void) {
	const uint32_t lo = 0x100;
	const uint32_t hi = 0x1FF;

	for (uint32_t a = lo; a <= hi; a++) {
		for (uint32_t b = lo; b <= hi; b++) {
			uint32_t expected = reliable_add_u32(a, b);
			uint32_t cpu_sum = a + b;
			if (cpu_sum != expected) {
				printf("0x%X+0x%X=0x%X,0x%X\n", a, b, cpu_sum, expected);
			}
		}
	}
}
