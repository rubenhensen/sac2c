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

#include <stdio.h>

#include "DataFlowMask.h"

#define DBUG_PREFIX "DFM"
#include "debug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "check_mem.h"

/*
 * definition of static data
 */

static unsigned int *access_mask_table = NULL;

/*
 * definition of abstract data types
 */

typedef struct MASK_BASE_T {
    size_t num_ids;
    size_t num_bitfields;
    char **ids;
    node **decls;
} mask_base_t;

typedef struct MASK_T {
    size_t num_bitfields;
    unsigned int *bitfield;
    mask_base_t *mask_base;
} mask_t;

/*
 * internal macros
 */

#define CHECK_MASK(mask)                                                                 \
    if (mask->num_bitfields < mask->mask_base->num_bitfields)                            \
        ExtendMask (mask);

/*
 * internal functions
 */

static void
ExtendMask (mask_t *mask)
{
    size_t i;
    unsigned int *old;

    DBUG_ENTER ();

    DBUG_PRINT ("Extending mask from %zu to %zu entries...", mask->num_bitfields, mask->mask_base->num_bitfields);

    old = mask->bitfield;
    mask->bitfield = (unsigned int *)MEMmalloc (mask->mask_base->num_bitfields
                                                * sizeof (unsigned int));
    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = old[i];
    }
    for (i = mask->num_bitfields; i < mask->mask_base->num_bitfields; i++) {
        mask->bitfield[i] = 0;
    }
    mask->num_bitfields = mask->mask_base->num_bitfields;
    old = MEMfree (old);

    DBUG_RETURN ();
}

/*
 * functions for dealing with mask data bases
 */

/*
 * Generate a mask base. This is a table that contains the
 * DECL and AVIS_NAME/ARG_NAME for each function argument
 * and local variable in the fundef.
 *
 */

