/*
 *
 * $Log$
 * Revision 1.1  2000/01/03 17:33:17  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:
 *
 * prefix: SAC_HM
 *
 * description:
 *
 *
 *
 *
 *
 *
 *****************************************************************************/

#define SAC_HM_CACHELIST_SIZE

unsigned int
SAC_HM_CachelistCheck (unsigned int cacheline, unsigned int *cachelist,
                       unsigned int max_cacheline)
{
    unsigned int i;

    i = 1;
    while (cachelist[i] < cacheline)
        i++;

    if ((i & 1) == cachelist[0]) {
        return (0);
    }

    if (cachelist[i] != max_cacheline) {
        return (cachelist[i] - cacheline);
    }

    return ((max_cacheline - cacheline) + cachelist[1]);
}

void
SAC_HM_CachelistIntersect (unsigned int *cl_res, unsigned int *cl1,
                           unsigned int cl1_offset, unsigned int *cl2,
                           unsigned int cl2_offset, unsigned int max_cacheline)
{
    unsigned int stat1, stat2, stat_res;
    unsigned int i1, i2, i_res;

    i1 = 1;
    while (cl1[i] < max_cacheline - cl1_offset)
        i++;
