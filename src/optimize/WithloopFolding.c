/*      $Id$
 *
 * $Log$
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
 * Revision 1.4  1998/02/24 14:19:20  srs
 * *** empty log message ***
 *
 * Revision 1.3  1998/02/09 15:58:20  srs
 * *** empty log message ***
 *
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
#include "Inline.h"
#include "typecheck.h"
#include "ConstantFolding.h"
#include "WithloopFolding.h"
#include "WLT.h"
#include "WLI.h"
#include "WLF.h"

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

    if (!iinfo)
        printf ("\nNULL\n");
    else if (iinfo->vector) {
        printf ("\nVECTOR shape [%d]:\n", iinfo->vector);
        for (i = 0; i < iinfo->vector; i++) {
            printf ("---%d---\n", i);

            if (!iinfo->permutation[i]) { /* constant */
                printf ("  constant %d\n", iinfo->const_arg[i]);
                continue;
            }

            printf ("  base %d\n", iinfo->permutation[i]);
            tmpii = iinfo;
            while (tmpii) {
                sel = tmpii->vector ? i : 0;
                if (tmpii->arg_no) {
                    if (1 == tmpii->arg_no)
                        printf ("   %d%s. ", tmpii->const_arg[sel],
                                prf_string[tmpii->prf]);
                    else
                        printf ("   .%s%d ", prf_string[tmpii->prf],
                                tmpii->const_arg[sel]);
                } else
                    printf ("   no prf ");
                printf ("(p:%d, v:%d)\n", tmpii->permutation[sel], tmpii->vector);
                tmpii = tmpii->last[sel];
            }
        }
    } else {
        printf ("\nSCALAR:\n");
        printf ("  base %d\n", iinfo->permutation[0]);
        tmpii = iinfo;
        sel = 0;
        if (tmpii->arg_no) {
            if (1 == tmpii->arg_no)
                printf ("   %d%s. ", tmpii->const_arg[sel], prf_string[tmpii->prf]);
            else
                printf ("   %s%d. ", prf_string[tmpii->prf], tmpii->const_arg[sel]);
            printf ("(p:%d, v:%d)\n", tmpii->permutation[sel], tmpii->vector);
        }
    }

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
 *   The N_with node has to be available to find the index vars.
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
 *   intern_gen *AppendInternGen(...)
 *
 * description:
 *   this function creates an intern_gen struct and inserts it in a
 *   intern_gen chain after append_to (if not NULL).
 *   The new intern_gen struct is returned.
 *
 * parameters:
 *   append_to: see description
 *   shape: number of slots allocated for the arrays *l etc.
 *   stepwidth: if != 0 allocate memory for step array
 *
 ******************************************************************************/

