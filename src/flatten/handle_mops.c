/*
 *
 * $Log$
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

#include "handle_mops.h"
#include "traverse.h"
#include "dbug.h"
#include "free.h"
#include "LookUpTable.h"

static LUT_t prec_lut;

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
    res = (prec_t *)Malloc (sizeof (prec_t));

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

    prec = Free (prec);

    DBUG_RETURN (prec);
}

/******************************************************************************
 *
 * function:
 *    LUT_t InitPrecLut()
 *
 * description:
 *    ...
 *
 ******************************************************************************/

static LUT_t
InitPrecLut ()
{
    LUT_t res;

    DBUG_ENTER ("InitPrecLut");

    res = GenerateLUT ();

    res = InsertIntoLUT_S (res, "*", MakePrec (Ass_l, 12));
    res = InsertIntoLUT_S (res, "/", MakePrec (Ass_l, 12));
    res = InsertIntoLUT_S (res, "%", MakePrec (Ass_l, 12));
    res = InsertIntoLUT_S (res, "+", MakePrec (Ass_l, 11));
    res = InsertIntoLUT_S (res, "-", MakePrec (Ass_l, 11));
    res = InsertIntoLUT_S (res, "<", MakePrec (Ass_l, 9));
    res = InsertIntoLUT_S (res, "<=", MakePrec (Ass_l, 9));
    res = InsertIntoLUT_S (res, ">", MakePrec (Ass_l, 9));
    res = InsertIntoLUT_S (res, ">=", MakePrec (Ass_l, 9));
    res = InsertIntoLUT_S (res, "==", MakePrec (Ass_l, 8));
    res = InsertIntoLUT_S (res, "!=", MakePrec (Ass_l, 8));
    res = InsertIntoLUT_S (res, "&&", MakePrec (Ass_l, 4));
    res = InsertIntoLUT_S (res, "||", MakePrec (Ass_l, 3));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    bool LeftAssoc( ids *lop, ids *rop)
 *
 * description:
 *    ...
 *
 ******************************************************************************/

static bool
LeftAssoc (ids *lop, ids *rop)
{
    prec_t **prec_p;
    prec_t *prec1, *prec2;
    bool res;

    static prec_t default_prec = {Ass_n, 0};

    DBUG_ENTER ("LeftAssoc");

    prec_p = (prec_t **)SearchInLUT_S (prec_lut, IDS_NAME (lop));
    if (prec_p == NULL) {
        prec1 = &default_prec; /* no precedence found */
    } else {
        prec1 = *prec_p;
    }

    prec_p = (prec_t **)SearchInLUT_S (prec_lut, IDS_NAME (rop));
    if (prec_p == NULL) {
        prec2 = &default_prec; /* no precedence found */
    } else {
        prec2 = *prec_p;
    }

    if (PREC_VAL (prec1) == PREC_VAL (prec2)) {
        if (PREC_ASS (prec1) == PREC_ASS (prec2)) {
            res = (PREC_ASS (prec1) == Ass_l);
        } else {
            WARN (linenum, ("infix function application with non-unique precedence;"
                            "choosing left associativity"));
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
 *    node * Mop2Ap( ids * op, node *mop)
 *
 * description:
 *   Transformes a N_mop node into a nesting of N_ap nodes according to the
 *   following algorithm (e's are expressions; o's are operations):
 *
 *   prec(op) > prec(o1)  =>  mop
 *
 *   e1 : e2               /  Ap{ o1, e1, e2} \
 *      o1            =>   \     ----         /
 *
 *   e1 : e2 : es         /              / Ap{ o1, e1, e2} : es     \
 *      o1 : o2 : os  =>  |  Mop2Ap( op, \                 o2 : os  / ) iff prec(o1)
 *                        |                                                 > prec(o2)
 *                        |              / Ap{ o1, e1, e1`} : es`  \
 *                        |  Mop2Ap( op, \                 os`     /  ) otherwise
 *                        |  where
 *                        |     /  e1`:es` \                 / e2:es   \
 *                        \     \     os`  /   = Mop2Ap( o1, \   o2:os /
 *
 ******************************************************************************/

static node *
Mop2Ap (ids *op, node *mop)
{
    node *res, *exprs, *new_expr, *exprs3, *exprs_prime;
    ids *fun_ids;
    char *name, *mod;

    DBUG_ENTER ("Mop2Ap");

    exprs = MOP_EXPRS (mop);
    fun_ids = MOP_OPS (mop);

    if (!LeftAssoc (op, fun_ids)) {
        if (IDS_NEXT (fun_ids) == NULL) {
            /* there is just one operation left */
            res = MakeAp (StringCopy (IDS_NAME (fun_ids)), StringCopy (IDS_MOD (fun_ids)),
                          exprs);
            MOP_OPS (mop) = FreeAllIds (MOP_OPS (mop));
            MOP_EXPRS (mop) = res;
        } else {
            /* we do have at least two operations */
            if (LeftAssoc (fun_ids, IDS_NEXT (fun_ids))) {
                exprs3 = EXPRS_EXPRS3 (exprs);
                EXPRS_EXPRS3 (exprs) = NULL;

                new_expr = MakeAp (StringCopy (IDS_NAME (fun_ids)),
                                   StringCopy (IDS_MOD (fun_ids)), exprs);

                MOP_OPS (mop) = FreeOneIds (fun_ids);
                MOP_EXPRS (mop) = MakeExprs (new_expr, exprs3);

                mop = Mop2Ap (op, mop);
            } else {
                MOP_OPS (mop) = IDS_NEXT (fun_ids);
                MOP_EXPRS (mop) = EXPRS_NEXT (exprs);
                mop = Mop2Ap (fun_ids, mop); /* where clause! */

                name = StringCopy (IDS_NAME (fun_ids));
                mod = StringCopy (IDS_MOD (fun_ids));

                exprs_prime = MOP_EXPRS (mop);

                res = MakeAp2 (name, mod, EXPRS_EXPR (exprs), EXPRS_EXPR (exprs_prime));
                EXPRS_EXPR (exprs_prime) = res;

                mop = Mop2Ap (op, mop);

                fun_ids = FreeOneIds (fun_ids);
                EXPRS_EXPR (exprs) = NULL;
                exprs = FreeNode (exprs);
            }
        }
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
 *    node *HandleMops( node *arg_node)
 *
 * description:
 *    starts the elimination of N_mop nodes
 *
 ******************************************************************************/

node *
HandleMops (node *arg_node)
{
    funtab *tmp_tab;
    node *info_node;

    DBUG_ENTER ("HandleMops");

    tmp_tab = act_tab;
    act_tab = hm_tab;

    prec_lut = InitPrecLut ();

    info_node = MakeInfo ();
    arg_node = Trav (arg_node, info_node);
    info_node = FreeNode (info_node);

    prec_lut = MapLUT_S (prec_lut, (void *(*)(void *))FreePrec);
    prec_lut = RemoveLUT (prec_lut);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *HMmop( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
HMmop (node *arg_node, node *arg_info)
{
    node *res;

    DBUG_ENTER ("HMmop");

    res = Mop2Ap (NULL, arg_node);

    res = Trav (res, arg_info);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    bool Name2Prf( char *name, prf *primfun)
 *
 * description:
 *   this function is only needed for converting the "new prf notation",
 *   which is needed for the new type checker, into the one that is
 *   required by the old type checker.....8-((
 *   Once the new TC is as powerful as the old one is, this function
 *   (and its call in flatten) becomes obsolete 8-)).
 *
 ******************************************************************************/

bool
Name2Prf (char *name, prf *primfun)
{
    bool res = TRUE;

    DBUG_ENTER ("Name2Prf");

    if (strcmp (name, "dim") == 0) {
        *primfun = F_dim;
    } else if (strcmp (name, "shape") == 0) {
        *primfun = F_shape;
    } else if (strcmp (name, "reshape") == 0) {
        *primfun = F_reshape;
    } else if (strcmp (name, "take") == 0) {
        *primfun = F_take;
    } else if (strcmp (name, "drop") == 0) {
        *primfun = F_drop;
    } else if (strcmp (name, "rotate") == 0) {
        *primfun = F_rotate;
    } else if (strcmp (name, "cat") == 0) {
        *primfun = F_cat;
    } else if (strcmp (name, "sel") == 0) {
        *primfun = F_sel;
    } else if (strcmp (name, "genarray") == 0) {
        *primfun = F_genarray;
    } else if (strcmp (name, "modarray") == 0) {
        *primfun = F_modarray;
    } else if (strcmp (name, "toi") == 0) {
        *primfun = F_toi_A;
    } else if (strcmp (name, "tof") == 0) {
        *primfun = F_tof_A;
    } else if (strcmp (name, "tod") == 0) {
        *primfun = F_tod_A;
    } else if (strcmp (name, "min") == 0) {
        *primfun = F_min;
    } else if (strcmp (name, "max") == 0) {
        *primfun = F_max;
    } else if (strcmp (name, "abs") == 0) {
        *primfun = F_abs;
    } else if (strcmp (name, "+") == 0) {
        *primfun = F_add_AxA;
    } else if (strcmp (name, "-") == 0) {
        *primfun = F_sub_AxA;
    } else if (strcmp (name, "*") == 0) {
        *primfun = F_mul_AxA;
    } else if (strcmp (name, "/") == 0) {
        *primfun = F_div_AxA;
    } else if (strcmp (name, "%") == 0) {
        *primfun = F_mod;
    } else if (strcmp (name, "==") == 0) {
        *primfun = F_eq;
    } else if (strcmp (name, "!=") == 0) {
        *primfun = F_neq;
    } else if (strcmp (name, ">") == 0) {
        *primfun = F_gt;
    } else if (strcmp (name, ">=") == 0) {
        *primfun = F_ge;
    } else if (strcmp (name, "<") == 0) {
        *primfun = F_lt;
    } else if (strcmp (name, "<=") == 0) {
        *primfun = F_le;
    } else if (strcmp (name, "!") == 0) {
        *primfun = F_not;
    } else if (strcmp (name, "&&") == 0) {
        *primfun = F_and;
    } else if (strcmp (name, "||") == 0) {
        *primfun = F_or;
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    node *HMap(node *arg_node, node *arg_info)
 *
 * description:
 *   this function is only needed for converting the "new prf notation",
 *   which is needed for the new type checker, into the one that is
 *   required by the old type checker.....8-((
 *   Once the new TC is as powerful as the old one is, this function
 *   (and its call in flatten) becomes obsolete 8-)).
 *
 ******************************************************************************/

node *
HMap (node *arg_node, node *arg_info)
{
    prf primfun;
    bool found;
    node *res, *exprs;

    DBUG_ENTER ("HMap");

    arg_node = TravSons (arg_node, arg_info);

    if (sbs == 1) {
        res = arg_node;
    } else {
        found = Name2Prf (AP_NAME (arg_node), &primfun);

        if (found) {
            exprs = AP_ARGS (arg_node);
            if ((primfun == F_sub_AxA) && (CountExprs (exprs) == 1)) {
                switch (NODE_TYPE (EXPRS_EXPR1 (exprs))) {
                case N_double:
                    res = MakeDouble (-DOUBLE_VAL (EXPRS_EXPR1 (exprs)));
                    arg_node = FreeNode (arg_node);
                    break;
                case N_float:
                    res = MakeFloat (-FLOAT_VAL (EXPRS_EXPR1 (exprs)));
                    arg_node = FreeNode (arg_node);
                    break;
                case N_num:
                    res = MakeNum (-NUM_VAL (EXPRS_EXPR1 (exprs)));
                    arg_node = FreeNode (arg_node);
                    break;
                default:
                    AP_ARGS (arg_node) = MakeExprs (MakeNum (0), exprs);
                    res = MakePrf (primfun, AP_ARGS (arg_node));
                    AP_ARGS (arg_node) = NULL;
                    arg_node = FreeNode (arg_node);
                }
            } else {
                res = MakePrf (primfun, AP_ARGS (arg_node));
                AP_ARGS (arg_node) = NULL;
                arg_node = FreeNode (arg_node);
            }
        } else {
            res = arg_node;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    node *HMNwithop(node *arg_node, node *arg_info)
 *
 * description:
 *   this function is only needed for converting the "new prf notation",
 *   which is needed for the new type checker, into the one that is
 *   required by the old type checker.....8-((
 *   Once the new TC is as powerful as the old one is, this function
 *   (and its call in flatten) becomes obsolete 8-)).
 *
 ******************************************************************************/

node *
HMNwithop (node *arg_node, node *arg_info)
{
    prf primfun;
    bool found;

    DBUG_ENTER ("HMNwithop");

    if ((sbs != 1) && (NWITHOP_TYPE (arg_node) == WO_foldfun)) {

        found = Name2Prf (NWITHOP_FUN (arg_node), &primfun);

        if (found) {
            NWITHOP_TYPE (arg_node) = WO_foldprf;
            NWITHOP_PRF (arg_node) = primfun;
        }
    }
    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