mask_base_t *
DFMgenMaskBase (node *arguments, node *vardecs)
{
    mask_base_t *base;
    size_t cnt;
    unsigned int access_mask;
    node *tmp;

    DBUG_ENTER ();

    DBUG_PRINT ("Creating Mask Base...");

    if (access_mask_table == NULL) {
        /*
         * The first time this function is called, a so-called access mask table is
         * created. For each bit in a bit mask, implemented as unsigned int, a
         * particular mask, of type unsigned int, is generated that allows
         * extraction of one bit from a data flow mask.
         *
         */

        access_mask_table
          = (unsigned int *)MEMmalloc (8 * sizeof (unsigned int) * sizeof (unsigned int));
        access_mask = 1;

        for (cnt = 0; (size_t)cnt < 8 * sizeof (unsigned int); cnt++) {
            DBUG_PRINT ("i %zu mask %u", cnt, access_mask);
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

    base = (mask_base_t *)MEMmalloc (sizeof (mask_base_t));

    base->ids = (char **)MEMmalloc (cnt * sizeof (char *));

    base->decls = (node **)MEMmalloc (cnt * sizeof (node *));

    base->num_ids = cnt;

    base->num_bitfields = (cnt / (sizeof (unsigned int) * 8)) + 1;

    DBUG_PRINT ("number of bitfields is %zu...", base->num_bitfields);

    /*
     * All local identifiers are stored in a table.
     */

    tmp = arguments;
    cnt = 0;

    while (tmp != NULL) {
        base->decls[cnt] = tmp;
        base->ids[cnt] = ARG_NAME (tmp);
        DBUG_PRINT ("Adding N_arg %s as element #%zu", AVIS_NAME (ARG_AVIS (tmp)), cnt);
        cnt += 1;
        tmp = ARG_NEXT (tmp);
    }

    tmp = vardecs;

    while (tmp != NULL) {
        base->decls[cnt] = tmp;
        base->ids[cnt] = VARDEC_NAME (tmp);
        DBUG_PRINT ("Adding N_vardec %s as element #%zu", VARDEC_NAME (tmp), cnt);
        cnt += 1;
        tmp = VARDEC_NEXT (tmp);
    }

    DBUG_RETURN (base);
}

mask_base_t *
DFMupdateMaskBase (mask_base_t *mask_base, node *arguments, node *vardecs)
{
    size_t cnt, old_num_ids, i;
    node *tmp;
    node **old_decls;

    DBUG_ENTER ();

    DBUG_PRINT ("Updating mask base...");

    DBUG_ASSERT (mask_base != NULL, "DFMupdateMaskBase() called with mask_base NULL");

    /*
     * The number of new local identifiers is counted.
     * All those identifiers that already exist within the old identifier table
     * are copied to a new temporary table of identical size which is initialized with
     * NULL-pointers, i.e. all pointers in the new identifier table that are still
     * set to NULL afterwards belong to identifiers which have been removed from
     * the set of local identifiers. These remain subsequently set to NULL, i.e.
     * the respective bits in the data flow masks are not going to be reused.
     */

    DBUG_PRINT ("Allocating tmp buffer for %zu decls", mask_base->num_ids);

    old_decls = (node **)MEMmalloc (mask_base->num_ids * sizeof (node *));

    for (i = 0; i < mask_base->num_ids; i++) {
        old_decls[i] = NULL;
    }

    tmp = arguments;
    cnt = mask_base->num_ids;

    while (tmp != NULL) {

        for (i = 0; i < mask_base->num_ids; i++) {
            if ((tmp == mask_base->decls[i])
                && (STReq (ARG_NAME (tmp), mask_base->ids[i]))) {
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
                && (STReq (VARDEC_NAME (tmp), mask_base->ids[i]))) {
                old_decls[i] = mask_base->decls[i];
                goto old_vardec_found;
            }
        }

        cnt += 1;

    old_vardec_found:
        tmp = VARDEC_NEXT (tmp);
    }

    /*
     * The original identifier table may now be released.
     * The mask data base is updated with the new values and a new identifier
     * table is allocated that provides sufficient space for all new local
     * identifiers.
     */

    MEMfree (mask_base->ids);
    MEMfree (mask_base->decls);

    old_num_ids = mask_base->num_ids;
    mask_base->num_ids = cnt;

    mask_base->num_bitfields = (mask_base->num_ids / (sizeof (unsigned int) * 8)) + 1;

    mask_base->ids = (char **)MEMmalloc ((mask_base->num_ids) * sizeof (char *));
    mask_base->decls = (node **)MEMmalloc ((mask_base->num_ids) * sizeof (node *));

    DBUG_PRINT ("number of bitfields is %zu...", mask_base->num_bitfields);

    /*
     * The temporary identifier table is copied to the newly allocated one and
     * its space is de-allocated afterwards.
     */

    for (i = 0; i < old_num_ids; i++) {
        mask_base->decls[i] = old_decls[i];
        mask_base->ids[i] = (old_decls[i] == NULL) ? NULL
                                                   : (NODE_TYPE (old_decls[i]) == N_arg
                                                        ? ARG_NAME (old_decls[i])
                                                        : VARDEC_NAME (old_decls[i]));
    }

    old_decls = MEMfree (old_decls);

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
        mask_base->ids[cnt] = VARDEC_NAME (tmp);
        cnt += 1;

    vardec_found:
        tmp = VARDEC_NEXT (tmp);
    }

    DBUG_RETURN (mask_base);
}

mask_base_t *
DFMremoveMaskBase (mask_base_t *mask_base)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Removing mask base...");

    DBUG_ASSERT (mask_base != NULL, "DFMremoveMaskBase() called with mask_base NULL");

    MEMfree (mask_base->ids);
    MEMfree (mask_base->decls);
    mask_base = MEMfree (mask_base);

    DBUG_RETURN (mask_base);
}

void
DFMtouchMaskBase (mask_base_t *mask_base, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Touching mask base...");

    DBUG_ASSERT (mask_base != NULL, "DFMtouchMaskBase() called with mask_base NULL");

    CHKMtouch (mask_base->ids, arg_info);
    CHKMtouch (mask_base->decls, arg_info);
    CHKMtouch (mask_base, arg_info);

    DBUG_RETURN ();
}

mask_base_t *
DFMgetMaskBase (mask_t *mask)
{
    DBUG_ENTER ();

    DBUG_RETURN (mask->mask_base);
}

