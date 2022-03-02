/**
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

#define DBUG_PREFIX "FPC"
#include "debug.h"

#include "ctinfo.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "traverse.h"
#include "DupTree.h"
#include "types.h"
#include "globals.h"
#include "type_utils.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"
#include "math_utils.h"

/*
 * enum for traversal modes:
 *
 * FPC_fundef: annotate argtabs at fundef
 * FPC_ap:     annotate argtabs at ap node
 */
typedef enum { FPC_fundef, FPC_ap } fpc_travmode;

/*
 * INFO structure
 */

struct INFO {
    node *fundef;
    argtab_t *argtab;
    node *lhs;
    fpc_travmode travmode;
};

/*
 * INFO macros
 */

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_ARGTAB(n) ((n)->argtab)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_TRAVMODE(n) ((n)->travmode)

/*
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ARGTAB (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_TRAVMODE (result) = FPC_fundef;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

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
    size_t pos, idx;
    size_t old_size;

    DBUG_ENTER ();

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
            DBUG_ASSERT (argtab->ptr_in[pos] == NULL, "argtab inconsistent");
            DBUG_ASSERT (argtab->ptr_out[pos] == NULL, "argtab inconsistent");
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

/** <!-- ****************************************************************** -->
 * @fn int HighestLinksign(node * args)
 *
 * @brief Returns the maximum linksign of the given arguments
 *
 * @param args N_arg chain
 *
 * @return maximum linksign found of -1 if args == NULL
 ******************************************************************************/
