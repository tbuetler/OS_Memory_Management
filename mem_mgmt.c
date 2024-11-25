#include "mem_mgmt.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

////////////////////// Public API

// Define the structures for TLB and Free Frame List
typedef struct {
    uint16_t vpn; // Virtual Page Number
    uint16_t pfn; // Physical Frame Number
    bool valid; // Indicates if the entry is valid
} TLBEntry;

// Global variables to hold memory management information
static TLBEntry *TLB; // The Translation Lookaside Buffer
static uint16_t *pageTable; // Page Table to map VPNs to PFNs
static bool *freeFrames; // Array to track which physical frames are free
static uint8_t tlbMaxSize; // Maximum number of entries in the TLB
static uint16_t totalFrames; // Total number of physical frames
static uint8_t pfnBits, vpnBits; // Number of bits for PFN and VPN
static uint8_t tlbIndex; // Index for FIFO replacement in the TLB
static uint16_t freeFrameCount; // Number of free frames
static uint16_t allocatedPages; // Number of pages that have been allocated
static uint8_t tlbEntriesCount; // Number of valid entries in the TLB

int setup(uint8_t tlb_max_size, uint16_t phy_frames, uint8_t pfn_bits, uint8_t vpn_bits) {
    // Store the parameters in global variables
    tlbMaxSize = tlb_max_size;
    totalFrames = phy_frames;
    pfnBits = pfn_bits;
    vpnBits = vpn_bits;

    // Allocate memory for the TLB, Page Table, and Free Frame List
    TLB = (TLBEntry *) calloc(tlbMaxSize, sizeof(TLBEntry)); // Allocate TLB entries

    if (!TLB) return -1; // Return -1 if allocation failed

    // Allocate memory for the page table
    pageTable = (uint16_t *) calloc((1 << vpnBits), sizeof(uint16_t));
    if (!pageTable) {
        free(TLB); // Free TLB if allocation failed
        return -1; // Return -1 if allocation failed
    }
    // Initialize all page table entries to -1 (indicating not mapped)
    memset(pageTable, -1, (1 << vpnBits) * sizeof(uint16_t));

    // Allocate free frame list
    freeFrames = (bool *) calloc(totalFrames, sizeof(bool));
    if (!freeFrames) {
        free(TLB); // Free TLB and page table if allocation failed
        free(pageTable);
        return -1; // Return -1 if allocation failed
    }
    // Mark all frames as free
    for (uint16_t i = 0; i < totalFrames; i++) {
        freeFrames[i] = true;
    }

    // Initialize other variables
    freeFrameCount = totalFrames;
    allocatedPages = 0;
    tlbEntriesCount = 0;
    tlbIndex = 0; // TLB replacement index starts at 0

    return 0; // Return 0 to indicate successful setup
}

int translate(uint64_t virtual_address, res_translate *result) {
    // Extract the VPN from the virtual address using bit shifting and masking
    uint16_t vpn = (virtual_address >> (64 - vpnBits)) & ((1 << vpnBits) - 1);
    // Extract the offset from the virtual address
    uint64_t offset = virtual_address & ((1 << (64 - pfnBits)) - 1);
    uint16_t pfn;

    // Check if the VPN is in the TLB (TLB hit check)
    bool tlbHit = false;
    for (uint8_t i = 0; i < tlbEntriesCount; i++) {
        if (TLB[i].valid && TLB[i].vpn == vpn) {
            pfn = TLB[i].pfn; // Get the PFN from the TLB
            tlbHit = true;
            break;
        }
    }

    if (tlbHit) {
        // TLB hit: use the PFN found and mark the result accordingly
        result->phy_address = ((uint64_t) pfn << (64 - pfnBits)) | offset;
        result->tlb_hit = true;
        result->new_frame = false;
        return 0;
    }

    // TLB miss: check the page table for the VPN
    if (pageTable[vpn] != (uint16_t) -1) {
        // Check if the page table entry is valid
        pfn = pageTable[vpn]; // Get the PFN from the page table
        // Add the new entry to the TLB using FIFO replacement
        TLB[tlbIndex].vpn = vpn;
        TLB[tlbIndex].pfn = pfn;
        TLB[tlbIndex].valid = true;
        tlbIndex = (tlbIndex + 1) % tlbMaxSize; // Update the TLB index
        if (tlbEntriesCount < tlbMaxSize) {
            tlbEntriesCount++; // Increase TLB entry count if not full
        }
        // Set the result
        result->phy_address = ((uint64_t) pfn << (64 - pfnBits)) | offset;
        result->tlb_hit = false;
        result->new_frame = false;
        return 0;
    }

    // Page not in memory: need to allocate a new frame
    int frame = -1;
    for (uint16_t i = 0; i < totalFrames; i++) {
        // Search for a free frame
        if (freeFrames[i]) {
            frame = i; // Found a free frame
            freeFrames[i] = false; // Mark the frame as used
            freeFrameCount--; // Decrease the count of free frames
            break;
        }
    }

    if (frame == -1) {
        // No free frames available: return an error
        return 1;
    }

    // Update the page table with the new frame
    pfn = (uint16_t) frame;
    pageTable[vpn] = pfn; // Map the VPN to the PFN
    // Add the new entry to the TLB using FIFO replacement
    TLB[tlbIndex].vpn = vpn;
    TLB[tlbIndex].pfn = pfn;
    TLB[tlbIndex].valid = true;
    tlbIndex = (tlbIndex + 1) % tlbMaxSize; // Update the TLB index
    if (tlbEntriesCount < tlbMaxSize) {
        tlbEntriesCount++; // Increase TLB entry count if not full
    }

    // Set the result with the new frame information
    result->phy_address = ((uint64_t) pfn << (64 - pfnBits)) | offset;
    result->tlb_hit = false;
    result->new_frame = true;
    allocatedPages++; // Increase the count of allocated pages

    return 0; // Return 0 to indicate successful translation
}

void status(res_status *result) {
    // Fill in the current status of the memory management system
    result->free_frame_count = freeFrameCount; // Number of free frames
    result->allocated_page_count = allocatedPages; // Number of allocated pages
    result->tlb_entries_count = tlbEntriesCount; // Number of valid TLB entries
}

void teardown(void) {
    // Free all allocated memory
    free(TLB);
    free(pageTable);
    free(freeFrames);
}
