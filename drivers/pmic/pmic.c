/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <stdbool.h>
#include <drivers/irqchip/r_intc.h>
#include <drivers/pmic/pmic.h>

#include "axp803.h"

void
pmic_reset(void)
{
	axp803_reset();
}

void
pmic_resume(void)
{
	axp803_resume();
}

void
pmic_shutdown(void)
{
	axp803_shutdown();
}

void
pmic_suspend(void)
{
	axp803_suspend();
}

bool
pmic_irq(void)
{
	if (axp803_irq())
		return true;
	return false;
}

void
pmic_init(void)
{
	axp803_init();

	/* Set the NMI IRQ to level triggered, active low. */
	r_intc_configure_nmi(NMI_LOW_LEVEL);

	/* Enable the IRQ. */
	r_intc_enable_irq(IRQ_NMI);
}
