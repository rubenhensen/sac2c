/* $Id$ */

/**
 * @defgroup tcp Type conversions precompilation
 * @ingroup prec Precompile
 *
 *
 *
 * @{
 */

/** <!-- ****************************************************************** -->
 *
 * @file typeconv_precompile.c
 *
 *   - Arguments of function applications are lifted if formal and actual types
 *     differ:
 *       b = fun( a);   =>   _tmp_a = a; b = fun( _tmp_a);
 *     Return values of function applications are lifted if formal and actual
 *     types differ:
 *       b = fun( a);   =>   _tmp_b = fun( a); b = _tmp_b;
 *
 ******************************************************************************/
#include "typeconv_precompile.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "traverse.h"
#include "NameTuplesUtils.h"
#include "new_types.h"
#include "shape.h"
#include "namespaces.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *preassigns;
    node *postassigns;
    node *endblockassigns;
};

/*
 * INFO macros
 */
#define INFO_TCP_FUNDEF(n) ((n)->fundef)
#define INFO_TCP_PREASSIGNS(n) ((n)->preassigns)
#define INFO_TCP_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_TCP_ENDBLOCKASSIGNS(n) ((n)->endblockassigns)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_TCP_FUNDEF (result) = NULL;
    INFO_TCP_PREASSIGNS (result) = NULL;
    INFO_TCP_POSTASSIGNS (result) = NULL;
    INFO_TCP_ENDBLOCKASSIGNS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *TCPdoTypeConversions( node *syntax_tree)
 *
 *  @param syntax_tree
 *
 *  @return the modified syntaxtree
 *
 *****************************************************************************/
node *
TCPdoTypeConversions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("TCPdoTypeConversions");

    info = MakeInfo ();

    TRAVpush (TR_tcp);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Function:
 *   node *LiftArg( node *arg, node *fundef,
 *                  ntype *ntype, node **new_assigns)
 *
 * Description:
 *   Lifts the given argument of a function application:
 *    - Generates a new and fresh varname.
 *    - Generates a new vardec and inserts it into the vardec chain of 'fundef'.
 *      If 'new_type' is not NULL, 'new_type' is used as VARDEC_TYPE instead
 *      of ID_TYPE(arg).
 *    - Builds a new assignment and inserts it into the assignment chain
 *      'new_assigns'.
 *
 ******************************************************************************/
static void
LiftArg (node *arg, node *fundef, ntype *new_type, node **new_assigns)
{
    char *new_name;
    node *new_ids;
    node *new_avis;

    DBUG_ENTER ("LiftArg");

    new_name = ILIBtmpVarName (ID_NAME (arg));

    /*
     * Insert vardec for new var
     */
    if (new_type == NULL) {
        new_type = ID_NTYPE (arg);
    }

    new_avis = TBmakeAvis (new_name, TYcopyType (new_type));

    FUNDEF_VARDEC (fundef) = TBmakeVardec (new_avis, FUNDEF_VARDEC (fundef));

    /*
     * Abstract the given argument out:
     *   ... = fun( A, ...);
     * is transformed into
     *   A' = A;
     *   ... = fun( A', ...);
     */
    new_ids = TBmakeIds (new_avis, NULL);

    (*new_assigns)
      = TBmakeAssign (TBmakeLet (new_ids, TBmakeId (ID_AVIS (arg))), (*new_assigns));

    ID_AVIS (arg) = new_avis;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void LiftIds( ids *ids_arg, node *fundef,
 *                 types *new_type, node **new_assigns)
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
LiftIds (node *ids_arg, node *fundef, ntype *new_type, node **new_assigns)
{
    char *new_name;
    node *new_id;
    node *new_avis;

    DBUG_ENTER ("LiftIds");

    new_name = ILIBtmpVarName (IDS_NAME (ids_arg));

    /*
     * Insert vardec for new var
     */
    if (new_type == NULL) {
        new_type = IDS_NTYPE (ids_arg);
    }

    new_avis = TBmakeAvis (new_name, TYcopyType (new_type));

    FUNDEF_VARDEC (fundef) = TBmakeVardec (new_avis, FUNDEF_VARDEC (fundef));

    /*
     * Abstract the found return value out:
     *   A = fun( ...);
     *   ... A ... A ...    // n references of A
     * is transformed into
     *   A' = fun( ...);
     *   A = A';
     *   ... A ... A ...    // n references of A
     */
    new_id = TBmakeId (new_avis);
    (*new_assigns)
      = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (ids_arg), NULL), new_id),
                      *new_assigns);

    IDS_AVIS (ids_arg) = new_avis;

    DBUG_VOID_RETURN;
}

/*
 * traversal functions
 */

/******************************************************************************
 *
 * function:
 *   node *TCPmodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
TCPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TCPmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *TCPfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 ******************************************************************************/
node *
TCPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TCPfundef");

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    INFO_TCP_FUNDEF (arg_info) = arg_node;

    INFO_TCP_POSTASSIGNS (arg_info) = NULL;
    INFO_TCP_PREASSIGNS (arg_info) = NULL;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *TCPassign( node *arg_node, info *arg_info)
 *
 * Description:
 *   Inserts the assignments found in INFO_TCP_PRE/POSTASSIGNS into the AST.
 *
 ******************************************************************************/
