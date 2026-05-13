/*
 * Copyright (c) 2022-2026 Siddharth Chandrasekaran <sidcha.dev@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _UTILS_ERROR_H_
#define _UTILS_ERROR_H_

#include <stdio.h>
#include <utils/utils.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ZEPHYR__

#include <zephyr/sys/__assert.h>

#elif defined(__BARE_METAL__)

/*
 * Bare-metal: avoid exit()/atexit cleanup (pulls in newlib stdio teardown).
 * printf is expected to be provided by the platform; on failure, halt.
 */
#define __ASSERT_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define __ASSERT(test, fmt, ...)                                          \
	do {                                                              \
		if (!(test)) {                                            \
			__ASSERT_PRINT("ASSERTION FAIL [%s] @ %s:%d\n\t" fmt "\n", \
				       STR(test), __FILE__, __LINE__,     \
				       ##__VA_ARGS__);                    \
			for (;;) { }                                      \
		}                                                         \
	} while (0)

#else /* everything else expects libc */

#define __ASSERT_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)

#define __ASSERT_LOC(test) \
        __ASSERT_PRINT("ASSERTION FAIL [%s] @ %s:%d\n", \
		       STR(test), __FILE__, __LINE__)

#define __ASSERT_MSG_INFO(fmt, ...) \
	__ASSERT_PRINT("\t" fmt "\n", ##__VA_ARGS__)

#define __ASSERT(test, fmt, ...)                                          \
        do {                                                              \
                if (!(test)) {                                            \
                        __ASSERT_LOC(test);                               \
                        __ASSERT_MSG_INFO(fmt, ##__VA_ARGS__);            \
			dump_trace(); \
			exit(EXIT_FAILURE); \
                }                                                         \
        } while (0)

#endif /* __ZEPHYR__ */

#ifdef __cplusplus
}
#endif

#endif /* _UTILS_ERROR_H_ */