mask_base_t *
DFMupdateMaskBaseAfterRenaming (mask_base_t *mask_base, node *arguments, node *vardecs)
{
    size_t i;
    node *tmp;

    DBUG_ENTER ();

    DBUG_ASSERT (mask_base != NULL,
                 "DFMupdateMaskBaseAfterRenaming() called with mask_base NULL");

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
DFMupdateMaskBaseAfterCompiling (mask_base_t *mask_base, node *arguments, node *vardecs)
{
    node *tmp;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (mask_base != NULL,
                 "DFMupdateMaskBaseAfterCompiling() called with mask_base NULL");

    /*
     * Because arguments are not compiled, we may start with the vardecs.
     */

    tmp = vardecs;

    while (tmp != NULL) {
        for (i = 0; i < mask_base->num_ids; i++) {
            if ((mask_base->ids[i] != NULL)
                && ((tmp == mask_base->decls[i])
                    || (STReq (VARDEC_NAME (tmp), mask_base->ids[i])))) {
                mask_base->decls[i] = tmp;
                goto vardec_found;
            }
        }

        DBUG_UNREACHABLE ("Variable declration removed during compilation");

    vardec_found:
        tmp = VARDEC_NEXT (tmp);
    }

    DBUG_RETURN (mask_base);
}

/*
 * functions for generating new masks
 */

mask_t *
DFMgenMaskClear (mask_base_t *mask_base)
{
    mask_t *new_mask;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (mask_base != NULL, "DFMgenMaskClear() called with mask_base NULL");

    DBUG_PRINT ("Generating a cleared mask...");

    new_mask = (mask_t *)MEMmalloc (sizeof (mask_t));

    new_mask->num_bitfields = mask_base->num_bitfields;

    new_mask->mask_base = mask_base;

    DBUG_PRINT ("...allocating %zu bitfields...", new_mask->num_bitfields);
    new_mask->bitfield
      = (unsigned int *)MEMmalloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = 0;
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMgenMaskSet (mask_base_t *mask_base)
{
    mask_t *new_mask;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (mask_base != NULL, "DFMgenMaskSet() called with mask_base NULL");

    DBUG_PRINT ("Generating a set mask...");

    new_mask = (mask_t *)MEMmalloc (sizeof (mask_t));

    new_mask->num_bitfields = mask_base->num_bitfields;

    new_mask->mask_base = mask_base;

    DBUG_PRINT ("...allocating %zu bitfields...", new_mask->num_bitfields);
    new_mask->bitfield
      = (unsigned int *)MEMmalloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = ~((unsigned int)0);
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMgenMaskCopy (mask_t *mask)
{
    mask_t *new_mask;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (mask != NULL, "DFMgenMaskCopy() called with mask NULL");

    DBUG_PRINT ("Generating a copy of mask...");

    CHECK_MASK (mask);

    new_mask = (mask_t *)MEMmalloc (sizeof (mask_t));

    new_mask->num_bitfields = mask->num_bitfields;

    new_mask->mask_base = mask->mask_base;

    DBUG_PRINT ("...allocating %zu bitfields...", new_mask->num_bitfields);
    new_mask->bitfield
      = (unsigned int *)MEMmalloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = mask->bitfield[i];
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMgenMaskInv (mask_t *mask)
{
    mask_t *new_mask;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (mask != NULL, "DFMgenMaskInv() called with mask NULL");

    DBUG_PRINT ("Generating an inverted copy of mask...");

    CHECK_MASK (mask);

    new_mask = (mask_t *)MEMmalloc (sizeof (mask_t));

    new_mask->num_bitfields = mask->num_bitfields;

    new_mask->mask_base = mask->mask_base;

    DBUG_PRINT ("...allocating %zu bitfields...", new_mask->num_bitfields);
    new_mask->bitfield
      = (unsigned int *)MEMmalloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = ~(mask->bitfield[i]);
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMgenMaskAnd (mask_t *mask1, mask_t *mask2)
{
    mask_t *new_mask;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (((mask1 != NULL) && (mask2 != NULL)),
                 "DFMgenMaskAnd() called with mask NULL");

    DBUG_ASSERT (mask1->mask_base == mask2->mask_base, "Combining incompatible masks");

    DBUG_PRINT ("Generating a bitwise-add copy of mask...");

    CHECK_MASK (mask1);
    CHECK_MASK (mask2);

    new_mask = (mask_t *)MEMmalloc (sizeof (mask_t));

    new_mask->num_bitfields = mask1->num_bitfields;

    new_mask->mask_base = mask1->mask_base;

    DBUG_PRINT ("...allocating %zu bitfields...", new_mask->num_bitfields);
    new_mask->bitfield
      = (unsigned int *)MEMmalloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = mask1->bitfield[i] & mask2->bitfield[i];
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMgenMaskOr (mask_t *mask1, mask_t *mask2)
{
    mask_t *new_mask;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (((mask1 != NULL) && (mask2 != NULL)),
                 "DFMgenMaskOr() called with mask NULL");

    DBUG_ASSERT (mask1->mask_base == mask2->mask_base, "Combining incompatible masks");

    DBUG_PRINT ("Generating a bitwise-or copy of mask...");

    CHECK_MASK (mask1);
    CHECK_MASK (mask2);

    new_mask = (mask_t *)MEMmalloc (sizeof (mask_t));

    new_mask->num_bitfields = mask1->num_bitfields;

    new_mask->mask_base = mask1->mask_base;

    DBUG_PRINT ("...allocating %zu bitfields...", new_mask->num_bitfields);
    new_mask->bitfield
      = (unsigned int *)MEMmalloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = mask1->bitfield[i] | mask2->bitfield[i];
    }

    DBUG_RETURN (new_mask);
}

mask_t *
DFMgenMaskMinus (mask_t *mask1, mask_t *mask2)
{
    mask_t *new_mask;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (((mask1 != NULL) && (mask2 != NULL)),
                 "DFMgenMaskMinus() called with mask NULL");

    DBUG_ASSERT (mask1->mask_base == mask2->mask_base, "Combining incompatible masks");

    DBUG_PRINT ("Generating a inverted bitwise-and copy of mask...");

    CHECK_MASK (mask1);
    CHECK_MASK (mask2);

    new_mask = (mask_t *)MEMmalloc (sizeof (mask_t));

    new_mask->num_bitfields = mask1->num_bitfields;

    new_mask->mask_base = mask1->mask_base;

    DBUG_PRINT ("...allocating %zu bitfields...", new_mask->num_bitfields);
    new_mask->bitfield
      = (unsigned int *)MEMmalloc (new_mask->num_bitfields * sizeof (unsigned int));

    for (i = 0; i < new_mask->num_bitfields; i++) {
        new_mask->bitfield[i] = mask1->bitfield[i] & ~(mask2->bitfield[i]);
    }

    DBUG_RETURN (new_mask);
}

/*
 * functions for updating an existing data flow mask
 */

void
DFMsetMaskClear (mask_t *mask)
{
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (mask != NULL, "DFMsetMaskClear() called with mask NULL");

    DBUG_PRINT ("Clear set mask...");

    CHECK_MASK (mask);

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = 0;
    }

    DBUG_RETURN ();
}

void
DFMsetMaskSet (mask_t *mask)
{
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (mask != NULL, "DFMsetMaskSet() called with mask NULL");

    DBUG_PRINT ("Set set mask...");

    CHECK_MASK (mask);

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = ~((unsigned int)0);
    }

    DBUG_RETURN ();
}

void
DFMsetMaskCopy (mask_t *mask, mask_t *mask2)
{
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (((mask != NULL) && (mask2 != NULL)),
                 "DFMgenMaskCopy() called with mask NULL");

    DBUG_ASSERT (mask->mask_base == mask2->mask_base, "Combining incompatible masks");

    DBUG_PRINT ("Copy set mask...");

    CHECK_MASK (mask);
    CHECK_MASK (mask2);

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = mask2->bitfield[i];
    }

    DBUG_RETURN ();
}

void
DFMsetMaskInv (mask_t *mask)
{
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (mask != NULL, "DFMsetMaskInv() called with mask NULL");

    DBUG_PRINT ("Invert set mask...");

    CHECK_MASK (mask);

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = ~mask->bitfield[i];
    }

    DBUG_RETURN ();
}

void
DFMsetMaskAnd (mask_t *mask, mask_t *mask2)
{
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (((mask != NULL) && (mask2 != NULL)),
                 "DFMsetMaskAnd() called with mask NULL");

    DBUG_ASSERT (mask->mask_base == mask2->mask_base, "Combining incompatible masks");

    DBUG_PRINT ("AND set masks...");

    CHECK_MASK (mask);
    CHECK_MASK (mask2);

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = mask->bitfield[i] & mask2->bitfield[i];
    }

    DBUG_RETURN ();
}

