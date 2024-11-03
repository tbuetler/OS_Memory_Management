#ifndef MEM_MGMT_H
#define MEM_MGMT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Initializes the simulation and allocates required resources.
 *
 * @param tlb_max_size Maximum number of TLB entries (0 - 255).
 * @param phy_frames   Total Number of physical frames available (0 - 16k).
 * @param pfn_bits     Number of bits from the 64-bit physical addr. used for
 *                     the physical frame number.
 * @param vpn_bits     Number of bits from the 64-bit virtual addr. used for the
 *                     virtual page number.
 * @return             0 after successful initialization, any other value in
 *                     case of error.
 */
int setup(uint8_t tlb_max_size, uint16_t phy_frames, uint8_t pfn_bits,
          uint8_t vpn_bits);

/**
 * Structure used by translate() for result return. Contains translated physical
 * address and indicators if a TLB hit occured or a new frame had to be
 * allocated.
 */
typedef struct {
  uint64_t phy_address;
  bool tlb_hit;
  bool new_frame;
} res_translate;

/**
 * Translates given virtual_address into the corresponding physical address by
 * consulting the TLB first and then the page table. **Attention**: result is
 * written to the res_translate structure pointer.
 *
 * @param virtual_address The virtual address to translate.
 * @param[out] result     Pointer to the structure for returning the result.
 * @return                0 on success, 1 if no free frame could be allocated.
 */
int translate(uint64_t virtual_address, res_translate *result);

/**
 * Structure used by status() to return information about the current simulation
 * state.
 */
typedef struct {
  uint16_t free_frame_count;
  uint16_t allocated_page_count;
  uint16_t tlb_entries_count;
} res_status;

/**
 * Can be called to obtain the current state of the simulation, i.e. the number
 * of free frames, allocated pages and valid TLB entries. Result is written to
 * the res_status structure pointer.
 *
 * @param[out] result Pointer to the structure for returning the result.
 */
void status(res_status *result);

/**
 * Called at the end of the simulation. Frees all allocated resources.
 */
void teardown(void);

#endif
