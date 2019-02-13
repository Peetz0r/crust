/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <barrier.h>
#include <spr.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Globally disable the external interrupt exception, with acquire semantics.
 * The return value is the previous state of the SPR and must be passed to
 * restore_interrupts() at the end of the critical section.
 */
static inline uint32_t
disable_interrupts(void)
{
	uint32_t flags = mfspr(SPR_SYS_SR_ADDR);

	mtspr(SPR_SYS_SR_ADDR, flags & ~SPR_SYS_SR_IEE_MASK);
	barrier();

	return flags;
}

/**
 * Globally enable the external interrupt exception, with release semantics.
 * This function should not be used to end a critical section; use
 * restore_interrupts() instead.
 */
static inline void
enable_interrupts(void)
{
	barrier();
	mtspr(SPR_SYS_SR_ADDR, SPR_SYS_SR_IEE_SET(mfspr(SPR_SYS_SR_ADDR), 1));
}

/**
 * Restore the state of the supervision register, with a full barrier. This
 * function should be used to end a critical section, and its argument must
 * come from the preceding call to disable_interrupts().
 *
 * @param flags The register contents returned by disable_interrupts().
 */
static inline void
restore_interrupts(uint32_t flags)
{
	barrier();
	mtspr(SPR_SYS_SR_ADDR, flags);
	barrier();
}

#endif /* INTERRUPTS_H */
