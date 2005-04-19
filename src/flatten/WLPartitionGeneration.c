/*
 *
 * $Log$
 * Revision 1.39  2005/04/19 17:11:37  ktr
 * Corrected a type access error.
 *
 * Revision 1.38  2005/04/15 08:47:35  ktr
 * replaced TYcopyType with TYeliminateAKV
 *
 * Revision 1.37  2005/03/19 23:11:53  sbs
 * AUD support added; default partitions are eliminated if a WL is identified as nonAUD.
 *
 * Revision 1.36  2005/01/26 10:32:10  mwe
 * only edit last log message ...
 *
 * Revision 1.35  2005/01/26 10:24:38  mwe
 * AVIS_SSACONST removed and replaced by usage of akv types
 *
 * Revision 1.34  2005/01/11 11:19:19  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.33  2004/12/16 17:47:10  ktr
 * TYisAKV inserted.
 *
 * Revision 1.32  2004/12/16 14:10:07  khf
 * added check on AKVs
 *
 * Revision 1.31  2004/12/10 17:40:37  khf
 * corrected test on shape extent
 *
 * Revision 1.30  2004/12/09 16:57:20  sbs
 * calls to TBmakePart adjusted.
 *
 * Revision 1.29  2004/12/08 17:59:15  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.28  2004/12/07 20:32:19  ktr
 * eliminated CONSTVEC which is superseded by ntypes.
 *
 * Revision 1.27  2004/11/27 00:41:57  khf
 * adjusted startfunctions
 *
 * Revision 1.26  2004/11/26 20:57:34  khf
 * corrected function names fron CF
 *
 * Revision 1.25  2004/11/25 23:28:04  khf
 * ccorrected error message :)
 *
 * Revision 1.24  2004/11/24 17:32:14  khf
 * SacDevCamp04
 *
 * Revision 1.23  2004/11/24 13:29:55  khf
 * SacDevCamp04: Compiles
 *
 * Revision 1.22  2004/10/22 10:29:32  khf
 * fixed bug 73
 *
 * Revision 1.21  2004/10/07 12:12:45  sah
 * added NCODE_INC_USED macro
 *
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
 * ATTENTION: This travesal works completly with new types
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "new_types.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "shape.h"
#include "free.h"
#include "DupTree.h"
#include "ctinfo.h"
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
    node *module;
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
#define INFO_WLPG_MODULE(n) (n->module)
#define INFO_WLPG_GENPROP(n) (n->genprob)
#define INFO_WLPG_GENSHP(n) (n->genshp)
#define INFO_WLPG_SUBPHASE(n) (n->subphase)

#define AtLeastAKD(type) ((TYisAKV (type) || TYisAKS (type) || TYisAKD (type)))

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_WLPG_WL (result) = NULL;
    INFO_WLPG_FUNDEF (result) = NULL;
    INFO_WLPG_LET (result) = NULL;
    INFO_WLPG_NASSIGNS (result) = NULL;
    INFO_WLPG_MODULE (result) = NULL;
    INFO_WLPG_GENPROP (result) = 0;
    INFO_WLPG_GENSHP (result) = 0;
    INFO_WLPG_SUBPHASE (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
    int dim = 0;

    DBUG_ENTER ("CreateStructConstant");

    DBUG_ASSERT ((nassigns != NULL), "NASSIGNS is empty");

    tmp1 = NULL;
    tmp2 = NULL;
    iterator = nassigns;

    while (iterator) {
        idn = DUPdupIdsId (LET_IDS (ASSIGN_INSTR (iterator)));

        if (tmp1) {
            EXPRS_NEXT (tmp2) = TBmakeExprs (idn, NULL);
            tmp2 = EXPRS_NEXT (tmp2);
        } else {
            tmp1 = TBmakeExprs (idn, tmp1);
            tmp2 = tmp1;
        }

        iterator = ASSIGN_NEXT (iterator);
        dim++;
    }

    tmp1 = TCmakeFlatArray (tmp1);

    if (expr != NULL) {
        expr = FREEdoFreeTree (expr);
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
    node *nassigns, *vardec, *tmp1, *tmp2, *_ids;
    char *nvarname;
    int i;

    DBUG_ENTER ("CreateIdxShapeSelAssigns");

    DBUG_ASSERT ((array != NULL), "array is empty");
    DBUG_ASSERT ((NODE_TYPE (array) == N_id),
                 "CreateIdxShapeSelAssigns not called with N_id");
    DBUG_ASSERT ((end >= begin), "illegal length found!");

    nassigns = NULL;

    for (i = end; i >= begin; i--) {
        nvarname = ILIBtmpVarName (ID_NAME (array));
        _ids = TBmakeIds (TBmakeAvis (nvarname, TYmakeAKS (TYmakeSimpleType (T_int),
                                                           SHmakeShape (0))),
                          NULL);

        vardec = TBmakeVardec (IDS_AVIS (_ids), NULL);

        fundef = TCaddVardecs (fundef, vardec);

        /* index position for selection */
        tmp1 = TBmakeNum (i);
        /* the array for selection */
        tmp2 = TBmakeExprs (DUPdoDupTree (array), NULL);
        tmp1 = TBmakePrf (F_idx_shape_sel, TBmakeExprs (tmp1, tmp2));

        nassigns = TBmakeAssign (TBmakeLet (_ids, tmp1), nassigns);

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
    node *tmp1, *tmp2, *tmp3, *nassigns, *vardec, *_ids;
    char *nvarname;

    DBUG_ENTER ("CreateNewAssigns");

    DBUG_ASSERT ((expr != NULL), "Expr is empty");
    DBUG_ASSERT ((NODE_TYPE (expr) == N_id), "CreateNewAssigns not called with N_id");
    DBUG_ASSERT ((TYisSimple (ID_NTYPE (expr)) == FALSE), "Id is a Scalar!!");

    nassigns = NULL;

    for (i = SHgetExtent (TYgetShape (ID_NTYPE (expr)), 0) - 1; i >= 0; i--) {
        nvarname = ILIBtmpVarName (ID_NAME (expr));
        _ids = TBmakeIds (TBmakeAvis (nvarname, TYmakeAKS (TYmakeSimpleType (T_int),
                                                           SHmakeShape (0))),
                          NULL);

        vardec = TBmakeVardec (IDS_AVIS (_ids), NULL);

        fundef = TCaddVardecs (fundef, vardec);

        /* index position for selection */
        tmp1 = TBmakeNum (i);
        /* the array for selection */
        tmp2 = TBmakeExprs (DUPdoDupNode (expr), NULL);

        tmp3 = TBmakePrf (F_idx_sel, TBmakeExprs (tmp1, tmp2));

        nassigns = TBmakeAssign (TBmakeLet (_ids, tmp3), nassigns);

        /* set correct backref to defining assignment */
        AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassigns;
    }

    DBUG_RETURN (nassigns);
}

