/*
 * $Log$
 * Revision 1.6  2004/12/07 18:01:15  sah
 * fixed stupid bug
 *
 * Revision 1.5  2004/11/29 15:04:51  sah
 * fixed-a-bug(tm)
 *
 * Revision 1.4  2004/11/29 14:43:20  sah
 * bugfixes
 *
 * Revision 1.3  2004/11/27 00:16:00  ktr
 * New barebones precompile.
 *
 * Revision 1.2  2004/11/26 23:13:36  sah
 * *** empty log message ***
 * Revision 1.1  2004/11/26 22:10:23  sah Initial
 * revision
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
 *
 ******************************************************************************/

#include "precompile.h"
#include "dbug.h"
#include "Error.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "traverse.h"
#include "DupTree.h"
#include "types.h"
#include "globals.h"
#include "type_utils.h"
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
#define INFO_FPC_FUNDEF(n) ((n)->fundef)
#define INFO_FPC_ARGTAB(n) ((n)->argtab)
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
    INFO_FPC_ARGTAB (result) = NULL;
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

    idx = RET_LINKSIGN (ret);

    if (!RET_ISREFCOUNTED (ret)) {
        argtag = ATG_out_nodesc;
    } else {
        argtag = ATG_out;

        /*
         * as this return value needs a descriptor, it cannot be returned using
         * the c return expression
         */
        if (idx == 0) {
            ERROR (line, ("Pragma 'linksign' or 'refcounting' illegal"));
            CONT_ERROR (("Return value must not use a descriptor"));
        }
    }

    /*
     * check whether this return value is mapped to the c return value
     */
    if (idx == 0) {
        node *retexprs;
        node *rets;

        /*
         * mark the C return value
         */
        RET_ISCRETURN (ret) = TRUE;

        /*
         * lookup the return N_exprs node corresponding to the current ret node
         * and save it as the c return expression for later use in compile
         */

        DBUG_ASSERT ((FUNDEF_RETURN (fundef) != NULL),
                     "FUNDEF_RETURN is missing although return values do exist.");

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

        RETURN_CRET (ret) = retexprs;
    }
    /*
     * update the argtab
     */
    if ((idx >= 0) && (idx < argtab->size)) {
        /*
         * as the ptr_in chain will be filled while traversing the arg chain
         * there should be no values there at this stage. The handling of args
         * has to be performed after handling ret values to handle in/out
         * parameters correctly.
         */
        DBUG_ASSERT ((argtab->ptr_in[idx] == NULL), "argtab is inconsistent");

        /*
         * check whether this entry already is in use and produce an error
         * message
         */
        if (argtab->tag[idx] == ATG_notag) {
            DBUG_ASSERT ((argtab->ptr_out[idx] == NULL), "argtab is inconsistent");

            argtab->ptr_out[idx] = ret;
            argtab->tag[idx] = argtag;

            DBUG_PRINT ("PREC",
                        ("%s(): out-arg " F_PTR
                         " (RET) inserted at position %d with tag %s.",
                         FUNDEF_NAME (fundef), ret, idx, global.argtag_string[argtag]));
        } else if (idx == 0) {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("Return value found twice"));
        } else {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("Out-parameter at position %d found twice", idx));
        }
    } else {
        ERROR (line, ("Pragma 'linksign' illegal"));
        CONT_ERROR (("Entry contains illegal value %d", idx));
    }

    DBUG_RETURN (argtab);
}

