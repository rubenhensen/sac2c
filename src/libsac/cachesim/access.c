/***********************************************************************
 *                                                                     *
 *                      Copyright (c) 1994-2007                        *
 *         SAC Research Foundation (http://www.sac-home.org/)          *
 *                                                                     *
 *                        All Rights Reserved                          *
 *                                                                     *
 *   The copyright holder makes no warranty of any kind with respect   *
 *   to this product and explicitly disclaims any implied warranties   *
 *   of merchantability or fitness for any particular purpose.         *
 *                                                                     *
 ***********************************************************************/

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#include "basic.h"
#include "cachesim.h"

/******************************************************************************
 *
 * macro(function):
 *   CACHE_PELM0(...)
 *     tCacheLevel  act_cl   : pointer to the actual cachelevel
 *     unsigned int cacheline: the number of the cacheline
 *
 * macro(function):
 *   CACHE_CONTENTS(...)
 *     ULINT* pElm0: pointer to element 0 of cacheline
 *     int    pos  : offset to the wanted element
 *
 * macro(function):
 *   CACHE_INVALID(...)
 *     ULINT* pElm0: pointer to element 0 of cacheline
 *     int    pos  : offset to the wanted element
 *
 * description:
 *   CACHE_PELM0(...) evaluates a pointer to the element 0 within a set/list
 *   belonging to a cacheline.
 *   A cacheentry consists of an invalid-bit and an address. Both are coded
 *   into the same ULINT. The two macros CACHE_CONTENTS(...) and
 *   CACHE_INVALID(...) isolate the wanted data and hide the other.
 *
 *****************************************************************************/
#define CACHE_PELM0(act_cl, cacheline)                                                   \
    (act_cl->data +                     /* base */                                       \
     cacheline * act_cl->associativity) /* offset */

#define CACHE_CONTENTS(pElm0, pos) (*(pElm0 + pos) & ~(1UL)) /* hide invalid-bit */

#define CACHE_INVALID(pElm0, pos) (*(pElm0 + pos) & (1UL)) /* hide all but invalid-bit   \
                                                            */

/******************************************************************************
 *
 * macro(procedure):
 *   void UPDATE_LRU_QUEUE(...)
 *     ULINT* pElm0    : pointer to element 0 within a cacheline
 *     int    removepos: position in LRU-queue which has to be removed
 *     ULINT  newvalue : new value which will be added (right)
 *
 * description:
 *   Removes an address given by removepos and adds a new address at the
 *   end of the LRU-queue (Last Recently Used-queue).
 *   The LRU-queue and the set belonging to a cacheline are the same, because
 *   it is never neccessary to know in which element of the set a address
 *   has been found!!!
 *
 *****************************************************************************/
#define UPDATE_LRU_QUEUE(pElm0, removepos, newvalue) /* ULINT*, int, ULINT */            \
    while (removepos < act_cl->associativity - 1) {                                      \
        *(pElm0 + removepos) = *(pElm0 + removepos + 1);                                 \
        removepos++;                                                                     \
    }                                                                                    \
    *(pElm0 + removepos) = newvalue;

/******************************************************************************
 *
 * macro(function):
 *   char x_BIT(...)
 *     char entry : a char which stores the entry in the low/right bits
 *
 * description:
 *   Returns a 1 if bit x is set in the entry and a 0 if not.
 *
 *****************************************************************************/
#define L_BIT(entry) ((entry & 4 /*%00000100*/) >> 2)

#define C_BIT(entry) ((entry & 2 /*%00000010*/) >> 1)

#define S_BIT(entry) (entry & 1 /*%00000001*/)

