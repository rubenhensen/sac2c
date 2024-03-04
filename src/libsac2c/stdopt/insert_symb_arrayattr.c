/** <!--********************************************************************-->
 *
 * @defgroup isaa Insert Symbolic Array Attributes
 *
 * This traversal augments all AVIS nodes with expressions for dim and shape.
 * In contrast to the new_types, these expressions do not need to be constant.
 *
 * ISAA may be divided up into 3 different levels of annotation:
 *
 * 1. classical / minimal style.
 *    we only generate dim/shape information inside functions; if an array is
 *    is passed to a build-in function we may determine its new dim/shape, but
 *    as soon as a user-defined function appears, we may do a saabind( dim(a),
 *    shape(a), a). This also holds for conditional and loop functions.
 * 2. cond/loop-augmented style
 *    this mode extends classic style by passing the dimension and shape of
 *    arrays as parameters and return values. inside and after the function we
 *    may saabind() to the supplied parameters.
 * 3. annotated style
 *    in order to circumvent certain problems of divergence in the combination
 *    of augmented style and lir, we do remove the saabind at top of loops and
 *    annotate the parameters, which form dim and shape of the array, directly
 *    to the parameter-avis.
 *    Annotated style requires augmented style.
 *
 * When using these annotation-levels, the following assertions shall hold:
 * 1. In classical style, AVIS_DIM and AVIS_SHAPE is set, if FIXME!
 *
 * 2. In augmented style, AVIS_DIM and AVIS_SHAPE is set on every avis, that
 *    is the result of a saabind or an expression, which result types can be
 *    determined. It is not set on user function results, function parameters
 *    and not saa-bound avis.
 * 3. In annotated style, AVIS_DIM and AVIS_SHAPE is set on every avis. Only
 *    exceptions are avis, that are the result of a user defined function and
 *    are not jet saa-bound.
 *
 * This traversal may in certain cases not export values from loops or
 * conditionals; concrete cases are, if the dim or shape of returned avis
 * are not known (e.g. dispatch_error(...)).
 *
 * the style to be used may be choosen below by setting the appropriate
 * defines. by default the classical style is choosen. this ensures maximal
 * compability.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/* FIXME, I AM A DIRTY HACK! */
#define ISAA_USE_AUGMENTED_STYLE 1
#define ISAA_USE_EVEN_ANNOTATED_STYLE 1

/** <!--********************************************************************-->
 *
 * @file insert_symb_arrayattr.c
 *
 * Prefix: ISAA
 *
 *****************************************************************************/
#include "insert_symb_arrayattr.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "ISAA"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "constants.h"
#include "makedimexpr.h"
#include "makeshapeexpr.h"
#include "print.h"
#include "check.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

enum travscope_t { TS_module, TS_fundef, TS_args };

enum travmode_t { TM_all, TM_then, TM_else };

struct INFO {
    enum travscope_t travscope;
    enum travmode_t travmode;
    node *preassign;
    node *postassign;
    node *fundef;
    node *preblock;
    node *lhs;
    node *rhs;
    node *withid;
    node *args;
    node *returnexpr;
    bool recap;
    bool funparams;
};

#define INFO_TRAVSCOPE(n) ((n)->travscope)
#define INFO_TRAVMODE(n) ((n)->travmode)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_POSTASSIGN(n) ((n)->postassign)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREBLOCK(n) ((n)->preblock)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_RHS(n) ((n)->rhs)
#define INFO_WITHID(n) ((n)->withid)
#define INFO_ARGS(n) ((n)->args)
#define INFO_RECAP(n) ((n)->recap)
#define INFO_FUNPARAMS(n) ((n)->funparams)
#define INFO_RETURNEXPR(n) ((n)->returnexpr)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_TRAVSCOPE (result) = TS_module;
    INFO_TRAVMODE (result) = TM_all;
    INFO_PREASSIGN (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_PREBLOCK (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_RHS (result) = NULL;
    INFO_WITHID (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_RECAP (result) = FALSE;
    INFO_FUNPARAMS (result) = FALSE;
    INFO_RETURNEXPR (result) = NULL;

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
 * @fn node *ISAAdoInsertShapeVariables( node *arg_node)
 *
 *****************************************************************************/
node *
ISAAdoInsertShapeVariables (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    INFO_TRAVSCOPE (arg_info)
      = (N_module == NODE_TYPE (arg_node)) ? TS_module : TS_fundef;
    TRAVpush (TR_isaa);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
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

/*
 * some little helper functions, useful for creating new avis noes of several
 * shapes and sizes.
 */

static node *
CreateScalarAvisFrom (node *source, node *fun)
{
    node *newavis = NULL;

    newavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (source)),
                          TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    AVIS_DIM (newavis) = TBmakeNum (0);
    AVIS_SHAPE (newavis) = TCmakeIntVector (NULL);

    if (NULL != fun) {
        FUNDEF_VARDECS (fun) = TBmakeVardec (newavis, FUNDEF_VARDECS (fun));
    }

    return (newavis);
}

static node *
CreateVectorAvisFrom (node *source, node *shape, node *fun)
{
    ntype *newtype = NULL;
    node *newavis = NULL;

    if (FALSE == TUdimKnown (AVIS_TYPE (source))) {
        newtype = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0));
    } else {
        newtype = TYmakeAKS (TYmakeSimpleType (T_int),
                             SHcreateShape (1, TYgetDim (AVIS_TYPE (source))));
    }

    newavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (source)), newtype);

    AVIS_DIM (newavis) = TBmakeNum (1);
    AVIS_SHAPE (newavis) = TCmakeIntVector (TBmakeExprs (shape, NULL));

    if (NULL != fun) {
        FUNDEF_VARDECS (fun) = TBmakeVardec (newavis, FUNDEF_VARDECS (fun));
    }

    return (newavis);
}

/*
 * the two following static functions assist in the creation and insertion
 * of explicit dimension and shape arguments for functions.
 */

