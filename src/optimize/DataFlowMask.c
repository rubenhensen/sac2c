/*
 *
 * $Log$
 * Revision 1.12  1998/06/19 09:08:43  sbs
 * mask_base->num_ids = cnt instead of
 * mask_base->num_ids += cnt  helps housekeeping the memory 8-))
 *
 * Revision 1.11  1998/06/05 18:45:53  dkr
 * fixed a bug in DFMUpdateMaskBaseAfterCompiling
 *
 * Revision 1.10  1998/06/05 16:15:41  cg
 * This module is now able to deal with compiled variable declarations (ICMs)
 *
 * Revision 1.9  1998/06/03 14:35:05  cg
 * added function DFMUpdateMaskBaseAfterRenaming for special update
 * during precompiling .
 *
 * Revision 1.8  1998/05/19 08:53:26  cg
 * added strtok() like functions for retrieving variables from masks
 *
 * Revision 1.7  1998/05/14 15:49:11  cg
 * bug fixed in function PrintMask
 *
 * Revision 1.6  1998/05/11 17:33:09  dkr
 * DFMTest?Masks() returns now the bit-sum of the given mask.
 * (we want to know *how many* bits are set)
 *
 * Revision 1.5  1998/05/07 15:36:04  cg
 * mechanism added that allows general updates of data flow masks,
 * i.e. with newly introduced identifiers as well as old ones removed.
 *
 * Revision 1.4  1998/05/07 10:08:31  dkr
 * uses now StringCopy() in DFMGenMaskBase(), DFMExtendMaskBase() when
 *   extracting the vardec-, arg-names (no sharing!!).
 *
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
 *   In this module the names of local identifiers are shared with the
 *   actual syntax tree by purpose. The way this module is encapsulated
 *   justifies this decision.
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
#include "tree_compound.h"

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
    node **decls;
} mask_base_t;

typedef struct {
    int num_bitfields;
    unsigned int *bitfield;
    mask_base_t *mask_base;
} mask_t;

/*
 * internal macros
 */

#define CHECK_MASK(mask)                                                                 \
    if (mask->num_bitfields < mask->mask_base->num_bitfields)                            \
        ExtendMask (mask);

#define VARDEC_OR_ICM_NEXT(node)                                                         \
    ((NODE_TYPE (node) == N_vardec) ? VARDEC_NEXT (node) : ASSIGN_NEXT (node))

#define VARDEC_OR_ICM_NAME(node)                                                         \
    ((NODE_TYPE (node) == N_vardec) ? VARDEC_NAME (node)                                 \
                                    : ID_NAME (ICM_ARG2 (ASSIGN_INSTR (node))))

/*
 * The data flow masks are also used by compile.c.
 * Thus, they ought to deal with compiled variable declarations (ICMs).
 */

/*
 * internal functions
 */

static void
ExtendMask (mask_t *mask)
{
    int i;
    unsigned int *old;

    DBUG_ENTER ("ExtendMask");

    old = mask->bitfield;
    mask->bitfield
      = (unsigned int *)Malloc (mask->mask_base->num_bitfields * sizeof (unsigned int));
    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = old[i];
    }
    for (i = mask->num_bitfields; i < mask->mask_base->num_bitfields; i++) {
        mask->bitfield[i] = 0;
    }
    mask->num_bitfields = mask->mask_base->num_bitfields;
    FREE (old);

    DBUG_VOID_RETURN;
}

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
        tmp = VARDEC_OR_ICM_NEXT (tmp);
    }

    /*
     * Second, a new mask data base data structure of appropriate size is allocated.
     */

    base = (mask_base_t *)Malloc (sizeof (mask_base_t));

    base->ids = (char **)Malloc (cnt * sizeof (char *));

    base->decls = (node **)Malloc (cnt * sizeof (node *));

    base->num_ids = cnt;

    base->num_bitfields = (cnt / (sizeof (unsigned int) * 8)) + 1;

    /*
     * The local identifiers are stored in a table.
     */

    tmp = arguments;
    cnt = 0;

    while (tmp != NULL) {
        base->decls[cnt] = tmp;
        base->ids[cnt] = ARG_NAME (tmp);
        cnt += 1;
        tmp = ARG_NEXT (tmp);
    }

    tmp = vardecs;

    while (tmp != NULL) {
        base->decls[cnt] = tmp;
        base->ids[cnt] = VARDEC_OR_ICM_NAME (tmp);
        cnt += 1;
        tmp = VARDEC_NEXT (tmp);
    }

    DBUG_RETURN (base);
}

