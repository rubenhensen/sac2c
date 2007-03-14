/*
 * $Id$
 */

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
 *    in order to circumvent certain problems of diverence in the combination
 *    of augmented style and lir, we do remove the saabind at top of loops and
 *    annotate the parameters, which form dim and shape of the array, directly
 *    to the parameter-avis.
 *    Annotated style requires augmented style and activates it automatically.
 *
 * the style to be used may be choosen below by setting the appropriate defines.
 * by default the classical style is choosen. this ensures maximal compability.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/* FIXME, I AM A DIRTY HACK! */
#define ISAA_USE_AUGMENTED_STYLE 0
#define ISAA_USE_EVEN_ANNOTATED_STYLE 0

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
#include "dbug.h"
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

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    enum { TS_module, TS_fundef, TS_args } travscope;
    enum { TM_all, TM_then, TM_else } travmode;
    node *preassign;
    node *postassign;
    node *fundef;
    node *preblock;
    node *lhs;
    node *rhs;
    node *withid;
    node *args;
    bool recap;
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

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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
 * @fn node *ISAAdoInsertShapeVariables( node *syntax_tree)
 *
 *****************************************************************************/
node *
ISAAdoInsertShapeVariables (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ISAAdoInsertShapeVariables");

    info = MakeInfo ();

    INFO_TRAVSCOPE (info) = TS_module;
    TRAVpush (TR_isaa);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *ISAAdoInsertShapeVariablesOneFundef( node *fundef)
 *
 *****************************************************************************/
node *
ISAAdoInsertShapeVariablesOneFundef (node *fundef)
{
    info *info;

    DBUG_ENTER ("ISAAdoInsertShapeVariablesOneFundef");

    info = MakeInfo ();

    INFO_TRAVSCOPE (info) = TS_fundef;
    TRAVpush (TR_isaa);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
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
 * the three following static functions assist in the creation and insertion
 * of explicit dimension and shape arguments for functions.
 */

static node *
PrependSAAInFormalArgs (node *arg_node, info *arg_info)
{
    node *avis;
    node *newshp;
    node *newdim;
    node *preargs = NULL;

    DBUG_ENTER ("PrependSAAInFormalArgs");

    if (NULL != ARG_NEXT (arg_node)) {
        ARG_NEXT (arg_node) = PrependSAAInFormalArgs (ARG_NEXT (arg_node), arg_info);
    }

    avis = ARG_AVIS (arg_node);

    if ((!TUdimKnown (AVIS_TYPE (avis))) && (FALSE == AVIS_HASSAAARGUMENTS (avis))) {
        DBUG_PRINT ("ISAA", ("inserting a formal dim for %s", AVIS_NAME (avis)));

        newdim = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)),
                             TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        AVIS_HASSAAARGUMENTS (newdim) = TRUE;

        /* within this phase we append N_ids to DIM/SHAPE. but they are cleared
         * later on anyway. */
        AVIS_DIM (avis) = TBmakeId (newdim);

        preargs = TBmakeArg (newdim, preargs);
    }

    if ((!TUshapeKnown (AVIS_TYPE (avis))) && (!AVIS_HASSAAARGUMENTS (avis))) {
        DBUG_PRINT ("ISAA", ("inserting a formal shape for %s", AVIS_NAME (avis)));

        /* create a new avis, assign it to AVIS_SHAPE(avis) and prepend it into
         * the parameter-list of our function. */

        newshp = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)),
                             TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));
        AVIS_SHAPE (newshp) = AVIS_DIM (avis);
        AVIS_HASSAAARGUMENTS (newshp) = TRUE;

        AVIS_SHAPE (avis) = TBmakeId (newshp);
        preargs = TBmakeArg (newshp, preargs);

        AVIS_HASDTTHENPROXY (avis) = FALSE;
        AVIS_HASDTELSEPROXY (avis) = FALSE;
    }

    AVIS_HASSAAARGUMENTS (avis) = TRUE;

    arg_node = TCappendArgs (preargs, arg_node);

    DBUG_RETURN (arg_node);
}

