/*
 *
 * $Log$
 * Revision 1.20  2004/09/30 10:56:25  khf
 * generate a scalar WL instead of a vector of zeros
 * to fill a genarray WL
 *
 * Revision 1.19  2004/09/28 11:37:47  khf
 * changed to new types
 *
 * Revision 1.18  2004/09/27 14:28:30  khf
 * CreateFullPartition() can now handle AUD modarray WLs
 *
 * Revision 1.17  2004/09/21 16:33:52  khf
 * NewIds() accepts N_num also, adapted creation of ntyes,
 * changed prf F_shape_sel to F_idx_sel
 *
 * Revision 1.16  2004/08/25 15:45:34  khf
 * Bug 44: faulty wlgenerator property for empty bounds resolved.
 * In ComputeGeneratorProperties only check for emptiness of
 * generator with non empty bounds
 *
 * Revision 1.15  2004/08/10 16:07:03  khf
 * CreateStructConstants(): expr can be NULL
 * CreateEmptyGenWLReplacement(): assign of freed LET_EXPR corrected
 *
 * Revision 1.14  2004/08/09 13:12:31  khf
 * some comments added
 *
 * Revision 1.13  2004/08/06 16:09:31  khf
 * CompleteGrid: determination of steps corrected
 *
 * Revision 1.12  2004/08/04 12:39:05  khf
 * by appliance of Constant Folding: the result of TRAV must not
 * be stored in MODUL_FUNS
 *
 * Revision 1.11  2004/08/03 11:15:02  khf
 * corrected size of array_shape
 *
 * Revision 1.10  2004/08/03 09:05:36  khf
 * some code brushing done
 * call of MakeNCode adjusted, corrected type of new struct constants
 * and identifiers
 * corrected inference of generator properties
 *
 * Revision 1.9  2004/07/22 17:26:23  khf
 * Special functions are now traversed when they are used
 *
 * Revision 1.8  2004/07/21 16:05:00  khf
 * no traverse in FUNDEF_NEXT in phase sacopt assured
 *
 * Revision 1.7  2004/07/21 12:48:33  khf
 * switch to new INFO structure
 * take changes of sbs in SSAWLT.c over
 *
 * Revision 1.6  2004/06/30 12:20:07  khf
 * WLPGfundef(): application of Constant Folding added
 * (PH_wlpartgen)
 *
 * Revision 1.5  2004/05/28 16:30:10  sbs
 * *** empty log message ***
 *
 * Revision 1.4  2004/04/08 08:13:25  khf
 * some corrections and new startfunction WLPartitionGenerationOPT
 * added
 *
 * Revision 1.3  2004/03/02 09:21:43  khf
 * some corrections and WLPGlet added
 *
 * Revision 1.2  2004/02/26 13:11:01  khf
 * WLPartitionGeneration implemented in parts (but not tested)
 *
 * Revision 1.1  2004/02/25 13:16:58  khf
 * Initial revision
 *
 *
 */

/**
 *
 * @file WLPartition Generation.c
 *
 * In this traversal AKS and AKD genarray/modarray withloops
 * with non-full partitions of the considered array are
 * transformed into withloops with full partitions.
 *
 * Ex. of AKS withloop:
 *    A = with(iv)
 *         ([3] < iv <= [6])
 *         {res = ...;
 *         }: res
 *        genarray([9]);
 *
 * is transformed into
 *
 *    A = with(iv)
 *         ([3] < iv <= [6])
 *         {res = ...;
 *         }: res
 *         ([0] < iv <= [3])
 *         {res = 0;
 *         }: res
 *         ([7] < iv <= [9])
 *         {res = 0;
 *         }: res
 *        genarray([9]);
 *
 * The modification of AKS withloops was already enforced by
 * WLT a subphase of WithloopFolding but is here extended for AKDs.
 *
 * Ex. of AKD withloop:
 *   int[1] a;
 *   int[1] b;
 *   int[1] c;
 *    B = with(iv)
 *         (a < iv <= b)
 *         {res = ...;
 *         }: res
 *        genarray([c]);
 *
 * is transformed into
 *
 *    A = with(iv)
 *         ([a[0]] < iv <= [b[0]])
 *         {res = ...;
 *         }: res
 *         ([0] < iv <= [a[0]])
 *         {res = 0;
 *         }: res
 *         ([b[0]] < iv <= [c[0]])
 *         {res = 0;
 *         }: res
 *        genarray([c[0]]);
 *
 */

#define NEW_INFO

#include <stdio.h>
#include <stdlib.h>

#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "Error.h"
#include "globals.h"
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "SSAConstantFolding.h"
#include "ssa.h"
#include "WLPartitionGeneration.h"

/**
 * INFO structure
 */
struct INFO {
    node *wl;
    node *fundef;
    node *let;
    node *nassign;
    node *modul;
    int genprob;
    int genshp;
    int subphase;
};

/*******************************************************************************
 *  Usage of arg_info:
 *  - node : WL      : reference to base node of current WL (N_Nwith)
 *  - node : FUNDEF  : pointer to last fundef node. needed to access vardecs.
 *  - node : LET     : pointer to N_let node of current WL.
 *                     LET_EXPR(ID) == INFO_WLPG_WL.
 *  - node : NASSIGNS: pointer to a list of new assigns, which where needed
 *                     to build structural constants and had to be inserted
 *                     in front of the considered with-loop.
 *  - int  : GENPROB : pointer to the status of the considered generator
 *                     concerning coverage of the entire range
 *  - int  : GENSHP  : pointer to the status of the bounds, step and width
 *                     of the considered generator concerning shape
 *
 ******************************************************************************/
#define INFO_WLPG_WL(n) (n->wl)
#define INFO_WLPG_FUNDEF(n) (n->fundef)
#define INFO_WLPG_LET(n) (n->let)
#define INFO_WLPG_NASSIGNS(n) (n->nassign)
#define INFO_WLPG_MODUL(n) (n->modul)
#define INFO_WLPG_GENPROP(n) (n->genprob)
#define INFO_WLPG_GENSHP(n) (n->genshp)
#define INFO_WLPG_SUBPHASE(n) (n->subphase)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_WLPG_WL (result) = NULL;
    INFO_WLPG_FUNDEF (result) = NULL;
    INFO_WLPG_LET (result) = NULL;
    INFO_WLPG_NASSIGNS (result) = NULL;
    INFO_WLPG_MODUL (result) = NULL;
    INFO_WLPG_GENPROP (result) = 0;
    INFO_WLPG_GENSHP (result) = 0;
    INFO_WLPG_SUBPHASE (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

typedef enum { GPT_empty, GPT_full, GPT_partial, GPT_unknown } gen_prop_t;

typedef enum {
    GV_constant,
    GV_struct_constant,
    GV_known_shape,
    GV_unknown_shape
} gen_shape_t;

typedef enum { SP_cf, SP_mod, SP_func } sub_phase_t;

#ifndef DBUG_OFF
static char *gen_prop_str[] = {"GPT_empty", "GPT_full", "GPT_partial", "GPT_unknown"};
#endif

/** <!--********************************************************************-->
 *
 * @fn node *CreateStructConstant( node *expr, node *nassigns)
 *
 *   @brief A structural constant is freshly generated by means of (nassigns)
 *          and (*expr) is modified to point to the new node.
 *          The old one is freed!
 *
 *   @param  node *expr     :  expr
 *           node *nassigns :  a chained list of N_assign nodes
 *   @return node *         :  updated expr
 ******************************************************************************/
static node *
CreateStructConstant (node *expr, node *nassigns)
{
    node *tmp1, *tmp2, *idn, *iterator;
    shpseg *nshpseg;
    int dim = 0;

    DBUG_ENTER ("CreateStructConstant");

    DBUG_ASSERT ((nassigns != NULL), "NASSIGNS is empty");

    tmp1 = NULL;
    tmp2 = NULL;
    iterator = nassigns;

    while (iterator) {
        idn = DupIds_Id (LET_IDS (ASSIGN_INSTR (iterator)));

        if (tmp1) {
            EXPRS_NEXT (tmp2) = MakeExprs (idn, NULL);
            tmp2 = EXPRS_NEXT (tmp2);
        } else {
            tmp1 = MakeExprs (idn, tmp1);
            tmp2 = tmp1;
        }

        iterator = ASSIGN_NEXT (iterator);
        dim++;
    }

    tmp1 = MakeFlatArray (tmp1);
    nshpseg = MakeShpseg (MakeNums (dim, NULL));
    ARRAY_TYPE (tmp1) = MakeTypes (T_int, 1, nshpseg, NULL, NULL);
    ARRAY_NTYPE (tmp1) = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (dim));
    if (expr != NULL) {
        expr = FreeTree (expr);
    }
    expr = tmp1;

    DBUG_RETURN (expr);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateIdxShapeSelAssigns( node *array, int begin, int end,
 *                                     node *fundef)
 *
 *   @brief expects (array) to point to an identifier and generates as many
 *          new assigns as ((end+1)-begin) indicates.
 *
 *   @param  node *array   :  N_id
 *           int begin     :
 *           int end       :
 *           node *fundef  :  N_fundef
 *   @return node *        :  a chained list of N_assign nodes
 ******************************************************************************/
static node *
CreateIdxShapeSelAssigns (node *array, int begin, int end, node *fundef)
{
    node *nassigns, *vardec, *tmp1, *tmp2;
    ids *_ids;
    char *nvarname;
    int i;

    DBUG_ENTER ("CreateIdxShapeSelAssigns");

    DBUG_ASSERT ((array != NULL), "array is empty");
    DBUG_ASSERT ((NODE_TYPE (array) == N_id),
                 "CreateIdxShapeSelAssigns not called with N_id");
    DBUG_ASSERT ((end >= begin), "illegal length found!");

    nassigns = NULL;

    for (i = end; i >= begin; i--) {
        nvarname = TmpVarName (ID_NAME (array));
        _ids = MakeIds (nvarname, NULL, ST_regular);
        vardec = MakeVardec (StringCopy (nvarname), MakeTypes1 (T_int), NULL);

        AVIS_TYPE (VARDEC_AVIS (vardec))
          = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (0));

        IDS_VARDEC (_ids) = vardec;
        IDS_AVIS (_ids) = VARDEC_AVIS (vardec);

        fundef = AddVardecs (fundef, vardec);

        /* index position for selection */
        tmp1 = MakeNum (i);
        /* the array for selection */
        tmp2 = MakeExprs (DupTree (array), NULL);
        tmp1 = MakePrf (F_idx_shape_sel, MakeExprs (tmp1, tmp2));

        nassigns = MakeAssign (MakeLet (tmp1, _ids), nassigns);

        /* set correct backref to defining assignment */
        AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassigns;
    }

    DBUG_RETURN (nassigns);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateNewAssigns( node *expr, node *fundef)
 *
 *   @brief expects (expr) to point to an identifier and generates as many
 *          new assigns as the value of the shapevector at position 0 of (expr)
 *          indicates.
 *
 *   @param  node *expr   :  expr
 *           node *fundef :  N_fundef
 *   @return node *       :  a chained list of N_assign nodes
 ******************************************************************************/
static node *
CreateNewAssigns (node *expr, node *fundef)
{
    int i;
    node *tmp1, *tmp2, *tmp3, *nassigns, *vardec;
    ids *_ids;
    char *nvarname;
    types *type;

    DBUG_ENTER ("CreateNewAssigns");

    DBUG_ASSERT ((expr != NULL), "Expr is empty");
    DBUG_ASSERT ((NODE_TYPE (expr) == N_id), "CreateNewAssigns not called with N_id");
    DBUG_ASSERT ((ID_DIM (expr) == 1), "Dimension of Id is not 1");
    DBUG_ASSERT ((ID_SHPSEG (expr) != NULL), "SHPSEG of Id points to NULL");

    nassigns = NULL;

    for (i = ID_SHAPE (expr, 0) - 1; i >= 0; i--) {
        nvarname = TmpVarName (ID_NAME (expr));
        _ids = MakeIds (nvarname, NULL, ST_regular);
        type = MakeTypes1 (T_int);
        vardec = MakeVardec (StringCopy (nvarname), type, NULL);

        AVIS_TYPE (VARDEC_AVIS (vardec))
          = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (0));
        IDS_VARDEC (_ids) = vardec;
        IDS_AVIS (_ids) = VARDEC_AVIS (vardec);

        fundef = AddVardecs (fundef, vardec);

        /* index position for selection */
        tmp1 = MakeNum (i);
        /* the array for selection */
        tmp2 = MakeExprs (DupNode (expr), NULL);

        tmp3 = MakePrf (F_idx_sel, MakeExprs (tmp1, tmp2));

        nassigns = MakeAssign (MakeLet (tmp3, _ids), nassigns);

        /* set correct backref to defining assignment */
        AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassigns;
    }

    DBUG_RETURN (nassigns);
}