mask_base_t *
DFMUpdateMaskBase (mask_base_t *mask_base, node *arguments, node *vardecs)
{
    int cnt, old_num_ids, i;
    node *tmp;
    node **old_decls;

    DBUG_ENTER ("DFMUpdateMaskBase");

    /*
     * The number of new local identifiers is counted.
     * All those identifiers that already exist within the old identifier table
     * are copied to a new temporary table of identical size which is initialized with
     * NULL-pointers, i.e. all pointers in the new identifier table that are still
     * set to NULL afterwards belong to identifiers which have been removed from
     * the set of local identifiers. These remain subsequently set to NULL, i.e.
     * the respective bits in the data flow masks are not going to be reused.
     */

    old_decls = (node **)Malloc (mask_base->num_ids * sizeof (node *));

    for (i = 0; i < mask_base->num_ids; i++) {
        old_decls[i] = NULL;
    }

    tmp = arguments;
    cnt = mask_base->num_ids;

    while (tmp != NULL) {

        for (i = 0; i < mask_base->num_ids; i++) {
            if ((tmp == mask_base->decls[i])
                && (0 == strcmp (ARG_NAME (tmp), mask_base->ids[i]))) {
                old_decls[i] = mask_base->decls[i];
                goto old_arg_found;
            }
        }

        cnt += 1;

    old_arg_found:
        tmp = ARG_NEXT (tmp);
    }

    tmp = vardecs;

    while (tmp != NULL) {

        for (i = 0; i < mask_base->num_ids; i++) {
            if ((tmp == mask_base->decls[i])
                && (0 == strcmp (VARDEC_OR_ICM_NAME (tmp), mask_base->ids[i]))) {
                old_decls[i] = mask_base->decls[i];
                goto old_vardec_found;
            }
        }

        cnt += 1;

    old_vardec_found:
        tmp = VARDEC_OR_ICM_NEXT (tmp);
    }

    /*
     * The original identifier table may now be released.
     * The mask data base is updated with the new values and a new identifier
     * table is allocated that provides sufficient space for all new local
     * identifiers.
     */

    FREE (mask_base->ids);
    FREE (mask_base->decls);

    old_num_ids = mask_base->num_ids;

    mask_base->num_ids = cnt;

    mask_base->num_bitfields = (mask_base->num_ids / (sizeof (unsigned int) * 8)) + 1;

    mask_base->ids = (char **)Malloc ((mask_base->num_ids) * sizeof (char *));
    mask_base->decls = (node **)Malloc ((mask_base->num_ids) * sizeof (node *));

    /*
     * The temporary identifier table is copied to the newly allocated one and
     * its space is de-allocated afterwards.
     */

    for (i = 0; i < old_num_ids; i++) {
        mask_base->decls[i] = old_decls[i];
        mask_base->ids[i]
          = (old_decls[i] == NULL)
              ? NULL
              : (NODE_TYPE (old_decls[i]) == N_arg ? ARG_NAME (old_decls[i])
                                                   : VARDEC_OR_ICM_NAME (old_decls[i]));
    }

    FREE (old_decls);

    /*
     * New local identifiers are appended to the identifier table.
     */

    tmp = arguments;
    cnt = old_num_ids;

    while (tmp != NULL) {

        for (i = 0; i < old_num_ids; i++) {
            if (mask_base->decls[i] == tmp) {
                goto arg_found;
            }
        }

        mask_base->decls[cnt] = tmp;
        mask_base->ids[cnt] = ARG_NAME (tmp);
        cnt += 1;

    arg_found:
        tmp = ARG_NEXT (tmp);
    }

    tmp = vardecs;

    while (tmp != NULL) {

        for (i = 0; i < old_num_ids; i++) {
            if (mask_base->decls[i] == tmp) {
                goto vardec_found;
            }
        }

        mask_base->decls[cnt] = tmp;
        mask_base->ids[cnt] = VARDEC_OR_ICM_NAME (tmp);
        cnt += 1;

    vardec_found:
        tmp = VARDEC_OR_ICM_NEXT (tmp);
    }

    DBUG_RETURN (mask_base);
}

