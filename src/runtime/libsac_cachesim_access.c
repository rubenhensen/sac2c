/*
 * $Log$
 * Revision 1.1  1999/02/17 17:25:33  her
 * Initial revision
 *
 */

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "sac_cachesim.h"

#define SIMPOINTER(act_cl, cacheline, elm)    /* tCacheLevel*, unsigned, int */          \
    (act_cl->data +                           /* base */                                 \
     cacheline * act_cl->associativity + elm) /* offset */

#define SIMCACHE(pElm0, elm)  /* tCacheLevel*, unsigned, int */                          \
    (*(pElm0 + elm) & ~(1UL)) /* hide invalid-bit */

#define SIMINVALID(pElm0, elm) /* tCacheLevel*, unsigned, int */                         \
    (*(pElm0 + elm) & (1UL))   /* hide all but invalid-bit */

#define UPDATE_LRU_ORDER(pElm0, index, newvalue) /* ULINT*, int, ULINT */                \
    while (index < act_cl->associativity - 1) {                                          \
        *(pElm0 + index) = *(pElm0 + index + 1);                                         \
        index++;                                                                         \
    }                                                                                    \
    *(pElm0 + index) = newvalue;

#define L_BIT(entry) ((entry & 4 /*%00000100*/) >> 2)

#define C_BIT(entry) ((entry & 2 /*%00000010*/) >> 1)

#define S_BIT(entry) (entry & 1 /*%00000001*/)

#define PTR_SH_2ENTRY(sh_ary, index) /* char*, int */ (&sh_ary[index >> 1])

#define GET_SH_ENTRY(p2Entry, index) /* char*, int */                                    \
    ((*p2Entry                       /* 2 entries of sh_ary */                           \
      >> (4 * !(index & 1)))         /* move 4 bits right if row is even */              \
     & 15 /*%00001111*/              /* hide 4 highest bits */                           \
    )

#define SETABLE_SH_ENTRY(p2Entry, index, entry) /* char*, int, char */                   \
    ((*p2Entry                                  /* 2 old entries of sh_ary */            \
      & (15 /*%00001111*/ << (4 * (index & 1))) /* clear the affected entry */           \
      )                                                                                  \
     | (entry << (4 * !(index & 1))) /* insert value into affected entry */              \
    )

#define CONCAT(functionname, type) functionname##type
#define SAC_CS_ACCESS_DM(type) CONCAT (SAC_CS_AccessDM, type)
#define SAC_CS_ACCESS_AS4(type) CONCAT (SAC_CS_AccessAS4, type)
/* the macro TYPE will be defined in commandline of gcc:
 * gcc ... -D TYPE=xxx ...
 * also the macros TYPEREAD, TYPEWFOW, TYPEWWV and TYPEWWA */

/* BEGIN: The folowing variables are declared in the
 * ´libsac_cachesim_basic.c´-file. Changes there have to be done here
 * too!!!  */
extern tCacheLevel *SAC_CS_cachelevel[MAX_CACHELEVEL + 1];
/* SAC_CS_cachelevel[0] is unused */

extern ULINT SAC_CS_hit[MAX_CACHELEVEL + 1], SAC_CS_invalid[MAX_CACHELEVEL + 1],
  SAC_CS_miss[MAX_CACHELEVEL + 1], SAC_CS_cold[MAX_CACHELEVEL + 1],
  SAC_CS_cross[MAX_CACHELEVEL + 1], SAC_CS_self[MAX_CACHELEVEL + 1];
/* SAC_CS_xxx[0] is unused */

extern int SAC_CS_level;

extern tFunRWAccess SAC_CS_ReadAccess, SAC_CS_WriteAccess,
  SAC_CS_read_access_table[MAX_CACHELEVEL + 2],
  SAC_CS_write_access_table[MAX_CACHELEVEL + 2];
/* SAC_CS_xxx_access_table[0] is unused,
 * SAC_CS_xxx_access_table[MAX_CACHELEVEL+1]
 *   for dummy/MainMem */
/* END: */