void
DFMsetMaskOr (mask_t *mask, mask_t *mask2)
{
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (((mask != NULL) && (mask2 != NULL)),
                 "DFMsetMaskOr() called with mask NULL");

    DBUG_ASSERT (mask->mask_base == mask2->mask_base, "Combining incompatible masks");

    DBUG_PRINT ("OR set masks...");

    CHECK_MASK (mask);
    CHECK_MASK (mask2);

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = mask->bitfield[i] | mask2->bitfield[i];
    }

    DBUG_RETURN ();
}

void
DFMsetMaskMinus (mask_t *mask, mask_t *mask2)
{
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (((mask != NULL) && (mask2 != NULL)),
                 "DFMsetMaskMinus() called with mask NULL");

    DBUG_ASSERT (mask->mask_base == mask2->mask_base, "Combining incompatible masks");

    DBUG_PRINT ("Invert AND set masks...");

    CHECK_MASK (mask);
    CHECK_MASK (mask2);

    for (i = 0; i < mask->num_bitfields; i++) {
        mask->bitfield[i] = mask->bitfield[i] & ~(mask2->bitfield[i]);
    }

    DBUG_RETURN ();
}

/*
 * functions for analyzing data flow masks
 */

size_t
DFMnumIds (mask_base_t *mask)
{
    DBUG_ENTER ();

    DBUG_ASSERT (mask != NULL, "DFMnumIds() called with mask NULL");

    DBUG_RETURN (mask->num_ids);
}