/******************************************************************************
 *
 * macro(function):
 *   char* GET_SH_PTR_2ENTRY(...)
 *     char* sh_ary : pointer to the actual shadowarray
 *     int   index  : index of an entry
 *
 * macro(function):
 *   char* GET_SH_ENTRY(...)
 *     char* p2Entry : pointer to char which stores the entry given by index
 *     int   index   : index of an entry
 *
 * macro(function):
 *   char* SETABLE_SH_ENTRY(...)
 *     char* p2Entry : pointer to char which stores the entry given by index
 *     int   index   : index of an entry
 *     char  entry   : new value for the entry given by index
 *
 * description:
 *   Because one entry of a shadowarray consists of 3 bit a char stores
 *   two entries.
 *   GET_SH_PTR_2ENTRY(...) evaluates a pointer to a char. One entry
 *   within this char is the one given by the index.
 *   Getting a pointer to the affected char (evaluated by GET_SH_PTR_2ENTRY)
 *   and the index GET_SH_ENTRY(...) evaluates a char. This char stores
 *   the entry given by the index in the low/right bits
 *   (the other bits are 0 !!!).
 *   Getting a pointer to the affected char (evaluated by GET_SH_PTR_2ENTRY),
 *   the index of an entry and the new value for this entry
 *   SETABLE_SH_ENTRY(...) evaluates a char. This char stores the new value
 *   and the unmodified other entry (both at the correct position).
 *   If p is the pointer to the affected char you can set the new entry
 *   this way:
 *     *p = SETABLE_SH_ENTRY(p, index, newvalue)
 *
 *****************************************************************************/
#define GET_SH_PTR_2ENTRY(sh_ary, index) /* char*, int */ (&sh_ary[index >> 1])

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

/******************************************************************************
 *
 * macro(function):
 *   ULINT GET_CACHELINE(...)
 *     tCacheLevel act_cl : pointer to the actual cachelevel
 *     ULINT       address: address of an arrayelement
 *
 * description:
 *   See macroname.
 *
 *****************************************************************************/
#define GET_CACHELINE(act_cl, address) ((address & act_cl->is_mask) >> act_cl->cls_bits)

/******************************************************************************
 *
 * macro(function):
 *   ULINT GET_ALIGNEDADDR(...)
 *     tCacheLevel act_cl : pointer to the actual cachelevel
 *     ULINT       address: address of an arrayelement
 *
 * description:
 *   Evaluates an address which is aligned (1st element in cacheline)
 *   to the cachelines of the actual cachelevel.
 *
 *****************************************************************************/
#define GET_ALIGNEDADDR(act_cl, address) (address & act_cl->cls_mask)

/******************************************************************************
 *
 * macro(procedure):
 *   void INIT_BASIC_VALUES(...)
 *     n/a           dummy_IN    : a dummy only to seperate input from output
 *     tCacheLevel*  cachelevel[]: a array keeping pointers to all cachelevels
 *     int           level       : number of the actual cachelevel
 *     void*         elemaddress : address of an arrayelement
 *     n/a           dummy_OUT   : a dummy only to seperate input from output
 *     tCacheLevel** act_cl      : evaluated pointer to actual cachelevel
 *     ULINT*        aligned_addr: evaluated aligned address to the elemaddress
 *     unsigned int* cacheline   : evaluated cacheline of aligned address
 *
 * description:
 *   Depending on the input parameters there are evaluated some values,
 *   which are often used.
 *
 *****************************************************************************/
#define INIT_BASIC_VALUES(dummy_IN, cachelevel, level, elemaddress, dummy_OUT, act_cl,   \
                          aligned_addr, cacheline)                                       \
    act_cl = cachelevel[level];                                                          \
    aligned_addr = GET_ALIGNEDADDR (act_cl, (ULINT)elemaddress);                         \
    cacheline = GET_CACHELINE (act_cl, aligned_addr);

/* These macros are used to expand the functionnames by an ending 'S'
 * (for simple analysis) or an ending 'D' (for detailed analysis)
 */
#define CONCAT(functionname, type) functionname##type

#define SAC_CS_ACCESS_DMREAD(type) CONCAT (SAC_CS_Access_DMRead_, type)
#define SAC_CS_ACCESS_DMFOW(type) CONCAT (SAC_CS_Access_DMFOW_, type)
#define SAC_CS_ACCESS_DMWV(type) CONCAT (SAC_CS_Access_DMWV_, type)
#define SAC_CS_ACCESS_DMWA(type) CONCAT (SAC_CS_Access_DMWA_, type)
#define SAC_CS_ACCESS_AS4READ(type) CONCAT (SAC_CS_Access_AS4Read_, type)
#define SAC_CS_ACCESS_AS4FOW(type) CONCAT (SAC_CS_Access_AS4FOW_, type)
#define SAC_CS_ACCESS_AS4WV(type) CONCAT (SAC_CS_Access_AS4WV_, type)
#define SAC_CS_ACCESS_AS4WA(type) CONCAT (SAC_CS_Access_AS4WA_, type)