node *
TCPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TCPassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if ((ASSIGN_NEXT (arg_node) == NULL)
        && (INFO_TCP_ENDBLOCKASSIGNS (arg_info) != NULL)) {
        ASSIGN_NEXT (arg_node) = INFO_TCP_ENDBLOCKASSIGNS (arg_info);
        INFO_TCP_ENDBLOCKASSIGNS (arg_info) = NULL;
    }

    if (INFO_TCP_POSTASSIGNS (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_TCP_POSTASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
        INFO_TCP_POSTASSIGNS (arg_info) = NULL;
    }

    if (INFO_TCP_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_TCP_PREASSIGNS (arg_info), arg_node);
        INFO_TCP_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ***************************************************************** -->
 * @brief removes prfs of type F_type_conv by replacing them with
 *        their second argument.
 *
 * @param arg_node N_prf node
 * @param arg_info info structure
 *
 * @return N_prf node != N_type_conv or the second argument of the given
 *         prf
 */
node *
TCPprf (node *arg_node, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ("TCPprf");

    if (PRF_PRF (arg_node) == F_type_conv) {
        /*
         * replace expressions of form:
         *     type_conv( <type>, <expr>)
         * by
         *     <expr>
         */
        tmp = arg_node;
        arg_node = PRF_ARG2 (tmp);
        PRF_ARG2 (tmp) = NULL;

        tmp = FREEdoFreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *TCPap( node *arg_node, info *arg_info)
 *
 * Description:
 *   Builds AP_ARGTAB and generates merging assignments for inout-arguments
 *   if needed.
 *
 ******************************************************************************/
node *
TCPap (node *arg_node, info *arg_info)
{
    argtab_t *fun_argtab, *ap_argtab;
    node *ids, *id;
    node *ret, *arg;
    int idx;
    shape_class_t actual_cls, formal_cls;

    DBUG_ENTER ("TCPap");

    DBUG_PRINT ("TCP", ("analyzing application of %s::%s ......",
                        NSgetName (AP_NS (arg_node)), AP_NAME (arg_node)));

    fun_argtab = FUNDEF_ARGTAB (AP_FUNDEF (arg_node));
    ap_argtab = AP_ARGTAB (arg_node);

    idx = 0;

    while (idx < fun_argtab->size) {
        ret = fun_argtab->ptr_out[idx];
        ids = ap_argtab->ptr_out[idx];

        if (ret != NULL) {
            DBUG_ASSERT (ids != NULL, "Malformed argtab!");

            actual_cls = NTUgetShapeClassFromNType (IDS_NTYPE (ids));
            formal_cls = NTUgetShapeClassFromNType (RET_TYPE (ret));

            if ((actual_cls != formal_cls)
                && (global.argtag_has_shp[fun_argtab->tag[idx]] || (actual_cls == C_scl)
                    || (formal_cls == C_scl))) {
                DBUG_ASSERT ((actual_cls != C_scl) && (formal_cls != C_scl),
                             "Conversion from or to scalar encountered!");

                DBUG_PRINT ("TCP",
                            ("Return value with inappropriate shape class found:"));
                DBUG_PRINT ("TCP", ("   ... %s ... = %s( ... ), %s instead of %s",
                                    FUNDEF_NAME (INFO_TCP_FUNDEF (arg_info)),
                                    IDS_NAME (ids), global.nt_shape_string[actual_cls],
                                    global.nt_shape_string[formal_cls]));
                LiftIds (ids, INFO_TCP_FUNDEF (arg_info), RET_TYPE (ret),
                         &(INFO_TCP_POSTASSIGNS (arg_info)));
            }
        }

        arg = fun_argtab->ptr_in[idx];

        if (arg != NULL) {
            id = EXPRS_EXPR (ap_argtab->ptr_in[idx]);

            DBUG_ASSERT (id != NULL, "Malformed argtab!");

            /*
             * global objects might appear in argument position here
             * so ensure they are handeled correctly!
             */
            if (NODE_TYPE (id) == N_globobj) {
                actual_cls
                  = NTUgetShapeClassFromNType (OBJDEF_TYPE (GLOBOBJ_OBJDEF (id)));
            } else {
                actual_cls = NTUgetShapeClassFromNType (ID_NTYPE (id));
            }
            formal_cls = NTUgetShapeClassFromNType (ARG_NTYPE (arg));

            if ((actual_cls != formal_cls)
                && (global.argtag_has_shp[fun_argtab->tag[idx]] || (actual_cls == C_scl)
                    || (formal_cls == C_scl))) {
                DBUG_ASSERT ((actual_cls != C_scl) && (formal_cls != C_scl),
                             "Conversion from or to scalar encountered!");

                DBUG_ASSERT ((NODE_TYPE (id) != N_globobj),
                             "possible lifting of global object encountered!");

                DBUG_PRINT ("TCP", ("Argument with inappropriate shape class found:"));
                DBUG_PRINT ("TCP", ("   ... = %s( ... %s ...), %s instead of %s",
                                    FUNDEF_NAME (INFO_TCP_FUNDEF (arg_info)),
                                    ID_NAME (id), global.nt_shape_string[actual_cls],
                                    global.nt_shape_string[formal_cls]));
                LiftArg (id, INFO_TCP_FUNDEF (arg_info), ARG_NTYPE (arg),
                         &(INFO_TCP_PREASSIGNS (arg_info)));
            }
        }
        idx++;
    }

    DBUG_RETURN (arg_node);
}

/* @} */
