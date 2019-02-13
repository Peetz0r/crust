/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitmap.h>
#include <stddef.h>

#include "ccu.h"

void
ccu_enable_one_clock(uintptr_t base, const struct ccu_clock *clock)
{
	if (clock->gate)
		bitmap_set(base, clock->gate);
	if (clock->reset)
		bitmap_set(base, clock->reset);
}