static node *
PrependSAAInFormalArgs (node *arg_node, info *arg_info)
{
    node *avis;

    node *newshp;
    node *newdim;

    node *newarg = NULL;

    DBUG_ENTER ();

    if (NULL != ARG_NEXT (arg_node)) {
        ARG_NEXT (arg_node) = PrependSAAInFormalArgs (ARG_NEXT (arg_node), arg_info);
    }

    avis = ARG_AVIS (arg_node);

#if ISAA_USE_EVEN_ANNOTATED_STYLE
    if ((NULL == AVIS_SHAPE (avis)) || (NULL == AVIS_DIM (avis))) {
#else
    if (!AVIS_HASSAAARGUMENTS (avis)) {
#endif
        if (!TUdimKnown (AVIS_TYPE (avis))) {
            DBUG_PRINT ("inserting a formal dim for %s", AVIS_NAME (avis));

            newdim = CreateScalarAvisFrom (avis, NULL);
            AVIS_HASSAAARGUMENTS (newdim) = TRUE;

            /* within this phase we append N_id to DIM/SHAPE. but they are cleared
             * later on (if in augmented style; annotated style will keep them). */
            AVIS_DIM (avis) = TBmakeId (newdim);
            newarg = TBmakeArg (newdim, NULL);
        } else {
            AVIS_DIM (avis) = TBmakeNum (TYgetDim (AVIS_TYPE (avis)));
        }

        if (!TUshapeKnown (AVIS_TYPE (avis))) {
            DBUG_PRINT ("inserting a formal shape for %s", AVIS_NAME (avis));

            newshp = CreateVectorAvisFrom (avis, DUPdoDupNode (AVIS_DIM (avis)), NULL);
            AVIS_HASSAAARGUMENTS (newshp) = TRUE;

            AVIS_SHAPE (avis) = TBmakeId (newshp);
            arg_node = TCappendArgs (newarg, TBmakeArg (newshp, arg_node));
        } else {
            AVIS_SHAPE (avis) = SHshape2Array (TYgetShape (AVIS_TYPE (avis)));
        }

        AVIS_HASSAAARGUMENTS (avis) = TRUE;
    }

#if ISAA_USE_EVEN_ANNOTATED_STYLE
    AVIS_HASDTTHENPROXY (avis) = TRUE;
    AVIS_HASDTELSEPROXY (avis) = TRUE;
#endif

    DBUG_RETURN (arg_node);
}

static node *
PrependSAAInConcreteArgs (node *arg_node, node *funargs, info *arg_info)
{
    node *avis;
    node *funavis;

    node *preass;
    node *newarg = NULL;

    node *newshp;
    node *newdim;

    DBUG_ENTER ();

    if ((NULL != EXPRS_NEXT (arg_node)) && (NULL != ARG_NEXT (funargs))) {
        EXPRS_NEXT (arg_node) = PrependSAAInConcreteArgs (EXPRS_NEXT (arg_node),
                                                          ARG_NEXT (funargs), arg_info);
    }

    /* there may be an saabind to our avis already */
    avis = ID_AVIS (EXPRS_EXPR (arg_node));
    while (NULL != AVIS_SUBST (avis)) {
        avis = AVIS_SUBST (avis);
    }

    funavis = ARG_AVIS (funargs);

    if (!AVIS_HASSAAARGUMENTS (funavis)) {

        if (!TUdimKnown (AVIS_TYPE (funavis))) {
            DBUG_PRINT ("inserting a concrete dim for %s in fun %s", AVIS_NAME (avis),
                        FUNDEF_NAME (INFO_FUNDEF (arg_info)));

            /* create the new avis that shall be holding our dimension */
            newdim = CreateScalarAvisFrom (avis, INFO_FUNDEF (arg_info));

            /* create a new assignment: newdim = dim( avis ); */
            preass = TBmakeAssign (TBmakeLet (TBmakeIds (newdim, NULL),
                                              TCmakePrf1 (F_dim_A, TBmakeId (avis))),
                                   NULL);
            AVIS_SSAASSIGN (newdim) = preass;
            INFO_PREASSIGN (arg_info)
              = TCappendAssign (INFO_PREASSIGN (arg_info), preass);

            /* prepend the new dim-avis in our argument list */
            newarg = TBmakeExprs (TBmakeId (newdim), NULL);
        }

        if (!TUshapeKnown (AVIS_TYPE (funavis))) {
            DBUG_PRINT ("inserting a concrete shape for %s in fun %s", AVIS_NAME (avis),
                        FUNDEF_NAME (INFO_FUNDEF (arg_info)));

            /* create the new avis that shall be holding our shape */
            DBUG_ASSERT (NULL != AVIS_DIM (avis),
                         "created concrete shape for argument without dim!");

            newshp = CreateVectorAvisFrom (funavis, DUPdoDupNode (AVIS_DIM (avis)),
                                           INFO_FUNDEF (arg_info));

            /* create a new assignment: newshp = shape( avis ); */
            preass = TBmakeAssign (TBmakeLet (TBmakeIds (newshp, NULL),
                                              TCmakePrf1 (F_shape_A, TBmakeId (avis))),
                                   NULL);
            AVIS_SSAASSIGN (newshp) = preass;
            INFO_PREASSIGN (arg_info)
              = TCappendAssign (INFO_PREASSIGN (arg_info), preass);

            /* put the dim-shape in our argument list, behind dim, before avis */
            arg_node = TCappendExprs (newarg, TBmakeExprs (TBmakeId (newshp), arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 * the following two functions do insert shape and dim-arguments explicitly as
 * new return values into lac functions.
 */

static node *
PrependSAAInConcreteResults (node *formalresults, node *concreteresults, node *fundef,
                             info *arg_info)
{
    node *newdim;
    node *newshp;

    node *avis = IDS_AVIS (concreteresults);
    node *newresults = NULL;

    ntype *ctype = AVIS_TYPE (IDS_AVIS (concreteresults));
    node *nextfr = RET_NEXT (formalresults);
    node *nextcr = IDS_NEXT (concreteresults);

    DBUG_ENTER ();

    /* we need to NULL this, else we would get circular _NEXTs */
    IDS_NEXT (concreteresults) = NULL;

    if (FALSE == AVIS_HASSAAARGUMENTS (avis)) {
        AVIS_HASSAAARGUMENTS (avis) = TRUE;

        /* we do check if there is a new result to be appended by comparing
         * the result types of our function definition with the types of
         * our results. If there is a discrepancy, than the formal type must be
         * more specific than the concrete type and thereby is the dim or
         * shape of the concrete result. */

        if (TY_eq != TYcmpTypes (ctype, RET_TYPE (formalresults))) {

            DBUG_ASSERT (((TYisAKD (ctype)) || (TYisAUD (ctype)) || (TYisAUDGZ (ctype))),
                         "arrived at unexpected type difference in fun application!");

            if ((TYisAUD (ctype)) || (TYisAUDGZ (ctype))) {
                DBUG_PRINT ("inserting a concrete result dim for %s", AVIS_NAME (avis));

                /* create the new avis for our dimension */
                newdim = CreateScalarAvisFrom (avis, INFO_FUNDEF (arg_info));
                AVIS_SSAASSIGN (newdim) = AVIS_SSAASSIGN (IDS_AVIS (concreteresults));
                AVIS_HASSAAARGUMENTS (newdim) = TRUE;

                /* insert the avis into our result chain and annotate at its array */
                newresults = TBmakeIds (newdim, NULL);
                newdim = TBmakeId (newdim);
                AVIS_DIM (avis) = newdim;

                /* our formal result types already contain dim, so we need to
                 * jump over these when traversing into the next element. */
                nextfr = RET_NEXT (nextfr);
            } else {
                newdim = TBmakeNum (TYgetDim (AVIS_TYPE (IDS_AVIS (concreteresults))));
                AVIS_DIM (avis) = newdim;
            }

            DBUG_PRINT ("inserting a concrete result shape for %s", AVIS_NAME (avis));

            /* create the new avis for our shape */
            newshp = CreateVectorAvisFrom (IDS_AVIS (concreteresults),
                                           DUPdoDupNode (newdim), INFO_FUNDEF (arg_info));
            AVIS_SSAASSIGN (newshp) = AVIS_SSAASSIGN (IDS_AVIS (concreteresults));
            AVIS_HASSAAARGUMENTS (newshp) = TRUE;

            /* insert the avis into our result chain and annotate at its array */
            concreteresults
              = TCappendIds (newresults, TBmakeIds (newshp, concreteresults));
            AVIS_SHAPE (avis) = TBmakeId (newshp);

            nextfr = RET_NEXT (nextfr);
        }
    }

    if ((NULL != nextfr) && (NULL != nextcr)) {
        concreteresults
          = TCappendIds (concreteresults,
                         PrependSAAInConcreteResults (nextfr, nextcr, fundef, arg_info));
    }

    DBUG_RETURN (concreteresults);
}

static node *
InsertTempCondVarFor (node *avis_ds, node *dim, node *avis, node *fundef, int thenelse)
{
    node *tmpds;
    node *tmpassign;
    node *retnode;
    node *ainstr;

    DBUG_ENTER ();

    /* this function is just for the (very seldom!)
     * case, that the shape or dimension of the returned value of our
     * lacfun is not an N_id, but merely a N_array. This can happen, if
     * the shape/dim of the returned value is _not_ known in the other case
     * but in this it is.
     */

    if (N_id != NODE_TYPE (avis_ds)) {

        if (NULL == dim) {
            tmpds = CreateScalarAvisFrom (avis, fundef);
        } else {
            tmpds = CreateVectorAvisFrom (avis, DUPdoDupNode (dim), fundef);
        }

        tmpassign
          = TBmakeAssign (TBmakeLet (TBmakeIds (tmpds, NULL), DUPdoDupNode (avis_ds)),
                          NULL);
        AVIS_SSAASSIGN (tmpds) = tmpassign;

        if (TRUE == thenelse) {
            ainstr = COND_THEN (ASSIGN_STMT (BLOCK_ASSIGNS (FUNDEF_BODY (fundef))));
        } else {
            ainstr = COND_ELSE (ASSIGN_STMT (BLOCK_ASSIGNS (FUNDEF_BODY (fundef))));
        }

        BLOCK_ASSIGNS (ainstr) = TCappendAssign (BLOCK_ASSIGNS (ainstr), tmpassign);
        retnode = TBmakeId (tmpds);
    } else {
        retnode = DUPdoDupNode (avis_ds);
    }

    DBUG_RETURN (retnode);
}

static node *
PrependSAAInFormalResults (node *returntype, node *returnexpr, node *fundef,
                           info *arg_info)
{
    node *avis;

    node *p;
    node *fc;
    node *sc;

    node *newdim = NULL;
    node *newshp = NULL;

    node *thennode;
    node *elsenode;

    node *newretexpr = NULL;
    node *newrettype = NULL;
    node *newassign = NULL;

    DBUG_ENTER ();

    /* this function returns three results:
     * 1. The N_ret to be put into the function definition
     *    as the returned node of the function itself
     * 2. The N_exprs to be put in the return( ... )
     *    via info_returnexpr
     * 3. The N_assign to be but right in front of the return-statement
     *    via info_postassign; they are taken care of in ISAAassign.
     */

    if ((NULL != RET_NEXT (returntype)) && (NULL != EXPRS_NEXT (returnexpr))) {
        RET_NEXT (returntype)
          = PrependSAAInFormalResults (RET_NEXT (returntype), EXPRS_NEXT (returnexpr),
                                       fundef, arg_info);
        EXPRS_NEXT (returnexpr) = INFO_RETURNEXPR (arg_info);
    }

    avis = ID_AVIS (EXPRS_EXPR (returnexpr));

    /* we do check here, if the conditional already has been traversed at its
     * then as well as at its else branch, to ensure that all our avis-dim/shape
     * are set properly. */
    if ((FALSE != AVIS_HASDTTHENPROXY (ARG_AVIS (FUNDEF_ARGS (fundef))))
        && (FALSE != AVIS_HASDTELSEPROXY (ARG_AVIS (FUNDEF_ARGS (fundef))))
        && (FALSE == AVIS_HASSAAARGUMENTS (avis))) {
        AVIS_HASSAAARGUMENTS (avis) = TRUE;

        /*
         * In order to return the dim and shape of results of a conditional, we
         * do need to create new funconds, which choose the right dim/shape.
         * these will look like this:
         * newdim = ( predicate ? dim( firstchoice ) : dim ( secondchoice ) );
         *
         * For this, we need to look up the predicate and the two choices for our
         * returning value. these are p, fc and sc. */
        p = FUNCOND_IF (LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (avis))));
        fc = ID_AVIS (FUNCOND_THEN (LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (avis)))));
        sc = ID_AVIS (FUNCOND_ELSE (LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (avis)))));

        thennode = AVIS_DIM (fc);
        elsenode = AVIS_DIM (sc);

        if ((FALSE == TUdimKnown (AVIS_TYPE (avis))) && (NULL != thennode)
            && (NULL != elsenode)) {
            DBUG_PRINT ("inserting a formal result type as dimension");

            /* create the new avis that holds our returned dimension */
            newdim = CreateScalarAvisFrom (avis, fundef);
            AVIS_HASSAAARGUMENTS (newdim) = TRUE;

            /* See InsertTempCondVarFor for a detailed description */
            thennode = InsertTempCondVarFor (AVIS_DIM (fc), NULL, avis, fundef, TRUE);
            elsenode = InsertTempCondVarFor (AVIS_DIM (sc), NULL, avis, fundef, FALSE);

            /* create the funcond for selecting the right dimension */
            newassign = TBmakeAssign (TBmakeLet (TBmakeIds (newdim, NULL),
                                                 TBmakeFuncond (DUPdoDupNode (p),
                                                                thennode, elsenode)),
                                      NULL);
            AVIS_SSAASSIGN (newdim) = newassign;

            newdim = TBmakeId (newdim);

            /* put our dimension into the chains of return( ) and fundef_ret */
            newretexpr = TBmakeExprs (DUPdoDupNode (newdim), NULL);
            newrettype = TBmakeRet (TYcopyType (AVIS_TYPE (ID_AVIS (newdim))), NULL);
        } else if ((FALSE == TUshapeKnown (AVIS_TYPE (avis)))
                   && (TRUE == TUdimKnown (AVIS_TYPE (avis)))) {

            newdim = TBmakeNum (TYgetDim (AVIS_TYPE (avis)));
        }

        if (NULL != newdim) {
            AVIS_DIM (avis) = newdim;
        }

        thennode = AVIS_SHAPE (fc);
        elsenode = AVIS_SHAPE (sc);

        if ((FALSE == TUshapeKnown (AVIS_TYPE (avis))) && (NULL != thennode)
            && (NULL != elsenode) && (NULL != newdim)) {
            DBUG_PRINT ("inserting a formal result type as shape");

            /* create the new avis that holds our returned shape */
            newshp = CreateVectorAvisFrom (avis, DUPdoDupNode (newdim), fundef);
            AVIS_HASSAAARGUMENTS (newshp) = TRUE;

            /* See InsertTempCondVarFor for a detailed description */
            thennode = InsertTempCondVarFor (AVIS_SHAPE (fc), newdim, avis, fundef, TRUE);
            elsenode
              = InsertTempCondVarFor (AVIS_SHAPE (sc), newdim, avis, fundef, FALSE);

            /* create the funcond for selecting the right shape */
            INFO_POSTASSIGN (arg_info)
              = TCappendAssign (newassign,
                                TBmakeAssign (TBmakeLet (TBmakeIds (newshp, NULL),
                                                         TBmakeFuncond (DUPdoDupNode (p),
                                                                        thennode,
                                                                        elsenode)),
                                              INFO_POSTASSIGN (arg_info)));
            if (NULL == newassign) {
                AVIS_SSAASSIGN (newshp) = INFO_POSTASSIGN (arg_info);
            } else {
                AVIS_SSAASSIGN (newshp) = ASSIGN_NEXT (INFO_POSTASSIGN (arg_info));
            }

            /* put our shape into the chains of return( ) and fundef_ret */
            returnexpr
              = TCappendExprs (newretexpr, TBmakeExprs (TBmakeId (newshp), returnexpr));
            returntype
              = TCappendRet (newrettype,
                             TBmakeRet (TYcopyType (AVIS_TYPE (newshp)), returntype));

            newshp = TBmakeId (newshp);
        } else if ((TRUE == TUshapeKnown (AVIS_TYPE (avis))) && (NULL != newdim)) {
            newshp = SHshape2Array (TYgetShape (AVIS_TYPE (avis)));
        }

        if (NULL != newshp) {
            AVIS_SHAPE (avis) = newshp;
        }
    }

    INFO_RETURNEXPR (arg_info) = returnexpr;

    DBUG_RETURN (returntype);
}

