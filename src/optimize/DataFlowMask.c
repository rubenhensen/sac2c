/*
 *
 * $Log$
 * Revision 1.3  1998/05/06 17:19:11  dkr
 * added DFMGenMaskMinus(), DFMSetMaskMinus()
 *
 * Revision 1.2  1998/05/06 09:57:04  cg
 * added include of globals.h
 *
 * Revision 1.1  1998/05/05 15:53:54  cg
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   DataFlowMask.c
 *
 * prefix: DFM
 *
 * description:
 *
 *   This module implements support for binary data flow masks.
 *   See the header file DataFlowMask.h for a detailed description.
 *
 *****************************************************************************/

#ifdef DFMtest
#define Malloc malloc
#define FREE free
#include <malloc.h>
#endif /* DFMtest */

#include <stdio.h>
#include <string.h>

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"

#ifndef DFMtest
#include "internal_lib.h"
#include "free.h"
#include "globals.h"
#endif /* DFMtest */

/*
 * definition of static data
 */

static unsigned int *access_mask_table = NULL;

/*
 * definition of abstract data types
 */

typedef struct {
    int num_ids;
    int num_bitfields;
    char **ids;
} mask_base_t;

typedef struct {
    int num_bitfields;
    unsigned int *bitfield;
    mask_base_t *mask_base;
} mask_t;

/*
 * functions for dealing with mask data bases
 */

mask_base_t *
DFMGenMaskBase (node *arguments, node *vardecs)
{
    mask_base_t *base;
    int cnt;
    unsigned int access_mask;
    node *tmp;

    DBUG_ENTER ("DFMGenMaskBase");

    if (access_mask_table == NULL) {
        /*
         * The first time this function is called, the so-called access mask table is
         * initialized. For each bit in a bit mask implemented as unsigned int, a
         * particular mask of type unsigned int is generated that allows to extract
         * exactly one bit froma data flow mask.
         */

        access_mask_table
          = (unsigned int *)Malloc (8 * sizeof (unsigned int) * sizeof (unsigned int));
        access_mask = 1;

        for (cnt = 0; cnt < 8 * sizeof (unsigned int); cnt++) {
            access_mask_table[cnt] = access_mask;
            access_mask <<= 1;
        }
    }

    /*
     * First, the number of local identifiers is counted.
     */

    tmp = arguments;
    cnt = 0;

    while (tmp != NULL) {
        cnt += 1;
        tmp = ARG_NEXT (tmp);
    }

    tmp = vardecs;

    while (tmp != NULL) {
        cnt += 1;
        tmp = VARDEC_NEXT (tmp);
    }

    /*
     * Second, a new mask data base data structure of appropriate size is allocated.
     */

    base = (mask_base_t *)Malloc (sizeof (mask_base_t));

    base->ids = (char **)Malloc (cnt * sizeof (char *));

    base->num_ids = cnt;

    base->num_bitfields = (cnt / (sizeof (unsigned int) * 8)) + 1;

    /*
     * The local identifiers are stored in a table.
     */

    tmp = arguments;
    cnt = 0;

    while (tmp != NULL) {
        base->ids[cnt] = ARG_NAME (tmp);
        cnt += 1;
        tmp = ARG_NEXT (tmp);
    }

    tmp = vardecs;

    while (tmp != NULL) {
        base->ids[cnt] = VARDEC_NAME (tmp);
        cnt += 1;
        tmp = VARDEC_NEXT (tmp);
    }

    DBUG_RETURN (base);
}

