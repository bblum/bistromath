#ifndef _ASSERT_H
#define _ASSERT_H

#include <stdio.h>

void assert_fail(const char *, const char *, int, const char *);

#define assert(expr) do { \
	if (!(expr)) assert_fail(#expr, __FILE__, __LINE__, __func__); \
} while (0)

#define panic(s) assert_fail(s, __FILE__, __LINE__, __func__)

#endif
