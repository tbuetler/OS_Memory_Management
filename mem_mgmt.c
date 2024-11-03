#include "mem_mgmt.h"
#include <stdint.h>

////////////////////// Public API

int setup(uint8_t tlb_max_size, uint16_t phy_frames, uint8_t pfn_bits,
          uint8_t vpn_bits) {
  // TODO
  return 0;
}

int translate(uint64_t virtual_address, res_translate *result) {
  // TODO
  return 0;
}

void status(res_status *result) {
  // TODO
}

void teardown(void) {
  // TODO
}