static node *
PrependSAAInConcreteArgs (node *arg_node, node *funargs, info *arg_info)
{
    node *avis;
    node *funavis;
    node *newshp;
    node *newdim;
    node *preass;
    node *preargs = NULL;

    DBUG_ENTER ("PrependSAAInConcreteArgs");

    if ((NULL != EXPRS_NEXT (arg_node)) && (NULL != ARG_NEXT (funargs))) {
        EXPRS_NEXT (arg_node) = PrependSAAInConcreteArgs (EXPRS_NEXT (arg_node),
                                                          ARG_NEXT (funargs), arg_info);
    }

    /* there may be an saabind to our avis already */
    avis = ID_AVIS (EXPRS_EXPR (arg_node));
    funavis = ARG_AVIS (funargs);
    while (NULL != AVIS_SUBST (avis)) {
        avis = AVIS_SUBST (avis);
    }

    if ((!TUdimKnown (AVIS_TYPE (funavis))) && (!AVIS_HASSAAARGUMENTS (funavis))) {
        DBUG_PRINT ("ISAA", ("inserting a concrete dim for %s in fun %s",
                             AVIS_NAME (avis), FUNDEF_NAME (INFO_FUNDEF (arg_info))));

        /* this is quite similar as to how we proceeded with the shape. */

        /* 1. */
        newdim = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)),
                             TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        AVIS_DIM (newdim) = TBmakeNum (0);
        AVIS_SHAPE (newdim) = TCmakeIntVector (NULL);

        /* 2. */
        preass = TBmakeAssign (TBmakeLet (TBmakeIds (newdim, NULL),
                                          TCmakePrf1 (F_dim, TBmakeId (avis))),
                               NULL);
        AVIS_SSAASSIGN (newdim) = preass;
        INFO_PREASSIGN (arg_info) = TCappendAssign (INFO_PREASSIGN (arg_info), preass);

        /* 3. */
        INFO_FUNDEF (arg_info)
          = TCaddVardecs (INFO_FUNDEF (arg_info), TBmakeVardec (newdim, NULL));

        /* 4. */
        preargs = TBmakeExprs (TBmakeId (newdim), preargs);
    }

    if ((!TUshapeKnown (AVIS_TYPE (funavis))) && (!AVIS_HASSAAARGUMENTS (funavis))) {
        DBUG_PRINT ("ISAA", ("inserting a concrete shape for %s in fun %s",
                             AVIS_NAME (avis), FUNDEF_NAME (INFO_FUNDEF (arg_info))));
        /* todo:
         * 1. create the new avis, which holds the shape.
         * 2. create an N_let, that assigns shape(arg) to our new avis.
         * 3. put the new avis into our FUNDEF_VARDEC list.
         * 4. prepend the new avis into our parameter list.
         */

        /* 1. */
        if (!TUdimKnown (AVIS_TYPE (funavis))) {
            newshp
              = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)),
                            TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));

            if (NULL != AVIS_DIM (avis)) {
                if (N_num == NODE_TYPE (AVIS_DIM (avis))) {
                    AVIS_SHAPE (newshp) = TCmakeIntVector (
                      TBmakeExprs (TBmakeNum (NUM_VAL (AVIS_DIM (avis))), NULL));
                } else /* N_id */ {
                    AVIS_SHAPE (newshp) = TCmakeIntVector (
                      TBmakeExprs (TBmakeId (ID_AVIS (AVIS_DIM (avis))), NULL));
                }
            }
        } else {
            newshp = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)),
                                 TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

            if (NULL != AVIS_DIM (avis)) {
                if (N_num == NODE_TYPE (AVIS_DIM (avis))) {
                    AVIS_SHAPE (newshp) = TCmakeIntVector (
                      TBmakeExprs (TBmakeNum (NUM_VAL (AVIS_DIM (avis))), NULL));
                } else /* N_id */ {
                    AVIS_SHAPE (newshp) = TCmakeIntVector (
                      TBmakeExprs (TBmakeId (ID_AVIS (AVIS_DIM (avis))), NULL));
                }
            }
        }
        AVIS_DIM (newshp) = TBmakeNum (1);

        /* 2. */
        preass = TBmakeAssign (TBmakeLet (TBmakeIds (newshp, NULL),
                                          TCmakePrf1 (F_shape, TBmakeId (avis))),
                               NULL);
        AVIS_SSAASSIGN (newshp) = preass;
        INFO_PREASSIGN (arg_info) = TCappendAssign (INFO_PREASSIGN (arg_info), preass);

        /* 3. */
        INFO_FUNDEF (arg_info)
          = TCaddVardecs (INFO_FUNDEF (arg_info), TBmakeVardec (newshp, NULL));

        /* 4. */
        preargs = TBmakeExprs (TBmakeId (newshp), preargs);
    }

    arg_node = TCappendExprs (preargs, arg_node);

    DBUG_RETURN (arg_node);
}

