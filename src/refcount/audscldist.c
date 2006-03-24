/*
 *  $Id$
 */

/**
 * @defgroup asd AUD SCL distincion
 * @ingroup emm
 *
 * @{
 */

/** <!-- ****************************************************************** -->
 *
 * @file audscldist.c
 *
 *   - Arguments of function applications are lifted if formal and actual types
 *     differ:
 *       b = fun( a);   =>   _tmp_a = copy(a); b = fun( _tmp_a);
 *     Return values of function applications are lifted if formal and actual
 *     types differ:
 *       b = fun( a);   =>   _tmp_b = fun( a); b = copy(_tmp_b);
 *
 ******************************************************************************/

#include "audscldist.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "ctinfo.h"
#include "traverse.h"
#include "NameTuplesUtils.h"
#include "new_types.h"
#include "shape.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *assign;
    node *preassigns;
    node *postassigns;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_ASSIGN(n) ((n)->assign)
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
    INFO_ASSIGN (result) = NULL;
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

/** <!--********************************************************************-->
 *
 * @fn node *ASDdoAudSclDistinction( node *syntax_tree)
 *
 *  @param syntax_tree
 *
 *  @return the modified syntaxtree
 *
 *****************************************************************************/
node *
ASDdoAudSclDistinction (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ASDdoAudSclDistinction");

    info = MakeInfo ();

    TRAVpush (TR_asd);
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
 *   node *LiftId(  node *id, ntype *new_type, info *arg_info)
 *
 * Description:
 *   Lifts the given id of a expr position
 *    - Generates a new and fresh varname.
 *    - Generates a new vardec and inserts it into the vardec chain of 'fundef'.
 *      If 'new_type' is not NULL, 'new_type' is used as VARDEC_TYPE instead
 *      of ID_TYPE(arg).
 *    - Builds a new copy assignment and inserts it into the assignment chain
 *      'new_assigns'.
 *
 *   Abstract the given argument out:
 *     ... = fun( A, ...);
 *   is transformed into
 *     A' = expr( A);
 *     ... = fun( A', ...);
 *
 ******************************************************************************/

static void
LiftId (node *id, ntype *new_type, info *arg_info)
{
    char *new_name;
    node *new_ids;
    node *new_avis;
    node *fundef;

    DBUG_ENTER ("LiftId");

    new_name = ILIBtmpVarName (ID_NAME (id));

    if (new_type == NULL) {
        new_type = ID_NTYPE (id);
    }

    new_avis = TBmakeAvis (new_name, TYcopyType (new_type));

    /*
     * Insert vardec for new var
     */
    fundef = INFO_FUNDEF (arg_info);
    FUNDEF_VARDEC (fundef) = TBmakeVardec (new_avis, FUNDEF_VARDEC (fundef));

    new_ids = TBmakeIds (new_avis, NULL);

    INFO_PREASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (new_ids, TCmakePrf1 (F_copy, TBmakeId (ID_AVIS (id)))),
                      INFO_PREASSIGNS (arg_info));

    AVIS_SSAASSIGN (new_avis) = INFO_PREASSIGNS (arg_info);

    ID_AVIS (id) = new_avis;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void LiftIds( node *ids_arg, ntype *new_type, info *arg_info)
 *
 * Description:
 *   Lifts the given return value of a function application:
 *    - Generates a new and fresh varname.
 *    - Generates a new vardec and inserts it into the vardec chain of 'fundef'.
 *      If 'new_type' is not NULL, 'new_type' is used as VARDEC_TYPE instead
 *      of IDS_VARDEC(ids_arg).
 *    - Builds a new copy assignment and inserts it into the assignment chain
 *      'new_assigns'.
 *    - Adjusts the name and vardec of 'ids_arg'.
 *
 *   Abstract the found return value out:
 *     A = fun( ...);
 *     ... A ... A ...    // n references of A
 *   is transformed into
 *     A' = fun( ...);
 *     A = copy( A');
 *     ... A ... A ...    // n references of A
 *
 ******************************************************************************/

static void
LiftIds (node *ids_arg, ntype *new_type, info *arg_info)
{
    char *new_name;
    node *new_id;
    node *new_avis;
    node *fundef;
    node *assign;

    DBUG_ENTER ("LiftIds");

    fundef = INFO_FUNDEF (arg_info);
    assign = INFO_ASSIGN (arg_info);

    new_name = ILIBtmpVarName (IDS_NAME (ids_arg));

    /*
     * Insert vardec for new var
     */
    if (new_type == NULL) {
        new_type = IDS_NTYPE (ids_arg);
    }

    new_avis = TBmakeAvis (new_name, TYcopyType (new_type));

    FUNDEF_VARDEC (fundef) = TBmakeVardec (new_avis, FUNDEF_VARDEC (fundef));

    new_id = TBmakeId (new_avis);
    INFO_POSTASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (ids_arg), NULL),
                                 TCmakePrf1 (F_copy, new_id)),
                      INFO_POSTASSIGNS (arg_info));

    AVIS_SSAASSIGN (IDS_AVIS (ids_arg)) = INFO_POSTASSIGNS (arg_info);

    IDS_AVIS (ids_arg) = new_avis;

    AVIS_SSAASSIGN (new_avis) = assign;

    DBUG_VOID_RETURN;
}

