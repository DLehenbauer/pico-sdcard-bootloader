// Standard
#include <algorithm>
#include <iostream>
#include <limits.h>
#include <random>
#include <utility>
#include <vector>

// Google Test
#include <gtest/gtest.h>

// Project
#include "interval_set.h"

// IntervalSetTester is a utility class for testing the 'interval_set_t' functionality.
// It internally contains both a very simple interval set implementation as well as wraps
// an 'interval_set_t' instance.  Each operation is performed on both, and the results
// compared to ensure that the 'interval_set_t' behaves as expected.
class IntervalSetTester {
private:
    std::set<uint32_t> elements;    // A set of unique elements contained by the interval.
    interval_set_t set;             // The 'interval_set_t' instance under test.

public:
    IntervalSetTester() {
        interval_set_init(&set);
    }

    ~IntervalSetTester() {
        interval_set_free(&set);
    }

    std::vector<std::pair<uint32_t, uint32_t>> get_intervals() const {
        // Build a vector of disjoint intervals from the current set of elements.
        std::vector<std::pair<uint32_t, uint32_t>> intervals;
        if (elements.empty()) { return intervals; }
        
        uint32_t start = *elements.begin();
        uint32_t end = start;
        
        for (auto it = ++elements.begin(); it != elements.end(); ++it) {
            if (*it > end + 1) {
                intervals.push_back({start, end + 1});
                start = *it;
            }
            end = *it;
        }
        
        intervals.push_back({start, end + 1});
        return intervals;
    }

    void check() {
        auto intervals = get_intervals();
        ASSERT_EQ(set.num_intervals, intervals.size());
        
        for (size_t i = 0; i < intervals.size(); ++i) {
            ASSERT_EQ(set.intervals[i].start, intervals[i].first);
            ASSERT_EQ(set.intervals[i].end, intervals[i].second);
        }
    }

    int add(uint32_t start, uint32_t end) {        
        int added = 0;

        for (uint32_t i = start; i < end; ++i) {
            if (elements.insert(i).second) {
                added++;
            }
        }

        int actual = interval_set_union(&set, start, end);
        EXPECT_EQ(added, actual);
        check();

        return added;
    }
    
    int num_intervals() const {
        return set.num_intervals;
    }
};

class IntervalSetSuite : public ::testing::Test {
protected:
    interval_set_t set;

    void SetUp() override {
        interval_set_init(&set);
    }

    void TearDown() override {
        interval_set_free(&set);
    }
    
    void assert_intervals(const std::vector<std::pair<uint32_t, uint32_t>>& expected) {
        ASSERT_EQ(expected.size(), set.num_intervals);
        for (size_t i = 0; i < expected.size() && i < set.num_intervals; i++) {
            ASSERT_EQ(set.intervals[i].start, expected[i].first);
            ASSERT_EQ(set.intervals[i].end, expected[i].second);
        }
    }
};

// Test initialization
TEST_F(IntervalSetSuite, Init) {
    assert_intervals({});
}

// Test freeing resources
TEST_F(IntervalSetSuite, Free) {
    // Free set
    interval_set_free(&set);
    
    // Check that resources were freed
    EXPECT_EQ(0, set.num_intervals);
    EXPECT_EQ(0, set.num_elements);
    EXPECT_EQ(0, set._capacity);
    EXPECT_EQ(nullptr, set.intervals);
    
    // Re-initialize for TearDown
    interval_set_init(&set);
}

TEST_F(IntervalSetSuite, RejectInvalid) {
    // Empty interval.
    EXPECT_EQ(/* # added: */ 0, interval_set_union(&set, 10, 10));

    // Malformed interval.
    EXPECT_EQ(/* # added: */ 0, interval_set_union(&set, 20, 10));
}

TEST_F(IntervalSetSuite, First) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});
}

TEST_F(IntervalSetSuite, Clear) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    interval_set_clear(&set);
    assert_intervals({});  // Should be empty after clearing
}

TEST_F(IntervalSetSuite, LeftAdjacent) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 0, 10));
    assert_intervals({{0, 20}});
}

