#include <memory.h>
#include <stdio.h>
#include "sac_cachesim.h"

#define MAX_CACHELEVEL 3
#define SIMCACHE(act_cl, col, row) /*act_cl->data[col,row]*/ 0
#define SIMINVALID(act_cl, col, row) /*act_cl->data[col,row]*/ 0

tCacheLevel *cl1, *cl2, *cl3, *cl[MAX_CACHELEVEL + 1];

int
fastlog2 (int value)
/* if value < 1 or value > 67108864=65536K=64M
 * or there exists no n with power(2,n)==value
 * then fastlog2 returns -1. Otherwise fastlog2 returns n.
 */
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
} // fastlog2

void
SAC_CPF_Initialize (int nr_of_cpu, ULINT cachesize1, int cachelinesize1,
                    int associativity1, tWritePolicy writepolicy1, ULINT cachesize2,
                    int cachelinesize2, int associativity2, tWritePolicy writepolicy2,
                    ULINT cachesize3, int cachelinesize3, int associativity3,
                    tWritePolicy writepolicy3)
{
    int integretyError;

    if (cachesize1 > 0) {
        // init main-structure
        cl1->cachesize = cachesize1 * 1024;
        cl1->cachelinesize = cachelinesize1;
        cl1->associativity = associativity1;
        // integrety checks && evaluate some vars
        integretyError = 0;
        integretyError = integretyError || (cl1->cachesize % associativity1 != 0);
        cl1->setsize = cl1->cachesize / associativity1;
        integretyError = integretyError
                         || (fastlog2 (cl1->setsize) <= fastlog2 (cachelinesize1))
                         || (fastlog2 (cachelinesize1) == -1);
        cl1->cls_mask = ~(0ul) << fastlog2 (cl1->cachelinesize);
        cl1->ss_bits = fastlog2 (cl1->setsize);
        cl1->ss_mask = ~(0ul) << cl1->ss_bits;
        cl1->max_cachelineindex = cl1->setsize / cl1->cachelinesize;
        if (integretyError) {
            /* cl1=NULL; */
        } // if
    } else
        cl1 = NULL;
} // SAC_CPF_Initialize

void
SAC_CPF_ReadAccess (void *baseaddress, void *elemaddress)
{
    unsigned int cacheline; // unsigned because of right-shift-operation ´>>´
    tCacheLevel *act_cl;
    int level = 1, set;

    ULINT aligned_addr;
    while (level <= MAX_CACHELEVEL) {
        act_cl = cl[level];
        aligned_addr = ((ULINT)elemaddress) & act_cl->cls_mask;
        cacheline = (aligned_addr & act_cl->ss_mask) >> act_cl->ss_bits;
        set = 0;
        while ((set < act_cl->associativity)
               && (SIMCACHE (act_cl, cacheline, set) != aligned_addr)) {
            set++;
        }
        if (set < act_cl->associativity) {
            if (!SIMINVALID (act_cl, cacheline, set)) {
            }
        } else {
        }
        level++;
    }
} // SAC_CPF_ReadAccess