/** <!--********************************************************************-->
 *
 * @fn ids *NewIds( node *nd, node *fundef)
 *
 *   @brief creates new IDS.
 *
 *   @param  node *nd       :  if N_id node the new name and type will be
 *                             infered from
 *           node *fundef   :  N_fundef
 *   @return ids *          :  new Ids
 ******************************************************************************/
static ids *
NewIds (node *nd, node *fundef)
{
    node *vardec;
    ids *_ids;
    char *nvarname;

    DBUG_ENTER ("NewIds");

    DBUG_ASSERT ((nd != NULL) && ((NODE_TYPE (nd) == N_id) || (NODE_TYPE (nd) == N_num)),
                 "ID is empty or not N_id/N_num");

    if (NODE_TYPE (nd) == N_id) {
        nvarname = TmpVarName (ID_NAME (nd));
        _ids = MakeIds (nvarname, NULL, ST_regular);
        vardec = MakeVardec (StringCopy (nvarname), DupOneTypes (ID_TYPE (nd)), NULL);

        AVIS_TYPE (VARDEC_AVIS (vardec)) = TYCopyType (AVIS_TYPE (ID_AVIS (nd)));
    } else {
        nvarname = TmpVar ();
        _ids = MakeIds (nvarname, NULL, ST_regular);
        vardec = MakeVardec (StringCopy (nvarname), MakeTypes1 (T_int), NULL);

        AVIS_TYPE (VARDEC_AVIS (vardec))
          = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (0));
    }

    IDS_VARDEC (_ids) = vardec;
    IDS_AVIS (_ids) = VARDEC_AVIS (vardec);

    fundef = AddVardecs (fundef, vardec);

    DBUG_RETURN (_ids);
}

/** <!--********************************************************************-->
 *
 * @fn node CreateEntryFlatArray(int entry, int number)
 *
 *   @brief creates an flat array with 'number' elements consisting of
 *          'entry'
 *
 *   @param  int entry   : entry within the new array
 *           int number  : number of elements in the new array
 *   @return node *      : N_array
 ******************************************************************************/
static node *
CreateEntryFlatArray (int entry, int number)
{
    node *tmp;
    int i;

    DBUG_ENTER ("CreateOneArray");

    DBUG_ASSERT ((number > 0), "dim is <= 0");

    tmp = NULL;
    for (i = 0; i < number; i++) {
        tmp = MakeExprs (MakeNum (entry), tmp);
    }
    tmp = MakeFlatArray (tmp);

    ARRAY_TYPE (tmp)
      = MakeTypes (T_int, 1, MakeShpseg (MakeNums (number, NULL)), NULL, NULL);
    ARRAY_NTYPE (tmp) = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (number));

    ARRAY_ISCONST (tmp) = TRUE;
    ARRAY_VECTYPE (tmp) = T_int;
    ARRAY_VECLEN (tmp) = number;
    ARRAY_CONSTVEC (tmp) = Array2Vec (T_int, ARRAY_AELEMS (tmp), NULL);
    DBUG_RETURN (tmp);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateNewPart(node * lb, node *ub, node * step, node *width,
 *                        node *withid, node *coden)
 *
 *   @brief creates a new N_Npart
 *
 *   @param  node *lb,*ub       : bound of new part
 *           node *step, *width : step, width of new part
 *           node *withid       : withid of new part
 *           node *coden        : Pointer of N_Ncode node where the
 *                                new generator shall point to.
 *   @return node *             : N_Npart
 ******************************************************************************/
node *
CreateNewPart (node *lb, node *ub, node *step, node *width, node *withid, node *coden)
{
    node *genn, *partn;

    DBUG_ENTER ("CreateNewPart");

    /* create tree structures */
    genn = MakeNGenerator (DupTree (lb), DupTree (ub), F_le, F_lt, DupTree (step),
                           DupTree (width));
    partn = MakeNPart (DupTree (withid), genn, coden);

    DBUG_RETURN (partn);
}

/** <!--********************************************************************-->
 *
 * @fn int NormalizeStepWidth(node **step, node **width)
 *
 *   @brief normalizes step and width. There are several forbidden and
 *          ambiguous combinations of bounds, step (s) and width (w).
 *
 *          allowed is the following (for each component):
 *          - w < s and s > 1 or
 *          - w == 1 and s == 1
 *
 *          transformations:
 *          - w == s: set w = s = 1 (componentwise)
 *          - vector s == 1 and vector w == 1: set both to NULL
 *          - vector s != NULL and vector w == NULL: create vector w = 1.
 *
 *
 *          errors:
 *          - w > s        (error no 1)
 *          - 1 > w        (error no 2)
 *          - w without s  (error no 3)
 *
 *   @param  node **step  :  step vector
 *           node **width :  width vektor
 *   @return int         :  returns 0 if no error was detected, else error no
 *                          (see above).
 ******************************************************************************/
static int
NormalizeStepWidth (node **step, node **width)
{
    node *stp, *wth;
    int stpnum, wthnum, veclen;
    int error = 0, is_1 = 1;

    DBUG_ENTER ("NormalizeStepWidth");

    if ((*width) != NULL && (*step) == NULL) {
        error = 3;
    } else if ((*step) != NULL) {

        if ((*width) == NULL) {
            /*  create width with constant 1 */
            if (ARRAY_ISCONST ((*step)))
                veclen = ARRAY_VECLEN ((*step));
            else
                veclen = CountExprs (ARRAY_AELEMS ((*step)));

            (*width) = CreateEntryFlatArray (1, veclen);
        }

        stp = ARRAY_AELEMS ((*step));
        wth = ARRAY_AELEMS ((*width));

        while (stp && !error) {
            DBUG_ASSERT ((wth != NULL), "dimensionality differs in step and width!");

            if ((NODE_TYPE (EXPRS_EXPR (stp)) == N_num)
                && (NODE_TYPE (EXPRS_EXPR (wth)) == N_num)) {

                stpnum = NUM_VAL (EXPRS_EXPR (stp));
                wthnum = NUM_VAL (EXPRS_EXPR (wth));

                if (wthnum > stpnum)
                    error = 1;
                else if (1 > wthnum)
                    error = 2;
                else if (wthnum == stpnum && stpnum != 1)
                    NUM_VAL (EXPRS_EXPR (stp)) = NUM_VAL (EXPRS_EXPR (wth)) = stpnum = 1;

                is_1 = is_1 && 1 == stpnum;
            } else if ((NODE_TYPE (EXPRS_EXPR (stp)) == N_id)
                       && (NODE_TYPE (EXPRS_EXPR (wth)) == N_id)) {
                if (!strcmp (ID_NAME (EXPRS_EXPR (stp)), ID_NAME (EXPRS_EXPR (wth)))) {
                    EXPRS_EXPR (stp) = FreeTree (EXPRS_EXPR (stp));
                    EXPRS_EXPR (stp) = MakeNum (1);
                    EXPRS_EXPR (wth) = FreeTree (EXPRS_EXPR (wth));
                    EXPRS_EXPR (wth) = MakeNum (1);
                    is_1 = is_1 && TRUE;
                } else {
                    is_1 = FALSE;
                }
            } else {
                is_1 = FALSE;
            }

            stp = EXPRS_NEXT (stp);
            wth = EXPRS_NEXT (wth);
        }

        /* if both vectors are 1 this is equivalent to no grid */

        if (!error && is_1) {
            (*step) = FreeTree (*step);
            (*width) = FreeTree (*width);
        }
    }

    DBUG_RETURN (error);
}

/** <!--********************************************************************-->
 *
 * @fn node *AppendPart2WL(node * wln, node *partn)
 *
 *   @brief append N_Npart npart to the existing parts of
 *          the N_Nwith node wln
 *
 *   @param  node *wln    : N_Nwith
 *           node *npart  : N_Npart
 *   @return node *       : modified N_Nwith
 ******************************************************************************/
