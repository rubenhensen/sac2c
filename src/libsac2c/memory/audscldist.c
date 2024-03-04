/** <!--*********************************************************************-->
 *
 * @defgroup asd AUD SCL distincion
 *
 *   Disambiguates AUD variables and scalars which have different
 *   representations.
 *
 *   Arguments of function applications are lifted if formal and actual types
 *   differ:
 *   <pre>
 *       b = fun( a);   =>   _tmp_a = copy(a); b = fun( _tmp_a);
 *   </pre>
 *   Return values of function applications are lifted if formal and actual
 *   types differ:
 *   <pre>
 *       b = fun( a);   =>   _tmp_b = fun( a); b = copy(_tmp_b);
 *   </pre>
 *
 *   Furthermore, we have to do a similar transformation for funconds/conditions.
 *   Firstly, we have to ensure that the predicate is a scalar variable. If it
 *   is not, we insert an assignment of the form
 *   <pre>
 *       if (p) ...     =>  _tmp_p = _copy_(p); if (_tmp_p) ...
 *   </pre>
 *   and update the funcond accordingly.
 *   Secondly, if the shape class of the result of a funcond and the shape class
 *   of the second or third argument do not match, we insert a copy like
 *   <pre>
 *       x = funcond(p, a, b)  =>  _tmp_a = a; x = funcond(p, _tmp_a, b);
 *   </pre>
 *
 * @ingroup mm
 *
 * @{
 *
 ******************************************************************************/

/** <!--*********************************************************************-->
 *
 * @file audscldist.c
 *
 * Prefix: ASD
 *
 ******************************************************************************/

#include "audscldist.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "ctinfo.h"
#include "traverse.h"
#include "NameTuplesUtils.h"
#include "new_types.h"
#include "shape.h"
#include "new_typecheck.h"

#define DBUG_PREFIX "ASD"
#include "debug.h"

#include "globals.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *assign;
    node *preassigns;
    node *postassigns;
    node *elseassigns;
    node *thenassigns;

    node *lhs;
    node *withop;
    ntype *cextypes;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_ASSIGN(n) ((n)->assign)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_THENASSIGNS(n) ((n)->thenassigns)
#define INFO_ELSEASSIGNS(n) ((n)->elseassigns)

#define INFO_LHS(n) ((n)->lhs)
#define INFO_WITHOP(n) ((n)->withop)
#define INFO_CEXTYPES(n) ((n)->cextypes)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_THENASSIGNS (result) = NULL;
    INFO_ELSEASSIGNS (result) = NULL;
    INFO_WITHOP (result) = NULL;
    INFO_CEXTYPES (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

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

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_asd);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 *  @fn ntype *GetLeastTypes(  ntype *p1, ntype *p2)
 *
 ******************************************************************************/
