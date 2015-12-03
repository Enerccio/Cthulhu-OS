#include "../commons.h"

#include <stdio.h>
#include <limits.h>

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)


typedef struct page
{
   uint64_t ignored    : 2;   // ignored
   uint64_t pwt        : 1;   // page-level write-through; indirectly determines the memory type used to access the PML4 table during linear-address translation (see Section 4.9.2)
   uint64_t pcd		   : 1;   // page-level cache disable; indirectly determines the memory type used to access the PML4 table during linear-address translation (see Section 4.9.2)
   uint64_t ignored2   : 7;   // ignored
   uint64_t address    : 54;  // rest is address + MAXPHYADDR
} page_t;



void initialize_paging();