static node *
AppendPart2WL (node *wln, node *partn)
{
    node *parts;
    int no_parts;

    DBUG_ENTER ("AppendPart2WL");

    parts = NWITH_PART (wln);
    /* at least one part exists */
    no_parts = 1;

    while (NPART_NEXT (parts)) {
        parts = NPART_NEXT (parts);
        no_parts++;
    }

    NPART_NEXT (parts) = partn;
    no_parts++;
    NWITH_PARTS (wln) = no_parts;

    DBUG_RETURN (wln);
}

/** <!--********************************************************************-->
 *
 * @fn node *CutSlices( node *ls, node *us, node *l, node *u, int dim,
 *                       node *wln, node *coden)
 *
 *   @brief  Creates a (full) partition by adding new N_Ngenerator nodes
 *           to the N_Nwith node.
 *           If the known part is a grid, this is ignored here (so the
 *           resulting N_Nwith node may still not be a full partition,
 *           see CompleteGrid()).
 *
 *   @param  node *ls, *us : bounds of the whole array
 *           node *l, *u   : bounds of the given part
 *           int dim       : number of elements of ls, us, l, u
 *           node *wln     : Pointer of N_Nwith node where the new generators
 *                           shall be inserted.
 *           node *coden   : Pointer of N_Ncode node where the new generators
 *                           shall point to.
 *   @return node *        : modified N_Nwith
 ******************************************************************************/
static node *
CutSlices (node *ls, node *us, node *l, node *u, int dim, node *wln, node *coden)
{
    node *lsc, *usc, *le, *ue, *lsce, *usce, *partn, *withidn, *ubn, *lbn;
    int i, d, lnum, lscnum, unum, uscnum;

    DBUG_ENTER ("CutSlices");

    /* create local copies of the arrays which atr modified here*/
    lsc = DupTree (ls);
    usc = DupTree (us);

    le = ARRAY_AELEMS (l);
    lsce = ARRAY_AELEMS (lsc);
    ue = ARRAY_AELEMS (u);
    usce = ARRAY_AELEMS (usc);

    withidn = DupTree (NPART_WITHID (NWITH_PART (wln)));

    for (d = 0; d < dim; d++) {
        /* Check whether there is a cuboid above (below) the given one. */

        if (NODE_TYPE (EXPRS_EXPR (le)) == N_num) {
            lnum = NUM_VAL (EXPRS_EXPR (le));
            lscnum = NUM_VAL (EXPRS_EXPR (lsce));
            if (lnum > lscnum) {
                partn = CreateNewPart (lsc, usc, NULL, NULL, withidn, coden);
                ubn = ARRAY_AELEMS (NPART_BOUND2 (partn));
                for (i = 0; i < d; i++) {
                    ubn = EXPRS_NEXT (ubn);
                }
                if (NODE_TYPE (EXPRS_EXPR (ubn)) == N_num) {
                    NUM_VAL (EXPRS_EXPR (ubn)) = lnum;
                } else {
                    EXPRS_EXPR (ubn) = FreeTree (EXPRS_EXPR (ubn));
                    EXPRS_EXPR (ubn) = DupTree (EXPRS_EXPR (le));
                }
                wln = AppendPart2WL (wln, partn);
            }
        } else {
            partn = CreateNewPart (lsc, usc, NULL, NULL, withidn, coden);
            ubn = ARRAY_AELEMS (NPART_BOUND2 (partn));
            for (i = 0; i < d; i++) {
                ubn = EXPRS_NEXT (ubn);
            }
            EXPRS_EXPR (ubn) = FreeTree (EXPRS_EXPR (ubn));
            EXPRS_EXPR (ubn) = DupTree (EXPRS_EXPR (le));
            if (ARRAY_ISCONST (NPART_BOUND2 (partn))) {
                ARRAY_ISCONST (NPART_BOUND2 (partn)) = FALSE;
                ARRAY_CONSTVEC (NPART_BOUND2 (partn))
                  = Free (ARRAY_CONSTVEC (NPART_BOUND2 (partn)));
            }
            wln = AppendPart2WL (wln, partn);
        }

        if ((NODE_TYPE (EXPRS_EXPR (ue)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (usce)) == N_num)) {
            unum = NUM_VAL (EXPRS_EXPR (ue));
            uscnum = NUM_VAL (EXPRS_EXPR (usce));
            if (unum < uscnum) {
                partn = CreateNewPart (lsc, usc, NULL, NULL, withidn, coden);
                lbn = ARRAY_AELEMS (NPART_BOUND1 (partn));
                for (i = 0; i < d; i++) {
                    lbn = EXPRS_NEXT (lbn);
                }
                NUM_VAL (EXPRS_EXPR (lbn)) = unum;
                wln = AppendPart2WL (wln, partn);
            }
        } else {
            partn = CreateNewPart (lsc, usc, NULL, NULL, withidn, coden);
            lbn = ARRAY_AELEMS (NPART_BOUND1 (partn));
            for (i = 0; i < d; i++) {
                lbn = EXPRS_NEXT (lbn);
            }
            EXPRS_EXPR (lbn) = FreeTree (EXPRS_EXPR (lbn));
            EXPRS_EXPR (lbn) = DupTree (EXPRS_EXPR (ue));
            if ((NODE_TYPE (EXPRS_EXPR (lbn)) != N_num)
                && (ARRAY_ISCONST (NPART_BOUND1 (partn)))) {
                ARRAY_ISCONST (NPART_BOUND1 (partn)) = FALSE;
                ARRAY_CONSTVEC (NPART_BOUND1 (partn))
                  = Free (ARRAY_CONSTVEC (NPART_BOUND1 (partn)));
            }
            wln = AppendPart2WL (wln, partn);
        }

        /* and modifiy array bounds to continue with next dimension */
        if (NODE_TYPE (EXPRS_EXPR (le)) == N_num) {
            NUM_VAL (EXPRS_EXPR (lsce)) = NUM_VAL (EXPRS_EXPR (le));
        } else {
            EXPRS_EXPR (lsce) = FreeTree (EXPRS_EXPR (lsce));
            EXPRS_EXPR (lsce) = DupTree (EXPRS_EXPR (le));
            if (ARRAY_ISCONST (lsc)) {
                ARRAY_ISCONST (lsc) = FALSE;
                ARRAY_CONSTVEC (lsc) = Free (ARRAY_CONSTVEC (lsc));
            }
        }

        if ((NODE_TYPE (EXPRS_EXPR (ue)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (usce)) == N_num)) {
            NUM_VAL (EXPRS_EXPR (usce)) = NUM_VAL (EXPRS_EXPR (ue));
        } else {
            EXPRS_EXPR (usce) = FreeTree (EXPRS_EXPR (usce));
            EXPRS_EXPR (usce) = DupTree (EXPRS_EXPR (ue));
            if ((NODE_TYPE (EXPRS_EXPR (usce)) != N_num) && (ARRAY_ISCONST (usc))) {
                ARRAY_ISCONST (usc) = FALSE;
                ARRAY_CONSTVEC (usc) = Free (ARRAY_CONSTVEC (usc));
            }
        }

        le = EXPRS_NEXT (le);
        lsce = EXPRS_NEXT (lsce);
        ue = EXPRS_NEXT (ue);
        usce = EXPRS_NEXT (usce);
    }

    lsc = FreeTree (lsc);
    usc = FreeTree (usc);
    withidn = FreeTree (withidn);

    DBUG_RETURN (wln);
}

/** <!--********************************************************************-->
 *
 * @fn node *CompleteGrid( node *ls, node *us, node *step, node *width,
 *                         int dim, node *wln, node *coden, info *arg_info)
 *
 *   @brief  adds new parts to N_Nwith which specify the elements
 *           left out by a grid.
 *
 *   @param  node *ls,*us       : bounds of the given part
 *           node *step, *width :
 *           int dim            : number of elements of ls, us
 *           node *wln          : N_Nwith where to add the new parts.
 *                                If ig != NULL, the same pointer is returned.
 *           node *coden        : Pointer of N_Ncode node where the new
 *                                generators shall point to.
 *           info *arg_info     : N_INFO is needed to create new vars
 *   @return intern_gen *       : chain of intern_gen struct
 ******************************************************************************/
