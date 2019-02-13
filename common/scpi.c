/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <barrier.h>
#include <debug.h>
#include <interrupts.h>
#include <platform.h>
#include <scpi.h>
#include <stdbool.h>
#include <stdint.h>
#include <system.h>
#include <util.h>
#include <drivers/css/css.h>
#include <drivers/msgbox/msgbox.h>
#include <drivers/watchdog/r_twd.h>

extern struct scpi_mem __scpi_mem[SCPI_CLIENTS];

/* The shared memory areas are in reverse order from the mailbox channels. */
#define SCPI_MEM_AREA(n) (__scpi_mem[SCPI_CLIENTS - n - 1])

/* Commands come on even channels; replies are sent on the next odd channel. */
#define TX_CHAN(client)  ((client) * 2 + 1)

enum {
	/** Do not send a reply to this command. */
	FLAG_NO_REPLY = BIT(0),
	/** Reject this command from the non-secure client. */
	FLAG_SECURE   = BIT(1),
};

struct scpi_cmd {
	/** Handler that can process a message and create a reply. */
	uint32_t      (*handler)(struct scpi_msg *rx_msg,
	                         struct scpi_msg *tx_msg);
	/** Expected size of received payload. */
	const uint8_t rx_size;
	/** Any combination of flags from above, as applicable. */
	const uint8_t flags;
};

/**
 * Handler for SCPI_CMD_SCP_READY: SCP ready.
 */
static uint32_t
scpi_scp_ready(unused struct scpi_msg *rx_msg, unused struct scpi_msg *tx_msg)
{
	/* Once ATF replies to "SCP ready", consider the boot complete. */
	system_boot_complete();

	return SCPI_OK;
}

/**
 * Handler for SCPI_CMD_GET_SCP_CAP: Get SCP capability.
 */
static uint32_t
scpi_get_scp_cap(unused struct scpi_msg *rx_msg, struct scpi_msg *tx_msg)
{
	/* SCPI protocol version (major, minor). */
	tx_msg->payload[0] = 1 << 16 | 2;
	/* Payload size limits (receive, transmit). */
	tx_msg->payload[1] = SCPI_PAYLOAD_SIZE << 16 | SCPI_PAYLOAD_SIZE;
	/* Firmware version (major, minor, patch). */
	tx_msg->payload[2] = 0 << 24 | 1 << 16 | 0;
	/* Commands enabled 0. */
	tx_msg->payload[3] = BIT(SCPI_CMD_SCP_READY) |
	                     BIT(SCPI_CMD_GET_SCP_CAP) |
	                     BIT(SCPI_CMD_SET_CSS_PWR) |
	                     BIT(SCPI_CMD_GET_CSS_PWR) |
	                     BIT(SCPI_CMD_SET_SYS_PWR);
	/* Commands enabled 1. */
	tx_msg->payload[4] = 0;
	/* Commands enabled 2. */
	tx_msg->payload[5] = 0;
	/* Commands enabled 3. */
	tx_msg->payload[6] = 0;

	tx_msg->size = 7 * sizeof(uint32_t);

	return SCPI_OK;
}

/**
 * Handler for SCPI_CMD_SET_CSS_PWR: Set CSS power state. This function sets
 * the power state of a single core, its parent cluster, and the CSS.
 *
 * The power state provided by PSCI is already coordinated. Simply turn things
 * on from highest to lowest power level, then turn things off from lowest to
 * highest power level. This ensures no power domain is turned on before its
 * parent, and no power domain is turned off before any of its children.
 */
