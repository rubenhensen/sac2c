/*
 *
 * $Log$
 * Revision 2.15  2000/08/01 13:16:10  dkr
 * WithloopFolding(), WithloopFoldingWLT(): parameter arg_info removed
 * WithloopFolding(): paramter loop added in order to handle break
 * specifiers correctly
 *
 * Revision 2.14  2000/07/28 14:35:11  dkr
 * minor changes in DBUG output done
 *
 * Revision 2.13  2000/07/12 15:13:17  dkr
 * function DuplicateTypes renamed into DupTypes
 *
 * Revision 2.12  2000/06/23 15:24:45  dkr
 * signature of DupTree changed
 *
 * Revision 2.11  2000/06/23 14:03:41  dkr
 * nodetype N_with removed
 *
 * Revision 2.10  2000/05/11 11:15:46  dkr
 * Function MakeNullVec renamed into CreateZeroVector
 *
 * Revision 2.9  2000/01/26 17:26:04  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.8  1999/07/15 20:38:11  sbs
 * ARRAY_ISCONST set where MakeArray is called.
 *
 * Revision 2.7  1999/07/14 14:36:32  bs
 * Bug fixed in ArrayST2ArrayInt.
 *
 * Revision 2.6  1999/07/08 14:54:27  sbs
 * Array2BoolVec used instead of Array2IntVec
 *
 * Revision 2.5  1999/05/12 11:39:24  jhs
 * Adjusted macros to new access on constant vectors.
 *
 * Revision 2.4  1999/03/31 10:52:26  bs
 * Bug in MakeNullVec fixed.
 *
 * Revision 2.3  1999/03/19 09:44:38  bs
 * Every Call of MakeArray will also create a compact vector propagation.
 *
 * Revision 2.2  1999/02/26 14:49:13  dkr
 * file moved from folder /optimize
 *
 * Revision 2.1  1999/02/23 12:41:42  sacbase
 * new release made
 *
 * Revision 1.26  1999/02/15 11:15:00  srs
 * MakeNullVec() can create scalars of all simpletypes now
 *
 * Revision 1.25  1999/02/12 13:48:08  srs
 * fixed infinite recursion in SearchWL()
 *
 * Revision 1.24  1999/02/06 13:35:15  srs
 * fixed bug in SearchWL()
 *
 * Revision 1.23  1999/02/01 11:59:46  cg
 * Bug fixed in MakeNullVec(): bool and char arrays are now handled
 * as well.
 *
 * Revision 1.22  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.21  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.20  1998/11/18 15:07:24  srs
 * N_empty nodes are supported now
 *
 * Revision 1.19  1998/07/14 12:58:25  srs
 * enhanced ASSERT text of CreateVardec()
 *
 * Revision 1.18  1998/05/15 14:41:01  srs
 * changed MakeNullVec() and added break specifier bo:wli
 *
 * Revision 1.17  1998/05/05 13:00:44  srs
 * splitted WLT and WLI/WLF
 *
 * Revision 1.16  1998/05/05 11:20:44  srs
 * changed SearchWL()
 *
 * Revision 1.15  1998/04/24 18:57:54  srs
 * added creation of types for N_array node
 *
 * Revision 1.14  1998/04/24 17:31:44  srs
 * added MakeNullVec() and changed CreateVardec()
 *
 * Revision 1.13  1998/04/20 09:07:59  srs
 * changed CreateInternGen() and WithloopFoldingWLT()
 *
 * Revision 1.12  1998/04/08 20:37:40  srs
 * Added several functions, changed arguments of Tree2InternGen(),
 * InternGen2Tree() counts NWITH_PARTS.
 *
 * Revision 1.11  1998/04/07 08:21:41  srs
 * changed SearchWL() and inserted second WLI-phase
 *
 * Revision 1.10  1998/04/03 11:38:52  srs
 * changed SearchWL, CreateArrayFromInternGen
 *
 * Revision 1.9  1998/04/01 07:51:53  srs
 * added many functions
 *
 * Revision 1.8  1998/03/22 18:13:34  srs
 * splitted file to
 * - WLT.c tranformations of WLs
 * - WLI.c gather information about WLs
 * - WLF.c fold WLs
 *
 * Revision 1.7  1998/03/18 08:33:07  srs
 * first running version of WLI
 *
 * Revision 1.6  1998/03/06 13:32:54  srs
 * added some WLI functions
 *
 * Revision 1.5  1998/02/27 13:38:10  srs
 * checkin with deactivated traversal
 *
 */

/*******************************************************************************
 This file organizes the withloop folding and makes some basic functions
 available.

 Withloop folding is done in 3 phases:
 1) WLT transforms WLs to apply the following phases
 1) WLI gathers information about the WLs
 2) WLF finds and folds suitable WLs.


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

#include "tree.h"
#include "types.h"
#include "internal_lib.h"
#include "free.h"
#include "print.h"
#include "DupTree.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "optimize.h"
#include "generatemasks.h"
#include "ConstantFolding.h"
#include "WithloopFolding.h"
#include "WLT.h"
#include "WLI.h"
#include "WLF.h"

/******************************************************************************
 *
 *  global variables
 *
 ******************************************************************************/