static node *
CompleteGrid (node *ls, node *us, node *step, node *width, int dim, node *wln,
              node *coden, info *arg_info)
{
    node *nw, *stpe, *wthe, *nwe, *partn, *withidn, *lbn, *wthn, *tmp1, *tmp2, *nassign;
    int i, d, stpnum, wthnum;
    ids *_ids;

    DBUG_ENTER ("CompleteGrid");

    /* create local copies of the arrays which atr modified here*/
    nw = DupTree (step);

    stpe = ARRAY_AELEMS (step);
    wthe = ARRAY_AELEMS (width);
    nwe = ARRAY_AELEMS (nw);

    withidn = DupTree (NPART_WITHID (NWITH_PART (wln)));

    for (d = 0; d < dim; d++) {

        if ((NODE_TYPE (EXPRS_EXPR (stpe)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (wthe)) == N_num)) {
            stpnum = NUM_VAL (EXPRS_EXPR (stpe));
            wthnum = NUM_VAL (EXPRS_EXPR (wthe));

            if (stpnum > wthnum) { /* create new grids */

                partn = CreateNewPart (ls, us, step, nw, withidn, coden);
                lbn = ARRAY_AELEMS (NPART_BOUND1 (partn));
                wthn = ARRAY_AELEMS (NPART_WIDTH (partn));

                for (i = 0; i < d; i++) {
                    wthn = EXPRS_NEXT (wthn);
                    lbn = EXPRS_NEXT (lbn);
                }

                if (NODE_TYPE (EXPRS_EXPR (lbn)) == N_num) {
                    NUM_VAL (EXPRS_EXPR (lbn)) = NUM_VAL (EXPRS_EXPR (lbn)) + wthnum;
                } else {

                    _ids = NewIds (EXPRS_EXPR (lbn), INFO_WLPG_FUNDEF (arg_info));

                    /* the identifier to add wthnum to */
                    tmp1 = DupTree (EXPRS_EXPR (lbn));
                    /* N_num wthnum */
                    tmp2 = MakeExprs (DupTree (EXPRS_EXPR (wthe)), NULL);

                    tmp1 = MakePrf (F_add_SxS, MakeExprs (tmp1, tmp2));

                    nassign = MakeAssign (MakeLet (tmp1, _ids), NULL);
                    /* set correct backref to defining assignment */
                    AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

                    INFO_WLPG_NASSIGNS (arg_info)
                      = AppendAssign (INFO_WLPG_NASSIGNS (arg_info), nassign);

                    EXPRS_EXPR (lbn) = FreeTree (EXPRS_EXPR (lbn));
                    EXPRS_EXPR (lbn) = DupIds_Id (_ids);
                }
                NUM_VAL (EXPRS_EXPR (wthn)) = stpnum - wthnum;
                i = NormalizeStepWidth (&(NPART_STEP (partn)), &(NPART_WIDTH (partn)));
                DBUG_ASSERT (!i, ("internal normalization failure"));

                wln = AppendPart2WL (wln, partn);
            }
        } else {
            partn = CreateNewPart (ls, us, step, nw, withidn, coden);
            lbn = ARRAY_AELEMS (NPART_BOUND1 (partn));
            wthn = ARRAY_AELEMS (NPART_WIDTH (partn));

            for (i = 0; i < d; i++) {
                wthn = EXPRS_NEXT (wthn);
                lbn = EXPRS_NEXT (lbn);
            }

            if ((NODE_TYPE (EXPRS_EXPR (lbn)) == N_num)
                && (NODE_TYPE (EXPRS_EXPR (wthe)) == N_num)) {
                NUM_VAL (EXPRS_EXPR (lbn))
                  = NUM_VAL (EXPRS_EXPR (lbn)) + NUM_VAL (EXPRS_EXPR (wthe));
            } else {

                _ids = NewIds (EXPRS_EXPR (lbn), INFO_WLPG_FUNDEF (arg_info));

                /* the identifier to add current width to */
                tmp1 = DupTree (EXPRS_EXPR (lbn));
                /* current width as N_num or N_id */
                tmp2 = MakeExprs (DupTree (EXPRS_EXPR (wthe)), NULL);

                tmp1 = MakePrf (F_add_SxS, MakeExprs (tmp1, tmp2));

                nassign = MakeAssign (MakeLet (tmp1, _ids), NULL);
                /* set correct backref to defining assignment */
                AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

                INFO_WLPG_NASSIGNS (arg_info)
                  = AppendAssign (INFO_WLPG_NASSIGNS (arg_info), nassign);

                EXPRS_EXPR (lbn) = FreeTree (EXPRS_EXPR (lbn));
                EXPRS_EXPR (lbn) = DupIds_Id (_ids);
            }

            if ((NODE_TYPE (EXPRS_EXPR (stpe)) == N_num)
                && (NODE_TYPE (EXPRS_EXPR (wthe)) == N_num)) {
                NUM_VAL (EXPRS_EXPR (wthn))
                  = NUM_VAL (EXPRS_EXPR (stpe)) - NUM_VAL (EXPRS_EXPR (wthe));
            } else {

                _ids = NewIds (EXPRS_EXPR (wthn), INFO_WLPG_FUNDEF (arg_info));

                /* the first identifier */
                tmp1 = DupTree (EXPRS_EXPR (stpe));
                /* the second identifier */
                tmp2 = MakeExprs (DupTree (EXPRS_EXPR (wthe)), NULL);

                tmp1 = MakePrf (F_sub_SxS, MakeExprs (tmp1, tmp2));

                nassign = MakeAssign (MakeLet (tmp1, _ids), NULL);
                /* set correct backref to defining assignment */
                AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

                INFO_WLPG_NASSIGNS (arg_info)
                  = AppendAssign (INFO_WLPG_NASSIGNS (arg_info), nassign);

                EXPRS_EXPR (wthn) = FreeTree (EXPRS_EXPR (wthn));
                EXPRS_EXPR (wthn) = DupIds_Id (_ids);
            }

            i = NormalizeStepWidth (&(NPART_STEP (partn)), &(NPART_WIDTH (partn)));
            DBUG_ASSERT (!i, ("internal normalization failure"));

            wln = AppendPart2WL (wln, partn);
        }

        /* and modifiy array bounds to continue with next dimension */
        if ((NODE_TYPE (EXPRS_EXPR (nwe)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (wthe)) == N_num)) {
            NUM_VAL (EXPRS_EXPR (nwe)) = NUM_VAL (EXPRS_EXPR (wthe));
        } else {
            EXPRS_EXPR (nwe) = FreeTree (EXPRS_EXPR (nwe));
            EXPRS_EXPR (nwe) = DupTree (EXPRS_EXPR (wthe));
            if ((NODE_TYPE (EXPRS_EXPR (nwe)) != N_num) && (ARRAY_ISCONST (nw))) {
                ARRAY_ISCONST (nw) = FALSE;
                ARRAY_CONSTVEC (nw) = Free (ARRAY_CONSTVEC (nw));
            }
        }

        stpe = EXPRS_NEXT (stpe);
        wthe = EXPRS_NEXT (wthe);
        nwe = EXPRS_NEXT (nwe);
    }

    nw = FreeTree (nw);

    DBUG_RETURN (wln);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateScalarWL( int dim, node *array_shape, simpletype btype,
 *                           node *expr, node *fundef)
 *
 *   @brief  build new genarray WL of shape 'array_shape' and blockinstr.
 *           'expr'
 *
 *   @param  int  *dim         : dimension of iteration space
 *           node *array_shape : shape and upper bound of WL
 *           simpletype btype  : type of 'expr'
 *           node *expr        : rhs of BLOCK_INSTR
 *           node *fundef      : N_FUNDEF
 *   @return node *            : N_Nwith
 ******************************************************************************/
static node *
CreateScalarWL (int dim, node *array_shape, simpletype btype, node *expr, node *fundef)
{
    node *wl;
    node *id;
    node *vardecs = NULL;
    ids *vec_ids;
    ids *scl_ids = NULL;
    ids *tmp_ids;
    int i;

    DBUG_ENTER ("CreateScalarWL");

    DBUG_ASSERT ((dim >= 0), "CreateScalarWith() used with unknown shape!");

    vec_ids = MakeIds (TmpVar (), NULL, ST_regular);
    vardecs
      = MakeVardec (StringCopy (IDS_NAME (vec_ids)),
                    MakeTypes (T_int, 1, MakeShpseg (MakeNums (dim, NULL)), NULL, NULL),
                    vardecs);
    AVIS_TYPE (VARDEC_AVIS (vardecs))
      = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (dim));
    IDS_VARDEC (vec_ids) = vardecs;
    IDS_AVIS (vec_ids) = VARDEC_AVIS (vardecs);

    for (i = 0; i < dim; i++) {
        tmp_ids = MakeIds (TmpVar (), NULL, ST_regular);
        vardecs
          = MakeVardec (StringCopy (IDS_NAME (tmp_ids)), MakeTypes1 (T_int), vardecs);
        IDS_NEXT (tmp_ids) = scl_ids;
        scl_ids = tmp_ids;
        AVIS_TYPE (VARDEC_AVIS (vardecs))
          = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (0));
        IDS_VARDEC (scl_ids) = vardecs;
        IDS_AVIS (scl_ids) = VARDEC_AVIS (vardecs);
    }

    id = MakeId (TmpVar (), NULL, ST_regular);
    vardecs = MakeVardec (StringCopy (ID_NAME (id)), MakeTypes1 (btype), vardecs);
    AVIS_TYPE (VARDEC_AVIS (vardecs))
      = TYMakeAKS (TYMakeSimpleType (btype), SHMakeShape (0));
    ID_VARDEC (id) = vardecs;
    ID_AVIS (id) = VARDEC_AVIS (vardecs);

    wl = MakeNWith (MakeNPart (MakeNWithid (vec_ids, scl_ids),
                               MakeNGenerator (CreateZeroVector (dim, T_int),
                                               DupNode (array_shape), F_le, F_lt, NULL,
                                               NULL),
                               NULL),
                    MakeNCode (MakeBlock (MakeAssignLet (StringCopy (ID_NAME (id)),
                                                         vardecs, expr),
                                          NULL),
                               MakeExprs (id, NULL)),
                    MakeNWithOp (WO_genarray, DupNode (array_shape)));
    NCODE_USED (NWITH_CODE (wl))++;
    NPART_CODE (NWITH_PART (wl)) = NWITH_CODE (wl);
    NWITH_PARTS (wl) = 1;

    fundef = AddVardecs (fundef, vardecs);

    DBUG_RETURN (wl);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateZeros( node *array, node **nassigns, node *fundef)
 *
 *   @brief creates an array of zeros depending on shape and type of 'array'
 *
 *   @param  node *array    :
 *           node **nassign :  != NULL iff new assignments have been created
 *                             (AKD case)
 *           node *fundef   :  N_FUNDEF
 *   @return node *         :  array of zeros
 ******************************************************************************/
static node *
CreateZeros (node *array, node **nassigns, node *fundef)
{
    node *zero = NULL, *assigns = NULL;
    ntype *array_type;
    simpletype btype;
    shape *shape;
    int dim;

    DBUG_ENTER ("CreateZeros");

    DBUG_ASSERT ((NODE_TYPE (array) == N_id), "no N_id node found!");

    array_type = ID_NTYPE (array);
    dim = TYGetDim (array_type);
    DBUG_ASSERT ((TYIsSimple (array_type) == FALSE), "N_id is no array type!");
    btype = TYGetSimpleType (TYGetScalar (array_type));
    shape = TYGetShape (array_type);

    if (dim == 0) {
        zero = CreateZeroScalar (btype);
    } else {
        node *array_shape = NULL;

        if (TYIsAKV (array_type) || TYIsAKS (array_type)) {

            array_shape = SHShape2Array (shape);
            ARRAY_NTYPE (array_shape)
              = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (dim));

            ARRAY_ISCONST (array_shape) = TRUE;
            ARRAY_VECTYPE (array_shape) = T_int;
            ARRAY_VECLEN (array_shape) = dim;
            ARRAY_CONSTVEC (array_shape)
              = Array2Vec (T_int, ARRAY_AELEMS (array_shape), NULL);

        } else { /* AKD array */
            assigns = CreateIdxShapeSelAssigns (array, 0, (dim - 1), fundef);
            array_shape = CreateStructConstant (array_shape, assigns);
        }
        zero = CreateScalarWL (dim, array_shape, btype, CreateZeroScalar (btype), fundef);
        array_shape = FreeNode (array_shape);
    }

    (*nassigns) = assigns;
    DBUG_RETURN (zero);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateArraySel( ids *sel_vec, ids *sel_ids, node *sel_array,
 *                           node **nassigns, node *fundef)
 *
 *   @brief creates an WL with element-wise reference on 'sel_array'
 *
 *   @param  ids  *sel_vec   : N_WITHID_VEC of current WL
 *           ids  *sel_ids   : N_WITHID_IDS of current WL
 *           node *sel_array :
 *           node **nassigns : != NULL iff new assignments have been created
 *                             (AKD case)
 *           node *fundef    : N_FUNDEF
 *   @return node *          : N_Nwith
 ******************************************************************************/