#if (ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE)
static node *
ISAAretraverse (node *fun, info *arg_info)
{
    int travscope;
    int travmode;
    node *preblock;
    node *preassign;
    node *fundef;

    DBUG_ENTER ("ISAAretraverse");

    preblock = INFO_PREBLOCK (arg_info);
    preassign = INFO_PREASSIGN (arg_info);
    travscope = INFO_TRAVSCOPE (arg_info);
    travmode = INFO_TRAVMODE (arg_info);
    fundef = INFO_FUNDEF (arg_info);
    INFO_PREBLOCK (arg_info) = NULL;
    INFO_PREASSIGN (arg_info) = NULL;
    INFO_TRAVSCOPE (arg_info) = TS_args;

    fun = TRAVdo (fun, arg_info);

    INFO_PREBLOCK (arg_info) = preblock;
    INFO_PREASSIGN (arg_info) = preassign;
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
PrependAssign (node *prefix, node *rest)
{
    DBUG_ENTER ("PrependAssign");

    if (prefix != NULL) {
        if (NODE_TYPE (rest) == N_empty) {
            rest = FREEdoFreeNode (rest);
            rest = prefix;
        } else {
            rest = TCappendAssign (prefix, rest);
        }
    }

    DBUG_RETURN (rest);
}

static node *
MakeDTProxy (node *avis, node *postass, info *arg_info)
{
    bool makeproxy = FALSE;

    DBUG_ENTER ("MakeDTProxy");
    /*
    DBUG_PRINT( "ISAA", ("enter MakeDTProxy for %s", AVIS_NAME( avis ) ) );
    */
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

    if (makeproxy) {
        node *dimavis;
        node *shpavis;
        node *dimnode;
        node *shpnode;
#if (ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE)
        node *dim_postass = NULL;
        node *shp_postass = NULL;
#endif
        node *proxyavis;
        node *fundef;

        fundef = INFO_FUNDEF (arg_info);

        dimavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)),
                              TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        AVIS_DIM (dimavis) = TBmakeNum (0);
        AVIS_SHAPE (dimavis) = TCmakeIntVector (NULL);

        shpavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)),
                              TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));
        AVIS_DIM (shpavis) = TBmakeNum (1);
        AVIS_SHAPE (shpavis) = TCmakeIntVector (TBmakeExprs (TBmakeId (dimavis), NULL));

        proxyavis
          = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)), TYcopyType (AVIS_TYPE (avis)));
        AVIS_DIM (proxyavis) = TBmakeId (dimavis);
        AVIS_SHAPE (proxyavis) = TBmakeId (shpavis);

        FUNDEF_VARDEC (fundef)
          = TBmakeVardec (dimavis,
                          TBmakeVardec (shpavis, TBmakeVardec (proxyavis,
                                                               FUNDEF_VARDEC (fundef))));

        postass
          = TBmakeAssign (TBmakeLet (TBmakeIds (proxyavis, NULL),
                                     TCmakePrf3 (F_saabind, TBmakeId (dimavis),
                                                 TBmakeId (shpavis), TBmakeId (avis))),
                          postass);
        AVIS_SSAASSIGN (proxyavis) = postass;

#if (ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE)
        /* if we have passed the shape as a parameter to our function, we may now
         * look up its avis in AVIS_SHAPE. */
        if ((NULL != AVIS_SHAPE (avis)) && (TS_args == INFO_TRAVSCOPE (arg_info))) {

            /* BEAUTIFY_ME: this is a little ugly:
             * as we want a saabind'ed shape, we have to proxify it first; but as
             * it would be proxified after this arg, we have to call for it by hand.
             */
            shp_postass = MakeDTProxy (ID_AVIS (AVIS_SHAPE (avis)), NULL, arg_info);

            if (NULL != AVIS_SUBST (ID_AVIS (AVIS_SHAPE (avis)))) {
                shpnode = TBmakeId (AVIS_SUBST (ID_AVIS (AVIS_SHAPE (avis))));
            } else {
                shpnode = TBmakeId (ID_AVIS (AVIS_SHAPE (avis)));
            }
        } else {
            shpnode = TCmakePrf1 (F_shape, TBmakeId (avis));
        }
