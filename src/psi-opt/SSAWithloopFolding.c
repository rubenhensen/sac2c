/*
 * $Log$
 * Revision 1.24  2005/01/26 10:24:38  mwe
 * AVIS_SSAASSIGN removed and replaced by usage of akv types
 *
 * Revision 1.23  2005/01/11 13:32:21  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.22  2004/12/08 18:02:10  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.21  2004/11/27 02:32:32  mwe
 * function renaming
 *
 * Revision 1.20  2004/11/26 15:51:50  jhb
 * WLFwithloopFoldingWLT changed WLFwithloopFoldingWlt
 *
 * Revision 1.19  2004/11/25 23:11:10  jhb
 * on the road again
 *
 * Revision 1.18  2004/11/16 16:35:08  mwe
 * code for type upgrade added
 * use ntype-structure instead of type-structure
 * new code deactivated by MWE_NTYPE_READY
 *
 * Revision 1.17  2004/10/07 12:12:45  sah
 * added NCODE_INC_USED macro
 *
 * Revision 1.16  2004/10/05 13:50:58  sah
 * lifted start of WLI/WLT traversal to the
 * defining source files to allow for local
 * info structures
 *
 * Revision 1.15  2004/07/14 14:17:36  sah
 * added SSADbugIndexInfo as a replacement for DebugIndexInfo
 * from old WithloopFolding, as that will be gone soon
 *
 * Revision 1.14  2004/02/25 15:53:06  cg
 * New functions RestoreSSAOneFunction and RestoreSSAOneFundef
 * now provide access to SSA transformations on a per function
 * basis.
 * Only functions from ssa.[ch] should be used to initiate the
 * transformation process in either direction!
 *
 * Revision 1.13  2003/06/11 21:52:05  ktr
 * Added support for multidimensional arrays.
 *
 * Revision 1.12  2003/04/10 11:56:03  dkr
 * SSAArrayST2ArrayInt modified: returns NULL if argument is a structural
 * constant only
 *
 * Revision 1.11  2002/10/09 02:11:39  dkr
 * constants modul used instead of ID/ARRAY_CONSTVEC
 *
 * Revision 1.10  2002/10/08 10:32:29  dkr
 * SSAArrayST2ArrayInt(): AVIS_SSACONST(ID_AVIS()) used instead of ID_CONSTVEC
 *
 * Revision 1.9  2002/09/11 23:17:23  dkr
 * prf_string replaced by mdb_prf
 *
 * Revision 1.8  2002/02/20 14:40:40  dkr
 * function DupTypes() renamed into DupAllTypes()
 *
 * Revision 1.7  2001/05/22 14:57:19  nmw
 *  comments corrected
 *
 * Revision 1.6  2001/05/17 14:09:32  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 1.5  2001/05/17 13:29:29  cg
 * De-allocation macros FREE_INTERN_GEN and FREE_INDEX_INFO
 * converted to functions.
 *
 * Revision 1.4  2001/05/16 19:52:47  nmw
 * reverted Free() to FREE() due to segfaults when used with linux :-(
 *
 * Revision 1.3  2001/05/16 13:43:08  nmw
 * unused old code removed, comments corrected
 * MALLOC/FREE changed to Malloc/Free
 *
 * Revision 1.2  2001/05/15 16:39:21  nmw
 * SSAWithloopFolding implemented (but not tested)
 *
 * Revision 1.1  2001/05/14 15:55:15  nmw
 * Initial revision
 *
 *
 * created from WithloopFolding.c, Revision 3.6  on 2001/05/14 by nmw
 */

/*
 * this code implementes the ssa aware version of the original withloop folding.
 * it reuses most code of the old implementation but the ssa form simplifies
 * many cases and allows to avoid any masks in the opt cycle.
 * most comments are unchanged from the original implementation because i (nmw)
 * do not know how everything works in this implementation...
 */

