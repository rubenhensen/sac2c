/*
 *
 * $Id$
 *
 */

/*****************************************************************************
 *
 * file:
 *
 * prefix:
 *
 * description:
 *
 *
 *
 *
 *
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "heapmgr.h"

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

void *
SAC_HM_PlaceArray (void *alloc, void *base, long int offset, long int cache_size)
{
    long int cache_mask = cache_size - 1;
    long int alloc_addr = (long int)alloc;
    long int cond;

    long int alloc_caddr = alloc_addr & cache_mask;
    long int base_caddr = ((long int)base) & cache_mask;
    long int wanted_caddr, required_offset;
    SAC_HM_header_t *res_ptr;

    wanted_caddr = (base_caddr + offset) & cache_mask;

    required_offset = wanted_caddr - alloc_caddr;
    cond = required_offset - 2 * SAC_HM_UNIT_SIZE;

    if (cond < 0) {
        required_offset += cache_size;
    }

    res_ptr = (SAC_HM_header_t *)(alloc_addr + required_offset);

    SAC_HM_ADDR_ARENA (res_ptr) = (SAC_HM_arena_t *)(alloc_addr | (long int)1);

#if 0  
  fprintf(stderr,
          "AllocPlace: original: %ld\n"
          "            cache:    %ld\n"
          "            other:    %ld\n"
          "            cache:    %ld\n"
          "            offset:   %ld\n"
          "            new:      %ld\n"
          "            cache:    %ld\n\n",
          alloc_addr, alloc_addr & cache_mask,
          (long int) base, base_caddr, offset,
          (long int)res_ptr, ((long int)res_ptr) & cache_mask);
#endif
    return ((void *)res_ptr);
}
