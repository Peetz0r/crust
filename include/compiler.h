/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMPILER_H
#define COMPILER_H

/* Attributes */
#define cold          __attribute__((__cold__))
#define constfn       __attribute__((__const__))
#define packed        __attribute__((__packed__))
#define pure          __attribute__((__pure__))
#define unused        __attribute__((__unused__))
#define used          __attribute__((__used__))
#define weak          __attribute__((__weak__))
#define noinline      __attribute__((__noinline__))

/* Builtins */
#define likely(e)     __builtin_expect(!!(e), 1)
#define unlikely(e)   __builtin_expect(e, 0)
#define unreachable() __builtin_unreachable()

/* Keywords */
#define alignas       _Alignas
#define alignof       _Alignof
#define asm           __asm__
#define noreturn      _Noreturn
#define static_assert _Static_assert

#endif /* COMPILER_H */
