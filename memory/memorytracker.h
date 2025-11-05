#ifndef MEMORY_TRACKER_H
#define MEMORY_TRACKER_H

#include <stdlib.h>

// #ifdef INTERNAL
#define malloc(size) f_malloc_tracker(size, #size, __FILE__, __LINE__) // Replaces malloc
#define free(ptr) f_tracK_free(ptr, #ptr, __FILE__, __LINE__)
// #define realloc(ptr, size) f_realloc_tracker(ptr, size, __FILE__, __LINE__) // Replaces realloc
// #endif


extern void *f_malloc_tracker(size_t size, const char *expr, const char *file, int line);
extern void f_tracK_free(void *ptr, const char *expr, const char *file, int line);
extern void f_trackListAllocations();
extern void *f_realloc_tracker(void *ptr, size_t size, const char *file, int line);
extern void f_test_print(void);

#endif