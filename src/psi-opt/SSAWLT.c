/*
 *
 * $Log$
 * Revision 1.26  2003/11/28 09:33:41  sbs
 * major code rewrite. Now, code inspection and code modification are better separated.
 * However, since the tricky code modification sections are copied from the old
 * code, some functions are stll pretty ugly. Furthermore, CreateFullPartition
 * contains some redundancy as the "full range" case is taken care of separately.
 * The major contribution of the code is the proper (hopefully 8-) treatment
 * of the default case in genarray WLs!!!
 *
 * Revision 1.25  2003/09/25 18:37:16  dkr
 * some code brushing done
 *
 * Revision 1.24  2003/04/14 14:53:41  sbs
 * initialization of genshape adjusted to its type, i.e., to int 8-)
 *
 * Revision 1.23  2003/04/11 17:48:35  sbs
 * genshape in CreateFullPartition initialized to NULL, just to please gcc 8-)
 *
 * Revision 1.22  2003/04/10 15:43:50  dkr
 * bug in CheckOptimizeArray() fixed
 *
 * Revision 1.21  2003/04/10 12:02:01  dkr
 * CreateFullPartition(): code reordered. the checks which determine the
 * correct value for the variable do_create are performed in correct
 * order now.
 *
 * Revision 1.20  2003/03/14 13:19:32  dkr
 * SSAWLTNwithop(): NWITHOP_ARRAY no longer inlined
 *
 * Revision 1.19  2003/03/13 00:06:42  dkr
 * SSAWLTNwithop() added
 *
 * Revision 1.18  2002/10/11 14:09:47  dkr
 * SSAWLTNgenerator() works correctly now (really!)
 *
 * Revision 1.17  2002/10/09 22:07:40  dkr
 * fixed a bug in SSAWLTNgenerator()
 *
 * Revision 1.16  2002/10/09 12:44:44  dkr
 * SSAWLTNgenerator(): *structural* constants used now
 *
 * Revision 1.15  2002/10/09 02:11:31  dkr
 * constants modul used instead of ID/ARRAY_CONSTVEC
 *
 * Revision 1.14  2002/10/08 10:33:47  dkr
 * CreateFullPartition(): modifications for dynamic shapes done
 *
 * Revision 1.11  2002/09/13 20:16:41  dkr
 * genarray-wls with empty index sets allow again... #@%&
 *
 * Revision 1.10  2002/09/13 19:05:11  dkr
 * genarray-wls with empty index sets are no longer allowed
 *
 * Revision 1.9  2002/06/27 17:01:41  dkr
 * - CheckOptimize...() works also for nested WLs now
 * - CreateSel() used now
 *
 * Revision 1.8  2002/06/25 23:58:04  ktr
 * CreateFullPartition now creates a full partition if OPT_WLS is activated.
 *
 * Revision 1.7  2002/06/21 14:03:32  dkr
 * Zero-Arrays are build correctly now (by CreateZero...())
 *
 * Revision 1.6  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 1.5  2001/05/17 13:40:26  nmw
 * MALLOC/FREE replaced by Malloc/Free, using result of Free()
 *
 * Revision 1.4  2001/05/16 19:52:47  nmw
 * reverted Free() to FREE() due to segfaults when used with linux :-(
 *
 * Revision 1.3  2001/05/16 13:41:44  nmw
 * unused old code removed, comments corrected
 * MALLOC/FREE changed to Malloc/Free
 *
 * Revision 1.2  2001/05/15 16:39:21  nmw
 * SSAWithloopFolding implemented (but not tested)
 *
 *
 * created from WLT.c, Revision 3.9 on 2001/05/14 by  nmw
 *
 */

/*
 * this implements the ssa form aware version of the original
 * WithloopTransformation implementation. it do not need any masks
 * instead it uses the advantages of the ssa form.
 * most code is unchanged from the original implementation and the
 * comments, too.
 */