#else
        shpnode = TCmakePrf1 (F_shape, TBmakeId (avis));
#endif

        postass = TBmakeAssign (TBmakeLet (TBmakeIds (shpavis, NULL), shpnode), postass);
        AVIS_SSAASSIGN (shpavis) = postass;

#if (ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE)
        /* same for the dimension we may have. */
        if ((NULL != AVIS_DIM (avis)) && (TS_args == INFO_TRAVSCOPE (arg_info))) {

            /* BEAUTIFY_ME: same as above */
            dim_postass = MakeDTProxy (ID_AVIS (AVIS_DIM (avis)), NULL, arg_info);

            if (NULL != AVIS_SUBST (ID_AVIS (AVIS_DIM (avis)))) {
                dimnode = TBmakeId (AVIS_SUBST (ID_AVIS (AVIS_DIM (avis))));
            } else {
                dimnode = TBmakeId (ID_AVIS (AVIS_DIM (avis)));
            }
        } else {
            dimnode = TCmakePrf1 (F_dim, TBmakeId (avis));
        }
#else
        dimnode = TCmakePrf1 (F_dim, TBmakeId (avis));
#endif

        postass = TBmakeAssign (TBmakeLet (TBmakeIds (dimavis, NULL), dimnode), postass);
        AVIS_SSAASSIGN (dimavis) = postass;

        AVIS_SUBST (avis) = proxyavis;

#if (ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE)
        if (NULL != shp_postass) {
            postass = PrependAssign (shp_postass, postass);
        }
        if (NULL != dim_postass) {
            postass = PrependAssign (dim_postass, postass);
        }
#endif

        switch (INFO_TRAVMODE (arg_info)) {
        case TM_then:
            AVIS_HASDTTHENPROXY (avis) = TRUE;
            AVIS_HASDTTHENPROXY (proxyavis) = TRUE;
            break;

        case TM_else:
            AVIS_HASDTELSEPROXY (avis) = TRUE;
            AVIS_HASDTELSEPROXY (proxyavis) = TRUE;
            break;

        case TM_all:
            AVIS_HASDTTHENPROXY (avis) = TRUE;
            AVIS_HASDTELSEPROXY (avis) = TRUE;
            AVIS_HASDTTHENPROXY (proxyavis) = TRUE;
            AVIS_HASDTELSEPROXY (proxyavis) = TRUE;
            break;
        }

#if (ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE)
        if ((AVIS_HASDTTHENPROXY (avis) == TRUE)
            && (AVIS_HASDTELSEPROXY (avis) == TRUE)) {
            /* clean the avis, as we do not want shape/dim on our parameters */
            AVIS_SHAPE (avis) = NULL;
            AVIS_DIM (avis) = NULL;
        }
#endif
    }

    DBUG_RETURN (postass);
}

static node *
MakeArgProxies (node *arg_node, info *arg_info)
{
    node *ass = NULL;

    DBUG_ENTER ("MakeArgProxies");

    if (arg_node != NULL) {
        ass = MakeArgProxies (ARG_NEXT (arg_node), arg_info);
        ass = MakeDTProxy (ARG_AVIS (arg_node), ass, arg_info);
    }

    DBUG_RETURN (ass);
}