#define SAC_CS_DETAILEDANALYSIS(type) CONCAT (SAC_CS_DetailedAnalysis_, type)
/* the macro TYPE will be defined in commandline of gcc:
 * gcc ... -D TYPE=x ...
 */

/* this forwadrdeclaration is necessary: DetailedAnalysis_Write is
 * declared and will be used in libsac_cachesim_access_advanced.o  but
 * DetailedAnalysis_Read is declared in libsac_cachesim_access.o but
 * will also be used in libsac_cachesim_access_advanced.o */
extern void SAC_CS_DetailedAnalysis_Read (tCacheLevel *act_cl, void *baseaddress,
                                          ULINT aligned_addr, unsigned cacheline);

/******************************************************************************
 *
 * function:
 *   int GetPosWithinSet(...)
 *     int*         pWasInvalid  : pointer to a flag
 *     tCacheLevel* act_cl       : pointer to the actual cachelevel
 *     ULINT*       pElm0        : pointer to element 0 of cacheline
 *     ULINT        aligned_addr : aligned address of an arrayelement
 *
 * description:
 *   Tries to find the aligned_addr in the set/list which is given by
 *   pElm0. If the aligned_addr was not found GetPosWithinSet(...)
 *   returns -1 and *pWasInvalid will be set to 0. Otherwise
 *   GetPosWithinSet(...) returns the number of the element where the
 *   aligned_addr has been found and if the invalid-flag was set *pWasInvalid
 *   will be set to 1 else to 0.
 *
 *****************************************************************************/
static int
GetPosWithinSet (int *pWas_invalid, tCacheLevel *act_cl, ULINT *pElm0, ULINT aligned_addr)
{
    int pos;

    pos = act_cl->associativity - 1;
    while ((pos >= 0) && (CACHE_CONTENTS (pElm0, pos) != aligned_addr)) {
        pos--;
    } /* while:pos */

    /* check if invalid-flag has been set at found pos */
    *pWas_invalid = (pos >= 0 ? CACHE_INVALID (pElm0, pos) : 0);
    return (pos);
} /* GetPosWithinSet */

/******************************************************************************
 *
 * procedure:
 *   void DetailedAnalysis(...)
 *     tCacheLevel* act_cl       : pointer to the actual cachelevel
 *     void*        baseaddress  : baseaddress of the array
 *     ULINT        aligned_addr : aligned address of an arrayelement
 *     unsigned int cacheline    : number of the cacheline of the arrayelement
 *
 * description:
 *   Generally speaking this procedure does the detailed analysis by
 *   classifying the misses as cold-start, self- or crossiterference.
 *   Therefor it uses the additional information provided by the entry 'E'
 *   of the shadowarray, which corresponds to the given aligned_addr.
 *   Furthermore all enties of the shadowarrays, which correspond to the
 *   same cacheline as 'E', will be updated.
 *
 *****************************************************************************/