static node *
CreateArraySel (ids *sel_vec, ids *sel_ids, node *sel_array, node **nassigns,
                node *fundef)
{
    node *sel;
    node *assigns = NULL;
    int len_index, dim_array;

    DBUG_ENTER ("CreateArraySel");

    DBUG_ASSERT ((NODE_TYPE (sel_array) == N_id), "no N_id node found!");

    len_index = IDS_SHAPE (sel_vec, 0);
    DBUG_ASSERT ((len_index > 0), "illegal index length found!");

    dim_array = TYGetDim (ID_NTYPE (sel_array));
    DBUG_ASSERT ((dim_array > 0), "illegal array dimensionality found!");

    if (len_index > dim_array) {
        DBUG_ASSERT ((0), "illegal array selection found!");
        sel = NULL;
    } else if ((len_index == dim_array)) {
        sel = MakePrf (F_sel, MakeExprs (DupIds_Id (sel_vec),
                                         MakeExprs (DupNode (sel_array), NULL)));
    } else { /* (len_index < dim_array) */
        node *new_index, *id, *vardec, *ass;
        node *array_shape = NULL;
        ids *tmp_ids;
        shape *shape, *mshape;
        simpletype btype;
        ntype *array_type;
        int dim;

        array_type = ID_NTYPE (sel_array);
        dim = (dim_array - len_index);

        btype = TYGetSimpleType (TYGetScalar (array_type));

        if (TYIsAKV (array_type) || TYIsAKS (array_type)) {

            shape = TYGetShape (array_type);
            mshape = SHDropFromShape (len_index, shape);
            array_shape = SHShape2Array (mshape);
            ARRAY_NTYPE (array_shape)
              = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (dim));

            ARRAY_ISCONST (array_shape) = TRUE;
            ARRAY_VECTYPE (array_shape) = T_int;
            ARRAY_VECLEN (array_shape) = dim;
            ARRAY_CONSTVEC (array_shape)
              = Array2Vec (T_int, ARRAY_AELEMS (array_shape), NULL);

            if (mshape) {
                SHFreeShape (mshape);
            }
        } else { /* AKD array */
            assigns
              = CreateIdxShapeSelAssigns (sel_array, len_index, (dim_array - 1), fundef);
            array_shape = CreateStructConstant (array_shape, assigns);
        }

        /*
         * create WL without expression
         */
        sel = CreateScalarWL (dim, array_shape, btype, NULL, fundef);

        array_shape = FreeNode (array_shape);

        /*
         * create index vector for F_sel
         */
        tmp_ids = sel_ids;
        while (IDS_NEXT (tmp_ids) != NULL) {
            tmp_ids = IDS_NEXT (tmp_ids);
        }
        IDS_NEXT (tmp_ids) = NWITH_IDS (sel); /* concat ids chains */
        new_index = Ids2Array (sel_ids);
        IDS_NEXT (tmp_ids) = NULL; /* restore ids chains */

        /*
         * create new id
         */
        id = MakeId (TmpVar (), NULL, ST_regular);
        vardec = MakeVardec (StringCopy (ID_NAME (id)),
                             DupOneTypes (ARRAY_TYPE (new_index)), NULL);
        ID_VARDEC (id) = vardec;
        ID_AVIS (id) = VARDEC_AVIS (vardec);

        AVIS_TYPE (VARDEC_AVIS (vardec)) = TYOldType2Type (ARRAY_TYPE (new_index));

        fundef = AddVardecs (fundef, vardec);

        /*
         * create expression 'sel( tmp, A)' and insert it into WL
         */
        ASSIGN_RHS (BLOCK_INSTR (NWITH_CBLOCK (sel)))
          = MakePrf (F_sel, MakeExprs (id, MakeExprs (DupNode (sel_array), NULL)));

        /*
         * create assignment 'tmp = [...];' and insert it into WL
         */
        ass = MakeAssignLet (StringCopy (ID_NAME (id)), vardec, new_index);
        ASSIGN_NEXT (ass) = BLOCK_INSTR (NWITH_CBLOCK (sel));
        BLOCK_INSTR (NWITH_CBLOCK (sel)) = ass;
    }

    (*nassigns) = assigns;
    DBUG_RETURN (sel);
}

/** <!--********************************************************************-->
 *
 * @fn  node *CreateFullPartition( node *wln, info *arg_info)
 *
 *   @brief  generates full partition if possible:
 *           - if withop is genarray and index vector has as much elements as
 *             dimension of resulting WL (withloop on scalars).
 *           - if withop is modarray: always (needed for compilation phase).
 *           Returns wln.
 *
 *   @param  node *wl       :  N_with node of the WL to transform
 *           info *arg_info :  is needed to access the vardecs of the current
 *                             function
 *   @return node *         :  modified N_with
 ******************************************************************************/
static node *
CreateFullPartition (node *wln, info *arg_info)
{
    node *coden, *idn, *nassign, *array_shape = NULL, *array_null, *vardec, *nassigns;
    ids *_ids;
    types *type;
    ntype *array_type;
    shape *shape, *mshape;
    char *nvarname;
    int gen_shape = 0;
    bool do_create;

    DBUG_ENTER ("CreateFullPartition");

    do_create = TRUE;

    /* get shape of the index vector (generator) */
    gen_shape = IDS_SHAPE (NWITH_VEC (wln), 0);
    DBUG_ASSERT ((gen_shape > 0), "shape of index vector has to be > 0!");

    /*
     * allocate "array_shape"
     */
    switch (NWITH_TYPE (wln)) {
    case WO_modarray: {
        /* create upper array bound */
        array_type = ID_NTYPE (NWITH_ARRAY (wln));

        if (TYIsAKV (array_type) || TYIsAKS (array_type)) {

            shape = TYGetShape (array_type);
            /* only for iteration space */
            mshape = SHTakeFromShape (gen_shape, shape);
            array_shape = SHShape2Array (mshape);
            ARRAY_NTYPE (array_shape)
              = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (gen_shape));

            ARRAY_ISCONST (array_shape) = TRUE;
            ARRAY_VECTYPE (array_shape) = T_int;
            ARRAY_VECLEN (array_shape) = gen_shape;
            ARRAY_CONSTVEC (array_shape)
              = Array2Vec (T_int, ARRAY_AELEMS (array_shape), NULL);

            if (mshape) {
                SHFreeShape (mshape);
            }
        } else if (TYIsAKD (array_type)) {

            nassigns = CreateIdxShapeSelAssigns (NWITH_ARRAY (wln), 0, (gen_shape - 1),
                                                 INFO_WLPG_FUNDEF (arg_info));
            array_shape = CreateStructConstant (array_shape, nassigns);
            INFO_WLPG_NASSIGNS (arg_info)
              = AppendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
        } else {
            /* dimension unknown */
            array_shape = NULL;
        }
    } break;

    case WO_genarray:
        array_shape = DupTree (NWITH_SHAPE (wln));
        break;

    default:
        DBUG_ASSERT ((0), "illegal NWITH_TYPE found!");
        array_shape = NULL;
        break;
    }

    do_create = (array_shape != NULL);

    /*
     * start creation
     */
    if (do_create) {
        /* create lower array bound */

        array_null = CreateEntryFlatArray (0, gen_shape);

        /* create code for all new parts */
        nassigns = NULL;
        if (NWITH_TYPE (wln) == WO_genarray) {
            if (sbs == 1) {
                if (NWITHOP_DEFAULT (NWITH_WITHOP (wln)) == NULL) {
                    coden = CreateZeros (NWITH_CEXPR (wln), &(nassigns),
                                         INFO_WLPG_FUNDEF (arg_info));
                } else {
                    coden = DupTree (NWITHOP_DEFAULT (NWITH_WITHOP (wln)));
                }
            } else {
                coden = CreateZeros (NWITH_CEXPR (wln), &(nassigns),
                                     INFO_WLPG_FUNDEF (arg_info));
            }
        } else { /* modarray */
            coden = CreateArraySel (NWITH_VEC (wln), NWITH_IDS (wln), NWITH_ARRAY (wln),
                                    &(nassigns), INFO_WLPG_FUNDEF (arg_info));
        }

        nvarname = TmpVar ();
        _ids = MakeIds (nvarname, NULL, ST_regular);
        /* determine type of expr in the operator (result of body) */
        type = ID_TYPE (NWITH_CEXPR (wln));
        vardec = MakeVardec (StringCopy (nvarname), DupOneTypes (type), NULL);

        AVIS_TYPE (VARDEC_AVIS (vardec))
          = TYCopyType (AVIS_TYPE (ID_AVIS (NWITH_CEXPR (wln))));
        IDS_VARDEC (_ids) = vardec;
        IDS_AVIS (_ids) = VARDEC_AVIS (vardec);
        INFO_WLPG_FUNDEF (arg_info) = AddVardecs (INFO_WLPG_FUNDEF (arg_info), vardec);

        idn = DupIds_Id (_ids);

        /* create new N_Ncode node  */
        nassign = MakeAssign (MakeLet (coden, _ids), NULL);
        /* set correct backref to defining assignment */
        AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;
        nassigns = AppendAssign (nassigns, nassign);
        coden = MakeNCode (MakeBlock (nassigns, NULL), MakeExprs (idn, NULL));

        /* create surrounding cuboids */
        wln = CutSlices (array_null, array_shape, NWITH_BOUND1 (wln), NWITH_BOUND2 (wln),
                         gen_shape, wln, coden);

        /* the original part can still be found at first position in wln.
           Now create grids. */
        if (NWITH_STEP (wln))
            wln = CompleteGrid (NWITH_BOUND1 (wln), NWITH_BOUND2 (wln), NWITH_STEP (wln),
                                NWITH_WIDTH (wln), gen_shape, wln, coden, arg_info);

        /* if new codes have been created, add them to code list */
        if (NPART_NEXT (NWITH_PART (wln))) {
            NCODE_NEXT (coden) = NWITH_CODE (wln);
            NWITH_CODE (wln) = coden;
        }

        /* free the above made arrays */
        array_null = FreeTree (array_null);
    }
    array_shape = FreeTree (array_shape);

    DBUG_RETURN (wln);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateEmptyGenWLReplacement( node *wl, info *arg_info)
 *
 *   @brief  computes the replacement for the given WL under the assumption that
 *           the generator is empty, i.e.,
 *
 *           with( lb <= iv < ub)              =>     with( 0*shp <= iv <shp)
 *           genarray( shp, exp, dexp)                genarray( shp, dexp, ---)
 *
 *           with( lb <= iv < ub)              =>     a
 *           modarray( a, NULL, exp)
 *
 *           with( lb <= iv < ub)              =>     neutral
 *           fold( fun, neutral, exp)
 *
 *           NOTE HERE, that wl is either MODIFIED in place or FREED entirely!!
 *
 *   @param  node *wl       :  N_with
 *           info *arg_info :  N_INFO
 *   @return node *         :  modified N_with
 ******************************************************************************/