static int
HighestLinksign (node *args)
{
    int res = -1;
    if (args != NULL) {
        res = MATHmax (ARG_LINKSIGN (args), HighestLinksign (ARG_NEXT (args)));
    }
    return (res);
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
    struct location loc = NODE_LOCATION (fundef);
    argtag_t argtag;
    size_t idx;

    DBUG_ENTER ();

    /*
     * Get the linksign value. This will just be the return's natural position
     * if the linksign pragma was not used.
     */
    idx = RET_LINKSIGN (ret);

    DBUG_PRINT ("out(%zu)", idx);

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
            CTIerror (loc, "Pragma 'linksign' or 'refcounting' illegal: "
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
            DBUG_ASSERT (FUNDEF_RETURN (fundef) != NULL, "FUNDEF_RETURN is missing "
                                                         "although return values do "
                                                         "exist and body"
                                                         " is non null.");

            DBUG_ASSERT (NODE_TYPE (FUNDEF_RETURN (fundef)) == N_return,
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
    if (idx >= argtab->size) {
        CTIerror (loc,
                  "Pragma 'linksign' illegal: "
                  "entry contains illegal value %zu",
                  idx);
        DBUG_RETURN (argtab);
    }

    /* Check whether this parameter was already given. */
    if (argtab->ptr_out[idx] != NULL) {
        CTIerror (loc,
                  "Pragma 'linksign' illegal: "
                  "out-parameter at position %zu found twice in function %s",
                  idx, FUNDEF_NAME (fundef));
        DBUG_RETURN (argtab);
    }

    /*
     * As the ptr_in chain will be filled while traversing the arg chain
     * there should be no values there at this stage. The handling of args
     * has to be performed after handling ret values to handle in/out
     * parameters correctly.
     */
    DBUG_ASSERT (argtab->ptr_in[idx] == NULL, "argtab is inconsistent");

    /*
     * Check whether this entry already is in use and produce an error
     * message.
     */
    if (argtab->tag[idx] != ATG_notag) {
        CTIerror (loc, "Pragma 'linksign' illegal: return value found twice");
        DBUG_RETURN (argtab);
    }

    DBUG_ASSERT (argtab->ptr_out[idx] == NULL, "argtab is inconsistent");

    argtab->ptr_out[idx] = ret;
    argtab->tag[idx] = argtag;

    DBUG_PRINT ("%s(): out-arg inserted at position %zu with tag %s.",
                FUNDEF_NAME (fundef), idx, global.argtag_string[argtag]);

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
    struct location loc = NODE_LOCATION (fundef);
    argtag_t argtag;
    size_t idx;

    DBUG_ENTER ();

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

    DBUG_PRINT ("in(%zu): %s", idx, AVIS_NAME (ARG_AVIS (arg)));

    /* Check for illegal index values. */
    if (idx >= argtab->size) {
        CTIerror (loc,
                  "Pragma 'linksign' illegal: entry contains illegal value %zu",
                  idx);
        DBUG_RETURN (argtab);
    }

    /* Index 0 is reserved for return positions. */
    if (idx == 0) {
        CTIerror (loc, "Pragma 'linksign' illegal: "
                       "in-parameter cannot be used as return value");
        DBUG_RETURN (argtab);
    }

    /* Check whether this parameter was already given. */
    if (argtab->ptr_in[idx] != NULL) {
        CTIerror (loc,
                  "Pragma 'linksign' illegal: "
                  "in-parameter at position %zu found twice in function %s",
                  idx, FUNDEF_NAME (fundef));
        DBUG_RETURN (argtab);
    }

    if (argtab->ptr_out[idx] == NULL) {

        /*
         * There is no corresponding out parameter, so just insert the argument
         * into the argtab as an in parameter.
         */
        DBUG_ASSERT (argtab->tag[idx] == ATG_notag, "argtab is inconsistent");

        argtab->ptr_in[idx] = arg;
        argtab->tag[idx] = argtag;

        DBUG_PRINT ("%s(): in-arg %s at position %zu with tag %s.", FUNDEF_NAME (fundef),
                    AVIS_NAME (ARG_AVIS (arg)), idx, global.argtag_string[argtag]);
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
        if (!(FUNDEF_ISSPMDFUN (fundef)) && !(FUNDEF_ISCUDAGLOBALFUN (fundef))
            && !(FUNDEF_ISCUDASTGLOBALFUN (fundef))
            && !(argtab->tag[idx] == ATG_out_nodesc && argtag == ATG_in_nodesc)) {
            CTIerror (loc, "Pragma 'linksign' illegal: "
                           "mappings allowed exclusively between parameters "
                           "without descriptor");
            DBUG_RETURN (argtab);
        }

        /* Check that both parameters have equal types. */
        if (!TYeqTypes (RET_TYPE (argtab->ptr_out[idx]), ARG_NTYPE (arg))) {
            CTIerror (loc, "Pragma 'linksign' illegal: "
                           "mappings allowed exclusively between parameters "
                           "with identical types");
        }

        /*
         * Merge the in and out parameters.
         *
         * SPMD functions get the inout tag. Other functions have no descriptors
         * and thus get the inout_nodesc_bx or inout_nodesc for boxed and unboxed
         * types, respectively.
         */
        if (FUNDEF_ISSPMDFUN (fundef) || FUNDEF_ISCUDAGLOBALFUN (fundef)
            || FUNDEF_ISCUDASTGLOBALFUN (fundef)) {
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

        DBUG_PRINT ("%s(): in-arg %s merged with out-arg %zu with tag %s.",
                    FUNDEF_NAME (fundef), AVIS_NAME (ARG_AVIS (arg)), idx,
                    global.argtag_string[argtag]);
    }

    DBUG_RETURN (argtab);
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

static size_t
GetArgtabIndexOut (node *ret, argtab_t *argtab)
{
    size_t idx;

    DBUG_ENTER ();

    idx = 0;

    while ((argtab->ptr_out[idx] != ret) && (idx < argtab->size)) {
        idx++;
    }

    DBUG_ASSERT (argtab->ptr_out[idx] == ret, "no index for out-parameter found!");

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

static size_t
GetArgtabIndexIn (node *arg, argtab_t *argtab)
{
    size_t idx;

    DBUG_ENTER ();

    idx = 0;
    while ((argtab->ptr_in[idx] != arg) && (idx < argtab->size)) {
        idx++;
    }

    DBUG_ASSERT (argtab->ptr_in[idx] == arg, "no index for in-parameter found!");

    DBUG_RETURN (idx);
}

/** <!-- ****************************************************************** -->
 * @fn argtab_t *BuildApArgtab( node *ap, node *lhs)
 *
 * @brief Computes the argtab for an N_ap node
 *
 * @param ap  N_ap node to compute argsign for
 * @param lhs corresponding lhs N_ids chain
 *
 * @return the constructed argtab
 ******************************************************************************/
static argtab_t *
BuildApArgtab (node *ap, node *lhs)
{
    node *fundef;
    node *ids;
    node *rets;
    node *exprs;
    node *args;
    argtab_t *ap_argtab;
    argtab_t *argtab;
    size_t idx = 0;
    size_t dots_offset = 0;

    DBUG_ENTER ();

    fundef = AP_FUNDEF (ap);

    DBUG_ASSERT (fundef != NULL, "AP_FUNDEF not found!");

    DBUG_PRINT ("Application of %s:%s().", NSgetName (FUNDEF_NS (fundef)),
                FUNDEF_NAME (fundef));

    ids = lhs;
    rets = FUNDEF_RETS (fundef);
    exprs = AP_ARGS (ap);
    args = FUNDEF_ARGS (fundef);

    ap_argtab = TBmakeArgtab (TCcountIds (ids) + TCcountExprs (exprs) + 1);

    argtab = FUNDEF_ARGTAB (fundef);

    if (argtab == NULL) {
        DBUG_ASSERT (argtab != NULL, "FUNDEF_ARGTAB not found!");
    }

    dots_offset = 0;
    idx = ap_argtab->size; /* to avoid a CC warning */

    while (ids != NULL) {
        if (dots_offset == 0) {
            /*
             * while handling true return values, get index
             */
            idx = GetArgtabIndexOut (rets, argtab);
        }
        DBUG_ASSERT (idx + dots_offset < ap_argtab->size, "illegal index");

        DBUG_ASSERT (idx < argtab->size, "illegal index");

        ap_argtab->ptr_out[idx + dots_offset] = ids;
        if (dots_offset == 0) {
            ap_argtab->tag[idx] = argtab->tag[idx];
        } else {
            /*
             * for ... results we only declare a descripto (ATG_out)
             * if the function has the refcountdots pragma set
             */
            if (FUNDEF_REFCOUNTDOTS (fundef)) {
                ap_argtab->tag[idx + dots_offset] = ATG_out;
            } else {
                ap_argtab->tag[idx + dots_offset] = ATG_out_nodesc;
            }
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
        DBUG_ASSERT (idx + dots_offset < ap_argtab->size, "illegal index");

        DBUG_ASSERT (idx < argtab->size, "illegal index");

        ap_argtab->ptr_in[idx + dots_offset] = exprs;
        if (dots_offset == 0) {
            ap_argtab->tag[idx] = argtab->tag[idx];
        } else {
            /*
             * for ... arguments we have to add descriptors (ATG_in) only
             * if this function has the refcountdots pragma
             */
            if (FUNDEF_REFCOUNTDOTS (fundef)) {
                ap_argtab->tag[idx + dots_offset] = ATG_in;
            } else {
                ap_argtab->tag[idx + dots_offset] = ATG_in_nodesc;
            }
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

    DBUG_RETURN (CompressArgtab (ap_argtab));
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
    DBUG_ENTER ();

    /*
     * 1) annotate linksigns at fundefs
     */
    INFO_TRAVMODE (arg_info) = FPC_fundef;

    MODULE_THREADFUNS (arg_node) = TRAVopt (MODULE_THREADFUNS (arg_node), arg_info);
    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    /*
     * 2) now that all fundefs have linksigns, annotate the N_ap nodes
     */
    INFO_TRAVMODE (arg_info) = FPC_ap;
    MODULE_THREADFUNS (arg_node) = TRAVopt (MODULE_THREADFUNS (arg_node), arg_info);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
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
    DBUG_ENTER ();

    DBUG_PRINT ("processing fundef %s...", CTIitemName (arg_node));

    INFO_FUNDEF (arg_info) = arg_node;

    if ((INFO_TRAVMODE (arg_info) == FPC_fundef) && !FUNDEF_ISZOMBIE (arg_node)) {
        size_t argtabsize
          = TCcountRets (FUNDEF_RETS (arg_node)) + TCcountArgs (FUNDEF_ARGS (arg_node));

        /* FIXME This causes a warning of conversion between size_t and int so cast for the meantime to ensure safety
           Linksign is int and this fun may ret -1 if arg_node = NULL. 
           Not changing Linksign to size_t for the time being incase it may ever be negative.
        */
        argtabsize = MATHmax ((int)argtabsize, HighestLinksign (FUNDEF_ARGS (arg_node)));
        INFO_ARGTAB (arg_info) = TBmakeArgtab (argtabsize + 1);

        FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

        CTIabortOnError ();

        /*
         * assign the argtab to the function
         */
        FUNDEF_ARGTAB (arg_node) = INFO_ARGTAB (arg_info);

        FUNDEF_ARGTAB (arg_node) = CompressArgtab (FUNDEF_ARGTAB (arg_node));
    } else if (INFO_TRAVMODE (arg_info) == FPC_ap) {
        /*
         * all FUNDEF_ARGTABs are build now -> traverse body
         */
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * traverse next fundef
     */
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    INFO_ARGTAB (arg_info)
      = InsertIntoOut (INFO_ARGTAB (arg_info), INFO_FUNDEF (arg_info), arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

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
FPCarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_ARGTAB (arg_info)
      = InsertIntoIn (INFO_ARGTAB (arg_info), INFO_FUNDEF (arg_info), arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *FPClet( node *arg_node, info *arg_info)
 *
 * @brief Memoizes the lhs in the info structure.
 ******************************************************************************/
node *
FPClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap && AP_ISSPAWNED (LET_EXPR (arg_node))) {
        INFO_LHS (arg_info) = LET_SYNC_IDS (arg_node);
    } else {
        INFO_LHS (arg_info) = LET_IDS (arg_node);
    }

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *FPCrange( node *arg_node, info *arg_info)
 *
 * @brief Sets the stage for the traversal of the contained N_ap node.
 ******************************************************************************/
node *
FPCrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RANGE_BODY (arg_node) = TRAVdo (RANGE_BODY (arg_node), arg_info);

    INFO_LHS (arg_info) = NULL;
    RANGE_RESULTS (arg_node) = TRAVopt (RANGE_RESULTS (arg_node), arg_info);

    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *FPCap( node *arg_node, info *arg_info)
 *
 * @brief Attaches an argtab to the given N_arg node.
 ******************************************************************************/
node *
FPCap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AP_ARGTAB (arg_node) = BuildApArgtab (arg_node, INFO_LHS (arg_info));

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

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_fpc);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/* @} */

#undef DBUG_PREFIX