/*******************************************************************************

 At the moment we cannot transform generators of WLs to other shapes (this
 could leed to more folding actions but the right transformations steps
 are not easily calculated).

 Transformations we do:
 - propagate arrays into the generator.
 - create new generators to receive a full partition of the array.
   - genarray: Only if the generator has as much elements as the array
               described by the WL.
   - modarray: always
 - replace index vectors. This transformation is not needed for WLF but
   prepares a better result for compile.
   - let iv=[i,j,k], then e.g. iv[[2]] is replaced by j,
   - let iv=[i,j,k], then [i,j,k] is replaced by iv.

 *******************************************************************************

 Usage of arg_info:
 - node[0]: NEXT   : store old information in nested WLs
 - node[1]: WL     : reference to base node of current WL (N_Nwith)
 - node[2]: ASSIGN : always the last N_assign node
 - node[3]: FUNDEF : pointer to last fundef node. needed to access vardecs.
 - node[4]: LET    : pointer to N_let node of current WL.
                     LET_EXPR(ID) == INFO_WLI_WL.
 - node[5]: REPLACE: if != NULL, replace WL with this node.

 ******************************************************************************/

#define INFO_WLT_GENPROP(n) (n->counter)

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "Error.h"
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "optimize.h"
#include "SSAConstantFolding.h"
#include "SSAWithloopFolding.h"
#include "SSAWLT.h"

typedef enum { GPT_empty, GPT_full, GPT_partial, GPT_unknown } gen_prop_t;

/******************************************************************************
 *
 * function:
 *   intern_gen *CutSlices( ...)
 *
 * description:
 *   Creates a (full) partition by adding new intern_gen structs.
 *   If the known part is a grid, this is ignored here (so the resulting
 *   intern_gen chain may still not be a full partition, see CompleteGrid()).
 *   The list of intern_gen is returned.
 *
 * parameters:
 *   ls, us : bounds of the whole array
 *   l, u   : bounds of the given part
 *   dim    : number of elements of ls, us, l, u
 *   ig     : chain of intern_gen struct where to add the new parts. If
 *            ig != NULL, the same pointer is returned.
 *   coden  : Pointer of N_Ncode node where the new generators shall point to.
 *
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

/******************************************************************************
 *
 * function:
 *   intern_gen *CompleteGrid( int *ls, int *us, int *step, int *width, int dim,
 *                             intern_gen *ig, node *coden)
 *
 * description:
 *   adds new generators which specify the elements left out by a grid.
 *
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

/******************************************************************************
 *
 * function:
 *   int check_genarray_full_part( node *wln)
 *
 * description:
 *   check whether the generator of this genarray-WL specifies a full partition
 *
 ******************************************************************************/

