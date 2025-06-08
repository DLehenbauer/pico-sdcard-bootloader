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

// Standard
#include <stdlib.h>
#include <string.h>

// Pico SDK
#include <pico.h>           // for MIN/MAX
#include <pico/assert.h>

// Project
#include "interval_set.h"

#ifdef NDEBUG
#define self_check(set) ((void)0)
#else
static void self_check(const interval_set_t* set) {
    // Ensure the set is initialized.
    assert(set->_capacity > 0);
    assert(set->intervals != NULL);

    // Ensure num_intervals is valid wrt. _capacity
    assert(set->num_intervals <= set->_capacity);

    // If there are no elements, there must also be no intervals and vice versa.
    assert((set->num_elements == 0) == (set->num_intervals == 0));

    if (set->num_intervals == 0) { return; }

    // Ensure the first interval is non-empty and well-formed.
    int num_in_interval = set->intervals[0].end - set->intervals[0].start;    
    assert(num_in_interval >= 0);

    // Keep a running total of the # of elements in the set.
    int total_elements = num_in_interval;

    for (int i = 1; i < set->num_intervals; i++) {
        // The current element must be disjoint from the previous one,
        assert(set->intervals[i - 1].end < set->intervals[i].start);

        // Ensure the current interval is non-empty and well-formed.
        int num_in_interval = set->intervals[i].end - set->intervals[i].start;
        assert(num_in_interval >= 0);

        total_elements += num_in_interval;
    }

    // 'set->num_elements' must match the total # of elements in all intervals.
    assert(set->num_elements == total_elements);
}
#endif

void interval_set_init(interval_set_t* set) {
    memset(set, 0, sizeof(interval_set_t));
    set->_capacity = 1;
    set->intervals = (interval_t*) malloc(sizeof(interval_t) * set->_capacity);
}

void interval_set_free(interval_set_t* set) {
    free(set->intervals);
    memset(set, 0, sizeof(interval_set_t));
}

void interval_set_clear(interval_set_t* set) {
    set->num_elements = 0;
    set->num_intervals = 0;

    self_check(set);
}

// Returns the index of the interval that intersects the given start point,
// or the index where the new interval should be inserted if no intersection exists.
static int find_interval(const interval_set_t* set, uint32_t start) {
    int left = 0;
    int right = set->num_intervals - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (set->intervals[mid].end < start) {
            left = mid + 1;
        } else if (mid > 0 && set->intervals[mid - 1].end >= start) {
            right = mid - 1;
        } else {
            return mid;
        }
    }

    return left;
}

// Returns the number of elements in [start, end) that are not also in the 'current' interval.
//
// This is used to calculate the number of new elements added to the set when the insertion
// intersects existing intervals.
static int diff_count(uint32_t start, uint32_t end, const interval_t* current) {
    const int diff_start = MAX(start, current->start);
    const int diff_end   = MIN(end, current->end);
    return diff_end - diff_start;
}

// Adds the elements in the interval [start, end) to the set.  Afterward, 'set' will contain
// the union of the existing intervals and the new interval.  The return value is the number
// of new elements added to the set.
int interval_set_union(interval_set_t* set, uint32_t start, uint32_t end) {
    if (start >= end) { return 0; }

    int added = end - start;
    int i = find_interval(set, start);

    if (i >= set->num_intervals || end < set->intervals[i].start) {
        //
        // Case 1: The new interval is disjoint.  Insert it at position 'i' and return.
        //
        
        // Before inserting, grow the intervals array if necessary.
        if (set->num_intervals == set->_capacity) {
            set->_capacity += (set->_capacity / 2) + 1;
            set->intervals = (interval_t*) realloc(set->intervals, sizeof(interval_t) * set->_capacity);
        }

        interval_t* inserted = &set->intervals[i];

        // Shift intervals starting at the insertion point and above to make room for the new interval.
        memmove(
            /* dest: */ inserted + 1,
            /*  src: */ inserted,
            /*    n: */ (set->num_intervals - i) * sizeof(interval_t));

        // Insert the new interval at index 'i'.
        inserted->start = start;
        inserted->end = end;
        set->num_intervals++;
    } else {
        //
        // Case 2: The new interval intersects an existing interval at index 'i'.
        //

        // 'dest' is the first interval instersected by [start, end).  We will extend this internal
        // to include any new elements in [start, end) that are not already in 'dest'.
        interval_t* dest = &set->intervals[i];

        // First, subtract the # of elements that are already in the 'dest' interval.  These elements
        // were already present in the set, so they are not being added by the union operation.
        added -= diff_count(start, end, dest);

        // Extend 'dest' to include the new interval [start, end).
        dest->start = MIN(start, dest->start);
        dest->end   = MAX(end, dest->end);
        
        // It's possible that [start, end) also intersects subsequent intervals in the set.  Starting
        // with the interval after 'dest', scan forwards and union any additional intervals that
        // intersect with [start, end) into 'dest'.
        i++;
        interval_t* candidate = dest + 1;   // DANGER: 'candidate' is OOB if 'set->num_intervals == i'

        while (i < set->num_intervals && candidate->start <= end) {
            // The 'candidate' interval also intersects with [start, end).  Again, subtract the #
            // of elements of [start, end) are already present in the 'candidate' interval.
            added -= diff_count(start, end, candidate);

            // Extend 'dest' to also include the 'candidate' interval.
            dest->end = MAX(dest->end, candidate->end);

            // Remove the 'candidate' interval from the set by shifting the subsequent intervals down.
            memmove(/* dest: */ candidate,
                    /*  src: */ candidate + 1,
                    /*    n: */ (set->num_intervals - i - 1) * sizeof(interval_t));

            set->num_intervals--;

            // DANGER: After the memmove above, 'candidate' now points to the interval after the
            //         one we just removed and is OOB if 'set->num_intervals == i'.
        }
    }

    // Update the total number of elements in the set.
    set->num_elements += added;

    self_check(set);

    return added;
}
