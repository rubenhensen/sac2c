/*
 *  $Id$
 */

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
#include "dbug.h"
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

    node *lhs;
    node *withop;
    ntype *cextypes;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_ASSIGN(n) ((n)->assign)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_POSTASSIGNS(n) ((n)->postassigns)

#define INFO_LHS(n) ((n)->lhs)
#define INFO_WITHOP(n) ((n)->withop)
#define INFO_CEXTYPES(n) ((n)->cextypes)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_WITHOP (result) = NULL;
    INFO_CEXTYPES (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

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

    DBUG_ENTER ("ASDdoAudSclDistinction");

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

    DBUG_ENTER ("GetLeastTypes");

    if ((p1 == NULL) || (p2 == NULL)) {
        res = p1 == NULL ? p2 : p1;
    } else {
        int i;

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
 *  @fn node *LiftId(  node *id, ntype *new_type, info *arg_info)
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
LiftId (node *id, ntype *new_type, info *arg_info)
{
    char *new_name;
    node *new_ids;
    node *new_avis;
    node *fundef;

    DBUG_ENTER ("LiftId");

    new_name = TRAVtmpVarName (ID_NAME (id));

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

/** <!--********************************************************************-->
 *
 * @fn void LiftIds( node *ids_arg, ntype *new_type, info *arg_info)
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

    new_name = TRAVtmpVarName (IDS_NAME (ids_arg));

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

/** <!--********************************************************************-->
 *
 * @fn node *ASDlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ASDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASDlet");

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

    DBUG_ENTER ("ASDwith");

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

    DBUG_ENTER ("ASDwith2");

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
    int i;
    node *cexprs, *withop;

    DBUG_ENTER ("ASDcode");

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    INFO_CEXTYPES (arg_info)
      = GetLeastTypes (INFO_CEXTYPES (arg_info),
                       NTCnewTypeCheck_Expr (CODE_CEXPRS (arg_node)));

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

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

                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

                ass = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                               TCmakePrf2 (F_type_conv,
                                                           TBmakeType (
                                                             TYcopyType (restype)),
                                                           TBmakeId (cexavis))),
                                    NULL);
                AVIS_SSAASSIGN (avis) = ass;

                EXPRS_EXPR (cexprs) = FREEdoFreeNode (EXPRS_EXPR (cexprs));
                EXPRS_EXPR (cexprs) = TBmakeId (avis);

                BLOCK_INSTR (CODE_CBLOCK (arg_node))
                  = TCappendAssign (BLOCK_INSTR (CODE_CBLOCK (arg_node)), ass);
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
    DBUG_ENTER ("ASDprf");

    switch (PRF_PRF (arg_node)) {
    case F_abs:
    case F_neg:
    case F_not_S:
    case F_not_V:
        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id) {
            node *id = PRF_ARG1 (arg_node);
            shape_class_t actual_cls = NTUgetShapeClassFromNType (ID_NTYPE (id));

            if (actual_cls != C_scl) {
                /*
                 * cg 11.6.07
                 *
                 * In my opinion this code should never be executed if we properly
                 * distinguish between scalar and vector/array prfs.
                 */
                ntype *nt;

                DBUG_PRINT ("ASD", ("Unary scalar prf applied to non-scalar found: "));
                DBUG_PRINT ("ASD", ("   ... = %s( ... %s ...), %s instead of %s",
                                    global.prf_name[PRF_PRF (arg_node)], ID_NAME (id),
                                    global.nt_shape_string[actual_cls],
                                    global.nt_shape_string[C_scl]));

                nt
                  = TYmakeAKS (TYcopyType (TYgetScalar (ID_NTYPE (id))), SHmakeShape (0));
                LiftId (id, nt, arg_info);
                nt = TYfreeType (nt);
            }
        }
        break;

    case F_type_conv: {
        node *id = PRF_ARG2 (arg_node);
        ntype *tg = TYPE_TYPE (PRF_ARG1 (arg_node));
        shape_class_t id_cls = NTUgetShapeClassFromNType (ID_NTYPE (id));
        shape_class_t tg_cls = NTUgetShapeClassFromNType (tg);

        if ((id_cls != tg_cls) && ((id_cls == C_scl) || (tg_cls == C_scl))) {
            DBUG_PRINT ("ASD", ("Shape class conversion disguised as typeconv found:"));
            PRF_ARG2 (arg_node) = NULL;
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = TCmakePrf1 (F_copy, id);
        }
    } break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- AUD SCL Distinction -->
 *****************************************************************************/
