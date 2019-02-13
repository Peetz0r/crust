/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef BITMAP_H
#define BITMAP_H

#include <limits.h>
#include <mmio.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

/**
 * Calculate a bit index from a byte offset and a bit offset within that byte.
 * Note that the bit offset can be greater than 8. In that case the bit will be
 * in a following byte (this is useful for e.g. 32-bit registers).
 *
 * @param byte The offset of the byte from the beginning of the bitmap.
 * @param bit  The offset (arithmetic shift) of the bit within the byte.
 */
#define BITMAP_INDEX(byte, bit) (CHAR_BIT * (byte) + (bit))

/**
 * Extract the word offset from a bitmap index.
 *
 * @param index An index into a bitmap, in bits.
 */
#define BITMAP_WORD(index)      ((index) / WORD_BIT)

/**
 * Extract the bit offset (in the word) from a bitmap index.
 *
 * @param index An index into a bitmap, in bits.
 */
#define BITMAP_BIT(index)       ((index) % WORD_BIT)

/**
 * Create a mask for the bit offset of a bitmap index.
 *
 * @param index An index into a bitmap, in bits.
 */
#define BITMAP_MASK(index)      BIT(BITMAP_BIT(index))

/**
 * Clear a bit in a bitmap.
 *
 * @param base  The address of the start of the bitmap.
 * @param index The index into the bitmap (in bits) of the bit to clear.
 */
static inline void
bitmap_clear(uintptr_t base, uint32_t index)
{
	uintptr_t addr = base + sizeof(uintptr_t) * BITMAP_WORD(index);

	mmio_clr_32(addr, BITMAP_MASK(index));
}

/**
 * Get a bit in a bitmap.
 *
 * @param base  The address of the start of the bitmap.
 * @param index The index into the bitmap (in bits) of the bit to get.
 */
static inline bool
bitmap_get(uintptr_t base, uint32_t index)
{
	uintptr_t addr = base + sizeof(uintptr_t) * BITMAP_WORD(index);

	return mmio_get_32(addr, BITMAP_MASK(index));
}

/**
 * Set a bit in a bitmap.
 *
 * @param base  The address of the start of the bitmap.
 * @param index The index into the bitmap (in bits) of the bit to set.
 */
static inline void
bitmap_set(uintptr_t base, uint32_t index)
{
	uintptr_t addr = base + sizeof(uintptr_t) * BITMAP_WORD(index);

	mmio_set_32(addr, BITMAP_MASK(index));
}

#endif /* BITMAP_H */