static node *
GenerateExtendedReturns (node *funret)
{
    ntype *newtype;
    node *newret = NULL;

    DBUG_ENTER ();

    /*
     * This function basically just generates a 'preview' of what a saa'ed version
     * of the supplied N_ret-Chain will look like. It is needed for a application
     * of PrependSAAInConcreteResults to a loop-function prior to applying
     * PrependSAAInFormalResults.
     */

    if (NULL != RET_NEXT (funret)) {
        RET_NEXT (funret) = GenerateExtendedReturns (RET_NEXT (funret));
    }

    if (FALSE == TUshapeKnown (RET_TYPE (funret))) {
        if (FALSE == TUdimKnown (RET_TYPE (funret))) {

            newtype = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0));

            /* Here we also do not know the dimension, so we generate a new ret
             * for it right away. */
            newret
              = TBmakeRet (TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)), newret);
        } else {
            newtype = TYmakeAKS (TYmakeSimpleType (T_int),
                                 SHcreateShape (1, TYgetDim (RET_TYPE (funret))));
        }

        newret = TCappendRet (newret, TBmakeRet (newtype, NULL));
    }

    funret = TCappendRet (newret, funret);

    DBUG_RETURN (funret);
}

#if (ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE)
static node *
ISAAretraverse (node *fun, bool save_args, node *newargs, info *arg_info)
{
    enum travscope_t travscope;
    enum travmode_t travmode;
    node *preblock;
    node *preassign;
    node *postassign;
    node *fundef;
    node *args = NULL;

    DBUG_ENTER ();

    preblock = INFO_PREBLOCK (arg_info);
    preassign = INFO_PREASSIGN (arg_info);
    postassign = INFO_POSTASSIGN (arg_info);
    travscope = INFO_TRAVSCOPE (arg_info);
    travmode = INFO_TRAVMODE (arg_info);
    fundef = INFO_FUNDEF (arg_info);

    INFO_PREBLOCK (arg_info) = NULL;
    INFO_PREASSIGN (arg_info) = NULL;
    INFO_POSTASSIGN (arg_info) = NULL;
    INFO_TRAVSCOPE (arg_info) = TS_args;

    if (TRUE == save_args) {
        args = INFO_ARGS (arg_info);
        INFO_ARGS (arg_info) = newargs;
    }

    DBUG_PRINT ("retraverse %s", FUNDEF_NAME (fun));
    fun = TRAVdo (fun, arg_info);

    if (TRUE == save_args) {
        INFO_ARGS (arg_info) = args;
    }

    INFO_PREBLOCK (arg_info) = preblock;
    INFO_PREASSIGN (arg_info) = preassign;
    INFO_POSTASSIGN (arg_info) = postassign;
    INFO_TRAVSCOPE (arg_info) = travscope;
    INFO_TRAVMODE (arg_info) = travmode;
    INFO_FUNDEF (arg_info) = fundef;

    DBUG_RETURN (fun);
}
#endif

