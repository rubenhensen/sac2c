/*
 *
 * $Log$
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

/*******************************************************************************
 *  Usage of arg_info:
 *  - node[0]: NEXT   : store old information in nested WLs
 *  - node[1]: WL     : reference to base node of current WL (N_Nwith)
 *  - node[2]: ASSIGN : always the last N_assign node
 *  - node[3]: FUNDEF : pointer to last fundef node. needed to access vardecs.
 *  - node[4]: LET    : pointer to N_let node of current WL.
 *                      LET_EXPR(ID) == INFO_WLPG_WL.
 *  - node[5]: REPLACE: if != NULL, replace WL with this node.
 *
 ******************************************************************************/

#define INFO_WLPG_NEXT(n) (n->node[0])
#define INFO_WLPG_WL(n) (n->node[1])
#define INFO_WLPG_ASSIGN(n) (n->node[2])
#define INFO_WLPG_FUNDEF(n) (n->node[3])
#define INFO_WLPG_LET(n) (n->node[4])
#define INFO_WLPG_REPLACE(n) (n->node[5])
#define INFO_WLPG_GENPROP(n) (n->counter)
#define INFO_WLPG_GENSHP(n) (n->int_data)
#define INFO_WLPG_NASSIGNS(n) ((node *)(n->dfmask[0]))

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
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
#include "../psi-opt/SSAWithloopFolding.h"
#include "ssa.h"
#include "WLPartitionGeneration.h"

typedef enum { GPT_empty, GPT_full, GPT_partial, GPT_unknown } gen_prop_t;

typedef enum {
    GV_constant,
    GV_struct_constant,
    GV_known_shape,
    GV_unknown_shape
} gen_shape_t;

/** <!--********************************************************************-->
 *
 * @fn intern_gen *CutSlices( int *ls, int *us, int *l, int *u, int dim,
 *                            intern_gen *ig, node *coden)
 *
 *   @brief  Creates a (full) partition by adding new intern_gen structs.
 *           If the known part is a grid, this is ignored here (so the
 *           resulting intern_gen chain may still not be a full partition,
 *           see CompleteGrid()).
 *
 *   @param  int *ls, *us  : bounds of the whole array
 *           int *l, *u    : bounds of the given part
 *           int dim       : number of elements of ls, us, l, u
 *           intern_gen ig : chain of intern_gen struct where to add the
 *                           new parts. If ig != NULL, the same pointer is
 *                           returned.
 *           node *coden   : Pointer of N_Ncode node where the new generators
 *                           shall point to.
 *   @return intern_gen *  : the list of intern_gen
 ******************************************************************************/
static intern_gen *
CutSlices (int *ls, int *us, int *l, int *u, int dim, intern_gen *ig, node *coden)
{
    int *lsc, *usc, i, d;
    intern_gen *root_ig = NULL;

    DBUG_ENTER ("CutSlices");

    /* create local copies of the arrays which atr modified here*/
    lsc = Malloc (sizeof (int) * dim);
    usc = Malloc (sizeof (int) * dim);
    for (i = 0; i < dim; i++) {
        lsc[i] = ls[i];
        usc[i] = us[i];
    }

    root_ig = ig;

    for (d = 0; d < dim; d++) {
        /* Check whether there is a cuboid above (below) the given one. */
        if (l[d] > lsc[d]) {
            ig = SSAAppendInternGen (ig, dim, coden, 0);
            for (i = 0; i < dim; i++) {
                ig->l[i] = lsc[i];
                ig->u[i] = usc[i];
            }
            ig->u[d] = l[d];

            if (!root_ig) {
                root_ig = ig;
            }
        }

        if (u[d] < usc[d]) {
            ig = SSAAppendInternGen (ig, dim, coden, 0);
            for (i = 0; i < dim; i++) {
                ig->l[i] = lsc[i];
                ig->u[i] = usc[i];
            }
            ig->l[d] = u[d];

            if (!root_ig) {
                root_ig = ig;
            }
        }

        /* and modify array bounds to  continue with next dimension */
        lsc[d] = l[d];
        usc[d] = u[d];
    }

    lsc = Free (lsc);
    usc = Free (usc);

    DBUG_RETURN (root_ig);
}

