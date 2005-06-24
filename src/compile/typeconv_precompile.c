/*
 *
 * $Log$
 * Revision 1.8  2005/06/24 16:09:23  sah
 * added insertion of type conversion assignments
 * for conditionals
 *
 * Revision 1.7  2005/06/18 13:13:49  sah
 * bugfixing
 *
 * Revision 1.6  2005/05/26 18:44:16  sbs
 * some DBUG prints added and unused vars eliminated
 *
 * Revision 1.5  2004/12/09 21:09:58  ktr
 * bugfix roundup
 *
 * Revision 1.4  2004/12/08 11:19:45  sah
 * fixed a not-enough-sleep-in-dk bug
 *
 * Revision 1.3  2004/12/07 17:46:23  sah
 * rearranged code to fix segfault
 *
 * Revision 1.2  2004/11/27 02:39:27  ktr
 * errorcorrection.
 *
 * Revision 1.1  2004/11/27 02:35:50  ktr
 * Initial revision
 *
 * Revision 1.2  2004/11/27 00:16:00  ktr
 * New barebones precompile.
 *
 * Revision 1.1  2004/11/26 17:53:24  sah
 * Initial revision
 *
 *
 *
 */

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
#define INFO_TCP_FUNDEF(n) ((n)->fundef)
#define INFO_TCP_PREASSIGNS(n) ((n)->preassigns)
#define INFO_TCP_POSTASSIGNS(n) ((n)->postassigns)

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
 *   node *LiftId( node *id, node *fundef,
 *                 ntype *ntype, node **new_assigns)
 *
 * Description:
 *   Lifts the given id of a cond or do
 *    - Generates a new and fresh varname.
 *    - Generates a new vardec and inserts it into the vardec chain of 'fundef'.
 *      If 'new_type' is not NULL, 'new_type' is used as VARDEC_TYPE instead
 *      of ID_TYPE(arg).
 *    - Builds a new assignment and inserts it into the assignment chain
 *      'new_assigns'.
 *
 ******************************************************************************/
static void
LiftId (node *id, node *fundef, ntype *new_type, node **new_assigns)
{
    char *new_name;
    node *new_ids;
    node *new_avis;

    DBUG_ENTER ("LiftId");

    new_name = ILIBtmpVarName (ID_NAME (id));

    /*
     * Insert vardec for new var
     */
    if (new_type == NULL) {
        new_type = ID_NTYPE (id);
    }

    new_avis = TBmakeAvis (new_name, TYcopyType (new_type));

    FUNDEF_VARDEC (fundef) = TBmakeVardec (new_avis, FUNDEF_VARDEC (fundef));

    /*
     * Abstract the given argument out:
     *   ... = cond ( A, ...) || while ( A )
     * is transformed into
     *   A' = A;
     *   ... = cond ( A', ...) || while ( A')
     */
    new_ids = TBmakeIds (new_avis, NULL);

    (*new_assigns)
      = TBmakeAssign (TBmakeLet (new_ids, TBmakeId (ID_AVIS (id))), (*new_assigns));

    ID_AVIS (id) = new_avis;

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

    DBUG_PRINT ("TCP",
                ("analyzing application of %s:%s ......",
                 FUNDEF_MOD (AP_FUNDEF (arg_node)), FUNDEF_NAME (AP_FUNDEF (arg_node))));

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

            actual_cls = NTUgetShapeClassFromNType (ID_NTYPE (id));
            formal_cls = NTUgetShapeClassFromNType (ARG_NTYPE (arg));

            if ((actual_cls != formal_cls)
                && (global.argtag_has_shp[fun_argtab->tag[idx]] || (actual_cls == C_scl)
                    || (formal_cls == C_scl))) {
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

/******************************************************************************
 *
 * Function:
 *   node *TCPcond( node *arg_node, info *arg_info)
 *
 * Description:
 *   inserts an assignment to ensure that the condition is an SCL of type
 *   bool, if needed
 *
 ******************************************************************************/
node *
TCPcond (node *arg_node, info *arg_info)
{
    shape_class_t cond_cls;
    ntype *cond_type;

    DBUG_ENTER ("TCPcond");

    cond_type = AVIS_TYPE (ID_AVIS (COND_COND (arg_node)));
    cond_cls = NTUgetShapeClassFromNType (cond_type);

    if (cond_cls != C_scl) {
        /*
         * non scalar cond-var found! we need to
         * insert an extra assignment to trigger a
         * type assertion and make sure that
         * the cond is a scalar
         */
        LiftId (COND_COND (arg_node), INFO_TCP_FUNDEF (arg_info),
                TYmakeAKS (TYcopyType (TYgetScalar (cond_type)), SHmakeShape (0)),
                &(INFO_TCP_PREASSIGNS (arg_info)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *TCPdo( node *arg_node, info *arg_info)
 *
 * Description:
 *   inserts an assignment to ensure that the condition is an SCL of type
 *   bool, if needed
 *
 ******************************************************************************/
node *
TCPdo (node *arg_node, info *arg_info)
{
    shape_class_t do_cls;
    ntype *do_type;

    DBUG_ENTER ("TCPdo");

    do_type = AVIS_TYPE (ID_AVIS (DO_COND (arg_node)));
    do_cls = NTUgetShapeClassFromNType (do_type);

    if (do_cls != C_scl) {
        /*
         * non scalar cond-var found! we need to
         * insert an extra assignment to trigger a
         * type assertion and make sure that
         * the cond is a scalar
         */
        LiftId (DO_COND (arg_node), INFO_TCP_FUNDEF (arg_info),
                TYmakeAKS (TYcopyType (TYgetScalar (do_type)), SHmakeShape (0)),
                &(INFO_TCP_PREASSIGNS (arg_info)));
    }

    DBUG_RETURN (arg_node);
}

/* @} */