static node *
CreateEmptyGenWLReplacement (node *wl, info *arg_info)
{
    ids *let_ids;
    int dim, i;
    node *lb, *ub, *lbe, *ube;
    node *code;
    node *tmpn, *assignn, *blockn, *cexpr;
    node *nassigns = NULL;
    ids *_ids;

    node *res;

    DBUG_ENTER ("CreateEmptyGenWLReplacement");

    switch (NWITH_TYPE (wl)) {
    case WO_genarray:
        res = wl;
        /*
         * First, we change the generator to full scope.
         */
        let_ids = LET_IDS (INFO_WLPG_LET (arg_info));
        dim = SHGetDim (TYGetShape (AVIS_TYPE (IDS_AVIS (let_ids))));
        lb = NWITH_BOUND1 (wl);
        ub = NWITH_BOUND2 (wl);
        lbe = ARRAY_AELEMS (lb);
        ube = ARRAY_AELEMS (ub);

        i = 0;
        while ((i < dim) && (lbe != NULL)) {
            NUM_VAL (EXPRS_EXPR (lbe)) = 0;
            NUM_VAL (EXPRS_EXPR (ube)) = IDS_SHAPE (let_ids, i);

            lbe = EXPRS_NEXT (lbe);
            ube = EXPRS_NEXT (ube);
            i++;
        }

        if (NWITH_STEP (wl)) {
            NWITH_STEP (wl) = FreeTree (NWITH_STEP (wl));
        }
        if (NWITH_WIDTH (wl)) {
            NWITH_WIDTH (wl) = FreeTree (NWITH_WIDTH (wl));
        }

        /*
         * Now we have to change the code. Either we can use DEFAULT (easy)
         * or we have to create zeros (ugly).
         */
        code = NWITH_CODE (wl);
        if (NWITH_DEFAULT (wl) != NULL) {
            NCODE_CBLOCK (code) = FreeTree (NCODE_CBLOCK (code));
            NCODE_CBLOCK (code) = MakeEmpty ();

            NCODE_CEXPR (code) = FreeTree (NCODE_CEXPR (code));
            NCODE_CEXPR (code) = DupTree (NWITH_DEFAULT (wl));

        } else {
            blockn = NCODE_CBLOCK (code);
            tmpn = BLOCK_INSTR (blockn);
            cexpr = NCODE_CEXPR (code);

            if (N_empty == NODE_TYPE (tmpn)) {
                /* there is no instruction in the block right now. */
                _ids = NewIds (cexpr, INFO_WLPG_FUNDEF (arg_info));

                tmpn = CreateZeros (cexpr, &(nassigns), INFO_WLPG_FUNDEF (arg_info));

                /* replace N_empty with new assignment "_ids = [0,..,0]" */
                assignn = MakeAssign (MakeLet (tmpn, _ids), NULL);
                nassigns = AppendAssign (nassigns, assignn);
                BLOCK_INSTR (blockn) = AppendAssign (BLOCK_INSTR (blockn), nassigns);

                /* set correct backref to defining assignment */
                AVIS_SSAASSIGN (IDS_AVIS (_ids)) = assignn;

                /* replace CEXPR */
                tmpn = NWITH_CODE (INFO_WLPG_WL (arg_info));
                NCODE_CEXPR (tmpn) = FreeTree (NCODE_CEXPR (tmpn));
                NCODE_CEXPR (tmpn) = DupIds_Id (_ids);

            } else {
                /* we have a non-empty block.
                   search cexpr assignment and make it the only one in the block. */

                assignn = DupNode (AVIS_SSAASSIGN (ID_AVIS (cexpr)));
                BLOCK_INSTR (blockn) = FreeTree (BLOCK_INSTR (blockn));
                LET_EXPR (ASSIGN_INSTR (assignn))
                  = FreeTree (LET_EXPR (ASSIGN_INSTR (assignn)));

                tmpn = CreateZeros (cexpr, &(nassigns), INFO_WLPG_FUNDEF (arg_info));
                NCODE_FLAG (code) = TRUE;
                LET_EXPR (ASSIGN_INSTR (assignn)) = tmpn;
                nassigns = AppendAssign (nassigns, assignn);

                BLOCK_INSTR (blockn) = nassigns;
            }
        }
        break;
    case WO_modarray:
        res = DupTree (NWITH_ARRAY (wl));
        wl = FreeTree (wl);
        break;
    default:
        res = DupTree (NWITH_NEUTRAL (wl));
        wl = FreeTree (wl);
        break;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn gen_shape_t PropagateArrayConstants( node **expr)
 *
 *   @brief expects (*expr) to point either to a scalar constant, to an array,
 *          or to an identifyer.
 *          If (**expr) is constant or structural constant, the according node
 *          (N_array / scalar) is freshly generated and (*expr) is modified to
 *          point to the new node. The old one is freed!
 *          If (**expr) is an identifier, the node is checked if it is possible
 *          to create an structural constant.
 *          It returns GV_constant iff (**expr) is constant or
 *          (*expr) == NULL!!
 *
 *   @param  node **expr  :  expr
 *   @return gen_shape_t  :  GV_constant, GV_struct_constant, GV_known_shape,
 *                           GV_unknown_shape
 ******************************************************************************/

static gen_shape_t
PropagateArrayConstants (node **expr)
{
    constant *const_expr;
    struct_constant *sco_expr;
    node *tmp;
    gen_shape_t gshape;

    DBUG_ENTER ("PropagateArrayConstants");

    gshape = GV_unknown_shape;

    if ((*expr) != NULL) {
        const_expr = COAST2Constant ((*expr));
        if (const_expr != NULL) {
            gshape = GV_constant;
            (*expr) = FreeTree (*expr);
            (*expr) = COConstant2AST (const_expr);
            const_expr = COFreeConstant (const_expr);

        } else {
            sco_expr = SCOExpr2StructConstant ((*expr));
            if (sco_expr != NULL) {
                gshape = GV_struct_constant;
                /*
                 * as the sco_expr may share some subexpressions with (*expr),
                 * we have to duplicate these BEFORE deleting (*expr)!!!
                 */
                tmp = SCODupStructConstant2Expr (sco_expr);
                (*expr) = FreeTree (*expr);
                (*expr) = tmp;
                sco_expr = SCOFreeStructConstant (sco_expr);

            } else if (ID_DIM ((*expr)) == 1) {
                if (ID_SHPSEG ((*expr)) != NULL) {
                    if (ID_SHAPE ((*expr), 0) > 0) {
                        gshape = GV_known_shape;
                    }
                }
            }
        }

    } else {
        gshape = GV_constant;
    }

    DBUG_RETURN (gshape);
}

/******************************************************************************
 *
 * function:
 *   node * CropBounds( node *wl, shape *max_shp)
 *
 * description:
 *   expects the bound expression for lb and ub to be constants!
 *   expects also the WL either to by WO_modarray or WO_genarray!
 *
 *   Checks, whether lb and ub fit into the shape given by max_shp.
 *   If they exceed max_shp, they are "fitted"!
 *   The potentially modified WL is returned.
 *
 ******************************************************************************/

static node *
CropBounds (node *wl, shape *max_shp)
{
    node *lbe, *ube;
    int dim;
    int lbnum, ubnum, tnum;

    DBUG_ENTER ("CropBounds");

    DBUG_ASSERT (((NWITH_TYPE (wl) == WO_modarray) || (NWITH_TYPE (wl) == WO_genarray)),
                 "CropBounds applied to wrong WL type!");
    lbe = ARRAY_AELEMS (NWITH_BOUND1 (wl));
    ube = ARRAY_AELEMS (NWITH_BOUND2 (wl));

    dim = 0;
    while (lbe) {
        DBUG_ASSERT ((ube != NULL), "dimensionality differs in lower and upper bound!");
        DBUG_ASSERT (((NODE_TYPE (EXPRS_EXPR (lbe)) == N_num)
                      && (NODE_TYPE (EXPRS_EXPR (ube)) == N_num)),
                     "generator bounds must be constant!");
        lbnum = NUM_VAL (EXPRS_EXPR (lbe));
        ubnum = NUM_VAL (EXPRS_EXPR (ube));

        DBUG_ASSERT ((dim < SHGetDim (max_shp)),
                     "dimensionality of lb greater than that of the result!");
        tnum = SHGetExtent (max_shp, dim);
        if (lbnum < 0) {
            NUM_VAL (EXPRS_EXPR (lbe)) = 0;
            WARN (NODE_LINE (wl),
                  ("lower bound of WL-generator in dim %d below zero: set to 0", dim));
        }
        if (ubnum > tnum) {
            NUM_VAL (EXPRS_EXPR (ube)) = tnum;
            WARN (NODE_LINE (wl),
                  ("upper bound of WL-generator in dim %d greater than shape:"
                   " set to %d",
                   dim, tnum));
        }

        dim++;
        lbe = EXPRS_NEXT (lbe);
        ube = EXPRS_NEXT (ube);
    }
    DBUG_RETURN (wl);
}

/******************************************************************************
 *
 * function:
 *   gen_prop_t ComputeGeneratorProperties( node *wl, shape *max_shp)
 *
 * description:
 *   Computes the properties of the given WLs generator.
 *
 ******************************************************************************/

static gen_prop_t
ComputeGeneratorProperties (node *wl, shape *max_shp)
{
    node *lbe, *ube, *steps, *width;
    gen_prop_t res = GPT_unknown;
    bool const_bounds, non_empty_bounds;
    constant *lbc, *ubc, *shpc, *tmpc, *tmp;
    shape *sh;

    DBUG_ENTER ("ComputeGeneratorProperties");

    lbe = NWITH_BOUND1 (wl);
    ube = NWITH_BOUND2 (wl);
    steps = NWITH_STEP (wl);
    width = NWITH_WIDTH (wl);

    lbc = COAST2Constant (lbe);
    ubc = COAST2Constant (ube);
    if (max_shp != NULL) {
        shpc = COMakeConstantFromShape (max_shp);
    } else {
        shpc = NULL;
    }
    const_bounds = ((lbc != NULL) && (ubc != NULL));

    /**
     * First, we check for emptyness:
     * (this is done prior to checking on GPT_full, as we may have both properties
     *  GPT_empty and GPT_full, in which case we prefer to obtain GPT_empty as
     *  result....)
     */
    if (const_bounds) {
        non_empty_bounds = (SHGetUnrLen (COGetShape (lbc)) > 0);

        if (non_empty_bounds) {
            tmpc = COGe (lbc, ubc);
            if (COIsTrue (tmpc, TRUE)) {
                res = GPT_empty;
            }
            tmpc = COFreeConstant (tmpc);
        }
    }

    if (res == GPT_unknown) {
        if ((NWITH_TYPE (wl) == WO_foldprf) || (NWITH_TYPE (wl) == WO_foldfun)) {
            res = GPT_full;
        } else {
            /**
             * We are dealing with a modarray or a genarray WL here!
             * Only if the bounds are constant and the shape is known,
             * we can do any better than GPT_unknown!
             */
            if (const_bounds && (shpc != NULL)) {
                /**
                 * In order to obtain be a full partition,
                 * ubc must be a prefix of shpc,
                 * all elements of lbc must be zero
                 * and there must be not step vector
                 */
                sh = COGetShape (ubc);
                tmp = COMakeConstantFromShape (sh);
                tmpc = COTake (tmp, shpc);

                tmp = COFreeConstant (tmp);
                shpc = COFreeConstant (shpc);

                shpc = tmpc;

                tmpc = COEq (ubc, shpc);
                if (COIsZero (lbc, TRUE) && COIsTrue (tmpc, TRUE)) {
                    if (steps == NULL) {
                        res = GPT_full;
                    } else {
                        res = GPT_partial;
                    }
                } else {
                    res = GPT_partial;
                }
                tmpc = COFreeConstant (tmpc);
            }
        }
    }

    /* Clean up constants */
    shpc = (shpc != NULL ? COFreeConstant (shpc) : NULL);
    ubc = (ubc != NULL ? COFreeConstant (ubc) : NULL);
    lbc = (lbc != NULL ? COFreeConstant (lbc) : NULL);

    DBUG_PRINT ("WLPG", ("generator property of with loop in line %d : %s", linenum,
                         gen_prop_str[res]));
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGmodul(node *arg_node, info *arg_info)
 *
 *   @brief first traversal of function definitions for ConstantFolding
 *          and WLPartitionGeneration
 *
 *   @param  node *arg_node:  N_modul
 *           info *arg_info:  N_info
 *   @return node *        :  N_modul
 ******************************************************************************/

node *
WLPGmodul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPGmodul");

    INFO_WLPG_MODUL (arg_info) = arg_node;

    DBUG_PRINT ("WLPG", ("1. Constant Folding"));
    /* For Constant Folding */
    if (MODUL_FUNS (arg_node) != NULL) {
        Trav (MODUL_FUNS (arg_node), arg_info);
    }

    if ((break_after == PH_wlenhance) && (0 == strcmp (break_specifier, "cf")))
        goto DONE;

    DBUG_PRINT ("WLPG", ("2. WLPartitionGeneration"));
    /* For WLPartitionGeneration module-wise */
    INFO_WLPG_SUBPHASE (arg_info) = SP_mod;
    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

DONE:
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGfundef(node *arg_node, info *arg_info)
 *
 *   @brief starts the traversal of the given fundef.
 *
 *   @param  node *arg_node:  N_fundef
 *           info *arg_info:  N_info
 *   @return node *        :  N_fundef
 ******************************************************************************/

node *
WLPGfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPGfundef");

    INFO_WLPG_WL (arg_info) = NULL;
    INFO_WLPG_FUNDEF (arg_info) = arg_node;

    if (INFO_WLPG_SUBPHASE (arg_info) == SP_cf) {
        /*
         * to compute primitive function for constant expressions, especially
         * for bounds, steps and widths of WL's generator.
         */
        if (optimize & OPT_CF) {
            arg_node
              = SSAConstantFolding (arg_node, INFO_WLPG_MODUL (arg_info)); /* ssacf_tab */
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }

    } else if (INFO_WLPG_SUBPHASE (arg_info) == SP_mod) {

        if (FUNDEF_BODY (arg_node)) {
            FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }
    } else if (INFO_WLPG_SUBPHASE (arg_info) == SP_func) {
        /* compiler_phase == PH_sacopt */
        if (FUNDEF_BODY (arg_node)) {
            FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGassign(node *arg_node, info *arg_info)
 *
 *   @brief traverse instruction. If this creates new assignments in
 *          INFO_WLPG_NASSIGNS these are inserted in front of the actual one.
 *
 *   @param  node *arg_node:  N_assign
 *           info *arg_info:  N_info
 *   @return node *        :  N_assign
 ******************************************************************************/

node *
WLPGassign (node *arg_node, info *arg_info)
{
    node *iterator;

    DBUG_ENTER ("WLPGassign");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    iterator = NULL;

    if (INFO_WLPG_NASSIGNS (arg_info) != NULL) {
        iterator = INFO_WLPG_NASSIGNS (arg_info);
        while (ASSIGN_NEXT (iterator)) {
            iterator = ASSIGN_NEXT (iterator);
        }
        ASSIGN_NEXT (iterator) = arg_node;
        /* to traverse not in circle */
        iterator = ASSIGN_NEXT (iterator);

        arg_node = INFO_WLPG_NASSIGNS (arg_info);
        INFO_WLPG_NASSIGNS (arg_info) = NULL;

        if (ASSIGN_NEXT (iterator) != NULL) {
            ASSIGN_NEXT (iterator) = Trav (ASSIGN_NEXT (iterator), arg_info);
        }
    } else {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGlet(node *arg_node, info *arg_info)
 *
 *   @brief traverses in expression and checks assigned ids for constant
 *          value and sets corresponding AVIS_SSACONST attribute
 *
 *   @param  node *arg_node:  N_let
 *           info *arg_info:  N_info
 *   @return node *        :  N_let
 ******************************************************************************/
node *
WLPGlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("WLPGlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "N_let with empty EXPR attribute.");

    INFO_WLPG_LET (arg_info) = arg_node;

    /*
     * Only ids nodes with one entry are considered.
     * Tuple of constants are not provided/supported in SaC until now.
     */
    if ((LET_IDS (arg_node) != NULL) && (IDS_NEXT (LET_IDS (arg_node)) == NULL)) {

        if (AVIS_SSACONST (IDS_AVIS (LET_IDS (arg_node))) == NULL) {
            AVIS_SSACONST (IDS_AVIS (LET_IDS (arg_node)))
              = COAST2Constant (LET_EXPR (arg_node));
        }
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGap(node *arg_node, info *arg_info)
 *
 *   @brief traverse in the fundef of the ap node if special function
 *          and current compiler phase is sacopt
 *
 *   @param  node *arg_node:  N_ap
 *           info *arg_info:  info
 *   @return node *        :  N_ap
 ******************************************************************************/
node *
WLPGap (node *arg_node, info *arg_info)
{
    info *tmp;

    DBUG_ENTER ("WLPGap");

    if ((INFO_WLPG_SUBPHASE (arg_info) == SP_func)
        && (FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))) {
        /*
         * special functions must be traversed when they are used
         */
        if (AP_FUNDEF (arg_node) != INFO_WLPG_FUNDEF (arg_info)) {

            /* stack arg_info */
            tmp = arg_info;
            arg_info = MakeInfo ();
            /* take current flag SUBPHASE */
            INFO_WLPG_SUBPHASE (arg_info) = INFO_WLPG_SUBPHASE (tmp);

            AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);

            arg_info = FreeInfo (arg_info);
            arg_info = tmp;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGNwith(node *arg_node, info *arg_info)
 *
 *   @brief  start traversal of this WL and store information in new arg_info
 *           node. The only N_Npart node (inclusive body) is traversed.
 *           Afterwards, if certain conditions are fulfilled,
 *           the WL is transformed into a WL with generators describing a full
 *           partition.
 *
 *   @param  node *arg_node:  N_Nwith
 *           info *arg_info:  N_info
 *   @return node *        :  N_Nwith
 ******************************************************************************/

node *
WLPGNwith (node *arg_node, info *arg_info)
{
    node *let_tmp;
    bool replace_wl = FALSE;

    DBUG_ENTER ("WLPGNwith");

    /*
     * Information about last N_let has to be stacked before traversal
     */
    let_tmp = INFO_WLPG_LET (arg_info);

    /*
     * The CODEs have to be traversed as they may contain further (nested) WLs
     * and I want to modify bottom up.
     */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    /* Pop N_let */
    INFO_WLPG_LET (arg_info) = let_tmp;

    /*
     * Once a full partition has been created, we do not need to inspect
     * the generators anymore. The indicator for this is the PARTS attribute.
     * Initially, it is set to -1; if we have successfully generated a full
     * partition, it carries a positive value.
     */
    if (NWITH_PARTS (arg_node) == -1) {

        /*
         * initialize WL traversal
         */
        INFO_WLPG_WL (arg_info) = arg_node; /* store the current node for later */

        /*
         * traverse the one and only (!) PART.
         * Besides minor changes in the generator, two values are computed
         * during this traversal:
         *
         *  INFO_WLPG_GENVAR(arg_info) and INFO_WLPG_GENPROP(arg_info) !!
         */
        DBUG_ASSERT ((NWITH_PART (arg_node) != NULL),
                     "NWITH_PARTS is -1 although no PART is available!");
        DBUG_ASSERT ((NPART_NEXT (NWITH_PART (arg_node)) == NULL),
                     "NWITH_PARTS is -1 although more than one PART is available!");
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

        /*
         * Now, we traverse the WITHOP sons for propagating (structural) constants.
         */
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

        if (INFO_WLPG_GENPROP (arg_info) == GPT_empty) {
            arg_node = CreateEmptyGenWLReplacement (arg_node, arg_info);
            replace_wl = TRUE;

        } else if (INFO_WLPG_GENPROP (arg_info) == GPT_full) {
            NWITH_PARTS (arg_node) = 1;

            if (NWITH_TYPE (arg_node) == WO_genarray) {
                if (NWITH_DEFAULT (arg_node) != NULL) {
                    NWITH_DEFAULT (arg_node) = FreeTree (NWITH_DEFAULT (arg_node));
                }
            }

        } else if ((INFO_WLPG_GENPROP (arg_info) == GPT_partial)
                   && ((INFO_WLPG_GENSHP (arg_info) == GV_constant)
                       || (INFO_WLPG_GENSHP (arg_info) == GV_struct_constant))) {
            arg_node = CreateFullPartition (arg_node, arg_info);
        }

    } else {
        /*
         * traverse the WITHOP sons for propagating (structural) constants.
         */
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    INFO_WLPG_WL (arg_info) = NULL;
    INFO_WLPG_LET (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGNwithop( node *arg_node, info *arg_info)
 *
 *   @brief Substitutes NWITHOP_SHAPE into the N_Nwithop node.
 *
 *   @param  node *arg_node:  N_Nwithop
 *           info *arg_info:  N_info
 *   @return node *        :  N_Nwithop
 ******************************************************************************/

node *
WLPGNwithop (node *arg_node, info *arg_info)
{
    node *nassigns, *f_def;
    gen_shape_t current_shape;

    DBUG_ENTER ("WLPGNwithop");

    f_def = INFO_WLPG_FUNDEF (arg_info);

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        if (NWITHOP_SHAPE (arg_node) != NULL) {
            NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        }
        current_shape = PropagateArrayConstants (&(NWITHOP_SHAPE (arg_node)));
        if (current_shape == GV_known_shape) {
            nassigns = CreateNewAssigns (NWITHOP_SHAPE (arg_node), f_def);
            NWITHOP_SHAPE (arg_node)
              = CreateStructConstant (NWITHOP_SHAPE (arg_node), nassigns);
            current_shape = GV_struct_constant;
            INFO_WLPG_NASSIGNS (arg_info)
              = AppendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
        }

        if (INFO_WLPG_GENSHP (arg_info) < current_shape) {
            INFO_WLPG_GENSHP (arg_info) = current_shape;
        }
        break;

    case WO_modarray:
        if (NWITHOP_ARRAY (arg_node) != NULL) {
            NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        }
        break;

    case WO_foldfun:
        /* here is no break missing */
    case WO_foldprf:
        if (NWITHOP_NEUTRAL (arg_node) != NULL) {
            NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        }
        break;

    case WO_unknown:
        DBUG_ASSERT ((0), "Unknown withop type found!");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGNpart(node *arg_node, info *arg_info)
 *
 *   @brief traverse only generator to propagate arrays
 *
 *   @param  node *arg_node:  N_Npart
 *           info *arg_info:  N_info
 *   @return node *        :  N_Npart
 ******************************************************************************/

node *
WLPGNpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPGNpart");

    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGNgenerator(node *arg_node, info *arg_info)
 *
 *   @brief  bounds, step and width vectors are substituted into the generator.
 *           Generators that surmount the array bounds (like [-5,3] or
 *           [11,10] > [maxdim1,maxdim2] = [10,10]) are changed to fitting
 *           gens.
 *           Via INFO_WLPG_GENPROP( arg_info) the status of the generator is
 *           returned. Possible values are (poss. ambiguities are resolved top
 *           to bottom):
 *           GPT_empty   : the generator is empty!
 *           GPT_full    : the generator covers the entire range!
 *           GPT_partial : the generator has constant upper and lower bounds,
 *                         but  - most likely  - only a part is covered!
 *           GPT_unknown : we don't know anything !
 *
 *           Via INFO_WLPG_GENSHP( arg_info) the status of the bounds, step and
 *           width vectors is returned.
 *           Possible values are (poss. ambiguities are resolved top
 *           to bottom):
 *           GV_constant        : the vectors are constant!
 *           GV_struct_constant : the vektors are at least structural constants!
 *           GV_unknown_shape   : we don't know anything !
 *
 *   @param  node *arg_node:  N_Ngenerator
 *           info *arg_info:  N_info
 *   @return node *        :  N_Ngenerator
 ******************************************************************************/

node *
WLPGNgenerator (node *arg_node, info *arg_info)
{
    node *wln, *f_def, *nassigns;
    ids *let_ids;
    shape *shp;
    ntype *type;
    bool check_bounds, check_stepwidth;
    gen_prop_t gprop;
    gen_shape_t current_shape, gshape;

    DBUG_ENTER ("WLPGNgenerator");

    wln = INFO_WLPG_WL (arg_info);
    f_def = INFO_WLPG_FUNDEF (arg_info);

    /*
     * First, we try to propagate (structural) constants into all sons:
     */
    current_shape = PropagateArrayConstants (&(NGEN_BOUND1 (arg_node)));
    if (current_shape == GV_known_shape) {
        nassigns = CreateNewAssigns (NGEN_BOUND1 (arg_node), f_def);
        NGEN_BOUND1 (arg_node) = CreateStructConstant (NGEN_BOUND1 (arg_node), nassigns);
        gshape = GV_struct_constant;
        INFO_WLPG_NASSIGNS (arg_info)
          = AppendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
    } else {
        gshape = current_shape;
    }

    current_shape = PropagateArrayConstants (&(NGEN_BOUND2 (arg_node)));
    if (current_shape == GV_known_shape) {
        nassigns = CreateNewAssigns (NGEN_BOUND2 (arg_node), f_def);
        NGEN_BOUND2 (arg_node) = CreateStructConstant (NGEN_BOUND2 (arg_node), nassigns);
        current_shape = GV_struct_constant;
        INFO_WLPG_NASSIGNS (arg_info)
          = AppendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }
    check_bounds = (gshape == GV_constant);

    current_shape = PropagateArrayConstants (&(NGEN_STEP (arg_node)));
    if (current_shape == GV_known_shape) {
        nassigns = CreateNewAssigns (NGEN_STEP (arg_node), f_def);
        NGEN_STEP (arg_node) = CreateStructConstant (NGEN_STEP (arg_node), nassigns);
        current_shape = GV_struct_constant;
        INFO_WLPG_NASSIGNS (arg_info)
          = AppendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }

    check_stepwidth = (current_shape <= GV_struct_constant);

    current_shape = PropagateArrayConstants (&(NGEN_WIDTH (arg_node)));
    if (current_shape == GV_known_shape) {
        nassigns = CreateNewAssigns (NGEN_WIDTH (arg_node), f_def);
        NGEN_WIDTH (arg_node) = CreateStructConstant (NGEN_WIDTH (arg_node), nassigns);
        current_shape = GV_struct_constant;
        INFO_WLPG_NASSIGNS (arg_info)
          = AppendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }

    check_stepwidth = (check_stepwidth && (current_shape <= GV_struct_constant));

    /**
     * find out the generator properties:
     */
    let_ids = LET_IDS (INFO_WLPG_LET (arg_info));
    type = AVIS_TYPE (IDS_AVIS (let_ids));

    if (TYIsAKS (type)) {
        shp = TYGetShape (type);
        if (check_bounds
            && ((NWITH_TYPE (wln) == WO_modarray) || (NWITH_TYPE (wln) == WO_genarray))) {
            wln = CropBounds (wln, shp);
        }
        gprop = ComputeGeneratorProperties (wln, shp);
    } else {
        gprop = ComputeGeneratorProperties (wln, NULL);
    }

    if (gshape == GV_struct_constant) {
        if (NWITH_IS_FOLD (wln))
            gprop = GPT_full;
        else
            gprop = GPT_partial;
    }

    if (check_stepwidth) {
        /* normalize step and width */
        switch (NormalizeStepWidth (&NGEN_STEP (arg_node), &NGEN_WIDTH (arg_node))) {
        case 1:
            ABORT (NODE_LINE (wln), ("component of width greater than step"));
        case 2:
            ABORT (NODE_LINE (wln), ("component of width less 0"));
        case 3:
            ABORT (NODE_LINE (wln), ("width vector without step vector"));
        }
    }

    INFO_WLPG_GENPROP (arg_info) = gprop;
    INFO_WLPG_GENSHP (arg_info) = gshape;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPartitionGeneration( node *arg_node)
 *
 *   @brief  Starting point for the partition generation if it was called
 *           from main.
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
WLPartitionGeneration (node *arg_node)
{
    funtab *tmp_tab;
    info *arg_info;

    DBUG_ENTER ("WLPartitionGeneration");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "WLPartitionGeneration not started with modul node");

    DBUG_PRINT ("WLPG", ("starting WLPartitionGeneration"));

    arg_info = MakeInfo ();
    INFO_WLPG_SUBPHASE (arg_info) = SP_cf;

    tmp_tab = act_tab;
    act_tab = wlpg_tab;

    arg_node = Trav (arg_node, arg_info);

    arg_info = FreeInfo (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPartitionGenerationOPT( node *arg_node)
 *
 *   @brief  Starting point for the partition generation if it was called
 *           from optimize.
 *
 *   @param  node *arg_node:  N_fundef
 *   @return node *        :  transformed N_fundef
 ******************************************************************************/

node *
WLPartitionGenerationOPT (node *arg_node)
{
    funtab *tmp_tab;
    info *arg_info;

    DBUG_ENTER ("WLPartitionGenerationOPT");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "WLPartitionGenerationOPT not started with fundef node");

    DBUG_PRINT ("WLPG", ("starting WLPartitionGenerationOPT"));

    arg_info = MakeInfo ();
    INFO_WLPG_SUBPHASE (arg_info) = SP_func;

    tmp_tab = act_tab;
    act_tab = wlpg_tab;

    arg_node = Trav (arg_node, arg_info);

    arg_info = FreeInfo (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}
