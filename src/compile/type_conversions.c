/*
 *
 * $Log$
 * Revision 1.1  2004/11/26 17:53:24  sah
 * Initial revision
 *
 *
 *
 */

/**
 * @defgroup fpc Function Precompile
 * @ingroup prec Precompile
 *
 * Functions needed by function precompile phase
 *
 * @{
 */

/** <!-- ****************************************************************** -->
 *
 * @file functionprecompile.c
 *
 * Things done during this traversal:
 *   - Function signatures are transformed into the final form:
 *     At most a single return value, remapping because of a linksign pragma,
 *     parameter tags (in, out, inout, ...).
 *     The reorganized layout is stored in FUNDEF_ARGTAB and AP_ARGTAB
 *     respectively. Note, that the node information of the AST is left as is.
 *   - Arguments of function applications are lifted if formal and actual types
 *     differ:
 *       b = fun( a);   =>   _tmp_a = a; b = fun( _tmp_a);
 *     Return values of function applications are lifted if formal and actual
 *     types differ:
 *       b = fun( a);   =>   _tmp_b = fun( a); b = _tmp_b;
 *
 ******************************************************************************/

#include <string.h>

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "prf.h"
#include "free.h"
#include "DupTree.h"
#include "traverse.h"
#include "convert.h"
#include "DataFlowMask.h"
#include "adjust_ids.h"
#include "NameTuplesUtils.h"
#include "map_cwrapper.h"
#include "scheduling.h"
#include "compile.h"
#include "precompile.h"
#include "markmemvals.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *preassigns;
    node *postassigns;
};

/*
 * INFO macros
 */
#define INFO_FPC_FUNDEF(n) ((n)->fundef)
#define INFO_FPC_PREASSIGNS(n) ((n)->preassigns)
#define INFO_FPC_POSTASSIGNS(n) ((n)->postassigns)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FPC_FUNDEF (result) = NULL;
    INFO_FPC_PREASSIGNS (result) = NULL;
    INFO_FPC_POSTASSIGNS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

#define FUNDEF_NO_DESC(n, idx)                                                           \
    ((FUNDEF_STATUS (n) == ST_Cfun)                                                      \
     && ((FUNDEF_PRAGMA (n) == NULL) || (FUNDEF_REFCOUNTING (n) == NULL)                 \
         || (PRAGMA_NUMPARAMS (FUNDEF_PRAGMA (n)) <= (idx))                              \
         || (!(FUNDEF_REFCOUNTING (n)[idx]))))

#define FUNDEF_HAS_LINKSIGN(n, idx)                                                      \
    (((FUNDEF_STATUS (n) == ST_Cfun) && (FUNDEF_PRAGMA (n) != NULL)                      \
      && (FUNDEF_LINKSIGN (n) != NULL)                                                   \
      && (PRAGMA_NUMPARAMS (FUNDEF_PRAGMA (n)) > (idx)))                                 \
       ? TRUE                                                                            \
       : FALSE)

#define FUNDEF_GET_LINKSIGN(n, idx, dots)                                                \
    (((FUNDEF_STATUS (n) == ST_Cfun) && (FUNDEF_PRAGMA (n) != NULL)                      \
      && (FUNDEF_LINKSIGN (n) != NULL)                                                   \
      && (PRAGMA_NUMPARAMS (FUNDEF_PRAGMA (n)) > (idx)))                                 \
       ? (FUNDEF_LINKSIGN (fundef))[idx]                                                 \
       : ((dots) ? (idx) : ((idx) + 1)))

/*
 * helper functions
 */

/******************************************************************************
 *
 * Function:
 *   void LiftIds( ids *ids_arg, node *fundef, types *new_type,
 *                 node **new_assigns)
 *
 * Description:
 *   Lifts the given return value of a function application:
 *    - Generates a new and fresh varname.
 *    - Generates a new vardec and inserts it into the vardec chain of 'fundef'.
 *      If 'new_type' is not NULL, 'new_type' is used as VARDEC_TYPE instead
 *      of IDS_VARDEC(ids_arg).
 *    - Builds a new assignment and inserts it into the assignment chain
 *      'new_assigns'.
 *    - Adjusts the name and vardec of 'ids_arg'.
 *
 ******************************************************************************/

