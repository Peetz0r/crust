/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PRIVATE_CCU_H
#define PRIVATE_CCU_H

struct ccu_clock {
	uint16_t gate;
	uint16_t reset;
};

void ccu_enable_one_clock(uintptr_t base, const struct ccu_clock *clock);

#endif /* PRIVATE_CCU_H */