TEST_F(IntervalSetSuite, LeftOverlapping) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    EXPECT_EQ(/* # added: */ 5, interval_set_union(&set, 5, 15));
    assert_intervals({{5, 20}});
}

TEST_F(IntervalSetSuite, LeftOverlappingMultiple) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 30, 40));
    assert_intervals({{10, 20}, {30, 40}});

    EXPECT_EQ(/* # added: */ 15, interval_set_union(&set, 5, 35));
    assert_intervals({{5, 40}});
}

TEST_F(IntervalSetSuite, OverlapRemovesOne) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 30, 40));
    assert_intervals({{10, 20}, {30, 40}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 50, 60));
    assert_intervals({{10, 20}, {30, 40}, {50, 60}});

    EXPECT_EQ(/* # added: */ 15, interval_set_union(&set, 5, 35));
    assert_intervals({{5, 40}, {50, 60}});
}

TEST_F(IntervalSetSuite, OverlapMovesMultiple) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 30, 40));
    assert_intervals({{10, 20}, {30, 40}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 50, 60));
    assert_intervals({{10, 20}, {30, 40}, {50, 60}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 70, 80));
    assert_intervals({{10, 20}, {30, 40}, {50, 60}, {70, 80}});

    EXPECT_EQ(/* # added: */ 20, interval_set_union(&set, 10, 60));
    assert_intervals({{10, 60}, {70, 80}});
}

TEST_F(IntervalSetSuite, OverlapAll) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 30, 40));
    assert_intervals({{10, 20}, {30, 40}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 50, 60));
    assert_intervals({{10, 20}, {30, 40}, {50, 60}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 70, 80));
    assert_intervals({{10, 20}, {30, 40}, {50, 60}, {70, 80}});

    EXPECT_EQ(/* # added: */ 30, interval_set_union(&set, 15, 75));
    assert_intervals({{10, 80}});
}

TEST_F(IntervalSetSuite, AddDisjointCenter) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 50, 60));
    assert_intervals({{10, 20}, {50, 60}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 30, 40));
    assert_intervals({{10, 20}, {30, 40}, {50, 60}});
}

TEST_F(IntervalSetSuite, OverlapAdjacentLeft) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 40, 50));
    assert_intervals({{40, 50}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 20, 30));
    assert_intervals({{20, 30}, {40, 50}});

    EXPECT_EQ(/* # added: */ 20, interval_set_union(&set, 10, 40));
    assert_intervals({{10, 50}});
}

TEST_F(IntervalSetSuite, RightAdjacent) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 20, 30));
    assert_intervals({{10, 30}});
}

TEST_F(IntervalSetSuite, RightOverlapping) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    EXPECT_EQ(/* # added: */ 5, interval_set_union(&set, 15, 25));
    assert_intervals({{10, 25}});
}

TEST_F(IntervalSetSuite, RightOverlappingMultiple) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 30, 40));
    assert_intervals({{10, 20}, {30, 40}});

    EXPECT_EQ(/* # added: */ 15, interval_set_union(&set, 15, 45));
    assert_intervals({{10, 45}});
}

TEST_F(IntervalSetSuite, RightSkippingMultiple) {
    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 10, 20));
    assert_intervals({{10, 20}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 30, 40));
    assert_intervals({{10, 20}, {30, 40}});

    EXPECT_EQ(/* # added: */ 10, interval_set_union(&set, 50, 60));
    assert_intervals({{10, 20}, {30, 40}, {50, 60}});
}

TEST_F(IntervalSetSuite, Random) {
    std::minstd_rand minstd(42);

    for (int j = 0; j < 1000; j++) {
        IntervalSetTester testSet;

        do {
            for (int i = 0; i < 2; i++) {
                uint32_t start = minstd() % 100;
                uint32_t length = (minstd() % 10) - 1;          // Random length between -1 and 9
                uint32_t end = std::min(start + length, 99u);   // Ensure end doesn't exceed 99
                testSet.add(start, end);
            }
        } while (testSet.num_intervals() > 1);
    }
}