#if DETAILED
#define SAC_CS_READORWRITE Write
#else
#define SAC_CS_READORWRITE Read
#endif
void SAC_CS_DETAILEDANALYSIS (SAC_CS_READORWRITE) (tCacheLevel *act_cl, void *baseaddress,
                                                   ULINT aligned_addr, unsigned cacheline)
{
    ULINT entry_addr, delta_entry_addr, nr_cachelines;
    unsigned i, idx;
    char *act_sh_ary, *p2Entry, entry;

    /*printf("DetailedAnalysis: enter\n");*/
    /* BEGIN: run through all shadowarrays and their affected entries
     */
    delta_entry_addr = act_cl->cachelinesize * act_cl->nr_cachelines;
    /* delta_entry_addr is the distance between two affected entries
     * within a shadowarray */
    nr_cachelines = act_cl->nr_cachelines;
    i = 0;
    while (i < MAX_SHADOWARRAYS && act_cl->shadowarrays[i] != NULL) {
        act_sh_ary = act_cl->shadowarrays[i];
        /* set 'idx' to index of the first affected entry
         * within 'act_sh_ary' */
        idx = (cacheline - GET_CACHELINE (act_cl, act_cl->shadowbases[i]) + nr_cachelines)
              % nr_cachelines; /* to get a positiv result */
        entry_addr = GET_ALIGNEDADDR (act_cl, act_cl->shadowbases[i])
                     + (act_cl->cachelinesize * idx);
        /* run through all affected entries */
        while (entry_addr <= act_cl->shadowalignedtop[i]) {
            p2Entry = GET_SH_PTR_2ENTRY (act_sh_ary, idx);
            entry = GET_SH_ENTRY (p2Entry, idx);
            if (aligned_addr == entry_addr) {
#if DETAILED
                SAC_CS_wcold[SAC_CS_level] += !L_BIT (entry);
                SAC_CS_wcross[SAC_CS_level] += C_BIT (entry);
                SAC_CS_wself[SAC_CS_level] += S_BIT (entry);
#else
                SAC_CS_rcold[SAC_CS_level] += !L_BIT (entry);
                SAC_CS_rcross[SAC_CS_level] += C_BIT (entry);
                SAC_CS_rself[SAC_CS_level] += S_BIT (entry);
#endif
                entry = 4 /*%00000100*/; /* set only l-bit */
            } else {
                entry
                  = entry
                    | (L_BIT (entry) << ((ULINT)baseaddress != act_cl->shadowbases[i]));
            } /* if */
            if (p2Entry == NULL) {
                printf ("p2Entry ==NULL\n");
                exit (1);
            }
            *p2Entry = SETABLE_SH_ENTRY (p2Entry, idx, entry);
            entry_addr += delta_entry_addr;
            idx += nr_cachelines;
        } /* while:entry_addr */
        i++;
    } /* while:i */
      /* END: run through all shadowarrays and their affected entries */
      /*printf("DetailedAnalysis: exit\n");*/
} /* DetailedAnalysis */

void SAC_CS_ACCESS_DMREAD (TYPE) (void *baseaddress, void *elemaddress)
{
    /* unsigned because of right-shift-operation '>>' */
    unsigned int cacheline;
    tCacheLevel *act_cl;
    int was_invalid;
    ULINT aligned_addr, *pElm0;

    INIT_BASIC_VALUES (IN:, SAC_CS_cachelevel, SAC_CS_level, elemaddress, OUT:, act_cl,
                       aligned_addr, cacheline);

    /* pElm0 is a pointer to the cachecontents of element 0 within the
     * set (or list) belonging to the affected cacheline
     */
    pElm0 = CACHE_PELM0 (act_cl, cacheline);
    was_invalid = CACHE_INVALID (pElm0, 0);
    if ((CACHE_CONTENTS (pElm0, 0) == aligned_addr) && !was_invalid) {
        /* we have a hit */
        SAC_CS_rhit[SAC_CS_level]++;

        SAC_CS_level = 1; /* next access has to seek in cachelevel 1 again */
        return;           /* leave function here */
    } else {
        /*printf("RDM: enter miss-case\n");*/
        /* we have a miss */
        SAC_CS_rmiss[SAC_CS_level]++;
        SAC_CS_rinvalid[SAC_CS_level] += was_invalid;

#if DETAILED
        /* run through all shadowarrays and their affected entries */
        SAC_CS_DetailedAnalysis_Read (act_cl, baseaddress, aligned_addr, cacheline);
#endif

        /* set new cachecontents */
        *pElm0 = aligned_addr;

        /* seek in next CacheLevel for elemaddress */
        SAC_CS_level++;
        SAC_CS_read_access_table[SAC_CS_level](baseaddress, elemaddress);

        /*printf("RDM: exit\n");*/
        return; /* leave function here */
    }           /* if-else:hit or miss */
} /* SAC_CS_Access_DMRead */

