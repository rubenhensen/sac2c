/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup btf Bundle to fundef
 *
 * Module description goes here.
 *
 * @ingroup btf
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file bundle_to_fundef.c
 *
 * Prefix: BTF
 *
 *****************************************************************************/
#include "bundle_to_fundef.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "user_types.h"
#include "namespaces.h"
#include "globals.h"
#include "str.h"
#include "tree_compound.h"
#include "free.h"
#include "deserialize.h"
#include "shape.h"
#include "DupTree.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool gencode;
    node *args;
    node *rets;
    node *vardecs;
    node *code;
};

/**
 * A template entry in the template info structure
 */
#define INFO_GENCODE(n) ((n)->gencode)
#define INFO_ARGS(n) ((n)->args)
#define INFO_RETS(n) ((n)->rets)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_CODE(n) ((n)->code)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_GENCODE (result) = FALSE;
    INFO_ARGS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_CODE (result) = NULL;

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
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *LoadPreludeFunctions(node *syntax_tree)
 *
 * @brief Loads the prelude functions needed for the wrappers
 *
 *****************************************************************************/
static node *
LoadPreludeFunctions (node *syntax_tree)
{
    DBUG_ENTER ("LoadPreludeFunctions");

    DSaddSymbolByName ("isInt", SET_wrapperhead, global.preludename);
    DSaddSymbolByName ("isBool", SET_wrapperhead, global.preludename);
    DSaddSymbolByName ("isFloat", SET_wrapperhead, global.preludename);
    DSaddSymbolByName ("isDouble", SET_wrapperhead, global.preludename);
    DSaddSymbolByName ("isChar", SET_wrapperhead, global.preludename);
    DSaddSymbolByName ("unwrap", SET_wrapperhead, global.preludename);
    DSaddSymbolByName ("wrapInt", SET_wrapperhead, global.preludename);
    DSaddSymbolByName ("wrapBool", SET_wrapperhead, global.preludename);
    DSaddSymbolByName ("wrapFloat", SET_wrapperhead, global.preludename);
    DSaddSymbolByName ("wrapDouble", SET_wrapperhead, global.preludename);
    DSaddSymbolByName ("wrapChar", SET_wrapperhead, global.preludename);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *ArgsToSacArgs(node *args)
 *
 * @brief Build from the given args chain a SACarg arg chain
 *        of same length.
 *
 *****************************************************************************/
static node *
ArgsToSacArgs (node *args)
{
    node *result = NULL;
    usertype sacarg;

    DBUG_ENTER ("ArgsToSacArgs");

    if (args != NULL) {
        result = ArgsToSacArgs (ARG_NEXT (args));

        sacarg = UTfindUserType ("SACarg", NSgetNamespace (global.preludename));

        DBUG_ASSERT ((sacarg != UT_NOT_DEFINED), "cannot find SACarg type in prelude!");

        result
          = TBmakeArg (TBmakeAvis (STRcpy (AVIS_NAME (ARG_AVIS (args))),
                                   TYmakeAKS (TYmakeUserType (sacarg), SHmakeShape (0))),
                       result);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *RetsToSacArgs(node *rets)
 *
 * @brief Build from the given rets chain a SACarg ret chain
 *        of same length.
 *
 *****************************************************************************/
static node *
RetsToSacArgs (node *rets)
{
    node *result = NULL;
    usertype sacarg;

    DBUG_ENTER ("RetsToSacArgs");

    if (rets != NULL) {
        result = RetsToSacArgs (RET_NEXT (rets));

        sacarg = UTfindUserType ("SACarg", NSgetNamespace (global.preludename));

        DBUG_ASSERT ((sacarg != UT_NOT_DEFINED), "cannot find SACarg type in prelude!");

        result = TBmakeRet (TYmakeAKS (TYmakeUserType (sacarg), SHmakeShape (0)), result);
    }

    DBUG_RETURN (result);
}

static node *
MakeDispatchError (node *fundef, info *arg_info)
{
    node *result;
    node *exprs;
    node *retids;
    int skip = 0;

    DBUG_ENTER ("MakeDispatchError");

    exprs = TBmakeExprs (TCmakeStrCopy (FUNDEF_NAME (fundef)),
                         TCcreateExprsFromArgs (INFO_ARGS (arg_info)));

    /*
     * we know that all retids are of type SACarg,
     * so the order in which we build the chain
     * does not matter
     */
    retids = INFO_RETS (arg_info);
    while (retids != NULL) {
        exprs
          = TBmakeExprs (TBmakeType (TYcopyType (AVIS_TYPE (IDS_AVIS (retids)))), exprs);
        retids = IDS_NEXT (retids);
        skip++;
    }
    exprs = TBmakeExprs (TBmakeNum (skip), exprs);

    result = TBmakeAssign (TBmakeLet (DUPdoDupTree (INFO_RETS (arg_info)),
                                      TBmakePrf (F_dispatch_error, exprs)),
                           NULL);

    DBUG_RETURN (result);
}

static node *
PickPredFun (simpletype type, node *args)
{
    char *name = NULL;
    node *result;

    DBUG_ENTER ("PickPredFun");

    switch (type) {
    case T_int:
        name = "isInt";
        break;
    case T_bool:
        name = "isBool";
        break;
    case T_float:
        name = "isFloat";
        break;
    case T_double:
        name = "isDouble";
        break;
    case T_char:
        name = "isChar";
        break;
    default:
        DBUG_ASSERT (0, "unhandled built-in type");
    }

    result = DSdispatchFunCall (NSgetNamespace (global.preludename), name, args);

    DBUG_RETURN (result);
}

static node *
BuildPredicateForArgs (node *apargs, node *wrapargs, node **precond, node **vardecs)
{
    node *result;
    node *predavis;
    node *andavis;
    node *args;
    node *assigns;
    simpletype argtype;

    DBUG_ENTER ("BuildPredicateForArgs");

    if (apargs != NULL) {
        result = BuildPredicateForArgs (ARG_NEXT (apargs), ARG_NEXT (wrapargs), precond,
                                        vardecs);

        DBUG_ASSERT (TYisSimple (TYgetScalar (AVIS_TYPE (ARG_AVIS (apargs)))),
                     "only built in types are supported!");
        argtype = TYgetSimpleType (TYgetScalar (AVIS_TYPE (ARG_AVIS (apargs))));

        predavis = TBmakeAvis (TRAVtmpVar (),
                               TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
        *vardecs = TBmakeVardec (predavis, *vardecs);

        andavis = TBmakeAvis (TRAVtmpVar (),
                              TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
        *vardecs = TBmakeVardec (andavis, *vardecs);

        assigns
          = TBmakeAssign (TBmakeLet (TBmakeIds (andavis, NULL),
                                     TCmakePrf2 (F_and_SxS, result, TBmakeId (predavis))),
                          NULL);
        result = TBmakeId (andavis);

        args = TBmakeExprs (TBmakeId (ARG_AVIS (wrapargs)), NULL);

        assigns = TBmakeAssign (TBmakeLet (TBmakeIds (predavis, NULL),
                                           PickPredFun (argtype, args)),
                                assigns);

        *precond = TCappendAssign (*precond, assigns);
    } else {
        result = TBmakeBool (TRUE);
    }

    DBUG_RETURN (result);
}

static node *
PickInputConversion (simpletype type, node *args)
{
    char *name = NULL;
    node *result;

    DBUG_ENTER ("PickInputConversion");

    switch (type) {
    case T_int:
        name = "unwrapInt";
        break;
    case T_bool:
        name = "unwrapBool";
        break;
    case T_float:
        name = "unwrapFloat";
        break;
    case T_double:
        name = "unwrapDouble";
        break;
    case T_char:
        name = "unwrapChar";
        break;
    default:
        DBUG_ASSERT (0, "unhandled built-in type");
    }

    result = DSdispatchFunCall (NSgetNamespace (global.preludename), name, args);

    DBUG_ASSERT ((result != NULL),
                 "Cannot find matching instance for sacprelude::unwrapXXX!");

    DBUG_RETURN (result);
}

static node *
ConvertInputs (node *apargs, node *wrapargs, node **vardecs, node **assigns)
{
    node *result = NULL;
    node *avis;
    simpletype argtype;
    node *args;

    DBUG_ENTER ("ConvertInputs");

    if (apargs != NULL) {
        result = ConvertInputs (ARG_NEXT (apargs), ARG_NEXT (wrapargs), vardecs, assigns);

        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (ARG_AVIS (apargs))));
        *vardecs = TBmakeVardec (avis, *vardecs);

        DBUG_ASSERT (TYisSimple (TYgetScalar (AVIS_TYPE (ARG_AVIS (apargs)))),
                     "only built in types are supported!");
        argtype = TYgetSimpleType (TYgetScalar (AVIS_TYPE (ARG_AVIS (apargs))));

        args = TBmakeExprs (TBmakeId (ARG_AVIS (wrapargs)), NULL);

        *assigns = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                            PickInputConversion (argtype, args)),
                                 *assigns);

        result = TBmakeExprs (TBmakeId (avis), result);
    }

    DBUG_RETURN (result);
}

static node *
ConvertOutputs (node *aprets, node *wrapretids, node **vardecs, node **assigns)
{
    node *result = NULL;
    node *avis;
    node *wrapinstance;

    DBUG_ENTER ("ConvertOutputs");

    if (aprets != NULL) {
        result
          = ConvertInputs (RET_NEXT (aprets), IDS_NEXT (wrapretids), vardecs, assigns);

        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (RET_TYPE (aprets)));
        *vardecs = TBmakeVardec (avis, *vardecs);

        result = TBmakeIds (avis, result);

        wrapinstance = DSdispatchFunCall (NSgetNamespace (global.preludename), "wrap",
                                          TBmakeExprs (TBmakeId (avis), NULL));

        DBUG_ASSERT ((wrapinstance != NULL),
                     "Cannot find matching instance of sacprelude::wrap!");

        *assigns = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (wrapretids), NULL),
                                            wrapinstance),
                                 *assigns);
    }

    DBUG_RETURN (result);
}

