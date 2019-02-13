/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

#if CONFIG_ASSERT
#if CONFIG_ASSERT_VERBOSE
#define assert(e) ((void)((e) || (panic("SCP: Assert! " #e), 0)))
#else
#define assert(e) ((void)((e) || (panic("SCP: Assert!"), 0)))
#endif
#else
#define assert(e) ((void)((e) || (unreachable(), 0)))
#endif

#if CONFIG_DEBUG_LOG

void debugS(const char *s);
void debugSX(const char *s, uint32_t x);
void debugSXSX(const char *s1, uint32_t x1, const char *s2, uint32_t x2);

#else

static inline void
debugS(unused const char *s)
{
}

static inline void
debugSX(unused const char *s, unused uint32_t x)
{
}

static inline void
debugSXSX(unused const char *s1, unused uint32_t x1, unused const char *s2,
          unused uint32_t x2)
{
}

#endif

noreturn void reset(void);

static inline noreturn void
panic(const char *s)
{
	debugS(s);
	reset();
}

#endif /* DEBUG_H */
