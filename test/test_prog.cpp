// Google Test
#include <gtest/gtest.h>

// Project
#include "prog.h"
#include "vector_table.h"

static const struct uf2_block valid_block = {
    .magic_start0 = UF2_MAGIC_START0,
    .magic_start1 = UF2_MAGIC_START1,
    .flags = UF2_FLAG_FAMILY_ID_PRESENT,
    .target_addr = PROG_AREA_BEGIN,
    .payload_size = FLASH_PAGE_SIZE,
    .block_no = 0,
    .num_blocks = 1,
    .file_size = RP2040_FAMILY_ID,
    .data = {
        // Initialize payload with a plausible vector table.
        SRAM_END & 0xFF,
        (SRAM_END >> 8) & 0xFF,
        (SRAM_END >> 16) & 0xFF,
        (SRAM_END >> 24) & 0xFF,
        ((XIP_BASE + PICO_FLASH_SIZE_BYTES / 2) & 0xFF) | 1,
        ((XIP_BASE + PICO_FLASH_SIZE_BYTES / 2) >> 8) & 0xFF,
        ((XIP_BASE + PICO_FLASH_SIZE_BYTES / 2) >> 16) & 0xFF,
        ((XIP_BASE + PICO_FLASH_SIZE_BYTES / 2) >> 24) & 0xFF,
    },
    .magic_end = UF2_MAGIC_END
};

// Add this at the top level of your file, outside any class
static bool callback_result = true;
static int g_last_block_accepted = -1;

static bool test_accept_block_cb(prog_t* prog, const struct uf2_block* block) {
    g_last_block_accepted = block->block_no;
    return callback_result;
}

class ProgSuite : public ::testing::Test {
protected:
    prog_t prog;
    int last_block_accepted;

    void SetUp() override {
        prog_init(&prog);        
        prog.accept_block = test_accept_block_cb;
    }

    void assert_ok(const struct uf2_block& block) {
        callback_result = true;
        int previous_written = prog.pages_written.num_elements;
        int previous_erased = prog.sectors_erased.num_elements;
        g_last_block_accepted = -1;

        EXPECT_TRUE(process_block(&prog, &block));

        // 'accept_block' callback must be invoked for the block.
        ASSERT_EQ(g_last_block_accepted, block.block_no);

        // The number of written pages must increase by 1 and erased sectors must not be zero.
        ASSERT_EQ(prog.pages_written.num_elements, previous_written + 1);
        ASSERT_GE(prog.sectors_erased.num_elements, 1);
    }

    void assert_skipped(const struct uf2_block& block) {
        callback_result = true;
        int previous_accepted = prog.num_blocks_accepted;
        int previous_written = prog.pages_written.num_elements;
        int previous_erased = prog.sectors_erased.num_elements;
        
        g_last_block_accepted = -1;
        
        // The block is a valid UF2 block, so process_block must return true
        // to continue programming.
        EXPECT_TRUE(process_block(&prog, &block));

        // However, the 'accept_block' callback must not be invoked
        ASSERT_EQ(g_last_block_accepted, -1);

        // 'prog.num_blocks_accepted' must not change.
        ASSERT_EQ(prog.num_blocks_accepted, previous_accepted);

        // The number of written pages and erased sectors must not change.
        ASSERT_EQ(prog.pages_written.num_elements, previous_written);
        ASSERT_EQ(prog.sectors_erased.num_elements, previous_erased);
    }

    void assert_bad(const struct uf2_block& block) {
        callback_result = true;
        int previous_accepted = prog.num_blocks_accepted;
        int previous_written = prog.pages_written.num_elements;
        int previous_erased = prog.sectors_erased.num_elements;

        g_last_block_accepted = -1;

        // The block is invalid, so process_block must return false to abort programming.
        EXPECT_FALSE(process_block(&prog, &block));

        // The bad block must not be accepted.
        ASSERT_EQ(g_last_block_accepted, -1);

        // 'prog.num_blocks_accepted' must not change.
        ASSERT_EQ(prog.num_blocks_accepted, previous_accepted);

        // The number of written pages and erased sectors must not change.
        ASSERT_EQ(prog.pages_written.num_elements, previous_written);
        ASSERT_EQ(prog.sectors_erased.num_elements, previous_erased);
    }

    void TearDown() override {
        prog_free(&prog);
    }
};

// Test valid block processing
TEST_F(ProgSuite, ProcessValidBlock) {
    assert_ok(valid_block);
}

// Test invalid magic numbers
TEST_F(ProgSuite, InvalidMagicNumbers) {
    // Invalid start0
    struct uf2_block block = valid_block;
    block.magic_start0++;
    assert_bad(block);
    
    // Invalid start1
    block = valid_block;
    block.magic_start1++;
    assert_bad(block);
    
    // Invalid end
    block = valid_block;
    block.magic_end++;
    assert_bad(block);
}