intern_gen *
AppendInternGen (intern_gen *append_to, int shape, node *code, int stepwidth)
{
    intern_gen *ig;
    int i;

    DBUG_ENTER ("AppendInternGen");

    ig = Malloc (sizeof (intern_gen));
    ig->l = Malloc (sizeof (int) * shape);
    ig->u = Malloc (sizeof (int) * shape);
    if (stepwidth) {
        ig->step = Malloc (sizeof (int) * shape);
        ig->width = Malloc (sizeof (int) * shape);
        for (i = 0; i < shape; i++)
            ig->step[i] = ig->width[i] = 1;
    } else {
        ig->step = NULL;
        ig->width = NULL;
    }

    if (append_to) {
        ig->next = append_to->next;
        append_to->next = ig;
    } else
        ig->next = NULL;

    ig->code = code;
    ig->shape = shape;

    DBUG_RETURN (ig);
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

            is_1 = is_1 && 1 == ig->step[1];
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
    DBUG_ASSERT (!arrayn || N_array == NODE_TYPE (arrayn), ("Wrong arrayn"));

    if (arrayn)
        arrayn = ARRAY_AELEMS (arrayn);
    if (!*iarray)
        *iarray = Malloc (shape * sizeof (int));

    for (i = 0; i < shape; i++)
        if (arrayn) {
            DBUG_ASSERT (N_num == NODE_TYPE (EXPRS_EXPR (arrayn)),
                         ("not a constant array"));
            (*iarray)[i] = NUM_VAL (EXPRS_EXPR (arrayn));
            arrayn = EXPRS_NEXT (arrayn);
        } else
            (*iarray)[i] = 0;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   intern_gen *Tree2InternGen(node *wln)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

intern_gen *
Tree2InternGen (node *wln)
{
    intern_gen *root, *tmp_ig;
    node *partn, *genn;
    int shape;

    DBUG_ENTER ("Tree2InternGen");

    partn = NWITH_PART (wln);
    root = NULL;
    tmp_ig = NULL;

    while (partn) {
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
CreateArrayFromInternGen (int *source, int number)
{
    node *arrayn, *tmpn;
    DBUG_ENTER (" *CreateArrayFromInternGen");

    tmpn = NULL;
    while (number)
        tmpn = MakeExprs (MakeNum (source[--number]), tmpn);
    arrayn = MakeArray (tmpn);

    DBUG_RETURN (arrayn);
}

/******************************************************************************
 *
 * function:
 *   node *InternGen2Tree(node *arg_node, intern_gen *ig)
 *
 * description:
 *   copies intern_gen struct to the generators of the given WL. All existing
 *   N_Npart nodes are deleted before. Returns wln.
 *
 ******************************************************************************/

node *
InternGen2Tree (node *wln, intern_gen *ig)
{
    node **part, *withidn, *genn, *b1n, *b2n, *stepn, *widthn;

    DBUG_ENTER ("InternGen2Tree");

    withidn = DupTree (NPART_WITHID (NWITH_PART (wln)), NULL);
    FreeTree (NWITH_PART (wln));
    part = &(NWITH_PART (wln));

    while (ig) {
        /* create generator components */
        b1n = CreateArrayFromInternGen (ig->l, ig->shape);
        b2n = CreateArrayFromInternGen (ig->u, ig->shape);
        stepn = ig->step ? CreateArrayFromInternGen (ig->step, ig->shape) : NULL;
        widthn = ig->width ? CreateArrayFromInternGen (ig->width, ig->shape) : NULL;

        /* create tree structures */
        genn = MakeNGenerator (b1n, b2n, F_le, F_lt, stepn, widthn);
        *part = MakeNPart (DupTree (withidn, NULL), genn, ig->code);
        ig = ig->next;
        part = &(NPART_NEXT ((*part)));
    }

    FREE (withidn);

    DBUG_RETURN (wln);
}

/******************************************************************************
 *
 * function:
 *   node *CreateVardec(char *name, types *type, node **vardecs)
 *
 * description:
 *   creates a new Vardec with 'name' of type 'type' at the beginning of
 *   the 'vardecs' chain. The node of the new Vardec is returned.
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

    DBUG_ENTER (" CreateVardec");

    /* search for already existing vardec for this name. */
    vardecn = SearchDecl (name, *vardecs); /* from Inline.c */

    /* if not found, create vardec. */
    if (!vardecn) {
        type = DuplicateTypes (type, 42);
        vardecn = MakeVardec (StringCopy (name), type, *vardecs);
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
 *
 *
 ******************************************************************************/

void
SearchWLHelp (int id_varno, node *assignn)
{
    int valid;

    valid = -1;

    while (ASSIGN_NEXT (assignn))
        assignn = ASSIGN_NEXT (assignn);

    /* if Id is defined in this last instr of the body*/
    if (ASSIGN_DEFMASK (assignn)[id_varno]) {
        if (N_let == ASSIGN_INSTRTYPE (assignn)
            && N_Nwith == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assignn))))
            /* this is the bad boy */
            NWITH_NO_CHANCE (LET_EXPR (ASSIGN_INSTR (assignn))) = 1;
        else /* if not, this may be a compound structure. */
            SearchWL (id_varno, assignn, &valid);
    } else /* else it is defined somewhere above and we can use the ASSIGN_MRDMASK */
        SearchWL (id_varno, (node *)ASSIGN_MRDMASK (assignn)[id_varno], &valid);
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
 *     SearchWL(varno, (node*)ASSIGN_MRDMASK(assignn)[varno], &valid)     ).
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
 *   this functions modifies the found WL in some cases. See remarks.
 *   make sure that *valid is not -1 if called. This is an internal flag
 *   used in conjunction with SearchWLHelp.
 *
 * remarks:
 *   The assign node is only returned if *valid is 1. Else we would have a
 *   problem returning multiple nodes if a WL is defined inside of both
 *   trees of a conditional, which could be recursive.
 *   If this happens (*valid==0) we still continue to find the WL to
 *   mark it as unfoldable. NWITH_NO_CHANCE is set to 1.
 *
 ******************************************************************************/

node *
SearchWL (int id_varno, node *startn, int *valid)
{
    node *tmpn;

    DBUG_ENTER ("SearchWL");

    if (-1 != *valid)
        *valid = 1;

    /* MRDs point at N_assign or at N_Npart nodes. If N_Npart, the Id is
       an index vector, not a WL. */
    if (startn && N_Npart == NODE_TYPE (startn))
        startn = NULL;

    if (startn) { /* N_assign node. */
        switch (ASSIGN_INSTRTYPE (startn)) {
        case N_while:
        case N_do:
            /* if the Id is referenced in N_while, N_do or N_cond we can
               set *valid to 0. */
            *valid = 0;

            /* we have to find out if idn referenced a WL. Therefore
               we traverse the body to its last assignment and call SearchWL
               again. This happens in SearchWLHelp. */
            if (N_while == ASSIGN_INSTRTYPE (startn))
                startn = BLOCK_INSTR (WHILE_BODY (ASSIGN_INSTR (startn)));
            else
                startn = BLOCK_INSTR (DO_BODY (ASSIGN_INSTR (startn)));
            SearchWLHelp (id_varno, startn);
            break;

        case N_cond:
            *valid = 0;
            if ((node *)COND_THENDEFMASK (ASSIGN_INSTR (startn))[id_varno]) {
                tmpn = COND_THENINSTR (ASSIGN_INSTR (startn)); /* then-part */
                SearchWLHelp (id_varno, tmpn);
            }
            if ((node *)COND_ELSEDEFMASK (ASSIGN_INSTR (startn))[id_varno]) {
                tmpn = COND_ELSEINSTR (ASSIGN_INSTR (startn)); /* else-part */
                SearchWLHelp (id_varno, tmpn);
            }
            break;

        case N_let:
            if (N_Nwith == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (startn)))) {
                if (-1 == *valid) { /* inside a compound node */
                    NWITH_NO_CHANCE (LET_EXPR (ASSIGN_INSTR (startn))) = 1;
                    *valid = 0;
                } /* else *valid is 1 and the assign node of the WL is in startn. */
            } else
                startn = NULL; /* this is not a WL. */
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
 *   node *WithloopFolding(node *arg_node, node* arg_info)
 *
 * description:
 *   starting point for the withloop folding.
 *
 *
 ******************************************************************************/

node *
WithloopFolding (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFWithloopFolding");

    DBUG_ASSERT (!arg_info, ("at the beginning of WLF: arg_info != NULL"));
    arg_info = MakeInfo ();

    /* WLT traversal: transform WLs */
    DBUG_EXECUTE ("WLx", NOTE_OPTIMIZER_PHASE ("  WLT"););
    act_tab = wlt_tab;
    arg_node = Trav (arg_node, arg_info);

    /* rebuild mask which is necessary because of the transformations in WLT. */
    DBUG_EXECUTE ("WLx", NOTE_OPTIMIZER_PHASE ("  GenerateMasks"););
    arg_node = GenerateMasks (arg_node, NULL);

    /* WLI traversal: search information */
    DBUG_EXECUTE ("WLx", NOTE_OPTIMIZER_PHASE ("  WLI"););
    act_tab = wli_tab;
    arg_node = Trav (arg_node, arg_info);

    /* WLF traversal: fold WLs */
    DBUG_EXECUTE ("WLx", NOTE_OPTIMIZER_PHASE ("  WLF"););
    act_tab = wlf_tab;
    arg_node = Trav (arg_node, arg_info);

    /* rebuild mask which is necessary because of WL-body-substitutions and
       inserted new variables to prevent wrong variable bindings. */
    DBUG_EXECUTE ("WLx", NOTE_OPTIMIZER_PHASE ("  GenerateMasks"););
    arg_node = GenerateMasks (arg_node, NULL);

    FREE (arg_info);

    DBUG_RETURN (arg_node);
}