/*******************************************************************************
 This file organizes the withloop folding and makes some basic functions
 available.

 Withloop folding is done in 3 phases:
 1) SSAWLT transforms WLs to apply the following phases
 1) SSAWLI gathers information about the WLs
 2) SSAWLF finds and folds suitable WLs.


 Assumption: We assume that all generators of a WL have the same
 shape.  Furthermore we assume that, if an N_Ncode is referenced by
 more than one generator, all these generators' indexes (vector and
 scalar) have the same names. This 'same name assumption' can even be
 expanded to all generators a WL has. This is true as a consequence of
 the folding mechanism we apply (induction): At the beginning, all WL
 have only one generator and it is obviously true. When another WL's
 body (foreign body) is folded into a this WL's body, the body is
 copied inclusive the generator indices. The foreign indices of the
 other WL are transformed (based on the origial indicies) to temp
 variables, which are applied to the substituted foreign body.

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"
#include "ctinfo.h"
#include "dbug.h"
#include "traverse.h"
#include "optimize.h"
#include "constants.h"
#include "ssa.h"
#include "SSAWithloopFolding.h"
#include "SSAWLI.h"
#include "SSAWLT.h"
#include "new_types.h"

/******************************************************************************
 *
 * function:
 *   index_info *WLFcreateIndex(int vector)
 *
 * description:
 *   create an incarnation of INDEX_INFO.
 *
 * remark:
 *   the argument 'vector' behaves like the component 'vector' of INDEX_INFO.
 *
 * reference:
 *   FREE_INDEX
 *
 ******************************************************************************/
index_info *
WLFcreateIndex (int vector)
{
    index_info *pindex;
    DBUG_ENTER ("WLFcreateInfoInfo");

    pindex = ILIBmalloc (sizeof (index_info));
    pindex->vector = vector;

    if (!vector)
        vector = 1;
    pindex->permutation = ILIBmalloc (sizeof (int) * vector);
    pindex->last = ILIBmalloc (sizeof (index_info *) * vector);
    pindex->const_arg = ILIBmalloc (sizeof (int) * vector);

    pindex->arg_no = 0;

    DBUG_RETURN (pindex);
}

/******************************************************************************
 *
 * function:
 *   index_info *WLFduplicateIndexInfo(index_info *iinfo)
 *
 * description:
 *   duplicates struct
 *
 ******************************************************************************/
index_info *
WLFduplicateIndexInfo (index_info *iinfo)
{
    index_info *new;
    int i, to;
    DBUG_ENTER ("WLFduplicateIndexInfo");
    DBUG_ASSERT (iinfo, ("parameter NULL"));

    new = WLFcreateIndex (iinfo->vector);

    to = iinfo->vector ? iinfo->vector : 1;
    for (i = 0; i < to; i++) {
        new->permutation[i] = iinfo->permutation[i];
        new->last[i] = iinfo->last[i];
        new->const_arg[i] = iinfo->const_arg[i];
    }

    new->prf = iinfo->prf;
    new->arg_no = iinfo->arg_no;

    DBUG_RETURN (new);
}

/******************************************************************************
 *
 * function:
 *   node *WLFvalidLocalId(node *idn)
 *
 * description:
 *   returns pointer to index_info if Id (idn) is a valid variable within
 *   the WL body (index vars are excluded). Returns NULL otherwise.
 *
 ******************************************************************************/
index_info *
WLFvalidLocalId (node *idn)
{
    index_info *iinfo;

    DBUG_ENTER ("WLFvalidLocalId");
    DBUG_ASSERT (N_id == NODE_TYPE (idn), ("not an id node"));

    /* get defining assignment via avis_ssaassign link */
    if (AVIS_SSAASSIGN (ID_AVIS (idn)) != NULL) {
        iinfo = SSAINDEX (AVIS_SSAASSIGN (ID_AVIS (idn)));
    } else {
        iinfo = NULL;
    }

    DBUG_RETURN (iinfo);
}

/******************************************************************************
 *
 * function:
 *   void WLFdbugIndexInfo(index info *iinfo)
 *
 * description:
 *   prints history of iinfo.
 *
 *
 ******************************************************************************/