mask_base_t *
DFMRemoveMaskBase (mask_base_t *mask_base)
{
    DBUG_ENTER ("DFMRemoveMaskBase");

    FREE (mask_base->ids);
    FREE (mask_base->decls);
    FREE (mask_base);

    DBUG_RETURN ((mask_base_t *)NULL);
}

mask_base_t *
DFMUpdateMaskBaseAfterRenaming (mask_base_t *mask_base, node *arguments, node *vardecs)
{
    int i;
    node *tmp;

    DBUG_ENTER ("DFMUpdateMaskBaseAfterRenaming");

    for (i = 0; i < mask_base->num_ids; i++) {
        tmp = arguments;
        while ((tmp != NULL) && (tmp != mask_base->decls[i])) {
            tmp = ARG_NEXT (tmp);
        }

        if (tmp == NULL) {
            tmp = vardecs;
            while ((tmp != NULL) && (tmp != mask_base->decls[i])) {
                tmp = VARDEC_NEXT (tmp);
            }

            if (tmp == NULL) {
                /*
                 * Variable i has been removed from the local identifier set.
                 */
                mask_base->decls[i] = NULL;
                mask_base->ids[i] = NULL;
            } else {
                /*
                 * Variable i still exists in the local identifier set.
                 * So it has been renamed which requires to store a reference
                 * to its new name.
                 */
                mask_base->ids[i] = VARDEC_NAME (mask_base->decls[i]);
            }
        } else {
            /*
             * Variable i still exists in the local identifier set.
             * So it has been renamed which requires to store a reference
             * to its new name.
             */
            mask_base->ids[i] = ARG_NAME (mask_base->decls[i]);
        }
    }

    DBUG_RETURN (mask_base);
}