static bool
check_genarray_full_part (node *wln)
{
    node *lowern, *uppern, *shapen;
    bool result;

    DBUG_ENTER ("check_genarray_full_part");

    shapen = NWITH_SHAPE (wln);
    lowern = NWITH_BOUND1 (wln);
    uppern = NWITH_BOUND2 (wln);

    DBUG_ASSERT (((NODE_TYPE (lowern) == N_array) && (NODE_TYPE (uppern) == N_array)),
                 "generator bounds must be arrays!");

    if (NODE_TYPE (shapen) == N_array) {
        result = TRUE;

        /* check lower bound */
        lowern = ARRAY_AELEMS (lowern);
        while (result && lowern) {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (lowern)) == N_num),
                         "lower generator bound must be constant!");
            if (NUM_VAL (EXPRS_EXPR (lowern)) != 0) {
                result = FALSE;
            }
            lowern = EXPRS_NEXT (lowern);
        }

        /* check upper bound */
        uppern = ARRAY_AELEMS (uppern);
        shapen = ARRAY_AELEMS (shapen);
        while (result && uppern) {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (uppern)) == N_num),
                         "upper generator bound must be constant!");
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (shapen)) == N_num),
                         "shape must be constant!");
            if (NUM_VAL (EXPRS_EXPR (shapen)) != NUM_VAL (EXPRS_EXPR (uppern))) {
                result = FALSE;
            }
            uppern = EXPRS_NEXT (uppern);
            shapen = EXPRS_NEXT (shapen);
        }
    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *CreateFullPartition( node *wln, node *arg_info)
 *
 * description:
 *   generates full partition if possible:
 *    - if withop is genarray and index vector has as much elements as
 *      dimension of resulting WL (withloop on scalars).
 *    - if withop is modarray: always (needed for compilation phase).
 *   Returns wln.
 *
 * parameters:
 *   the wln is the N_Nwith node of the WL to transfrom.
 *   arg_info is needed to access the vardecs of the current function.
 *
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
    int dim, gen_shape = 0;
    bool do_create;

    DBUG_ENTER ("CreateFullPartition");

    /*
     * only if we do not have a full partition yet
     * and the index vector has known shape.
     */
    do_create
      = ((NWITH_PARTS (wln) < 0) && (GetShapeDim (IDS_TYPE (NWITH_VEC (wln))) >= 0));

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
            int i;

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
     * do some additional checks
     */
    if (do_create) {
        switch (NWITH_TYPE (wln)) {
        case WO_modarray:
            /* noop */
            break;

        case WO_genarray:
            /*
             * if the CEXPR of a genarray WL is not a scalar, we have to create
             * new parts where the CEXPR is a null-vector. Other than the
             * modarray case, we have to create this null-vector first. In general,
             * this can lead to worse code, because another WL is inserted and
             * it is not guaranteed that we can fold later. So we don't create
             * a full partition here.
             *
             * But there is a special case: If the original generator is a
             * full partition itself, we do not have to create the null-vector
             * and so can create a partition with NWITH_PARTS == 1.
             */
            if ((!NWITH_STEP (wln)) && /* no grid */
                check_genarray_full_part (wln)) {
                do_create = FALSE;
                NWITH_PARTS (wln) = 1;
            } else {
                dim = GetShapeDim (type);

                if (optimize & OPT_WLS) {
                    do_create = (dim >= 0);
                } else {
                    do_create = (dim == 0);
                }
            }
            break;

        default:
            DBUG_ASSERT ((0), "illegal NWITH_TYPE found!");
            break;
        }
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

/******************************************************************************
 *
 * function:
 *   node *CreateEmptyGenWLReplacement( node *wl, node *arg_info)
 *
 * description:
 *   computes the replacement for the given WL under the assumption that
 *   the generator is empty, i.e.,
 *
 *    with( lb <= iv < ub)              =>     with( 0*shp <= iv <shp)
 *    genarray( shp, exp, dexp)                genarray( shp, dexp, ---)
 *
 *    with( lb <= iv < ub)              =>     a
 *    modarray( a, NULL, exp)
 *
 *    with( lb <= iv < ub)              =>     neutral
 *    fold( fun, neutral, exp)
 *
 *   NOTE HERE, that wl is either MODIFIED in place or FREED entirely!!
 *
 ******************************************************************************/

node *
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
        let_ids = LET_IDS (INFO_WLI_LET (arg_info));
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
         * Now we have to change the code. Either we can use DEAFAULT (easy)
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
                type = ID_TYPE (NWITH_CEXPR (INFO_WLI_WL (arg_info)));
                IDS_VARDEC (_ids)
                  = SSACreateVardec (varname, type,
                                     &(FUNDEF_VARDEC (INFO_WLI_FUNDEF (arg_info))));
                IDS_AVIS (_ids) = VARDEC_AVIS (IDS_VARDEC (_ids));

                tmpn = CreateZeroFromType (type, FALSE, INFO_WLI_FUNDEF (arg_info));

                /* replace N_empty with new assignment "_ids = [0,..,0]" */
                assignn = MakeAssign (MakeLet (tmpn, _ids), NULL);
                BLOCK_INSTR (blockn) = assignn;

                /* set correct backref to defining assignment */
                AVIS_SSAASSIGN (IDS_AVIS (_ids)) = assignn;

                /* replace CEXPR */
                idn = MakeId (StringCopy (varname), NULL, ST_regular);
                ID_VARDEC (idn) = IDS_VARDEC (_ids);
                ID_AVIS (idn) = IDS_AVIS (_ids);
                tmpn = NWITH_CODE (INFO_WLI_WL (arg_info));
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

                tmpn = CreateZeroFromType (type, FALSE, INFO_WLI_FUNDEF (arg_info));
                NCODE_FLAG (NWITH_CODE (INFO_WLI_WL (arg_info))) = TRUE;

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

/******************************************************************************
 *
 * function:
 *   bool PropagateArrayConstants( node **expr)
 *
 * description:
 *   expects (*expr) to point either to a scalar constant, to an array,
 *   or to an identifyer.
 *   If (**expr) is constant or structural constant, the according node
 *   (N_array / scalar) is freshly generated and (*expr) is modified to point
 *   to the new node. The old one is freed!
 *   returns TRUE iff (**expr) is constant or (*expr) == NULL!!
 *
 ******************************************************************************/

static bool
PropagateArrayConstants (node **expr)
{
    constant *const_expr;
    struct_constant *sco_expr;
    bool res = FALSE;

    DBUG_ENTER ("PropagateArrayConstants");

    if ((*expr) != NULL) {
        const_expr = COAST2Constant ((*expr));
        if (const_expr != NULL) {
            res = TRUE;
            (*expr) = FreeTree (*expr);
            (*expr) = COConstant2AST (const_expr);
            const_expr = COFreeConstant (const_expr);

        } else {
            sco_expr = SCOExpr2StructConstant ((*expr));
            if (sco_expr != NULL) {
                (*expr) = FreeTree (*expr);
                (*expr) = SCODupStructConstant2Expr (sco_expr);
                sco_expr = SCOFreeStructConstant (sco_expr);
            }
        }
    } else {
        res = TRUE;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   gen_prop_t CheckGeneratorBounds( node *wl, shape *max_shp)
 *
 * description:
 *   expects the bound expression for lb and ub to be constants!
 *   checks, whether these fit into the shape given by max_shp.
 *   If they exceed max_shp, they are "fitted"!
 *   Furthermore, it is checked whether the generator is empty (GPT_empty is
 *   returned) or may cover the entire range (GPT_full is returned).
 *   Otherwise GPT_partial is returned.
 *
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
        }

        dim++;
        lbe = EXPRS_NEXT (lbe);
        ube = EXPRS_NEXT (ube);
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *CheckOptimizeSel( node *sel, node *arg_info)
 *
 * description:
 *   checks if 'sel' is an application with index vector and constant. If
 *   possible, the prf sel is replaced by A scalar index vector.
 *
 * example:
 *   tmp = iv[[1]];       =>        tmp = j;
 *
 ******************************************************************************/

static node *
CheckOptimizeSel (node *sel, node *arg_info)
{
    int index;
    node *ivn, *indexn, *datan;
    ids *_ids;

    DBUG_ENTER ("CheckOptimizeSel");

    while ((arg_info != NULL) && (INFO_WLI_WL (arg_info) != NULL)) {
        /* first check if the array is the index vector and the index is a
           constant in range. */
        ivn = PRF_ARG2 (sel);
        indexn = PRF_ARG1 (sel);

        if (N_id == NODE_TYPE (indexn)) {
            /* resolve id by accessing SSAASSIGN attribute */
            if (AVIS_SSAASSIGN (ID_AVIS (indexn)) != NULL) {
                datan = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (indexn)));
            } else {
                /* unknown definition (arg) */
                datan = NULL;
            }
        } else {
            datan = indexn;
        }

        if (datan && (N_array == NODE_TYPE (datan))
            && (N_num == NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (datan))))
            && (-1 == SSALocateIndexVar (ivn, INFO_WLI_WL (arg_info)))) {
            index = NUM_VAL (EXPRS_EXPR (ARRAY_AELEMS (datan)));

            /* find index'th scalar index var */
            _ids = NWITH_IDS (INFO_WLI_WL (arg_info));
            while (index > 0 && IDS_NEXT (_ids)) {
                index--;
                _ids = IDS_NEXT (_ids);
            }

            if (!index) { /* found scalar index var. */
                sel = FreeTree (sel);
                sel = MakeId (StringCopy (IDS_NAME (_ids)), NULL, ST_regular);
                ID_VARDEC (sel) = IDS_VARDEC (_ids);
                ID_AVIS (sel) = IDS_AVIS (_ids);
                wlt_expr++;
                break;
            }
        }

        arg_info = INFO_WLI_NEXT (arg_info);
    }

    DBUG_RETURN (sel);
}

