/*
 * this code implements the SSA-aware version of the original withloop folding.
 * it reuses most code of the old implementation, but the ssa form simplifies
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


 The following assumption, unfortunately, does not hold in
 an ssaiv environment. Specifically, each set of generators
 has DIFFERENT sets of vector and scalar WITHID names.


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

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"
#include "ctinfo.h"
#include "print.h"

#define DBUG_PREFIX "WLF"
#include "debug.h"

#include "traverse.h"
#include "constants.h"
#include "SSAWithloopFolding.h"
#include "SSAWLI.h"
#include "SSAWLF.h"
#include "new_types.h"
#include "shape.h"
#include "pattern_match.h"

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
    DBUG_ENTER ();

    pindex = (index_info *)MEMmalloc (sizeof (index_info));
    pindex->vector = vector;

    if (!vector)
        vector = 1;
    pindex->permutation = (int *)MEMmalloc (sizeof (int) * vector);
    pindex->last = (index_info **)MEMmalloc (sizeof (index_info *) * vector);
    pindex->const_arg = (int *)MEMmalloc (sizeof (int) * vector);

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
    index_info *xnew;
    int i, to;
    DBUG_ENTER ();
    DBUG_ASSERT (iinfo, "parameter NULL");

    xnew = WLFcreateIndex (iinfo->vector);

    to = iinfo->vector ? iinfo->vector : 1;
    for (i = 0; i < to; i++) {
        xnew->permutation[i] = iinfo->permutation[i];
        xnew->last[i] = iinfo->last[i];
        xnew->const_arg[i] = iinfo->const_arg[i];
    }

    xnew->mprf = iinfo->mprf;
    xnew->arg_no = iinfo->arg_no;

    DBUG_RETURN (xnew);
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

    DBUG_ENTER ();
    DBUG_ASSERT (N_id == NODE_TYPE (idn), "not an id node");

    /* get defining assignment via avis_ssaassign link */
    if (AVIS_SSAASSIGN (ID_AVIS (idn)) != NULL) {
        iinfo = ASSIGN_INDEX (AVIS_SSAASSIGN (ID_AVIS (idn)));
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

    DBUG_ENTER ();

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
                                global.prf_name[tmpii->mprf]);
                    else
                        printf ("|   .%s%d ", global.prf_name[tmpii->mprf],
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
                printf ("|   %d%s. ", tmpii->const_arg[sel],
                        global.prf_name[tmpii->mprf]);
            else
                printf ("|   %s%d. ", global.prf_name[tmpii->mprf],
                        tmpii->const_arg[sel]);
            printf ("|(p:%d, v:%d)\n", tmpii->permutation[sel], tmpii->vector);
        }
    }
    printf (
      "|---------------------------------------------------------------------------\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   int WLFlocateIndexVar(node *idn, node* wln)
 *
 * description:
 *   Searches for the Id (idn) in the WL generator (index var).
 *   The N_with node has to be available to find the index vars.
 *   That is, this function performs:
 *
 *         result = idn member (WITHID_VEC(wln) ++ WITHID_IDS(wln)
 *
 *   But, being C code, it has to be a little wordier...
 *
 * return:
 *   -1: Id is the index vector
 *    0: Id not found
 *    x with x gt 0: Id is the x'th scalar index variable.
 *
 * remark:
 *  In order to work with -ssaiv, we have to search the WITH_IDS
 *  of all partitions of the WL.
 *
 *   We can no longer, in the -ssaiv world, "exploit here that
 *   the index variables of all N_Withid nodes in
 *   one WL have the same names."
 *
 ******************************************************************************/
int
WLFlocateIndexVar (node *idn, node *wln)
{
    node *_ids;
    node *partn;
    node *mywln;
    int result = 0;
    int i;

    DBUG_ENTER ();
    DBUG_ASSERT (N_with == NODE_TYPE (wln), "wln is not N_with node");

    partn = WITH_PART (wln);
    while ((result == 0) && NULL != partn) {

        mywln = PART_WITHID (partn);
        _ids = WITHID_VEC (mywln);

        if (IDS_AVIS (_ids) == ID_AVIS (idn)) {
            DBUG_PRINT ("WLFlocateIndexVar found WITH_ID %s", AVIS_NAME (ID_AVIS (idn)));
            result = -1;
        } else {
            i = 1;
            _ids = WITHID_IDS (mywln);
            while (_ids != NULL) {
                if (IDS_AVIS (_ids) == ID_AVIS (idn)) {
                    result = i;
                    DBUG_PRINT ("WLFlocateIndexVar found WITH_IDS %s",
                                AVIS_NAME (ID_AVIS (idn)));
                    break;
                }
                i++;
                _ids = IDS_NEXT (_ids);
            }
        }
        partn = (TRUE == global.ssaiv) ? PART_NEXT (partn) : NULL;
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

    DBUG_ENTER ();

    ig = (intern_gen *)MEMmalloc (sizeof (intern_gen));
    ig->shape = shape;
    ig->code = NULL;
    ig->next = NULL;

    ig->l = (int *)MEMmalloc (sizeof (int) * shape);
    ig->u = (int *)MEMmalloc (sizeof (int) * shape);
    if (stepwidth) {
        ig->step = (int *)MEMmalloc (sizeof (int) * shape);
        ig->width = (int *)MEMmalloc (sizeof (int) * shape);
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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
 *   void WLFprintfInternGen (FILE *file, intern_gen *ig, bool with_code,
 *                            bool whole_chain)
 *
 * description:
 *   used for debugging purposes only.
 *   This function prints the given intern_gen structure ig to file.
 *   The argument with_code indicates whether to include the code and
 *   the argument whole_chain indicates whether to print the first generator
 *   only or all of them.
 *
 ******************************************************************************/
void
WLFprintfInternGen (FILE *file, intern_gen *ig, bool with_code, bool whole_chain)
{
    int i;
    DBUG_ENTER ();

    fprintf (file, "(L=[");
    for (i = 0; i < ig->shape; i++) {
        fprintf (file, "%d,", ig->l[i]);
    }
    fprintf (file, "], U=[");
    for (i = 0; i < ig->shape; i++) {
        fprintf (file, "%d,", ig->u[i]);
    }
    if (ig->step) {
        fprintf (file, "], S=[");
        for (i = 0; i < ig->shape; i++) {
            fprintf (file, "%d,", ig->step[i]);
        }
    }
    if (ig->width) {
        fprintf (file, "], W=[");
        for (i = 0; i < ig->shape; i++) {
            fprintf (file, "%d,", ig->width[i]);
        }
    }
    fprintf (file, "])");

    if (with_code) {
        fprintf (file, " code:");
        PRTdoPrintFile (file, ig->code);
    }

   if (whole_chain && (ig->next != NULL)) {
       fprintf (file, " followed by ");
       WLFprintfInternGen (file, ig->next, with_code, whole_chain);
   }

    DBUG_RETURN ();
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

    DBUG_ENTER ();

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
            ig->step = MEMfree (ig->step);
            ig->width = MEMfree (ig->width);
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
    pattern *pat;
    node *arr = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (iarray != NULL, "no iarray found!");

    /* Dereference arrayn to find N_array node, maybe */
    pat = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));
    if (PMmatchFlat (pat, arrayn)) {
        arrayn = arr;
    }
    pat = PMfree (pat);

    if (*iarray == NULL) {
        *iarray = (int *)MEMmalloc (shape * sizeof (int));
    }

    if (arrayn == NULL) {
        for (i = 0; i < shape; i++) {
            (*iarray)[i] = 0;
        }
    } else if (NODE_TYPE (arrayn) == N_array) {
        tmp_co = COaST2Constant (arrayn);
        if (tmp_co != NULL) {
            tmp = (int *)COgetDataVec (tmp_co);
            for (i = 0; i < shape; i++) {
                (*iarray)[i] = tmp[i];
            }
            tmp_co = COfreeConstant (tmp_co);
        } else {
            *iarray = MEMfree (*iarray);
        }
    } else /* (NODE_TYPE(arrayn) == N_id) */ {
        DBUG_ASSERT (NODE_TYPE (arrayn) == N_id, "wrong arrayn");

        if (TYisAKV (ID_NTYPE (arrayn))) {
            tmp = (int *)COgetDataVec (TYgetValue (ID_NTYPE (arrayn)));
            for (i = 0; i < shape; i++) {
                (*iarray)[i] = tmp[i];
            }
        } else {
            *iarray = MEMfree (*iarray);
        }
    }

    DBUG_RETURN ();
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

    DBUG_ENTER ();

    partn = WITH_PART (wln);
    root = NULL;
    tmp_ig = NULL;

    while (partn) {
        if (!filter || PART_CODE (partn) == filter) {
            genn = PART_GENERATOR (partn);
            shape = SHgetUnrLen (TYgetShape (IDS_NTYPE (PART_VEC (partn))));
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
                CTIabort (NODE_LOCATION (wln), "Component of width greater than step");
            case 2:
                CTIabort (NODE_LOCATION (wln), "Component of width less 0");
            case 3:
                CTIabort (NODE_LOCATION (wln), "Width vector without step vector");
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

    DBUG_ENTER ();

    tmpn = NULL;
    for (i = number - 1; i >= 0; i--) {
        tmpn = TBmakeExprs (TBmakeNum (source[i]), tmpn);
    }
    arrayn = TCmakeIntVector (tmpn);

    DBUG_RETURN (arrayn);
}

/******************************************************************************
 *
 * function:
 *   node *WLFInternGen2Tree(node *arg_node, intern_gen *ig)
 *
 * description:
 *   copy intern_gen struct to the generators of the given WL. All existing
 *   N_Npart nodes are deleted before. Count number of N_Npart nodes
 *   Return wln.
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

    DBUG_ENTER ();

    withidn = DUPdoDupTree (PART_WITHID (WITH_PART (wln)));
    FREEdoFreeTree (WITH_PART (wln));
    part = &(WITH_PART (wln));
    no_parts = 0;

    /* create type for N_array nodes*/
    while (ig) {
        /* create generator components */
        b1n = WLFcreateArrayFromInternGen (ig->l, ig->shape);
        b2n = WLFcreateArrayFromInternGen (ig->u, ig->shape);
        stepn = ig->step ? WLFcreateArrayFromInternGen (ig->step, ig->shape) : NULL;
        widthn = ig->width ? WLFcreateArrayFromInternGen (ig->width, ig->shape) : NULL;
        /* create tree structures */
        genn = TBmakeGenerator (F_wl_le, F_wl_lt, b1n, b2n, stepn, widthn);
        *part = TBmakePart (ig->code, DUPdoDupTree (withidn), genn);
        CODE_INC_USED (ig->code);

        ig = ig->next;
        part = &(PART_NEXT ((*part)));
        no_parts++;
    }

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
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("FREE", "Removing intern gen (WLF)");

    DBUG_ASSERT (tmp != NULL, "cannot free a NULL intern gen (WLF)!");

    tmp->l = MEMfree (tmp->l);
    tmp->u = MEMfree (tmp->u);
    tmp->step = MEMfree (tmp->step);
    tmp->width = MEMfree (tmp->width);

    tmp = MEMfree (tmp);

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

    DBUG_ENTER ();

    while (ig) {
        tmpig = ig;
        ig = ig->next;
        WLFfreeInternGen (tmpig);
    }

    DBUG_RETURN (ig);
}

#undef DBUG_PREFIX