mask_base_t *
DFMUpdateMaskBaseAfterCompiling (mask_base_t *mask_base, node *arguments, node *vardecs)
{
    node *tmp;
    int i;

    DBUG_ENTER ("DFMUpdateMaskBaseAfterCompiling");

    /*
     * Because arguments are not compiled, we may start with the vardecs.
     */

    tmp = vardecs;

    while (tmp != NULL) {
        for (i = 0; i < mask_base->num_ids; i++) {
            if ((mask_base->ids[i] != NULL)
                && ((tmp == mask_base->decls[i])
                    || (0 == strcmp (VARDEC_OR_ICM_NAME (tmp), mask_base->ids[i])))) {
                mask_base->decls[i] = tmp;
                goto vardec_found;
            }
        }

        DBUG_ASSERT (0, "Variable declration removed during compilation");

    vardec_found:
        tmp = VARDEC_OR_ICM_NEXT (tmp);
    }

    DBUG_RETURN (mask_base);
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

    CHECK_MASK (mask);

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

    CHECK_MASK (mask);

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

    CHECK_MASK (mask1);
    CHECK_MASK (mask2);

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

    CHECK_MASK (mask1);
    CHECK_MASK (mask2);

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

    CHECK_MASK (mask1);
    CHECK_MASK (mask2);

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

    CHECK_MASK (mask);

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

    CHECK_MASK (mask);

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

    CHECK_MASK (mask);
    CHECK_MASK (mask2);

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

    CHECK_MASK (mask);

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

    CHECK_MASK (mask);
    CHECK_MASK (mask2);

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

    CHECK_MASK (mask);
    CHECK_MASK (mask2);

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

    CHECK_MASK (mask);
    CHECK_MASK (mask2);

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

    CHECK_MASK (mask);

    res = 0;

    for (i = 0; i < mask->num_bitfields; i++) {
        if (mask->bitfield[i] != 0) {
            res++;
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

    CHECK_MASK (mask1);
    CHECK_MASK (mask2);

    res = 0;

    for (i = 0; i < mask1->num_bitfields; i++) {
        if ((mask1->bitfield[i] & mask2->bitfield[i]) != 0) {
            res++;
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

    CHECK_MASK (mask1);
    CHECK_MASK (mask2);
    CHECK_MASK (mask3);

    res = 0;

    for (i = 0; i < mask1->num_bitfields; i++) {
        if ((mask1->bitfield[i] & mask2->bitfield[i] & mask3->bitfield[i]) != 0) {
            res++;
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

    CHECK_MASK (mask);

    i = 0;
    j = 0;

    for (cnt = 0; cnt < mask->mask_base->num_ids; cnt++) {
        if ((mask->bitfield[i] & access_mask_table[j])
            && (mask->mask_base->ids[cnt] != NULL)) {
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
DFMSetMaskEntryClear (mask_t *mask, char *id, node *decl)
{
    int i;

    DBUG_ENTER ("DFMSetMaskEntryClear");

    DBUG_ASSERT (((id != NULL) || (decl != NULL)),
                 "Neither name nor declaration provided to call to DFMSetMaskEntryClear");

    CHECK_MASK (mask);

    if (decl == NULL) {
        for (i = 0; i < mask->mask_base->num_ids; i++) {
            if ((mask->mask_base->ids[i] != NULL)
                && (0 == strcmp (mask->mask_base->ids[i], id))) {
                break;
            }
        }
    } else {
        for (i = 0; i < mask->mask_base->num_ids; i++) {
            if (mask->mask_base->decls[i] == decl) {
                break;
            }
        }
    }

    DBUG_ASSERT ((i < mask->mask_base->num_ids), "Identifier not present in mask");

    mask->bitfield[i / (8 * sizeof (unsigned int))]
      &= ~(access_mask_table[i % (8 * sizeof (unsigned int))]);

    DBUG_VOID_RETURN;
}

void
DFMSetMaskEntrySet (mask_t *mask, char *id, node *decl)
{
    int i;

    DBUG_ENTER ("DFMSetMaskEntrySet");

    DBUG_ASSERT (((id != NULL) || (decl != NULL)),
                 "Neither name nor declaration provided to call to DFMSetMaskEntrySet");

    CHECK_MASK (mask);

    if (decl == NULL) {
        for (i = 0; i < mask->mask_base->num_ids; i++) {
            if ((mask->mask_base->ids[i] != NULL)
                && (0 == strcmp (mask->mask_base->ids[i], id))) {
                break;
            }
        }
    } else {
        for (i = 0; i < mask->mask_base->num_ids; i++) {
            if (mask->mask_base->decls[i] == decl) {
                break;
            }
        }
    }

    DBUG_ASSERT ((i < mask->mask_base->num_ids), "Identifier not present in mask");

    mask->bitfield[i / (8 * sizeof (unsigned int))]
      |= access_mask_table[i % (8 * sizeof (unsigned int))];

    DBUG_VOID_RETURN;
}

int
DFMTestMaskEntry (mask_t *mask, char *id, node *decl)
{
    int i, res;

    DBUG_ENTER ("DFMTestMaskEntry");

    DBUG_ASSERT (((id != NULL) || (decl != NULL)),
                 "Neither name nor declaration provided to call to DFMTestMaskEntry");

    CHECK_MASK (mask);

    if (decl == NULL) {
        for (i = 0; i < mask->mask_base->num_ids; i++) {
            if ((mask->mask_base->ids[i] != NULL)
                && (0 == strcmp (mask->mask_base->ids[i], id))) {
                break;
            }
        }
    } else {
        for (i = 0; i < mask->mask_base->num_ids; i++) {
            if (mask->mask_base->decls[i] == decl) {
                break;
            }
        }
    }

    DBUG_ASSERT ((i < mask->mask_base->num_ids), "Identifier not present in mask");

    res = mask->bitfield[i / (8 * sizeof (unsigned int))]
          & access_mask_table[i % (8 * sizeof (unsigned int))];

    DBUG_RETURN (res);
}

char *
DFMGetMaskEntryNameClear (mask_t *mask)
{
    char *ret;

    static mask_t *store_mask;
    static int i;

    DBUG_ENTER ("DFMGetMaskEntryNameClear");

    if (mask != NULL) {
        CHECK_MASK (mask);
        store_mask = mask;
        i = 0;
    }

    while ((i < store_mask->mask_base->num_ids)
           && (store_mask->bitfield[i / (8 * sizeof (unsigned int))]
               & access_mask_table[i % (8 * sizeof (unsigned int))])) {
        i++;
    }

    ret = (i == store_mask->mask_base->num_ids) ? NULL : store_mask->mask_base->ids[i++];

    DBUG_RETURN (ret);
}

char *
DFMGetMaskEntryNameSet (mask_t *mask)
{
    char *ret;

    static mask_t *store_mask;
    static int i;

    DBUG_ENTER ("DFMGetMaskEntryNameClear");

    if (mask != NULL) {
        CHECK_MASK (mask);
        store_mask = mask;
        i = 0;
    }

    while ((i < store_mask->mask_base->num_ids)
           && (!(store_mask->bitfield[i / (8 * sizeof (unsigned int))]
                 & access_mask_table[i % (8 * sizeof (unsigned int))]))) {
        i++;
    }

    ret = (i == store_mask->mask_base->num_ids) ? NULL : store_mask->mask_base->ids[i++];

    DBUG_RETURN (ret);
}

node *
DFMGetMaskEntryDeclClear (mask_t *mask)
{
    node *ret;

    static mask_t *store_mask;
    static int i;

    DBUG_ENTER ("DFMGetMaskEntryDeclClear");

    if (mask != NULL) {
        CHECK_MASK (mask);
        store_mask = mask;
        i = 0;
    }

    while ((i < store_mask->mask_base->num_ids)
           && (store_mask->bitfield[i / (8 * sizeof (unsigned int))]
               & access_mask_table[i % (8 * sizeof (unsigned int))])) {
        i++;
    }

    ret
      = (i == store_mask->mask_base->num_ids) ? NULL : store_mask->mask_base->decls[i++];

    DBUG_RETURN (ret);
}

node *
DFMGetMaskEntryDeclSet (mask_t *mask)
{
    node *ret;

    static mask_t *store_mask;
    static int i;

    DBUG_ENTER ("DFMGetMaskEntryDeclClear");

    if (mask != NULL) {
        CHECK_MASK (mask);
        store_mask = mask;
        i = 0;
    }

    while ((i < store_mask->mask_base->num_ids)
           && (!(store_mask->bitfield[i / (8 * sizeof (unsigned int))]
                 & access_mask_table[i % (8 * sizeof (unsigned int))]))) {
        i++;
    }

    ret
      = (i == store_mask->mask_base->num_ids) ? NULL : store_mask->mask_base->decls[i++];

    DBUG_RETURN (ret);
}

node *
DFMVar2Decl (mask_t *mask, char *var)
{
    node *ret = NULL;
    int i;

    DBUG_ENTER ("DFMVar2Decl");

    for (i = 0; i < mask->mask_base->num_ids; i++) {
        if (0 == strcmp (mask->mask_base->ids[i], var)) {
            ret = mask->mask_base->decls[i];
            break;
        }
    }

    DBUG_RETURN (ret);
}