/*
 * the following static functions assist in the creation of proxies.
 */

static node *
MakeDTProxy (node *avis, node *postass, info *arg_info)
{
    bool makeproxy = FALSE;
    bool islacfun = FALSE;
    node *newass = NULL;

    DBUG_ENTER ();
    /*
    DBUG_PRINT( "ISAA", ("enter MakeDTProxy in %s for %s",
                         FUNDEF_NAME( INFO_FUNDEF( arg_info ) ),
                         AVIS_NAME( avis ) ) );
    */
#if ISAA_USE_EVEN_ANNOTATED_STYLE
    if ((TRUE == FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info)))
        && (TRUE == INFO_FUNPARAMS (arg_info))) {
        islacfun = TRUE;
    }

    if (((NULL == AVIS_SHAPE (avis)) || (NULL == AVIS_DIM (avis)))
        && (TRUE == islacfun)) {
        makeproxy = TRUE;
    } else {
#endif
        switch (INFO_TRAVMODE (arg_info)) {
        case TM_then:
            makeproxy = (!AVIS_HASDTTHENPROXY (avis));
            break;

        case TM_else:
            makeproxy = (!AVIS_HASDTELSEPROXY (avis));
            break;

        case TM_all:
            makeproxy = ((!AVIS_HASDTTHENPROXY (avis)) || (!AVIS_HASDTELSEPROXY (avis)));
            break;
        }
#if ISAA_USE_EVEN_ANNOTATED_STYLE
    }