/** <!-- ******************************************************************* -->
 *
 * @fn  argtab_t *InsertIntoIn( argtab_t *argtab, node *fundef, node *arg)
 *
 * @brief Inserts an in-argument into the argtab.
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

    idx = ARG_LINKSIGN (arg);

    if (!ARG_ISREFCOUNTED (arg)) {
        if (ARG_ISREFERENCE (arg)) {
            if ((FUNDEF_ISEXTERN (fundef)) && (TUisBoxed (ARG_NTYPE (arg)))) {
                argtag = ATG_inout_nodesc_bx;
            } else {
                argtag = ATG_inout_nodesc;
            }
        } else {
            argtag = ATG_in_nodesc;
        }
    } else {
        if (ARG_ISREFERENCE (arg)) {
            argtag = ATG_inout;
        } else {
            argtag = ATG_in;
        }
    }

    if (idx == 0) {
        ERROR (line, ("Pragma 'linksign' illegal"));
        CONT_ERROR (("In-parameter cannot be used as return value"));
    } else {
        if ((idx > 0) && (idx < argtab->size)) {
            /*
             * this is a in only argument, as no outptr is set
             */
            if (argtab->ptr_in[idx] == NULL) {
                if (argtab->ptr_out[idx] == NULL) {
                    DBUG_ASSERT ((argtab->tag[idx] == ATG_notag),
                                 "argtab is inconsistent");

                    argtab->ptr_in[idx] = arg;
                    argtab->tag[idx] = argtag;

                    DBUG_PRINT ("PREC",
                                ("%s(): in-arg " F_PTR "," F_PTR
                                 " (ARG,TYPE) merged with out-arg " F_PTR
                                 " (TYPE) with tag %s.",
                                 FUNDEF_NAME (fundef), arg, ARG_NTYPE (arg),
                                 argtab->ptr_out[idx], global.argtag_string[argtag]));
                } else {
                    /*
                     * there is already an outptr, so both must have no descriptor and
                     * the types must be equal
                     */
                    if ((argtab->tag[idx] == ATG_out_nodesc)
                        && (argtag == ATG_in_nodesc)) {
                        /*
                         * merge 'argtab->ptr_out[idx]' and 'arg'
                         */
                        if (TYeqTypes (RET_TYPE (argtab->ptr_out[idx]),
                                       ARG_NTYPE (arg))) {
                            if (TUisBoxed (ARG_NTYPE (arg))) {
                                argtag = ATG_inout_nodesc_bx;
                            } else {
                                argtag = ATG_inout_nodesc;
                            }

                            argtab->ptr_in[idx] = arg;
                            argtab->tag[idx] = argtag;

                            DBUG_PRINT ("PREC", ("%s(): in-arg " F_PTR "," F_PTR
                                                 " (ARG,TYPE) merged with out-arg " F_PTR
                                                 " (TYPE) with tag %s.",
                                                 FUNDEF_NAME (fundef), arg,
                                                 ARG_NTYPE (arg), argtab->ptr_out[idx],
                                                 global.argtag_string[argtag]));
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
                }
            } else {
                ERROR (line, ("Pragma 'linksign' illegal"));
                CONT_ERROR (("In-parameter at position %d found twice", idx));
            }
        } else {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("Entry contains illegal value %d", idx));
        }
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

    INFO_FPC_FUNDEF (arg_info) = arg_node;

    if (!FUNDEF_ISZOMBIE (arg_node)) {
        INFO_FPC_ARGTAB (arg_info) = TBmakeArgtab (TCcountFunctionParams (arg_node) + 1);

        if (FUNDEF_RETS (arg_node) != NULL) {
            FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
        }
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
        ABORT_ON_ERROR;

        /*
         * assign the argtab to the function
         */
        FUNDEF_ARGTAB (arg_node) = INFO_FPC_ARGTAB (arg_info);

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
        INFO_FPC_POSTASSIGNS (arg_info) = NULL;
        INFO_FPC_PREASSIGNS (arg_info) = NULL;
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

    INFO_FPC_ARGTAB (arg_info)
      = InsertIntoOut (INFO_FPC_ARGTAB (arg_info), INFO_FPC_FUNDEF (arg_info), arg_node);

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

    INFO_FPC_ARGTAB (arg_info)
      = InsertIntoIn (INFO_FPC_ARGTAB (arg_info), INFO_FPC_FUNDEF (arg_info), arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

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

    DBUG_ASSERT ((argtab->ptr_in[idx] != arg), "no index for in-parameter found!");

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

        DBUG_PRINT ("PREC", ("Application of %s:%s().", FUNDEF_MOD (fundef),
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
            DBUG_ASSERT ((rets != NULL), "application is inconsistent");

            if (dots_offset == 0) {
                /*
                 * while handling true return values, get index
                 */
                idx = GetArgtabIndexOut (rets, argtab);
            }
            DBUG_ASSERT ((idx + dots_offset < ap_argtab->size), "illegal index");

            DBUG_ASSERT ((idx < argtab->size), "illegal index");

            ap_argtab->ptr_out[idx + dots_offset] = ids;
            ap_argtab->tag[idx + dots_offset] = argtab->tag[idx];

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
            DBUG_ASSERT ((args != NULL), "application is inconsistant");

            if (dots_offset == 0) {
                idx = GetArgtabIndexIn (args, argtab);
            }
            DBUG_ASSERT ((idx + dots_offset < ap_argtab->size), "illegal index");

            DBUG_ASSERT ((idx < argtab->size), "illegal index");

            ap_argtab->ptr_in[idx + dots_offset] = exprs;
            ap_argtab->tag[idx + dots_offset] = argtab->tag[idx];

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

        ABORT_ON_ERROR;

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
