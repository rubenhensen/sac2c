/*
 * $Log$
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

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
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
#include "CheckAvis.h"
#include "SSATransform.h"
#include "SSAWithloopFolding.h"
#include "SSAWLT.h"
#include "SSAWLI.h"

/******************************************************************************
 *
 * function:
 *   index_info *SSACreateIndex(int vector)
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
SSACreateIndex (int vector)
{
    index_info *pindex;
    DBUG_ENTER ("SSACreateInfoInfo");

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
 *   index_info *SSADuplicateIndexInfo(index_info *iinfo)
 *
 * description:
 *   duplicates struct
 *
 ******************************************************************************/
index_info *
SSADuplicateIndexInfo (index_info *iinfo)
{
    index_info *new;
    int i, to;
    DBUG_ENTER ("SSADuplicateIndexInfo");
    DBUG_ASSERT (iinfo, ("parameter NULL"));

    new = SSACreateIndex (iinfo->vector);

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
 *   node *SSAValidLocalId(node *idn)
 *
 * description:
 *   returns pointer to index_info if Id (idn) is a valid variable within
 *   the WL body (index vars are excluded). Returns NULL otherwise.
 *
 ******************************************************************************/
index_info *
SSAValidLocalId (node *idn)
{
    index_info *iinfo;

    DBUG_ENTER ("SSAValidLocalId");
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
 *   int SSALocateIndexVar(node *idn, node* wln)
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
SSALocateIndexVar (node *idn, node *wln)
{
    ids *_ids;
    int result = 0, i;

    DBUG_ENTER ("SSALocateIndexVar");
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
 *   intern_gen *SSACreateInternGen(int shape, int stepwidth)
 *
 * description:
 *   allocate memory for an intern_gen struct. The parameter shape is needed
 *   to allocate right sized mem for the bounds. If stepwidth is not 0,
 *   memory for step/width is allocated, too.
 *
 ******************************************************************************/
intern_gen *
SSACreateInternGen (int shape, int stepwidth)
{
    intern_gen *ig;

    DBUG_ENTER ("SSACreateInternGen");

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
 *   intern_gen *SSAAppendInternGen(...)
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
SSAAppendInternGen (intern_gen *append_to, int shape, node *code, int stepwidth)
{
    intern_gen *ig;
    int i;

    DBUG_ENTER ("SSAAppendInternGen");

    ig = SSACreateInternGen (shape, stepwidth);

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
 *   intern_gen *SSACopyInternGen(intern_gen *source)
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
SSACopyInternGen (intern_gen *source)
{
    intern_gen *ig;
    int i;

    DBUG_ENTER ("SSACopyInternGen");

    ig = SSACreateInternGen (source->shape, NULL != source->step);
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
 *   int SSANormalizeInternGen(intern_gen *ig)
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
SSANormalizeInternGen (intern_gen *ig)
{
    int error = 0, i = 0, is_1 = 1;

    DBUG_ENTER ("SSANormalizeInternGen");

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
            ig->step = Free (ig->step);
            ig->width = Free (ig->width);
        }
    }

    DBUG_RETURN (error);
}

/******************************************************************************
 *
 * function:
 *   void SSAArrayST2ArrayInt(node *arrayn, int **iarray, int shape)
 *
 * description:
 *   copies 'shape' numbers of given constant array node (1 dimension) to
 *   *iarray.
 *   If *iarray = NULL, memory is allocated.
 *   If arrayn is NULL, iarray is filles with 0.
 *
 ******************************************************************************/
void
SSAArrayST2ArrayInt (node *arrayn, int **iarray, int shape)
{
    int i;

    DBUG_ENTER ("SSAArrayST2ArrayInt");

    if (!*iarray) {
        *iarray = Malloc (shape * sizeof (int));
    }

    if (arrayn == NULL) {
        for (i = 0; i < shape; i++) {
            (*iarray)[i] = 0;
        }
    } else if (NODE_TYPE (arrayn) == N_array) {
        for (i = 0; i < shape; i++) {
            (*iarray)[i] = ((int *)ARRAY_CONSTVEC (arrayn))[i];
        }
    } else /* (NODE_TYPE(arrayn) == N_id) */ {
        DBUG_ASSERT ((NODE_TYPE (arrayn) == N_id), ("Wrong arrayn"));

        for (i = 0; i < shape; i++) {
            (*iarray)[i] = ((int *)ID_CONSTVEC (arrayn))[i];
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   intern_gen *SSATree2InternGen(node *wln)
 *
 * description:
 *   copies generators of given WL to intern_gen struct. If filter is not
 *   NULL, only generators pointing at the N_Ncode node filter are copied.
 *
 ******************************************************************************/
intern_gen *
SSATree2InternGen (node *wln, node *filter)
{
    intern_gen *root, *tmp_ig;
    node *partn, *genn;
    int shape;

    DBUG_ENTER ("SSATree2InternGen");

    partn = NWITH_PART (wln);
    root = NULL;
    tmp_ig = NULL;

    while (partn) {
        if (!filter || NPART_CODE (partn) == filter) {
            genn = NPART_GEN (partn);
            shape = IDS_SHAPE (NPART_VEC (partn), 0);
            tmp_ig = SSAAppendInternGen (tmp_ig, shape, NPART_CODE (partn),
                                         (NGEN_STEP (genn) != NULL)
                                           || (NGEN_WIDTH (genn) != NULL));
            if (!root)
                root = tmp_ig;

            /* copy vector information to intern_gen */
            SSAArrayST2ArrayInt (NGEN_BOUND1 (genn), &tmp_ig->l, shape); /* l */
            SSAArrayST2ArrayInt (NGEN_BOUND2 (genn), &tmp_ig->u, shape); /* u */
            if (NGEN_STEP (genn))
                SSAArrayST2ArrayInt (NGEN_STEP (genn), &tmp_ig->step, shape); /* step */
            if (NGEN_WIDTH (genn))
                SSAArrayST2ArrayInt (NGEN_WIDTH (genn), &tmp_ig->width,
                                     shape); /* width */

            /* normalize step and width */
            switch (SSANormalizeInternGen (tmp_ig)) {
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
 *   node *SSACreateArrayFromInternGen(int *source, int number)
 *
 * description:
 *   copies 'number' elements of the array source to an N_array struct and
 *   returns it.
 *
 ******************************************************************************/
static node *
SSACreateArrayFromInternGen (int *source, int number, types *type)
{
    node *arrayn, *tmpn;
    int i;

    DBUG_ENTER ("SSACreateArrayFromInternGen");

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
 *   node *SSAInternGen2Tree(node *arg_node, intern_gen *ig)
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
SSAInternGen2Tree (node *wln, intern_gen *ig)
{
    node **part, *withidn, *genn, *b1n, *b2n, *stepn, *widthn;
    types *type;
    shpseg *shpseg;
    int no_parts; /* number of N_Npart nodes */

    DBUG_ENTER ("SSAInternGen2Tree");

    withidn = DupTree (NPART_WITHID (NWITH_PART (wln)));
    FreeTree (NWITH_PART (wln));
    part = &(NWITH_PART (wln));
    no_parts = 0;

    /* create type for N_array nodes*/
    shpseg = MakeShpseg (MakeNums (ig->shape, NULL));
    /* nums struct is freed inside MakeShpseg. */

    type = MakeTypes (T_int, 1, shpseg, NULL, NULL);

    while (ig) {
        /* create generator components */
        b1n = SSACreateArrayFromInternGen (ig->l, ig->shape, type);
        b2n = SSACreateArrayFromInternGen (ig->u, ig->shape, type);
        stepn = ig->step ? SSACreateArrayFromInternGen (ig->step, ig->shape, type) : NULL;
        widthn
          = ig->width ? SSACreateArrayFromInternGen (ig->width, ig->shape, type) : NULL;

        /* create tree structures */
        genn = MakeNGenerator (b1n, b2n, F_le, F_lt, stepn, widthn);
        *part = MakeNPart (DupTree (withidn), genn, ig->code);

        ig = ig->next;
        part = &(NPART_NEXT ((*part)));
        no_parts++;
    }

    NWITH_PARTS (wln) = no_parts;

    /* should this be a FreeTree() ?? ?!! */
    FreeTree (withidn);
    FreeOneTypes (type);

    DBUG_RETURN (wln);
}

/******************************************************************************
 *
 * function:
 *   void SSAFreeInternGenChain(intern_gen *ig)
 *
 * description:
 *   Frees all memory allocated by ig and returns NULL (ig is NOT set to NULL).
 *
 ******************************************************************************/

intern_gen *
SSAFreeInternGen (intern_gen *tmp)
{
    DBUG_ENTER ("SSAFreeInternGen");

    DBUG_PRINT ("FREE", ("Removing intern gen (WLF)"));

    DBUG_ASSERT ((tmp != NULL), "cannot free a NULL intern gen (WLF)!");

    tmp->l = Free (tmp->l);
    tmp->u = Free (tmp->u);
    tmp->step = Free (tmp->step);
    tmp->width = Free (tmp->width);

    tmp = Free (tmp);

    DBUG_RETURN (tmp);
}

/******************************************************************************
 *
 * function:
 *   void SSAFreeInternGenChain(intern_gen *ig)
 *
 * description:
 *   Frees all memory allocated by ig and returns NULL (ig is NOT set to NULL).
 *
 ******************************************************************************/

intern_gen *
SSAFreeInternGenChain (intern_gen *ig)
{
    intern_gen *tmpig;

    DBUG_ENTER ("SSAFreeInternGenChain");

    while (ig) {
        tmpig = ig;
        ig = ig->next;
        SSAFreeInternGen (tmpig);
    }

    DBUG_RETURN (ig);
}

/******************************************************************************
 *
 * function:
 *   node *SSACreateVardec(char *name, types *type, node **vardecs)
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
 *   does not preserve ssa form ##nmw## because of missing ssacount attribute
 *
 ******************************************************************************/
node *
SSACreateVardec (char *name, types *type, node **vardecs)
{
    node *vardecn;
    char *c;

    DBUG_ENTER ("SSACreateVardec");

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

        /* create ssacnt node: to be implemented */

        *vardecs = vardecn;
    }

    DBUG_RETURN (vardecn);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWithloopFolding( node *arg_node, int loop)
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
 *   SSATransform.
 *
 ******************************************************************************/
node *
SSAWithloopFolding (node *arg_node, int loop)
{
    funtab *tmp_tab;
    node *arg_info;
    int expr;

    DBUG_ENTER ("SSAWithloopFolding");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "SSAWithloopFolding called for non fundef node");

    if (!(FUNDEF_IS_LACFUN (arg_node))) {
        arg_info = MakeInfo ();

        DBUG_PRINT ("OPT", ("SSAWLI"));
        DBUG_PRINT ("OPTMEM",
                    ("mem currently allocated: %d bytes", current_allocated_mem));
        tmp_tab = act_tab;

        /* SSAWLI traversal */
        act_tab = ssawli_tab;
        arg_node = Trav (arg_node, arg_info);

        /* break after WLI? */
        if ((break_after != PH_sacopt) || (break_cycle_specifier != loop)
            || strcmp (break_specifier, "wli")) {

            /* SSAWLF traversal: fold WLs */
            DBUG_PRINT ("OPT", ("SSAWLF"));
            DBUG_PRINT ("OPTMEM",
                        ("mem currently allocated: %d bytes", current_allocated_mem));
            act_tab = ssawlf_tab;
            arg_node = Trav (arg_node, arg_info);

            expr = (wlf_expr - old_wlf_expr);
            DBUG_PRINT ("OPT", ("                        result: %d", expr));
        }
        DBUG_PRINT ("OPTMEM",
                    ("mem currently allocated: %d bytes", current_allocated_mem));

        act_tab = tmp_tab;
        arg_info = FreeTree (arg_info);

        /* restore ssa form */
        arg_node = CheckAvisOneFunction (arg_node);
        arg_node = SSATransformOneFunction (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWithloopFoldingWLT( node *arg_node)
 *
 * description:
 *   executes only SSAWLT phase, not SSAWLI and SSAWLF.
 *   after withloop transformation the ssaform is restored by calling
 *   CheckAvis and SSATransform.
 *
 ******************************************************************************/
node *
SSAWithloopFoldingWLT (node *arg_node)
{
    funtab *tmp_tab;
    node *arg_info;
    int expr;

    DBUG_ENTER ("SSAWithloopFoldingWLT");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "SSAWithloopFoldingWLT not called for fundef node");

    if (!(FUNDEF_IS_LACFUN (arg_node))) {
        /* start ssawlt traversal only in non-special fundefs */
        arg_info = MakeInfo ();

        /* SSAWLT traversal: transform WLs */
        DBUG_PRINT ("OPT", ("SSAWLT"));
        DBUG_PRINT ("OPTMEM",
                    ("mem currently allocated: %d bytes", current_allocated_mem));
        tmp_tab = act_tab;
        act_tab = ssawlt_tab;
        arg_node = Trav (arg_node, arg_info);
        expr = (wlt_expr - old_wlt_expr);
        DBUG_PRINT ("OPT", ("                        result: %d", expr));
        DBUG_PRINT ("OPTMEM",
                    ("mem currently allocated: %d bytes", current_allocated_mem));

        arg_info = FreeTree (arg_info);
        act_tab = tmp_tab;

        /* restore ssa form */
        arg_node = CheckAvisOneFunction (arg_node);
        arg_node = SSATransformOneFunction (arg_node);
    }

    DBUG_RETURN (arg_node);
}
