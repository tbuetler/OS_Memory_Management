#include "mem_mgmt.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define TLB_MAX_SIZE 5
#define PHYSICAL_FRAMES 9
#define PFN_BITS 16
#define VPN_BITS 8
#define OFFSET_BITS (64 - PFN_BITS)

/**
 * Helper function, asserts that the values returned by status() correspond to
 * the parameters given.
 *
 * @param free_frame_count     Expected number of free frames.
 * @param allocated_page_count Expected number of allocated pages.
 * @param tlb_entries_count    Expected number of TLB entries.
 */
static void assert_status(uint16_t free_frame_count,
                          uint16_t allocated_page_count,
                          uint16_t tlb_entries_count) {
  res_status result;
  status(&result);
  assert(result.free_frame_count == free_frame_count);
  assert(result.allocated_page_count == allocated_page_count);
  assert(result.tlb_entries_count == tlb_entries_count);
}

/**
 * Main function, runs the simulation.
 *
 * First, memory management is initialized with the symbolic constants defined
 * at the beginning of this file and two arrays are created.
 * The virt_addrs[]-array contains the addresses to be translated, the
 * results[]-array is used for storing and evaluating/comparing the returned
 * results.
 *
 * After initialization, a series of tests is conducted and the simulation ends
 * by freeing any acquired resources.
 */
int main(void) {
  // initialize simulation
  assert(setup(TLB_MAX_SIZE, PHYSICAL_FRAMES, PFN_BITS, VPN_BITS) == 0);
  assert_status(PHYSICAL_FRAMES, 0, 0);

  // initialize data structures used for the tests below
  uint64_t virt_addrs[] = {
	  0x23450000000000, 0x23FF0000005678, 0x11FF0000000000,
	  0x11FF0000000000, 0x18FF0000000000, 0x19080000000000,
	  0x11FF0000000000, 0x23450000000000, 0x00FF0000000000,
	  0x0123456789ABCD, 0x02000000000000, 0x03000000000000,
	  0x04000000000000, 0x05000000000000};
  size_t num_addrs = sizeof(virt_addrs) / sizeof(uint64_t);

  res_translate results[num_addrs];
  for (size_t i = 0; i < num_addrs; i++) {
    results[i].phy_address = 0;
    results[i].tlb_hit = false;
    results[i].new_frame = false;
  }

  ////////////////////// test 1
  puts("Test 1: Fresh address (no TLB hit, new frame)...");
  assert(translate(virt_addrs[0], &results[0]) == 0);
  printf("Virtual: %#016lx Physical: %#016lx\n", virt_addrs[0],
         results[0].phy_address);
  assert(results[0].tlb_hit == false);
  assert(results[0].new_frame == true);
  assert_status(PHYSICAL_FRAMES - 1, 1, 1);

  ////////////////////// test 2
  puts("\nTest 2: Fresh address, same frame (TLB hit)...");
  assert(translate(virt_addrs[1], &results[1]) == 0);
  printf("Virtual: %#016lx Physical: %#016lx\n", virt_addrs[1],
         results[1].phy_address);
  assert(results[1].tlb_hit == true);
  assert(results[1].new_frame == false);
  assert((results[1].phy_address >> OFFSET_BITS) ==
         (results[0].phy_address >> OFFSET_BITS));
  assert_status(PHYSICAL_FRAMES - 1, 1, 1);

  ////////////////////// test 3
  puts("\nTest 3: Fresh address (no TLB hit, new frame)...");
  assert(translate(virt_addrs[2], &results[2]) == 0);
  printf("Virtual: %#016lx Physical: %#016lx\n", virt_addrs[2],
         results[2].phy_address);
  assert(results[2].tlb_hit == false);
  assert(results[2].new_frame == true);
  assert((results[2].phy_address >> OFFSET_BITS) !=
         (results[1].phy_address >> OFFSET_BITS));
  assert_status(PHYSICAL_FRAMES - 2, 2, 2);

  ////////////////////// test 4
  puts("\nTest 4: Some more translations...");
  for (size_t i = 3; i < num_addrs - 1; i++) {
    assert(translate(virt_addrs[i], &results[i]) == 0);
    printf("Virtual: %#016lx Physical: %#016lx (i=%zu)\n", virt_addrs[i],
           results[i].phy_address, i);
    if (i == 3 || i == 6 || i == 7)
      assert(results[i].tlb_hit == true);
    else
      assert(results[i].tlb_hit == false);
  }

  assert_status(0, PHYSICAL_FRAMES, TLB_MAX_SIZE);
  assert(results[0].phy_address == results[7].phy_address);
  assert(results[6].phy_address == results[2].phy_address);
  assert(results[3].phy_address != results[4].phy_address);

  ////////////////////// test 5
  puts("\nTest 5: Out of memory (no free frames left)...");
  res_translate result = {0};
  assert(translate(0xFFFFFFFFFFFFFFFF, &result) != 0);
  assert_status(0, PHYSICAL_FRAMES, TLB_MAX_SIZE);

  // simulation done - cleanup!
  puts("\nSimulation completed!");
  teardown();
}