#endif

    if (makeproxy) {
        node *dimavis = NULL;
        node *shpavis = NULL;
        node *dimnode = NULL;
        node *shpnode = NULL;
        node *proxyavis = NULL;
        node *fundef;
#if ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE
        node *dim_postass = NULL;
        node *shp_postass = NULL;
#endif

        fundef = INFO_FUNDEF (arg_info);

        /* if we do not use annotated_style (see above) this is always false */
        if (FALSE == islacfun) {

            dimavis = CreateScalarAvisFrom (avis, fundef);
            shpavis = CreateVectorAvisFrom (avis, TBmakeId (dimavis), fundef);

            proxyavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)),
                                    TYcopyType (AVIS_TYPE (avis)));
            AVIS_DIM (proxyavis) = TBmakeId (dimavis);
            AVIS_SHAPE (proxyavis) = TBmakeId (shpavis);
            FUNDEF_VARDECS (fundef) = TBmakeVardec (proxyavis, FUNDEF_VARDECS (fundef));

            /* proxyavis = saabind( dimavis, shpavis, avis ); */
            newass = TBmakeAssign (TBmakeLet (TBmakeIds (proxyavis, NULL),
                                              TCmakePrf3 (F_saabind, TBmakeId (dimavis),
                                                          TBmakeId (shpavis),
                                                          TBmakeId (avis))),
                                   newass);
            AVIS_SSAASSIGN (proxyavis) = newass;
            AVIS_SUBST (avis) = proxyavis;
        }

#if ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE
        /* if we have passed the shape as a parameter to our function, we may now
         * look up its avis in AVIS_SHAPE. */

        if (NULL != AVIS_SHAPE (avis)) {
            if (N_id == NODE_TYPE (AVIS_SHAPE (avis))) {

                /* BEAUTIFY_ME: this is a little ugly:
                 * as we want a saabind'ed shape, we have to proxify it first; but as
                 * it would be proxified after this arg, we have to call for it by hand.
                 */
                shp_postass = MakeDTProxy (ID_AVIS (AVIS_SHAPE (avis)), NULL, arg_info);

                if (NULL != AVIS_SUBST (ID_AVIS (AVIS_SHAPE (avis)))) {
                    shpnode = TBmakeId (AVIS_SUBST (ID_AVIS (AVIS_SHAPE (avis))));
                }
            }

            if (NULL == shpnode) {
                shpnode = DUPdoDupNode (AVIS_SHAPE (avis));
            }
        } else {
            shpnode = TCmakePrf1 (F_shape_A, TBmakeId (avis));
        }
#else
        shpnode = TCmakePrf1 (F_shape_A, TBmakeId (avis));
#endif

        if (FALSE == islacfun) {
            newass
              = TBmakeAssign (TBmakeLet (TBmakeIds (shpavis, NULL), shpnode), newass);
            AVIS_SSAASSIGN (shpavis) = newass;
        } else if ((NULL == AVIS_SHAPE (avis)) && (TRUE == islacfun)) {
            if (N_id == NODE_TYPE (shpnode)) {
                AVIS_SHAPE (avis) = shpnode;
            } else {
                /* in this case the shape has to be statically known */
                shpnode = FREEdoFreeNode (shpnode);
                AVIS_SHAPE (avis) = SHshape2Array (TYgetShape (AVIS_TYPE (avis)));
            }
        }

#if ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE
        /* same for the dimension we may have. */
        if (NULL != AVIS_DIM (avis)) {
            if (N_id == NODE_TYPE (AVIS_DIM (avis))) {
                dim_postass = MakeDTProxy (ID_AVIS (AVIS_DIM (avis)), NULL, arg_info);

                if (NULL != AVIS_SUBST (ID_AVIS (AVIS_DIM (avis)))) {
                    dimnode = TBmakeId (AVIS_SUBST (ID_AVIS (AVIS_DIM (avis))));
                }
            }

            if (NULL == dimnode) {
                dimnode = DUPdoDupNode (AVIS_DIM (avis));
            }
        } else {
            dimnode = TCmakePrf1 (F_dim_A, TBmakeId (avis));
        }