mask_base_t *
DFMExtendMaskBase (mask_base_t *mask_base, node *arguments, node *vardecs)
{
    int cnt, old_num_ids, i;
    node *tmp;
    char **old_ids;

    DBUG_ENTER ("DFMExtendMaskBase");

    /*
     * The new number of local identifiers is counted.
     */

    tmp = arguments;
    cnt = 0;

    while (tmp != NULL) {
        cnt += 1;
        tmp = ARG_NEXT (tmp);
    }

    tmp = vardecs;

    while (tmp != NULL) {
        cnt += 1;
        tmp = VARDEC_NEXT (tmp);
    }

    DBUG_ASSERT ((cnt >= mask_base->num_ids), "Number of local identifiers decreased");

    /*
     * The mask data base is updated with the new values and a new identifier
     * table is allocated.
     */

    old_ids = mask_base->ids;

    mask_base->ids = (char **)Malloc (cnt * sizeof (char *));

    old_num_ids = mask_base->num_ids;

    mask_base->num_ids = cnt;

    mask_base->num_bitfields = (cnt / (sizeof (unsigned int) * 8)) + 1;

    /*
     * The old identifier table is copied to the newly allocated one and
     * its space is de-allocated afterwards.
     */

    for (i = 0; i < mask_base->num_ids; i++) {
        mask_base->ids[i] = old_ids[i];
    }

    FREE (old_ids);

    /*
     * New local identifiers are appended to the identifier table.
     */

    tmp = arguments;
    cnt = mask_base->num_ids;

    while (tmp != NULL) {

        for (i = 0; i < old_num_ids; i++) {
            if (0 == strcmp (mask_base->ids[i], ARG_NAME (tmp))) {
                goto arg_found;
            }
        }

        mask_base->ids[cnt] = ARG_NAME (tmp);
        cnt += 1;

    arg_found:
        tmp = ARG_NEXT (tmp);
    }

    tmp = vardecs;

    while (tmp != NULL) {

        for (i = 0; i < old_num_ids; i++) {
            if (0 == strcmp (mask_base->ids[i], VARDEC_NAME (tmp))) {
                goto vardec_found;
            }
        }

        mask_base->ids[cnt] = VARDEC_NAME (tmp);
        cnt += 1;

    vardec_found:
        tmp = ARG_NEXT (tmp);
    }

    DBUG_RETURN (mask_base);
}

mask_base_t *
DFMRemoveMaskBase (mask_base_t *mask_base)
{
    DBUG_ENTER ("DFMRemoveMaskBase");

    FREE (mask_base->ids);
    FREE (mask_base);

    DBUG_RETURN ((mask_base_t *)NULL);
}

/*
 * functions for generating new masks
 */