void
WLFdbugIndexInfo (index_info *iinfo)
{
    int i, sel;
    index_info *tmpii;

    DBUG_ENTER ("WLFdbugIndexInfo");

    printf (
      "\n|-------------------------INDEX-INFO----------------------------------------\n");
    if (!iinfo)
        printf ("|NULL\n");
    else if (iinfo->vector) {
        printf ("|VECTOR shape [%d]:\n", iinfo->vector);
        for (i = 0; i < iinfo->vector; i++) {
            printf ("|---%d---\n", i);

            if (!iinfo->permutation[i]) { /* constant */
                printf ("|  constant %d\n", iinfo->const_arg[i]);
                continue;
            }

            printf ("|  base %d\n", iinfo->permutation[i]);
            tmpii = iinfo;
            while (tmpii) {
                sel = tmpii->vector ? i : 0;
                if (tmpii->arg_no) {
                    if (1 == tmpii->arg_no)
                        printf ("|   %d%s. ", tmpii->const_arg[sel],
                                global.mdb_prf[tmpii->prf]);
                    else
                        printf ("|   .%s%d ", global.mdb_prf[tmpii->prf],
                                tmpii->const_arg[sel]);
                } else
                    printf ("|   no prf ");
                printf ("|(p:%d, v:%d)\n", tmpii->permutation[sel], tmpii->vector);
                tmpii = tmpii->last[sel];
            }
        }
    } else {
        printf ("|SCALAR:\n");
        printf ("|  base %d\n", iinfo->permutation[0]);
        tmpii = iinfo;
        sel = 0;
        if (tmpii->arg_no) {
            if (1 == tmpii->arg_no)
                printf ("|   %d%s. ", tmpii->const_arg[sel], global.mdb_prf[tmpii->prf]);
            else
                printf ("|   %s%d. ", global.mdb_prf[tmpii->prf], tmpii->const_arg[sel]);
            printf ("|(p:%d, v:%d)\n", tmpii->permutation[sel], tmpii->vector);
        }
    }
    printf (
      "|---------------------------------------------------------------------------\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   int WLFlocateIndexVar(node *idn, node* wln)
 *
 * description:
 *   Searches for the Id (idn) in the WL generator (index var).
 *   The N_With node has to be available to find the index vars.
 *
 * return:
 *   -1: Id is the index vector
 *    0: Id not found
 *    x with x gt 0: Id is the x'th scalar index variable.
 *
 * remark:
 *   we exploit here that the index variables of all N_Withid nodes in
 *   one WL have the same names.
 *
 ******************************************************************************/
int
WLFlocateIndexVar (node *idn, node *wln)
{
    node *_ids;
    int result = 0, i;

    DBUG_ENTER ("WLFlocateIndexVar");
    DBUG_ASSERT (N_with == NODE_TYPE (wln), ("wln is not N_with node"));

    wln = PART_WITHID (WITH_PART (wln));
    _ids = WITHID_VEC (wln);

    if (!strcmp (IDS_NAME (_ids), ID_NAME (idn)))
        result = -1;

    i = 1;
    _ids = WITHID_IDS (wln);
    while (_ids && !result) {
        if (!strcmp (IDS_NAME (_ids), ID_NAME (idn)))
            result = i;
        i++;
        _ids = IDS_NEXT (_ids);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *WLFcreateInternGen(int shape, int stepwidth)
 *
 * description:
 *   allocate memory for an intern_gen struct. The parameter shape is needed
 *   to allocate right sized mem for the bounds. If stepwidth is not 0,
 *   memory for step/width is allocated, too.
 *
 ******************************************************************************/
intern_gen *
WLFcreateInternGen (int shape, int stepwidth)
{
    intern_gen *ig;

    DBUG_ENTER ("WLFcreateInternGen");

    ig = ILIBmalloc (sizeof (intern_gen));
    ig->shape = shape;
    ig->code = NULL;
    ig->next = NULL;

    ig->l = ILIBmalloc (sizeof (int) * shape);
    ig->u = ILIBmalloc (sizeof (int) * shape);
    if (stepwidth) {
        ig->step = ILIBmalloc (sizeof (int) * shape);
        ig->width = ILIBmalloc (sizeof (int) * shape);
    } else {
        ig->step = NULL;
        ig->width = NULL;
    }

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *WLFappendInternGen(...)
 *
 * description:
 *   this function creates an intern_gen struct and inserts it in a
 *   intern_gen chain after append_to (if not NULL).
 *   The new intern_gen struct is returned.
 *
 * parameters:
 *   append_to: see description
 *   shape/stepwidth: see CreateInternGen().
 *   code: N_Ncode node this intern_gen struct points to.
 *
 ******************************************************************************/
intern_gen *
WLFappendInternGen (intern_gen *append_to, int shape, node *code, int stepwidth)
{
    intern_gen *ig;
    int i;

    DBUG_ENTER ("WLFappendInternGen");

    ig = WLFcreateInternGen (shape, stepwidth);

    if (stepwidth)
        for (i = 0; i < shape; i++)
            ig->step[i] = ig->width[i] = 1;

    if (append_to) {
        ig->next = append_to->next;
        append_to->next = ig;
    }

    ig->code = code;

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *WLFcopyInternGen(intern_gen *source)
 *
 * description:
 *   Copy the struct source and return it's pointer. Only the first struct
 *   is copied, not the structs which can be reached with ->next.
 *
 * attention:
 *   the 'next' component is not copied and instead set to NULL.
 *
 ******************************************************************************/
intern_gen *
WLFcopyInternGen (intern_gen *source)
{
    intern_gen *ig;
    int i;

    DBUG_ENTER ("WLFcopyInternGen");

    ig = WLFcreateInternGen (source->shape, NULL != source->step);
    ig->code = source->code;

    for (i = 0; i < ig->shape; i++) {
        ig->l[i] = source->l[i];
        ig->u[i] = source->u[i];
        if (source->step) {
            ig->step[i] = source->step[i];
            ig->width[i] = source->width[i];
        }
    }

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   int WLFnormalizeInternGen(intern_gen *ig)
 *
 * description:
 *   normalizes step and width. There are several forbidden and ambiguous
 *   combinations of bounds, step (s) and width (w).
 *
 *   allowed is the following (for each component):
 *   - w < s and s > 1 or
 *   - w == 1 and s == 1
 *
 *   transformations:
 *   - w == s: set w = s = 1 (componentwise)
 *   - vector s == 1 and vector w == 1: set both to NULL
 *   - vector s != NULL and vector w == NULL: create vector w = 1. This
 *                                            was done in AppendInternGen().
 *
 *   errors:
 *   - w > s        (error no 1)
 *   - 1 > w        (error no 2)
 *   - w without s  (error no 3)
 *
 * return:
 *   returns 0 if no error was detected, else error no (see above).
 *
 ******************************************************************************/
int
WLFnormalizeInternGen (intern_gen *ig)
{
    int error = 0, i = 0, is_1 = 1;

    DBUG_ENTER ("WLFnormalizeInternGen");

    if (ig->width && !ig->step)
        error = 3;
    else if (ig->step) {
        while (i < ig->shape && !error) {
            if (ig->width[i] > ig->step[i])
                error = 1;
            else if (1 > ig->width[i])
                error = 2;
            else if (ig->width[i] == ig->step[i] && ig->step[i] != 1)
                ig->step[i] = ig->width[i] = 1;

            is_1 = is_1 && 1 == ig->step[i];
            i++;
        }

        /* if both vectors are 1 this is equivalent to no grid. */
        if (!error && is_1) {
            ig->step = ILIBfree (ig->step);
            ig->width = ILIBfree (ig->width);
        }
    }

    DBUG_RETURN (error);
}

/******************************************************************************
 *
 * function:
 *   void WLFarrayST2ArrayInt(node *arrayn, int **iarray, int shape)
 *
 * description:
 *   copies 'shape' numbers of given constant array node (1 dimension) to
 *   *iarray.
 *   If *iarray = NULL, memory is allocated.
 *   If arrayn is NULL, iarray is filles with 0.
 *
 ******************************************************************************/
void
WLFarrayST2ArrayInt (node *arrayn, int **iarray, int shape)
{
    constant *tmp_co;
    int *tmp;
    int i;

    DBUG_ENTER ("WLFarrayST2ArrayInt");

    DBUG_ASSERT ((iarray != NULL), "no iarray found!");

    if (*iarray == NULL) {
        *iarray = ILIBmalloc (shape * sizeof (int));
    }

    if (arrayn == NULL) {
        for (i = 0; i < shape; i++) {
            (*iarray)[i] = 0;
        }
    } else if (NODE_TYPE (arrayn) == N_array) {
        tmp_co = COaST2Constant (arrayn);
        if (tmp_co != NULL) {
            tmp = COgetDataVec (tmp_co);
            for (i = 0; i < shape; i++) {
                (*iarray)[i] = tmp[i];
            }
            tmp_co = COfreeConstant (tmp_co);
        } else {
            *iarray = ILIBfree (*iarray);
        }
    } else /* (NODE_TYPE(arrayn) == N_id) */ {
        DBUG_ASSERT ((NODE_TYPE (arrayn) == N_id), "wrong arrayn");

        if (TYisAKV (AVIS_TYPE (ID_AVIS (arrayn)))) {
            tmp
              = COgetDataVec (COcopyConstant (TYgetValue (AVIS_TYPE (ID_AVIS (arrayn)))));
            for (i = 0; i < shape; i++) {
                (*iarray)[i] = tmp[i];
            }
        } else {
            *iarray = ILIBfree (*iarray);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   intern_gen *WLFTree2InternGen(node *wln)
 *
 * description:
 *   copies generators of given WL to intern_gen struct. If filter is not
 *   NULL, only generators pointing at the N_Ncode node filter are copied.
 *
 ******************************************************************************/
intern_gen *
WLFtree2InternGen (node *wln, node *filter)
{
    intern_gen *root, *tmp_ig;
    node *partn, *genn;
    int shape;

    DBUG_ENTER ("WLFtree2InternGen");

    partn = WITH_PART (wln);
    root = NULL;
    tmp_ig = NULL;

    while (partn) {
        if (!filter || PART_CODE (partn) == filter) {
            genn = PART_GENERATOR (partn);
            shape = IDS_SHAPE (PART_VEC (partn), 0);
            tmp_ig = WLFappendInternGen (tmp_ig, shape, PART_CODE (partn),
                                         (GENERATOR_STEP (genn) != NULL)
                                           || (GENERATOR_WIDTH (genn) != NULL));
            if (!root)
                root = tmp_ig;

            /* copy vector information to intern_gen */
            WLFarrayST2ArrayInt (GENERATOR_BOUND1 (genn), &tmp_ig->l, shape); /* l */
            WLFarrayST2ArrayInt (GENERATOR_BOUND2 (genn), &tmp_ig->u, shape); /* u */
            if (GENERATOR_STEP (genn))
                WLFarrayST2ArrayInt (GENERATOR_STEP (genn), &tmp_ig->step,
                                     shape); /* step */
            if (GENERATOR_WIDTH (genn))
                WLFarrayST2ArrayInt (GENERATOR_WIDTH (genn), &tmp_ig->width,
                                     shape); /* width */

            /* normalize step and width */
            switch (WLFnormalizeInternGen (tmp_ig)) {
            case 1:
                CTIabortLine (NODE_LINE (wln), "Component of width greater than step");
            case 2:
                CTIabortLine (NODE_LINE (wln), "Component of width less 0");
            case 3:
                CTIabortLine (NODE_LINE (wln), "Width vector without step vector");
            }
        }

        partn = PART_NEXT (partn);
    }

    DBUG_RETURN (root);
}

/******************************************************************************
 *
 * function:
 *   node *WLFcreateArrayFromInternGen( int *source, int number)
 *
 * description:
 *   copies 'number' elements of the array source to an N_array struct and
 *   returns it.
 *
 ******************************************************************************/
static node *
WLFcreateArrayFromInternGen (int *source, int number)
{
    node *arrayn, *tmpn;
    int i;

    DBUG_ENTER ("WLFcreateArrayFromInternGen");

    tmpn = NULL;
    for (i = number - 1; i >= 0; i--) {
        tmpn = TBmakeExprs (TBmakeNum (source[i]), tmpn);
    }
    arrayn = TCmakeFlatArray (tmpn);

    DBUG_RETURN (arrayn);
}

/******************************************************************************
 *
 * function:
 *   node *WLFInternGen2Tree(node *arg_node, intern_gen *ig)
 *
 * description:
 *   copy intern_gen struct to the generators of the given WL. All existing
 *   N_Npart nodes are deleted before. Count number of N_Npart nodes and
 *   set WITH_PARTS. Return wln.
 *
 * remark:
 *  don't forget to free intern_gen chain.
 *
 ******************************************************************************/
node *
WLFinternGen2Tree (node *wln, intern_gen *ig)
{
    node **part, *withidn, *genn, *b1n, *b2n, *stepn, *widthn;
    int no_parts; /* number of N_Npart nodes */

    DBUG_ENTER ("WLFinternGen2Tree");

    withidn = DUPdoDupTree (PART_WITHID (WITH_PART (wln)));
    FREEdoFreeTree (WITH_PART (wln));
    part = &(WITH_PART (wln));
    no_parts = 0;

    /* create type for N_array nodes*/
#ifdef MWE_TYPE_READY
    while (ig) {
        /* create generator components */
        b1n = WLFcreateArrayFromInternGen (ig->l, ig->shape);
        b2n = WLFcreateArrayFromInternGen (ig->u, ig->shape);
        stepn = ig->step ? WLFcreateArrayFromInternGen (ig->step, ig->shape) : NULL;
        widthn = ig->width ? WLFcreateArrayFromInternGen (ig->width, ig->shape) : NULL;
#else
    while (ig) {
        /* create generator components */
        b1n = WLFcreateArrayFromInternGen (ig->l, ig->shape);
        b2n = WLFcreateArrayFromInternGen (ig->u, ig->shape);
        stepn = ig->step ? WLFcreateArrayFromInternGen (ig->step, ig->shape) : NULL;
        widthn = ig->width ? WLFcreateArrayFromInternGen (ig->width, ig->shape) : NULL;
#endif
        /* create tree structures */
        genn = TBmakeGenerator (F_le, F_lt, b1n, b2n, stepn, widthn);
        *part = TBmakePart (DUPdoDupTree (withidn), genn, ig->code);
        CODE_INC_USED (ig->code);

        ig = ig->next;
        part = &(PART_NEXT ((*part)));
        no_parts++;
    }

    WITH_PARTS (wln) = no_parts;

    FREEdoFreeTree (withidn);

    DBUG_RETURN (wln);
}

/******************************************************************************
 *
 * function:
 *   void WLFFreeInternGenChain(intern_gen *ig)
 *
 * description:
 *   Frees all memory allocated by ig and returns NULL (ig is NOT set to NULL).
 *
 ******************************************************************************/

intern_gen *
WLFfreeInternGen (intern_gen *tmp)
{
    DBUG_ENTER ("WLFfreeInternGen");

    DBUG_PRINT ("FREE", ("Removing intern gen (WLF)"));

    DBUG_ASSERT ((tmp != NULL), "cannot free a NULL intern gen (WLF)!");

    tmp->l = ILIBfree (tmp->l);
    tmp->u = ILIBfree (tmp->u);
    tmp->step = ILIBfree (tmp->step);
    tmp->width = ILIBfree (tmp->width);

    tmp = ILIBfree (tmp);

    DBUG_RETURN (tmp);
}

/******************************************************************************
 *
 * function:
 *   void WLFFreeInternGenChain(intern_gen *ig)
 *
 * description:
 *   Frees all memory allocated by ig and returns NULL (ig is NOT set to NULL).
 *
 ******************************************************************************/

intern_gen *
WLFfreeInternGenChain (intern_gen *ig)
{
    intern_gen *tmpig;

    DBUG_ENTER ("WLFfreeInternGenChain");

    while (ig) {
        tmpig = ig;
        ig = ig->next;
        WLFfreeInternGen (tmpig);
    }

    DBUG_RETURN (ig);
}

#ifdef MWE_TYPE_READY
/******************************************************************************
 *
 * function:
 *   node *WLFcreateVardec(char *name, ntype *type, node **vardecs)
 *
 * description:
 *   creates a new Vardec with 'name' of type 'type' at the beginning of
 *   the 'vardecs' chain. The node of the new Vardec is returned.
 *   If a vardec for this name already exists, this node is returned.
 *
 *
 * remark:
 *   new memory is allocated for name. It is expected that type
 *   is a pointer to an existing type  and it is duplicated, too.
 *
 *   does not preserve ssa form because of missing ssacount attribute
 *
 ******************************************************************************/
node *
WLFcreateVardec (char *name, ntype *type, node **vardecs)
{
    node *vardecn;
    char *c;

    DBUG_ENTER ("WLFcreateVardec");

    /* search for already existing vardec for this name. */
    vardecn = TCsearchDecl (name, *vardecs);

    /* if not found, create vardec. */
    if (!vardecn) {
        if (!type) {
            c = ILIBmalloc (50);
            c[0] = 0;
            c = strcat (c, "parameter type is NULL for variable ");
            c = strcat (c, name);
            DBUG_ASSERT (0, (c));
        }

        type = TYcopyType (type);
        vardecn = TBmakeVardec (ILIBstringCopy (name), NULL, type, *vardecs);
        VARDEC_VARNO (vardecn) = -1;

        /* create ssacnt node: to be implemented */

        *vardecs = vardecn;
    }

    DBUG_RETURN (vardecn);
}
#else
/******************************************************************************
 *
 * function:
 *   node *WLFcreateVardec(char *name, types *type, node **vardecs)
 *
 * description:
 *   creates a new Vardec with 'name' of type 'type' at the beginning of
 *   the 'vardecs' chain. The node of the new Vardec is returned.
 *   If a vardec for this name already exists, this node is returned.
 *
 *
 * remark:
 *   new memory is allocated for name. It is expected that type
 *   is a pointer to an existing type  and it is duplicated, too.
 *
 *   does not preserve ssa form because of missing ssacount attribute
 *
 ******************************************************************************/
node *
WLFcreateVardec (node *avis, node **vardecs)
{
    node *vardecn;
    /*char *c;*/ /* TODO must be change to new type */

    DBUG_ENTER ("WLFcreateVardec");

    /* search for already existing vardec for this name. */
    vardecn = TCsearchDecl (AVIS_NAME (avis), *vardecs);

    /* if not found, create vardec. */
    /*if (!vardecn) {
     *if (!AVIS_TYPE( avis));
     *  c = ILIBmalloc(50);
     *  c[0] = 0;
     *  c = strcat(c,"parameter type is NULL for variable ");
     *  c = strcat(c,name);
     *  DBUG_ASSERT(0,(c));
     *}
     *
     *type = DUPdupAllTypes(type); */
    vardecn = TBmakeVardec (avis, *vardecs);
    VARDEC_VARNO (vardecn) = -1;

    /* create ssacnt node: to be implemented */

    *vardecs = vardecn;
    /*}*/

    DBUG_RETURN (vardecn);
}
#endif

/******************************************************************************
 *
 * function:
 *   node *WLFwithloopFolding( node *arg_node, int loop)
 *
 * description:
 *   starting point for the withloop folding. it traverses the AST two times
 *     1. ssawli traversal to get needed information from tree
 *     2. ssawlf traversal to do the withloop folding
 *
 *   'loop' specifies the number of the current optimization cycle and is
 *   needed for correct handling of break specifiers.
 *
 *   after folding withloops the ssaform is restored by calling CheckAvis and
 *   WLFTransform.
 *
 ******************************************************************************/
node *
WLFdoWithloopFolding (node *arg_node, int loop)
{
    DBUG_ENTER ("WLFwithloopFolding");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "WLFwithloopFolding called for non fundef node");

    if (!(FUNDEF_ISLACFUN (arg_node))) {

        /* WLISSA traversal */
        WLIdoWLI (arg_node);

        /* break after WLI? */
        if ((global.break_after != PH_sacopt) || (global.break_cycle_specifier != loop)
            || strcmp (global.break_specifier, "wli")) {

            /* SSAWLF traversal: fold WLs */
            WLTdoWLT (arg_node);
        }

        /* restore ssa form */
        arg_node = SSArestoreSsaOneFunction (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFwithloopFoldingWLT( node *arg_node)
 *
 * description:
 *   executes only SSAWLT phase, not SSAWLI and SSAWLF.
 *   after withloop transformation the ssaform is restored by calling
 *   CheckAvis and SSATransform.
 *
 ******************************************************************************/
node *
WLFdoWithloopFoldingWlt (node *arg_node)
{
    DBUG_ENTER ("WLIwithloopFoldingWLT");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "WLIwithloopFoldingWLT not called for fundef node");

    if (!(FUNDEF_ISLACFUN (arg_node))) {
        /* start ssawlt traversal only in non-special fundefs */

        /* SSAWLT traversal: transform WLs */
        WLTdoWLT (arg_node);

        /* restore ssa form */
        arg_node = SSArestoreSsaOneFunction (arg_node);
    }

    DBUG_RETURN (arg_node);
}