#else
        dimnode = TCmakePrf1 (F_dim_A, TBmakeId (avis));
#endif

        if (FALSE == islacfun) {
            newass
              = TBmakeAssign (TBmakeLet (TBmakeIds (dimavis, NULL), dimnode), newass);
            AVIS_SSAASSIGN (dimavis) = newass;
        } else if ((NULL == AVIS_DIM (avis)) && (TRUE == islacfun)) {
            if (N_id == NODE_TYPE (dimnode)) {
                AVIS_DIM (avis) = dimnode;
            } else {
                /* in this case the dimension has to be statically known */
                dimnode = FREEdoFreeNode (dimnode);
                AVIS_DIM (avis) = TBmakeNum (TYgetDim (AVIS_TYPE (avis)));
            }
        }

#if ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE
        if (NULL != shp_postass) {
            newass = TCappendAssign (shp_postass, newass);
        }
        if (NULL != dim_postass) {
            newass = TCappendAssign (dim_postass, newass);
        }
#endif

        switch (INFO_TRAVMODE (arg_info)) {
        case TM_then:
            AVIS_HASDTTHENPROXY (avis) = TRUE;
            break;

        case TM_else:
            AVIS_HASDTELSEPROXY (avis) = TRUE;
            break;

        default:
            AVIS_HASDTTHENPROXY (avis) = TRUE;
            AVIS_HASDTELSEPROXY (avis) = TRUE;
            break;
        }

        if (FALSE == islacfun) {
            switch (INFO_TRAVMODE (arg_info)) {

            case TM_then:
                AVIS_HASDTTHENPROXY (proxyavis) = TRUE;
                break;

            case TM_else:
                AVIS_HASDTELSEPROXY (proxyavis) = TRUE;
                break;

            default:
                AVIS_HASDTTHENPROXY (proxyavis) = TRUE;
                AVIS_HASDTELSEPROXY (proxyavis) = TRUE;
                break;
            }
        }

        /* do NOT remove dim/shape from avis if in annotated style */
#if ISAA_USE_AUGMENTED_STYLE && !ISAA_USE_EVEN_ANNOTATED_STYLE
        if (((AVIS_HASDTTHENPROXY (avis) == TRUE)
             || (AVIS_HASDTELSEPROXY (avis) == TRUE))) {

            /* clean the avis, as we do not want shape/dim on our parameters */
            AVIS_SHAPE (avis) = FREEoptFreeNode(AVIS_SHAPE (avis));

            AVIS_DIM (avis) = FREEoptFreeNode(AVIS_DIM (avis));
        }
#endif
    } /* if ( makeproxy ) */

    postass = TCappendAssign (postass, newass);

    DBUG_RETURN (postass);
}

static node *
MakeArgProxies (node *arg_node, info *arg_info)
{
    node *ass = NULL;

    DBUG_ENTER ();

    if (arg_node != NULL) {
        ass = MakeArgProxies (ARG_NEXT (arg_node), arg_info);
        ass = MakeDTProxy (ARG_AVIS (arg_node), ass, arg_info);
    }

    DBUG_RETURN (ass);
}

