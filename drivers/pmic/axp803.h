/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PRIVATE_AXP803_H
#define PRIVATE_AXP803_H

#if CONFIG_PMIC_AXP803

noreturn void axp803_reset(void);
void axp803_resume(void);
noreturn void axp803_shutdown(void);
void axp803_suspend(void);
bool axp803_irq(void);
void axp803_init(void);

#else

static inline void
axp803_reset(void)
{
}

static inline void
axp803_resume(void)
{
}

static inline void
axp803_shutdown(void)
{
}

static inline void
axp803_suspend(void)
{
}

static inline bool
axp803_irq(void)
{
	return false;
}

static inline void
axp803_init(void)
{
}

#endif

#endif /* PRIVATE_AXP803_H */