void SAC_CS_ACCESS_DM (TYPE) (void *baseaddress, void *elemaddress)
{
    /* unsigned because of right-shift-operation ´>>´ */
    unsigned int cacheline;
    tCacheLevel *act_cl;
    int was_invalid;
    ULINT aligned_addr, aligned_base, *pElm0;
#ifndef TYPEWWA
    int i = 0, row;
    ULINT entry_addr, delta_entry_addr;
    char *act_sh_ary, *p2Entry, entry;
#endif

    act_cl = SAC_CS_cachelevel[SAC_CS_level];
    aligned_addr = ((ULINT)elemaddress) & act_cl->cls_mask;
    aligned_base = ((ULINT)baseaddress) & act_cl->cls_mask;
    cacheline = (aligned_addr & act_cl->is_mask) >> act_cl->cls_bits;
    pElm0 = SIMPOINTER (act_cl, cacheline, 0 /*elm0*/);
    /* pElm0 is a pointer to the cachecontense of element 0 within the
     * set (or list) belonging to the affected cacheline */

    /* check if invalid-flag has been set */
    was_invalid = SIMINVALID (pElm0, 0);
    if ((SIMCACHE (pElm0, 0) == aligned_addr) && !was_invalid) {
        /* we have a hit */
        SAC_CS_hit[SAC_CS_level]++;
        SAC_CS_level = 1;
        return; /* leave function here */
    } else {
        /* we have a miss */

#ifndef TYPEWWA
        /* BEGIN: run through all shadowarrays and their affected entries */
        delta_entry_addr = act_cl->cachelinesize * act_cl->nr_cachelines;
        i = 0;
        while (i < MAX_SHADOWARRAYS && act_cl->shadowarrays[i] != NULL) {
            act_sh_ary = act_cl->shadowarrays[i];
            /* set ´row´ to index of the first affected entry
             * within ´act_sh_ary´ */
            row = ((cacheline + act_cl->nr_cachelines)
                   - ((act_cl->shadowbases[i] & act_cl->is_mask) >> act_cl->cls_bits)
                   /* that is the cacheline of the  shadowbaseaddress */
                   )
                  % act_cl->nr_cachelines;
            entry_addr = (act_cl->shadowbases[i] & act_cl->cls_mask)
                         /* that is the aligned shadowbaseaddress */
                         + (act_cl->cachelinesize * row);
            /* run through all affected entries (all in the same row) */
            while (entry_addr <= act_cl->shadowalignedtop[i]) {
                p2Entry = PTR_SH_2ENTRY (act_sh_ary, row);
                entry = GET_SH_ENTRY (p2Entry, row);
                if (aligned_addr == entry_addr) {
                    SAC_CS_cold[SAC_CS_level] += !L_BIT (entry);
                    SAC_CS_cross[SAC_CS_level] += C_BIT (entry);
                    SAC_CS_self[SAC_CS_level] += S_BIT (entry);
                    entry = 4 /*%00000100*/; /* set only l-bit */
                } else {
                    entry = entry
                            | (L_BIT (entry)
                               << ((ULINT)baseaddress != act_cl->shadowbases[i]));
                }
                *p2Entry = SETABLE_SH_ENTRY (p2Entry, row, entry);
                /* delta_entry_addr == act_cl->cachelinesize*act_cl->nr_cachelines */
                entry_addr += delta_entry_addr;
                row += act_cl->nr_cachelines;
            } /* while */
            i++;
        } /* while: i */
          /* END: run through all shadowarrays and their affected entries */
#endif

        SAC_CS_miss[SAC_CS_level]++;
        SAC_CS_invalid[SAC_CS_level] += was_invalid;

#ifdef TYPEWWA
#elif defined(TYPEWWV)
        /* set new cache-contens */
        *pElm0 = aligned_addr | (1UL) /*make invalid*/;
#else
        /* set new cache-contens */
        *pElm0 = aligned_addr;
#endif

#ifdef TYPEWWA
#elif defined(TYPEREAD)
        /* seek in next CacheLevel for elemaddress */
        SAC_CS_level++;
        SAC_CS_read_access_table[SAC_CS_level](baseaddress, elemaddress);
#else
        /* seek in next CacheLevel for elemaddress */
        SAC_CS_level++;
        SAC_CS_write_access_table[SAC_CS_level](baseaddress, elemaddress);
#endif
        return; /* leave function here */
    }           /* if-else:hit or miss */
} /* SAC_CS_AccessDM */

/* !!! FOR TESTS ONLY XXX !!! */
#
#
#define SAC_CS_ACCESS_AS4_(type) CONCAT (SAC_CS_AccessAS4_, type)