int
DFMtestMask (mask_t *mask)
{
    size_t i, j;
    int res;

    DBUG_ENTER ();

    DBUG_PRINT ("Testing mask...");

    DBUG_ASSERT (mask != NULL, "DFMtestMask() called with mask NULL");

    CHECK_MASK (mask);

    res = 0;

    DBUG_PRINT ("num_bitfields = %zu", mask->num_bitfields);

#if 0
  this old version only tests whether any bit is set,
  the new version really counts

  for (i=0; i<mask->num_bitfields; i++) {
    if (mask->bitfield[i] != 0) {
      res++;
      break;
    }
  }
#endif

    for (i = 0; i < mask->num_bitfields; i++) {
        for (j = 0; j < (8 * sizeof (unsigned int)); j++) {
            if ((mask->bitfield[i] & access_mask_table[j]) > 0) {
                res++;
            }
        }
    }

    DBUG_RETURN (res);
}

int
DFMtest2Masks (mask_t *mask1, mask_t *mask2)
{
    size_t i, j;
    int res;

    DBUG_ENTER ();

    DBUG_PRINT ("Testing two masks...");

    DBUG_ASSERT (((mask1 != NULL) && (mask2 != NULL)),
                 "DFMtest2Masks() called with mask NULL");

    DBUG_ASSERT (mask1->mask_base == mask2->mask_base, "Combining incompatible masks");

    CHECK_MASK (mask1);
    CHECK_MASK (mask2);

    res = 0;

#if 0
  this old version only tests whether any bit is set,
  the new version really counts

  for (i=0; i<mask1->num_bitfields; i++) {
    if ((mask1->bitfield[i] & mask2->bitfield[i]) != 0) {
      res++;
      break;
    }
  }
#endif

    for (i = 0; i < mask1->num_bitfields; i++) {
        for (j = 0; j < (8 * sizeof (unsigned int)); j++) {
            if (((mask1->bitfield[i] & access_mask_table[j]) > 0)
                && ((mask2->bitfield[i] & access_mask_table[j]) > 0)) {
                res++;
            }
        }
    }

    DBUG_RETURN (res);
}

