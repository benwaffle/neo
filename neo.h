#pragma once

#if !defined(__STDC__) || __STDC_VERSION__ < 201112L
  #error Please use C11 or later
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>

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
		if (strstr(line, "[heap]") != NULL) {
			found_heap = true;
			sscanf(line, "%x-%x", &heap_start, &heap_end);
			assert(heap_end == sbrk(0));
		}
	}
	fclose(maps);

	if (found_heap)
		if (!(heap_start <= p && p <= heap_end)) // not on heap
			return;
	free(*(void**)p);
}

#define autofree __attribute__((cleanup(_heap_check_free)))
#define var __auto_type
typedef char *string;
#define autostr autofree string

static string mkstring(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char *s;
	vasprintf(&s, fmt, args);

	va_end(args);
	return s;
}