/******************************************************************************
 *
 * function:
 *   node *CheckOptimizeArray( node *array, node *arg_info)
 *
 * description:
 *   Checks if array is constructed from all scalar index variables
 *   with permutation == identity. Then the array subtree is replaced
 *   by the index vector.
 *
 * example:
 *   sel = [i,j,k];       =>       sel = iv;
 *
 ******************************************************************************/

static node *
CheckOptimizeArray (node *array, node *arg_info)
{
    int elts;
    ids *_ids;
    node *tmpn;
    char *vec_name;

    DBUG_ENTER ("CheckOptimizeArray");

    DBUG_ASSERT ((N_array == NODE_TYPE (array)), "no N_array node");

    while ((arg_info != NULL) && (INFO_WLI_WL (arg_info) != NULL)) {
        /* shape of index vector */
        _ids = NWITH_VEC (INFO_WLI_WL (arg_info));

        if (GetShapeDim (IDS_TYPE (_ids)) >= 0) {
            tmpn = ARRAY_AELEMS (array);
            elts = 0;
            while (tmpn) {
                if ((N_id == NODE_TYPE (EXPRS_EXPR (tmpn)))
                    && (SSALocateIndexVar (EXPRS_EXPR (tmpn), INFO_WLI_WL (arg_info))
                        == elts + 1)) {
                    elts++;
                    tmpn = EXPRS_NEXT (tmpn);
                } else {
                    tmpn = NULL;
                    elts = (-1);
                }
            }

            if (elts == IDS_SHAPE (_ids, 0)) { /* change to index vector */
                /* free subtree and make new id node. */
                array = FreeTree (array);
                vec_name = StringCopy (IDS_NAME (_ids));
                array = MakeId (vec_name, NULL, ST_regular);
                ID_VARDEC (array) = IDS_VARDEC (_ids);
                ID_AVIS (array) = IDS_AVIS (_ids);
                wlt_expr++;
                break;
            }
        }

        arg_info = INFO_WLI_NEXT (arg_info);
    }

    DBUG_RETURN (array);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLTfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   starts the traversal of the given fundef
 *
 ******************************************************************************/

node *
SSAWLTfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAWLTfundef");

    INFO_WLI_WL (arg_info) = NULL;
    INFO_WLI_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node)) {
        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLTassign(node *arg_node, node *arg_info)
 *
 * description:
 *   store actual assign node in arg_info and traverse instruction
 *
 ******************************************************************************/

node *
SSAWLTassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAWLTassign");

    INFO_WLI_ASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLTcond(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse sons in the given order, can possibly replaced by TravSons
 *
 ******************************************************************************/

node *
SSAWLTcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAWLTcond");

    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
    COND_THENINSTR (arg_node) = Trav (COND_THENINSTR (arg_node), arg_info);
    COND_ELSEINSTR (arg_node) = Trav (COND_ELSEINSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLTap(node *arg_node, node *arg_info)
 *
 * description:
 *   1. traverse args
 *   2. traverse in applicated fundef if it is a special one.
 *
 ******************************************************************************/

node *
SSAWLTap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("SSAWLTap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* non-recursive call of special fundef */
    if ((AP_FUNDEF (arg_node) != NULL) && (FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (INFO_WLI_FUNDEF (arg_info) != AP_FUNDEF (arg_node))) {

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        new_arg_info = FreeTree (new_arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLTlet(node *arg_node, node *arg_info)
 *
 * description:
 *   the following occurences are searched and replaced (iv=[i,j,k]):
 *   - constant indexing of index vector:
 *     iv[[1]] is replaces by j
 *   - construction of iv with index scalars:
 *     [i,j,k] is replaced by iv.
 *
 *   These are optimizations for the compiler phase and are not needed for WLF.
 *
 ******************************************************************************/

node *
SSAWLTlet (node *arg_node, node *arg_info)
{
    node *tmpn;

    DBUG_ENTER ("SSAWLTlet");

    if (INFO_WLI_WL (arg_info)) {
        /*
         * if we are inside a WL we have to look for
         *   1:  ..= sel(a1,a2)  where a1 may be a const vec and a2 iv.
         *   2:  ..= prf(a1,a2)  where a1 and a2 may be [i,j,k]
         *   3:  ..= expr        where expr may be [i,j,k]
         */

        if ((N_prf == NODE_TYPE (LET_EXPR (arg_node)))
            && (F_sel == PRF_PRF (LET_EXPR (arg_node)))) {
            /*
             * 1:
             */
            LET_EXPR (arg_node) = CheckOptimizeSel (LET_EXPR (arg_node), arg_info);
        }

        if (N_prf == NODE_TYPE (LET_EXPR (arg_node))) {
            /*
             * 2:
             */
            tmpn = PRF_ARGS (LET_EXPR (arg_node));
            while (tmpn != NULL) {
                if (N_array == NODE_TYPE (EXPRS_EXPR (tmpn))) {
                    EXPRS_EXPR (tmpn) = CheckOptimizeArray (EXPRS_EXPR (tmpn), arg_info);
                }
                tmpn = EXPRS_NEXT (tmpn);
            }
        }

        if (N_array == NODE_TYPE (LET_EXPR (arg_node))) {
            /*
             * 3:
             */
            LET_EXPR (arg_node) = CheckOptimizeArray (LET_EXPR (arg_node), arg_info);
        }
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLTNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   start traversal of this WL and store information in new arg_info node.
 *   All N_Npart nodes (inclusive bodies) are traversed.
 *   Afterwards, if WL is foldable and certain conditions are fulfilled,
 *   the WL is transformed into a WL with generators describing a full
 *   partition.
 *
 ******************************************************************************/

node *
SSAWLTNwith (node *arg_node, node *arg_info)
{
    node *tmpn;
    bool replace_wl = FALSE;

    DBUG_ENTER ("SSAWLTNwith");

    /**
     * Once a full partition has been created, we do not need to inspect
     * the generators anymore. The indicator for this is the PARTS attribute.
     * Initially, it is set to -1; if we have successfully generated a full
     * partition, it carries a positive value.
     * However, the WITHOP sons and the CODEs have to be inspected anyways.
     * The WITHOP sons have to be inspected because they may allow for some
     * constant propagation (to be utilized later).
     * The CODEs have to be traversed as they may contain further (nested) WLs.
     */
    if (NWITH_PARTS (arg_node) == -1) {
        /*
         * initialize WL traversal
         */
        INFO_WLI_WL (arg_info) = arg_node; /* store the current node for later */
        INFO_WLI_LET (arg_info) = ASSIGN_INSTR (INFO_WLI_ASSIGN (arg_info));

        NWITH_FOLDABLE (arg_node) = TRUE;

        /*
         * traverse the one and only (!) PART.
         * Besides minor changes in the generator, two values are computed
         * during this traversal:
         *
         *  NWITH_FOLDABLE( arg_node)   and  INFO_WLT_GENPROP(arg_info) !!
         */
        if (NWITH_PART (arg_node) != NULL) {
            DBUG_ASSERT ((NPART_NEXT (NWITH_PART (arg_node)) == NULL),
                         "PARTS is -1 although more than one PART is available!");
            NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
        }

        if (INFO_WLT_GENPROP (arg_info) == GPT_empty) {
            arg_node = CreateEmptyGenWLReplacement (arg_node, arg_info);
            replace_wl = TRUE;

        } else if (INFO_WLT_GENPROP (arg_info) == GPT_full) {
            NWITH_PARTS (arg_node) = 1;
            if (NWITH_TYPE (arg_node) == WO_genarray) {
                if (NWITH_DEFAULT (arg_node) != NULL) {
                    NWITH_DEFAULT (arg_node) = FreeTree (NWITH_DEFAULT (arg_node));
                }
            }

        } else if ((INFO_WLT_GENPROP (arg_info) == GPT_partial)
                   && NWITH_FOLDABLE (arg_node)) {
            arg_node = CreateFullPartition (arg_node, arg_info);
        }
    }

    if (!replace_wl) { /* The WL still exists 8-) */
        /*
         * Now, we traverse the WITHOP sons for propagating (structural) constants.
         */
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

        /*
         * Finally, we check for "inner" WLs, i.e., we traverse the codes.
         * As the index vector optimizations need to have access to ALL genvars,
         * we have to stack the arg_info before traversing the code chain!
         */
        tmpn = MakeInfo ();
        INFO_WLI_FUNDEF (tmpn) = INFO_WLI_FUNDEF (arg_info);
        INFO_WLI_ASSIGN (tmpn) = INFO_WLI_ASSIGN (arg_info);
        INFO_WLI_NEXT (tmpn) = arg_info;
        arg_info = tmpn;

        if (NWITH_CODE (arg_node) != NULL) {
            NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
        }

        /*
         * pop arg_info again!
         */
        tmpn = arg_info;
        arg_info = INFO_WLI_NEXT (arg_info);
        tmpn = Free (tmpn);
    }

    INFO_WLI_WL (arg_info) = NULL;
    INFO_WLI_LET (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLTNwithop( node *arg_node, node *arg_info)
 *
 * description:
 *   Substitutes NWITHOP_SHAPE into the N_Nwithop node.
 *
 ******************************************************************************/

node *
SSAWLTNwithop (node *arg_node, node *arg_info)
{
    bool is_const;

    DBUG_ENTER ("SSAWLTNwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        if (NWITHOP_SHAPE (arg_node) != NULL) {
            NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        }
        is_const = PropagateArrayConstants (&(NWITHOP_SHAPE (arg_node)));
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

/******************************************************************************
 *
 * function:
 *   node *SSAWLTNpart(node *arg_node, node *arg_info)
 *
 * description:
 *   1. traverse generator to propagate arrays,
 *   2. traverse appropriate body.
 *
 ******************************************************************************/

node *
SSAWLTNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAWLTNpart");

    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    if (NPART_NEXT (arg_node) != NULL) {
        NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLTNgenerator(node *arg_node, node *arg_info)
 *
 * description:
 *   bounds, step and width vectors are substituted into the generator.
 *   If any son is not a constant vector the N_Nwith attribut FOLDABLE is set
 *   to FALSE.
 *   Generators that surmount the array bounds (like [-5,3] or [11,10] >
 *   [maxdim1,maxdim2] = [10,10]) are changed to fitting gens.
 *   Via INFO_WLT_GENPROP( arg_info) the status of the generator is
 *   returned. Possible values are (poss. ambiguities are resolved top
 *   to bottom):
 *     GPT_empty   : the generator is empty!
 *     GPT_full    : the generator covers the entire range!
 *     GPT_partial : the generator has constant upper and lower bounds,
 *                   but  - most likely  - only a part is covered!
 *     GPT_unknown : we don't know anything !
 *
 ******************************************************************************/

node *
SSAWLTNgenerator (node *arg_node, node *arg_info)
{
    node *wln;
    ids *let_ids;
    shape *shp;
    bool is_const, check_bounds, foldable;
    gen_prop_t res;

    DBUG_ENTER ("SSAWLTNgenerator");

    wln = INFO_WLI_WL (arg_info);
    /*
     * First, we try to propagate (structural) constants into all sons:
     */
    check_bounds = TRUE;
    is_const = PropagateArrayConstants (&(NGEN_BOUND1 (arg_node)));
    check_bounds = (check_bounds && is_const);

    is_const = PropagateArrayConstants (&(NGEN_BOUND2 (arg_node)));
    check_bounds = (check_bounds && is_const);

    foldable = check_bounds;
    is_const = PropagateArrayConstants (&(NGEN_STEP (arg_node)));
    foldable = (foldable && is_const);

    is_const = PropagateArrayConstants (&(NGEN_WIDTH (arg_node)));
    foldable = (foldable && is_const);

    NWITH_FOLDABLE (wln) = foldable;

    /*
     * check bound ranges
     */
    let_ids = LET_IDS (INFO_WLI_LET (arg_info));
    if (check_bounds && (GetShapeDim (IDS_TYPE (let_ids)) >= 0)) {
        shp = SHOldTypes2Shape (IDS_TYPE (let_ids));
        res = CheckGeneratorBounds (wln, shp);
        shp = SHFreeShape (shp);
        if ((res == GPT_full) && (NGEN_STEP (arg_node) != NULL)) {
            res = GPT_partial;
        }
    } else {
        res = GPT_unknown;
    }

    INFO_WLT_GENPROP (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAWLTNcode( node *arg_node, node *arg_info)
 *
 * description:
 *   marks this N_Ncode node as processed and enters the code block.
 *
 ******************************************************************************/

node *
SSAWLTNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAWLTNcode");

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