int
DFMtest3Masks (mask_t *mask1, mask_t *mask2, mask_t *mask3)
{
    size_t i, j;
    int res;

    DBUG_ENTER ();

    DBUG_PRINT ("Testing three masks...");

    DBUG_ASSERT (((mask1 != NULL) && (mask2 != NULL) && (mask3 != NULL)),
                 "DFMtest3Masks() called with mask NULL");

    DBUG_ASSERT (((mask1->mask_base == mask2->mask_base)
                  && (mask1->mask_base == mask3->mask_base)),
                 "Combining incompatible masks");

    CHECK_MASK (mask1);
    CHECK_MASK (mask2);
    CHECK_MASK (mask3);

    res = 0;

#if 0
  this old version only tests whether any bit is set,
  the new version really counts

  for (i=0; i<mask1->num_bitfields; i++) {
    if ((mask1->bitfield[i] & mask2->bitfield[i] & mask3->bitfield[i]) != 0) {
      res++;
      break;
    }
  }
#endif

    for (i = 0; i < mask1->num_bitfields; i++) {
        for (j = 0; j < (8 * sizeof (unsigned int)); j++) {
            if (((mask1->bitfield[i] & access_mask_table[j]) > 0)
                && ((mask2->bitfield[i] & access_mask_table[j]) > 0)
                && ((mask3->bitfield[i] & access_mask_table[j]) > 0)) {
                res++;
            }
        }
    }

    DBUG_RETURN (res);
}

/*
 * functions for removing data flow masks
 */

mask_t *
DFMremoveMask (mask_t *mask)
{
    DBUG_ENTER ();

    DBUG_ASSERT (mask != NULL, "DFMremoveMask() called with mask NULL");

    MEMfree (mask->bitfield);
    mask = MEMfree (mask);

    DBUG_RETURN (mask);
}

/*
 * functions for touching data flow masks
 */

void
DFMtouchMask (mask_t *mask, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (mask != NULL, "DFMtouchMask() called with mask NULL");

    CHKMtouch (mask->bitfield, arg_info);
    CHKMtouch (mask, arg_info);

    DBUG_RETURN ();
}

/*
 * functions for visualization of data flow masks
 */

void
DFMprintMask (FILE *handle, const char *format, mask_t *mask)
{
    size_t i, j, cnt;

    DBUG_ENTER ();

    DBUG_PRINT ("Print mask...");

    DBUG_ASSERT (mask != NULL, "DFMprintMask() called with mask NULL");

    CHECK_MASK (mask);

    if (handle == NULL) {
        /*
         * NULL -> stderr
         * This is done for use in a debugging session.
         */
        handle = stderr;
    }

    i = 0;
    j = 0;

    for (cnt = 0; cnt < mask->mask_base->num_ids; cnt++) {
        if ((mask->bitfield[i] & access_mask_table[j])
            && (mask->mask_base->ids[cnt] != NULL)) {
            fprintf (handle, format, mask->mask_base->ids[cnt]);
        }

        if (j == 8 * sizeof (unsigned int) - 1) {
            i++;
            j = 0;
        } else {
            j++;
        }
    }

    fprintf (handle, "\n");

    DBUG_RETURN ();
}

void
DFMprintMaskDetailed (FILE *handle, mask_t *mask)
{
    size_t i, j, cnt;

    DBUG_ENTER ();

    DBUG_PRINT ("Print mask (detailed)...");

    DBUG_ASSERT (mask != NULL, "DFMprintMaskDetailed() called with mask NULL");

    CHECK_MASK (mask);

    if (handle == NULL) {
        /*
         * NULL -> stderr
         * This is done for use in a debugging session.
         */
        handle = stderr;
    }

    i = 0;
    j = 0;

    for (cnt = 0; cnt < mask->mask_base->num_ids; cnt++) {
        if (mask->mask_base->ids[cnt] != NULL) {
            if (mask->bitfield[i] & access_mask_table[j]) {
                fprintf (handle, "%s " F_PTR "\n", mask->mask_base->ids[cnt],
                         (void *)mask->mask_base->decls[cnt]);
            } else {
                fprintf (handle, "[%s] " F_PTR "\n", mask->mask_base->ids[cnt],
                         (void *)mask->mask_base->decls[cnt]);
            }
        }

        if (j == 8 * sizeof (unsigned int) - 1) {
            i++;
            j = 0;
        } else {
            j++;
        }
    }

    fprintf (handle, "\n");

    DBUG_RETURN ();
}