static node *
BuildApplication (node *fundef, info *arg_info)
{
    node *result;
    node *preconv = NULL;
    node *postconv = NULL;
    node *apply;

    DBUG_ENTER ("BuildApplication");

    apply = TBmakeAssign (TBmakeLet (ConvertOutputs (FUNDEF_RETS (fundef),
                                                     INFO_RETS (arg_info),
                                                     &INFO_VARDECS (arg_info), &postconv),
                                     TBmakeAp (fundef,
                                               ConvertInputs (FUNDEF_ARGS (fundef),
                                                              INFO_ARGS (arg_info),
                                                              &INFO_VARDECS (arg_info),
                                                              &preconv))),
                          NULL);

    result = TCappendAssign (preconv, TCappendAssign (apply, postconv));

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *BTFdoBundleToFundef( node *syntax_tree)
 *
 *****************************************************************************/
node *
BTFdoBundleToFundef (node *syntax_tree)
{
    info *info;
    node *fundefs;

    DBUG_ENTER ("BTFdoBundleToFundef");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "BTFdoBundleToFundef operates on modules only!");

    /*
     * we take the current fundef chain
     * out of the module to hide the functions
     * from the module system!
     */
    fundefs = MODULE_FUNS (syntax_tree);
    MODULE_FUNS (syntax_tree) = NULL;

    DSinitDeserialize (syntax_tree);

    /*
     * preload the prelude functions needed
     */
    syntax_tree = LoadPreludeFunctions (syntax_tree);

    info = MakeInfo ();

    TRAVpush (TR_btf);
    fundefs = TRAVdo (fundefs, info);
    TRAVpop ();

    info = FreeInfo (info);

    DSfinishDeserialize (syntax_tree);

    /*
     * and back in they go
     */
    MODULE_FUNS (syntax_tree) = TCappendFundef (MODULE_FUNS (syntax_tree), fundefs);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *BTFfunbundle(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
BTFfunbundle (node *arg_node, info *arg_info)
{
    node *result;
    node *block;
    node *args;
    node *rets;
    node *retassign;

    DBUG_ENTER ("BTFfunbundle");

    /*
     * bottom up
     */
    if (FUNBUNDLE_NEXT (arg_node) != NULL) {
        FUNBUNDLE_NEXT (arg_node) = TRAVdo (FUNBUNDLE_NEXT (arg_node), arg_info);
    }

    args = ArgsToSacArgs (FUNDEF_ARGS (FUNBUNDLE_FUNDEF (arg_node)));
    rets = RetsToSacArgs (FUNDEF_RETS (FUNBUNDLE_FUNDEF (arg_node)));

    INFO_ARGS (arg_info) = args;
    INFO_RETS (arg_info) = TCcreateIdsFromRets (rets, &INFO_VARDECS (arg_info));

    INFO_GENCODE (arg_info) = TRUE;
    FUNBUNDLE_FUNDEF (arg_node) = TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);
    INFO_GENCODE (arg_info) = FALSE;

    /*
     * add return statement
     */
    retassign = TBmakeReturn (TCcreateExprsFromIds (INFO_RETS (arg_info)));

    INFO_CODE (arg_info)
      = TCappendAssign (INFO_CODE (arg_info), TBmakeAssign (retassign, NULL));

    block = TBmakeBlock (INFO_CODE (arg_info), INFO_VARDECS (arg_info));
    INFO_CODE (arg_info) = NULL;
    INFO_VARDECS (arg_info) = NULL;

    result = TBmakeFundef (STRcpy (FUNBUNDLE_NAME (arg_node)),
                           NSdupNamespace (FUNBUNDLE_NS (arg_node)), rets, args, block,
                           FUNBUNDLE_FUNDEF (arg_node));

    FUNDEF_LINKNAME (result) = STRcat (CWRAPPER_PREFIX, FUNBUNDLE_EXTNAME (arg_node));
    FUNDEF_RETURN (result) = retassign;

    FUNBUNDLE_FUNDEF (arg_node) = NULL;

    result = TCappendFundef (result, FUNBUNDLE_NEXT (arg_node));
    arg_node = FREEdoFreeNode (arg_node);

    INFO_RETS (arg_info) = FREEdoFreeTree (INFO_RETS (arg_info));
    INFO_ARGS (arg_info) = NULL;

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *BTFfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
BTFfundef (node *arg_node, info *arg_info)
{
    node *predicate = NULL;
    node *precond = NULL;
    node *thencode = NULL;

    DBUG_ENTER ("BTFfundef");

    if (INFO_GENCODE (arg_info)) {
        /*
         * inside funbundle
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            /*
             * process the other instances bottom up
             */

            if (FUNDEF_NEXT (arg_node) != NULL) {
                FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
            } else {
                INFO_CODE (arg_info) = MakeDispatchError (arg_node, arg_info);
            }

            predicate
              = BuildPredicateForArgs (FUNDEF_ARGS (arg_node), INFO_ARGS (arg_info),
                                       &precond, &INFO_VARDECS (arg_info));

            thencode = BuildApplication (arg_node, arg_info);

            INFO_CODE (arg_info)
              = TBmakeAssign (TBmakeCond (predicate, TBmakeBlock (thencode, NULL),
                                          TBmakeBlock (INFO_CODE (arg_info), NULL)),
                              NULL);

            INFO_CODE (arg_info) = TCappendAssign (precond, INFO_CODE (arg_info));
        } else {
            INFO_CODE (arg_info) = BuildApplication (arg_node, arg_info);
        }
    } else {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Bundle to fundef -->
 *****************************************************************************/
