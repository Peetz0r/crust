/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <devices.h>
#include <mmio.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

enum {
	UART_THR = 0x0000,
	UART_LSR = 0x0014,
};

enum {
	UART_LSR_THRE = BIT(5),
};

static void
debug_putc(char c)
{
	mmio_poll_32(DEV_UART0 + UART_LSR, UART_LSR_THRE);
	mmio_write_32(DEV_UART0 + UART_THR, c);
}

static void
debug_puts(const char *s)
{
	for (char c; (c = *s); ++s)
		debug_putc(c);
}

static void
debug_putbegin(void)
{
	debug_puts("SCP:\t ");
}

static void
debug_putend(void)
{
	debug_putc('\r');
	debug_putc('\n');
}

static void
debug_putx(uint32_t x)
{
	static const char digits[16] = "0123456789abcdef";
	bool nonzero = false;

	debug_putc('0');
	debug_putc('x');
	for (int i = 28; i >= 0; i -= 4) {
		uint32_t digit = (x >> i) & 0xf;
		nonzero |= digit;
		if (nonzero || i == 0)
			debug_putc(digits[digit]);
	}
}

void
debugS(const char *s)
{
	debug_putbegin();
	debug_puts(s);
	debug_putend();
}

void
debugSX(const char *s, uint32_t x)
{
	debug_putbegin();
	debug_puts(s);
	debug_putx(x);
	debug_putend();
}

void
debugSXSX(const char *s1, uint32_t x1, const char *s2, uint32_t x2)
{
	debug_putbegin();
	debug_puts(s1);
	debug_putx(x1);
	debug_puts(s2);
	debug_putx(x2);
	debug_putend();
}
