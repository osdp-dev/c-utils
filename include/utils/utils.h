/*
 * Copyright (c) 2020-2026 Siddharth Chandrasekaran <sidcha@osdp.dev>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _UTILS_UTILS_H_
#define _UTILS_UTILS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL                           ((void *)0)
#endif

#ifndef TRUE
#define TRUE                           (1)
#endif

#ifndef FALSE
#define FALSE                          (0)
#endif

#define BYTE_0(x)                      (uint8_t)(((x) >>  0) & 0xFF)
#define BYTE_1(x)                      (uint8_t)(((x) >>  8) & 0xFF)
#define BYTE_2(x)                      (uint8_t)(((x) >> 16) & 0xFF)
#define BYTE_3(x)                      (uint8_t)(((x) >> 24) & 0xFF)

#ifndef BIT
#define BIT(n)                         (1ull << (n))
#endif
#define MASK(n)                        (BIT((n) + 1) - 1)
#define BIT_IS_SET(m, n)               (bool)((m) & BIT(n))
#define BIT_SET(m, n)                  ((m) |=  BIT(n))
#define BIT_CLEAR(m, n)                ((m) &= ~BIT(n))
#define BIT_FLIP(m, n)                 ((m) ^=  BIT(n))
#define BIT_TEST_SET(m, n)             ({ BIT_IS_SET(m, n) ? false : (bool) BIT_SET(m, n) })

#define ARG_UNUSED(x)                  (void)(x)

#define STR(x) #x
#define XSTR(x) STR(x)

#ifndef __ZEPHYR__
#define STRINGIFY(x) #x
#endif

#ifndef ROUND_UP
#define ROUND_UP(x, y) ((x + y - 1) & ~ (y - 1))
#endif

#define MATH_MOD(a, b)                 (((a % b) + b) % b)

#define IS_POW2(n)                     ((n) & ((n) - 1))

#define ARRAY_SIZEOF(x) \
	(sizeof(x) / sizeof(x[0]))

#define ARRAY_BASE(ptr, type, offset) \
	((char *)(ptr)) - ((sizeof(type)) * offset)

#define OFFSET_OF(type, field) (size_t)(&((type *)(0))->field)

#ifndef CONTAINER_OF
#define CONTAINER_OF(ptr, type, field) \
        ((type *)(((char *)(ptr)) - OFFSET_OF(type, field)))
#endif

#ifndef MAX
#define MAX(a,b) ({ \
		__typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; \
	})
#endif

#ifndef MIN
#define MIN(a,b) ({ \
		__typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _b : _a; \
	})
#endif

#define SWAP(a,b) { \
		__typeof__ (a) _tmp; \
		_tmp = a; \
		b = a; \
		a = _tmp; \
	}

#define __READ_ONCE(x)	(*(volatile typeof(x) *)&(x))
#define READ_ONCE(x) ({ \
		typeof(x) *__xp = &(x); \
		typeof(x) __x = __READ_ONCE(*__xp); \
		__x; \
	})

#define WRITE_ONCE(x, val) do { \
		typeof(x) *__xp = &(x); \
		*(volatile typeof(x) *)__xp = (val); \
	} while (0)

#define ABS(x)		((x) >= 0  ? (x) : -(x))

/*
 * config_enabled() from the kernel. Zephyr's <zephyr/sys/util_macro.h>
 * defines IS_ENABLED() without an #ifndef guard, so a unit that included
 * this header first and Zephyr's second would see it redefined; on Zephyr,
 * take that definition instead.
 */
#if defined(__ZEPHYR__)
#include <zephyr/sys/util_macro.h>
#elif !defined(IS_ENABLED)
#define __IS_ENABLED1(x)             __IS_ENABLED2(__XXXX ## x)
#define __XXXX1                       __YYYY,
#define __IS_ENABLED2(y)             __IS_ENABLED3(y 1, 0)
#define __IS_ENABLED3(_i, val, ...)   val

#define IS_ENABLED(x)                 __IS_ENABLED1(x)
#endif

/* gcc attribute shorthands */
#ifndef __fallthrough
#if __GNUC__ >= 7 || defined(__clang__)
#define __fallthrough        __attribute__((fallthrough))
#else
#define __fallthrough
#endif
#endif

#ifndef __packed
#define __packed        __attribute__((__packed__))
#endif

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#ifdef __weak
#undef __weak
#endif

#if (defined(_WIN32) || defined(_WIN64))

