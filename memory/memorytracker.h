/*
 * memorytracker v0.01 - Uthowaipru Chowdhury Baiching 2025
 *
 * Memory allocation tracker for debugging purposes.
 * Uses dynamic array (via STB_DS) for simpler tracking.
 *
 * Usage:
 * - Include this header and compile with memorytracker.c
 * - Use normally - allocations are automatically tracked
 * - Call f_trackListAllocations() to see current allocations that haven't been freed yet
 */

#ifndef MEMORY_TRACKER_H
#define MEMORY_TRACKER_H

#include <stdlib.h>

#define malloc(size) f_malloc_tracker(size, #size, __FILE__, __LINE__) // Replaces malloc
#define free(ptr) f_track_free(ptr, #ptr, __FILE__, __LINE__)

extern void *f_malloc_tracker(size_t size, const char *expr, const char *file, int line); // allocates with malloc, adds it to the list to track and returns the allotted address for user
extern void f_track_free(void *ptr, const char *expr, const char *file, int line); // free's the allocated memory and removes that from tracking list
extern void f_trackListAllocations(); // returns the list of unfreed malloc's filename and line numbers

#endif