/** <!--********************************************************************-->
 *
 * @fn intern_gen *CompleteGrid( int *ls, int *us, int *step, int *width,
 *                               int dim, intern_gen *ig, node *coden)
 *
 *   @brief  adds new generators which specify the elements left out by a grid.
 *
 *   @param  int *ls,*us       : bounds of the given part
 *           int *step, *width :
 *           int dim           : number of elements of ls, us
 *           intern_gen *ig    : chain of intern_gen struct where to add the
 *                                new parts.
 *                                If ig != NULL, the same pointer is returned.
 *           node *coden       : Pointer of N_Ncode node where the new
 *                                generators shall point to.
 *   @return intern_gen *      : chain of intern_gen struct
 ******************************************************************************/
static intern_gen *
CompleteGrid (int *ls, int *us, int *step, int *width, int dim, intern_gen *ig,
              node *coden)
{
    int i, d, *nw;
    intern_gen *root_ig;

    DBUG_ENTER ("CompleteGrid");

    nw = Malloc (sizeof (int) * dim);
    for (i = 0; i < dim; i++)
        nw[i] = step[i];

    root_ig = ig;

    for (d = 0; d < dim; d++) {
        if (step[d] > width[d]) { /* create new gris */
            ig = SSAAppendInternGen (ig, dim, coden, 1);
            for (i = 0; i < dim; i++) {
                ig->l[i] = ls[i];
                ig->u[i] = us[i];
                ig->step[i] = step[i];
                ig->width[i] = nw[i];
            }
            ig->l[d] = ig->l[d] + width[d];
            ig->width[d] = step[d] - width[d];
            i = SSANormalizeInternGen (ig);
            DBUG_ASSERT (!i, ("internal normalization failure"));

            if (!root_ig) {
                root_ig = ig;
            }
        }

        nw[d] = width[d];
    }

    nw = Free (nw);

    DBUG_RETURN (root_ig);
}

/** <!--********************************************************************-->
 *
 * @fn  node *CreateFullPartition( node *wln, node *arg_info)
 *
 *   @brief  generates full partition if possible:
 *           - if withop is genarray and index vector has as much elements as
 *             dimension of resulting WL (withloop on scalars).
 *           - if withop is modarray: always (needed for compilation phase).
 *           Returns wln.
 *
 *   @param  node *wl       :  N_with node of the WL to transform
 *           node *arg_info :  is needed to access the vardecs of the current
 *                             function
 *   @return node *         :  modified N_with
 ******************************************************************************/