#include <io.h>
#define isatty                  _isatty /* from io.h */
#define fileno                  _fileno
#define __format_printf(x, y)
#define __noreturn              __declspec(noreturn)
#define __weak
#if defined(_MSC_VER)
#define __unreachable()         __assume(0)
#else
#define __unreachable()         __builtin_unreachable()
#endif
#define likely(p)               (p)
#define unlikely(p)             (p)
#define PATH_SEPARATOR          '\\'

#elif defined(__ZEPHYR__)

#include <zephyr/toolchain.h>  // __weak, __builtin_xxx() etc.,

/*
 * The #undef __weak above unconditionally drops any prior definition. When
 * <zephyr/toolchain.h> was already pulled in (e.g. via <zephyr/kernel.h>),
 * its include guard makes the re-include a no-op and __weak stays undefined.
 * Restore it so header-level __weak declarations below still parse.
 */
#ifndef __weak
#define __weak                  __attribute__((weak))
#endif
#define PATH_SEPARATOR          '/'
#define __unreachable()         __builtin_unreachable()
#define __format_printf(x, y)   __attribute__((format(printf, x, y)))
#ifndef __noreturn
#define __noreturn              __attribute__((noreturn))
#endif

#elif defined(ESP_PLATFORM)

#include <esp_compiler.h>  // likely(), unlikely()
#include <sys/cdefs.h>     // __noreturn

/*
 * ESP-IDF ships its own versions of some of these, so it is detected here
 * rather than left to the generic branch below, which would define them a
 * second time.
 *
 * likely() and unlikely() are taken from <esp_compiler.h>, included above so
 * that the SDK's definitions are in effect no matter what else the translation
 * unit pulls in. Both that header and this one guard with #ifndef, so leaving
 * it to chance means the first include wins -- and since IDF makes these depend
 * on CONFIG_COMPILER_OPTIMIZATION_PERF, the same macro could expand to
 * __builtin_expect() in one translation unit and to (x) in another.
 *
 * __weak is guarded because IDF defines it in no header we pull in, and
 * deferring to the SDK is right if that ever changes.
 *
 * __noreturn is not guarded. IDF's <sys/cdefs.h> -- included above so that its
 * definition is already in effect here -- defines it as empty for every libc
 * but picolibc. Deferring to that leaves a macro that silently drops the
 * attribute: a function that never returns then looks to its caller like one
 * that falls off the end, which IDF compiles with -Werror=all. Replace it.
 *
 * That header has no include guard of its own, so a translation unit that
 * pulls in a libc header *after* this one gets the empty definition back.
 * Include <utils/utils.h> last.
 */
#ifndef __weak
#define __weak                  __attribute__((weak))
#endif
#undef __noreturn
#define __noreturn              __attribute__((noreturn))
#define __format_printf(x, y)   __attribute__((format(printf, x, y)))
#define __unreachable()         __builtin_unreachable()
#define PATH_SEPARATOR          '/'

#else

#define __format_printf(x, y)   __attribute__((format(printf, x, y)))
#define __noreturn              __attribute__((noreturn))
#define __weak                  __attribute__((weak))
#define __unreachable()         __builtin_unreachable()
#define likely(p)               __builtin_expect(!!(p), 1)
#define unlikely(p)             __builtin_expect(!!(p), 0)
#define PATH_SEPARATOR          '/'
#endif

/*
 * Platforms we can detect and serve directly: each has a tick source and an
 * entropy source that utils implements natively, so no application-supplied
 * hooks are needed. __BARE_METAL__ on anything else means a target we cannot
 * identify -- there is no portable way to find a clock or entropy on it, so
 * such a target must supply them itself.
 */
#if defined(ARDUINO) || defined(__ZEPHYR__) || defined(MYNEWT)
#define UTILS_KNOWN_PLATFORM 1
#endif

#if defined(__BARE_METAL__) && !defined(UTILS_KNOWN_PLATFORM)
#define UTILS_UNKNOWN_TARGET 1
#endif

#if defined(__BARE_METAL__) && defined(USE_32BIT_TICK_T)
/*
 * Bare-metal timing note:
 *
 * When __BARE_METAL__ and USE_32BIT_TICK_T are both enabled, tick_t uses
 * 32-bit storage. The selected tick source and tick width together define the
 * maximum delay interval that can be measured safely in the project.
 *
 * Example: with a 1 kHz tick source, uint32_t spans about 49.7 days.
 * Make sure your largest required delay/timeout remains safe for your chosen
 * tick source and counter width.
 */