nodelist *search_wl_nodelist; /* usage: see comment in code of SearchWL() */

/******************************************************************************
 *
 *  forward declarations
 *
 ******************************************************************************/

node *SearchWL (int, node *, int *, int, int);

/******************************************************************************
 *
 * function:
 *   void DbugIndexInfo(index info *iinfo)
 *
 * description:
 *   prints history of iinfo.
 *
 *
 ******************************************************************************/

void
DbugIndexInfo (index_info *iinfo)
{
    int i, sel;
    index_info *tmpii;

    DBUG_ENTER ("DbugIndexInfo");

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
                                prf_string[tmpii->prf]);
                    else
                        printf ("|   .%s%d ", prf_string[tmpii->prf],
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
                printf ("|   %d%s. ", tmpii->const_arg[sel], prf_string[tmpii->prf]);
            else
                printf ("|   %s%d. ", prf_string[tmpii->prf], tmpii->const_arg[sel]);
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
 *   void DbugInternGen(intern_gen *ig)
 *
 * description:
 *   prints all informations of the given intern_gen chain.
 *
 *
 ******************************************************************************/

void
DbugInternGen (intern_gen *ig)
{
    int i;

    DBUG_ENTER ("DbugInternGen");

    while (ig) {
        printf ("[%d", ig->l[0]);
        for (i = 1; i < ig->shape; i++)
            printf (",%d", ig->l[i]);
        printf ("] -> [%d", ig->u[0]);
        for (i = 1; i < ig->shape; i++)
            printf (",%d", ig->u[i]);
        printf ("]");

        if (ig->step) {
            printf (" step [%d", ig->step[0]);
            for (i = 1; i < ig->shape; i++)
                printf (",%d", ig->step[i]);
            printf ("]");
        }
        if (ig->width) {
            printf (" width [%d", ig->width[0]);
            for (i = 1; i < ig->shape; i++)
                printf (",%d", ig->width[i]);
            printf ("]");
        }

        printf ("   code: %p\n", ig->code);

        ig = ig->next;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   index_info *CreateIndex(int vector)
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
CreateIndex (int vector)
{
    index_info *pindex;
    DBUG_ENTER ("CreateInfoInfo");

    pindex = Malloc (sizeof (index_info));
    pindex->vector = vector;

    if (!vector)
        vector = 1;
    pindex->permutation = Malloc (sizeof (int) * vector);
    pindex->last = Malloc (sizeof (index_info *) * vector);
    pindex->const_arg = Malloc (sizeof (int) * vector);

    pindex->arg_no = 0;

    DBUG_RETURN (pindex);
}

/******************************************************************************
 *
 * function:
 *   index_info *DuplicateIndexInfo(index_info *iinfo)
 *
 * description:
 *   duplicates struct
 *
 ******************************************************************************/

index_info *
DuplicateIndexInfo (index_info *iinfo)
{
    index_info *new;
    int i, to;
    DBUG_ENTER ("DuplicateIndexInfo");
    DBUG_ASSERT (iinfo, ("parameter NULL"));

    new = CreateIndex (iinfo->vector);

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
 *   node *ValicLocalId(node *idn)
 *
 * description:
 *   returns pointer to index_info if Id (idn) is a valid variable within
 *   the WL body (index vars are excluded). Returns NULL otherwise.
 *
 ******************************************************************************/

index_info *
ValidLocalId (node *idn)
{
    index_info *iinfo;

    DBUG_ENTER ("ValicLocalId");
    DBUG_ASSERT (N_id == NODE_TYPE (idn), ("not an id node"));

    idn = MRD (ID_VARNO (idn));
    if (idn)
        iinfo = INDEX (idn);
    else
        iinfo = NULL;

    DBUG_RETURN (iinfo);
}

/******************************************************************************
 *
 * function:
 *   int LocateIndexVar(node *idn, node* wln)
 *
 * description:
 *   Searches for the Id (idn) in the WL generator (index var).
 *   The N_Nwith node has to be available to find the index vars.
 *
 * return:
 *   -1: Id is the index vector
 *    0: Id not found
 *    x with x gt 0: Id is the x'th scalar index variable.
 *
 * remark:
 *   we exploit here that the index variables of all N_Nwithid nodes in
 *   one WL have the same names.
 *
 ******************************************************************************/

int
LocateIndexVar (node *idn, node *wln)
{
    ids *_ids;
    int result = 0, i;

    DBUG_ENTER ("LocateIndexVar");
    DBUG_ASSERT (N_Nwith == NODE_TYPE (wln), ("wln is not N_Nwith node"));

    wln = NPART_WITHID (NWITH_PART (wln));
    _ids = NWITHID_VEC (wln);

    if (!strcmp (IDS_NAME (_ids), ID_NAME (idn)))
        result = -1;

    i = 1;
    _ids = NWITHID_IDS (wln);
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
 *   intern_gen *CreateInternGen(int shape, int stepwidth)
 *
 * description:
 *   allocate memory for an intern_gen struct. The parameter shape is needed
 *   to allocate right sized mem for the bounds. If stepwidth is not 0,
 *   memory for step/width is allocated, too.
 *
 ******************************************************************************/

intern_gen *
CreateInternGen (int shape, int stepwidth)
{
    intern_gen *ig;

    DBUG_ENTER ("CreateInternGen");

    ig = Malloc (sizeof (intern_gen));
    ig->shape = shape;
    ig->code = NULL;
    ig->next = NULL;

    ig->l = Malloc (sizeof (int) * shape);
    ig->u = Malloc (sizeof (int) * shape);
    if (stepwidth) {
        ig->step = Malloc (sizeof (int) * shape);
        ig->width = Malloc (sizeof (int) * shape);
    } else {
        ig->step = NULL;
        ig->width = NULL;
    }

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   intern_gen *AppendInternGen(...)
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
AppendInternGen (intern_gen *append_to, int shape, node *code, int stepwidth)
{
    intern_gen *ig;
    int i;

    DBUG_ENTER ("AppendInternGen");

    ig = CreateInternGen (shape, stepwidth);

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
 *   intern_gen *CopyInternGen(intern_gen *source)
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
CopyInternGen (intern_gen *source)
{
    intern_gen *ig;
    int i;

    DBUG_ENTER ("CopyInternGen");

    ig = CreateInternGen (source->shape, NULL != source->step);
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
 *   intern_gen *MoveInternGen(intern_gen *source, intern_gen *dest)
 *
 * description:
 *   expects two intern_gen structs and inserts the first struct from source
 *   at the beginning of dest (*dest is changed). The next struct of source
 *   is returned.
 *
 ******************************************************************************/

intern_gen *
MoveInternGen (intern_gen *source, intern_gen **dest)
{
    intern_gen *ret;

    DBUG_ENTER ("MoveInternGen");
    DBUG_ASSERT (source, ("source is NULL"));

    ret = source->next;
    source->next = *dest;
    *dest = source;

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   int NormalizeInternGen(intern_gen *ig)
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
NormalizeInternGen (intern_gen *ig)
{
    int error = 0, i = 0, is_1 = 1;

    DBUG_ENTER ("NormalizeInternGen");

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
            FREE (ig->step);
            FREE (ig->width);
        }
    }

    DBUG_RETURN (error);
}

/******************************************************************************
 *
 * function:
 *   void ArrayST2ArrayInt(node *arrayn, int **iarray, int shape)
 *
 * description:
 *   copies 'shape' numbers of given constant array node (1 dimension) to
 *   *iarray. If *iarray = NULL, memory is allocated. If arrayn is NULL, iarray
 *   is filles with 0.
 *
 ******************************************************************************/

void
ArrayST2ArrayInt (node *arrayn, int **iarray, int shape)
{
    int i;

    DBUG_ENTER ("ArrayST2ArrayInt");

    if (!*iarray)
        *iarray = Malloc (shape * sizeof (int));

    if (arrayn == NULL) {
        for (i = 0; i < shape; i++)
            (*iarray)[i] = 0;
    } else if (NODE_TYPE (arrayn) == N_array) {
        for (i = 0; i < shape; i++)
            (*iarray)[i] = ((int *)ARRAY_CONSTVEC (arrayn))[i];
    } else /* (NODE_TYPE(arrayn) == N_id) */ {
        DBUG_ASSERT ((NODE_TYPE (arrayn) == N_id), ("Wrong arrayn"));

        for (i = 0; i < shape; i++)
            (*iarray)[i] = ((int *)ID_CONSTVEC (arrayn))[i];
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   intern_gen *Tree2InternGen(node *wln)
 *
 * description:
 *   copies generators of given WL to intern_gen struct. If filter is not
 *   NULL, only generators pointing at the N_Ncode node filter are copied.
 *
 ******************************************************************************/

intern_gen *
Tree2InternGen (node *wln, node *filter)
{
    intern_gen *root, *tmp_ig;
    node *partn, *genn;
    int shape;

    DBUG_ENTER ("Tree2InternGen");

    partn = NWITH_PART (wln);
    root = NULL;
    tmp_ig = NULL;

    while (partn) {
        if (!filter || NPART_CODE (partn) == filter) {
            genn = NPART_GEN (partn);
            shape = IDS_SHAPE (NPART_VEC (partn), 0);
            tmp_ig = AppendInternGen (tmp_ig, shape, NPART_CODE (partn),
                                      (int)NGEN_STEP (genn) || (int)NGEN_WIDTH (genn));
            if (!root)
                root = tmp_ig;

            /* copy vector information to intern_gen */
            ArrayST2ArrayInt (NGEN_BOUND1 (genn), &tmp_ig->l, shape); /* l */
            ArrayST2ArrayInt (NGEN_BOUND2 (genn), &tmp_ig->u, shape); /* u */
            if (NGEN_STEP (genn))
                ArrayST2ArrayInt (NGEN_STEP (genn), &tmp_ig->step, shape); /* step */
            if (NGEN_WIDTH (genn))
                ArrayST2ArrayInt (NGEN_WIDTH (genn), &tmp_ig->width, shape); /* width */

            /* normalize step and width */
            switch (NormalizeInternGen (tmp_ig)) {
            case 1:
                ABORT (NODE_LINE (wln), ("component of width greater than step"));
            case 2:
                ABORT (NODE_LINE (wln), ("component of width less 0"));
            case 3:
                ABORT (NODE_LINE (wln), ("width vector without step vector"));
            }
        }

        partn = NPART_NEXT (partn);
    }

    DBUG_RETURN (root);
}

/******************************************************************************
 *
 * function:
 *   node *CreateArrayFromInternGen(int *source, int number)
 *
 * description:
 *   copies 'number' elements of the array source to an N_array struct and
 *   returns it.
 *
 ******************************************************************************/

node *
CreateArrayFromInternGen (int *source, int number, types *type)
{
    node *arrayn, *tmpn;
    int i;

    DBUG_ENTER ("CreateArrayFromInternGen");

    tmpn = NULL;
    for (i = number - 1; i >= 0; i--) {
        tmpn = MakeExprs (MakeNum (source[i]), tmpn);
    }
    arrayn = MakeArray (tmpn);
    ARRAY_ISCONST (arrayn) = TRUE;
    ARRAY_VECTYPE (arrayn) = T_int;
    ((int *)ARRAY_CONSTVEC (arrayn)) = Array2IntVec (tmpn, NULL);
    ARRAY_VECLEN (arrayn) = number;
    ARRAY_TYPE (arrayn) = DupTypes (type);

    DBUG_RETURN (arrayn);
}

/******************************************************************************
 *
 * function:
 *   node *InternGen2Tree(node *arg_node, intern_gen *ig)
 *
 * description:
 *   copy intern_gen struct to the generators of the given WL. All existing
 *   N_Npart nodes are deleted before. Count number of N_Npart nodes and
 *   set NWITH_PARTS. Return wln.
 *
 * remark:
 *  don't forget to free intern_gen chain.
 *
 ******************************************************************************/

node *
InternGen2Tree (node *wln, intern_gen *ig)
{
    node **part, *withidn, *genn, *b1n, *b2n, *stepn, *widthn;
    types *type;
    shpseg *shpseg;
    int no_parts; /* number of N_Npart nodes */

    DBUG_ENTER ("InternGen2Tree");

    withidn = DupTree (NPART_WITHID (NWITH_PART (wln)));
    FreeTree (NWITH_PART (wln));
    part = &(NWITH_PART (wln));
    no_parts = 0;

    /* create type for N_array nodes*/
    shpseg = MakeShpseg (
      MakeNums (ig->shape, NULL)); /* nums struct is freed inside MakeShpseg. */
    type = MakeType (T_int, 1, shpseg, NULL, NULL);

    while (ig) {
        /* create generator components */
        b1n = CreateArrayFromInternGen (ig->l, ig->shape, type);
        b2n = CreateArrayFromInternGen (ig->u, ig->shape, type);
        stepn = ig->step ? CreateArrayFromInternGen (ig->step, ig->shape, type) : NULL;
        widthn = ig->width ? CreateArrayFromInternGen (ig->width, ig->shape, type) : NULL;

        /* create tree structures */
        genn = MakeNGenerator (b1n, b2n, F_le, F_lt, stepn, widthn);
        *part = MakeNPart (DupTree (withidn), genn, ig->code);

        ig = ig->next;
        part = &(NPART_NEXT ((*part)));
        no_parts++;
    }

    NWITH_PARTS (wln) = no_parts;

    FREE (withidn);
    FreeOneTypes (type);

    DBUG_RETURN (wln);
}

/******************************************************************************
 *
 * function:
 *   void FreeInternGenChain(intern_gen *ig)
 *
 * description:
 *   Frees all memory allocated by ig and returns NULL (ig is NOT set to NULL).
 *
 ******************************************************************************/

intern_gen *
FreeInternGenChain (intern_gen *ig)
{
    intern_gen *tmpig;

    DBUG_ENTER ("FreeInternGenChain");

    while (ig) {
        tmpig = ig;
        ig = ig->next;
        FREE_INTERN_GEN (tmpig);
    }

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   node *CreateVardec(char *name, types *type, node **vardecs)
 *
 * description:
 *   creates a new Vardec with 'name' of type 'type' at the beginning of
 *   the 'vardecs' chain. The node of the new Vardec is returned.
 *   If a vardec for this name already exists, this node is returned.
 *
 * remark:
 *   new memory is allocated for name. It is expected that type
 *   is a pointer to an existing type  and it is duplicated, too.
 *
 ******************************************************************************/

node *
CreateVardec (char *name, types *type, node **vardecs)
{
    node *vardecn;
    char *c;

    DBUG_ENTER ("CreateVardec");

    /* search for already existing vardec for this name. */
    vardecn = SearchDecl (name, *vardecs);

    /* if not found, create vardec. */
    if (!vardecn) {
        if (!type) {
            c = Malloc (50);
            c[0] = 0;
            c = strcat (c, "parameter type is NULL for variable ");
            c = strcat (c, name);
            DBUG_ASSERT (0, (c));
        }

        type = DupTypes (type);
        vardecn = MakeVardec (StringCopy (name), type, *vardecs);
        VARDEC_VARNO (vardecn) = -1;

        *vardecs = vardecn;
    }

    DBUG_RETURN (vardecn);
}

/******************************************************************************
 *
 * function:
 *   void SearchWLHelp(node *idn, node *assignn)
 *
 * description:
 *   help function for SearchWL.
 *   search last definition of id_varno in this block and mark it invalid.
 *
 ******************************************************************************/

void
SearchWLHelp (int id_varno, node *assignn, int *valid, int mode, int ol)
{
    *valid = -1;

    if (NODE_TYPE (assignn) == N_empty) {
        /* we entered an empty assignment-block. We have no chance to
           find a WL here, so we can exit without doing anything. */
    } else {
        while (ASSIGN_NEXT (assignn))
            assignn = ASSIGN_NEXT (assignn);

        /* if Id is defined in this last instr of the body*/
        if (ASSIGN_DEFMASK (assignn)[id_varno]) {
            if (N_let == ASSIGN_INSTRTYPE (assignn)
                && N_Nwith == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assignn)))) {
                /* this is the bad boy */
                if (0 == mode || 1 == mode)
                    NWITH_NO_CHANCE (LET_EXPR (ASSIGN_INSTR (assignn))) = TRUE;
                if (0 == mode)
                    NWITH_REFERENCED (LET_EXPR (ASSIGN_INSTR (assignn)))++;
            } else /* if not, this may be a compound node. */
                SearchWL (id_varno, assignn, valid, mode, ol);
        } else { /* else it is defined somewhere above and we can use the ASSIGN_MRDMASK
                  */
            assignn = (node *)ASSIGN_MRDMASK (assignn)[id_varno];
            SearchWL (id_varno, assignn, valid, mode, ol);
        }
    }
}

/******************************************************************************
 *
 * function:
 *   node *SearchWL(node *idn, node *start_search, int *valid);
 *
 * description:
 *   Searches a WL Id (specified with id_varno) somewhere above in the syntax
 *   tree, starting at startn. startn has to be the first possible tip
 *   where to find the WL, not the assign node of the Id itself. (Example:
 *     SearchWL(varno, (node*)ASSIGN_MRDMASK(assignn)[varno], )   ).
 *
 *   Returns an assign node, if the Id describes a WL (N_Nwith). This of
 *   course is the assign node of the WL. Else NULL. See remarks.
 *
 *   The parameter *valid is set to 0 iff the name decribes a WL BUT if there
 *   is no chance that the WL can be used for folding. That is when it is
 *   inside a do/while loop or inside a conditional and referenced (idn)
 *   outside. Else it is 1.
 *
 * attention:
 *   this functions modifies the found WL subject to mode. See documentaion
 *   of StartSearchWL for more information on mode.
 *   Make sure that *valid is not 1 if called. This is an internal flag
 *   used in conjunction with SearchWLHelp.
 *
 * remarks:
 *   The assign node is only returned if *valid is 1. Else we would have a
 *   problem returning many nodes if a WL is defined inside of both
 *   trees of a conditional or a loop, which could be recursive.
 *   If this happens (*valid==0) we still continue to find the WL to
 *   mark it (see mode).
 *
 *  to understand the usage of search_wl_nodelist see comment in N_let case.
 *
 ******************************************************************************/

node *
SearchWL (int id_varno, node *startn, int *valid, int mode, int original_level)
{
    node *tmpn;
    int loop_level, ct, ce;

    DBUG_ENTER ("SearchWL");

    /* MRDs point at N_assign or N_Npart nodes.
       If N_Npart, the Id is an index vector, not a WL. Return NULL */
    if (startn && (N_Npart == NODE_TYPE (startn))) {
        startn = NULL;
    }

    /* If we reached a node which is marked in search_wl_nodelist we
       abort recursion because this node has already been traversed. */
    if (startn && NodeListFind (search_wl_nodelist, startn))
        startn = NULL;

    if (startn) { /* This is an N_assign node. */
        DBUG_ASSERT (N_assign == NODE_TYPE (startn), ("startn is not an assign node"));
        loop_level = ASSIGN_LEVEL (startn);

        while (loop_level > original_level) {
            /* jumped INTO a do-loop. The MRDs of do loops are stored
               differently than the MRDs in while loops. Correct this: find
               do node of same level. That's the equivalent to
               while loops that we expect. */
            startn = (node *)ASSIGN_MRDMASK (startn)[id_varno];
            loop_level = ASSIGN_LEVEL (startn);
        }

        switch (ASSIGN_INSTRTYPE (startn)) {
        case N_while:
        case N_do:
            if (N_while == ASSIGN_INSTRTYPE (startn))
                tmpn = BLOCK_INSTR (WHILE_BODY (ASSIGN_INSTR (startn))); /* while */
            else
                tmpn = BLOCK_INSTR (DO_BODY (ASSIGN_INSTR (startn))); /* do */

            /* now we have to distinguish where the 'pointer comes from'. Do we
               use the Id from within the loop or from behind it? */
            if (loop_level == original_level) {
                /* the referencing Id is behind the loop, same level. It
                   references an ID within the loop. If this ID is defined by
                   a WL, we definitly cannot use this WL for folding. */

                /* we have to find out if idn references a WL. Therefore
                   we traverse the body to its last assignment and call SearchWL
                   again. This happens in SearchWLHelp. */
                SearchWLHelp (id_varno, tmpn, valid, mode, original_level + 1);

                /* additionally, in case of a while loop, we have to search
                   for a WL before the loop, too. */
                if (N_while == ASSIGN_INSTRTYPE (startn)) {
                    tmpn = (node *)ASSIGN_MRDMASK (startn)[id_varno];
                    *valid = -1;
                    SearchWL (id_varno, tmpn, valid, mode, original_level);
                }
            } else { /* loop_level < original_level */
                /* this means: Id is defined in this loop (and of course before the loop,
                   too), but we did not find it directly by MRD. Hence it is defined
                   - below the occurence of the Id and we connot use it for folding.
                   - as index of an OLD WL which does not store MRDs correctly in
                     its assign nodes but only in superior compound nodes. */

                /* first, mark this node in search_wl_nodelist.  */
                search_wl_nodelist = NodeListAppend (search_wl_nodelist, startn, NULL);

                /* Search last definition in body and mark it invalid. This
                   MAY go wrong if we don't create MRDs in wli_phase 1. */
                SearchWLHelp (id_varno, tmpn, valid, mode, loop_level + 1);

                /* Search definition above loop to mark it invalid. */
                tmpn = (node *)ASSIGN_MRDMASK (startn)[id_varno];
                *valid = -1;
                SearchWL (id_varno, tmpn, valid, mode, loop_level);
            }
            *valid = 0;
            break;

        case N_cond:
            if (loop_level == original_level) {
                ct = ce = 0;
                if ((node *)COND_THENDEFMASK (ASSIGN_INSTR (startn))[id_varno]) {
                    /* is Id defined in the then subtree? */
                    ct = 1;
                    tmpn = COND_THENINSTR (ASSIGN_INSTR (startn)); /* then-part */
                    SearchWLHelp (id_varno, tmpn, valid, mode, original_level + 1);
                }
                if ((node *)COND_ELSEDEFMASK (ASSIGN_INSTR (startn))[id_varno]) {
                    ce = 1;
                    tmpn = COND_ELSEINSTR (ASSIGN_INSTR (startn)); /* else-part */
                    SearchWLHelp (id_varno, tmpn, valid, mode, original_level + 1);
                }
                if (!ct || !ce) { /* if Id is not defined in both trees,
                                     continue search above the conditional. */
                    tmpn = (node *)ASSIGN_MRDMASK (startn)[id_varno];
                    *valid = -1;
                    SearchWL (id_varno, tmpn, valid, mode, original_level);
                }

                *valid = 0;
            } else { /* loop_level < original_level */
                /* This means: Id is defined in this condition, but we did not
                   find it by MRD because it is defined in the other branch or
                   behind the occurance of Id in the same branch. Neither definition
                   interests us because we search the definition that is bound
                   to the current Id. This HAS (*) to be defined before the conditional,
                   so call SearchWL with a new startn and loop_level.
                   (*)Exception: Index Variables of OLD WLs, see comment above. */
                tmpn = (node *)ASSIGN_MRDMASK (startn)[id_varno];
                startn = SearchWL (id_varno, tmpn, valid, mode, loop_level);
            }
            break;

        case N_let:
            /*   consider the following example:
                 w = ...
                 do {
                   ...1
                   w = w;
                   ...2
                 }

                 where ...1 and ...2 does not define w.
                 This example can only exist if CF or DCR is disabled.

                 Now, when we reach the assignment w = w in this context, we
                 start to search for the right Id - for w. The MRD points to
                 the do-node, so we know that w is not defined in ...1. That's
                 why we start to search w in ...2. Again we reach the
                 assignment w=w and would end in an infinite recursion.

                 Another problem with the same effect can happen even with a more
                 complex expression. Instead of w=w examine the example
                 w=modarray(w,.,.) below.

                 while(.)                    (1)
                   ...1
                   while(.)                  (2)
                     ...2
                     w = modarray(w,.,.);    (3)
                     ...3

                 Here we start in line (3), search for w and find the mrd in (2).
                 Because w is not defined in ...2, we search again in ...3 (and
                 find (3) which is no problem) and outside the loop (2). Since
                 w is not defined in ...1, we find the next mrd of w in (1).
                 Hence we search w before (1) (which is no problem) and then
                 we traverse the body of (1) again. We see that w is defined in
                 (2), traverse into (2) and reach the line (3) again which
                 leads to an infinite recursion.

                 solution: we introduce a node list search_wl_nodelist to store
                 nodes of loops where we already have been. If we reach such a
                 loop again, we can abort recursion. */

            /* if the Id we search for is renamed, let's search for the new
               name to find a referenced WL. */
            if (N_id == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (startn)))) {
                id_varno = ID_VARNO (LET_EXPR (ASSIGN_INSTR (startn)));
                tmpn = (node *)ASSIGN_MRDMASK (startn)[id_varno];
                startn = SearchWL (id_varno, tmpn, valid, mode, original_level);
            } else if (N_Nwith == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (startn)))) {
                /* now we found a WL */
                if (0 == mode)
                    NWITH_REFERENCED (LET_EXPR (ASSIGN_INSTR (startn)))++;
                if (-1 == *valid) { /* but we cannot use it for folding */
                    if (0 == mode || 1 == mode)
                        NWITH_NO_CHANCE (LET_EXPR (ASSIGN_INSTR (startn))) = TRUE;
                } else { /* *valid is 1 and the assign node of the WL is in startn. */
                    DBUG_ASSERT (1 == *valid, ("wrong value for variable valid"));
                    /*           if (1 == mode) */
                    /*             NWITH_REFERENCED_FOLD(LET_EXPR(ASSIGN_INSTR(startn)))++;
                     */
                }
            } else {
                startn = NULL; /* this is not a WL. */
            }
            break;

        default:
            DBUG_ASSERT (0, ("should not happen. Which node?"));
            break;
        }
    }

    if (!*valid)
        startn = NULL;

    DBUG_RETURN (startn);
}

/******************************************************************************
 *
 * function:
 *   node *StartSearchWL(node *idn, node *assignn)
 *
 * description:
 *   This function searches the MRD (WL) for idn. If a WL is found which
 *   can be used for folding, the assign node of this WL is returned.
 *   assignn is the assign node in which idn is situated.
 *   mode is used as follows:
 *
 *   0: If a WL is found which can not be used for folding, NWITH_NO_CHANCE is
 *      set. Additionally, for every found WL, NWITH_REFERENCED is incremented.
 *
 *   1: If a WL is found which can not be used for folding, NWITH_NO_CHANCE is
 *      set.
 *
 *   2: No special behaviour, just returns found WL or NULL.
 *
 *   This functions can affect various WL. See SearchWL for more details.
 *
 ******************************************************************************/

node *
StartSearchWL (node *idn, node *assignn, int mode)
{
    node *resultn, *mrdn;
    int varno, valid;

    DBUG_ENTER ("StartSearchWLRef");
    DBUG_ASSERT ((0 <= mode && 2 >= mode), ("wrong value given for mode"));

    valid = 1; /* 1 is important, which means 'foldable' */
    varno = ID_VARNO (idn);
    mrdn = (node *)ASSIGN_MRDMASK (assignn)[varno];
    search_wl_nodelist = NULL;
    resultn = SearchWL (varno, mrdn, &valid, mode, ASSIGN_LEVEL (assignn));
    search_wl_nodelist = NodeListFree (search_wl_nodelist, 0);

    DBUG_ASSERT (0 == valid || 1 == valid, ("invalid value for variable valid"));

    DBUG_RETURN (resultn);
}

/******************************************************************************
 *
 * function:
 *   node *CreateZeroVector( int dim, simpletype type)
 *
 * description:
 *   returns an N_array node with 'dim' components, each 0. If dim == 0,
 *   a scalar 0 is returned.
 *
 ******************************************************************************/

node *
CreateZeroVector (int dim, simpletype type)
{
    node *resultn, *tmpn;
    int i;
    shpseg *shpseg;

    DBUG_ENTER ("CreateZeroVector");

    if (dim == 0) {
        switch (type) {
        case T_int:
            resultn = MakeNum (0);
            break;
        case T_float:
            resultn = MakeFloat (0);
            break;
        case T_double:
            resultn = MakeDouble (0);
            break;
        case T_bool:
            resultn = MakeBool (0);
            break;
        case T_char:
            resultn = MakeChar ('\0');
            break;
        default:
            DBUG_ASSERT (0, ("unkown type"));
        }
    } else {
        tmpn = NULL;
        for (i = 0; i < dim; i++)
            switch (type) {
            case T_int:
                tmpn = MakeExprs (MakeNum (0), tmpn);
                break;
            case T_float:
                tmpn = MakeExprs (MakeFloat (0), tmpn);
                break;
            case T_double:
                tmpn = MakeExprs (MakeDouble (0), tmpn);
                break;
            case T_bool:
                tmpn = MakeExprs (MakeBool (0), tmpn);
                break;
            case T_char:
                tmpn = MakeExprs (MakeChar ('\0'), tmpn);
                break;
            default:
                DBUG_ASSERT (0, ("unkown type"));
            }

        resultn = MakeArray (tmpn);
        ARRAY_ISCONST (resultn) = TRUE;
        ARRAY_VECTYPE (resultn) = type;
        ARRAY_VECLEN (resultn) = dim;
        switch (type) {
        case T_int:
            ((int *)ARRAY_CONSTVEC (resultn)) = Array2IntVec (tmpn, NULL);
            break;
        case T_float:
            ((float *)ARRAY_CONSTVEC (resultn)) = Array2FloatVec (tmpn, NULL);
            break;
        case T_double:
            ((double *)ARRAY_CONSTVEC (resultn)) = Array2DblVec (tmpn, NULL);
            break;
        case T_bool:
            ((int *)ARRAY_CONSTVEC (resultn)) = Array2BoolVec (tmpn, NULL);
            break;
        case T_char:
            ((char *)ARRAY_CONSTVEC (resultn)) = Array2CharVec (tmpn, NULL);
            break;
        default:
            DBUG_ASSERT (0, ("unkown type"));
        }
        shpseg = MakeShpseg (
          MakeNums (dim, NULL)); /* nums struct is freed inside MakeShpseg. */
        ARRAY_TYPE (resultn) = MakeType (type, 1, shpseg, NULL, NULL);
    }

    DBUG_RETURN (resultn);
}

/******************************************************************************
 *
 * function:
 *   node *WithloopFolding( node *arg_node, int loop)
 *
 * description:
 *   starting point for the withloop folding.
 *
 *   'loop' specifies the number of the current optimization cycle and is
 *   needed for correct handling of break specifiers.
 *
 ******************************************************************************/

node *
WithloopFolding (node *arg_node, int loop)
{
    funtab *tmp_tab;
    node *arg_info;
    int expr;

    DBUG_ENTER ("WithloopFolding");

    arg_info = MakeInfo ();

    /* WLI traversal: create MRD
       This phase does NOTHING important but to create MRD lists which are needed
       for SearchWL in phase 2. SearchWLHelp does not work properly when
       the end of a compound node is searched and then (not existing) MRD
       masks are needed. When the new flatten exists we can remove this
       phase because SearchWLHelp will not need MRD masks. The DEF mask can be
       used to finde the wanted definition (which IS the last definition
       because of unique names. */

    DBUG_PRINT ("OPT", ("WLI 1"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));
    wli_phase = 1;
    tmp_tab = act_tab;
    act_tab = wli_tab;
    arg_node = Trav (arg_node, arg_info);

    /* WLI traversal: search information */
    DBUG_PRINT ("OPT", ("WLI 2"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));
    wli_phase = 2;
    act_tab = wli_tab;
    arg_node = Trav (arg_node, arg_info);

    /* break after WLI? */
    if ((break_after != PH_sacopt) || (break_cycle_specifier != loop)
        || strcmp (break_specifier, "wli")) {
        /* WLF traversal: fold WLs */
        DBUG_PRINT ("OPT", ("WLF"));
        DBUG_PRINT ("OPTMEM",
                    ("mem currently allocated: %d bytes", current_allocated_mem));
        act_tab = wlf_tab;
        arg_node = Trav (arg_node, arg_info);
        expr = (wlf_expr - old_wlf_expr);
        DBUG_PRINT ("OPT", ("                        result: %d", expr));
    }
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    act_tab = tmp_tab;
    arg_info = FreeTree (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WithloopFoldingWLT( node *arg_node)
 *
 * description:
 *   executes only WLT phase, not WLI and WLF.
 *
 ******************************************************************************/

node *
WithloopFoldingWLT (node *arg_node)
{
    funtab *tmp_tab;
    node *arg_info;
    int expr;

    DBUG_ENTER ("WithloopFoldingWLT");

    arg_info = MakeInfo ();

    /* WLT traversal: transform WLs */
    DBUG_PRINT ("OPT", ("WLT"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));
    tmp_tab = act_tab;
    act_tab = wlt_tab;
    arg_node = Trav (arg_node, arg_info);
    expr = (wlt_expr - old_wlt_expr);
    DBUG_PRINT ("OPT", ("                        result: %d", expr));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    arg_info = FreeTree (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}