/** <!--********************************************************************-->
 *
 * @fn node *NewIds( node *nd, node *fundef)
 *
 *   @brief creates new IDS.
 *
 *   @param  node *nd       :  if N_id node the new name and type will be
 *                             infered from
 *           node *fundef   :  N_fundef
 *   @return node *         :  new Ids
 ******************************************************************************/
static node *
NewIds (node *nd, node *fundef)
{
    node *vardec, *_ids;
    char *nvarname;

    DBUG_ENTER ("NewIds");

    DBUG_ASSERT ((nd != NULL) && ((NODE_TYPE (nd) == N_id) || (NODE_TYPE (nd) == N_num)),
                 "ID is empty or not N_id/N_num");

    if (NODE_TYPE (nd) == N_id) {
        nvarname = ILIBtmpVarName (ID_NAME (nd));
        _ids
          = TBmakeIds (TBmakeAvis (nvarname, TYeliminateAKV (AVIS_TYPE (ID_AVIS (nd)))),
                       NULL);

        vardec = TBmakeVardec (IDS_AVIS (_ids), NULL);

    } else {
        nvarname = ILIBtmpVar ();
        _ids = TBmakeIds (TBmakeAvis (nvarname, TYmakeAKS (TYmakeSimpleType (T_int),
                                                           SHmakeShape (0))),
                          NULL);

        vardec = TBmakeVardec (IDS_AVIS (_ids), NULL);
    }

    fundef = TCaddVardecs (fundef, vardec);

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
        tmp = TBmakeExprs (TBmakeNum (entry), tmp);
    }
    tmp = TCmakeFlatArray (tmp);

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
    genn = TBmakeGenerator (F_le, F_lt, DUPdoDupTree (lb), DUPdoDupTree (ub),
                            DUPdoDupTree (step), DUPdoDupTree (width));
    partn = TBmakePart (coden, DUPdoDupTree (withid), genn);
    CODE_INC_USED (coden);

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
            veclen = TCcountExprs (ARRAY_AELEMS ((*step)));

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
                    EXPRS_EXPR (stp) = FREEdoFreeTree (EXPRS_EXPR (stp));
                    EXPRS_EXPR (stp) = TBmakeNum (1);
                    EXPRS_EXPR (wth) = FREEdoFreeTree (EXPRS_EXPR (wth));
                    EXPRS_EXPR (wth) = TBmakeNum (1);
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
            (*step) = FREEdoFreeTree (*step);
            (*width) = FREEdoFreeTree (*width);
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

    parts = WITH_PART (wln);
    /* at least one part exists */
    no_parts = 1;

    while (PART_NEXT (parts)) {
        parts = PART_NEXT (parts);
        no_parts++;
    }

    PART_NEXT (parts) = partn;
    no_parts++;
    WITH_PARTS (wln) = no_parts;

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
    lsc = DUPdoDupTree (ls);
    usc = DUPdoDupTree (us);

    le = ARRAY_AELEMS (l);
    lsce = ARRAY_AELEMS (lsc);
    ue = ARRAY_AELEMS (u);
    usce = ARRAY_AELEMS (usc);

    withidn = DUPdoDupTree (PART_WITHID (WITH_PART (wln)));

    for (d = 0; d < dim; d++) {
        /* Check whether there is a cuboid above (below) the given one. */

        if (NODE_TYPE (EXPRS_EXPR (le)) == N_num) {
            lnum = NUM_VAL (EXPRS_EXPR (le));
            lscnum = NUM_VAL (EXPRS_EXPR (lsce));
            if (lnum > lscnum) {
                partn = CreateNewPart (lsc, usc, NULL, NULL, withidn, coden);
                ubn = ARRAY_AELEMS (PART_BOUND2 (partn));
                for (i = 0; i < d; i++) {
                    ubn = EXPRS_NEXT (ubn);
                }
                if (NODE_TYPE (EXPRS_EXPR (ubn)) == N_num) {
                    NUM_VAL (EXPRS_EXPR (ubn)) = lnum;
                } else {
                    EXPRS_EXPR (ubn) = FREEdoFreeTree (EXPRS_EXPR (ubn));
                    EXPRS_EXPR (ubn) = DUPdoDupTree (EXPRS_EXPR (le));
                }
                wln = AppendPart2WL (wln, partn);
            }
        } else {
            partn = CreateNewPart (lsc, usc, NULL, NULL, withidn, coden);
            ubn = ARRAY_AELEMS (PART_BOUND2 (partn));
            for (i = 0; i < d; i++) {
                ubn = EXPRS_NEXT (ubn);
            }
            EXPRS_EXPR (ubn) = FREEdoFreeTree (EXPRS_EXPR (ubn));
            EXPRS_EXPR (ubn) = DUPdoDupTree (EXPRS_EXPR (le));
            wln = AppendPart2WL (wln, partn);
        }

        if ((NODE_TYPE (EXPRS_EXPR (ue)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (usce)) == N_num)) {
            unum = NUM_VAL (EXPRS_EXPR (ue));
            uscnum = NUM_VAL (EXPRS_EXPR (usce));
            if (unum < uscnum) {
                partn = CreateNewPart (lsc, usc, NULL, NULL, withidn, coden);
                lbn = ARRAY_AELEMS (PART_BOUND1 (partn));
                for (i = 0; i < d; i++) {
                    lbn = EXPRS_NEXT (lbn);
                }
                NUM_VAL (EXPRS_EXPR (lbn)) = unum;
                wln = AppendPart2WL (wln, partn);
            }
        } else {
            partn = CreateNewPart (lsc, usc, NULL, NULL, withidn, coden);
            lbn = ARRAY_AELEMS (PART_BOUND1 (partn));
            for (i = 0; i < d; i++) {
                lbn = EXPRS_NEXT (lbn);
            }
            EXPRS_EXPR (lbn) = FREEdoFreeTree (EXPRS_EXPR (lbn));
            EXPRS_EXPR (lbn) = DUPdoDupTree (EXPRS_EXPR (ue));
            wln = AppendPart2WL (wln, partn);
        }

        /* and modifiy array bounds to continue with next dimension */
        if (NODE_TYPE (EXPRS_EXPR (le)) == N_num) {
            NUM_VAL (EXPRS_EXPR (lsce)) = NUM_VAL (EXPRS_EXPR (le));
        } else {
            EXPRS_EXPR (lsce) = FREEdoFreeTree (EXPRS_EXPR (lsce));
            EXPRS_EXPR (lsce) = DUPdoDupTree (EXPRS_EXPR (le));
        }

        if ((NODE_TYPE (EXPRS_EXPR (ue)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (usce)) == N_num)) {
            NUM_VAL (EXPRS_EXPR (usce)) = NUM_VAL (EXPRS_EXPR (ue));
        } else {
            EXPRS_EXPR (usce) = FREEdoFreeTree (EXPRS_EXPR (usce));
            EXPRS_EXPR (usce) = DUPdoDupTree (EXPRS_EXPR (ue));
        }

        le = EXPRS_NEXT (le);
        lsce = EXPRS_NEXT (lsce);
        ue = EXPRS_NEXT (ue);
        usce = EXPRS_NEXT (usce);
    }

    lsc = FREEdoFreeTree (lsc);
    usc = FREEdoFreeTree (usc);
    withidn = FREEdoFreeTree (withidn);

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
    node *nw, *stpe, *wthe, *nwe, *partn, *withidn, *lbn, *wthn, *tmp1, *tmp2, *nassign,
      *_ids;
    int i, d, stpnum, wthnum;

    DBUG_ENTER ("CompleteGrid");

    /* create local copies of the arrays which atr modified here*/
    nw = DUPdoDupTree (step);

    stpe = ARRAY_AELEMS (step);
    wthe = ARRAY_AELEMS (width);
    nwe = ARRAY_AELEMS (nw);

    withidn = DUPdoDupTree (PART_WITHID (WITH_PART (wln)));

    for (d = 0; d < dim; d++) {

        if ((NODE_TYPE (EXPRS_EXPR (stpe)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (wthe)) == N_num)) {
            stpnum = NUM_VAL (EXPRS_EXPR (stpe));
            wthnum = NUM_VAL (EXPRS_EXPR (wthe));

            if (stpnum > wthnum) { /* create new grids */

                partn = CreateNewPart (ls, us, step, nw, withidn, coden);
                lbn = ARRAY_AELEMS (PART_BOUND1 (partn));
                wthn = ARRAY_AELEMS (PART_WIDTH (partn));

                for (i = 0; i < d; i++) {
                    wthn = EXPRS_NEXT (wthn);
                    lbn = EXPRS_NEXT (lbn);
                }

                if (NODE_TYPE (EXPRS_EXPR (lbn)) == N_num) {
                    NUM_VAL (EXPRS_EXPR (lbn)) = NUM_VAL (EXPRS_EXPR (lbn)) + wthnum;
                } else {

                    _ids = NewIds (EXPRS_EXPR (lbn), INFO_WLPG_FUNDEF (arg_info));

                    /* the identifier to add wthnum to */
                    tmp1 = DUPdoDupTree (EXPRS_EXPR (lbn));
                    /* N_num wthnum */
                    tmp2 = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (wthe)), NULL);

                    tmp1 = TBmakePrf (F_add_SxS, TBmakeExprs (tmp1, tmp2));

                    nassign = TBmakeAssign (TBmakeLet (_ids, tmp1), NULL);
                    /* set correct backref to defining assignment */
                    AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

                    INFO_WLPG_NASSIGNS (arg_info)
                      = TCappendAssign (INFO_WLPG_NASSIGNS (arg_info), nassign);

                    EXPRS_EXPR (lbn) = FREEdoFreeTree (EXPRS_EXPR (lbn));
                    EXPRS_EXPR (lbn) = DUPdupIdsId (_ids);
                }
                NUM_VAL (EXPRS_EXPR (wthn)) = stpnum - wthnum;
                i = NormalizeStepWidth (&(PART_STEP (partn)), &(PART_WIDTH (partn)));
                DBUG_ASSERT (!i, ("internal normalization failure"));

                wln = AppendPart2WL (wln, partn);
            }
        } else {
            partn = CreateNewPart (ls, us, step, nw, withidn, coden);
            lbn = ARRAY_AELEMS (PART_BOUND1 (partn));
            wthn = ARRAY_AELEMS (PART_WIDTH (partn));

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
                tmp1 = DUPdoDupTree (EXPRS_EXPR (lbn));
                /* current width as N_num or N_id */
                tmp2 = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (wthe)), NULL);

                tmp1 = TBmakePrf (F_add_SxS, TBmakeExprs (tmp1, tmp2));

                nassign = TBmakeAssign (TBmakeLet (_ids, tmp1), NULL);
                /* set correct backref to defining assignment */
                AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

                INFO_WLPG_NASSIGNS (arg_info)
                  = TCappendAssign (INFO_WLPG_NASSIGNS (arg_info), nassign);

                EXPRS_EXPR (lbn) = FREEdoFreeTree (EXPRS_EXPR (lbn));
                EXPRS_EXPR (lbn) = DUPdupIdsId (_ids);
            }

            if ((NODE_TYPE (EXPRS_EXPR (stpe)) == N_num)
                && (NODE_TYPE (EXPRS_EXPR (wthe)) == N_num)) {
                NUM_VAL (EXPRS_EXPR (wthn))
                  = NUM_VAL (EXPRS_EXPR (stpe)) - NUM_VAL (EXPRS_EXPR (wthe));
            } else {

                _ids = NewIds (EXPRS_EXPR (wthn), INFO_WLPG_FUNDEF (arg_info));

                /* the first identifier */
                tmp1 = DUPdoDupTree (EXPRS_EXPR (stpe));
                /* the second identifier */
                tmp2 = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (wthe)), NULL);

                tmp1 = TBmakePrf (F_sub_SxS, TBmakeExprs (tmp1, tmp2));

                nassign = TBmakeAssign (TBmakeLet (_ids, tmp1), NULL);
                /* set correct backref to defining assignment */
                AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;

                INFO_WLPG_NASSIGNS (arg_info)
                  = TCappendAssign (INFO_WLPG_NASSIGNS (arg_info), nassign);

                EXPRS_EXPR (wthn) = FREEdoFreeTree (EXPRS_EXPR (wthn));
                EXPRS_EXPR (wthn) = DUPdupIdsId (_ids);
            }

            i = NormalizeStepWidth (&(PART_STEP (partn)), &(PART_WIDTH (partn)));
            DBUG_ASSERT (!i, ("internal normalization failure"));

            wln = AppendPart2WL (wln, partn);
        }

        /* and modifiy array bounds to continue with next dimension */
        if ((NODE_TYPE (EXPRS_EXPR (nwe)) == N_num)
            && (NODE_TYPE (EXPRS_EXPR (wthe)) == N_num)) {
            NUM_VAL (EXPRS_EXPR (nwe)) = NUM_VAL (EXPRS_EXPR (wthe));
        } else {
            EXPRS_EXPR (nwe) = FREEdoFreeTree (EXPRS_EXPR (nwe));
            EXPRS_EXPR (nwe) = DUPdoDupTree (EXPRS_EXPR (wthe));
        }

        stpe = EXPRS_NEXT (stpe);
        wthe = EXPRS_NEXT (wthe);
        nwe = EXPRS_NEXT (nwe);
    }

    nw = FREEdoFreeTree (nw);

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
    node *vec_ids;
    node *scl_ids = NULL;
    node *tmp_ids;
    int i;

    DBUG_ENTER ("CreateScalarWL");

    DBUG_ASSERT ((dim >= 0), "CreateScalarWl() used with unknown shape!");

    vec_ids = TBmakeIds (TBmakeAvis (ILIBtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                               SHcreateShape (1, dim))),
                         NULL);

    vardecs = TBmakeVardec (IDS_AVIS (vec_ids), vardecs);

    for (i = 0; i < dim; i++) {
        tmp_ids
          = TBmakeIds (TBmakeAvis (ILIBtmpVar (),
                                   TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0))),
                       NULL);

        vardecs = TBmakeVardec (IDS_AVIS (tmp_ids), vardecs);
        IDS_NEXT (tmp_ids) = scl_ids;
        scl_ids = tmp_ids;
    }

    id = TBmakeId (
      TBmakeAvis (ILIBtmpVar (), TYmakeAKS (TYmakeSimpleType (btype), SHmakeShape (0))));
    vardecs = TBmakeVardec (ID_AVIS (id), vardecs);

    wl
      = TBmakeWith (TBmakePart (NULL, TBmakeWithid (vec_ids, scl_ids),
                                TBmakeGenerator (F_le, F_lt,
                                                 TCcreateZeroVector (dim, T_int),
                                                 DUPdoDupNode (array_shape), NULL, NULL)),
                    TBmakeCode (TBmakeBlock (TCmakeAssignLet (ID_AVIS (id), expr), NULL),
                                TBmakeExprs (id, NULL)),
                    TBmakeGenarray (DUPdoDupNode (array_shape), NULL));
    CODE_USED (WITH_CODE (wl))++;
    PART_CODE (WITH_PART (wl)) = WITH_CODE (wl);
    WITH_PARTS (wl) = 1;

    fundef = TCaddVardecs (fundef, vardecs);

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
    dim = TYgetDim (array_type);
    DBUG_ASSERT ((TYisSimple (array_type) == FALSE), "N_id is no array type!");
    btype = TYgetSimpleType (TYgetScalar (array_type));
    shape = TYgetShape (array_type);

    if (dim == 0) {
        zero = TCcreateZeroScalar (btype);
    } else {
        node *array_shape = NULL;

        if (TYisAKV (array_type) || TYisAKS (array_type)) {

            array_shape = SHshape2Array (shape);
        } else { /* AKD array */
            assigns = CreateIdxShapeSelAssigns (array, 0, (dim - 1), fundef);
            array_shape = CreateStructConstant (array_shape, assigns);
        }
        zero
          = CreateScalarWL (dim, array_shape, btype, TCcreateZeroScalar (btype), fundef);
        array_shape = FREEdoFreeNode (array_shape);
    }

    (*nassigns) = assigns;
    DBUG_RETURN (zero);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateArraySel( node *sel_vec, node *sel_ids, node *sel_array,
 *                           node **nassigns, node *fundef)
 *
 *   @brief creates an WL with element-wise reference on 'sel_array'
 *
 *   @param  node *sel_vec   : N_WITHID_VEC of current WL
 *           node *sel_ids   : N_WITHID_IDS of current WL
 *           node *sel_array :
 *           node **nassigns : != NULL iff new assignments have been created
 *                             (AKD case)
 *           node *fundef    : N_FUNDEF
 *   @return node *          : N_Nwith
 ******************************************************************************/