typedef uint32_t tick_t;
#else
typedef uint64_t tick_t;
#endif

/**
 * @brief Return a random uint32.
 *
 * Entropy depends on the platform: hosted builds forward to libc rand();
 * Arduino uses the core's hardware TRNG where one exists (ESP32, ESP8266)
 * and its random() PRNG otherwise; unknown bare-metal targets have no
 * definition at all and must supply one (a missing one is a link error).
 *
 * Only the TRNG-backed paths are cryptographically strong. Platforms that
 * can do better (an STM32 RNG peripheral, an mbedTLS DRBG, /dev/urandom)
 * should override this -- it is __weak wherever it is defined.
 */
#ifdef UTILS_UNKNOWN_TARGET
/* Strong declaration: a __weak undefined reference resolves to NULL and
 * would be called. Keep it strong so a missing definition fails the link. */
uint32_t rand_u32(void);
#else
__weak uint32_t rand_u32(void);
#endif

/**
 * @brief Return random number between 0 and `limit` both inclusive.
 *
 * Backed by rand_u32(). Non-cryptographic.
 */
int randint(int limit);

/**
 * @brief Rounds up 32-bit v to nearest power of 2. If v is already a power
 * of 2 it is returned unmodified.
 */
uint32_t round_up_pow2(uint32_t v);

/**
 * @brief Retruns number of digits in a given number in its decimal form.
 */
int num_digits_in_number(int num);

/**
 * @brief Dumps an array of bytes in HEX and ASCII formats for debugging. `head`
 * is string that is printed before the actual bytes are dumped.
 *
 * Example:
 * 	int len;
 * 	uint8_t data[MAX_LEN];
 * 	len = get_data_from_somewhere(data, MAX_LEN);
 * 	hexdump(data, len, "Data From Somewhere");
 */
void hexdump(const void *data, size_t len, const char *fmt, ...);

/**
 * @brief Get the time in micro seconds.
 */
tick_t usec_now();

/**
 * @brief Get time elapsed in micro seconds since `last`. Used along with
 * usec_now().
 */
tick_t usec_since(tick_t last);

/**
 * @brief Get the time in milli seconds.
 */
tick_t millis_now();

/**
 * @brief Get time in seconds and micro_seconds
 */
void get_time(uint32_t *seconds, uint32_t *micro_seconds);

/**
 * @brief Addd an ISO 8601 datetime string to `buf` for upto `size` bytes.
 *
 * Note: The format is YYYY-MM-DDThh:mm:ssZ, so size must alteast be 21.
 */
int add_iso8601_utc_datetime(char *buf, size_t size);

/**
 * @brief Get time elapsed in milli seconds since `last`. Used along with
 * millis_now().
 */
tick_t millis_since(tick_t last);

/**
 * @brief Print the stack trace
 */
void dump_trace(void);

static inline bool char_is_space(int c)
{
	unsigned char d = (unsigned char)(c - 9);
	return (0x80001FU >> (d & 31)) & (1U >> (d >> 5));
}

static inline bool char_is_digit(int c)
{
	return (unsigned int)(('0' - 1 - c) & (c - ('9' + 1))) >> (sizeof(c) * 8 - 1);
}

static inline bool char_is_alpha(int c)
{
	return (unsigned int)(('a' - 1 - (c | 32)) & ((c | 32) - ('z' + 1))) >> (sizeof(c) * 8 - 1);
}

static inline uint8_t u8_bit_reverse(uint8_t b)
{
	b = (((b & 0xaa) >> 1) | ((b & 0x55) << 1));
	b = (((b & 0xcc) >> 2) | ((b & 0x33) << 2));
	return ((b >> 4) |  (b << 4));
}

static inline uint16_t u16_bit_reverse(uint16_t x)
{
	x = (((x & 0xaaaa) >> 1) | ((x & 0x5555) << 1));
	x = (((x & 0xcccc) >> 2) | ((x & 0x3333) << 2));
	x = (((x & 0xf0f0) >> 4) | ((x & 0x0f0f) << 4));
	return((x >> 8) | (x << 8));
}

static inline uint32_t u32_bit_reverse(uint32_t x)
{
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	return ((x >> 16) | (x << 16));
}

#ifdef __cplusplus
}
#endif

#endif /* _UTILS_UTILS_H_ */