void SAC_CS_ACCESS_DMFOW (TYPE) (void *baseaddress, void *elemaddress)
{
    /* unsigned because of right-shift-operation '>>' */
    unsigned int cacheline;
    tCacheLevel *act_cl;
    int was_invalid;
    ULINT aligned_addr, *pElm0;

    INIT_BASIC_VALUES (IN:, SAC_CS_cachelevel, SAC_CS_level, elemaddress, OUT:, act_cl,
                       aligned_addr, cacheline);

    /* pElm0 is a pointer to the cachecontents of element 0 within the
     * set (or list) belonging to the affected cacheline
     */
    pElm0 = CACHE_PELM0 (act_cl, cacheline);
    was_invalid = CACHE_INVALID (pElm0, 0);
    if ((CACHE_CONTENTS (pElm0, 0) == aligned_addr) && !was_invalid) {
        /* we have a hit */
        SAC_CS_whit[SAC_CS_level]++;

        SAC_CS_level = 1; /* next access has to seek in cachelevel 1 again */
        return;           /* leave function here */
    } else {
        /*printf("WFOW: enter miss-case\n");*/
        /* we have a miss */
        SAC_CS_wmiss[SAC_CS_level]++;
        SAC_CS_winvalid[SAC_CS_level] += was_invalid;

#if DETAILED
        /* run through all shadowarrays and their affected entries */
        SAC_CS_DetailedAnalysis_Write (act_cl, baseaddress, aligned_addr, cacheline);
#endif

        /* set new cachecontents */
        *pElm0 = aligned_addr;

        /* seek in next CacheLevel for elemaddress */
        SAC_CS_level++;
        SAC_CS_write_access_table[SAC_CS_level](baseaddress, elemaddress);

        /*printf("WFOW: exit\n");*/
        return; /* leave function here */
    }           /* if-else:hit or miss */
} /* SAC_CS_Access_DMFOW */

void SAC_CS_ACCESS_DMWV (TYPE) (void *baseaddress, void *elemaddress)
{
    /* unsigned because of right-shift-operation '>>' */
    unsigned int cacheline;
    tCacheLevel *act_cl;
    int was_invalid;
    ULINT aligned_addr, *pElm0;

    INIT_BASIC_VALUES (IN:, SAC_CS_cachelevel, SAC_CS_level, elemaddress, OUT:, act_cl,
                       aligned_addr, cacheline);

    /* pElm0 is a pointer to the cachecontents of element 0 within the
     * set (or list) belonging to the affected cacheline
     */
    pElm0 = CACHE_PELM0 (act_cl, cacheline);
    was_invalid = CACHE_INVALID (pElm0, 0);
    if ((CACHE_CONTENTS (pElm0, 0) == aligned_addr) && !was_invalid) {
        /* we have a hit */
        SAC_CS_whit[SAC_CS_level]++;

        SAC_CS_level = 1; /* next access has to seek in cachelevel 1 again */
        return;           /* leave function here */
    } else {
        /* we have a miss */
        SAC_CS_wmiss[SAC_CS_level]++;
        SAC_CS_winvalid[SAC_CS_level] += was_invalid;

#if DETAILED
        /* run through all shadowarrays and their affected entries */
        SAC_CS_DetailedAnalysis_Write (act_cl, baseaddress, aligned_addr, cacheline);
#endif

        /* set new cachecontents */
        *pElm0 = aligned_addr | (1UL) /*make invalid*/;

        /* seek in next CacheLevel for elemaddress */
        SAC_CS_level++;
        SAC_CS_write_access_table[SAC_CS_level](baseaddress, elemaddress);

        return; /* leave function here */
    }           /* if-else:hit or miss */
} /* SAC_CS_Access_DMWV */

void SAC_CS_ACCESS_DMWA (TYPE) (void *baseaddress, void *elemaddress)
{
    /* unsigned because of right-shift-operation '>>' */
    unsigned int cacheline;
    tCacheLevel *act_cl;
    int was_invalid;
    ULINT aligned_addr, *pElm0;

    INIT_BASIC_VALUES (IN:, SAC_CS_cachelevel, SAC_CS_level, elemaddress, OUT:, act_cl,
                       aligned_addr, cacheline);

    /* pElm0 is a pointer to the cachecontents of element 0 within the
     * set (or list) belonging to the affected cacheline
     */
    pElm0 = CACHE_PELM0 (act_cl, cacheline);
    was_invalid = CACHE_INVALID (pElm0, 0);
    if ((CACHE_CONTENTS (pElm0, 0) == aligned_addr) && !was_invalid) {
        /* we have a hit */
        SAC_CS_whit[SAC_CS_level]++;

        SAC_CS_level = 1; /* next access has to seek in cachelevel 1 again */
        return;           /* leave function here */
    } else {
        /* we have a miss */
        SAC_CS_wmiss[SAC_CS_level]++;
        SAC_CS_winvalid[SAC_CS_level] += was_invalid;

#if DETAILED
        /* run through all shadowarrays and their affected entries */
        SAC_CS_DetailedAnalysis_Write (act_cl, baseaddress, aligned_addr, cacheline);
#endif

        return; /* leave function here */
    }           /* if-else:hit or miss */
} /* SAC_CS_Access_DMWA */

