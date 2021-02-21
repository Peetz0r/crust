/*
 * Copyright © 2019-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef CSS_PRIVATE_H
#define CSS_PRIVATE_H

#include <stdbool.h>
#include <stdint.h>
#include <platform/css.h>

struct power_state {
	uint8_t core[MAX_CLUSTERS][MAX_CORES_PER_CLUSTER];
	uint8_t cluster[MAX_CLUSTERS];
	uint8_t css;
};

extern struct power_state power_state;

/**
 * Get the number of cores present in a cluster.
 *
 * The number returned cannot be greater than 8.
 *
 * @param cluster The index of the cluster.
 */
uint32_t css_get_core_count(uint32_t cluster) ATTRIBUTE(const);

/**
 * Raise the state of the compute subsystem (CSS). The new state will always be
 * shallower than the previous state, and at least as shallow as the shallowest
 * cluster state in the CSS.
 *
 * @param old_state The previous coordinated state for the CSS.
 * @param new_state The new, shallower coordinated state for the CSS.
 */
void css_raise_css_state(uint32_t old_state, uint32_t new_state);

/**
 * Lower the state of the compute subsystem (CSS). The new state will always be
 * deeper than the previous state, and at least as shallow as the shallowest
 * cluster state in the CSS.
 *
 * Note: Lowering the state only makes sense if the previous state was "on".
 *
 * @param new_state The new, deeper coordinated state for the CSS.
 */
void css_lower_css_state(uint32_t new_state);

/**
 * Raise the state of a cluster. The new state will always be shallower than
 * the previous state, and at least as shallow as the shallowest core state in
 * this cluster.
 *
 * @param cluster   The index of the cluster.
 * @param old_state The previous coordinated state for this cluster.
 * @param new_state The new, shallower coordinated state for this cluster.
 */
void css_raise_cluster_state(uint32_t cluster, uint32_t old_state,
                             uint32_t new_state);

/**
 * Lower the state of a cluster. The new state will always be deeper than
 * the previous state, and at least as shallow as the shallowest core state in
 * this cluster.
 *
 * Note: Lowering the state only makes sense if the previous state was "on".
 *
 * @param cluster   The index of the cluster.
 * @param new_state The new, deeper coordinated state for this cluster.
 */
void css_lower_cluster_state(uint32_t cluster, uint32_t new_state);

/**
 * Raise the state of a CPU core. The new state will always be shallower than
 * the previous state.
 *
 * @param cluster   The index of the cluster.
 * @param core      The index of the core within the cluster.
 * @param old_state The previous coordinated state for this core.
 * @param new_state The new, shallower coordinated state for this core.
 */
void css_raise_core_state(uint32_t cluster, uint32_t core, uint32_t old_state,
                          uint32_t new_state);

/**
 * Lower the state of a CPU core. The new state will always be deeper than
 * the previous state.
 *
 * Note: Lowering the state only makes sense if the previous state was "on".
 *
 * @param cluster   The index of the cluster.
 * @param core      The index of the core within the cluster.
 * @param new_state The new, deeper coordinated state for this core.
 */
void css_lower_core_state(uint32_t cluster, uint32_t core, uint32_t new_state);

/**
 * Enable or disable power to a core or cluster power domain.
 *
 * When enabling a power switch, the power domain will be turned on gradually
 * to minimize inrush current and voltage drops.
 *
 * The mapping of cores/clusters to register addresses is platform-dependent.
 *
 * @param addr    The address of the register controlling the power switch.
 * @param enable  Whether to enable or disable the power switch.
 */
void css_set_power_switch(uintptr_t addr, bool enable);

#endif /* CSS_PRIVATE_H */
