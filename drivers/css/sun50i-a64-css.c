/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <mmio.h>
#include <scpi_protocol.h>
#include <stdint.h>
#include <platform/cpucfg.h>
#include <platform/prcm.h>

#include "css.h"

/* Reset Vector Base Address. */
static uint32_t rvba;

void
css_init(void)
{
	/* Program all cores to start in AArch64 mode. */
	mmio_set_32(C0_CTRL_REG0, C0_CTRL_REG0_AA64nAA32_MASK);
}

void
css_raise_css_state(uint32_t old_state, uint32_t new_state UNUSED)
{
	if (old_state == SCPI_CSS_OFF) {
		/* Deassert the CPU subsystem reset (active-low). */
		mmio_write_32(CPU_SYS_RESET_REG, CPU_SYS_RESET);
	}
}

void
css_lower_css_state(uint32_t new_state)
{
	if (new_state < SCPI_CSS_OFF)
		return;
	/* Assert the CPU subsystem reset (active-low). */
	mmio_write_32(CPU_SYS_RESET_REG, 0);
}

void
css_raise_cluster_state(uint32_t cluster UNUSED, uint32_t old_state,
                        uint32_t new_state UNUSED)
{
	if (old_state == SCPI_CSS_OFF) {
		/* Apply power to the cluster power domain. */
		css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(0), true);
		/* Release the cluster output clamps. */
		mmio_clr_32(C0_PWROFF_GATING_REG, C0_PWROFF_GATING);
		udelay(1);
		/* Deassert DBGPWRDUP for all cores. */
		mmio_write_32(DBG_REG0, 0);
		/* Assert all cluster and core resets (active-low). */
		mmio_write_32(C0_RST_CTRL_REG, 0);
		/* Program all cores to start in AArch64 mode. */
		mmio_set_32(C0_CTRL_REG0, C0_CTRL_REG0_AA64nAA32_MASK);
		/* Enable hardware L2 cache flush (active-low). */
		mmio_clr_32(C0_CTRL_REG0, C0_CTRL_REG0_L2RSTDISABLE);
		/* Put the cluster back into coherency (deassert ACINACTM). */
		mmio_clr_32(C0_CTRL_REG1, C0_CTRL_REG1_ACINACTM);
		/* Deassert all cluster resets (active-low). */
		mmio_write_32(C0_RST_CTRL_REG, C0_RST_CTRL_REG_MASK);
		/* Restore the reset vector base addresses for all cores. */
		for (uint32_t i = 0; i < css_get_core_count(cluster); ++i)
			mmio_write_32(RVBA_LO_REG(i), rvba);
	}
}

void
css_lower_cluster_state(uint32_t cluster UNUSED, uint32_t new_state)
{
	if (new_state < SCPI_CSS_OFF)
		return;
	/* Save the power-on reset vector base address from core 0. */
	rvba = mmio_read_32(RVBA_LO_REG(0));
	/* Assert L2FLUSHREQ to clean the cluster L2 cache. */
	mmio_set_32(C0_CTRL_REG2, C0_CTRL_REG2_L2FLUSHREQ);
	/* Wait for L2FLUSHDONE to go high. */
	mmio_poll_32(L2_STATUS_REG, L2_STATUS_REG_L2FLUSHDONE);
	/* Deassert L2FLUSHREQ. */
	mmio_clr_32(C0_CTRL_REG2, C0_CTRL_REG2_L2FLUSHREQ);
	/* Remove the cluster from coherency (assert ACINACTM). */
	mmio_set_32(C0_CTRL_REG1, C0_CTRL_REG1_ACINACTM);
	/* Wait for the cluster (L2 cache) to be idle. */
	mmio_poll_32(C0_CPU_STATUS_REG,
	             C0_CPU_STATUS_REG_STANDBYWFIL2);
	/* Assert all cluster resets (active-low). */
	mmio_write_32(C0_RST_CTRL_REG, 0);
	/* Activate the cluster output clamps. */
	mmio_set_32(C0_PWROFF_GATING_REG, C0_PWROFF_GATING);
	/* Remove power from the cluster power domain. */
	css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(0), false);
}

void
css_raise_core_state(uint32_t cluster UNUSED, uint32_t core,
                     uint32_t old_state, uint32_t new_state UNUSED)
{
	if (old_state == SCPI_CSS_OFF) {
		/* Assert core reset (active-low). */
		mmio_clr_32(C0_RST_CTRL_REG, C0_RST_CTRL_REG_nCORERESET(core));
		/* Assert core power-on reset (active-low). */
		mmio_clr_32(C0_PWRON_RESET_REG,
		            C0_PWRON_RESET_REG_nCPUPORESET(core));
		/* Core 0 does not have a separate power domain. */
		if (core > 0) {
			/* Turn on power to the core power domain. */
			css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(core),
			                     true);
			/* Release the core output clamps. */
			mmio_clr_32(C0_PWROFF_GATING_REG,
			            C0_CPUn_PWROFF_GATING(core));
		}
		/* Deassert core power-on reset (active-low). */
		mmio_set_32(C0_PWRON_RESET_REG,
		            C0_PWRON_RESET_REG_nCPUPORESET(core));
		/* Deassert core reset (active-low). */
		mmio_set_32(C0_RST_CTRL_REG, C0_RST_CTRL_REG_nCORERESET(core));
		/* Assert DBGPWRDUP (allow debug access to the core). */
		mmio_set_32(DBG_REG0, DBG_REG0_DBGPWRDUP(core));
	}
}

void
css_lower_core_state(uint32_t cluster UNUSED, uint32_t core,
                     uint32_t new_state)
{
	if (new_state < SCPI_CSS_OFF)
		return;
	/* Wait for the core to be in WFI and ready to shut down. */
	mmio_poll_32(C0_CPU_STATUS_REG,
	             C0_CPU_STATUS_REG_STANDBYWFI(core));
	/* Deassert DBGPWRDUP (prevent debug access to the core). */
	mmio_clr_32(DBG_REG0, DBG_REG0_DBGPWRDUP(core));
	/* Core 0 does not have a separate power domain. */
	if (core > 0) {
		/* Activate the core output clamps. */
		mmio_set_32(C0_PWROFF_GATING_REG,
		            C0_CPUn_PWROFF_GATING(core));
	}
	/* Assert core reset (active-low). */
	mmio_clr_32(C0_RST_CTRL_REG, C0_RST_CTRL_REG_nCORERESET(core));
	/* Assert core power-on reset (active-low). */
	mmio_clr_32(C0_PWRON_RESET_REG,
	            C0_PWRON_RESET_REG_nCPUPORESET(core));
	/* Core 0 does not have a separate power domain. */
	if (core > 0) {
		/* Remove power from the core power domain. */
		css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(core),
		                     false);
	}
}