mask_t *
DFMGenMaskClear (mask_base_t *mask_base)
{
    mask_t *new_mask;
    int i;

    DBUG_ENTER ("DFMGenMaskClear");

    new_mask = Malloc (sizeof (mask_t));

    new_mask->num_bitfields = mask_base->num_bitfields;

    new_mask->mask_base = mask_base;

    new_mask->bitfield
      = (unsigned int *)Malloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = 0;
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMGenMaskSet (mask_base_t *mask_base)
{
    mask_t *new_mask;
    int i;

    DBUG_ENTER ("DFMGenMaskSet");

    new_mask = Malloc (sizeof (mask_t));

    new_mask->num_bitfields = mask_base->num_bitfields;

    new_mask->mask_base = mask_base;

    new_mask->bitfield
      = (unsigned int *)Malloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = ~((unsigned int)0);
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMGenMaskCopy (mask_t *mask)
{
    mask_t *new_mask;
    int i;

    DBUG_ENTER ("DFMGenMaskCopy");

    new_mask = Malloc (sizeof (mask_t));

    new_mask->num_bitfields = mask->num_bitfields;

    new_mask->mask_base = mask->mask_base;

    new_mask->bitfield
      = (unsigned int *)Malloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = mask->bitfield[i];
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMGenMaskInv (mask_t *mask)
{
    mask_t *new_mask;
    int i;

    DBUG_ENTER ("DFMGenMaskInv");

    new_mask = Malloc (sizeof (mask_t));

    new_mask->num_bitfields = mask->num_bitfields;

    new_mask->mask_base = mask->mask_base;

    new_mask->bitfield
      = (unsigned int *)Malloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = ~(mask->bitfield[i]);
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMGenMaskAnd (mask_t *mask1, mask_t *mask2)
{
    mask_t *new_mask;
    int i;

    DBUG_ENTER ("DFMGenMaskAnd");

    DBUG_ASSERT ((mask1->mask_base == mask2->mask_base), "Combining incompatible masks");

    new_mask = Malloc (sizeof (mask_t));

    new_mask->num_bitfields = mask1->num_bitfields;

    new_mask->mask_base = mask1->mask_base;

    new_mask->bitfield
      = (unsigned int *)Malloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = mask1->bitfield[i] & mask2->bitfield[i];
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMGenMaskOr (mask_t *mask1, mask_t *mask2)
{
    mask_t *new_mask;
    int i;

    DBUG_ENTER ("DFMGenMaskOr");

    DBUG_ASSERT ((mask1->mask_base == mask2->mask_base), "Combining incompatible masks");

    new_mask = Malloc (sizeof (mask_t));

    new_mask->num_bitfields = mask1->num_bitfields;

    new_mask->mask_base = mask1->mask_base;

    new_mask->bitfield
      = (unsigned int *)Malloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = mask1->bitfield[i] | mask2->bitfield[i];
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMGenMaskMinus (mask_t *mask1, mask_t *mask2)
{
    mask_t *new_mask;
    int i;

    DBUG_ENTER ("DFMGenMaskMinus");

    DBUG_ASSERT ((mask1->mask_base == mask2->mask_base), "Combining incompatible masks");

    new_mask = Malloc (sizeof (mask_t));

    new_mask->num_bitfields = mask1->num_bitfields;

    new_mask->mask_base = mask1->mask_base;

    new_mask->bitfield
      = (unsigned int *)Malloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = mask1->bitfield[i] & ~(mask2->bitfield[i]);
    }

    DBUG_RETURN (new_mask);
}

/*
 * functions for updating an existing data flow mask
 */

void
DFMSetMaskClear (mask_t *mask)
{
    int i;

    DBUG_ENTER ("DFMSetMaskClear");

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = 0;
    }

    DBUG_VOID_RETURN;
}

void
DFMSetMaskSet (mask_t *mask)
{
    int i;

    DBUG_ENTER ("DFMSetMaskSet");

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = ~((unsigned int)0);
    }

    DBUG_VOID_RETURN;
}

void
DFMSetMaskCopy (mask_t *mask, mask_t *mask2)
{
    int i;

    DBUG_ENTER ("DFMSetMaskCopy");

    DBUG_ASSERT ((mask->mask_base == mask2->mask_base), "Combining incompatible masks");

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = mask2->bitfield[i];
    }

    DBUG_VOID_RETURN;
}

void
DFMSetMaskInv (mask_t *mask)
{
    int i;

    DBUG_ENTER ("DFMSetMaskInv");

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = ~mask->bitfield[i];
    }

    DBUG_VOID_RETURN;
}

void
DFMSetMaskAnd (mask_t *mask, mask_t *mask2)
{
    int i;

    DBUG_ENTER ("DFMSetMaskAnd");

    DBUG_ASSERT ((mask->mask_base == mask2->mask_base), "Combining incompatible masks");

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = mask->bitfield[i] & mask2->bitfield[i];
    }

    DBUG_VOID_RETURN;
}

void
DFMSetMaskOr (mask_t *mask, mask_t *mask2)
{
    int i;

    DBUG_ENTER ("DFMSetMaskOr");

    DBUG_ASSERT ((mask->mask_base == mask2->mask_base), "Combining incompatible masks");

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = mask->bitfield[i] | mask2->bitfield[i];
    }

    DBUG_VOID_RETURN;
}

void
DFMSetMaskMinus (mask_t *mask, mask_t *mask2)
{
    int i;

    DBUG_ENTER ("DFMSetMaskMinus");

    DBUG_ASSERT ((mask->mask_base == mask2->mask_base), "Combining incompatible masks");

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = mask->bitfield[i] & ~(mask2->bitfield[i]);
    }

    DBUG_VOID_RETURN;
}

/*
 * functions for analyzing data flow masks
 */

int
DFMTestMask (mask_t *mask)
{
    int i, res;

    DBUG_ENTER ("DFMTestMask");

    res = 0;

    for (i = 0; i < mask->num_bitfields; i++) {
        if (mask->bitfield[i] != 0) {
            res = 1;
            break;
        }
    }

    DBUG_RETURN (res);
}