// Test invalid family ID
TEST_F(ProgSuite, InvalidFamilyID) {   
    // Flag not set
    struct uf2_block block = valid_block;
    block.flags &= ~UF2_FLAG_FAMILY_ID_PRESENT;
    assert_skipped(block);
    
    // Flag set but wrong family ID
    block = valid_block;
    block.file_size++;
    assert_skipped(block);
}

// Test block count validation
TEST_F(ProgSuite, BlockCountValidation) {
    // First block with zero num_blocks (invalid)
    struct uf2_block block = valid_block;
    block.num_blocks = 0;
    assert_bad(block);
    
    // Process a valid first block
    assert_ok(valid_block);
    
    // Second block with different num_blocks
    block = valid_block;
    block.block_no++;
    block.num_blocks++;
    assert_bad(block);
}

// Test block number validation
TEST_F(ProgSuite, BlockOrderValidation) {
    // Process valid first block
    assert_ok(valid_block);

    // Next block should have block_no = 1, but it's still 0.
    assert_bad(valid_block);
    
    // Should still be expecting block_no = 1.
    struct uf2_block block = valid_block;
    block.block_no = 2; // Should be 1
    assert_bad(block);
    
    // Block number >= total blocks
    block = valid_block;
    block.block_no = valid_block.num_blocks;
    assert_bad(block);
}

// Test NOT_MAIN_FLASH flag
TEST_F(ProgSuite, NotMainFlashFlag) {
    struct uf2_block block = valid_block;
    block.flags |= UF2_FLAG_NOT_MAIN_FLASH;
    assert_skipped(block);
}

// Test address alignment validation
TEST_F(ProgSuite, AddressAlignment) {
    // Unaligned target address
    struct uf2_block block = valid_block;
    block.target_addr++; // Unaligned
    assert_bad(block);
}

// Test payload size validation
TEST_F(ProgSuite, PayloadSizeValidation) {
    // Invalid payload size
    struct uf2_block block = valid_block;
    block.payload_size--; // Not equal to page size
    assert_bad(block);
    
    block = valid_block;
    block.payload_size++; // Not equal to page size
    assert_bad(block);
}

// Test program area bounds validation
TEST_F(ProgSuite, ProgramAreaBounds) {
    // Address below program area
    struct uf2_block block = valid_block;
    block.target_addr = PROG_AREA_BEGIN - 1;
    assert_bad(block);
    
    // Address above program area
    block = valid_block;
    block.target_addr = PROG_AREA_END + 1;
    assert_bad(block);
}

// Test duplicate page writing detection
TEST_F(ProgSuite, DuplicatePageWriting) {
    struct uf2_block block = valid_block;
    block.num_blocks = 2;
    assert_ok(block);
    
    // Try writing to the same page again
    block.block_no = 1; // Next block
    assert_bad(block); // Should fail (duplicate write)
    
    // Try writing to a different page
    block.target_addr += FLASH_PAGE_SIZE;
    assert_ok(block);
}

// Test vector table validation
TEST_F(ProgSuite, VectorTableValidation) {
    // Create a block targeting vector table address but with invalid vector table data
    struct uf2_block block = valid_block;
    block.target_addr = VECTOR_TABLE_ADDR;
    memset(block.data, 0xFF, sizeof(block.data)); // Invalid vector table
    
    // With current implementation, this will fail since check_vector_table will return false
    assert_bad(block);
}

TEST_F(ProgSuite, MultipleBlocks) {
    struct uf2_block block = valid_block;
    block.num_blocks = 3;

    // Sequential blocks should be accepted.
    for (block.block_no = 0; block.block_no < block.num_blocks; block.block_no++) {
        assert_ok(block);
        block.target_addr += FLASH_PAGE_SIZE;
    }

    // Extra block should be rejected.
    assert_bad(block);
}

TEST_F(ProgSuite, MultipleBlocksWithAddressGaps) {
    struct uf2_block block = valid_block;
    block.num_blocks = 3;

    // Sequential blocks should be accepted.
    for (block.block_no = 0; block.block_no < block.num_blocks; block.block_no++) {
        assert_ok(block);
        block.target_addr += FLASH_PAGE_SIZE * (block.block_no + 1);
    }

    // Extra block should be rejected.
    assert_bad(block);
}

TEST_F(ProgSuite, RejectedByCallback) {
    callback_result = false; // Simulate callback rejecting the block
    
    // Process the block, which should be rejected by the callback
    g_last_block_accepted = -1; // Reset last accepted block
    EXPECT_FALSE(process_block(&prog, &valid_block));

    // The callback is invoked, hence g_last_block_accepted is now 0.
    EXPECT_EQ(g_last_block_accepted, 0);
    
    // However, the callback return false.  Therefore num_blocks_accepted remains 0.
    EXPECT_EQ(prog.num_blocks_accepted, 0);

    // Reset callback result for future tests
    callback_result = true;
}
