/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <scpi.h>
#include <stdint.h>
#include <util.h>
#include <drivers/css/css.h>

/*
 * These generic functions provide a safe default implementation of the CSS
 * API, so platforms may implement power domain control functions
 * incrementally. Platforms should implement the getters before the setters.
 */

/**
 * Generic implementation used when no platform support is available. Because
 * the generic setters prevent changing the CSS state, the CSS must always be
 * running.
 */
weak uint8_t
css_get_css_state(void)
{
	/* Assume the CSS is always on. */
	return CSS_POWER_ON;
}

/**
 * Generic implementation used when no platform support is available. Assume
 * the minimum possible number of clusters is present.
 */
constfn weak uint8_t
css_get_cluster_count(void)
{
	/* Assume the CSS contains a single cluster with a single core. */
	return 1;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic setters prevent changing any cluster state, the set of running
 * clusters is always equal to the set of clusters initialized by the boot ROM.
 */
weak uint8_t
css_get_cluster_state(unused uint8_t cluster)
{
	/* Assume present clusters/cores are always on. */
	if (cluster >= css_get_cluster_count())
		return CSS_POWER_OFF;

	return CSS_POWER_ON;
}

/**
 * Generic implementation used when no platform support is available. Assume
 * the minimum possible number of cores is present in each cluster.
 */
constfn weak uint8_t
css_get_core_count(unused uint8_t cluster)
{
	/* Assume the CSS contains a single cluster with a single core. */
	return 1;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic setters prevent changing any core state, the set of running
 * cores is always equal to the set of cores initialized by the boot ROM.
 */
weak uint8_t
css_get_core_state(uint8_t cluster, uint8_t core)
{
	/* Assume present clusters/cores are always on. */
	if (cluster >= css_get_cluster_count())
		return CSS_POWER_OFF;
	if (core >= css_get_core_count(cluster))
		return CSS_POWER_OFF;

	return CSS_POWER_ON;
}

/*
 * There should usually be no reason to override this weak definition. It
 * correctly implements the algorithm specified in the CSS API using the
 * lower-level core state API. However, this definition is declared weak for
 * consistency with the other functions and flexibility for future platforms.
 */
weak uint8_t
css_get_online_cores(uint8_t cluster)
{
	uint8_t cores;
	uint8_t mask = 0;

	assert(cluster < css_get_cluster_count());

	cores = css_get_core_count(cluster);
	for (uint8_t core = 0; core < cores; ++core) {
		if (css_get_core_state(cluster, core) != CSS_POWER_OFF)
			mask |= BIT(core);
	}

	return mask;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic code does not know how to control the hardware, prohibit changes
 * to the CSS state by default without a platform-specific implementation.
 */
weak int
css_set_css_state(uint8_t state)
{
	/* Reject any attempt to change CSS, cluster, or core power states. */
	return state == css_get_css_state() ? SCPI_OK : SCPI_E_SUPPORT;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic code does not know how to control the hardware, prohibit changes
 * to the cluster state by default without a platform-specific implementation.
 */
weak int
css_set_cluster_state(uint8_t cluster, uint8_t state)
{
	/* Reject any attempt to change CSS, cluster, or core power states. */
	return state == css_get_cluster_state(cluster)
	       ? SCPI_OK
	       : SCPI_E_SUPPORT;
}

/**
 * Generic implementation used when no platform support is available. Because
 * the generic code does not know how to control the hardware, prohibit changes
 * to the core state by default without a platform-specific implementation.
 */
weak int
css_set_core_state(uint8_t cluster, uint8_t core, uint8_t state)
{
	/* Reject any attempt to change CSS, cluster, or core power states. */
	return state == css_get_core_state(cluster, core)
	       ? SCPI_OK
	       : SCPI_E_SUPPORT;
}