static uint32_t
scpi_set_css_pwr(struct scpi_msg *rx_msg, unused struct scpi_msg *tx_msg)
{
	uint32_t descriptor    = rx_msg->payload[0];
	uint8_t  core          = (descriptor >> 0x00) & GENMASK(3, 0);
	uint8_t  cluster       = (descriptor >> 0x04) & GENMASK(3, 0);
	uint8_t  core_state    = (descriptor >> 0x08) & GENMASK(3, 0);
	uint8_t  cluster_state = (descriptor >> 0x0c) & GENMASK(3, 0);
	uint8_t  css_state     = (descriptor >> 0x10) & GENMASK(3, 0);
	int ret;

	if (css_state == CSS_POWER_ON) {
		ret = css_set_css_state(css_state);
		if (ret != SCPI_OK)
			return ret;
	}
	if (cluster_state == CSS_POWER_ON) {
		ret = css_set_cluster_state(cluster, cluster_state);
		if (ret != SCPI_OK)
			return ret;
	}
	if ((ret = css_set_core_state(cluster, core, core_state)) != SCPI_OK)
		return ret;
	if (cluster_state != CSS_POWER_ON) {
		ret = css_set_cluster_state(cluster, cluster_state);
		if (ret != SCPI_OK)
			return ret;
	}
	if (css_state != CSS_POWER_ON) {
		ret = css_set_css_state(css_state);
		if (ret != SCPI_OK)
			return ret;
	}
	/* Turning everything off means system suspend. */
	if (css_state == CSS_POWER_OFF)
		system_suspend();

	return SCPI_OK;
}

/*
 * Handler for SCPI_CMD_GET_CSS_PWR: Get CSS power state.
 *
 * This gets the power states of all clusters and all cores they contain.
 */
#define CLUSTER_ID(x)          ((x) & GENMASK(3, 0))
#define CLUSTER_POWER_STATE(x) (((x) & GENMASK(3, 0)) << 4)
#define CORE_POWER_STATES(x)   ((x) << 8)
static uint32_t
scpi_get_css_pwr(unused struct scpi_msg *rx_msg, struct scpi_msg *tx_msg)
{
	uint8_t  clusters = css_get_cluster_count();
	uint16_t descriptor;

	/* Each cluster has its own power state descriptor. */
	for (uint8_t i = 0; i < clusters; ++i) {
		descriptor = CLUSTER_ID(i) |
		             CLUSTER_POWER_STATE(css_get_cluster_state(i)) |
		             CORE_POWER_STATES(css_get_online_cores(i));
		/* Work around the hardware byte swapping, since this is an
		 * array of elements each aligned to less than 4 bytes. */
		((uint16_t *)tx_msg->payload)[i ^ 1] = descriptor;
	}

	tx_msg->size = clusters * sizeof(descriptor);

	return SCPI_OK;
}

/**
 * Handler for SCPI_CMD_SET_SYS_PWR: Set system power state.
 */
static uint32_t
scpi_set_sys_pwr(struct scpi_msg *rx_msg, unused struct scpi_msg *tx_msg)
{
	uint8_t requested_state = rx_msg->payload[0];

	if (requested_state == SCPI_SYSTEM_POWER_REBOOT ||
	    requested_state == SCPI_SYSTEM_POWER_RESET)
		system_reset();

	if (requested_state == SCPI_SYSTEM_POWER_SHUTDOWN) {
		system_shutdown();
		return SCPI_OK;
	}

	return SCPI_E_PWRSTATE;
}

/**
 * The list of supported SCPI commands.
 */
static const struct scpi_cmd scpi_cmds[] = {
	[SCPI_CMD_SCP_READY] = {
		.handler = scpi_scp_ready,
		.flags   = FLAG_NO_REPLY | FLAG_SECURE,
	},
	[SCPI_CMD_GET_SCP_CAP] = {
		.handler = scpi_get_scp_cap,
	},
	[SCPI_CMD_SET_CSS_PWR] = {
		.handler = scpi_set_css_pwr,
		.rx_size = sizeof(uint32_t),
		.flags   = FLAG_NO_REPLY | FLAG_SECURE,
	},
	[SCPI_CMD_GET_CSS_PWR] = {
		.handler = scpi_get_css_pwr,
	},
	[SCPI_CMD_SET_SYS_PWR] = {
		.handler = scpi_set_sys_pwr,
		.rx_size = sizeof(uint8_t),
		.flags   = FLAG_SECURE,
	},
};

/**
 * Wait for a previous transmission to be acknowledged, with a timeout.
 *
 * Acquire semantics on the shared memory buffer.
 */
