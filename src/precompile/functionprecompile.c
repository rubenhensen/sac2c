/**
 *
 * $Id$
 *
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
 *
 ******************************************************************************/

#include "dbug.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "traverse.h"
#include "DupTree.h"
#include "types.h"
#include "globals.h"
#include "type_utils.h"
#include "internal_lib.h"
#include "namespaces.h"

/*
 * INFO structure
 */

struct INFO {
    node *fundef;
    argtab_t *argtab;
    node *preassigns;
    node *postassigns;
};

/*
 * INFO macros
 */

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_ARGTAB(n) ((n)->argtab)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_POSTASSIGNS(n) ((n)->postassigns)

/*
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ARGTAB (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_POSTASSIGNS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * helper functions
 */

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

static int
HighestLinksign (node *args)
{
    int res = -1;
    if (args != NULL) {
        res = MAX (ARG_LINKSIGN (args), HighestLinksign (ARG_NEXT (args)));
    }
    return (res);
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

/** <!-- ******************************************************************* -->
 *
 * @fn argtab_t *InsertIntoOut( argtab_t argtab, node *fundef, node *ret)
 *
 * @brief Inserts an out-argument into the argtab.
 *
 ******************************************************************************/

static argtab_t *
InsertIntoOut (argtab_t *argtab, node *fundef, node *ret)
{
    argtag_t argtag;
    int idx;
    int line;

    DBUG_ENTER ("InsertIntoOut");

    line = NODE_LINE (fundef);

    /*
     * Get the linksign value. This will just be the return's natural position
     * if the linksign pragma was not used.
     */
    idx = RET_LINKSIGN (ret);

    DBUG_PRINT ("FPC", ("out(%d)", idx));

    /* We do not pass descriptors for non-refcounted parameters. */
    if (!RET_ISREFCOUNTED (ret)) {
        argtag = ATG_out_nodesc;
    } else {
        argtag = ATG_out;

        /*
         * As this return value needs a descriptor, it cannot be returned using
         * the C return expression.
         */
        if (idx == 0) {
            CTIerrorLine (line, "Pragma 'linksign' or 'refcounting' illegal: "
                                "return value must not use a descriptor");
        }
    }

    /*
     * Check whether this return value is mapped to the C return value.
     */
    if (idx == 0) {
        node *retexprs;
        node *rets;

        RET_ISCRETURN (ret) = TRUE;

        /*
         * Look up the return N_exprs node corresponding to the current ret node
         * and save it as the C return expression for later use in compile.
         * This is only done if the function has a body and thus will be
         * compiled.
         */

        if (FUNDEF_BODY (fundef) != NULL) {
            DBUG_ASSERT ((FUNDEF_RETURN (fundef) != NULL), "FUNDEF_RETURN is missing "
                                                           "although return values do "
                                                           "exist and body"
                                                           " is non null.");

            DBUG_ASSERT ((NODE_TYPE (FUNDEF_RETURN (fundef)) == N_return),
                         "no N_return node found!");

            retexprs = RETURN_EXPRS (FUNDEF_RETURN (fundef));
            rets = FUNDEF_RETS (fundef);

            while ((rets != NULL) && (retexprs != NULL) && (rets != ret)) {
                rets = RET_NEXT (rets);
                retexprs = EXPRS_NEXT (retexprs);
            }

            DBUG_ASSERT (((retexprs != NULL) && (rets != NULL)),
                         "not enough return values found!");

            RETURN_CRET (FUNDEF_RETURN (fundef)) = retexprs;
        }
    }

    /* Check for illegal index values. */
    if ((idx < 0) || (idx >= argtab->size)) {
        CTIerrorLine (line,
                      "Pragma 'linksign' illegal: "
                      "entry contains illegal value %d",
                      idx);
        DBUG_RETURN (argtab);
    }

    /* Check whether this parameter was already given. */
    if (argtab->ptr_out[idx] != NULL) {
        CTIerrorLine (line,
                      "Pragma 'linksign' illegal: "
                      "out-parameter at position %d found twice",
                      idx);
        DBUG_RETURN (argtab);
    }

    /*
     * As the ptr_in chain will be filled while traversing the arg chain
     * there should be no values there at this stage. The handling of args
     * has to be performed after handling ret values to handle in/out
     * parameters correctly.
     */
    DBUG_ASSERT ((argtab->ptr_in[idx] == NULL), "argtab is inconsistent");

    /*
     * Check whether this entry already is in use and produce an error
     * message.
     */
    if (argtab->tag[idx] != ATG_notag) {
        CTIerrorLine (line, "Pragma 'linksign' illegal: return value found twice");
        DBUG_RETURN (argtab);
    }

    DBUG_ASSERT ((argtab->ptr_out[idx] == NULL), "argtab is inconsistent");

    argtab->ptr_out[idx] = ret;
    argtab->tag[idx] = argtag;

    DBUG_PRINT ("FPC", ("%s(): out-arg inserted at position %d with tag %s.",
                        FUNDEF_NAME (fundef), idx, global.argtag_string[argtag]));

    DBUG_RETURN (argtab);
}

/** <!-- ******************************************************************* -->
 *
 * @fn  argtab_t *InsertIntoIn( argtab_t *argtab, node *fundef, node *arg)
 *
 * @brief Inserts an in-argument into the argtab, merges with out-arguments
 * if the linksign pragma was applied.
 *
 ******************************************************************************/

static argtab_t *
InsertIntoIn (argtab_t *argtab, node *fundef, node *arg)
{
    argtag_t argtag;
    int line;
    int idx;

    DBUG_ENTER ("InsertIntoIn");

    line = NODE_LINE (fundef);

    /* Check if the type is not refcounted, so we don't pass the descriptor. */
    if (!ARG_ISREFCOUNTED (arg)) {

        /* Reference parameters are passed as inout, otherwise pass as in. */
        if (ARG_ISREFERENCE (arg)) {

            /* External boxed types do not need an additional pointer dereference.
             * The _bx suffix accomplishes this in the backend. */
            if (FUNDEF_ISEXTERN (fundef) && TUisBoxed (ARG_NTYPE (arg))) {
                argtag = ATG_inout_nodesc_bx;
            } else {
                argtag = ATG_inout_nodesc;
            }

        } else {
            argtag = ATG_in_nodesc;
        }
    } else {

        /* Same as above: Reference parameters are passed as inout. */
        if (ARG_ISREFERENCE (arg)) {
            argtag = ATG_inout;
        } else {
            argtag = ATG_in;
        }
    }

    /*
     * Get the linksign value. This will just be the parameter's natural position
     * if the linksign pragma was not used.
     */
    idx = ARG_LINKSIGN (arg);

    DBUG_PRINT ("FPC", ("in(%d): %s", idx, AVIS_NAME (ARG_AVIS (arg))));

    /* Check for illegal index values. */
    if ((idx < 0) || (idx >= argtab->size)) {
        CTIerrorLine (line,
                      "Pragma 'linksign' illegal: "
                      "entry contains illegal value %d",
                      idx);
        DBUG_RETURN (argtab);
    }

    /* Index 0 is reserved for return positions. */
    if (idx == 0) {
        CTIerrorLine (line, "Pragma 'linksign' illegal: "
                            "in-parameter cannot be used as return value");
        DBUG_RETURN (argtab);
    }

    /* Check whether this parameter was already given. */
    if (argtab->ptr_in[idx] != NULL) {
        CTIerrorLine (line,
                      "Pragma 'linksign' illegal: "
                      "in-parameter at position %d found twice",
                      idx);
        DBUG_RETURN (argtab);
    }

    if (argtab->ptr_out[idx] == NULL) {

        /*
         * There is no corresponding out parameter, so just insert the argument
         * into the argtab as an in parameter.
         */
        DBUG_ASSERT ((argtab->tag[idx] == ATG_notag), "argtab is inconsistent");

        argtab->ptr_in[idx] = arg;
        argtab->tag[idx] = argtag;

        DBUG_PRINT ("FPC",
                    ("%s(): in-arg %s at position %d with tag %s.", FUNDEF_NAME (fundef),
                     AVIS_NAME (ARG_AVIS (arg)), idx, global.argtag_string[argtag]));
    } else {
        /*
         * There is already an out parameter, so we are going to merge the in
         * parameter with the out parameter. This normally only happens when the
         * linksign pragma was used on an external function.
         *
         * This can only be done under two conditions: - Both parameters should be
         * of the same type.  - Both parameters must not have a descriptor, because
         * we do not reference count external data.
         *
         * NOTE: inout parameters are also used for SPMD functions. In that case
         * the last restriction does not apply.
         */

        /* Check that both parameters have no descriptors. */
        if (!(FUNDEF_ISSPMDFUN (fundef))
            && !(argtab->tag[idx] == ATG_out_nodesc && argtag == ATG_in_nodesc)) {
            CTIerrorLine (line, "Pragma 'linksign' illegal: "
                                "mappings allowed exclusively between parameters"
                                " without descriptor");
            DBUG_RETURN (argtab);
        }

        /* Check that both parameters have equal types. */
        if (!TYeqTypes (RET_TYPE (argtab->ptr_out[idx]), ARG_NTYPE (arg))) {
            CTIerrorLine (line, "Pragma 'linksign' illegal: "
                                "mappings allowed exclusively between parameters"
                                " with identical types");
        }

        /*
         * Merge the in and out parameters.
         *
         * SPMD functions get the inout tag. Other functions have no descriptors
         * and thus get the inout_nodesc_bx or inout_nodesc for boxed and unboxed
         * types, respectively.
         */
        if (FUNDEF_ISSPMDFUN (fundef)) {
            argtag = ATG_inout;
        } else {
            if (TUisBoxed (ARG_NTYPE (arg))) {
                argtag = ATG_inout_nodesc_bx;
            } else {
                argtag = ATG_inout_nodesc;
            }
        }

        argtab->ptr_in[idx] = arg;
        argtab->tag[idx] = argtag;

        DBUG_PRINT ("FPC", ("%s(): in-arg %s merged with out-arg %d with tag %s.",
                            FUNDEF_NAME (fundef), AVIS_NAME (ARG_AVIS (arg)), idx,
                            global.argtag_string[argtag]));
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
    DBUG_ENTER ("FPCfundef");

    DBUG_PRINT ("FPC", ("processing fundef %s:%s...", NSgetName (FUNDEF_NS (arg_node)),
                        FUNDEF_NAME (arg_node)));

    INFO_FUNDEF (arg_info) = arg_node;

    if (!FUNDEF_ISZOMBIE (arg_node)) {
        int argtabsize
          = TCcountRets (FUNDEF_RETS (arg_node)) + TCcountArgs (FUNDEF_ARGS (arg_node));

        argtabsize = MAX (argtabsize, HighestLinksign (FUNDEF_ARGS (arg_node)));
        INFO_ARGTAB (arg_info) = TBmakeArgtab (argtabsize + 1);

        if (FUNDEF_RETS (arg_node) != NULL) {
            FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
        }
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        CTIabortOnError ();

        /*
         * assign the argtab to the function
         */
        FUNDEF_ARGTAB (arg_node) = INFO_ARGTAB (arg_info);

        FUNDEF_ARGTAB (arg_node) = CompressArgtab (FUNDEF_ARGTAB (arg_node));

        /*
         * traverse next fundef
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
        /*
         * all FUNDEF_ARGTABs are build now -> traverse body
         */
        INFO_POSTASSIGNS (arg_info) = NULL;
        INFO_PREASSIGNS (arg_info) = NULL;
        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *FPCret( node *arg_node, info *arg_info)
 *
 * Description:
 *   Builds FUNDEF_ARGTAB for ret nodes
 *
 ******************************************************************************/

node *
FPCret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FPCret");

    INFO_ARGTAB (arg_info)
      = InsertIntoOut (INFO_ARGTAB (arg_info), INFO_FUNDEF (arg_info), arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *FPCarg( node *arg_node, info *arg_info)
 *
 * Description:
 *   Builds FUNDEF_ARGTAB for arg nodes
 *
 ******************************************************************************/

node *
FPCarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FPCret");

    INFO_ARGTAB (arg_info)
      = InsertIntoIn (INFO_ARGTAB (arg_info), INFO_FUNDEF (arg_info), arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *FPCassign( node *arg_node, info *arg_info)
 *
 * Description:
 *   Inserts the assignments found in INFO_PRE/POSTASSIGNS into the AST.
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

    if (INFO_POSTASSIGNS (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGNS (arg_info) = NULL;
    }
    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   int GetArgtabIndexOut( node *ret, argtab_t *argtab)
 *
 * Description:
 *
 *
 ******************************************************************************/

static int
GetArgtabIndexOut (node *ret, argtab_t *argtab)
{
    int idx;

    DBUG_ENTER ("GetArgtabIndexOut");

    idx = 0;

    while ((argtab->ptr_out[idx] != ret) && (idx < argtab->size)) {
        idx++;
    }

    DBUG_ASSERT ((argtab->ptr_out[idx] == ret), "no index for out-parameter found!");

    DBUG_RETURN (idx);
}

/******************************************************************************
 *
 * Function:
 *   int GetArgtabIndexIn( node *arg, argtab_t *argtab)
 *
 * Description:
 *
 *
 ******************************************************************************/

static int
GetArgtabIndexIn (node *arg, argtab_t *argtab)
{
    int idx;

    DBUG_ENTER ("GetArgtabIndexIn");

    idx = 0;
    while ((argtab->ptr_in[idx] != arg) && (idx < argtab->size)) {
        idx++;
    }

    DBUG_ASSERT ((argtab->ptr_in[idx] == arg), "no index for in-parameter found!");

    DBUG_RETURN (idx);
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
    DBUG_ENTER ("FPClet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
        /*
         * only function applications are of interest
         */

        node *fundef;
        node *ids;
        node *rets;
        node *exprs;
        node *args;
        argtab_t *ap_argtab;
        argtab_t *argtab;
        int idx = 0;
        int dots_offset = 0;

        fundef = AP_FUNDEF (LET_EXPR (arg_node));

        DBUG_ASSERT ((fundef != NULL), "AP_FUNDEF not found!");

        DBUG_PRINT ("FPC", ("Application of %s:%s().", NSgetName (FUNDEF_NS (fundef)),
                            FUNDEF_NAME (fundef)));

        ids = LET_IDS (arg_node);
        rets = FUNDEF_RETS (fundef);
        exprs = AP_ARGS (LET_EXPR (arg_node));
        args = FUNDEF_ARGS (fundef);

        ap_argtab = TBmakeArgtab (TCcountIds (ids) + TCcountExprs (exprs) + 1);

        argtab = FUNDEF_ARGTAB (fundef);

        DBUG_ASSERT ((argtab != NULL), "FUNDEF_ARGTAB not found!");

        dots_offset = 0;
        idx = ap_argtab->size; /* to avoid a CC warning */

        while (ids != NULL) {
            if (dots_offset == 0) {
                /*
                 * while handling true return values, get index
                 */
                idx = GetArgtabIndexOut (rets, argtab);
            }
            DBUG_ASSERT ((idx + dots_offset < ap_argtab->size), "illegal index");

            DBUG_ASSERT ((idx < argtab->size), "illegal index");

            ap_argtab->ptr_out[idx + dots_offset] = ids;
            if (dots_offset == 0) {
                ap_argtab->tag[idx] = argtab->tag[idx];
            } else {
                ap_argtab->tag[idx + dots_offset] = ATG_out_nodesc;
            }

            ids = IDS_NEXT (ids);

            if (rets != NULL) {
                rets = RET_NEXT (rets);

                /*
                 * if we have reached the last ret, all following return values are
                 * ... return values
                 */
                if (rets == NULL) {
                    idx = argtab->size - 1;
                    dots_offset = 1;
                }
            } else {
                dots_offset++;
            }
        }

        dots_offset = 0;
        idx = ap_argtab->size; /* to avoid a CC warning */

        while (exprs != NULL) {
            DBUG_ASSERT (((args != NULL) || (dots_offset != 0)),
                         "application is inconsistant");

            if (dots_offset == 0) {
                idx = GetArgtabIndexIn (args, argtab);
            }
            DBUG_ASSERT ((idx + dots_offset < ap_argtab->size), "illegal index");

            DBUG_ASSERT ((idx < argtab->size), "illegal index");

            ap_argtab->ptr_in[idx + dots_offset] = exprs;
            if (dots_offset == 0) {
                ap_argtab->tag[idx] = argtab->tag[idx];
            } else {
                ap_argtab->tag[idx + dots_offset] = ATG_in_nodesc;
            }

            exprs = EXPRS_NEXT (exprs);

            if (args != NULL) {
                args = ARG_NEXT (args);

                if (args == NULL) {
                    /*
                     * we have reached a ... argument
                     */
                    idx = argtab->size - 1;
                    dots_offset = 1;
                }
            } else {
                dots_offset++;
            }
        }

        CTIabortOnError ();

        AP_ARGTAB (LET_EXPR (arg_node)) = CompressArgtab (ap_argtab);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn FPCdoFunctionPrecompile
 *
 * @brief starts FPC traversal
 *****************************************************************************/
node *
FPCdoFunctionPrecompile (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("FPCdoFunctionPrecompile");

    info = MakeInfo ();

    TRAVpush (TR_fpc);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/* @} */