static node *
RemoveAvisSubst (node *fundef)
{
    DBUG_ENTER ();

    FUNDEF_ARGS (fundef) = TRAVopt (FUNDEF_ARGS (fundef), NULL);
    FUNDEF_VARDECS (fundef) = TRAVopt (FUNDEF_VARDECS (fundef), NULL);

    DBUG_RETURN (fundef);
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

/** <!--********************************************************************-->
 *
 * @fn node *ISAAfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("entering function %s", FUNDEF_NAME (arg_node));

    if ((NULL != FUNDEF_BODY (arg_node))

#if ISAA_USE_EVEN_ANNOTATED_STYLE
        /* in annotated style, we only want to traverse lac-funs, if they are
         * already proxified. if not, skip them (they are traversed later) */
        && ((TS_args == INFO_TRAVSCOPE (arg_info))
            || (TRUE != FUNDEF_ISLACFUN (arg_node)))
#endif
    ) {
        INFO_FUNDEF (arg_info) = arg_node;

        /*
         * Resolve shape variables to dim, shape, idx_shape_sel
         * In case of cond functions, we need a copy of those variables
         */
        if (FUNDEF_ISCONDFUN (arg_node)) {
            INFO_TRAVMODE (arg_info) = TM_then;
            arg_node = RemoveAvisSubst (arg_node);

            INFO_FUNPARAMS (arg_info) = TRUE;
            INFO_PREBLOCK (arg_info) = MakeArgProxies (FUNDEF_ARGS (arg_node), arg_info);
            INFO_FUNPARAMS (arg_info) = FALSE;

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            INFO_TRAVMODE (arg_info) = TM_else;
            arg_node = RemoveAvisSubst (arg_node);

            INFO_FUNPARAMS (arg_info) = TRUE;
            INFO_PREBLOCK (arg_info) = MakeArgProxies (FUNDEF_ARGS (arg_node), arg_info);
            INFO_FUNPARAMS (arg_info) = FALSE;

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        } else {
            node *preblock;

            INFO_TRAVMODE (arg_info) = TM_all;
            arg_node = RemoveAvisSubst (arg_node);

            INFO_FUNPARAMS (arg_info) = TRUE;
            preblock = MakeArgProxies (FUNDEF_ARGS (arg_node), arg_info);
            INFO_FUNPARAMS (arg_info) = FALSE;

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            FUNDEF_ASSIGNS (arg_node)
              = TCappendAssign (preblock, FUNDEF_ASSIGNS (arg_node));
            INFO_PREBLOCK (arg_info) = NULL;
        }

        /*
         * Clean up the AVIS_SUBST mess
         */
        arg_node = RemoveAvisSubst (arg_node);
    }

    if (INFO_TRAVSCOPE (arg_info) == TS_module) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAap (node *arg_node, info *arg_info)
{
#if ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE
    node *fun;
    node *retnode = NULL;
    node *retprev = NULL;
    node *lhs = NULL;
    node *innerargs = NULL;
#endif

    DBUG_ENTER ();

#if ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE

    fun = AP_FUNDEF (arg_node);

    if ((NULL != FUNDEF_ARGS (fun)) && (fun != INFO_FUNDEF (arg_info))
        && ((TS_args != INFO_TRAVSCOPE (arg_info)) || (TRUE == FUNDEF_ISLACFUN (fun)))
        && (TRUE == FUNDEF_ISLACFUN (fun))) {
        /* At this point we do call an inner lac-function. Several tasks have to be
         * performed now:
         * 1. Create a copy of the functions arguments (innerargs); this is needed
         *    in a loop-function to recreate the inner call.
         * 2. Traverse the concrete arguments to annotate the SAA-information.
         * 3. Traverse the formal arguments to do just the same
         * 4. Retraverse the function, optimising everything within
         * 5. Introduce SAA-arguments for the function results
         */

        /* 1. */
        innerargs = DUPdoDupTree (FUNDEF_ARGS (fun));

        /* 2., 3. */
        AP_ARGS (arg_node)
          = PrependSAAInConcreteArgs (AP_ARGS (arg_node), FUNDEF_ARGS (fun), arg_info);
        FUNDEF_ARGS (fun) = PrependSAAInFormalArgs (FUNDEF_ARGS (fun), arg_info);

        lhs = INFO_LHS (arg_info);

        if (TRUE == FUNDEF_ISCONDFUN (fun)) {
            DBUG_PRINT ("calling the cond fun %s", FUNDEF_NAME (fun));

            /* 4. */
            AP_FUNDEF (arg_node) = ISAAretraverse (fun, FALSE, NULL, arg_info);
        } else if (TRUE == FUNDEF_ISLOOPFUN (fun)) {
            DBUG_PRINT ("calling the loop fun %s", FUNDEF_NAME (fun));

            /* Create a SAA-augmented version of the N_ret-chain, os we may apply
             * PrependSAAInConreteResults before creating real formal augmented
             * results. */
            retprev = GenerateExtendedReturns (DUPdoDupTree (FUNDEF_RETS (fun)));

            LET_IDS (ASSIGN_STMT (AVIS_SSAASSIGN (IDS_AVIS (lhs))))
              = PrependSAAInConcreteResults (retprev, INFO_LHS (arg_info),
                                             INFO_FUNDEF (arg_info), arg_info);
            retprev = FREEdoFreeTree (retprev);

            /* 4. */
            AP_FUNDEF (arg_node) = ISAAretraverse (fun, TRUE, innerargs, arg_info);
        }

        innerargs = FREEdoFreeTree (innerargs);

        /* 5. */
        /* this is rather ugly: we have to search for the N_assign prior to the
         * assign containing the N_return node */
        retnode = BLOCK_ASSIGNS (FUNDEF_BODY (fun));
        while ((NULL != retnode) && (N_return != NODE_TYPE (ASSIGN_STMT (retnode)))) {
            retprev = retnode;
            retnode = ASSIGN_NEXT (retnode);
        }

        DBUG_ASSERT (((NULL != retnode)
                      && (N_return == NODE_TYPE (ASSIGN_STMT (retnode)))),
                     "could not find return node of specified function!");

        FUNDEF_RETS (fun)
          = PrependSAAInFormalResults (FUNDEF_RETS (fun),
                                       RETURN_EXPRS (ASSIGN_STMT (retnode)), fun,
                                       arg_info);

        RETURN_EXPRS (ASSIGN_STMT (retnode)) = INFO_RETURNEXPR (arg_info);
        ASSIGN_NEXT (retprev) = TCappendAssign (INFO_POSTASSIGN (arg_info), retnode);
        INFO_POSTASSIGN (arg_info) = NULL;

        if (TRUE == FUNDEF_ISCONDFUN (fun)) {
            /* replace the lhs of the application with the new one */
            LET_IDS (ASSIGN_STMT (AVIS_SSAASSIGN (IDS_AVIS (lhs))))
              = PrependSAAInConcreteResults (FUNDEF_RETS (fun), lhs,
                                             INFO_FUNDEF (arg_info), arg_info);
        }
    } else if ((TS_args == INFO_TRAVSCOPE (arg_info)) && (TRUE == FUNDEF_ISLOOPFUN (fun))
               && (fun == INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("inner application of the loop fun %s", FUNDEF_NAME (fun));

        /* 1. We now may insert the dim/shape arguments in our recursive loop
         *    call; thisfor we need the copy of our arguments (called innerargs
         *    above, here INFO_ARGS(arg_info)).
         */

        /* no new N_assigns in the holy callback-conditional! */
        INFO_RECAP (arg_info) = TRUE;

        AP_ARGS (arg_node)
          = PrependSAAInConcreteArgs (AP_ARGS (arg_node), INFO_ARGS (arg_info), arg_info);

        /* introduce the new results in the inner calling */
        retprev = GenerateExtendedReturns (DUPdoDupTree (FUNDEF_RETS (fun)));

        LET_IDS (ASSIGN_STMT (AVIS_SSAASSIGN (IDS_AVIS (INFO_LHS (arg_info)))))
          = PrependSAAInConcreteResults (retprev, INFO_LHS (arg_info),
                                         INFO_FUNDEF (arg_info), arg_info);

        retprev = FREEdoFreeTree (retprev);
    }

    /* we may now traverse the arguments, in order to take care of AVIS_SUBST */
    AP_ARGS (arg_node) = TRAVopt(AP_ARGS (arg_node), arg_info);

#else
    TRAVcont (arg_node, arg_info);
#endif

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_SUBST (arg_node) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAassign (node *arg_node, info *arg_info)
{
    node *preassign;
    node *postassign;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /*
     * It is essential that no bindings must be introduced inside the
     * conditional of a LOOP function
     * Thus, we defer this until leaving the leaving the conditional
     */
    if (!INFO_RECAP (arg_info)) {
        preassign = INFO_PREASSIGN (arg_info);
        INFO_PREASSIGN (arg_info) = NULL;

        postassign = INFO_POSTASSIGN (arg_info);
        INFO_POSTASSIGN (arg_info) = NULL;

        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

        if (postassign != NULL) {
            ASSIGN_NEXT (arg_node) = TCappendAssign (postassign, ASSIGN_NEXT (arg_node));
        }

        if (preassign != NULL) {
            arg_node = TCappendAssign (preassign, arg_node);
        }
    } else {
        INFO_RECAP (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        INFO_LHS (arg_info) = LET_IDS (arg_node);
        INFO_RHS (arg_info) = LET_EXPR (arg_node);

        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAids (node *arg_node, info *arg_info)
{
    node *avis;
    node *preass = NULL;

    DBUG_ENTER ();

    avis = IDS_AVIS (arg_node);

    if ((NODE_TYPE (INFO_RHS (arg_info)) != N_ap)
        && (NODE_TYPE (INFO_RHS (arg_info)) != N_str)
        && (!((NODE_TYPE (INFO_RHS (arg_info)) == N_prf)
              && ((PRF_PRF (INFO_RHS (arg_info)) == F_saabind)
                  || (PRF_PRF (INFO_RHS (arg_info)) == F_type_conv)
                  || (PRF_PRF (INFO_RHS (arg_info)) == F_type_fix))))) {
        if (AVIS_DIM (avis) == NULL) {
            if (TUdimKnown (AVIS_TYPE (avis))) {
                AVIS_DIM (avis) = TBmakeNum (TYgetDim (AVIS_TYPE (avis)));
            } else {
                preass
                  = TCappendAssign (preass,
                                    MDEdoMakeDimExpression (INFO_RHS (arg_info), avis,
                                                            INFO_LHS (arg_info),
                                                            INFO_FUNDEF (arg_info)));
            }
        }

        if ((AVIS_DIM (avis) != NULL) && (AVIS_SHAPE (avis) == NULL)) {
            if (TUshapeKnown (AVIS_TYPE (avis))) {
                AVIS_SHAPE (avis) = SHshape2Array (TYgetShape (AVIS_TYPE (avis)));
            } else {
                preass
                  = TCappendAssign (preass,
                                    MSEdoMakeShapeExpression (INFO_RHS (arg_info), avis,
                                                              INFO_LHS (arg_info),
                                                              INFO_FUNDEF (arg_info)));
            }
        }

        /* if we could neither detect dim/shape statically nor were we able to
         * infer expressions, and _if_ the right-hand-side is a fold-withloop,
         * exactly _then_ we need a proxification.
         */

        if ((NULL == AVIS_DIM (avis)) && (NULL == AVIS_SHAPE (avis)) && (NULL == preass)
            && (N_with == NODE_TYPE (INFO_RHS (arg_info)))
            && (N_fold == NODE_TYPE (WITH_WITHOP (INFO_RHS (arg_info))))) {
            INFO_POSTASSIGN (arg_info)
              = MakeDTProxy (avis, INFO_POSTASSIGN (arg_info), arg_info);
        } else if (NULL != preass) {
            INFO_PREASSIGN (arg_info)
              = TCappendAssign (INFO_PREASSIGN (arg_info), preass);
        }
    } else {
        if (!((FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info)))
              && (NODE_TYPE (INFO_RHS (arg_info)) == N_ap)
              && (AP_FUNDEF (INFO_RHS (arg_info)) == INFO_FUNDEF (arg_info)))) {
            INFO_POSTASSIGN (arg_info)
              = MakeDTProxy (avis, INFO_POSTASSIGN (arg_info), arg_info);
        }
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAwith (node *arg_node, info *arg_info)
{
    node *oldwithid;

    DBUG_ENTER ();

    oldwithid = INFO_WITHID (arg_info);
    INFO_WITHID (arg_info) = WITH_WITHID (arg_node);

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_WITHID (arg_info) = oldwithid;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAApart( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAApart (node *arg_node, info *arg_info)
{
    node *ids;

    DBUG_ENTER ();

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    if (NODE_TYPE (PART_GENERATOR (arg_node)) == N_generator) {
        node *ivavis = IDS_AVIS (PART_VEC (arg_node));

        if (AVIS_DIM (ivavis) == NULL) {
            AVIS_DIM (ivavis) = TBmakeNum (1);
        }

        if (AVIS_SHAPE (ivavis) == NULL) {
            node *lb = GENERATOR_BOUND1 (PART_GENERATOR (arg_node));

            if (NODE_TYPE (lb) == N_array) {
                AVIS_SHAPE (ivavis) = SHshape2Array (ARRAY_FRAMESHAPE (lb));
            } else {
                AVIS_SHAPE (ivavis) = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (lb)));
            }
        }

        ids = PART_IDS (arg_node);
        while (ids != NULL) {
            node *idsavis = IDS_AVIS (ids);

            if (AVIS_DIM (idsavis) == NULL) {
                AVIS_DIM (idsavis) = TBmakeNum (0);
            }

            if (AVIS_SHAPE (idsavis) == NULL) {
                AVIS_SHAPE (idsavis) = TCmakeIntVector (NULL);
            }

            ids = IDS_NEXT (ids);
        }
    }

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAcode( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    CODE_CEXPRS (arg_node) = TRAVopt (CODE_CEXPRS (arg_node), arg_info);

    AVIS_SUBST (IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)))) = NULL;

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAcond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_TRAVMODE (arg_info)) {
    case TM_then:
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

        BLOCK_ASSIGNS (COND_THEN (arg_node))
          = TCappendAssign (INFO_PREBLOCK (arg_info),
                            BLOCK_ASSIGNS (COND_THEN (arg_node)));
        INFO_PREBLOCK (arg_info) = NULL;
        break;

    case TM_else:
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

        BLOCK_ASSIGNS (COND_ELSE (arg_node))
          = TCappendAssign (INFO_PREBLOCK (arg_info),
                            BLOCK_ASSIGNS (COND_ELSE (arg_node)));
        INFO_PREBLOCK (arg_info) = NULL;
        break;

    case TM_all:
        arg_node = TRAVcont (arg_node, arg_info);
        break;

    default:
        DBUG_UNREACHABLE ("Illegal traversal mode");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAfuncond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ISAAfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_TRAVMODE (arg_info)) {
    case TM_then:
        FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
        break;

    case TM_else:
        FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);
        break;

    case TM_all:
        arg_node = TRAVcont (arg_node, arg_info);
        break;

    default:
        DBUG_UNREACHABLE ("Illegal traversal mode");
        break;
    }

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Insert Shape Variables -->
 *****************************************************************************/

#undef DBUG_PREFIX