static void
LiftIds (node *ids_arg, node *fundef, types *new_type, node **new_assigns)
{
    char *new_name;
    node *new_vardec;
    node *new_id;

    DBUG_ENTER ("LiftIds");

    new_name = ILIBtmpVarName (IDS_NAME (ids_arg));
    /*
     * Insert vardec for new var
     */
    if (new_type == NULL) {
        new_type = IDS_TYPE (ids_arg);
    }
    new_vardec = TBmakeVardec (ILIBstringCopy (new_name), DupAllTypes (new_type), NULL);
    fundef = AddVardecs (fundef, new_vardec);

    /*
     * Abstract the found return value out:
     *   A:n = fun( ...);
     *   ... A:n ... A:1 ...    // n references of A
     * is transformed into
     *   __A:1 = fun( ...);
     *   A:n = __A:1;
     *   ... A:n ... A:1 ...    // n references of A
     */
    new_id = MakeId (new_name, NULL, ST_regular);
    ID_VARDEC (new_id) = new_vardec;
    (*new_assigns) = MakeAssign (MakeLet (new_id, DupOneIds (ids_arg)), (*new_assigns));

    IDS_NAME (ids_arg) = ILIBfree (IDS_NAME (ids_arg));
    IDS_NAME (ids_arg) = ILIBstringCopy (new_name);
    IDS_VARDEC (ids_arg) = new_vardec;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   argtab_t *CompressArgtab( argtab_t *argtab)
 *
 * Description:
 *   Empty entries in 'argtab' are moved to the end of the table.
 *
 ******************************************************************************/

static argtab_t *
CompressArgtab (argtab_t *argtab)
{
    int pos, idx;
    int old_size;

    DBUG_ENTER ("CompressArgtab");

    pos = idx = 1;
    while (pos < argtab->size) {
        if (argtab->tag[pos] != ATG_notag) {
            if (pos > idx) {
                argtab->tag[idx] = argtab->tag[pos];
                argtab->ptr_in[idx] = argtab->ptr_in[pos];
                argtab->ptr_out[idx] = argtab->ptr_out[pos];
            }
            idx++;
        } else {
            DBUG_ASSERT ((argtab->ptr_in[pos] == NULL), "argtab inconsistent");
            DBUG_ASSERT ((argtab->ptr_out[pos] == NULL), "argtab inconsistent");
        }
        pos++;
    }
    /*
     * adjust size
     */
    old_size = argtab->size;
    argtab->size = idx;

    /*
     * clear superfluous entries
     */
    for (; idx < old_size; idx++) {
        argtab->tag[idx] = ATG_notag;
        argtab->ptr_in[idx] = NULL;
        argtab->ptr_out[idx] = NULL;
    }

    DBUG_RETURN (argtab);
}

/*
 * traversal functions
 */

/******************************************************************************
 *
 * function:
 *   node *FPCmodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
FPCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FPCmodule");

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   argtab_t *InsertOut( argtab_t argtab, node *fundef,
 *                        int param_id, types *rettype,
 *                        bool *dots, node *ret)
 *
 * Description:
 *   Inserts an out-argument into the argtab.
 *
 ******************************************************************************/

static argtab_t *
InsertOut (argtab_t *argtab, node *fundef, int param_id, types *rettype, bool *dots,
           node *ret)
{
    argtag_t argtag;
    int idx;
    int line;

    DBUG_ENTER ("InsertOut");

    line = NODE_LINE (fundef);

    idx = FUNDEF_GET_LINKSIGN (fundef, param_id, *dots);
    if (TYPES_BASETYPE (rettype) != T_dots) {
        if (FUNDEF_NO_DESC (fundef, param_id)) {
            argtag = ATG_out_nodesc;
        } else {
            argtag = ATG_out;

            if (idx == 0) {
                ERROR (line, ("Pragma 'linksign' or 'refcounting' illegal"));
                CONT_ERROR (("Return value must not use a descriptor"));
            }
        }

        if ((argtab->ptr_out[0] == NULL) && (argtag == ATG_out_nodesc)
            && ((FUNDEF_PRAGMA (fundef) == NULL) || (FUNDEF_LINKSIGN (fundef) == NULL))) {
            node *ret_exprs;
            int i;

            /*
             * no linksign pragma given and no C return value found yet?
             *  -> use this out-param as C return value!
             */
            idx = 0;

            /*
             * set RETURN_CRET(ret)
             */
            if (ret != NULL) {
                DBUG_ASSERT ((NODE_TYPE (ret) == N_return), "no N_return node found!");

                ret_exprs = RETURN_EXPRS (ret);
                for (i = 0; i < param_id; i++) {
                    DBUG_ASSERT ((ret_exprs != NULL), "not enough return values found!");
                    ret_exprs = EXPRS_NEXT (ret_exprs);
                }
                DBUG_ASSERT ((ret_exprs != NULL), "not enough return values found!");

                RETURN_CRET (ret) = ret_exprs;
            }
        }
    } else {
        DBUG_ASSERT (((*dots) == FALSE), "more than one T_dots parameter found");

        if ((idx != argtab->size - 1) && FUNDEF_HAS_LINKSIGN (fundef, param_id)) {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("Parameter '...' must be mapped to the last position"));
        }
        (*dots) = TRUE;

        idx = argtab->size - 1;
        argtag = FUNDEF_NO_DESC (fundef, param_id) ? ATG_out_nodesc : ATG_out;
    }

    if (idx == 0) {
        /*
         * mark the C return value
         */
        TYPES_STATUS (rettype) = ST_crettype;
    }

    if ((idx >= 0) && (idx < argtab->size)) {
        DBUG_ASSERT ((argtab->ptr_in[idx] == NULL), "argtab is inconsistent");

        if (argtab->tag[idx] == ATG_notag) {
            DBUG_ASSERT ((argtab->ptr_out[idx] == NULL), "argtab is inconsistent");

            argtab->ptr_out[idx] = rettype;
            argtab->tag[idx] = argtag;

            DBUG_PRINT ("PREC", ("%s(): out-arg " F_PTR
                                 " (TYPE) inserted at position %d with tag %s.",
                                 FUNDEF_NAME (fundef), rettype, idx, ATG_string[argtag]));
        } else if (idx == 0) {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("Return value found twice"));
        } else {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("Out-parameter at position %d found twice", idx));
        }
    } else {
        ERROR (line, ("Pragma 'linksign' illegal"));
        CONT_ERROR (("Entry at position %d contains illegal value %d", param_id, idx));
    }

    DBUG_RETURN (argtab);
}

