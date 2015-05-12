/**
 * This is probably the least portable header ever written. Some stuff I use:
 *
 * C11
 * /proc/self/maps
 * asprintf
 * __attribute__((cleanup))
 * __auto_type
 */

#pragma once

// require C11 (or gnu11)
#if !defined(__STDC__) || __STDC_VERSION__ < 201112L
  #error Please use C11 or later
#endif

// require gcc >= 4.9.2
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if !defined(__GNUC__) || GCC_VERSION < 40902
  #error Please use GCC 4.9.2 or later
#endif

// use all the goodies
#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>

// p should be a void**, not void*
// only free *p if it points to the heap
static void _heap_check_free(void *p)
{
	bool found_heap = false;

	FILE *maps = fopen("/proc/self/maps", "r");
	void *heap_start = NULL, *heap_end = NULL;

	while (!found_heap) {
		char line[128];
		if (fgets(line, sizeof line, maps) == NULL) {
			break; // EOF
		}
		if (strstr(line, "[heap]") != NULL) { // look for [heap] in /proc/self/maps
			found_heap = true;
			sscanf(line, "%x-%x", (unsigned int *)&heap_start, (unsigned int *)&heap_end);

			// should always be true, unless brk or sbrk is called between fgets() and this
			assert(heap_end == sbrk(0));
		}
	}
	fclose(maps);

	// valgrind oddly hides [heap] from maps, so we need the check
	if (found_heap)
		if (!(heap_start <= p && p <= heap_end))
			return;
	free(*(void**)p);
}

// call _heap_check_free on the variable when it goes out of scope
#define autofree __attribute__((cleanup(_heap_check_free)))

// type inference! can only declare 1 variable per statement
#define var __auto_type

typedef char *string;
#define autostr autofree string

// nicer way to use asprintf()
// autostr x = mkstring("my favorite int is %d", 17);
static string mkstring(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char *s;
	vasprintf(&s, fmt, args);

	va_end(args);
	return s;
}
