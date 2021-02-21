/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <mmio.h>
#include <scpi_protocol.h>
#include <stdint.h>
#include <platform/cpucfg.h>

#include "css.h"

#define CPUIDLE_DELAY 24 /* 24 us. */

/* Reset Vector Base Address. */
static uint32_t rvba;

static void
write_cpuidle_en_reg(uint32_t val)
{
	mmio_write_32(CPUIDLE_EN_REG, CPUIDLE_EN_REG_KEY0);
	mmio_write_32(CPUIDLE_EN_REG, CPUIDLE_EN_REG_KEY1 | val);
}

void
css_init(void)
{
	/* Program all cores to start in AArch64 mode. */
	mmio_set_32(C0_CTRL_REG0, C0_CTRL_REG0_AA64nAA32_MASK);
	/* Enable the CPUIDLE hardware. */
	write_cpuidle_en_reg(CPUIDLE_EN_REG_CPUIDLE_EN);
	/* Enable CPUIDLE wakeup for core 0. */
	mmio_write_32(CPUIDLE_WAKE_REG, CPUIDLE_WAKE_REG_CPUn_IRQ(0));
	/* Set CPUIDLE state transition delays. */
	mmio_write_32(PWR_SW_DELAY_REG, CPUIDLE_DELAY);
	mmio_write_32(CONFIG_DELAY_REG, CPUIDLE_DELAY);
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
	/* If CPUIDLE is already enabled, suspend was aborted. Do nothing. */
	if (mmio_get_32(CPUIDLE_EN_REG, CPUIDLE_EN_REG_CPUIDLE_EN))
		return;
	if (old_state >= SCPI_CSS_OFF_IDLE) {
		/* Deassert the cluster hard reset (active-low). */
		mmio_write_32(C0_PWRON_RESET_REG, C0_PWRON_RESET_REG_nH_RST);
		/* Deassert DBGPWRDUP for all cores. */
		mmio_write_32(DBG_REG0, 0);
		/* Assert all cluster and core resets (active-low). */
		mmio_write_32(C0_RST_CTRL_REG, 0);
		/* Program all cores to start in AArch64 mode. */
		mmio_set_32(C0_CTRL_REG0, C0_CTRL_REG0_AA64nAA32_MASK);
		/* Enable hardware L2 cache flush (active-low). */
		mmio_clr_32(C0_CTRL_REG0, C0_CTRL_REG0_L2RSTDISABLE);
		/* Deassert all cluster resets (active-low). */
		mmio_write_32(C0_RST_CTRL_REG, C0_RST_CTRL_REG_MASK);
		/* Restore the reset vector base addresses for all cores. */
		for (uint32_t i = 0; i < css_get_core_count(cluster); ++i)
			mmio_write_32(RVBA_LO_REG(i), rvba);
	}
	/* Put the cluster back into coherency (deassert ACINACTM). */
	mmio_clr_32(C0_CTRL_REG1, C0_CTRL_REG1_ACINACTM);
	/* Enable the CPUIDLE hardware. */
	write_cpuidle_en_reg(CPUIDLE_EN_REG_CPUIDLE_EN);
}

void
css_lower_cluster_state(uint32_t cluster UNUSED, uint32_t new_state)
{
	/* Disable the CPUIDLE hardware to prevent core wakeup from idle. */
	write_cpuidle_en_reg(0);
	/* If some CPU is not idle, refuse to suspend the cluster. */
	const uint32_t mask = CPUIDLE_STAT_REG_CPUn_OFF_MASK;

	if (mmio_get_32(CPUIDLE_STAT_REG, mask) != mask) {
		write_cpuidle_en_reg(CPUIDLE_EN_REG_CPUIDLE_EN);
		return;
	}
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
	if (new_state < SCPI_CSS_OFF_IDLE)
		return;
	/* Save the power-on reset vector base address from core 0. */
	rvba = mmio_read_32(RVBA_LO_REG(0));
	/* Assert all cluster resets (active-low). */
	mmio_write_32(C0_RST_CTRL_REG, 0);
	/* Assert all power-on resets (active-low). */
	mmio_write_32(C0_PWRON_RESET_REG, 0);
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
		/* Turn on power to the core power domain. */
		css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(core), true);
		/* Release the core output clamps. */
		mmio_clr_32(C0_PWROFF_GATING_REG, C0_CPUn_PWROFF_GATING(core));
		/* Deassert core power-on reset (active-low). */
		mmio_set_32(C0_PWRON_RESET_REG,
		            C0_PWRON_RESET_REG_nCPUPORESET(core));
		/* Deassert core reset (active-low). */
		mmio_set_32(C0_RST_CTRL_REG, C0_RST_CTRL_REG_nCORERESET(core));
		/* Assert DBGPWRDUP (allow debug access to the core). */
		mmio_set_32(DBG_REG0, DBG_REG0_DBGPWRDUP(core));
		/* Enable CPUIDLE wakeup for this core. */
		mmio_set_32(CPUIDLE_WAKE_REG, CPUIDLE_WAKE_REG_CPUn_IRQ(core));
	}
}

void
css_lower_core_state(uint32_t cluster UNUSED, uint32_t core,
                     uint32_t new_state)
{
	if (new_state < SCPI_CSS_OFF_IDLE)
		return;
	if (new_state >= SCPI_CSS_OFF) {
		/* Disable CPUIDLE wakeup for this core. */
		mmio_clr_32(CPUIDLE_WAKE_REG, CPUIDLE_WAKE_REG_CPUn_IRQ(core));
	}
	/* Trigger CPU power off. */
	mmio_write_32(CLOSE_FLAG_REG, CLOSE_FLAG_REG_CPUn(core));
	/* Wait for CPU power off to complete. */
	mmio_pollz_32(CLOSE_FLAG_REG, CLOSE_FLAG_REG_CPUn(core));
}