static void
scpi_wait_txdone(uint8_t client)
{
	uint32_t timeout = r_twd_read_counter() + 100 * CLK_KHZ;
	bool last_tx_done;

	/* If this loop times out, either the client is _really_ slow, or it is
	 * dead, or it didn't care about the last message. If the client didn't
	 * read our last reply in 100 ms, then it's probably not going to read
	 * it while we are writing the new reply. While we could drain the
	 * FIFO, that still doesn't solve the race, so don't bother with it. */
	while (r_twd_read_counter() < timeout) {
		last_tx_done = msgbox_last_tx_done(TX_CHAN(client));
		if (last_tx_done)
			break;
	}
	barrier();

	if (!last_tx_done) {
		debugSX("Dropped SCPI command ",
		        SCPI_MEM_AREA(client).tx_msg.command);
	}
}

/**
 * Send a prepared SCPI message, ensuring it is first flushed to memory.
 *
 * Release semantics on the shared memory buffer.
 */
static void
scpi_send_message(uint8_t client)
{
	barrier();
	msgbox_send(TX_CHAN(client), SCPI_VIRTUAL_CHANNEL);
}

/**
 * Create and send an SCPI message. This function disables interrupts to
 * prevent an incoming SCPI message from using the TX buffer.
 */
static void
scpi_create_message(uint8_t client, uint8_t command)
{
	struct scpi_msg *tx_msg = &SCPI_MEM_AREA(client).tx_msg;
	uint32_t flags = disable_interrupts();

	/* Try to ensure the TX buffer is free before writing to it. */
	scpi_wait_txdone(client);

	/* Create the message header. */
	tx_msg->command = command;
	tx_msg->sender  = SCPI_SENDER_SCP;
	tx_msg->size    = 0;
	tx_msg->status  = SCPI_OK;

	scpi_send_message(client);

	restore_interrupts(flags);
}

/**
 * Generic SCPI command handling function.
 */
static void
scpi_handle_message(uint8_t client)
{
	struct scpi_msg *rx_msg = &SCPI_MEM_AREA(client).rx_msg;
	struct scpi_msg *tx_msg = &SCPI_MEM_AREA(client).tx_msg;
	const struct scpi_cmd *cmd;

	/* Try to ensure the TX buffer is free before writing to it. */
	scpi_wait_txdone(client);

	/* Initialize the response (defaults for unsupported commands). */
	tx_msg->command = rx_msg->command;
	tx_msg->sender  = rx_msg->sender;
	tx_msg->size    = 0;
	tx_msg->status  = SCPI_E_SUPPORT;

	/* Avoid reading past the end of the array; reply with the error. */
	if (rx_msg->command >= ARRAY_SIZE(scpi_cmds))
		goto reply;
	cmd = &scpi_cmds[rx_msg->command];

	/* Update the command status and payload based on the message. */
	if ((cmd->flags & FLAG_SECURE) && client != SCPI_CLIENT_SECURE) {
		/* Prevent Linux from sending commands that bypass PSCI. */
		tx_msg->status = SCPI_E_ACCESS;
	} else if (rx_msg->size != cmd->rx_size) {
		/* Check that the request payload matches the expected size. */
		tx_msg->status = SCPI_E_SIZE;
	} else if (cmd->handler) {
		/* Run the handler for this command to make a response. */
		tx_msg->status = cmd->handler(rx_msg, tx_msg);
	}

	if (cmd->flags & FLAG_NO_REPLY)
		return;

reply:
	scpi_send_message(client);
}

void
scpi_receive_message(uint8_t chan, uint32_t msg)
{
	uint8_t client = chan / 2;

	/* Do not try to parse messages sent with a different protocol. */
	if (msg != SCPI_VIRTUAL_CHANNEL)
		return;

	scpi_handle_message(client);
}

void
scpi_init(void)
{
	/* Enable the message box channels. */
	msgbox_enable_chan(MSGBOX_CHAN_SCPI_EL2_RX);
	msgbox_enable_chan(MSGBOX_CHAN_SCPI_EL3_RX);

	/* Send an "SCP Ready" message to ATF if not already booted. */
	if (system_state == SYSTEM_BOOT)
		scpi_create_message(SCPI_CLIENT_SECURE, SCPI_CMD_SCP_READY);
}