static node *
RemoveAvisSubst (node *fundef)
{
    DBUG_ENTER ("RemoveAvisSubst");

    if (FUNDEF_ARGS (fundef) != NULL) {
        FUNDEF_ARGS (fundef) = TRAVdo (FUNDEF_ARGS (fundef), NULL);
    }
    if (FUNDEF_VARDEC (fundef) != NULL) {
        FUNDEF_VARDEC (fundef) = TRAVdo (FUNDEF_VARDEC (fundef), NULL);
    }

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
    DBUG_ENTER ("ISAAfundef");

    DBUG_PRINT ("ISAA", ("enter %s", FUNDEF_NAME (arg_node)));

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;

        /*
         * Resolve shape variables to dim, shape, idx_shape_sel
         * In case of cond functions, we need a copy of those variables
         */
        if (FUNDEF_ISCONDFUN (arg_node)) {
            INFO_TRAVMODE (arg_info) = TM_then;
            arg_node = RemoveAvisSubst (arg_node);

            INFO_PREBLOCK (arg_info) = MakeArgProxies (FUNDEF_ARGS (arg_node), arg_info);

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            INFO_TRAVMODE (arg_info) = TM_else;
            arg_node = RemoveAvisSubst (arg_node);

            INFO_PREBLOCK (arg_info) = MakeArgProxies (FUNDEF_ARGS (arg_node), arg_info);

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        } else {
            node *preblock;

            INFO_TRAVMODE (arg_info) = TM_all;
            arg_node = RemoveAvisSubst (arg_node);

            preblock = MakeArgProxies (FUNDEF_ARGS (arg_node), arg_info);

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            FUNDEF_INSTR (arg_node) = TCappendAssign (preblock, FUNDEF_INSTR (arg_node));
            INFO_PREBLOCK (arg_info) = NULL;
        }

        /*
         * Clean up the AVIS_SUBST mess
         */
        arg_node = RemoveAvisSubst (arg_node);
    }

    if ((INFO_TRAVSCOPE (arg_info) == TS_module) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

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
#if (ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE)
    node *fun;
#endif

    DBUG_ENTER ("ISAAap");

#if (ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE)
    fun = AP_FUNDEF (arg_node);

    if ((NULL != FUNDEF_ARGS (fun)) && (TS_args != INFO_TRAVSCOPE (arg_info))
        && (fun != INFO_FUNDEF (arg_info))) {
        if (FUNDEF_ISCONDFUN (fun)) {
            /* three things have to be done in order to set the SAA up:
             * 1. Introduce the new arguments on both application and function side.
             * 2. Generate new proxys inside the function, thereby propagating the
             *    new parameters and removing the AVIS_DIM and AVIS_SHAPE info.
             */

            /* 1. */
            AP_ARGS (arg_node) = PrependSAAInConcreteArgs (AP_ARGS (arg_node),
                                                           FUNDEF_ARGS (fun), arg_info);
            FUNDEF_ARGS (fun) = PrependSAAInFormalArgs (FUNDEF_ARGS (fun), arg_info);

            AP_FUNDEF (arg_node) = ISAAretraverse (fun, arg_info);
        } else if (FUNDEF_ISDOFUN (fun)) {
            /* two things done:
             * 1. Copy FUNDEF_ARGS to INFO_ARGS, so we have a original reference when
             *    traversing the function later on.
             * 2. insert arguments, just like for IsCondFun above.
             */

            DBUG_PRINT ("ISAA", ("calling a loop fundef %s. (info %s)", FUNDEF_NAME (fun),
                                 FUNDEF_NAME (INFO_FUNDEF (arg_info))));

            /* 1. */
            INFO_ARGS (arg_info) = DUPdoDupTree (FUNDEF_ARGS (fun));

            /* 2. */
            FUNDEF_ARGS (fun) = PrependSAAInFormalArgs (FUNDEF_ARGS (fun), arg_info);
            AP_ARGS (arg_node)
              = PrependSAAInConcreteArgs (AP_ARGS (arg_node), INFO_ARGS (arg_info),
                                          arg_info);

            AP_FUNDEF (arg_node) = ISAAretraverse (fun, arg_info);
        }

    } else if ((TS_args == INFO_TRAVSCOPE (arg_info)) && (FUNDEF_ISDOFUN (fun))
               && (fun == INFO_FUNDEF (arg_info))) {

        DBUG_PRINT ("ISAA", ("inner application of a loop fundef %s. (info %s)",
                             FUNDEF_NAME (AP_FUNDEF (arg_node)),
                             FUNDEF_NAME (INFO_FUNDEF (arg_info))));

        INFO_RECAP (arg_info) = TRUE;

        /* now the time has come, when we deliberately need the copied args. */
        AP_ARGS (arg_node)
          = PrependSAAInConcreteArgs (AP_ARGS (arg_node), INFO_ARGS (arg_info), arg_info);

        INFO_ARGS (arg_info) = FREEdoFreeTree (INFO_ARGS (arg_info));
    }

    /* we may now traverse the arguments, in order to take care of AVIS_SUBST */
    if (NULL != AP_ARGS (arg_node)) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }
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
    DBUG_ENTER ("ISAAavis");

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
    DBUG_ENTER ("ISAAblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

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

    DBUG_ENTER ("ISAAassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

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

        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }

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
    DBUG_ENTER ("ISAAlet");

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

    DBUG_ENTER ("ISAAids");

    avis = IDS_AVIS (arg_node);

#if (ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE)
    if (TS_args != INFO_TRAVSCOPE (arg_info)) {
#endif

        if ((NODE_TYPE (INFO_RHS (arg_info)) != N_ap)
            && (!((NODE_TYPE (INFO_RHS (arg_info)) == N_prf)
                  && ((PRF_PRF (INFO_RHS (arg_info)) == F_saabind)
                      || (PRF_PRF (INFO_RHS (arg_info)) == F_type_conv))))) {
            if (AVIS_DIM (avis) == NULL) {
                if (TUdimKnown (AVIS_TYPE (avis))) {
                    AVIS_DIM (avis) = TBmakeNum (TYgetDim (AVIS_TYPE (avis)));
                } else {
                    INFO_PREASSIGN (arg_info)
                      = TCappendAssign (INFO_PREASSIGN (arg_info),
                                        MDEdoMakeDimExpression (INFO_RHS (arg_info), avis,
                                                                INFO_LHS (arg_info),
                                                                INFO_FUNDEF (arg_info)));
                }
            }

            if ((AVIS_DIM (avis) != NULL) && (AVIS_SHAPE (avis) == NULL)) {
                if (TUshapeKnown (AVIS_TYPE (avis))) {
                    AVIS_SHAPE (avis) = SHshape2Array (TYgetShape (AVIS_TYPE (avis)));
                } else {
                    INFO_PREASSIGN (arg_info)
                      = TCappendAssign (INFO_PREASSIGN (arg_info),
                                        MSEdoMakeShapeExpression (INFO_RHS (arg_info),
                                                                  avis,
                                                                  INFO_LHS (arg_info),
                                                                  INFO_FUNDEF (
                                                                    arg_info)));
                }
            }
        } else {
            if (!((FUNDEF_ISDOFUN (INFO_FUNDEF (arg_info)))
                  && (NODE_TYPE (INFO_RHS (arg_info)) == N_ap)
                  && (AP_FUNDEF (INFO_RHS (arg_info)) == INFO_FUNDEF (arg_info)))) {
                INFO_POSTASSIGN (arg_info)
                  = MakeDTProxy (avis, INFO_POSTASSIGN (arg_info), arg_info);
            }
        }

#if (ISAA_USE_AUGMENTED_STYLE || ISAA_USE_EVEN_ANNOTATED_STYLE)
    }
#endif

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

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

    DBUG_ENTER ("ISAAwith");

    oldwithid = INFO_WITHID (arg_info);
    INFO_WITHID (arg_info) = WITH_WITHID (arg_node);

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }
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

    DBUG_ENTER ("ISAApart");

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    if (NODE_TYPE (PART_GENERATOR (arg_node)) == N_generator) {
        node *ivavis = IDS_AVIS (PART_VEC (arg_node));

        if (AVIS_DIM (ivavis) == NULL) {
            AVIS_DIM (ivavis) = TBmakeNum (1);
        }

        if (AVIS_SHAPE (ivavis) == NULL) {
            node *lb = GENERATOR_BOUND1 (PART_GENERATOR (arg_node));

            if (NODE_TYPE (lb) == N_array) {
                AVIS_SHAPE (ivavis) = SHshape2Array (ARRAY_SHAPE (lb));
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

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

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
    DBUG_ENTER ("ISAAcode");

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }
    if (CODE_CEXPRS (arg_node) != NULL) {
        CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    }

    AVIS_SUBST (IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)))) = NULL;

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

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
    DBUG_ENTER ("ISAAid");

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
    DBUG_ENTER ("ISAAcond");

    switch (INFO_TRAVMODE (arg_info)) {
    case TM_then:
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

        BLOCK_INSTR (COND_THEN (arg_node))
          = PrependAssign (INFO_PREBLOCK (arg_info), BLOCK_INSTR (COND_THEN (arg_node)));
        INFO_PREBLOCK (arg_info) = NULL;
        break;

    case TM_else:
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

        BLOCK_INSTR (COND_ELSE (arg_node))
          = PrependAssign (INFO_PREBLOCK (arg_info), BLOCK_INSTR (COND_ELSE (arg_node)));
        INFO_PREBLOCK (arg_info) = NULL;
        break;

    case TM_all:
        arg_node = TRAVcont (arg_node, arg_info);
        break;

    default:
        DBUG_ASSERT ((0), "Illegal traversal mode");
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
    DBUG_ENTER ("ISAAfuncond");

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
        DBUG_ASSERT ((0), "Illegal traversal mode");
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
