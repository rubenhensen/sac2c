/*
 *
 * $Log$
 * Revision 1.22  2005/06/09 10:47:36  sbs
 * EXPRS_EXPR forgotten in recursive call of Mop2Ap
 *
 * Revision 1.21  2005/01/11 11:19:19  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.20  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 1.19  2004/11/25 14:17:53  khf
 * SacDevCamp04
 *
 * Revision 1.18  2004/11/25 10:58:49  khf
 * SacDecCamp04: COMPILES!
 *
 * Revision 1.17  2004/07/16 14:41:34  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.16  2002/09/13 14:22:57  dkr
 * Mop2Ap() corrected
 *
 * Revision 1.15  2002/09/13 12:47:58  sbs
 * Mop2Ap redone...
 *
 * Revision 1.14  2002/09/11 23:22:47  dkr
 * HMAdjustFunNames() removed
 *
 * Revision 1.13  2002/09/09 19:31:30  dkr
 * prf_name_str renamed into prf_name_string
 *
 * Revision 1.12  2002/09/03 15:27:16  sbs
 * F_mod supported.
 *
 * Revision 1.11  2002/08/15 11:46:26  dkr
 * function ApplyToEach_S() renamed into MapLUT_S()
 *
 * Revision 1.10  2002/08/14 13:49:43  sbs
 * handling of unary minus for old type checker adopted
 * more closely to the old solution 8-(((
 *
 * Revision 1.9  2002/08/14 12:09:17  sbs
 * HMap transforma unary minus for old tc better now....
 *
 * Revision 1.8  2002/08/14 11:51:08  sbs
 * HMAdjustFunNames debugged....
 *
 * Revision 1.7  2002/08/14 09:27:41  sbs
 * HMNwithop debugged and calls to TravSons inserted.
 *
 * Revision 1.6  2002/08/13 17:17:24  sbs
 * bug eliminated
 *
 * Revision 1.5  2002/08/13 17:14:17  sbs
 * HMfundef changed into HMAdjustFundef
 *
 * Revision 1.3  2002/08/13 15:16:11  sbs
 * now, unary +, - are handles in the same way the old ugly
 * type checker wants it to be ....
 *
 * Revision 1.2  2002/08/13 14:40:14  sbs
 * HMNwithop added.
 *
 * Revision 1.1  2002/08/13 10:22:39  sbs
 * Initial revision
 *
 */

#define NEW_INFO

#include "handle_mops.h"
#include "traverse.h"
#include "node_basic.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "ctinfo.h"
#include "dbug.h"
#include "free.h"
#include "LookUpTable.h"

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

    DBUG_ENTER ("MakePrec");
    res = (prec_t *)ILIBmalloc (sizeof (prec_t));

    PREC_ASS (res) = assoc;
    PREC_VAL (res) = val;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    prec_t *FreePrec( prec_t * prec)
 *
 * description:
 *    ...
 *
 ******************************************************************************/

static prec_t *
FreePrec (prec_t *prec)
{
    DBUG_ENTER ("FreePrec");

    prec = ILIBfree (prec);

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
InitPrecLut ()
{
    lut_t *res;

    DBUG_ENTER ("InitPrecLut");

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

    DBUG_ENTER ("LeftAssoc");

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
            CTIwarnLine (global.linenum,
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

    DBUG_ENTER ("Mop2Ap");

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
            EXPRS_EXPR (SPMOP_OPS (mop)) = NULL;
            SPMOP_OPS (mop) = FREEdoFreeNode (fun_ids);

            mop = Mop2Ap (op, mop);
        } else {
            SPMOP_EXPRS (mop) = EXPRS_NEXT (exprs);
            SPMOP_OPS (mop) = EXPRS_NEXT (fun_ids);
            mop = Mop2Ap (EXPRS_EXPR (fun_ids), mop); /* where clause! */

            exprs_prime = SPMOP_EXPRS (mop);

            ap = TBmakeSpap (EXPRS_EXPR (fun_ids),
                             TBmakeExprs (EXPRS_EXPR (exprs),
                                          TBmakeExprs (EXPRS_EXPR (exprs_prime), NULL)));

            EXPRS_EXPR (exprs_prime) = ap;

            mop = Mop2Ap (op, mop);

            EXPRS_EXPR (exprs) = NULL;
            exprs = FREEdoFreeNode (exprs);

            /* we have reused the id stored in the mop, so do not free it */
            EXPRS_EXPR (fun_ids) = NULL;
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
    DBUG_ENTER ("HMdoHandleMops");

    prec_lut = InitPrecLut ();

    TRAVpush (TR_hm);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    prec_lut = LUTmapLutS (prec_lut, (void *(*)(void *))FreePrec);
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

    DBUG_ENTER ("HMspmop");

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