/*
 * functions for the manipulation of single data flow mask entries
 */

void
DFMsetMaskEntryClear (mask_t *mask, const char *id, node *avis)
{
    size_t i;
    node *decl = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("Clear mask entry for N_avis %s...", AVIS_NAME (avis));

    DBUG_ASSERT (mask != NULL, "DFMsetMaskEntryClear() called with mask NULL");

    if (avis != NULL) {
        DBUG_ASSERT (N_avis == NODE_TYPE (avis), "avis expected!");
        decl = AVIS_DECL (avis);
    }

    DBUG_ASSERT (((id != NULL) || (decl != NULL)),
                 "Neither name nor declaration provided to call to DFMsetMaskEntryClear");

    DBUG_PRINT ("mask has %zu bitfields, mask base has %zu bitfields", mask->num_bitfields, mask->mask_base->num_bitfields);
    CHECK_MASK (mask);

    if (decl == NULL) {
        for (i = 0; i < mask->mask_base->num_ids; i++) {
            if ((mask->mask_base->ids[i] != NULL)
                && (STReq (mask->mask_base->ids[i], id))) {
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

    DBUG_ASSERT (i < mask->mask_base->num_ids, "Identifier not present in mask: %s",
                 id != NULL ? id : AVIS_NAME (avis));

    mask->bitfield[i / (8 * sizeof (unsigned int))]
      &= ~(access_mask_table[i % (8 * sizeof (unsigned int))]);

    DBUG_RETURN ();
}

void
DFMsetMaskEntrySet (mask_t *mask, const char *id, node *avis)
{
    size_t i;
    node *decl = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("Set mask entry...");

    DBUG_ASSERT (mask != NULL, "called with mask NULL");

    if (avis != NULL) {
        DBUG_ASSERT (N_avis == NODE_TYPE (avis), "avis expected!");
        decl = AVIS_DECL (avis);
    }

    DBUG_ASSERT (((id != NULL) || (decl != NULL)),
                 "Neither name nor declaration provided to call");
    DBUG_EXECUTE (if (id != NULL) {
        fprintf (stderr, "DFMsetMastEntrySet() called for identifier %s\n", id);
    } else {
        fprintf (stderr, "DFMsetMastEntrySet() called for declaration of %s\n",
                 DECL_NAME (decl));
    });

    DBUG_PRINT ("mask has %zu bitfields, mask base has %zu bitfields", mask->num_bitfields, mask->mask_base->num_bitfields);
    CHECK_MASK (mask);

    if (decl == NULL) {
        for (i = 0; i < mask->mask_base->num_ids; i++) {
            if ((mask->mask_base->ids[i] != NULL)
                && (STReq (mask->mask_base->ids[i], id))) {
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

    DBUG_ASSERT (i < mask->mask_base->num_ids, "Identifier not present in mask: %s",
                 id != NULL ? id : AVIS_NAME (avis));

    mask->bitfield[i / (8 * sizeof (unsigned int))]
      |= access_mask_table[i % (8 * sizeof (unsigned int))];

    DBUG_RETURN ();
}

bool
DFMtestMaskEntry (mask_t *mask, const char *id, node *avis)
{
    size_t i, res;
    bool bres;
    node *decl = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("Test mask entry...");

    DBUG_ASSERT (mask != NULL, "DFMtestMaskEntry() called with mask NULL");

    if (avis != NULL) {

        DBUG_ASSERT (N_avis == NODE_TYPE (avis), "avis expected!");
        decl = AVIS_DECL (avis);
    }

    DBUG_ASSERT (((id != NULL) || (decl != NULL)),
                 "Neither name nor declaration provided to call to DFMtestMaskEntry");

    CHECK_MASK (mask);

    if (decl == NULL) {
        for (i = 0; i < mask->mask_base->num_ids; i++) {
            if ((mask->mask_base->ids[i] != NULL)
                && (STReq (mask->mask_base->ids[i], id))) {
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

    DBUG_ASSERT (i < mask->mask_base->num_ids, "Identifier not present in mask: %s",
                 id != NULL ? id : AVIS_NAME (avis));

    res = mask->bitfield[i / (8 * sizeof (unsigned int))]
          & access_mask_table[i % (8 * sizeof (unsigned int))];

    DBUG_PRINT ("i %zu mask %u acces %u res %zu", i,
                mask->bitfield[i / (8 * sizeof (unsigned int))],
                access_mask_table[i % (8 * sizeof (unsigned int))], res);

    bres = (res != 0);
    DBUG_RETURN (bres);
}

char *
DFMgetMaskEntryNameClear (mask_t *mask)
{
    char *ret;

    static mask_t *store_mask;
    static size_t i;

    DBUG_ENTER ();

    DBUG_PRINT ("Get mask entry name...");

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
DFMgetMaskEntryNameSet (mask_t *mask)
{
    char *ret;

    static mask_t *store_mask;
    static size_t i;

    DBUG_ENTER ();

    DBUG_PRINT ("Get mask entry name set...");

    if (mask != NULL) {
        CHECK_MASK (mask);
        store_mask = mask;
        i = 0;
    }

    while ((i < store_mask->mask_base->num_ids)
           && (!(store_mask->bitfield[i / (8 * sizeof (size_t))]
                 & access_mask_table[i % (8 * sizeof (unsigned int))]))) {
        i++;
    }

    ret = (i == store_mask->mask_base->num_ids) ? NULL : store_mask->mask_base->ids[i++];

    DBUG_RETURN (ret);
}

node *
DFMgetMaskEntryDeclClear (mask_t *mask)
{
    node *ret;

    static mask_t *store_mask;
    static size_t i;

    DBUG_ENTER ();
    
    DBUG_PRINT ("Get mask entry decl...");

    if (mask != NULL) {
        CHECK_MASK (mask);
        store_mask = mask;
        i = 0;
    }

    while ((i < store_mask->mask_base->num_ids)
           && (store_mask->bitfield[i / (8 * sizeof (size_t))]
               & access_mask_table[i % (8 * sizeof (unsigned int))])) {
        i++;
    }

    ret
      = (i == store_mask->mask_base->num_ids) ? NULL : store_mask->mask_base->decls[i++];

    DBUG_RETURN (ret);
}

node *
DFMgetMaskEntryDeclSet (mask_t *mask)
{
    node *ret;

    static mask_t *store_mask;
    static size_t i;

    DBUG_ENTER ();

    DBUG_PRINT ("Get mask entry decl set...");

    if (mask != NULL) {
        DBUG_PRINT ("Storing mask for further calls...");
        CHECK_MASK (mask);
        store_mask = mask;
        i = 0;
    }

    DBUG_ASSERT (store_mask != NULL, "getMaskEntryDeclSet() called with mask NULL");

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
DFMvar2Decl (mask_t *mask, char *var)
{
    node *ret = NULL;
    size_t i;

    DBUG_ENTER ();

    DBUG_ASSERT (mask != NULL, "DFMvar2Decl() called with mask NULL");

    for (i = 0; i < mask->mask_base->num_ids; i++) {
        if (STReq (mask->mask_base->ids[i], var)) {
            ret = mask->mask_base->decls[i];
            break;
        }
    }

    DBUG_RETURN (ret);
}

node *
DFMgetMaskEntryAvisSet (mask_t *mask)
{
    node *res = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("Get mask entry avis set...");

    res = DFMgetMaskEntryDeclSet (mask);
    if (res != NULL) {
        res = DECL_AVIS (res);
    }

    DBUG_RETURN (res);
}

node *
DFMgetMaskEntryAvisClear (mask_t *mask)
{
    node *res;

    DBUG_ENTER ();

    DBUG_PRINT ("Get mask entry avis clear...");

    res = DFMgetMaskEntryDeclClear (mask);

    if (res != NULL) {
        res = DECL_AVIS (res);
    }

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