/******************************************************************************
 *
 * Function:
 *   argtab_t *InsertIn( argtab_t *argtab, node *fundef,
 *                       int param_id, node *arg,
 *                       bool *dots)
 *
 * Description:
 *   Inserts an in-argument into the argtab.
 *
 ******************************************************************************/

static argtab_t *
InsertIn (argtab_t *argtab, node *fundef, int param_id, node *arg, bool *dots)
{
    argtag_t argtag;
    int idx;
    int line;

    DBUG_ENTER ("InsertIn");

    line = NODE_LINE (fundef);

    idx = FUNDEF_GET_LINKSIGN (fundef, param_id, *dots);
    if (ARG_BASETYPE (arg) != T_dots) {
        if (FUNDEF_NO_DESC (fundef, param_id)) {
            if (ARG_ATTRIB (arg) == ST_reference) {
                if ((FUNDEF_STATUS (fundef) == ST_Cfun) && (IsBoxed (ARG_TYPE (arg)))) {
                    argtag = ATG_inout_nodesc_bx;
                } else {
                    argtag = ATG_inout_nodesc;
                }
            } else {
                argtag = ATG_in_nodesc;
            }
        } else {
            argtag = (ARG_ATTRIB (arg) == ST_reference) ? ATG_inout : ATG_in;
        }
    } else {
        DBUG_ASSERT (((*dots) == FALSE), "more than one T_dots parameter found");

        if ((idx != argtab->size - 2) && FUNDEF_HAS_LINKSIGN (fundef, param_id)) {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("Parameter '...' must be mapped to the last position"));
        }
        (*dots) = TRUE;

        idx = argtab->size - 1;
        argtag = FUNDEF_NO_DESC (fundef, param_id) ? ATG_in_nodesc : ATG_in;
    }

    if (idx == 0) {
        ERROR (line, ("Pragma 'linksign' illegal"));
        CONT_ERROR (
          ("In-parameter at position %d cannot be used as return value", param_id));
    } else if ((idx > 0) && (idx < argtab->size)) {
        if (argtab->ptr_in[idx] == NULL) {
            if (argtab->ptr_out[idx] == NULL) {
                DBUG_ASSERT ((argtab->tag[idx] == ATG_notag), "argtab is inconsistent");

                argtab->ptr_in[idx] = arg;
                argtab->tag[idx] = argtag;

                DBUG_PRINT ("PREC", ("%s(): in-arg " F_PTR "," F_PTR
                                     " (ARG,TYPE) inserted at position %d with tag %s.",
                                     FUNDEF_NAME (fundef), arg, ARG_TYPE (arg), idx,
                                     ATG_string[argtag]));
            } else if ((argtab->tag[idx] == ATG_out_nodesc)
                       && (argtag == ATG_in_nodesc)) {
                /*
                 * merge 'argtab->ptr_out[idx]' and 'arg'
                 */
                if (CmpTypes (argtab->ptr_out[idx], ARG_TYPE (arg)) == CMP_equal) {
                    argtag
                      = IsBoxed (ARG_TYPE (arg)) ? ATG_inout_nodesc_bx : ATG_inout_nodesc;
                    argtab->ptr_in[idx] = arg;
                    argtab->tag[idx] = argtag;

                    DBUG_PRINT ("PREC", ("%s(): in-arg " F_PTR "," F_PTR
                                         " (ARG,TYPE) merged with out-arg " F_PTR
                                         " (TYPE) at position %d with tag %s.",
                                         FUNDEF_NAME (fundef), arg, ARG_TYPE (arg),
                                         argtab->ptr_out[idx], idx, ATG_string[argtag]));
                } else {
                    ERROR (line, ("Pragma 'linksign' illegal"));
                    CONT_ERROR (("Mappings allowed exclusively between parameters"
                                 " with identical types"));
                }
            } else {
                ERROR (line, ("Pragma 'linksign' illegal"));
                CONT_ERROR (("Mappings allowed exclusively between parameters"
                             " without descriptor"));
            }
        } else {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("In-parameter at position %d found twice", idx));
        }
    } else {
        ERROR (line, ("Pragma 'linksign' illegal"));
        CONT_ERROR (("Entry at position %d contains illegal value %d", param_id, idx));
    }

    DBUG_RETURN (argtab);
}

