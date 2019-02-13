/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <asm/macros.S>
#include <asm/spr.h>

func reset
	l.addi	r3, r0, 0		# Invalidate instruction cache
	l.addi	r4, r0, 4096		# Cache lines (256) * block size (16)
1:	l.mtspr	r0, r3, SPR_ICACHE_ICBIR_ADDR
	l.sfltu	r3, r4
	l.bf	1b
	l.addi	r3, r3, 16		# Cache block size
	l.psync				# Flush CPU pipeline
	l.mfspr	r3, r0, SPR_SYS_SR_ADDR	# Enable instruction cache
	l.ori	r3, r3, SPR_SYS_SR_ICE_MASK
	l.mtspr	r0, r3, SPR_SYS_SR_ADDR
	l.nop				# One cache block of nops
	l.nop
	l.nop
	l.nop
	l.movhi	r5, hi(__bss_start)	# Clear .bss
	l.ori	r3, r5, lo(__bss_start)
	l.ori	r4, r5, lo(__bss_end)
1:	l.sw    0(r3), r0
	l.sfltu r3, r4
	l.bf	1b
	l.addi	r3, r3, 4
	l.ori	r1, r5, lo(__stack_end)	# Initialize stack pointer
	l.j	main			# Jump to C entry point
	l.or	r3, r2, r2		# First argument is exception number
endfunc reset
