/**
 * https://github.com/DLehenbauer/pico-sdcard-bootloader
 * SPDX-License-Identifier: 0BSD
 */

#pragma once

// Standard
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// A half-open interval [start, end)
typedef struct {
    uint32_t start;     // Inclusive start of the interval
    uint32_t end;       // Exclusive end of the interval
} interval_t;

// A set of disjoint intervals.
typedef struct {
    interval_t* intervals;  // Dynamic array of intervals
    int num_elements;       // Number of elements contained in the set
    int num_intervals;      // Number of disjoint intervals in the set
    int _capacity;          // Private: capacity of the current intervals array
} interval_set_t;

// Initialize a new interval set.
void interval_set_init(interval_set_t* set);

// Free resources used by the interval set.
void interval_set_free(interval_set_t* set);

// Remove all intervals from the set.
void interval_set_clear(interval_set_t* set);

// Updates 'set' to be the union of [start, end) and any existing intervals in 'set'.
// Returns the number of new elements added to the set.
int interval_set_union(interval_set_t* set, uint32_t start, uint32_t end);

#ifdef __cplusplus
}  // extern "C"
#endif
