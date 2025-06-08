/**
 * BSD Zero Clause License (SPDX: 0BSD)
 * 
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
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