int
DFMTest2Masks (mask_t *mask1, mask_t *mask2)
{
    int i, res;

    DBUG_ENTER ("DFMTest2Masks");

    DBUG_ASSERT ((mask1->mask_base == mask2->mask_base), "Combining incompatible masks");

    res = 0;

    for (i = 0; i < mask1->num_bitfields; i++) {
        if ((mask1->bitfield[i] & mask2->bitfield[i]) != 0) {
            res = 1;
            break;
        }
    }

    DBUG_RETURN (res);
}

int
DFMTest3Masks (mask_t *mask1, mask_t *mask2, mask_t *mask3)
{
    int i, res;

    DBUG_ENTER ("DFMTest3Masks");

    DBUG_ASSERT (((mask1->mask_base == mask2->mask_base)
                  && (mask1->mask_base == mask3->mask_base)),
                 "Combining incompatible masks");

    res = 0;

    for (i = 0; i < mask1->num_bitfields; i++) {
        if ((mask1->bitfield[i] & mask2->bitfield[i] & mask3->bitfield[i]) != 0) {
            res = 1;
            break;
        }
    }

    DBUG_RETURN (res);
}

/*
 * functions for removing data flow masks
 */

mask_t *
DFMRemoveMask (mask_t *mask)
{
    DBUG_ENTER ("DFMRemoveMask");

    FREE (mask->bitfield);
    FREE (mask);

    DBUG_RETURN ((mask_t *)NULL);
}

/*
 * functions for visualization of data flow masks
 */

void
DFMPrintMask (FILE *handle, const char *format, mask_t *mask)
{
    int i, j, cnt;

    DBUG_ENTER ("DFMPrintMask");

    i = 0;
    j = 0;

    for (cnt = 0; cnt < mask->mask_base->num_ids; cnt++) {
        if (mask->bitfield[i] & access_mask_table[j]) {
            fprintf (handle, format, mask->mask_base->ids[cnt]);
        }

        if (j == 8 * sizeof (unsigned int) - 1) {
            i += 1;
            j = 0;
        } else {
            j += 1;
        }
    }

    DBUG_VOID_RETURN;
}

/*
 * functions for the manipulation of single data flow mask entries
 */

void
DFMSetMaskEntryClear (mask_t *mask, char *id)
{
    int i;

    DBUG_ENTER ("DFMSetMaskEntryClear");

    for (i = 0; i < mask->mask_base->num_ids; i++) {
        if (0 == strcmp (mask->mask_base->ids[i], id)) {
            break;
        }
    }

    DBUG_ASSERT ((i < mask->mask_base->num_ids), "Identifier not present in mask");

    mask->bitfield[i / (8 * sizeof (unsigned int))]
      &= ~(access_mask_table[i % (8 * sizeof (unsigned int))]);

    DBUG_VOID_RETURN;
}

void
DFMSetMaskEntrySet (mask_t *mask, char *id)
{
    int i;

    DBUG_ENTER ("DFMSetMaskEntrySet");

    for (i = 0; i < mask->mask_base->num_ids; i++) {
        if (0 == strcmp (mask->mask_base->ids[i], id)) {
            break;
        }
    }

    DBUG_ASSERT ((i < mask->mask_base->num_ids), "Identifier not present in mask");

    mask->bitfield[i / (8 * sizeof (unsigned int))]
      |= access_mask_table[i % (8 * sizeof (unsigned int))];

    DBUG_VOID_RETURN;
}

int
DFMTestMaskEntry (mask_t *mask, char *id)
{
    int i, res;

    DBUG_ENTER ("FMTestMaskEntry");

    for (i = 0; i < mask->mask_base->num_ids; i++) {
        if (0 == strcmp (mask->mask_base->ids[i], id)) {
            break;
        }
    }

    DBUG_ASSERT ((i < mask->mask_base->num_ids), "Identifier not present in mask");

    res = mask->bitfield[i / (8 * sizeof (unsigned int))]
          & access_mask_table[i % (8 * sizeof (unsigned int))];

    DBUG_RETURN (res);
}
