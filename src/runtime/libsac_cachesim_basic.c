/*
 * $Log$
 * Revision 2.1  1999/02/23 12:43:36  sacbase
 * new release made
 *
 * Revision 1.1  1999/02/17 17:25:18  her
 * Initial revision
 *
 */

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include "sac_cachesim.h"

/* BEGIN: The folowing variables are declared as extern in the
 * ´libsac_cachesim_access.c´-file. Changes here have to be done there
 * too!!!  */
tCacheLevel *SAC_CS_cachelevel[MAX_CACHELEVEL + 1]; /* SAC_CS_cachelevel[0] is unused */

ULINT SAC_CS_hit[MAX_CACHELEVEL + 1], SAC_CS_invalid[MAX_CACHELEVEL + 1],
  SAC_CS_miss[MAX_CACHELEVEL + 1], SAC_CS_cold[MAX_CACHELEVEL + 1],
  SAC_CS_cross[MAX_CACHELEVEL + 1], SAC_CS_self[MAX_CACHELEVEL + 1];
/* SAC_CS_XXX[0] is unused */

int SAC_CS_level = 1;

tFunRWAccess SAC_CS_ReadAccess, SAC_CS_WriteAccess,
  SAC_CS_read_access_table[MAX_CACHELEVEL + 2],
  SAC_CS_write_access_table[MAX_CACHELEVEL + 2];
/* SAC_CS_xxx_access_table[0] is unused,
   SAC_CS_xxx_access_table[MAX_CACHELEVEL+1] for dummy/MainMem */
/* END: */

/* forward declarations */
void SAC_CS_AccessMM (void *baseaddress, void *elemaddress);
void SAC_CS_AccessDMRead (void *baseaddress, void *elemaddress);
void SAC_CS_AccessDMWFOW (void *baseaddress, void *elemaddress);
void SAC_CS_AccessDMWWV (void *baseaddress, void *elemaddress);
void SAC_CS_AccessDMWWA (void *baseaddress, void *elemaddress);
void SAC_CS_AccessAS4Read (void *baseaddress, void *elemaddress);
void SAC_CS_AccessAS4WFOW (void *baseaddress, void *elemaddress);
void SAC_CS_AccessAS4WWV (void *baseaddress, void *elemaddress);
void SAC_CS_AccessAS4WWA (void *baseaddress, void *elemaddress);

/*
 * int fastlog2(int value)
 *
 * if value < 1 or value > 67108864=65536K=64M
 * or there exists no n with power(2,n)==value
 * then fastlog2 returns -1. Otherwise fastlog2 returns n.
 */
int
fastlog2 (int value)
{
    switch (value) {
    case 1:
        return (0);
    case 2:
        return (1);
    case 4:
        return (2);
    case 8:
        return (3);
    case 16:
        return (4);
    case 32:
        return (5);
    case 64:
        return (6);
    case 128:
        return (7);
    case 256:
        return (8);
    case 512:
        return (9);
    case 1024:
        return (10);
    case 2048:
        return (11);
    case 4096:
        return (12);
    case 8192:
        return (13);
    case 16384:
        return (14);
    case 32768:
        return (15);
    case 65536:
        return (16);
    case 131072:
        return (17);
    case 262144:
        return (18);
    case 524288:
        return (19);
    case 1048576:
        return (20);
    case 2097152:
        return (21);
    case 4194304:
        return (22);
    case 8388608:
        return (23);
    case 16777216:
        return (24);
    case 33554432:
        return (25);
    case 67108864:
        return (26);

    default:
        return (-1);
    }
} /* fastlog2 */

