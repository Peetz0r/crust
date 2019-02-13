/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <delay.h>
#include <interrupts.h>
#include <mmio.h>
#include <stdint.h>
#include <system.h>
#include <drivers/css/css.h>
#include <drivers/gpio/r_pio.h>
#include <drivers/pmic/pmic.h>
#include <drivers/watchdog/r_twd.h>

volatile uint32_t system_state;

void
system_boot_complete(void)
{
	assert(system_state == SYSTEM_BOOT);

	system_state = SYSTEM_RUNTIME;
}

void
system_shutdown(void)
{
	uint32_t flags = disable_interrupts();

	/* If the system is already off, do nothing. */
	if (system_state == SYSTEM_OFF)
		return;

	/* Mark the system as being suspended. */
	system_state = SYSTEM_SUSPEND;

	/* State management is done, so resume handling interrupts. */
	restore_interrupts(flags);

	/* Try to shutdown the system using the PMIC. */
	pmic_shutdown();

	/* If the firmware is still running, enter fake "off" state. */
	system_state = SYSTEM_OFF;
}

noreturn void
system_reset(void)
{
	/* Try to reset the system using the PMIC. */
	pmic_reset();

	/* Reset the SoC using the trusted watchdog (1 clock cycle). */
	r_twd_set_interval(1);
	r_twd_restart();
	udelay(1);

	/* In case this fails for some reason, reset the firmware. */
	reset();
}

void
system_suspend(void)
{
	uint32_t flags = disable_interrupts();

	/* If the system is already suspended, do nothing. */
	if (system_state == SYSTEM_SUSPEND)
		return;

	/* Mark the system as being suspended. */
	system_state = SYSTEM_SUSPEND;

	/* State management is done, so resume handling interrupts. */
	restore_interrupts(flags);

	/* Turn off unnecessary devices and enable wakeup sources. */
	r_pio_enable_wakeup();
	pmic_suspend();
}

void
system_resume(void)
{
	uint32_t flags = disable_interrupts();

	/* Reset the system if resuming from a fake "off" state. */
	if (system_state == SYSTEM_OFF)
		system_reset();
	/* If the system is already running, do nothing. */
	if (system_state != SYSTEM_SUSPEND)
		return;

	/* Mark the system as no longer being suspended. */
	system_state = SYSTEM_RUNTIME;

	/* State management is done, so resume handling interrupts. */
	restore_interrupts(flags);

	/* Turn devices back on. */
	pmic_resume();
	r_pio_disable_wakeup();

	/* Resume execution on the CSS. */
	css_set_css_state(CSS_POWER_ON);
	css_set_cluster_state(0, CSS_POWER_ON);
	css_set_core_state(0, 0, CSS_POWER_ON);
}
