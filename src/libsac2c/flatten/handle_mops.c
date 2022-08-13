#include "handle_mops.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "free.h"
#include "LookUpTable.h"
#include "globals.h"

static lut_t *prec_lut;

typedef enum { Ass_l, Ass_r, Ass_n } assoc_t;

typedef struct PREC_T {
    assoc_t assoc;
    int prec;
} prec_t;

#define PREC_ASS(n) ((n)->assoc)
#define PREC_VAL(n) ((n)->prec)

/******************************************************************************
 ***
 ***                   Some static helper functions:
 ***                   -----------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    prec_t *MakePrec( assoc_t assoc, int val)
 *
 * description:
 *    ...
 *
 ******************************************************************************/

static prec_t *
MakePrec (assoc_t assoc, int val)
{
    prec_t *res;

    DBUG_ENTER ();
    res = (prec_t *)MEMmalloc (sizeof (prec_t));

    PREC_ASS (res) = assoc;
    PREC_VAL (res) = val;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    prec_t *FreePrecInLUT( prec_t * prec, void * ignored)
 *
 * description:
 *    Free PREC structure via LUTmap* functions - second argument is unused
 *    but obligatory with LUTmap* function.
 *
 ******************************************************************************/

static prec_t *
FreePrecInLUT (prec_t *prec, void *ignored)
{
    DBUG_ENTER ();

    prec = MEMfree (prec);

    DBUG_RETURN (prec);
}

/******************************************************************************
 *
 * function:
 *    lut_t *InitPrecLut()
 *
 * description:
 *    ...
 *
 ******************************************************************************/

static lut_t *
InitPrecLut (void)
{
    lut_t *res;

    DBUG_ENTER ();

    res = LUTgenerateLut ();

    res = LUTinsertIntoLutS (res, "*", MakePrec (Ass_l, 12));
    res = LUTinsertIntoLutS (res, "/", MakePrec (Ass_l, 12));
    res = LUTinsertIntoLutS (res, "%", MakePrec (Ass_l, 12));
    res = LUTinsertIntoLutS (res, "+", MakePrec (Ass_l, 11));
    res = LUTinsertIntoLutS (res, "-", MakePrec (Ass_l, 11));
    res = LUTinsertIntoLutS (res, "<", MakePrec (Ass_l, 9));
    res = LUTinsertIntoLutS (res, "<=", MakePrec (Ass_l, 9));
    res = LUTinsertIntoLutS (res, ">", MakePrec (Ass_l, 9));
    res = LUTinsertIntoLutS (res, ">=", MakePrec (Ass_l, 9));
    res = LUTinsertIntoLutS (res, "==", MakePrec (Ass_l, 8));
    res = LUTinsertIntoLutS (res, "!=", MakePrec (Ass_l, 8));
    res = LUTinsertIntoLutS (res, "&&", MakePrec (Ass_l, 4));
    res = LUTinsertIntoLutS (res, "||", MakePrec (Ass_l, 3));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    bool LeftAssoc( node *lop, node *rop)
 *
 * description:
 *    ...
 *
 ******************************************************************************/

static bool
LeftAssoc (node *lop, node *rop)
{
    prec_t **prec_p;
    prec_t *prec1, *prec2;
    bool res;

    static prec_t default_prec = {Ass_n, 0};

    DBUG_ENTER ();

    prec_p = (prec_t **)LUTsearchInLutS (prec_lut, SPID_NAME (lop));
    if (prec_p == NULL) {
        prec1 = &default_prec; /* no precedence found */
    } else {
        prec1 = *prec_p;
    }

    prec_p = (prec_t **)LUTsearchInLutS (prec_lut, SPID_NAME (rop));
    if (prec_p == NULL) {
        prec2 = &default_prec; /* no precedence found */
    } else {
        prec2 = *prec_p;
    }

    if (PREC_VAL (prec1) == PREC_VAL (prec2)) {
        if (PREC_ASS (prec1) == PREC_ASS (prec2)) {
            res = (PREC_ASS (prec1) == Ass_l);
        } else {
            CTIwarn (LINE_TO_LOC (global.linenum),
                     "Infix function application with non-unique precedence: "
                     "choosing left associativity");
            res = TRUE;
        }
    } else {
        res = (PREC_VAL (prec1) > PREC_VAL (prec2));
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    node *Mop2Ap( ids *op, node *mop)
 *
 * description:
 *   Transformes a N_mop node into a nesting of N_spap nodes according to the
 *   following algorithm (e's are expressions; o's are operations):
 *
 *   e1                   e1
 *   ---              =>  ---
 *
 *   e1 : e2              /             / e1 : e2 \                 iff prec(op)
 *      o1            =>  |             \    o1   /                   > prec(o1)
 *                        |
 *                        | Mop2Ap( op, / Ap{o1,e1,e2} \ )          iff prec(o1)
 *                        \             \    ---       /              > prec(o2)
 *
 *   e1 : e2 : es         /             / e1 : e2 : es    \         iff prec(op)
 *      o1 : o2 : os  =>  |             \    o1 : o2 : os /           > prec(o1)
 *                        |
 *                        | Mop2Ap( op, / Ap{o1,e1,e2} : es    \ )  iff prec(o1)
 *                        |             \              o2 : os /      > prec(o2)
 *                        |
 *                        | Mop2Ap( op, / Ap{o1,e1,e1'} : es' \ )   otherwise
 *                        |             \               os'   /
 *                        |   where
 *                        |         / e1' : es' \ = Mop2Ap( o1, / e2 : es    \ )
 *                        \         \     os'   /               \    o2 : os /
 *
 ******************************************************************************/

static node *
Mop2Ap (node *op, node *mop)
{
    node *ap, *exprs, *exprs3, *exprs_prime;
    node *fun_ids;

    DBUG_ENTER ();

    exprs = SPMOP_EXPRS (mop);
    fun_ids = SPMOP_OPS (mop);

    if ((fun_ids != NULL) && ((op == NULL) || (!LeftAssoc (op, EXPRS_EXPR (fun_ids))))) {
        /*
         * there is at least one operation left  -AND-
         * 'op' is weaker (or not present at all) than 'o1'
         *    -> recursive call needed
         */
        if ((EXPRS_NEXT (fun_ids) == NULL)
            || LeftAssoc (EXPRS_EXPR (fun_ids), EXPRS_EXPR (EXPRS_NEXT (fun_ids)))) {
            /*
             * 'o2' is weaker (or not present at all) than 'o1'
             *    -> build  Ap{o1,e1,e2}
             */
            exprs3 = EXPRS_EXPRS3 (exprs);
            EXPRS_EXPRS3 (exprs) = NULL;

            ap = TBmakeSpap (EXPRS_EXPR (fun_ids), exprs);

            SPMOP_EXPRS (mop) = TBmakeExprs (ap, exprs3);

            /* we have reused the id stored in the mop, so do not free it */
            EXPRS_EXPR (fun_ids) = NULL;
            SPMOP_OPS (mop) = fun_ids = FREEdoFreeNode (fun_ids);

            mop = Mop2Ap (op, mop);
        } else {
            /*
             * be careful: when cutting a N_exprs chain
             * into several parts, always make sure that the
             * NEXT pointer is zero on both ends of the
             * resulting chains. This is important as the
             * pointer the NEXT points to may be freed.
             * Although this code never uses this NEXT pointer,
             * the traversal mechanism does, which leads
             * to an memory access into an already freed
             * heap area.
             */
            SPMOP_EXPRS (mop) = EXPRS_NEXT (exprs);
            EXPRS_NEXT (exprs) = NULL;
            SPMOP_OPS (mop) = EXPRS_NEXT (fun_ids);
            EXPRS_NEXT (fun_ids) = NULL;
            mop = Mop2Ap (EXPRS_EXPR (fun_ids), mop); /* where clause! */

            exprs_prime = SPMOP_EXPRS (mop);

            ap = TBmakeSpap (EXPRS_EXPR (fun_ids),
                             TBmakeExprs (EXPRS_EXPR (exprs),
                                          TBmakeExprs (EXPRS_EXPR (exprs_prime), NULL)));

            EXPRS_EXPR (exprs_prime) = ap;

            mop = Mop2Ap (op, mop);

            /*
             * we have used the expressing stored in the exprs chain,
             * so do not free it
             */
            EXPRS_EXPR (exprs) = NULL;
            exprs = FREEdoFreeNode (exprs);

            /* we have reused the id stored in the mop, so do not free it */
            EXPRS_EXPR (fun_ids) = NULL;
            EXPRS_NEXT (fun_ids) = NULL;
            fun_ids = FREEdoFreeNode (fun_ids);
        }
    } else {
        /*
         * 'mop' is a single expression  -OR-  'op' is stronger than 'o1'
         *    -> do nothing
         */
    }

    DBUG_RETURN (mop);
}

/******************************************************************************
 ***
 ***      Here, the exported functions for handling N_mop nodes:
 ***      ------------------------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    node *HMdoHandleMops( node *arg_node)
 *
 * description:
 *    starts the elimination of N_mop nodes
 *
 ******************************************************************************/

node *
HMdoHandleMops (node *arg_node)
{
    DBUG_ENTER ();

    prec_lut = InitPrecLut ();

    TRAVpush (TR_hm);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    prec_lut = LUTmapLutS (prec_lut, (void *(*)(void *, void *))FreePrecInLUT);
    prec_lut = LUTremoveLut (prec_lut);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *HMspmop( node *arg_node, info *arg_info)
 *
 * description:
 *    Converts a N_mop node into a nested N_ap node.
 *
 ******************************************************************************/

node *
HMspmop (node *arg_node, info *arg_info)
{
    node *mop, *res;

    DBUG_ENTER ();

    mop = Mop2Ap (NULL, arg_node);
    DBUG_ASSERT (((mop != NULL) && (NODE_TYPE (mop) == N_spmop)
                  && (SPMOP_OPS (mop) == NULL) && (SPMOP_EXPRS (mop) != NULL)
                  && (EXPRS_NEXT (SPMOP_EXPRS (mop)) == NULL)),
                 "illegal result of Mop2Ap() found!");
    res = EXPRS_EXPR (SPMOP_EXPRS (mop));
    EXPRS_EXPR (SPMOP_EXPRS (mop)) = NULL;
    mop = FREEdoFreeTree (mop);

    res = TRAVdo (res, arg_info);

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