static node *
CreateFullPartition (node *wln, node *arg_info)
{
    node *coden, *idn;
    ids *_ids;
    intern_gen *ig;
    types *type;
    char *varname;
    int *array_null, *array_shape;
    int dim, i, gen_shape = 0;
    bool do_create;

    DBUG_ENTER ("CreateFullPartition");

    do_create = TRUE;

    if (do_create) {
        /* get shape of the index vector (generator) */
        gen_shape = IDS_SHAPE (NWITH_VEC (wln), 0);
    }

    /* determine type of expr in the operator (result of body) */
    type = ID_TYPE (NWITH_CEXPR (wln));

    /*
     * allocate "array_shape"
     */
    if (do_create) {
        switch (NWITH_TYPE (wln)) {
        case WO_modarray: {
            /* create upper array bound */
            shpseg *tmp = Type2Shpseg (ID_TYPE (NWITH_ARRAY (wln)), &dim);

            if (dim >= 0) {
                array_shape = (int *)Malloc (dim * sizeof (int));
                for (i = 0; i < dim; i++) {
                    array_shape[i] = SHPSEG_SHAPE (tmp, i);
                }
                if (tmp != NULL) {
                    tmp = FreeShpseg (tmp);
                }
            } else {
                array_shape = NULL;
            }
        } break;

        case WO_genarray:
            array_shape = NULL;
            SSAArrayST2ArrayInt (NWITH_SHAPE (wln), &array_shape, gen_shape);
            break;

        default:
            DBUG_ASSERT ((0), "illegal NWITH_TYPE found!");
            array_shape = NULL;
            break;
        }
        do_create = (array_shape != NULL);
    } else {
        array_shape = NULL;
    }

    /*
     * start creation
     */
    if (do_create) {
        /* create lower array bound */
        array_null = NULL;
        SSAArrayST2ArrayInt (NULL, &array_null, gen_shape);

        /* create code for all new parts */
        if (NWITH_TYPE (wln) == WO_genarray) {
            /* create a zero of the correct type */
            if (sbs == 1) {
                if (NWITHOP_DEFAULT (NWITH_WITHOP (wln)) == NULL) {
                    coden = CreateZeroFromType (type, FALSE, INFO_WLI_FUNDEF (arg_info));
                } else {
                    coden = DupTree (NWITHOP_DEFAULT (NWITH_WITHOP (wln)));
                }
            } else {
                coden = CreateZeroFromType (type, FALSE, INFO_WLI_FUNDEF (arg_info));
            }
        } else { /* modarray */
            /*
             * we build NO with-loop here in order to ease optimizations
             *    B = modarray( A, iv, sel( iv, A));   ->   B = A;
             * and to avoid loss of performance
             *    sel( iv, A)   ->   idx_sel( ..., A)
             */
            coden = CreateSel (NWITH_VEC (wln), NWITH_IDS (wln), NWITH_ARRAY (wln), FALSE,
                               INFO_WLI_FUNDEF (arg_info));
        }
        varname = TmpVar ();
        _ids = MakeIds (varname, NULL, ST_regular);
        IDS_VARDEC (_ids)
          = SSACreateVardec (varname, type,
                             &(FUNDEF_VARDEC (INFO_WLI_FUNDEF (arg_info))));
        /* varname is duplicated here (own mem) */
        idn = MakeId (StringCopy (varname), NULL, ST_regular);
        ID_VARDEC (idn) = IDS_VARDEC (_ids);

        /* create new N_Ncode node  */
        coden
          = MakeNCode (MakeBlock (MakeAssign (MakeLet (coden, _ids), NULL), NULL), idn);

        /* now, copy the only part to ig */
        ig = SSATree2InternGen (wln, NULL);
        DBUG_ASSERT (!ig->next, ("more than one part exist"));
        /* create surrounding cuboids */
        ig = CutSlices (array_null, array_shape, ig->l, ig->u, ig->shape, ig, coden);
        /* the original part can still be found at *ig. Now create grids. */
        if (ig->step)
            ig = CompleteGrid (ig->l, ig->u, ig->step, ig->width, ig->shape, ig, coden);

        /* if new codes have been created, add them to code list */
        if (ig->next) {
            NCODE_NEXT (coden) = NWITH_CODE (wln);
            NWITH_CODE (wln) = coden;
        }

        wln = SSAInternGen2Tree (wln, ig);

        /* free the above made arrays */
        ig = SSAFreeInternGenChain (ig);
        array_null = Free (array_null);
    }
    array_shape = Free (array_shape);

    DBUG_RETURN (wln);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateEmptyGenWLReplacement( node *wl, node *arg_info)
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
 *           node *arg_info :  N_INFO
 *   @return node *         :  modified N_with
 ******************************************************************************/
static node *
CreateEmptyGenWLReplacement (node *wl, node *arg_info)
{
    ids *let_ids;
    int dim, i;
    node *lb, *ub, *lbe, *ube;
    node *code;
    node *tmpn, *assignn, *idn, *blockn;
    ids *_ids;
    char *varname;
    types *type;

    node *res;

    DBUG_ENTER ("CreateEmptyGenWLReplacement");

    switch (NWITH_TYPE (wl)) {
    case WO_genarray:
        res = wl;
        /*
         * First, we change the generator to full scope.
         */
        let_ids = LET_IDS (INFO_WLPG_LET (arg_info));
        dim = GetDim (IDS_TYPE (let_ids));
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

            if (N_empty == NODE_TYPE (tmpn)) {
                /* there is no instruction in the block right now. */
                BLOCK_INSTR (blockn) = FreeTree (BLOCK_INSTR (blockn));
                /* first, introduce a new variable */
                varname = TmpVar ();
                _ids = MakeIds (varname, NULL, ST_regular);
                /* determine type of expr in the operator (result of body) */
                type = ID_TYPE (NWITH_CEXPR (INFO_WLPG_WL (arg_info)));
                IDS_VARDEC (_ids)
                  = SSACreateVardec (varname, type,
                                     &(FUNDEF_VARDEC (INFO_WLPG_FUNDEF (arg_info))));
                IDS_AVIS (_ids) = VARDEC_AVIS (IDS_VARDEC (_ids));

                tmpn = CreateZeroFromType (type, FALSE, INFO_WLPG_FUNDEF (arg_info));

                /* replace N_empty with new assignment "_ids = [0,..,0]" */
                assignn = MakeAssign (MakeLet (tmpn, _ids), NULL);
                BLOCK_INSTR (blockn) = assignn;

                /* set correct backref to defining assignment */
                AVIS_SSAASSIGN (IDS_AVIS (_ids)) = assignn;

                /* replace CEXPR */
                idn = MakeId (StringCopy (varname), NULL, ST_regular);
                ID_VARDEC (idn) = IDS_VARDEC (_ids);
                ID_AVIS (idn) = IDS_AVIS (_ids);
                tmpn = NWITH_CODE (INFO_WLPG_WL (arg_info));
                NCODE_CEXPR (tmpn) = FreeTree (NCODE_CEXPR (tmpn));
                NCODE_CEXPR (tmpn) = idn;

            } else {
                /* we have a non-empty block.
                   search last assignment and make it the only one in the block. */
                while (ASSIGN_NEXT (tmpn)) {
                    tmpn = ASSIGN_NEXT (tmpn);
                }
                assignn = DupTree (tmpn);
                FreeTree (BLOCK_INSTR (blockn));
                FreeTree (LET_EXPR (ASSIGN_INSTR (assignn)));
                BLOCK_INSTR (blockn) = assignn;
                _ids = LET_IDS (ASSIGN_INSTR (assignn));
                type = IDS_TYPE (_ids);

                tmpn = CreateZeroFromType (type, FALSE, INFO_WLPG_FUNDEF (arg_info));
                NCODE_FLAG (NWITH_CODE (INFO_WLPG_WL (arg_info))) = TRUE;

                LET_EXPR (ASSIGN_INSTR (assignn)) = tmpn;
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
 * @fn node *CreateNewAssigns( node *expr, node *f_def)
 *
 *   @brief expects (expr) to point to an identifier and generates as many
 *          new assigns as the value of the shapevektor at position 0 of (expr)
 *          indicates.
 *
 *   @param  node *expr  :  expr
 *           node *f_def :  N_fundef
 *   @return node *      :  a chained list of N_assign nodes
 ******************************************************************************/
static node *
CreateNewAssigns (node *expr, node *f_def)
{
    int num_elems, iterator;
    node *tmp1, *tmp2, *tmp3, *nassigns;
    ids *_ids;
    char *varname, *nvarname;
    types *type;

    DBUG_ENTER ("CreateNewAssigns");

    DBUG_ASSERT ((expr != NULL), "Expr is empty");
    DBUG_ASSERT ((NODE_TYPE (expr) == N_id), "CreateNewAssigns not called with N_id");
    DBUG_ASSERT ((ID_DIM (expr) == 1), "Dimension of Id is not 1");
    DBUG_ASSERT ((ID_SHPSEG (expr) != NULL), "SHPSEG of Id points to NULL");

    varname = ID_NAME (expr);
    num_elems = ID_SHAPE (expr, 0);

    iterator = num_elems - 1;
    nassigns = NULL;

    while (num_elems > 0) {
        nvarname = TmpVarName (varname);
        _ids = MakeIds (nvarname, NULL, ST_regular);
        type = ID_TYPE (expr);
        IDS_VARDEC (_ids) = SSACreateVardec (nvarname, type, &(FUNDEF_VARDEC (f_def)));
        IDS_AVIS (_ids) = VARDEC_AVIS (IDS_VARDEC (_ids));

        /* not flat! */

        /* index position for selection */
        /* tmp1  = MakeFlatArray( MakeExprs( MakeNum( iterator), NULL));*/
        tmp1 = MakeNum (iterator);
        /* the array for selection */
        tmp2 = MakeExprs (DupTree (expr), NULL);

        tmp3 = MakePrf (F_idx_sel, MakeExprs (tmp1, tmp2));

        nassigns = MakeAssign (MakeLet (tmp3, _ids), nassigns);

        /* set correct backref to defining assignment */
        AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassigns;

        iterator--;
        num_elems--;
    }

    DBUG_RETURN (nassigns);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateStructConstant( node *expr, node *nassigns)
 *
 *   @brief expects (expr) point to an identifier.
 *          A structural constant is freshly generated by means of (nassigns)
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
    ids *tmp_ids;
    char *varname;

    DBUG_ENTER ("CreateStructConstant");

    DBUG_ASSERT ((expr != NULL), "Expr is empty");

    DBUG_ASSERT ((NODE_TYPE (expr) == N_id), "CreateStructConstant not called with N_id");

    DBUG_ASSERT ((nassigns != NULL), "NASSIGNS is empty");

    tmp1 = NULL;
    tmp2 = NULL;
    iterator = nassigns;

    while (iterator) {
        tmp_ids = LET_IDS (ASSIGN_INSTR (iterator));
        varname = StringCopy (IDS_NAME (tmp_ids));

        idn = MakeId (varname, NULL, ST_regular);
        ID_VARDEC (idn) = IDS_VARDEC (tmp_ids);
        ID_AVIS (idn) = IDS_AVIS (tmp_ids);

        if (tmp1) {
            EXPRS_NEXT (tmp2) = MakeExprs (idn, NULL);
            tmp2 = EXPRS_NEXT (tmp2);
        } else {
            tmp1 = MakeExprs (idn, tmp1);
            tmp2 = tmp1;
        }

        iterator = ASSIGN_NEXT (iterator);
    }

    tmp1 = MakeFlatArray (tmp1);
    expr = FreeTree (expr);
    expr = tmp1;

    DBUG_RETURN (expr);
}

/** <!--********************************************************************-->
 *
 * @fn node *AppendAssigns( node *nassigns, node *oassigns)
 *
 *   @brief appends nassigns on oassigns.
 *
 *   @param  node *nassigns :  a new chained list of N_assign nodes
 *           node *oassigns :  a old chained list of N_assign nodes
 *   @return node *         :  updated oassigns
 ******************************************************************************/
static node *
AppendAssigns (node *nassigns, node *oassigns)
{
    node *iterator;

    DBUG_ENTER ("AppendAssigns");

    if (oassigns) {
        iterator = oassigns;
        while (ASSIGN_NEXT (iterator)) {
            iterator = ASSIGN_NEXT (iterator);
        }
        ASSIGN_NEXT (iterator) = nassigns;
    } else {
        oassigns = nassigns;
    }

    DBUG_RETURN (oassigns);
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

/** <!--********************************************************************-->
 *
 * @fn gen_prop_t CheckGeneratorBounds( node *wl, shape *max_shp)
 *
 *   @brief expects the bound expression for lb and ub to be constants!
 *          checks, whether these fit into the shape given by max_shp.
 *          If they exceed max_shp, they are "fitted"!
 *          Furthermore, it is checked whether the generator is empty
 *          (GPT_empty is returned) or may cover the entire range
 *          (GPT_full is returned). Otherwise GPT_partial is returned.
 *
 *   @param  node *wl        :  N_Nwith
 *           shape *max_shape:  shape of the wl
 *   @return gen_prob_t      :  GPT_empty, GPT_full, GPT_partial or GPT_unknown
 ******************************************************************************/

static gen_prop_t
CheckGeneratorBounds (node *wl, shape *max_shp)
{
    node *lbe, *ube;
    int dim;
    int lbnum, ubnum, tnum;
    gen_prop_t res;

    DBUG_ENTER ("CheckGeneratorBounds");

    lbe = ARRAY_AELEMS (NWITH_BOUND1 (wl));
    ube = ARRAY_AELEMS (NWITH_BOUND2 (wl));
    res = GPT_full;

    dim = 0;
    while (lbe) {
        DBUG_ASSERT ((ube != NULL), "dimensionality differs in lower and upper bound!");
        DBUG_ASSERT (((NODE_TYPE (EXPRS_EXPR (lbe)) == N_num)
                      && (NODE_TYPE (EXPRS_EXPR (ube)) == N_num)),
                     "generator bounds must be constant!");
        lbnum = NUM_VAL (EXPRS_EXPR (lbe));
        ubnum = NUM_VAL (EXPRS_EXPR (ube));

        if ((NWITH_TYPE (wl) == WO_modarray) || (NWITH_TYPE (wl) == WO_genarray)) {
            DBUG_ASSERT ((dim < SHGetDim (max_shp)),
                         "dimensionality of lb greater than that of the result!");
            tnum = SHGetExtent (max_shp, dim);
            if (lbnum < 0) {
                lbnum = NUM_VAL (EXPRS_EXPR (lbe)) = 0;
                WARN (NODE_LINE (wl),
                      ("lower bound of WL-generator in dim %d below zero: set to 0",
                       dim));
            } else if (lbnum > 0) {
                res = GPT_partial;
            }
            if (ubnum > tnum) {
                ubnum = NUM_VAL (EXPRS_EXPR (ube)) = tnum;
                WARN (NODE_LINE (wl),
                      ("upper bound of WL-generator in dim %d greater than shape:"
                       " set to %d",
                       dim, tnum));
            } else if (ubnum < tnum) {
                res = GPT_partial;
            }
        }

        if (lbnum >= ubnum) {
            /* empty set of indices */
            res = GPT_empty;
            break;
        }

        dim++;
        lbe = EXPRS_NEXT (lbe);
        ube = EXPRS_NEXT (ube);
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGmodul(node *arg_node, node *arg_info)
 *
 *   @brief traverses all function definitions
 *
 *   @param  node *arg_node:  N_modul
 *           node *arg_info:  N_info
 *   @return node *        :  N_modul
 ******************************************************************************/

node *
WLPGmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLPGmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGfundef(node *arg_node, node *arg_info)
 *
 *   @brief starts the traversal of the given fundef
 *
 *   @param  node *arg_node:  N_fundef
 *           node *arg_info:  N_info
 *   @return node *        :  N_fundef
 ******************************************************************************/

node *
WLPGfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLPGfundef");

    INFO_WLPG_WL (arg_info) = NULL;
    INFO_WLPG_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node)) {
        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGassign(node *arg_node, node *arg_info)
 *
 *   @brief store actual assign node in arg_info and traverse instruction
 *
 *   @param  node *arg_node:  N_assign
 *           node *arg_info:  N_info
 *   @return node *        :  N_assign
 ******************************************************************************/

node *
WLPGassign (node *arg_node, node *arg_info)
{
    node *iterator;

    DBUG_ENTER ("WLPGassign");

    INFO_WLPG_ASSIGN (arg_info) = arg_node;

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
 * @fn node *WLPGlet(node *arg_node, node *arg_info)
 *
 *   @brief traverses in expression and checks assigned ids for constant
 *          value and sets corresponding AVIS_SSACONST attribute
 *
 *   @param  node *arg_node:  N_let
 *           node *arg_info:  N_info
 *   @return node *        :  N_let
 ******************************************************************************/
node *
WLPGlet (node *arg_node, node *arg_info)
{

    DBUG_ENTER ("WLPGlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "N_let with empty EXPR attribute.");

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
 * @fn node *WLPGNwith(node *arg_node, node *arg_info)
 *
 *   @brief  start traversal of this WL and store information in new arg_info
 *           node. The only N_Npart node (inclusive body) is traversed.
 *           Afterwards, if certain conditions are fulfilled,
 *           the WL is transformed into a WL with generators describing a full
 *           partition.
 *
 *   @param  node *arg_node:  N_Nwith
 *           node *arg_info:  N_info
 *   @return node *        :  N_Nwith
 ******************************************************************************/

node *
WLPGNwith (node *arg_node, node *arg_info)
{
    bool replace_wl = FALSE;

    DBUG_ENTER ("WLPGNwith");

    /*
     * The CODEs have to be traversed as they may contain further (nested) WLs
     * and I want to modify bottom up.
     */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    /*
     * Until this compilerphase no withloop has been checked
     * if it's only PART covers a full partition. The indicator
     * for this is the PARTS attribute. Initially, it is set to -1;
     * if we have successfully generated a full partition,
     * it carries a positive value.
     */
    DBUG_ASSERT ((NWITH_PARTS (arg_node) == -1), "WL has a full partition");

    /*
     * initialize WL traversal
     */
    INFO_WLPG_WL (arg_info) = arg_node; /* store the current node for later */
    INFO_WLPG_LET (arg_info) = ASSIGN_INSTR (INFO_WLPG_ASSIGN (arg_info));

    /*
     * traverse the one and only (!) PART.
     * Besides minor changes in the generator, two values are computed
     * during this traversal:
     *
     *  INFO_WLPG_GENVAR(arg_info) and INFO_WLPG_GENPROP(arg_info) !!
     */
    if (NWITH_PART (arg_node) != NULL) {
        DBUG_ASSERT ((NPART_NEXT (NWITH_PART (arg_node)) == NULL),
                     "PARTS is -1 although more than one PART is available!");
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    /*
     * Now, we traverse the WITHOP sons for propagating (structural) constants.
     */
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /* only for testing */
    if (INFO_WLPG_GENSHP (arg_info) == GV_constant) {
        NWITH_FOLDABLE (arg_node) = TRUE;
    } else {
        NWITH_FOLDABLE (arg_node) = FALSE;
    }
    /* only for testing */

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

        /*
           } else if( (INFO_WLPG_GENPROP(arg_info) == GPT_partial) &&
           ( (INFO_WLPG_GENSHP(arg_info) == GV_constant) ||
           (INFO_WLPG_GENSHP(arg_info) == GV_struct_constant) ) ) {
           arg_node = CreateFullPartition( arg_node, arg_info);
           }
        */

        /* only for testing */
    } else if (INFO_WLPG_GENPROP (arg_info) == GPT_partial
               && (INFO_WLPG_GENSHP (arg_info) == GV_constant)) {
        arg_node = CreateFullPartition (arg_node, arg_info);
    }

    /* only for testing */

    INFO_WLPG_WL (arg_info) = NULL;
    INFO_WLPG_LET (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGNwithop( node *arg_node, node *arg_info)
 *
 *   @brief Substitutes NWITHOP_SHAPE into the N_Nwithop node.
 *
 *   @param  node *arg_node:  N_Nwithop
 *           node *arg_info:  N_info
 *   @return node *        :  N_Nwithop
 ******************************************************************************/

node *
WLPGNwithop (node *arg_node, node *arg_info)
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
            INFO_WLPG_NASSIGNS (arg_info)
              = AppendAssigns (nassigns, INFO_WLPG_NASSIGNS (arg_info));
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

    default:
        DBUG_ASSERT ((0), "illegal NWITHOP_TYPE found!");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGNpart(node *arg_node, node *arg_info)
 *
 *   @brief traverse only generator to propagate arrays
 *
 *   @param  node *arg_node:  N_Npart
 *           node *arg_info:  N_info
 *   @return node *        :  N_Npart
 ******************************************************************************/

node *
WLPGNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLPGNpart");

    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPGNgenerator(node *arg_node, node *arg_info)
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
 *           node *arg_info:  N_info
 *   @return node *        :  N_Ngenerator
 ******************************************************************************/

node *
WLPGNgenerator (node *arg_node, node *arg_info)
{
    node *wln, *f_def, *nassigns;
    ids *let_ids;
    shape *shp;
    bool check_bounds;
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
          = AppendAssigns (nassigns, INFO_WLPG_NASSIGNS (arg_info));
    } else {
        gshape = current_shape;
    }

    current_shape = PropagateArrayConstants (&(NGEN_BOUND2 (arg_node)));
    if (current_shape == GV_known_shape) {
        nassigns = CreateNewAssigns (NGEN_BOUND2 (arg_node), f_def);
        NGEN_BOUND2 (arg_node) = CreateStructConstant (NGEN_BOUND2 (arg_node), nassigns);
        current_shape = GV_struct_constant;
        INFO_WLPG_NASSIGNS (arg_info)
          = AppendAssigns (nassigns, INFO_WLPG_NASSIGNS (arg_info));
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
          = AppendAssigns (nassigns, INFO_WLPG_NASSIGNS (arg_info));
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }

    current_shape = PropagateArrayConstants (&(NGEN_WIDTH (arg_node)));
    if (current_shape == GV_known_shape) {
        nassigns = CreateNewAssigns (NGEN_WIDTH (arg_node), f_def);
        NGEN_WIDTH (arg_node) = CreateStructConstant (NGEN_WIDTH (arg_node), nassigns);
        current_shape = GV_struct_constant;
        INFO_WLPG_NASSIGNS (arg_info)
          = AppendAssigns (nassigns, INFO_WLPG_NASSIGNS (arg_info));
    }
    if (gshape < current_shape) {
        gshape = current_shape;
    }

    /*
     * check bound ranges
     */
    let_ids = LET_IDS (INFO_WLPG_LET (arg_info));
    if (check_bounds && (GetShapeDim (IDS_TYPE (let_ids)) >= 0)) {
        shp = SHOldTypes2Shape (IDS_TYPE (let_ids));
        gprop = CheckGeneratorBounds (wln, shp);
        shp = SHFreeShape (shp);
        if ((gprop == GPT_full) && (NGEN_STEP (arg_node) != NULL)) {
            gprop = GPT_partial;
        }
    } else if (gshape == GV_struct_constant) {
        gprop = GPT_partial;
    } else {
        gprop = GPT_unknown;
    }

    INFO_WLPG_GENPROP (arg_info) = gprop;
    INFO_WLPG_GENSHP (arg_info) = gshape;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLPartitionGeneration( node *arg_node)
 *
 *   @brief  First transform code in SSA form. Then start the generation
 *           of full partitions and after that convert back to standard form.
 *
 *   @param  node *arg_node:  the whole syntax tree
 *   @return node *        :  the transformed syntax tree
 ******************************************************************************/

node *
WLPartitionGeneration (node *arg_node)
{
    funtab *tmp_tab;
    node *arg_info;

    DBUG_ENTER ("WLPartitionGeneration");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "WLPartitionGeneration not started with modul node");

    DBUG_PRINT ("WLPG", ("starting WLPartitionGeneration"));

    /* transformation in ssa-form */
    arg_node = DoSSA (arg_node);
    /* necessary to guarantee, that the compilation can be stopped
       during the call of DoSSA */
    if ((break_after == PH_sacopt)
        && ((0 == strcmp (break_specifier, "l2f"))
            || (0 == strcmp (break_specifier, "cha"))
            || (0 == strcmp (break_specifier, "ssa")))) {
        goto DONE;
    }

    arg_info = MakeInfo ();

    tmp_tab = act_tab;
    act_tab = wlpg_tab;

    arg_node = Trav (arg_node, arg_info);

    arg_info = FreeTree (arg_info);
    act_tab = tmp_tab;

    /* undo tranformation in ssa-form */
    arg_node = UndoSSA (arg_node);
    /* necessary to guarantee, that the compilation can be stopped
       during the call of UndoSSA */
    if ((break_after == PH_sacopt)
        && ((0 == strcmp (break_specifier, "ussa"))
            || (0 == strcmp (break_specifier, "f2l")))) {
        goto DONE;
    }

DONE:
    DBUG_RETURN (arg_node);
}