/*
 * traversal functions
 */

/******************************************************************************
 *
 * function:
 *   node *ASDmodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
ASDmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASDmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *ASDfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 ******************************************************************************/
node *
ASDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASDfundef");

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    INFO_FUNDEF (arg_info) = arg_node;

    INFO_POSTASSIGNS (arg_info) = NULL;
    INFO_PREASSIGNS (arg_info) = NULL;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *ASDassign( node *arg_node, info *arg_info)
 *
 * Description:
 *   Inserts the assignments found in INFO_PRE/POSTASSIGNS into the AST.
 *
 ******************************************************************************/
node *
ASDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASDassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_ASSIGN (arg_info) = arg_node;

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
 *   node *ASDap( node *arg_node, info *arg_info)
 *
 * Description:
 *   Builds AP_ARGTAB and generates merging assignments for inout-arguments
 *   if needed.
 *
 ******************************************************************************/
node *
ASDap (node *arg_node, info *arg_info)
{
    node *ids, *exprs, *id;
    node *ret, *arg;
    shape_class_t actual_cls, formal_cls;

    DBUG_ENTER ("ASDap");

    DBUG_PRINT ("ASD", ("analyzing application of %s ......",
                        CTIitemName (AP_FUNDEF (arg_node))));

    ret = FUNDEF_RETS (AP_FUNDEF (arg_node));
    ids = ASSIGN_LHS (INFO_ASSIGN (arg_info));

    while (ret != NULL) {
        actual_cls = NTUgetShapeClassFromNType (IDS_NTYPE (ids));
        formal_cls = NTUgetShapeClassFromNType (RET_TYPE (ret));

        if ((actual_cls != formal_cls)
            && ((actual_cls == C_scl) || (formal_cls == C_scl))) {
            DBUG_PRINT ("ASD", ("Return value with inappropriate shape class found:"));
            DBUG_PRINT ("ASD", ("   ... %s ... = %s( ... ), %s instead of %s",
                                FUNDEF_NAME (INFO_FUNDEF (arg_info)), IDS_NAME (ids),
                                global.nt_shape_string[actual_cls],
                                global.nt_shape_string[formal_cls]));
            LiftIds (ids, RET_TYPE (ret), arg_info);
        }

        ret = RET_NEXT (ret);
        ids = IDS_NEXT (ids);
    }

    arg = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    exprs = AP_ARGS (arg_node);

    while (arg != NULL) {
        id = EXPRS_EXPR (exprs);

        actual_cls = NTUgetShapeClassFromNType (ID_NTYPE (id));
        formal_cls = NTUgetShapeClassFromNType (ARG_NTYPE (arg));

        if ((actual_cls != formal_cls)
            && ((actual_cls == C_scl) || (formal_cls == C_scl))) {
            DBUG_PRINT ("ASD", ("Argument with inappropriate shape class found:"));
            DBUG_PRINT ("ASD", ("   ... = %s( ... %s ...), %s instead of %s",
                                FUNDEF_NAME (INFO_FUNDEF (arg_info)), ID_NAME (id),
                                global.nt_shape_string[actual_cls],
                                global.nt_shape_string[formal_cls]));

            LiftId (id, ARG_NTYPE (arg), arg_info);
        }

        arg = ARG_NEXT (arg);
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *ASDcond( node *arg_node, info *arg_info)
 *
 * Description:
 *   inserts an assignment to ensure that the condition is an SCL of type
 *   bool, if needed
 *
 ******************************************************************************/
node *
ASDcond (node *arg_node, info *arg_info)
{
    shape_class_t cond_cls;
    ntype *cond_type;
    node *funcond_ass;

    DBUG_ENTER ("ASDcond");

    funcond_ass = ASSIGN_NEXT (INFO_ASSIGN (arg_info));

    /*
     * first traverse the two blocks
     * as we insert the assigns on our way up
     */
    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    }

    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }

    if (NODE_TYPE (COND_COND (arg_node)) == N_id) {
        cond_type = AVIS_TYPE (ID_AVIS (COND_COND (arg_node)));
        cond_cls = NTUgetShapeClassFromNType (cond_type);

        if (cond_cls != C_scl) {

            /*
             * non scalar cond-var found! we need to
             * insert an extra assignment to trigger a
             * type assertion and make sure that
             * the cond is a scalar
             */
            LiftId (COND_COND (arg_node),
                    TYmakeAKS (TYcopyType (TYgetScalar (cond_type)), SHmakeShape (0)),
                    arg_info);

            /*
             * Exchange predicate identifier in all subsequent funcond nodes
             */
            while ((NODE_TYPE (ASSIGN_INSTR (funcond_ass)) == N_let)
                   && (NODE_TYPE (ASSIGN_RHS (funcond_ass)) == N_funcond)) {
                ID_AVIS (FUNCOND_IF (ASSIGN_RHS (funcond_ass)))
                  = ID_AVIS (COND_COND (arg_node));

                funcond_ass = ASSIGN_NEXT (funcond_ass);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/* @} */