void
SAC_CS_Initialize (int nr_of_cpu, tProfilingLevel profilinglevel, ULINT cachesize1,
                   int cachelinesize1, int associativity1, tWritePolicy writepolicy1,
                   ULINT cachesize2, int cachelinesize2, int associativity2,
                   tWritePolicy writepolicy2, ULINT cachesize3, int cachelinesize3,
                   int associativity3, tWritePolicy writepolicy3)
{
    int integretyError, i;
    tCacheLevel *act_cl;

    /* BEGIN: cachelevel 1 */
    /* bind the right Access_XX functions to Read- WriteAccess userfunction
     * and read- write_access_table */
    if (associativity1 == 1) {
        SAC_CS_read_access_table[1] = &SAC_CS_AccessDMRead;
        switch (writepolicy1) {
        case Default:
            SAC_CS_write_access_table[1] = &SAC_CS_AccessDMWFOW;
            break;
        case fetch_on_write:
            SAC_CS_write_access_table[1] = &SAC_CS_AccessDMWFOW;
            break;
        case write_validate:
            SAC_CS_write_access_table[1] = &SAC_CS_AccessDMWWV;
            break;
        case write_around:
            SAC_CS_write_access_table[1] = &SAC_CS_AccessDMWWA;
            break;
        } /* switch */
    } else {
        SAC_CS_read_access_table[1] = &SAC_CS_AccessAS4Read;
        switch (writepolicy1) {
        case Default:
            SAC_CS_write_access_table[1] = &SAC_CS_AccessAS4WFOW;
            break;
        case fetch_on_write:
            SAC_CS_write_access_table[1] = &SAC_CS_AccessAS4WFOW;
            break;
        case write_validate:
            SAC_CS_write_access_table[1] = &SAC_CS_AccessAS4WWV;
            break;
        case write_around:
            SAC_CS_write_access_table[1] = &SAC_CS_AccessAS4WWA;
            break;
        } /* switch */
    }
    SAC_CS_cachelevel[1] = (tCacheLevel *)calloc (1, sizeof (tCacheLevel));
    act_cl = SAC_CS_cachelevel[1];
    if ((cachesize1 > 0) && (act_cl != NULL)) {
        /* init main-structure */
        act_cl->cachesize = cachesize1 * 1024; /* kbyte -> byte */
        act_cl->cachelinesize = cachelinesize1;
        act_cl->associativity = associativity1;
        act_cl->data = (ULINT *)malloc (act_cl->cachesize * sizeof (ULINT));
        /* integrety checks && evaluate some vars */
        integretyError = 0;
        integretyError = integretyError || (act_cl->cachesize % associativity1 != 0);
        act_cl->internsize = act_cl->cachesize / associativity1;
        integretyError = integretyError
                         || (fastlog2 (act_cl->internsize) <= fastlog2 (cachelinesize1))
                         || (fastlog2 (cachelinesize1) == -1);
        act_cl->cls_bits = fastlog2 (act_cl->cachelinesize);
        act_cl->cls_mask = ~(0ul) << act_cl->cls_bits;
        act_cl->is_bits = fastlog2 (act_cl->internsize);
        act_cl->is_mask = ~(0ul) >> ((sizeof (ULINT) * 8) - act_cl->is_bits);
        act_cl->nr_cachelines = act_cl->internsize / act_cl->cachelinesize;
        if (integretyError) {
            SAC_CS_read_access_table[1] = &SAC_CS_AccessMM;
            free (act_cl);
            act_cl = NULL;
            fprintf (stderr, "libsac_cachesim: "
                             "invalid cacheparameters for level 1\n");
        }
    } else {
        SAC_CS_read_access_table[1] = &SAC_CS_AccessMM;
        free (act_cl);
        act_cl = NULL;
    }

    /* init array of shadowarrays */
    for (i = 0; (i < MAX_SHADOWARRAYS) && (act_cl != NULL); i++) {
        act_cl->shadowarrays[i] = NULL;
    }
    /* END: cachelevel 1 */

    SAC_CS_read_access_table[2] = &SAC_CS_AccessMM;
    SAC_CS_write_access_table[2] = &SAC_CS_AccessMM;

    SAC_CS_ReadAccess = SAC_CS_read_access_table[1];
    SAC_CS_WriteAccess = SAC_CS_write_access_table[1];

} /* SAC_CS_Initialize */

void
SAC_CS_Finalize (void)
{
    unsigned i, j;

    for (i = 1; i <= MAX_CACHELEVEL; i++) {
        if (SAC_CS_cachelevel[i] != NULL) {
            for (j = 0; j < MAX_SHADOWARRAYS; j++) {
                if (SAC_CS_cachelevel[i]->shadowarrays[j] != NULL) {
                    free (SAC_CS_cachelevel[i]->shadowarrays[j]);
                    SAC_CS_cachelevel[i]->shadowarrays[j] = NULL;
                } /* if */
            }     /* for: j */
        }         /* if */
    }             /* for: i*/
} /* SAC_CS_Finalize */