void SAC_CS_ACCESS_AS4 (TYPE) (void *baseaddress, void *elemaddress)
{
    /* unsigned because of right-shift-operation ´>>´ */
    unsigned int cacheline;
    tCacheLevel *act_cl;
    int elm, was_invalid;
    ULINT aligned_addr, aligned_base, newlast, *pElm0;
#ifndef TYPEWWA
    int i = 0, row;
    ULINT entry_addr, delta_entry_addr;
    char *act_sh_ary, *p2Entry, entry;
#endif

    act_cl = SAC_CS_cachelevel[SAC_CS_level];
    aligned_addr = ((ULINT)elemaddress) & act_cl->cls_mask;
    aligned_base = ((ULINT)baseaddress) & act_cl->cls_mask;
    cacheline = (aligned_addr & act_cl->is_mask) >> act_cl->cls_bits;
    pElm0 = SIMPOINTER (act_cl, cacheline, 0 /*elm0*/);
    /* pElm0 is a pointer to the cachecontents of element 0 within the
     * set (or list) belonging to the affected cacheline */

    elm = act_cl->associativity - 1;
    while ((elm >= 0) && (SIMCACHE (pElm0, elm) != aligned_addr)) {
        elm--;
    } /* while:elm */
    /* check if invalid-flag has been set */
    was_invalid = SIMINVALID (pElm0, elm);
    if ((elm >= 0) && !was_invalid) {
        /* we have a hit */
        SAC_CS_hit[SAC_CS_level]++;
        /* update LRU-order */
        newlast = *(pElm0 + elm);
        UPDATE_LRU_ORDER (pElm0, elm, newlast);

        SAC_CS_level = 1;
        return; /* leave function here */
    } else {
        /* we have a miss */

#ifndef TYPEWWA
        /* BEGIN: run through all shadowarrays and their affected entries */
        delta_entry_addr = act_cl->cachelinesize * act_cl->nr_cachelines;
        i = 0;
        while (i < MAX_SHADOWARRAYS && act_cl->shadowarrays[i] != NULL) {
            act_sh_ary = act_cl->shadowarrays[i];
            /* set ´row´ to index of the first affected entry
             * within ´act_sh_ary´ */
            row = ((cacheline + act_cl->nr_cachelines)
                   - ((act_cl->shadowbases[i] & act_cl->is_mask) >> act_cl->cls_bits)
                   /* that is the cacheline of the  shadowbaseaddress */
                   )
                  % act_cl->nr_cachelines;
            entry_addr = (act_cl->shadowbases[i] & act_cl->cls_mask)
                         /* that is the aligned shadowbaseaddress */
                         + (act_cl->cachelinesize * row);
            /* run through all affected entries (all in the same row) */
            while (entry_addr <= act_cl->shadowalignedtop[i]) {
                p2Entry = PTR_SH_2ENTRY (act_sh_ary, row);
                entry = GET_SH_ENTRY (p2Entry, row);
                if (aligned_addr == entry_addr) {
                    SAC_CS_cold[SAC_CS_level] += !L_BIT (entry);
                    SAC_CS_cross[SAC_CS_level] += C_BIT (entry);
                    SAC_CS_self[SAC_CS_level] += S_BIT (entry);
                    entry = 4 /*%00000100*/; /* set only l-bit */
                } else {
                    entry = entry
                            | (L_BIT (entry)
                               << ((ULINT)baseaddress != act_cl->shadowbases[i]));
                }
                *p2Entry = SETABLE_SH_ENTRY (p2Entry, row, entry);
                /* delta_entry_addr == act_cl->cachelinesize*act_cl->nr_cachelines */
                entry_addr += delta_entry_addr;
                row += act_cl->nr_cachelines;
            } /* while:entry_addr */
            i++;
        } /* while:i */
          /* END: run through all shadowarrays and their affected entries */
#endif

        SAC_CS_miss[SAC_CS_level]++;
        SAC_CS_invalid[SAC_CS_level] += was_invalid;

#ifdef TYPEWWA
#elif defined(TYPEWWV)
        /* update LRU-order and set new cache-contens */
        i = was_invalid * elm;
        /* this is aquivalent to:
         * i=(was_invalid  ?  i=elm/the invalid/  :  0/the LRU/); */
        UPDATE_LRU_ORDER (pElm0, i, aligned_addr | (1UL) /*make invalid*/);
#else
        /* update LRU-order and set new cache-contens */
        i = was_invalid * elm;
        /* this is aquivalent to:
         * i=(was_invalid  ?  i=elm/the invalid/  :  0/the LRU/); */
        UPDATE_LRU_ORDER (pElm0, i, aligned_addr);
#endif

#ifdef TYPEWWA
#elif defined(TYPEREAD)
        /* seek in next CacheLevel for elemaddress */
        SAC_CS_level++;
        SAC_CS_read_access_table[SAC_CS_level](baseaddress, elemaddress);
#else
        /* seek in next CacheLevel for elemaddress */
        SAC_CS_level++;
        SAC_CS_write_access_table[SAC_CS_level](baseaddress, elemaddress);
#endif
        return; /* leave function here */
    }           /* if-else:hit or miss */
} /* SAC_CS_AccessAS4 */