static ntype *
GetLeastTypes (ntype *p1, ntype *p2)
{
    ntype *res;

    DBUG_ENTER ();

    if ((p1 == NULL) || (p2 == NULL)) {
        res = p1 == NULL ? p2 : p1;
    } else {
        size_t i;

        res = TYmakeEmptyProductType (TYgetProductSize (p1));

        for (i = 0; i < TYgetProductSize (p1); i++) {
            ntype *t1, *t2;

            t1 = TYeliminateAKV (TYgetProductMember (p1, i));
            t2 = TYeliminateAKV (TYgetProductMember (p2, i));

            TYsetProductMember (res, i, TYcopyType (TYleTypes (t1, t2) ? t1 : t2));

            t1 = TYfreeType (t1);
            t2 = TYfreeType (t2);
        }

        p1 = TYfreeType (p1);
        p2 = TYfreeType (p2);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 *  @fn node *LiftId(  node *id, ntype *new_type, node *fundef,
 *                     node **new_assigns)
 *
 *  @brief Lifts the given id of a expr position
 *
 *    - Generates a new and fresh varname.
 *
 *    - Generates a new vardec and inserts it into the vardec chain of 'fundef'.
 *      If 'new_type' is not NULL, 'new_type' is used as VARDEC_TYPE instead
 *      of ID_TYPE(arg).
 *
 *    - Builds a new copy assignment and inserts it into the assignment chain
 *      'new_assigns'.
 *
 *   Abstract the given argument out:
 *   <pre>
 *     ... = fun( A, ...);
 *   is transformed into
 *     A' = expr( A);
 *     ... = fun( A', ...);
 *   </pre>
 *
 ******************************************************************************/
static void
LiftId (node *id, ntype *new_type, node *fundef, node **new_assigns)
{
    char *new_name;
    node *new_ids;
    node *new_avis;

    DBUG_ENTER ();

    new_name = TRAVtmpVarName (ID_NAME (id));

    if (new_type == NULL) {
        new_type = ID_NTYPE (id);
    }

    new_avis = TBmakeAvis (new_name, TYcopyType (new_type));

    /*
     * Insert vardec for new var
     */
    FUNDEF_VARDECS (fundef) = TBmakeVardec (new_avis, FUNDEF_VARDECS (fundef));

    new_ids = TBmakeIds (new_avis, NULL);

    *new_assigns
      = TBmakeAssign (TBmakeLet (new_ids, TCmakePrf1 (F_copy, TBmakeId (ID_AVIS (id)))),
                      *new_assigns);

    AVIS_SSAASSIGN (new_avis) = *new_assigns;

    ID_AVIS (id) = new_avis;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void LiftIds( node *ids_arg, ntype *new_type, node *fundef,
 *                   node **new_assigns, node *new_ssaassign)
 *
 * @brief Lifts the given return value of a function application:
 *
 *    - Generates a new and fresh varname.
 *
 *    - Generates a new vardec and inserts it into the vardec chain of 'fundef'.
 *      If 'new_type' is not NULL, 'new_type' is used as VARDEC_TYPE instead
 *      of IDS_VARDEC(ids_arg).
 *
 *    - Builds a new copy assignment and inserts it into the assignment chain
 *      'new_assigns'.
 *
 *    - Adjusts the name and vardec of 'ids_arg'.
 *
 *   Abstract the found return value out:
 *   <pre>
 *     A = fun( ...);
 *     ... A ... A ...    // n references of A
 *   is transformed into
 *     A' = fun( ...);
 *     A = copy( A');
 *     ... A ... A ...    // n references of A
 *   </pre>
 *
 ******************************************************************************/
static void
LiftIds (node *ids_arg, ntype *new_type, node *fundef, node **new_assigns,
         node *new_ssaassign)
{
    char *new_name;
    node *new_id;
    node *new_avis;

    DBUG_ENTER ();

    new_name = TRAVtmpVarName (IDS_NAME (ids_arg));

    /*
     * Insert vardec for new var
     */
    if (new_type == NULL) {
        new_type = IDS_NTYPE (ids_arg);
    }

    new_avis = TBmakeAvis (new_name, TYcopyType (new_type));

    FUNDEF_VARDECS (fundef) = TBmakeVardec (new_avis, FUNDEF_VARDECS (fundef));

    new_id = TBmakeId (new_avis);
    *new_assigns = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (ids_arg), NULL),
                                            TCmakePrf1 (F_copy, new_id)),
                                 *new_assigns);

    AVIS_SSAASSIGN (IDS_AVIS (ids_arg)) = *new_assigns;

    IDS_AVIS (ids_arg) = new_avis;

    AVIS_SSAASSIGN (new_avis) = new_ssaassign;

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

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
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

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
    DBUG_ENTER ();

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    INFO_FUNDEF (arg_info) = arg_node;

    INFO_POSTASSIGNS (arg_info) = NULL;
    INFO_PREASSIGNS (arg_info) = NULL;

    FUNDEF_BODY (arg_node) = TRAVopt(FUNDEF_BODY (arg_node), arg_info);

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
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    INFO_ASSIGN (arg_info) = arg_node;

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

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

    DBUG_ENTER ();

    DBUG_PRINT ("analyzing application of %s ......", CTIitemName (AP_FUNDEF (arg_node)));

    ret = FUNDEF_RETS (AP_FUNDEF (arg_node));

    if (!AP_ISSPAWNED (arg_node)) {
        ids = ASSIGN_LHS (INFO_ASSIGN (arg_info));
    } else {
        ids = ASSIGN_SYNC_LHS (INFO_ASSIGN (arg_info));
    }

    while (ret != NULL) {
        actual_cls = NTUgetShapeClassFromNType (IDS_NTYPE (ids));
        formal_cls = NTUgetShapeClassFromNType (RET_TYPE (ret));

        if ((actual_cls != formal_cls)
            && ((actual_cls == C_scl) || (formal_cls == C_scl))) {
            DBUG_PRINT ("Return value with inappropriate shape class found:");
            DBUG_PRINT ("   ... %s ... = %s( ... ), %s instead of %s",
                        FUNDEF_NAME (INFO_FUNDEF (arg_info)), IDS_NAME (ids),
                        global.nt_shape_string[actual_cls],
                        global.nt_shape_string[formal_cls]);
            LiftIds (ids, RET_TYPE (ret), INFO_FUNDEF (arg_info),
                     &INFO_POSTASSIGNS (arg_info), INFO_ASSIGN (arg_info));
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
            DBUG_PRINT ("Argument with inappropriate shape class found:");
            DBUG_PRINT ("   ... = %s( ... %s ...), %s instead of %s",
                        FUNDEF_NAME (INFO_FUNDEF (arg_info)), ID_NAME (id),
                        global.nt_shape_string[actual_cls],
                        global.nt_shape_string[formal_cls]);

            LiftId (id, ARG_NTYPE (arg), INFO_FUNDEF (arg_info),
                    &INFO_PREASSIGNS (arg_info));
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

    DBUG_ENTER ();

    funcond_ass = ASSIGN_NEXT (INFO_ASSIGN (arg_info));

    /*
     * insert assignments produced at funconds */
    if (INFO_THENASSIGNS (arg_info) != NULL) {
        BLOCK_ASSIGNS (COND_THEN (arg_node))
          = TCappendAssign (BLOCK_ASSIGNS (COND_THEN (arg_node)),
                            INFO_THENASSIGNS (arg_info));
        INFO_THENASSIGNS (arg_info) = NULL;
    }
    if (INFO_ELSEASSIGNS (arg_info) != NULL) {
        BLOCK_ASSIGNS (COND_ELSE (arg_node))
          = TCappendAssign (BLOCK_ASSIGNS (COND_ELSE (arg_node)),
                            INFO_ELSEASSIGNS (arg_info));
        INFO_ELSEASSIGNS (arg_info) = NULL;
    }

    /*
     * first traverse the two blocks
     * as we insert the assigns on our way up
     */
    COND_THEN (arg_node) = TRAVopt(COND_THEN (arg_node), arg_info);

    COND_ELSE (arg_node) = TRAVopt(COND_ELSE (arg_node), arg_info);

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
                    INFO_FUNDEF (arg_info), &INFO_PREASSIGNS (arg_info));

            /*
             * Exchange predicate identifier in all subsequent funcond nodes
             */
            while ((NODE_TYPE (ASSIGN_STMT (funcond_ass)) == N_let)
                   && (NODE_TYPE (ASSIGN_RHS (funcond_ass)) == N_funcond)) {
                ID_AVIS (FUNCOND_IF (ASSIGN_RHS (funcond_ass)))
                  = ID_AVIS (COND_COND (arg_node));

                funcond_ass = ASSIGN_NEXT (funcond_ass);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *ASDfuncond( node *arg_node, info *arg_info)
 *
 * Description:
 *   Inserts conversion assignments for the then/else arguments of the
 *   funcond if necessary.
 *
 ******************************************************************************/
node *
ASDfuncond (node *arg_node, info *arg_info)
{
    shape_class_t result_cls, then_cls, else_cls;

    DBUG_ENTER ();

    DBUG_PRINT ("analyzing funcond...");

    result_cls = NTUgetShapeClassFromNType (IDS_NTYPE (INFO_LHS (arg_info)));

    if (NODE_TYPE (FUNCOND_THEN (arg_node)) == N_id) {
        then_cls = NTUgetShapeClassFromNType (ID_NTYPE (FUNCOND_THEN (arg_node)));

        if ((result_cls != then_cls) && ((result_cls == C_scl) || (then_cls == C_scl))) {
            DBUG_PRINT ("Then branch and result of funcond have "
                        "different ishape classes: %s instead of %s",
                        global.nt_shape_string[then_cls],
                        global.nt_shape_string[result_cls]);
            LiftId (FUNCOND_THEN (arg_node), IDS_NTYPE (INFO_LHS (arg_info)),
                    INFO_FUNDEF (arg_info), &INFO_THENASSIGNS (arg_info));
        }
    }

    if (NODE_TYPE (FUNCOND_ELSE (arg_node)) == N_id) {
        else_cls = NTUgetShapeClassFromNType (ID_NTYPE (FUNCOND_ELSE (arg_node)));

        if ((result_cls != else_cls) && ((result_cls == C_scl) || (else_cls == C_scl))) {
            DBUG_PRINT ("Else branch and result of funcond have "
                        "different ishape classes: %s instead of %s",
                        global.nt_shape_string[else_cls],
                        global.nt_shape_string[result_cls]);
            LiftId (FUNCOND_ELSE (arg_node), IDS_NTYPE (INFO_LHS (arg_info)),
                    INFO_FUNDEF (arg_info), &INFO_ELSEASSIGNS (arg_info));
        }
    }

    DBUG_PRINT ("...funcond done");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASDlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ASDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASDwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ASDwith (node *arg_node, info *arg_info)
{
    ntype *oldcextypes;
    node *oldwithop;

    DBUG_ENTER ();

    oldcextypes = INFO_CEXTYPES (arg_info);
    oldwithop = INFO_WITHOP (arg_info);

    INFO_CEXTYPES (arg_info) = NULL;
    INFO_WITHOP (arg_info) = WITH_WITHOP (arg_node);

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_CEXTYPES (arg_info) = TYfreeType (INFO_CEXTYPES (arg_info));
    INFO_CEXTYPES (arg_info) = oldcextypes;
    INFO_WITHOP (arg_info) = oldwithop;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASDwith2( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ASDwith2 (node *arg_node, info *arg_info)
{
    ntype *oldcextypes;
    node *oldwithop;

    DBUG_ENTER ();

    oldcextypes = INFO_CEXTYPES (arg_info);
    oldwithop = INFO_WITHOP (arg_info);

    INFO_CEXTYPES (arg_info) = NULL;
    INFO_WITHOP (arg_info) = WITH2_WITHOP (arg_node);

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    INFO_CEXTYPES (arg_info) = TYfreeType (INFO_CEXTYPES (arg_info));
    INFO_CEXTYPES (arg_info) = oldcextypes;
    INFO_WITHOP (arg_info) = oldwithop;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASDcode( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ASDcode (node *arg_node, info *arg_info)
{
    size_t i;
    node *cexprs, *withop;

    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVopt(CODE_CBLOCK (arg_node), arg_info);

    INFO_CEXTYPES (arg_info)
      = GetLeastTypes (INFO_CEXTYPES (arg_info),
                       NTCnewTypeCheck_Expr (CODE_CEXPRS (arg_node)));

    CODE_NEXT (arg_node) = TRAVopt(CODE_NEXT (arg_node), arg_info);

    i = 0;
    cexprs = CODE_CEXPRS (arg_node);
    withop = INFO_WITHOP (arg_info);

    while (cexprs != NULL) {
        if ((NODE_TYPE (withop) == N_genarray) || (NODE_TYPE (withop) == N_modarray)) {
            ntype *restype, *cextype;

            restype = TYgetProductMember (INFO_CEXTYPES (arg_info), i);
            cextype = ID_NTYPE (EXPRS_EXPR (cexprs));

            if (TYcmpTypes (restype, cextype) == TY_lt) {
                node *avis, *ass;
                node *cexavis = ID_AVIS (EXPRS_EXPR (cexprs));

                avis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (cexavis)),
                                   TYcopyType (restype));

                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

                ass = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                               TCmakePrf2 (F_type_conv,
                                                           TBmakeType (
                                                             TYcopyType (restype)),
                                                           TBmakeId (cexavis))),
                                    NULL);
                AVIS_SSAASSIGN (avis) = ass;

                EXPRS_EXPR (cexprs) = FREEdoFreeNode (EXPRS_EXPR (cexprs));
                EXPRS_EXPR (cexprs) = TBmakeId (avis);

                BLOCK_ASSIGNS (CODE_CBLOCK (arg_node))
                  = TCappendAssign (BLOCK_ASSIGNS (CODE_CBLOCK (arg_node)), ass);
            }
        }

        i++;
        withop = WITHOP_NEXT (withop);
        cexprs = EXPRS_NEXT (cexprs);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASDprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ASDprf (node *arg_node, info *arg_info)
{
    node *args, *arg;
    ntype *tg, *nt;
    shape_class_t actual_cls;
    shape_class_t tg_cls;
    unsigned int arg_cnt;

    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_type_conv || PRF_PRF (arg_node) == F_type_fix) {

        arg = PRF_ARG2 (arg_node);
        tg = TYPE_TYPE (PRF_ARG1 (arg_node));
        actual_cls = NTUgetShapeClassFromNType (ID_NTYPE (arg));
        tg_cls = NTUgetShapeClassFromNType (tg);

        if ((actual_cls != tg_cls) && ((actual_cls == C_scl) || (tg_cls == C_scl))) {
            DBUG_PRINT ("Shape class conversion disguised as typeconv found:");
            PRF_ARG2 (arg_node) = NULL;
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = TCmakePrf1 (F_copy, arg);
        }
    } else {

        arg_cnt = 0;
        args = PRF_ARGS (arg_node);
        while (args != NULL) {
            arg = EXPRS_EXPR (args);
            if ((PRF_ARGENCODING (PRF_PRF (arg_node), arg_cnt) == PA_S)
                && (NODE_TYPE (arg) == N_id)) {
                actual_cls = NTUgetShapeClassFromNType (ID_NTYPE (arg));
                if (actual_cls != C_scl) {

                    DBUG_PRINT (" prf applied to non-scalar in scalar arg-pos found: ");
                    DBUG_PRINT ("   ... = %s( ... %s ...), %s instead of %s",
                                global.prf_name[PRF_PRF (arg_node)], ID_NAME (arg),
                                global.nt_shape_string[actual_cls],
                                global.nt_shape_string[C_scl]);

                    nt = TYmakeAKS (TYcopyType (TYgetScalar (ID_NTYPE (arg))),
                                    SHmakeShape (0));
                    LiftId (arg, nt, INFO_FUNDEF (arg_info), &INFO_PREASSIGNS (arg_info));
                    nt = TYfreeType (nt);
                }
            }
            args = EXPRS_NEXT (args);
            arg_cnt++;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- AUD SCL Distinction -->
 *****************************************************************************/

#undef DBUG_PREFIX