void
SAC_CS_RegisterArray (void *baseaddress, int size /* in byte */)
{
    int i = 0, level, nr_blocks, error_msg_done = 0;
    tCacheLevel *cl;

    for (level = 1; level <= MAX_CACHELEVEL; level++) {
        cl = SAC_CS_cachelevel[level];
        if (cl != NULL) {
            /* find free slot in array of shadowarrays */
            while (i < MAX_SHADOWARRAYS && cl->shadowarrays[i] != NULL) {
                i++;
            }
            if (i < MAX_SHADOWARRAYS) {
                nr_blocks = (size + (cl->cachelinesize - 1)) / cl->cachelinesize;
                if (((ULINT)baseaddress % cl->cachelinesize)
                    > (cl->cachelinesize
                       - (((ULINT)baseaddress + size - 1) % cl->cachelinesize) - 1)) {
                    nr_blocks++;
                }
                cl->shadowbases[i] = (ULINT)baseaddress;
                cl->shadowalignedtop[i] = ((ULINT)baseaddress + size - 1) & cl->cls_mask;
                cl->shadowmaxindices[i] = ((nr_blocks + 1) / 2) - 1;
                cl->shadownrcols[i]
                  = (nr_blocks + (cl->nr_cachelines - 1)) / cl->nr_cachelines;
                cl->shadowarrays[i]
                  = (char *)calloc ((cl->nr_cachelines * cl->shadownrcols[i]) / 2,
                                    sizeof (char));
                /* already initiated by 0 */

                printf ("i: %d  nr_blocks: %d  shadownrcols[i]: %d\n", i, nr_blocks,
                        cl->shadownrcols[i]);
            } else {
                if (!error_msg_done) {
                    error_msg_done = 1;
                    fprintf (stderr,
                             "libsac_cachesim: "
                             "more than %d registered arrays (baseaddress: %lu)\n",
                             MAX_SHADOWARRAYS, (ULINT)baseaddress);
                } /* if: !error_msg_done */
            }     /* if-else: i<MAX_SHADOWARRAYS */
        }         /* if: cl != NULL */
    }             /* for: a */
} /* SAC_CS_RegisterArray */

void
SAC_CS_UnregisterArray (void *baseaddress)
{
    int i, j, lastused;
    tCacheLevel *cl;

    for (i = 1; i <= MAX_CACHELEVEL; i++) {
        cl = SAC_CS_cachelevel[i];
        if (cl != NULL) {
            /* find index (j) to remove */
            j = 0;
            while ((j < MAX_SHADOWARRAYS) && (cl->shadowbases[j] != (ULINT)baseaddress)) {
                j++;
            }
            if (j < MAX_SHADOWARRAYS) {
                /* find last used index */
                lastused = j;
                while ((lastused < MAX_SHADOWARRAYS)
                       && (cl->shadowarrays[lastused] != NULL)) {
                    lastused++;
                }           /* while: lastused */
                lastused--; /* correction in both cases */
                if (j != lastused) {
                    /* overwrite j with lastused and remove lastused then
                     * (to keep an contigous range of used indices) */
                    free (cl->shadowarrays[j]);
                    cl->shadowarrays[j] = cl->shadowarrays[lastused];
                    cl->shadowarrays[lastused] = NULL;
                    /* so the following free doesn´t kill the new j  */
                    cl->shadowbases[j] = cl->shadowbases[lastused];
                    cl->shadowalignedtop[j] = cl->shadowalignedtop[lastused];
                    cl->shadowmaxindices[j] = cl->shadowmaxindices[lastused];
                    cl->shadownrcols[j] = cl->shadownrcols[lastused];
                }
                /* remove lastused */
                free (cl->shadowarrays[lastused]);
                /* lastused is NULL if j was overwritten! */
                cl->shadowarrays[lastused] = NULL;
                cl->shadowbases[lastused] = 0;
                /* other members of this struct may stay as they are */
            } /* if */
        }     /* if */
    }         /*for: i*/
} /* SAC_CS_UnregisterArray */

void
SAC_CS_AccessMM (void *baseaddress, void *elemaddress)
{
    SAC_CS_level = 1;
} /* SAC_CS_AccessMM */

void
SAC_CS_ShowResults (void)
{
    int i;

    for (i = 1; i <= MAX_CACHELEVEL; i++) {
        if (SAC_CS_cachelevel[i] != NULL) {
            fprintf (stdout,
                     "Results for Level %d\n"
                     "Hits: %lu / %.1f%%   Total Misses: %lu / %.1f%%\n"
                     "Cold: %lu   Cross: %lu   Self: %lu   Invalid: %lu\n",
                     i, SAC_CS_hit[i],
                     (double)SAC_CS_hit[i] / (double)(SAC_CS_hit[i] + SAC_CS_miss[i])
                       * 100.0,
                     SAC_CS_miss[i],
                     (double)SAC_CS_miss[i] / (double)(SAC_CS_hit[i] + SAC_CS_miss[i])
                       * 100.0,
                     SAC_CS_cold[i], SAC_CS_cross[i], SAC_CS_self[i], SAC_CS_invalid[i]);
        } /* if */
    }     /* for: i */
} /* SAC_CS_ShowResults */