void SAC_CS_ACCESS_AS4READ (TYPE) (void *baseaddress, void *elemaddress)
{
    /* unsigned because of right-shift-operation '>>' */
    unsigned int cacheline;
    tCacheLevel *act_cl;
    int pos, removepos, was_invalid;
    ULINT aligned_addr, newvalue, *pElm0;

    INIT_BASIC_VALUES (IN:, SAC_CS_cachelevel, SAC_CS_level, elemaddress, OUT:, act_cl,
                       aligned_addr, cacheline);

    /* pElm0 is a pointer to the cachecontents of element 0 within the
     * set (or list) belonging to the affected cacheline
     */
    pElm0 = CACHE_PELM0 (act_cl, cacheline);
    pos = GetPosWithinSet (&was_invalid, act_cl, pElm0, aligned_addr);
    if ((pos >= 0) && !was_invalid) {
        /* we have a hit */
        SAC_CS_rhit[SAC_CS_level]++;
        /* update LRU-order */
        newvalue = *(pElm0 + pos);
        UPDATE_LRU_QUEUE (pElm0, pos, newvalue);

        SAC_CS_level = 1; /* next access has to seek in cachelevel 1 again */
        return;           /* leave function here */
    } else {
        /* we have a miss */
        SAC_CS_rmiss[SAC_CS_level]++;
        SAC_CS_rinvalid[SAC_CS_level] += was_invalid;

#if DETAILED
        /* run through all shadowarrays and their affected entries */
        SAC_CS_DetailedAnalysis_Read (act_cl, baseaddress, aligned_addr, cacheline);
#endif

        /* update LRU-order and set new cachecontents */
        removepos = was_invalid * pos;
        /* this is aquivalent to:
         * removepos=(was_invalid  ?  pos/the invalid/  :  0/the LRU/); */
        UPDATE_LRU_QUEUE (pElm0, removepos, aligned_addr);

        /* seek in next CacheLevel for elemaddress */
        SAC_CS_level++;
        SAC_CS_read_access_table[SAC_CS_level](baseaddress, elemaddress);

        return; /* leave function here */
    }           /* if-else:hit or miss */
} /* SAC_CS_Access_AS4Read */

void SAC_CS_ACCESS_AS4FOW (TYPE) (void *baseaddress, void *elemaddress)
{
    /* unsigned because of right-shift-operation '>>' */
    unsigned int cacheline;
    tCacheLevel *act_cl;
    int pos, removepos, was_invalid;
    ULINT aligned_addr, newvalue, *pElm0;

    INIT_BASIC_VALUES (IN:, SAC_CS_cachelevel, SAC_CS_level, elemaddress, OUT:, act_cl,
                       aligned_addr, cacheline);

    /* pElm0 is a pointer to the cachecontents of element 0 within the
     * set (or list) belonging to the affected cacheline
     */
    pElm0 = CACHE_PELM0 (act_cl, cacheline);
    pos = GetPosWithinSet (&was_invalid, act_cl, pElm0, aligned_addr);
    if ((pos >= 0) && !was_invalid) {
        /* we have a hit */
        SAC_CS_whit[SAC_CS_level]++;
        /* update LRU-order */
        newvalue = *(pElm0 + pos);
        UPDATE_LRU_QUEUE (pElm0, pos, newvalue);

        SAC_CS_level = 1; /* next access has to seek in cachelevel 1 again */
        return;           /* leave function here */
    } else {
        /* we have a miss */
        SAC_CS_wmiss[SAC_CS_level]++;
        SAC_CS_winvalid[SAC_CS_level] += was_invalid;

#if DETAILED
        /* run through all shadowarrays and their affected entries */
        SAC_CS_DetailedAnalysis_Write (act_cl, baseaddress, aligned_addr, cacheline);
#endif

        /* update LRU-order and set new cachecontents */
        removepos = was_invalid * pos;
        /* this is aquivalent to:
         * removepos=(was_invalid  ?  pos/the invalid/  :  0/the LRU/); */
        UPDATE_LRU_QUEUE (pElm0, removepos, aligned_addr);

        /* seek in next CacheLevel for elemaddress */
        SAC_CS_level++;
        SAC_CS_write_access_table[SAC_CS_level](baseaddress, elemaddress);

        return; /* leave function here */
    }           /* if-else:hit or miss */
} /* SAC_CS_Access_AS4FOW */

