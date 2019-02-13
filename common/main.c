/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <exception.h>
#include <irqf.h>
#include <scpi.h>
#include <spr.h>
#include <stdbool.h>
#include <stdint.h>
#include <system.h>
#include <drivers/clock/ccu.h>
#include <drivers/clock/r_prcm.h>
#include <drivers/css/css.h>
#include <drivers/gpio/r_pio.h>
#include <drivers/irqchip/r_intc.h>
#include <drivers/msgbox/msgbox.h>
#include <drivers/pmic/pmic.h>
#include <drivers/rsb/r_rsb.h>
#include <drivers/watchdog/r_twd.h>

/**
 * Main entry point. Performs setup and then enters an event loop.
 */
noreturn void
main(uint32_t exception)
{
	if (exception <= TRAP_EXCEPTION) {
		debugSXSX("Reset by exception ", exception, " at ",
		          mfspr(SPR_SYS_EPCR_INDEX(0)));
	}

	/* Initialize RTC block hardware devices. */
	r_prcm_init();
	r_twd_init();
	r_intc_init();
	r_pio_init();
	r_rsb_init();

	/* Initialize normal block hardware devices. */
	ccu_init();
	msgbox_init();

	/* Initialize external hardware devices. */
	pmic_init();

	/* Initialize the system power state machine. Assume that if the system
	 * is already booted, some secondary CPUs will have been turned on. */
	system_state = css_get_online_cores(0) == 1
	               ? SYSTEM_BOOT
	               : SYSTEM_RUNTIME;

	/* Initialize services. */
	irqf_init();
	scpi_init();

	do {
		r_twd_restart();
	} while (true);
}