static node *
CreateArraySel (node *sel_vec, node *sel_ids, node *sel_array, node **nassigns,
                node *fundef)
{
    node *sel;
    node *assigns = NULL;
    int len_index, dim_array;

    DBUG_ENTER ("CreateArraySel");

    DBUG_ASSERT ((NODE_TYPE (sel_array) == N_id), "no N_id node found!");

    len_index = SHgetExtent (TYgetShape (IDS_NTYPE (sel_vec)), 0);
    DBUG_ASSERT ((len_index > 0), "illegal index length found!");

    dim_array = TYgetDim (ID_NTYPE (sel_array));
    DBUG_ASSERT ((dim_array > 0), "illegal array dimensionality found!");

    if (len_index > dim_array) {
        DBUG_ASSERT ((0), "illegal array selection found!");
        sel = NULL;
    } else if ((len_index == dim_array)) {
        sel
          = TBmakePrf (F_sel, TBmakeExprs (DUPdupIdsId (sel_vec),
                                           TBmakeExprs (DUPdoDupNode (sel_array), NULL)));
    } else { /* (len_index < dim_array) */
        node *new_index, *id, *vardec, *ass, *tmp_ids, *avis;
        node *array_shape = NULL;
        shape *shape, *mshape;
        simpletype btype;
        ntype *array_type;
        int dim;

        array_type = ID_NTYPE (sel_array);
        dim = (dim_array - len_index);

        btype = TYgetSimpleType (TYgetScalar (array_type));

        if (TYisAKV (array_type) || TYisAKS (array_type)) {

            shape = TYgetShape (array_type);
            mshape = SHdropFromShape (len_index, shape);
            array_shape = SHshape2Array (mshape);

            if (mshape) {
                SHfreeShape (mshape);
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

        array_shape = FREEdoFreeNode (array_shape);

        /*
         * create index vector for F_sel
         */
        tmp_ids = sel_ids;
        while (IDS_NEXT (tmp_ids) != NULL) {
            tmp_ids = IDS_NEXT (tmp_ids);
        }
        IDS_NEXT (tmp_ids) = WITH_IDS (sel); /* concat ids chains */
        new_index = TCids2Array (sel_ids);
        IDS_NEXT (tmp_ids) = NULL; /* restore ids chains */

        /*
         * create new id
         */
        avis = TBmakeAvis (ILIBtmpVar (),
                           TYmakeAKS (TYmakeSimpleType (T_int),
                                      SHcreateShape (1, TCcountIds (sel_ids))));
        id = TBmakeId (avis);
        vardec = TBmakeVardec (avis, NULL);

        fundef = TCaddVardecs (fundef, vardec);

        /*
         * create expression 'sel( tmp, A)' and insert it into WL
         */
        ASSIGN_RHS (BLOCK_INSTR (WITH_CBLOCK (sel)))
          = TBmakePrf (F_sel,
                       TBmakeExprs (id, TBmakeExprs (DUPdoDupNode (sel_array), NULL)));

        /*
         * create assignment 'tmp = [...];' and insert it into WL
         */
        ass = TCmakeAssignLet (ID_AVIS (id), new_index);
        ASSIGN_NEXT (ass) = BLOCK_INSTR (WITH_CBLOCK (sel));
        BLOCK_INSTR (WITH_CBLOCK (sel)) = ass;
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
    node *coden, *idn, *nassign, *array_shape = NULL, *array_null, *vardec, *nassigns,
                                 *_ids;
    ntype *array_type;
    shape *shape, *mshape;
    int gen_shape = 0;
    bool do_create;

    DBUG_ENTER ("CreateFullPartition");

    if (PART_NEXT (WITH_PART (wln)) != NULL) {
        /* we do have a default partition here */
        PART_NEXT (WITH_PART (wln)) = FREEdoFreeTree (PART_NEXT (WITH_PART (wln)));
    }

    do_create = TRUE;

    /* get shape of the index vector (generator) */
    gen_shape = SHgetExtent (TYgetShape (IDS_NTYPE (WITH_VEC (wln))), 0);
    DBUG_ASSERT ((gen_shape > 0), "shape of index vector has to be > 0!");

    /*
     * allocate "array_shape"
     */
    switch (NODE_TYPE (WITH_WITHOP (wln))) {
    case N_modarray: {
        /* create upper array bound */
        array_type = ID_NTYPE (MODARRAY_ARRAY (WITH_WITHOP (wln)));

        if (TYisAKV (array_type) || TYisAKS (array_type)) {

            shape = TYgetShape (array_type);
            /* only for iteration space */
            mshape = SHtakeFromShape (gen_shape, shape);
            array_shape = SHshape2Array (mshape);

            if (mshape) {
                SHfreeShape (mshape);
            }
        } else if (TYisAKD (array_type)) {

            nassigns
              = CreateIdxShapeSelAssigns (MODARRAY_ARRAY (WITH_WITHOP (wln)), 0,
                                          (gen_shape - 1), INFO_WLPG_FUNDEF (arg_info));
            array_shape = CreateStructConstant (array_shape, nassigns);
            INFO_WLPG_NASSIGNS (arg_info)
              = TCappendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
        } else {
            /* dimension unknown */
            array_shape = NULL;
        }
    } break;

    case N_genarray:
        array_shape = DUPdoDupTree (GENARRAY_SHAPE (WITH_WITHOP (wln)));
        break;

    default:
        DBUG_ASSERT ((0), "illegal WITH_TYPE found!");
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
        if (NODE_TYPE (WITH_WITHOP (wln)) == N_genarray) {
            if (GENARRAY_DEFAULT (WITH_WITHOP (wln)) == NULL) {
                coden = CreateZeros (EXPRS_EXPR (WITH_CEXPRS (wln)), &(nassigns),
                                     INFO_WLPG_FUNDEF (arg_info));
            } else {
                coden = DUPdoDupTree (GENARRAY_DEFAULT (WITH_WITHOP (wln)));
            }
        } else { /* modarray */
            coden = CreateArraySel (WITHID_VEC (WITH_WITHID (wln)),
                                    WITHID_IDS (WITH_WITHID (wln)),
                                    MODARRAY_ARRAY (WITH_WITHOP (wln)), &(nassigns),
                                    INFO_WLPG_FUNDEF (arg_info));
        }

        _ids = TBmakeIds (TBmakeAvis (ILIBtmpVar (), TYeliminateAKV (AVIS_TYPE (ID_AVIS (
                                                       EXPRS_EXPR (WITH_CEXPRS (wln)))))),
                          NULL);
        vardec = TBmakeVardec (IDS_AVIS (_ids), NULL);

        INFO_WLPG_FUNDEF (arg_info) = TCaddVardecs (INFO_WLPG_FUNDEF (arg_info), vardec);

        idn = DUPdupIdsId (_ids);

        /* create new N_code node  */
        nassign = TBmakeAssign (TBmakeLet (_ids, coden), NULL);
        /* set correct backref to defining assignment */
        AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;
        nassigns = TCappendAssign (nassigns, nassign);
        coden = TBmakeCode (TBmakeBlock (nassigns, NULL), TBmakeExprs (idn, NULL));

        /* create surrounding cuboids */
        wln = CutSlices (array_null, array_shape, WITH_BOUND1 (wln), WITH_BOUND2 (wln),
                         gen_shape, wln, coden);

        /* the original part can still be found at first position in wln.
           Now create grids. */
        if (WITH_STEP (wln))
            wln = CompleteGrid (WITH_BOUND1 (wln), WITH_BOUND2 (wln), WITH_STEP (wln),
                                WITH_WIDTH (wln), gen_shape, wln, coden, arg_info);

        /* if new codes have been created, add them to code list */
        if (PART_NEXT (WITH_PART (wln))) {
            CODE_NEXT (coden) = WITH_CODE (wln);
            WITH_CODE (wln) = coden;
        }

        /* free the above made arrays */
        array_null = FREEdoFreeTree (array_null);
    }
    array_shape = FREEdoFreeTree (array_shape);

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
    node *let_ids;
    int dim, i;
    node *lb, *ub, *lbe, *ube;
    node *code;
    node *tmpn, *assignn, *blockn, *cexpr;
    node *nassigns = NULL;
    node *_ids;

    node *res;

    DBUG_ENTER ("CreateEmptyGenWLReplacement");

    switch (NODE_TYPE (WITH_WITHOP (wl))) {
    case N_genarray:
        res = wl;
        /*
         * First, we change the generator to full scope.
         */
        let_ids = LET_IDS (INFO_WLPG_LET (arg_info));
        dim = SHgetDim (TYgetShape (AVIS_TYPE (IDS_AVIS (let_ids))));
        lb = WITH_BOUND1 (wl);
        ub = WITH_BOUND2 (wl);
        lbe = ARRAY_AELEMS (lb);
        ube = ARRAY_AELEMS (ub);

        i = 0;
        while ((i < dim) && (lbe != NULL)) {
            NUM_VAL (EXPRS_EXPR (lbe)) = 0;
            NUM_VAL (EXPRS_EXPR (ube))
              = SHgetExtent (TYgetShape (IDS_NTYPE (let_ids)), i);

            lbe = EXPRS_NEXT (lbe);
            ube = EXPRS_NEXT (ube);
            i++;
        }

        if (WITH_STEP (wl)) {
            WITH_STEP (wl) = FREEdoFreeTree (WITH_STEP (wl));
        }
        if (WITH_WIDTH (wl)) {
            WITH_WIDTH (wl) = FREEdoFreeTree (WITH_WIDTH (wl));
        }

        /*
         * Now we have to change the code. Either we can use DEFAULT (easy)
         * or we have to create zeros (ugly).
         */
        code = WITH_CODE (wl);
        if (GENARRAY_DEFAULT (WITH_WITHOP (wl)) != NULL) {
            CODE_CBLOCK (code) = FREEdoFreeTree (CODE_CBLOCK (code));
            CODE_CBLOCK (code) = TBmakeEmpty ();

            EXPRS_EXPR (CODE_CEXPRS (code))
              = FREEdoFreeTree (EXPRS_EXPR (CODE_CEXPRS (code)));
            EXPRS_EXPR (CODE_CEXPRS (code))
              = DUPdoDupTree (GENARRAY_DEFAULT (WITH_WITHOP (wl)));
        } else {
            blockn = CODE_CBLOCK (code);
            tmpn = BLOCK_INSTR (blockn);
            cexpr = EXPRS_EXPR (CODE_CEXPRS (code));

            if (N_empty == NODE_TYPE (tmpn)) {
                /* there is no instruction in the block right now. */
                _ids = NewIds (cexpr, INFO_WLPG_FUNDEF (arg_info));

                tmpn = CreateZeros (cexpr, &(nassigns), INFO_WLPG_FUNDEF (arg_info));

                /* replace N_empty with new assignment "_ids = [0,..,0]" */
                assignn = TBmakeAssign (TBmakeLet (_ids, tmpn), NULL);
                nassigns = TCappendAssign (nassigns, assignn);
                BLOCK_INSTR (blockn) = TCappendAssign (BLOCK_INSTR (blockn), nassigns);

                /* set correct backref to defining assignment */
                AVIS_SSAASSIGN (IDS_AVIS (_ids)) = assignn;

                /* replace CEXPR */
                tmpn = WITH_CODE (INFO_WLPG_WL (arg_info));
                EXPRS_EXPR (CODE_CEXPRS (tmpn))
                  = FREEdoFreeTree (EXPRS_EXPR (CODE_CEXPRS (tmpn)));
                EXPRS_EXPR (CODE_CEXPRS (tmpn)) = DUPdupIdsId (_ids);

            } else {
                /* we have a non-empty block.
                   search cexpr assignment and make it the only one in the block. */

                assignn = DUPdoDupNode (AVIS_SSAASSIGN (ID_AVIS (cexpr)));
                BLOCK_INSTR (blockn) = FREEdoFreeTree (BLOCK_INSTR (blockn));
                LET_EXPR (ASSIGN_INSTR (assignn))
                  = FREEdoFreeTree (LET_EXPR (ASSIGN_INSTR (assignn)));

                tmpn = CreateZeros (cexpr, &(nassigns), INFO_WLPG_FUNDEF (arg_info));
                CODE_VISITED (code) = TRUE;
                LET_EXPR (ASSIGN_INSTR (assignn)) = tmpn;
                nassigns = TCappendAssign (nassigns, assignn);

                BLOCK_INSTR (blockn) = nassigns;
            }
        }
        break;
    case N_modarray:
        res = DUPdoDupTree (MODARRAY_ARRAY (WITH_WITHOP (wl)));
        wl = FREEdoFreeTree (wl);
        break;
    case N_fold:
        res = DUPdoDupTree (FOLD_NEUTRAL (WITH_WITHOP (wl)));
        wl = FREEdoFreeTree (wl);
        break;
    default:
        DBUG_ASSERT ((0), "illegal WITHOP node found!");
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
        const_expr = COaST2Constant ((*expr));
        if (const_expr != NULL) {
            gshape = GV_constant;
            (*expr) = FREEdoFreeTree (*expr);
            (*expr) = COconstant2AST (const_expr);
            const_expr = COfreeConstant (const_expr);

        } else {
            sco_expr = CFscoExpr2StructConstant ((*expr));
            if (sco_expr != NULL) {
                gshape = GV_struct_constant;
                /*
                 * as the sco_expr may share some subexpressions with (*expr),
                 * we have to duplicate these BEFORE deleting (*expr)!!!
                 */
                tmp = CFscoDupStructConstant2Expr (sco_expr);
                (*expr) = FREEdoFreeTree (*expr);
                (*expr) = tmp;
                sco_expr = CFscoFreeStructConstant (sco_expr);

            } else if ((TYisAKS (ID_NTYPE ((*expr))) || TYisAKV (ID_NTYPE ((*expr))))
                       && SHgetExtent (TYgetShape (ID_NTYPE ((*expr))), 0) > 0) {
                gshape = GV_known_shape;
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

    DBUG_ASSERT (((NODE_TYPE (WITH_WITHOP (wl)) == N_modarray)
                  || (NODE_TYPE (WITH_WITHOP (wl)) == N_genarray)),
                 "CropBounds applied to wrong WL type!");
    lbe = ARRAY_AELEMS (WITH_BOUND1 (wl));
    ube = ARRAY_AELEMS (WITH_BOUND2 (wl));

    dim = 0;
    while (lbe) {
        DBUG_ASSERT ((ube != NULL), "dimensionality differs in lower and upper bound!");
        DBUG_ASSERT (((NODE_TYPE (EXPRS_EXPR (lbe)) == N_num)
                      && (NODE_TYPE (EXPRS_EXPR (ube)) == N_num)),
                     "generator bounds must be constant!");
        lbnum = NUM_VAL (EXPRS_EXPR (lbe));
        ubnum = NUM_VAL (EXPRS_EXPR (ube));

        DBUG_ASSERT ((dim < SHgetDim (max_shp)),
                     "dimensionality of lb greater than that of the result!");
        tnum = SHgetExtent (max_shp, dim);
        if (lbnum < 0) {
            NUM_VAL (EXPRS_EXPR (lbe)) = 0;
            CTIerrorLine (NODE_LINE (wl),
                          "Lower bound of WL-generator in dim %d below zero: %d", dim,
                          lbnum);
        }
        if (ubnum > tnum) {
            NUM_VAL (EXPRS_EXPR (ube)) = tnum;
            CTIerrorLine (NODE_LINE (wl),
                          "Upper bound of WL-generator in dim %d greater than shape %d: "
                          "%d",
                          dim, tnum, ubnum);
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

    lbe = WITH_BOUND1 (wl);
    ube = WITH_BOUND2 (wl);
    steps = WITH_STEP (wl);
    width = WITH_WIDTH (wl);

    lbc = COaST2Constant (lbe);
    ubc = COaST2Constant (ube);
    if (max_shp != NULL) {
        shpc = COmakeConstantFromShape (max_shp);
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
        non_empty_bounds = (SHgetUnrLen (COgetShape (lbc)) > 0);

        if (non_empty_bounds) {
            tmpc = COge (lbc, ubc);
            if (COisTrue (tmpc, TRUE)) {
                res = GPT_empty;
            }
            tmpc = COfreeConstant (tmpc);
        }
    }

    if (res == GPT_unknown) {
        if ((NODE_TYPE (WITH_WITHOP (wl)) == N_fold)) {
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
                sh = COgetShape (ubc);
                tmp = COmakeConstantFromShape (sh);
                tmpc = COtake (tmp, shpc);

                tmp = COfreeConstant (tmp);
                shpc = COfreeConstant (shpc);

                shpc = tmpc;

                tmpc = COeq (ubc, shpc);
                if (COisZero (lbc, TRUE) && COisTrue (tmpc, TRUE)) {
                    if (steps == NULL) {
                        res = GPT_full;
                    } else {
                        res = GPT_partial;
                    }
                } else {
                    res = GPT_partial;
                }
                tmpc = COfreeConstant (tmpc);
            }
        }
    }

    /* Clean up constants */
    shpc = (shpc != NULL ? COfreeConstant (shpc) : NULL);
    ubc = (ubc != NULL ? COfreeConstant (ubc) : NULL);
    lbc = (lbc != NULL ? COfreeConstant (lbc) : NULL);

    DBUG_PRINT ("WLPG", ("generator property of with loop in line %d : %s",
                         global.linenum, gen_prop_str[res]));
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGmodule(node *arg_node, info *arg_info)
 *
 *   @brief first traversal of function definitions for ConstantFolding
 *          and WLPartitionGeneration
 *
 *   @param  node *arg_node:  N_module
 *           info *arg_info:  N_info
 *   @return node *        :  N_module
 ******************************************************************************/

node *
WLPGmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPGmodule");

    INFO_WLPG_MODULE (arg_info) = arg_node;

    DBUG_PRINT ("WLPG", ("1. Constant Folding"));
    /* For Constant Folding */
    if (MODULE_FUNS (arg_node) != NULL) {
        TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if ((global.break_after == PH_wlenhance)
        && (0 == strcmp (global.break_specifier, "cf")))
        goto DONE;

    DBUG_PRINT ("WLPG", ("2. WLPartitionGeneration"));
    /* For WLPartitionGeneration module-wise */
    INFO_WLPG_SUBPHASE (arg_info) = SP_mod;
    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
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
        if (global.optimize.docf) {
            arg_node = CFdoConstantFolding (arg_node,
                                            INFO_WLPG_MODULE (arg_info)); /* ssacf_tab */
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }

    } else if (INFO_WLPG_SUBPHASE (arg_info) == SP_mod) {

        if (FUNDEF_BODY (arg_node)) {
            FUNDEF_INSTR (arg_node) = TRAVdo (FUNDEF_INSTR (arg_node), arg_info);
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    } else if (INFO_WLPG_SUBPHASE (arg_info) == SP_func) {
        /* compiler_phase == PH_sacopt */
        if (FUNDEF_BODY (arg_node)) {
            FUNDEF_INSTR (arg_node) = TRAVdo (FUNDEF_INSTR (arg_node), arg_info);
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

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

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
            ASSIGN_NEXT (iterator) = TRAVdo (ASSIGN_NEXT (iterator), arg_info);
        }
    } else {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGlet(node *arg_node, info *arg_info)
 *
 *   @brief traverses in expression and checks assigned ids for constant
 *          value and creates corresponding akv type
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

        if (!TYisAKV (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))))) {
            constant *new_co = NULL;
            simpletype simple;

            new_co = COaST2Constant (LET_EXPR (arg_node));
            if (NULL != new_co) {
                /*
                 * it is possible to infer constant value
                 */
                simple = TYgetSimpleType (
                  TYgetScalar (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)))));
                AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)))
                  = TYfreeType (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))));
                AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)))
                  = TYmakeAKV (TYmakeSimpleType (simple), new_co);
            }
        }
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

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
        && (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))) {
        /*
         * special functions must be traversed when they are used
         */
        if (AP_FUNDEF (arg_node) != INFO_WLPG_FUNDEF (arg_info)) {

            /* stack arg_info */
            tmp = arg_info;
            arg_info = MakeInfo ();
            /* take current flag SUBPHASE */
            INFO_WLPG_SUBPHASE (arg_info) = INFO_WLPG_SUBPHASE (tmp);

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            arg_info = FreeInfo (arg_info);
            arg_info = tmp;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGwith(node *arg_node, info *arg_info)
 *
 *   @brief  start traversal of this WL and store information in new arg_info
 *           node. The only N_part node (inclusive body) is traversed.
 *           Afterwards, if certain conditions are fulfilled,
 *           the WL is transformed into a WL with generators describing a full
 *           partition.
 *
 *   @param  node *arg_node:  N_with
 *           info *arg_info:  N_info
 *   @return node *        :  N_with
 ******************************************************************************/

node *
WLPGwith (node *arg_node, info *arg_info)
{
    node *let_tmp;
    bool replace_wl = FALSE;

    DBUG_ENTER ("WLPGwith");

    /*
     * Information about last N_let has to be stacked before traversal
     */
    let_tmp = INFO_WLPG_LET (arg_info);

    /*
     * The CODEs have to be traversed as they may contain further (nested) WLs
     * and I want to modify bottom up.
     */
    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    /* Pop N_let */
    INFO_WLPG_LET (arg_info) = let_tmp;

    /*
     * Once a full partition has been created, we do not need to inspect
     * the generators anymore. The indicator for this is the PARTS attribute.
     * Initially, it is set to -1; if we have successfully generated a full
     * partition, it carries a positive value.
     */
    if (WITH_PARTS (arg_node) == -1) {

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
        DBUG_ASSERT ((WITH_PART (arg_node) != NULL),
                     "NWITH_PARTS is -1 although no PART is available!");
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

        /*
         * Now, we traverse the WITHOP sons for propagating (structural) constants.
         */
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        if (INFO_WLPG_GENPROP (arg_info) == GPT_empty) {
            arg_node = CreateEmptyGenWLReplacement (arg_node, arg_info);
            replace_wl = TRUE;

        } else if (INFO_WLPG_GENPROP (arg_info) == GPT_full) {
            WITH_PARTS (arg_node) = 1;

            if (NODE_TYPE (WITH_WITHOP (arg_node)) == N_genarray) {
                if (GENARRAY_DEFAULT (WITH_WITHOP (arg_node)) != NULL) {
                    GENARRAY_DEFAULT (WITH_WITHOP (arg_node))
                      = FREEdoFreeTree (GENARRAY_DEFAULT (WITH_WITHOP (arg_node)));
                }
            }

        } else if ((INFO_WLPG_GENPROP (arg_info) == GPT_partial)
                   && ((INFO_WLPG_GENSHP (arg_info) == GV_constant)
                       || (INFO_WLPG_GENSHP (arg_info) == GV_struct_constant))
                   && AtLeastAKD (ID_NTYPE (EXPRS_EXPR (WITH_CEXPRS (arg_node))))) {
            arg_node = CreateFullPartition (arg_node, arg_info);
        }

    } else {
        /*
         * traverse the WITHOP sons for propagating (structural) constants.
         */
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    INFO_WLPG_WL (arg_info) = NULL;
    INFO_WLPG_LET (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGgenarray( node *arg_node, info *arg_info)
 *
 *   @brief Substitutes GENARRAY_SHAPE into the N_genarray node.
 *
 *   @param  node *arg_node:  N_genarray
 *           info *arg_info:  N_info
 *   @return node *        :  N_genarray
 ******************************************************************************/

node *
WLPGgenarray (node *arg_node, info *arg_info)
{
    node *nassigns, *f_def;
    gen_shape_t current_shape;

    DBUG_ENTER ("WLPGgenarray");

    f_def = INFO_WLPG_FUNDEF (arg_info);

    if (GENARRAY_SHAPE (arg_node) != NULL) {
        GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    }
    current_shape = PropagateArrayConstants (&(GENARRAY_SHAPE (arg_node)));
    if (current_shape == GV_known_shape) {
        nassigns = CreateNewAssigns (GENARRAY_SHAPE (arg_node), f_def);
        GENARRAY_SHAPE (arg_node)
          = CreateStructConstant (GENARRAY_SHAPE (arg_node), nassigns);
        current_shape = GV_struct_constant;
        INFO_WLPG_NASSIGNS (arg_info)
          = TCappendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
    }

    if (INFO_WLPG_GENSHP (arg_info) < current_shape) {
        INFO_WLPG_GENSHP (arg_info) = current_shape;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGpart(node *arg_node, info *arg_info)
 *
 *   @brief traverse only generator to propagate arrays
 *
 *   @param  node *arg_node:  N_part
 *           info *arg_info:  N_info
 *   @return node *        :  N_part
 ******************************************************************************/

node *
WLPGpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLPGpart");

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGgenerator(node *arg_node, info *arg_info)
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
 *   @param  node *arg_node:  N_generator
 *           info *arg_info:  N_info
 *   @return node *        :  N_generator
 ******************************************************************************/

node *
WLPGgenerator (node *arg_node, info *arg_info)
{
    node *wln, *f_def, *nassigns, *let_ids;
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
    current_shape = PropagateArrayConstants (&(GENERATOR_BOUND1 (arg_node)));
    if (current_shape == GV_known_shape) {
        nassigns = CreateNewAssigns (GENERATOR_BOUND1 (arg_node), f_def);
        GENERATOR_BOUND1 (arg_node)
          = CreateStructConstant (GENERATOR_BOUND1 (arg_node), nassigns);
        gshape = GV_struct_constant;
        INFO_WLPG_NASSIGNS (arg_info)
          = TCappendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
    } else {
        gshape = current_shape;
    }

    current_shape = PropagateArrayConstants (&(GENERATOR_BOUND2 (arg_node)));
    if (current_shape == GV_known_shape) {
        nassigns = CreateNewAssigns (GENERATOR_BOUND2 (arg_node), f_def);
        GENERATOR_BOUND2 (arg_node)
          = CreateStructConstant (GENERATOR_BOUND2 (arg_node), nassigns);
        current_shape = GV_struct_constant;
        INFO_WLPG_NASSIGNS (arg_info)
          = TCappendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }
    check_bounds = (gshape == GV_constant);

    current_shape = PropagateArrayConstants (&(GENERATOR_STEP (arg_node)));
    if (current_shape == GV_known_shape) {
        nassigns = CreateNewAssigns (GENERATOR_STEP (arg_node), f_def);
        GENERATOR_STEP (arg_node)
          = CreateStructConstant (GENERATOR_STEP (arg_node), nassigns);
        current_shape = GV_struct_constant;
        INFO_WLPG_NASSIGNS (arg_info)
          = TCappendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }

    check_stepwidth = (current_shape <= GV_struct_constant);

    current_shape = PropagateArrayConstants (&(GENERATOR_WIDTH (arg_node)));
    if (current_shape == GV_known_shape) {
        nassigns = CreateNewAssigns (GENERATOR_WIDTH (arg_node), f_def);
        GENERATOR_WIDTH (arg_node)
          = CreateStructConstant (GENERATOR_WIDTH (arg_node), nassigns);
        current_shape = GV_struct_constant;
        INFO_WLPG_NASSIGNS (arg_info)
          = TCappendAssign (INFO_WLPG_NASSIGNS (arg_info), nassigns);
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

    if (TYisAKS (type) || TYisAKV (type)) {
        shp = TYgetShape (type);
        if (check_bounds
            && ((NODE_TYPE (WITH_WITHOP (wln)) == N_modarray)
                || (NODE_TYPE (WITH_WITHOP (wln)) == N_genarray))) {
            wln = CropBounds (wln, shp);
        }
        gprop = ComputeGeneratorProperties (wln, shp);
    } else {
        gprop = ComputeGeneratorProperties (wln, NULL);
    }

    if (gshape == GV_struct_constant) {
        if (NODE_TYPE (WITH_WITHOP (wln)) == N_fold) {
            gprop = GPT_full;
        } else {
            gprop = GPT_partial;
        }
    }

    if (check_stepwidth) {
        /* normalize step and width */
        switch (
          NormalizeStepWidth (&GENERATOR_STEP (arg_node), &GENERATOR_WIDTH (arg_node))) {
        case 1:
            CTIabortLine (NODE_LINE (wln), "Component of width greater than step");
            break;
        case 2:
            CTIabortLine (NODE_LINE (wln), "Component of width less than zero");
            break;
        case 3:
            CTIabortLine (NODE_LINE (wln), "Width vector without step vector");
            break;
        }
    }

    INFO_WLPG_GENPROP (arg_info) = gprop;
    INFO_WLPG_GENSHP (arg_info) = gshape;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGdoPartitionGeneration( node *arg_node)
 *
 *   @brief  Starting point for the partition generation if it was called
 *           from main.
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
WLPGdoWlPartitionGeneration (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("WLPGdoWlPartitionGeneration");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "WLPGdoWlPartitionGeneration not started with module node");

    DBUG_PRINT ("WLPG", ("starting WLPGdoWlPartitionGeneration"));

    arg_info = MakeInfo ();
    INFO_WLPG_SUBPHASE (arg_info) = SP_cf;

    TRAVpush (TR_wlpg);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGdoPartitionGenerationOpt( node *arg_node)
 *
 *   @brief  Starting point for the partition generation if it was called
 *           from optimize.
 *
 *   @param  node *arg_node:  N_fundef
 *   @return node *        :  transformed N_fundef
 ******************************************************************************/

node *
WLPGdoWlPartitionGenerationOpt (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("WLPGdoWlPartitionGenerationOpt");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "WLPGdoWlPartitionGenerationOpt not started with fundef node");

    DBUG_PRINT ("WLPG", ("starting WLPGdoWlPartitionGenerationOpt"));

    arg_info = MakeInfo ();
    INFO_WLPG_SUBPHASE (arg_info) = SP_func;

    TRAVpush (TR_wlpg);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