/******************************************************************************
 *
 * Function:
 *   node *FPCfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Builds FUNDEF_ARGTAB.
 *
 ******************************************************************************/

node *
FPCfundef (node *arg_node, info *arg_info)
{
    types *rettypes;
    node *args;
    argtab_t *argtab;
    int param_id;
    bool dots;

    DBUG_ENTER ("FPCfundef");

    if (!FUNDEF_ISZOMBIE (arg_node)) {
        rettypes = FUNDEF_TYPES (arg_node);
        args = FUNDEF_ARGS (arg_node);
        param_id = 0;
        dots = FALSE;
        argtab = MakeArgtab (CountFunctionParams (arg_node) + 1);

        while (rettypes != NULL) {
            if (TYPES_BASETYPE (rettypes) != T_void) {
                argtab = InsertOut (argtab, arg_node, param_id, rettypes, &dots,
                                    FUNDEF_RETURN (arg_node));
                param_id++;
            }

            rettypes = TYPES_NEXT (rettypes);
        }

        while (args != NULL) {
            argtab = InsertIn (argtab, arg_node, param_id, args, &dots);
            param_id++;

            args = ARG_NEXT (args);
        }

        ABORT_ON_ERROR;
        FUNDEF_ARGTAB (arg_node) = CompressArgtab (argtab);

        /*
         * traverse next fundef
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }

        INFO_FPC_FUNDEF (arg_info) = arg_node;

        /*
         * all FUNDEF_ARGTABs are build now
         *  -> traverse body
         */
        INFO_FPC_POSTASSIGNS (arg_info) = NULL;
        INFO_FPC_PREASSIGNS (arg_info) = NULL;
        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        /*
         * FUNDEF_PRAGMA is still needed during the last traversal!!!
         *  -> it is removed later
         */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *FPCassign( node *arg_node, info *arg_info)
 *
 * Description:
 *   Inserts the assignments found in INFO_FPC_PRE/POSTASSIGNS into the AST.
 *
 ******************************************************************************/

node *
FPCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FPCassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_FPC_POSTASSIGNS (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_FPC_POSTASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
        INFO_FPC_POSTASSIGNS (arg_info) = NULL;
    }
    if (INFO_FPC_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_FPC_PREASSIGNS (arg_info), arg_node);
        INFO_FPC_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeMergeAssigns( argtab_t *argtab, info *arg_info)
 *
 * Description:
 *   Builds merging assignments for inout-arguments if needed and stores them
 *   in 'arg_info'.
 *
 ******************************************************************************/

static info *
MakeMergeAssigns (argtab_t *argtab, info *arg_info)
{
    node *expr;
    int i;
    node *pre_assigns = INFO_FPC_PREASSIGNS (arg_info);
    node *post_assigns = INFO_FPC_POSTASSIGNS (arg_info);

    DBUG_ENTER ("MakeMergeAssigns");

    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    for (i = argtab->size - 1; i >= 1; i--) {
        if ((argtab->ptr_out[i] != NULL) && (argtab->ptr_in[i] != NULL)) {
            DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_exprs),
                         "no N_exprs node found in argtab!");
            expr = EXPRS_EXPR (argtab->ptr_in[i]);

            if ((NODE_TYPE (expr) != N_id)
                || (!ILIBstringCompare (IDS_NAME (((node *)argtab->ptr_out[i])),
                                        ID_NAME (expr)))) {
                node *out_ids;

                DBUG_ASSERT (NODE_TYPE (expr) == N_id,
                             "Non N_id function argument found!!!");
                /*
                 * argument is a refcounted ID node
                 *  -> must be converted into a unique object
                 *
                 ******
                 *
                 *  a = fun( b);   ->   a' = to_unq( b);
                 *                           fun( a');
                 *                      a  = from_unq( a');
                 */

                /*
                 * a'
                 */
                out_ids = DUPdoDupNode (argtab->ptr_out[i]);

                /*
                 * to_unq( b)
                 */
                expr = TBmakePrf (F_to_unq, TBmakeExprs (DUPdoDupTree (expr), NULL));

                /*
                 * a = to_unq( b);
                 */
                /*
                 * append at tail of 'pre_assigns'!!!
                 */
                pre_assigns
                  = TCappendAssign (pre_assigns,
                                    TBmakeAssign (TBmakeLet (out_ids, expr), NULL)),
                  /*
                   * a
                   */
                  out_ids = DUPdoDupNode (argtab->ptr_out[i]);

                /*
                 * from_unq( a)
                 */
                expr = DUPdupIdsId (argtab->ptr_out[i]);
                expr = TBmakePrf (F_from_unq, TBmakeExprs (expr, NULL));

                /*
                 * append at head of 'post_assigns'!!!
                 */
                post_assigns = TBmakeAssign (TBmakeLet (out_ids, expr), post_assigns);

                DBUG_PRINT ("PREC", ("Assignments %s = to_unq(...) added",
                                     IDS_NAME (((node *)argtab->ptr_out[i]))));
            }
        }
    }

    INFO_FPC_PREASSIGNS (arg_info) = pre_assigns;
    INFO_FPC_POSTASSIGNS (arg_info) = post_assigns;

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   node *FPClet( node *arg_node, info *arg_info)
 *
 * Description:
 *   Builds AP_ARGTAB and generates merging assignments for inout-arguments
 *   if needed.
 *
 ******************************************************************************/

node *
FPClet (node *arg_node, info *arg_info)
{
    node *ap, *fundef;
    argtab_t *ap_argtab, *argtab;
    node *ap_ids;
    types *rettypes;
    node *ap_exprs;
    node *args;
    int idx, dots_off;
    node *ap_id;
    shape_class_t actual_cls, formal_cls;

    DBUG_ENTER ("FPClet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    ap = LET_EXPR (arg_node);
    if (NODE_TYPE (ap) == N_ap) {
        fundef = AP_FUNDEF (ap);
        DBUG_ASSERT ((fundef != NULL), "AP_FUNDEF not found!");

        DBUG_PRINT ("PREC", ("Application of %s().", FUNDEF_NAME (fundef)));

        ap_ids = LET_IDS (arg_node);
        rettypes = FUNDEF_TYPES (fundef);
        ap_exprs = AP_ARGS (ap);
        args = FUNDEF_ARGS (fundef);

        ap_argtab = MakeArgtab (TCcountIds (ap_ids) + TCcountExprs (ap_exprs) + 1);
        argtab = FUNDEF_ARGTAB (fundef);
        DBUG_ASSERT ((argtab != NULL), "FUNDEF_ARGTAB not found!");

        dots_off = 0;
        idx = ap_argtab->size; /* to avoid a CC warning */
        while (ap_ids != NULL) {
            DBUG_ASSERT ((rettypes != NULL), "application is inconsistent");

            if (dots_off == 0) {
                idx = GetArgtabIndexOut (rettypes, argtab);
            }
            DBUG_ASSERT ((idx + dots_off < ap_argtab->size), "illegal index");
            DBUG_ASSERT ((idx < argtab->size), "illegal index");
            ap_argtab->ptr_out[idx + dots_off] = ap_ids;
            ap_argtab->tag[idx + dots_off] = argtab->tag[idx];

            if (TYPES_BASETYPE (rettypes) != T_dots) {
                actual_cls = GetShapeClassFromTypes (IDS_TYPE (ap_ids));
                formal_cls = GetShapeClassFromTypes (rettypes);
                if ((actual_cls != formal_cls)
                    && (ATG_has_shp[argtab->tag[idx]] || (actual_cls == C_scl)
                        || (formal_cls == C_scl))) {
                    DBUG_PRINT ("PREC",
                                ("Return value with inappropriate shape class found:"));
                    DBUG_PRINT ("PREC", ("   ... %s ... = %s( ... ), %s instead of %s",
                                         FUNDEF_NAME (fundef), IDS_NAME (ap_ids),
                                         nt_shape_string[actual_cls],
                                         nt_shape_string[formal_cls]));
                    LiftIds (ap_ids, INFO_FPC_FUNDEF (arg_info), rettypes,
                             &(INFO_FPC_POSTASSIGNS (arg_info)));
                }
            }

            ap_ids = IDS_NEXT (ap_ids);
            if (TYPES_BASETYPE (rettypes) != T_dots) {
                rettypes = TYPES_NEXT (rettypes);
            } else {
                dots_off++;
            }
        }

        dots_off = 0;
        idx = ap_argtab->size; /* to avoid a CC warning */
        while (ap_exprs != NULL) {
            DBUG_ASSERT ((args != NULL), "application is inconsistant");

            if (dots_off == 0) {
                idx = GetArgtabIndexIn (ARG_TYPE (args), argtab);
            }
            DBUG_ASSERT ((idx + dots_off < ap_argtab->size), "illegal index");
            DBUG_ASSERT ((idx < argtab->size), "illegal index");
            ap_argtab->ptr_in[idx + dots_off] = ap_exprs;
            ap_argtab->tag[idx + dots_off] = argtab->tag[idx];

            ap_id = EXPRS_EXPR (ap_exprs);

            DBUG_ASSERT ((NODE_TYPE (ap_id) == N_id), "no N_id node found!");
            if (ARG_BASETYPE (args) != T_dots) {
                formal_cls = GetShapeClassFromTypes (ARG_TYPE (args));
                actual_cls = GetShapeClassFromTypes (ID_TYPE (ap_id));
                if ((actual_cls != formal_cls)
                    && (ATG_has_shp[argtab->tag[idx]] || (actual_cls == C_scl)
                        || (formal_cls == C_scl))) {
                    DBUG_PRINT ("PREC",
                                ("Argument with inappropriate shape class found:"));
                    DBUG_PRINT ("PREC", ("   ... = %s( ... %s ...), %s instead of %s",
                                         FUNDEF_NAME (fundef), ID_NAME (ap_id),
                                         nt_shape_string[actual_cls],
                                         nt_shape_string[formal_cls]));
                    EXPRS_EXPR (ap_exprs)
                      = LiftArg (ap_id, INFO_FPC_FUNDEF (arg_info), ARG_TYPE (args), TRUE,
                                 &(INFO_FPC_PREASSIGNS (arg_info)));
                }
            }

            ap_exprs = EXPRS_NEXT (ap_exprs);
            if (ARG_BASETYPE (args) != T_dots) {
                args = ARG_NEXT (args);
            } else {
                dots_off++;
            }
        }

        ABORT_ON_ERROR;
        AP_ARGTAB (ap) = CompressArgtab (ap_argtab);

        /*
         * builds merging assignments if needed and stores them in 'arg_info'
         */
        arg_info = MakeMergeAssigns (ap_argtab, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *FPCap( node *arg_node, info *arg_info)
 *
 * Description:
 *   Lifts scalars from all N_ap and some N_prf applications.
 *
 ******************************************************************************/

node *
FPCap (node *arg_node, info *arg_info)
{
    prf *prf;
    node *args, *arg;
    int arg_cnt;
    types *type;

    DBUG_ENTER ("FPCap");

    args = AP_ (arg_node);
    arg_cnt = 0;
    while (args != NULL) {
        arg = EXPRS_EXPR (args);
        if ((NODE_TYPE (arg) != N_id) && (NODE_TYPE (arg) != N_array)) {
            switch (NODE_TYPE (arg)) {
            case N_num:
                type = MakeTypes1 (T_int);
                break;
            case N_float:
                type = MakeTypes1 (T_float);
                break;
            case N_double:
                type = MakeTypes1 (T_double);
                break;
            case N_bool:
                type = MakeTypes1 (T_bool);
                break;
            case N_char:
                type = MakeTypes1 (T_char);
                break;
            default:
                DBUG_ASSERT ((0), "illegal node type found!");
                type = NULL;
                break;
            }

            EXPRS_EXPR (args) = LiftArg (arg, INFO_FPC_FUNDEF (arg_info), type, TRUE,
                                         &(INFO_FPC_PREASSIGNS (arg_info)));
        }
        args = EXPRS_NEXT (args);
        arg_cnt++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *FPCprf( node *arg_node, info *arg_info)
 *
 * Description:
 *   Lifts scalars from all N_ap and some N_prf applications.
 *
 ******************************************************************************/

node *
FPCprf (node *arg_node, info *arg_info)
{
    prf *prf;
    node *args, *arg;
    int arg_cnt;
    types *type;

    DBUG_ENTER ("FPCprf");

    /*
     * prf F_alloc is only pseudo syntax which will be eliminated in compile.c.
     * It's args must not be traversed.
     */
    if ((PRF_PRF (arg_node) != F_alloc) && (PRF_PRF (arg_node) != F_alloc_or_reuse)
        && (PRF_ARGS (arg_node) != NULL)) {
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    }

    prf = &(PRF_PRF (arg_node));

    args = PRF_ARGS (arg_node);
    arg_cnt = 0;
    while (args != NULL) {
        arg = EXPRS_EXPR (args);
        if ((NODE_TYPE (arg) != N_id) && (NODE_TYPE (arg) != N_array)) {
            if ((*prf == F_dim) ||                          /* F_dim arg */
                (*prf == F_shape) ||                        /* F_shape arg */
                ((*prf == F_modarray) && (arg_cnt == 0))) { /* 1st F_modarray arg */
                switch (NODE_TYPE (arg)) {
                case N_num:
                    type = MakeTypes1 (T_int);
                    break;
                case N_float:
                    type = MakeTypes1 (T_float);
                    break;
                case N_double:
                    type = MakeTypes1 (T_double);
                    break;
                case N_bool:
                    type = MakeTypes1 (T_bool);
                    break;
                case N_char:
                    type = MakeTypes1 (T_char);
                    break;
                default:
                    DBUG_ASSERT ((0), "illegal node type found!");
                    type = NULL;
                    break;
                }

                EXPRS_EXPR (args) = LiftArg (arg, INFO_FPC_FUNDEF (arg_info), type, TRUE,
                                             &(INFO_FPC_PREASSIGNS (arg_info)));
            }
        }
        args = EXPRS_NEXT (args);
        arg_cnt++;
    }

    DBUG_RETURN (arg_node);
}

/* @} */