void SAC_CS_ACCESS_AS4WV (TYPE) (void *baseaddress, void *elemaddress)
{
    /* unsigned because of right-shift-operation '>>' */
    unsigned int cacheline;
    tCacheLevel *act_cl;
    int pos, removepos, was_invalid;
    ULINT aligned_addr, newvalue, *pElm0;

    INIT_BASIC_VALUES (IN:, SAC_CS_cachelevel, SAC_CS_level, elemaddress, OUT:, act_cl,
                       aligned_addr, cacheline);

    /* pElm0 is a pointer to the cachecontents of element 0 within the
     * set (or list) belonging to the affected cacheline
     */
    pElm0 = CACHE_PELM0 (act_cl, cacheline);
    pos = GetPosWithinSet (&was_invalid, act_cl, pElm0, aligned_addr);
    if ((pos >= 0) && !was_invalid) {
        /* we have a hit */
        SAC_CS_whit[SAC_CS_level]++;
        /* update LRU-order */
        newvalue = *(pElm0 + pos);
        UPDATE_LRU_QUEUE (pElm0, pos, newvalue);

        SAC_CS_level = 1; /* next access has to seek in cachelevel 1 again */
        return;           /* leave function here */
    } else {
        /* we have a miss */
        SAC_CS_wmiss[SAC_CS_level]++;
        SAC_CS_winvalid[SAC_CS_level] += was_invalid;

#if DETAILED
        /* run through all shadowarrays and their affected entries */
        SAC_CS_DetailedAnalysis_Write (act_cl, baseaddress, aligned_addr, cacheline);
#endif

        /* update LRU-order and set new cachecontents */
        removepos = was_invalid * pos;
        /* this is aquivalent to:
         * removepos=(was_invalid  ?  pos/the invalid/  :  0/the LRU/); */
        UPDATE_LRU_QUEUE (pElm0, removepos, aligned_addr | (1UL) /*make invalid*/);

        /* seek in next CacheLevel for elemaddress */
        SAC_CS_level++;
        SAC_CS_write_access_table[SAC_CS_level](baseaddress, elemaddress);

        return; /* leave function here */
    }           /* if-else:hit or miss */
} /* SAC_CS_Access_AS4WV */

void SAC_CS_ACCESS_AS4WA (TYPE) (void *baseaddress, void *elemaddress)
{
    /* unsigned because of right-shift-operation '>>' */
    unsigned int cacheline;
    tCacheLevel *act_cl;
    int pos, was_invalid;
    ULINT aligned_addr, newvalue, *pElm0;

    INIT_BASIC_VALUES (IN:, SAC_CS_cachelevel, SAC_CS_level, elemaddress, OUT:, act_cl,
                       aligned_addr, cacheline);

    /* pElm0 is a pointer to the cachecontents of element 0 within the
     * set (or list) belonging to the affected cacheline
     */
    pElm0 = CACHE_PELM0 (act_cl, cacheline);
    pos = GetPosWithinSet (&was_invalid, act_cl, pElm0, aligned_addr);
    if ((pos >= 0) && !was_invalid) {
        /* we have a hit */
        SAC_CS_whit[SAC_CS_level]++;
        /* update LRU-order */
        newvalue = *(pElm0 + pos);
        UPDATE_LRU_QUEUE (pElm0, pos, newvalue);

        SAC_CS_level = 1; /* next access has to seek in cachelevel 1 again */
        return;           /* leave function here */
    } else {
        /* we have a miss */
        SAC_CS_wmiss[SAC_CS_level]++;
        SAC_CS_winvalid[SAC_CS_level] += was_invalid;

#if DETAILED
        /* run through all shadowarrays and their affected entries */
        SAC_CS_DetailedAnalysis_Write (act_cl, baseaddress, aligned_addr, cacheline);
#endif

        return; /* leave function here */
    }           /* if-else:hit or miss */
} /* SAC_CS_Access_AS4WA */
