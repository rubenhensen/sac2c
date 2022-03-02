#include <stdio.h>

#define DBUG_PREFIX "NTC"
#include "debug.h"

#include "ctinfo.h"

#include "types.h"
#include "phase.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "DupTree.h"
#include "globals.h"
#include "print.h"
#include "shape.h"
#include "insert_vardec.h"
#include "create_wrappers.h"
#include "split_wrappers.h"

#include "user_types.h"
#include "new_types.h"
#include "type_utils.h"
#include "sig_deps.h"
#include "ssi.h"
#include "new_typecheck.h"
#include "ct_basic.h"
#include "ct_fun.h"
#include "ct_prf.h"
#include "ct_with.h"
#include "type_errors.h"
#include "type_utils.h"
#include "specialize.h"
#include "constants.h"
#include "deserialize.h"
#include "namespaces.h"
#include "resolvesymboltypes.h"
#include "map_call_graph.h"
#include "map_fun_trav.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

/**
 *
 * @defgroup ntc Type Inference
 *
 * This group contains all those files/ modules that belong to the
 * (new) type inference system.
 *
 * @{
 */

/**
 *
 * @file new_typecheck.c
 *
 * This file contains the central type inference / type checking mechanism
 * of the compiler. It relies on support from several other modules.
 * These are:
 *   ... to be continued
 */

/*******************************************************************************
 * Thus, we finally find the following usages of the arg_info node:
 *
 *    INFO_NTC_TYPE             the inferred type of the expression traversed
 *    INFO_NTC_NUM_EXPRS_SOFAR  is used to count the number of exprs while
 *                              traversing them
 * ...
 */

/**
 * INFO structure
 */
struct INFO {
    ntype *type;
    ntype *gen_type;
    ntype *bodies_type;
    size_t num_exprs_sofar;
    node *last_assign;
    node *ptr_return;
    node *wl_ops;
    ntype *accu;
    size_t fold_cnt;
    ntype *prop_objs;
    size_t prop_cnt;
    bool is_type_upgrade;
};

/**
 * INFO macros
 */
#define INFO_TYPE(n) (n->type)
#define INFO_GEN_TYPE(n) (n->gen_type)
#define INFO_BODIES_TYPE(n) (n->bodies_type)
#define INFO_NUM_EXPRS_SOFAR(n) (n->num_exprs_sofar)
#define INFO_LAST_ASSIGN(n) (n->last_assign)
#define INFO_RETURN(n) (n->ptr_return)
#define INFO_WL_OPS(n) (n->wl_ops)
#define INFO_EXP_ACCU(n) (n->accu)
#define INFO_ACT_FOLD_POS(n) (n->fold_cnt)
#define INFO_PROP_OBJS(n) (n->prop_objs)
#define INFO_ACT_PROP_OBJ(n) (n->prop_cnt)
#define INFO_IS_TYPE_UPGRADE(n) (n->is_type_upgrade)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_TYPE (result) = NULL;
    INFO_GEN_TYPE (result) = NULL;
    INFO_BODIES_TYPE (result) = NULL;
    INFO_NUM_EXPRS_SOFAR (result) = 0;
    INFO_LAST_ASSIGN (result) = NULL;
    INFO_RETURN (result) = NULL;
    INFO_WL_OPS (result) = NULL;
    INFO_EXP_ACCU (result) = NULL;
    INFO_ACT_FOLD_POS (result) = 0;
    INFO_PROP_OBJS (result) = NULL;
    INFO_ACT_PROP_OBJ (result) = 0;
    INFO_IS_TYPE_UPGRADE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 *
 * Static global variables
 *
 */

static const ct_funptr prf_tc_funtab[] = {
#define PRFntc_fun(ntc_fun) ntc_fun
#include "prf_info.mac"
};

static const te_funptr prf_te_funtab[] = {
#define PRFte_fun(te_fun) te_fun
#include "prf_info.mac"
};

/**
 *
 * @name Entry functions for calling the type inference:
 *
 * @{
 */

static node *
MarkWrapperAsChecked (node *fundef, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISWRAPPERFUN (fundef)) {
        FUNDEF_TCSTAT (fundef) = NTC_checked;
    }

    DBUG_RETURN (fundef);
}

static node *
TagAsUnchecked (node *fundef, info *info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Function: %s marked as unchecked", FUNDEF_NAME (fundef));
    FUNDEF_TCSTAT (fundef) = NTC_not_checked;

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCdoNewTypeCheckOneFunction( node *arg_node)
 *
 *****************************************************************************/

static node *
NTCdoNewTypeCheckOneFunction (node *arg_node)
{
    ntype *old_rets = NULL, *new_rets = NULL, *new_rets_fixed = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "NTCdoNewTypeCheckOneFunction can only be applied to N_fundef");

    if (!FUNDEF_ISWRAPPERFUN (arg_node) && !FUNDEF_ISLACFUN (arg_node)
        && (FUNDEF_BODY (arg_node) != NULL)) {
        int oldmaxspec;
        info *arg_info;

        /*
         * De-activate specialising
         */
        oldmaxspec = global.maxspec;
        global.maxspec = 0;

        /*
         * Apply typechecker
         */
        DBUG_PRINT ("Untagging function: %s", FUNDEF_NAME (arg_node));
        MCGdoMapCallGraph (arg_node, TagAsUnchecked, NULL, MCGcontLacFun, NULL);
        arg_node = TagAsUnchecked (arg_node, NULL);

        if (FUNDEF_RETS (arg_node) != NULL) {
            /**
             * collect the old return types for later comparison
             */
            old_rets = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));

            /**
             * no alphaMax here, as icc may actually lead to less precise
             * types!!! (see also UpdateVarSignature in specialize.c!)
             */
            FUNDEF_RETS (arg_node) = TUrettypes2alphaMax (FUNDEF_RETS (arg_node));
        }

        TRAVpush (TR_ntc);

        arg_info = MakeInfo ();
        INFO_IS_TYPE_UPGRADE (arg_info) = TRUE;

        arg_node = TRAVdo (arg_node, arg_info);

        arg_info = FreeInfo (arg_info);

        TRAVpop ();

        /**
         * check for return type upgrades:
         */
        if (FUNDEF_RETS (arg_node) != NULL) {
            new_rets = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));
            new_rets_fixed = TYfixAndEliminateAlpha (new_rets);
            FUNDEF_WASUPGRADED (arg_node) = !TYeqTypes (old_rets, new_rets_fixed);
            old_rets = TYfreeType (old_rets);
            new_rets = TYfreeType (new_rets);
            new_rets_fixed = TYfreeType (new_rets_fixed);
        } else {
            FUNDEF_WASUPGRADED (arg_node) = FALSE;
        }

        /**
         * increase global optimisation counter
         * if function was upgraded
         */
        if (FUNDEF_WASUPGRADED (arg_node)) {
            global.optcounters.tup_upgrades++;
        }

        /*
         * Restore global.maxspec
         */
        global.maxspec = oldmaxspec;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCdoNewTypeCheck( node *arg_node)
 *
 * description:
 *    starts the new type checking traversal!
 *
 ******************************************************************************/
node *
NTCdoNewTypeCheck (node *arg_node)
{
    info *arg_info;
    bool ok;

    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module) || (NODE_TYPE (arg_node) == N_fundef),
                 "NTCdoNewTypeCheck() not called with N_module/N_fundef node!");

    if (N_module == NODE_TYPE (arg_node)) {

        if (!SSIassumptionSystemIsInitialized ()) {
            ok = SSIinitAssumptionSystem (SDhandleContradiction, SDhandleElimination);
            DBUG_ASSERT (ok, "Initialisation of Assumption System went wrong!");
        }

        SPECinitSpecChain ();

        /**
         * Before starting the type checking mechanism, we first mark all
         * wrapper functions as NTC_checked (as these have no bodies).
         * For all other functions, we rely on FUNDEF_TCSTAT being set
         * properly. This is done by the TBmakeFundef function and
         * the module system (for imported/used functions).
         */
        MODULE_FUNS (arg_node)
          = MFTdoMapFunTrav (MODULE_FUNS (arg_node), NULL, MarkWrapperAsChecked);

        /*
         * Now we have to initialize the deserialisation unit, as
         * specializations may add new functions as dependencies
         * of bodies to the ast
         */
        DSinitDeserialize (arg_node);

        TRAVpush (TR_ntc);

        arg_info = MakeInfo ();
        arg_node = TRAVdo (arg_node, arg_info);
        arg_info = FreeInfo (arg_info);

        TRAVpop ();

        /*
         * from here on, no more functions are deserialized, so we can
         * finish the deseralization engine
         */
        DSfinishDeserialize (arg_node);
    } else {
        arg_node = NTCdoNewTypeCheckOneFunction (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCdoNewReTypeCheck( node *arg_node)
 *
 * description:
 *    starts the new type checking traversal and rechecks the entire tree.
 *
 ******************************************************************************/

static node *
ResetTCstatus (node *fundef, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (fundef) != NULL) {
        FUNDEF_TCSTAT (fundef) = NTC_not_checked;
    }

    DBUG_RETURN (fundef);
}

node *
NTCdoNewReTypeCheck (node *arg_node)
{
    info *arg_info;
    int oldmaxspec;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "NTCdoNewReTypeCheck() not called with N_module node!");

    /*
     * mark all functions that can be rechecked as not checked (this
     * excludes wrapper functions, as these cannot be typechecked due
     * to the semantics of F_dispatch_error!)
     */
    MODULE_FUNS (arg_node)
      = MFTdoMapFunTrav (MODULE_FUNS (arg_node), NULL, ResetTCstatus);

    /*
     * De-activate specialising
     */
    oldmaxspec = global.maxspec;
    global.maxspec = 0;

    TRAVpush (TR_ntc);

    arg_info = MakeInfo ();
    arg_node = TRAVdo (arg_node, arg_info);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    global.maxspec = oldmaxspec;

    DBUG_RETURN (arg_node);
}

static node *
ResetLacTypes (node *fundef, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISLACFUN (fundef)) {
        FUNDEF_ARGS (fundef) = TUargtypes2unknownAUD (FUNDEF_ARGS (fundef));
        FUNDEF_RETS (fundef) = TUrettypes2unknownAUD (FUNDEF_RETS (fundef));
    }

    DBUG_RETURN (fundef);
}

static node *
ResetWrapperTypes (node *fundef, info *arg_info)
{
    ntype *type;
    node *impl;

    DBUG_ENTER ();

    if (FUNDEF_ISWRAPPERFUN (fundef) && (FUNDEF_BODY (fundef) != NULL)) {
        type = FUNDEF_WRAPPERTYPE (fundef);

        DBUG_PRINT ("resetting wrapper types for %s", CTIitemName (fundef));
        if (TYisFun (type)) {
            FUNDEF_WRAPPERTYPE (fundef) = TUrebuildWrapperTypeAlpha (type);
            FUNDEF_RETS (fundef) = TUrettypes2alphaAUDMax (FUNDEF_RETS (fundef));
        } else {
            impl = FUNDEF_IMPL (fundef);
            if (FUNDEF_BODY (impl) != NULL) {
                FUNDEF_RETS (impl) = TUrettypes2alphaAUDMax (FUNDEF_RETS (impl));
            } else {
                FUNDEF_RETS (impl) = TUrettypes2alphaFix (FUNDEF_RETS (impl));
            }
            FUNDEF_WRAPPERTYPE (fundef) = TUmakeProductTypeFromRets (FUNDEF_RETS (impl));
        }

        type = TYfreeType (type);
    }

    DBUG_RETURN (fundef);
}

static node *
ResetIsolatedFunTypes (node *fundef, info *arg_info)
{
    DBUG_ENTER ();

    if (!FUNDEF_ISWRAPPERFUN (fundef) && (FUNDEF_BODY (fundef) != NULL)) {
        /*
         * we do not need to exclude the instances that have been done
         * during the handling of wrappers as TUrettypes2alphaAUDMax
         * does not modify existing alphas!
         */
        DBUG_PRINT ("resetting return types for %s", CTIitemName (fundef));
        FUNDEF_RETS (fundef) = TUrettypes2alphaAUDMax (FUNDEF_RETS (fundef));
    }

    DBUG_RETURN (fundef);
}


node *
NTCdoNewReTypeCheckFromScratch (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "NTCdoNewReTypeCheckFromScratch() not called with "
                 "N_module node!");

    MODULE_FUNS (arg_node)
      = MFTdoMapFunTrav (MODULE_FUNS (arg_node), NULL, ResetLacTypes);

    /*
     * open up all wrapper types
     * this implicitly opens up the return types of all instances of
     * that wrapper as well!
     */
    MODULE_FUNS (arg_node)
      = MFTdoMapFunTrav (MODULE_FUNS (arg_node), NULL, ResetWrapperTypes);

    /*
     * finally, we need to open up the return types of all dispatched
     * functions whose wrappers have been elided already! Otherwise,
     * these never get improved anymore!
     */
    MODULE_FUNS (arg_node)
      = MFTdoMapFunTrav (MODULE_FUNS (arg_node), NULL, ResetIsolatedFunTypes);

    arg_node = NTCdoNewReTypeCheck (arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCNewTypeCheck_Expr( node *arg_node)
 *
 * description:
 *    Infers the type of an expression and fixes/eliminates alpha types.
 *    This function should be used *after* the regular type check phase only!
 *
 ******************************************************************************/

ntype *
NTCnewTypeCheck_Expr (node *arg_node)
{
    info *arg_info;
    ntype *type;

    DBUG_ENTER ();

    TRAVpush (TR_ntc);

    arg_info = MakeInfo ();
    arg_node = TRAVdo (arg_node, arg_info);
    type = INFO_TYPE (arg_info);
    type = TYfixAndEliminateAlpha (type);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    DBUG_RETURN (type);
}

/* @} */

/**
 *
 * @name Local helper functions:
 *
 * @{
 */

/******************************************************************************
 *
 * function:
 *    node *TypeCheckFunctionBody( node *fundef, info *arg_info)
 *
 * description:
 *    main function for type checking a given fundef node.
 *
 ******************************************************************************/

static node *
TypeCheckFunctionBody (node *fundef, info *arg_info)
{
    ntype *spec_type, *inf_type;
    ntype *stype, *itype;
    size_t i, inf_n, spec_n;
    bool ok;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
    node *arg;
#endif

    DBUG_ENTER ();

    FUNDEF_TCSTAT (fundef) = NTC_checking;

    DBUG_PRINT ("type checking function \"%s\" with", CTIitemName (fundef));

    /**
     * First, we have to ensure that ALL return types are in fact type vars.
     * Normal functions have been adjusted by create_wrappers.
     * LaC funs have been introduced by lac2fun with return types unknown[*].
     * These have to be replaced by PURE type vars, i.e., without any upper
     * limit!
     */
    if (FUNDEF_ISLACFUN (fundef)) {
        FUNDEF_RETS (fundef) = TUrettypes2alphaMax (FUNDEF_RETS (fundef));
    }

    DBUG_EXECUTE (arg = FUNDEF_ARGS (fundef); while (arg != NULL) {
        tmp_str = TYtype2String (AVIS_TYPE (ARG_AVIS (arg)), FALSE, 0);
        DBUG_PRINT ("  -> argument type: %s", tmp_str);
        tmp_str = MEMfree (tmp_str);
        arg = ARG_NEXT (arg);
    } tmp_str = TYtype2String (TUmakeProductTypeFromRets (FUNDEF_RETS (fundef)), FALSE,
                                              0);
                  DBUG_PRINT ("  -> return type %s", tmp_str);
                  tmp_str = MEMfree (tmp_str));

    /*
     * Then, we infer the type of the body:
     */
    if (NULL != FUNDEF_BODY (fundef)) {
        FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), arg_info);

        /*
         * A pointer to the return node is available in INFO_RETURN( arg_info)
         * now (cf. NTCreturn).
         */

        FUNDEF_RETURN (fundef) = INFO_RETURN (arg_info);
        INFO_RETURN (arg_info) = NULL;

    } else {
        DBUG_ASSERT (FUNDEF_ISEXTERN (fundef),
                     "non external function with NULL body found"
                     " but not expected here!");

        /*
         * We simply accept the type found in the external. declaration here:
         */
        INFO_TYPE (arg_info) = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));

        DBUG_PRINT ("trusting imported return type");
    }

    /*
     * The inferred result type is now available in INFO_TYPE( arg_info).
     * Iff legal, we insert it into the specified result type.
     */

    inf_type = INFO_TYPE (arg_info);

    if (!inf_type) {
        CTIabort (NODE_LOCATION (fundef),
                      "Could not infer the return type of function \"%s\".",
                      FUNDEF_NAME (fundef));
    }

    inf_n = TYgetProductSize (inf_type);

    DBUG_EXECUTE (tmp_str = TYtype2String (inf_type, FALSE, 0));
    DBUG_PRINT ("Function %s: inferred return type of \"%s\" is %s", FUNDEF_NAME (fundef),
                CTIitemName (fundef), tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    spec_type = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));
    spec_n = TYgetProductSize (spec_type);

    if ((spec_n > inf_n) || ((spec_n < inf_n) && !FUNDEF_HASDOTRETS (fundef))) {
        CTIabort (NODE_LOCATION (fundef),
                      "Number of return expressions in function \"%s\" does not match"
                      " the number of return types specified",
                      FUNDEF_NAME (fundef));
    }

    for (i = 0; i < TYgetProductSize (spec_type); i++) {
        stype = TYgetProductMember (spec_type, i);
        itype = TYgetProductMember (inf_type, i);

        ok = SSInewTypeRel (itype, stype);

        if (!ok) {
            CTIabort (NODE_LOCATION (fundef),
                          "Function %s: Component #%zu of inferred return type (%s) is "
                          "not within %s",
                          FUNDEF_NAME (fundef), i, TYtype2String (itype, FALSE, 0),
                          TYtype2String (stype, FALSE, 0));
        }
        /**
         * Now, we check whether we could infer at least one approximation for each
         * return value of a function. However, we have to make sure that this is a
         * top-level tc-run, since runs that are triggered during tc of other functions
         * may lack approximations due to mutual recursion.
         *
         * Example:
         *
         * bool foo( bool a)
         * {
         *   if(  a) {
         *     res = foo( goo( a));
         *   } else {
         *     res = false;
         *   }
         *   return( res);
         * }
         *
         * bool goo( bool a)
         * {
         *   res = foo( _not_(a));
         *   return( res);
         * }
         *
         * When TC goo during TC foo, we will not get an approximation yet, as the
         * else branch in foo has not yet been seen.
         *
         * we are in fact in the top-level function iff (global.act_info_chn == NULL)
         * holds!
         */
        if ((global.act_info_chn == NULL) && TYisAlpha (stype)
            && (SSIgetMin (TYgetAlpha (stype)) == NULL)) {
            CTIabort (NODE_LOCATION (fundef),
                          "Function %s: Component #%zu of inferred return type (%s) has "
                          "no lower bound;"
                          " an application of \"%s\" will not terminate",
                          FUNDEF_NAME (fundef), i, TYtype2String (stype, FALSE, 0),
                          FUNDEF_NAME (fundef));
        }
    }
    TYfreeType (inf_type);
    INFO_TYPE (arg_info) = NULL;

    DBUG_EXECUTE (tmp_str = TYtype2String (spec_type, FALSE, 0));
    DBUG_PRINT ("final return type of \"%s\" is: %s", CTIitemName (fundef), tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    /* now the functions is entirely typechecked, so we mark it as checked */
    FUNDEF_TCSTAT (fundef) = NTC_checked;

    DBUG_RETURN (fundef);
}

/* @} */

/**
 *
 * @name Traversal functions for the type inference system:
 *
 * @{
 */

/******************************************************************************
 *
 * function:
 *    node *NTCmodule(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCfundef(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCfundef (node *arg_node, info *arg_info)
{
    node *specialized_fundefs;
    node *copied_special_fundefs;
    node *integrated_fundefs;

    DBUG_ENTER ();

    if ((FUNDEF_TCSTAT (arg_node) == NTC_not_checked) && !FUNDEF_ISLACFUN (arg_node)) {
        /**
         * we are checking a new function; therefore, the actual
         * info chain (kept for extended error messages only) is reset:
         */
        global.act_info_chn = NULL;
        DBUG_PRINT_TAG ("NTC_INFOCHN",
                        "global.act_info_chn reset to NULL for function %s",
                        FUNDEF_NAME (arg_node));

        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        arg_node = TypeCheckFunctionBody (arg_node, arg_info);
    }

    if (INFO_IS_TYPE_UPGRADE (arg_info)) {
        /*
         * do nothing
         *
         * The traversal of functions is performed outside the type checker
         * in the opt cycle and we definitely have no specialised functions
         * and we certainly do not want to extract lac funs hooked by DupTree
         * since they would be lost immediately in outer space.
         */
    } else {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        if (NULL == FUNDEF_NEXT (arg_node)) {
            specialized_fundefs = SPECresetSpecChain ();
            copied_special_fundefs = DUPgetCopiedSpecialFundefs ();
            integrated_fundefs
              = TCappendFundef (specialized_fundefs, copied_special_fundefs);

            if (integrated_fundefs != NULL) {
                FUNDEF_NEXT (arg_node) = TRAVdo (integrated_fundefs, arg_info);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * NTCblock( node *arg_node, info *arg_info )
 *
 * description:
 *
 ******************************************************************************/

node *
NTCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * NTCvardec( node *arg_node, info *arg_info )
 *
 * description:
 *  existing types are converted into type vars with upper bounds!
 *
 ******************************************************************************/

node *
NTCvardec (node *arg_node, info *arg_info)
{
    node *avis;
    ntype *type;

#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    avis = VARDEC_AVIS (arg_node);
    type = AVIS_TYPE (avis);

    if (type != NULL) {
        /**
         * this means that the vardec has been created
         *  (a) through a variable declaration   or
         *  (b) by duplicating a type-checked function   or
         *  (c) by an earlier run of the type-checker
         * In ALL these cases, we want to make sure that only more specific
         * types can be inferred!
         * While in (a) and (c) the type is fixed, in (b) it may be an alpha!
         *     TUtype2alphaMax( type);
         * would do that job. However, this precludes us from recognizing
         * uses of non-defined vars when running the first time!
         * Hence, we just eliminate the existing types and start from scratch.
         * The desired subtyping issue is (at least in case of a)!) achieved
         * by the explicit insertion of _type_conv_ operations in phase 5.
         */
        DBUG_EXECUTE (tmp_str = TYtype2String (type, FALSE, 0));
        DBUG_PRINT ("eliminating type declaration %s %s", tmp_str, AVIS_NAME (avis));
        DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));
        AVIS_TYPE (avis) = NULL;
        type = TYfreeType (type);
    }

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCassign(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTCassign (node *arg_node, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ();

    tmp = INFO_LAST_ASSIGN (arg_info);

    INFO_LAST_ASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    INFO_LAST_ASSIGN (arg_info) = tmp;

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCcond(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTCcond (node *arg_node, info *arg_info)
{
    ntype *args;
    ntype *res;
    te_info *info;

    DBUG_ENTER ();

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    args = TYmakeProductType (1, INFO_TYPE (arg_info));

    info = TEmakeInfo (global.linenum, global.filename, TE_cond, "predicate");

    res = NTCCTcomputeType (NTCCTcond, info, args);

    args = TYfreeType (args);
    res = TYfreeType (res);

    DBUG_PRINT ("traversing then branch...");
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    DBUG_PRINT ("traversing else branch...");
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCfuncond(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTCfuncond (node *arg_node, info *arg_info)
{
    ntype *pred, *rhs1, *rhs2;
    ntype *args, *res;
    te_info *info;

    DBUG_ENTER ();

    FUNCOND_IF (arg_node) = TRAVdo (FUNCOND_IF (arg_node), arg_info);
    pred = INFO_TYPE (arg_info);
    INFO_TYPE (arg_info) = NULL;

    FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
    rhs1 = INFO_TYPE (arg_info);
    INFO_TYPE (arg_info) = NULL;

    FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);
    rhs2 = INFO_TYPE (arg_info);
    INFO_TYPE (arg_info) = NULL;

    args = TYmakeProductType (3, pred, rhs1, rhs2);

    info = TEmakeInfo (global.linenum, global.filename, TE_funcond, "conditional");

    /**
     * Here, we need to be able to approximate the result type from
     * less that ALL argument types, as we may deal with recursion
     * which means we need to get an approximation from ONE branch
     * of a conditional to be able to get one for the other branch.
     * This is achieved by using
     *    NTCCTcomputeTypeNonStrict   instead of
     *    NTCCTcomputeType:
     */
    res = NTCCTcomputeTypeNonStrict (NTCCTfuncond, info, args);

    args = TYfreeType (args);

    INFO_TYPE (arg_info) = TYgetProductMember (res, 0);
    res = TYfreeTypeConstructor (res);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTClet(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTClet (node *arg_node, info *arg_info)
{
    ntype *rhs_type, *existing_type, *declared_type, *inferred_type, *max;
    node *lhs;
    size_t i;
    bool ok;

#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    /*
     * Infer the RHS type :
     */
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    rhs_type = INFO_TYPE (arg_info);
    INFO_TYPE (arg_info) = NULL;

    /**
     * attach the RHS type(s) to the var(s) on the LHS:
     *
     * However, the LHS may have a type already. This can be due to
     * a) a vardec
     * b) an funcond node that combines this var with another one in another
     *    branch of a conditional!
     * In both cases, we add the RHS type as a new <= constraint!
     */
    lhs = LET_IDS (arg_node);

    if ((NODE_TYPE (LET_EXPR (arg_node)) == N_ap)
        || (NODE_TYPE (LET_EXPR (arg_node)) == N_prf)
        || (NODE_TYPE (LET_EXPR (arg_node)) == N_with)) {
        if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
            DBUG_ASSERT (TCcountIds (lhs) >= TYgetProductSize (rhs_type),
                         "fun ap yields more return values  than lhs vars available!");
        } else if (NODE_TYPE (LET_EXPR (arg_node)) == N_prf) {
            if ((PRF_PRF (LET_EXPR (arg_node)) != F_type_error)
                && (TCcountIds (lhs) != TYgetProductSize (rhs_type))) {
                CTIabort (LINE_TO_LOC (global.linenum),
                              "%s yields %zu instead of %zu return value(s)",
                              global.prf_name[PRF_PRF (LET_EXPR (arg_node))],
                              TYgetProductSize (rhs_type), TCcountIds (lhs));
            }
        } else {
            if (TCcountIds (lhs) != TYgetProductSize (rhs_type)) {
                CTIabort (LINE_TO_LOC (global.linenum),
                              "with loop returns %zu value(s)"
                              " but %zu variable(s) specified on the lhs",
                              TYgetProductSize (rhs_type), TCcountIds (lhs));
            }
        }
        i = 0;
        while (lhs) {
            existing_type = AVIS_TYPE (IDS_AVIS (lhs));
            declared_type = AVIS_DECLTYPE (IDS_AVIS (lhs));
            if (i < TYgetProductSize (rhs_type)) {

                inferred_type = TYgetProductMember (rhs_type, i);

                if (existing_type == NULL) {
                    AVIS_TYPE (IDS_AVIS (lhs)) = inferred_type;
                } else {
                    /* A likely cause of failure in next line is multiple
                     * assigns of LHS (This is SSA, remember.)
                     */
                    DBUG_ASSERT (TYisAlpha (existing_type),
                                 "non-alpha type for LHS found!");
                    ok = SSInewTypeRel (inferred_type, existing_type);
                    if (!ok) {
                        CTIabort (NODE_LOCATION (arg_node),
                                      "Component #%zu of inferred RHS type (%s) does not "
                                      "match %s",
                                      i, TYtype2String (inferred_type, FALSE, 0),
                                      TYtype2String (existing_type, FALSE, 0));
                    }
                }

                DBUG_EXECUTE (tmp_str
                              = TYtype2String (AVIS_TYPE (IDS_AVIS (lhs)), FALSE, 0));
                DBUG_PRINT ("  type of \"%s\" is %s", IDS_NAME (lhs), tmp_str);
                DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));
            } else {
                if (existing_type == NULL) {
                    if (declared_type == NULL) {
                        CTIabort (LINE_TO_LOC (global.linenum),
                                      "Cannot infer type of \"%s\" as it corresponds to "
                                      "\"...\" "
                                      "return type -- missing type declaration",
                                      IDS_NAME (lhs));
                    } else {
                        /**
                         * ' commented out the following warning as it was issued to often
                        with
                         * StdIO; left it here as I do not know whether a warning in
                        principle
                         * would be the better way to go for anyways....
                         *
                        CTIwarnLine( global.linenum,
                                     "Cannot infer type of \"%s\" as it corresponds to
                        \"...\" " "return type -- relying on type declaration", IDS_NAME(
                        lhs));
                         */

                        /*
                         * we use the declared type...
                         */
                        inferred_type = TYmakeAlphaType (TYcopyType (declared_type));
                        ok = SSInewMin (TYgetAlpha (inferred_type),
                                        TYcopyType (declared_type));
                        AVIS_TYPE (IDS_AVIS (lhs)) = inferred_type;
                    }
                } else {
                    /*
                     * reuse the type we have infered earlier...
                     */
                    /* A likely cause of failure in next line is multiple
                     * assigns of LHS (This is SSA, remember.)
                     */
                    DBUG_ASSERT (TYisAlpha (existing_type),
                                 "non-alpha type for LHS found!");
                    max = SSIgetMax (TYgetAlpha (existing_type));
                    DBUG_ASSERT (max != NULL, "null max for LHS type found!");
                    ok = SSInewMin (TYgetAlpha (existing_type), TYcopyType (max));
                }
            }

            if ((NODE_TYPE (LET_EXPR (arg_node)) != N_prf)
                || (PRF_PRF (LET_EXPR (arg_node)) != F_type_error)) {
                i++;
            }
            lhs = IDS_NEXT (lhs);
        }
        TYfreeTypeConstructor (rhs_type);
    } else {

        /* lhs must be one ids only since rhs is not a function application! */
        if (TCcountIds (lhs) != 1) {
            CTIabort (LINE_TO_LOC (global.linenum),
                          "rhs yields one value, %zu vars specified on the lhs",
                          TCcountIds (lhs));
        }

        existing_type = AVIS_TYPE (IDS_AVIS (lhs));
        inferred_type = rhs_type;

        if (existing_type == NULL) {
            AVIS_TYPE (IDS_AVIS (lhs)) = inferred_type;
        } else {
            /* A likely cause of failure in next line is multiple
             * assigns of LHS (This is SSA, remember.)
             */
            DBUG_ASSERT (TYisAlpha (existing_type), "non-alpha type for LHS found!");
            ok = SSInewTypeRel (inferred_type, existing_type);
            if (!ok) {
                CTIabort (NODE_LOCATION (arg_node),
                              "Inferred RHS type (%s) does not match %s",
                              TYtype2String (inferred_type, FALSE, 0),
                              TYtype2String (existing_type, FALSE, 0));
            }
        }

        DBUG_EXECUTE (tmp_str = TYtype2String (AVIS_TYPE (IDS_AVIS (lhs)), FALSE, 0));
        DBUG_PRINT ("  type of \"%s\" is %s", IDS_NAME (lhs), tmp_str);
        DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *NTCreturn(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * First we collect the return types. NTCexprs puts them into a product type
     * which is expected in INFO_TYPE( arg_info) afterwards (cf. NTCfundef)!
     * INFO_NUM_EXPRS_SOFAR is used to count the number of exprs "on the fly"!
     */
    INFO_NUM_EXPRS_SOFAR (arg_info) = 0;

    if (RETURN_EXPRS (arg_node) == NULL) {
        /* we are dealing with a void function here! */
        INFO_TYPE (arg_info) = TYmakeProductType (0);
    } else {
        RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_ASSERT (TYisProd (INFO_TYPE (arg_info)),
                 "NTCexprs did not create a product type");

    /*
     * Finally, we send the return node back to the fundef to fill FUNDEF_RETURN
     * properly !! We use INFO_RETURN( arg_info) for this purpose.
     */

    INFO_RETURN (arg_info) = arg_node;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCap(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCap (node *arg_node, info *arg_info)
{
    ntype *args, *res;
    node *wrapper;
    te_info *old_info_chn;
    ct_funptr ntc_fun;

    DBUG_ENTER ();

    /*
     * First we collect the argument types. NTCexprs puts them into a product type
     * which is expected in INFO_TYPE( arg_info) afterwards!
     * INFO_NUM_EXPRS_SOFAR is used to count the number of exprs "on the fly"!
     */
    INFO_NUM_EXPRS_SOFAR (arg_info) = 0;

    if (NULL != AP_ARGS (arg_node)) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    } else {
        INFO_TYPE (arg_info) = TYmakeProductType (0);
    }

    DBUG_ASSERT (TYisProd (INFO_TYPE (arg_info)),
                 "NTCexprs did not create a product type");

    args = INFO_TYPE (arg_info);
    INFO_TYPE (arg_info) = NULL;

    /**
     * Now, we investigate the pointer to the function definition:
     */
    wrapper = AP_FUNDEF (arg_node);

    if (!FUNDEF_ISWRAPPERFUN (wrapper) && !FUNDEF_ISLACFUN (wrapper)) {
        /**
         * the fun call has been dispatched already!
         * (i.e., we are in type-upgrade)
         * Essentially, we need to get the existing return type only!
         * This is done by NTCCTdispatched_udf.
         * However, in case of bottom argument types we need to propagate these!
         * This is done in NTCCTcomputeType.
         */
        ntc_fun = NTCCTudfDispatched;

    } else {
        /**
         * the fun call has not yet been dispatched; we may have to specialize!
         * This is done in NTCCTudf.
         * However, in case of bottom argument types we need to propagate these!
         * This is done in NTCCTcomputeType.
         */
        ntc_fun = NTCCTudf;
    }

    old_info_chn = global.act_info_chn;
    global.act_info_chn
      = TEmakeInfoUdf (global.linenum, global.filename, TE_udf,
                       NSgetName (FUNDEF_NS (wrapper)), FUNDEF_NAME (wrapper), wrapper,
                       INFO_LAST_ASSIGN (arg_info), global.act_info_chn);
    DBUG_PRINT_TAG ("TEINFO", "TE info %p created for udf ap %p",
                    (void *)global.act_info_chn,
                    (void *)arg_node);
    res = NTCCTcomputeType (ntc_fun, global.act_info_chn, args);

    global.act_info_chn = old_info_chn;
    DBUG_PRINT_TAG ("NTC_INFOCHN", "global.act_info_chn set back to %p",
                    (void *)old_info_chn);

    TYfreeType (args);
    INFO_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCprf(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCprf (node *arg_node, info *arg_info)
{
    ntype *args, *res;
    node *argexprs;
    size_t pos;
    prf prf;
    te_info *info;
    ntype *alpha, *def_obj;

    DBUG_ENTER ();

    prf = PRF_PRF (arg_node);

    if (prf == F_accu) {
        if (INFO_EXP_ACCU (arg_info) != NULL) {
            /**
             * we are dealing with a non-first partition of a MG-WL here!
             */
            res = TYcopyType (INFO_EXP_ACCU (arg_info));
        } else {
            argexprs = PRF_EXPRS2 (arg_node); /* skip iv! */
            pos = 0;

            res = TYmakeEmptyProductType (TCcountExprs (argexprs));

            while (argexprs != NULL) {
                alpha = TYmakeAlphaType (NULL);
                res = TYsetProductMember (res, pos, alpha);
                pos++;
                argexprs = EXPRS_NEXT (argexprs);
            }

            INFO_EXP_ACCU (arg_info) = TYcopyType (res);
        }
    } else if (prf == F_prop_obj_in) {
        if (INFO_PROP_OBJS (arg_info) != NULL) {
            /**
             * we are dealing with a non-first partition of a MG-WL here!
             */
            res = TYcopyType (INFO_PROP_OBJS (arg_info));
        } else {
            argexprs = PRF_EXPRS2 (arg_node); /* skip iv! */
            pos = 0;

            res = TYmakeEmptyProductType (TCcountExprs (argexprs));

            while (argexprs != NULL) {
                alpha = TYmakeAlphaType (NULL);
                def_obj = AVIS_TYPE (ID_AVIS (EXPRS_EXPR (argexprs)));
                SSInewTypeRel (def_obj, alpha);
                res = TYsetProductMember (res, pos, alpha);
                pos++;
                argexprs = EXPRS_NEXT (argexprs);
            }

            INFO_PROP_OBJS (arg_info) = TYcopyType (res);
        }
    } else {
        /*
         * First we collect the argument types. NTCexprs puts them into a product type
         * which is expected in INFO_TYPE( arg_info) afterwards!
         * INFO_NUM_EXPRS_SOFAR is used to count the number of exprs "on the fly"!
         */
        INFO_NUM_EXPRS_SOFAR (arg_info) = 0;

        if (NULL != PRF_ARGS (arg_node)) {
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        } else {
            INFO_TYPE (arg_info) = TYmakeProductType (0);
        }

        DBUG_ASSERT (TYisProd (INFO_TYPE (arg_info)),
                     "NTCexprs did not create a product type");

        args = INFO_TYPE (arg_info);
        INFO_TYPE (arg_info) = NULL;

        info = TEmakeInfoPrf (global.linenum, global.filename, TE_prf,
                              global.prf_name[prf], prf, prf_te_funtab[prf](args));
        res = NTCCTcomputeType (prf_tc_funtab[prf], info, args);
        TYfreeType (args);
    }
    INFO_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCexprs(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCexprs (node *arg_node, info *arg_info)
{
    ntype *type = NULL;

    DBUG_ENTER ();

    if (NULL != EXPRS_EXPR (arg_node)) {
        EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);
        type = INFO_TYPE (arg_info);
        INFO_TYPE (arg_info) = NULL;
    }
    INFO_NUM_EXPRS_SOFAR (arg_info)++;

    if (NULL != EXPRS_NEXT (arg_node)) {
        EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
    } else {
        INFO_TYPE (arg_info) = TYmakeEmptyProductType (INFO_NUM_EXPRS_SOFAR (arg_info));
    }

    INFO_NUM_EXPRS_SOFAR (arg_info)--;
    INFO_TYPE (arg_info)
      = TYsetProductMember (INFO_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info), type);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCarray(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NTCarray (node *arg_node, info *arg_info)
{
    size_t old_num_exprs;
    ntype *type, *elems;
    te_info *info;
#ifndef DBUG_OFF
    char *tmp_str1 = NULL, *tmp_str2 = NULL;
#endif

    DBUG_ENTER ();

    if (NULL != ARRAY_AELEMS (arg_node)) {
        /*
         * First we collect the element types. NTCexprs puts them into a product
         * type which is expected in INFO_TYPE( arg_info) afterwards!
         * INFO_NUM_EXPRS_SOFAR is used to count the number of exprs
         * "on the fly"!
         *
         * ATTENTION!!
         * We need to have the ARRAY_FRAMESHAPE in order to compute proper result
         * types!
         * In the initial type check, this is always an int[n] shape which means
         * that it can be ignored. However, later, i.e., in TUP, this
         * information may have changed (compare bug 111!).
         * To cope with that situation properly, we add an artificial type
         *   int[ARRAY_FRAMESHAPE]  which in NTCCTprf_array is combined with
         * the element type by TYnestTypes.
         */
        /**
         * INFO_NUM_EXPRS_SOFAR needs to be stacked here, as N_arrays may
         * appear in N_genarray sons. As the M-OP-WLs use this field as well
         * for counting the number of ops encountered, we need to stack them
         * (cf. bug310)
         */
        old_num_exprs = INFO_NUM_EXPRS_SOFAR (arg_info);
        INFO_NUM_EXPRS_SOFAR (arg_info) = 1;

        ARRAY_AELEMS (arg_node) = TRAVdo (ARRAY_AELEMS (arg_node), arg_info);

        DBUG_ASSERT (TYisProd (INFO_TYPE (arg_info)),
                     "NTCexprs did not create a product type");

        /**
         * Now, we create the type    int[ARRAY_FRAMESHAPE]:
         */
        INFO_NUM_EXPRS_SOFAR (arg_info)--;
        INFO_TYPE (arg_info)
          = TYsetProductMember (INFO_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info),
                                TYmakeAKS (TYmakeSimpleType (T_int),
                                           SHcopyShape (ARRAY_FRAMESHAPE (arg_node))));
        elems = INFO_TYPE (arg_info);
        INFO_TYPE (arg_info) = NULL;

        INFO_NUM_EXPRS_SOFAR (arg_info) = old_num_exprs;

        /**
         * Now, we built the resulting (AKS-)type type from the product type found:
         */
        info = TEmakeInfoPrf (global.linenum, global.filename, TE_prf,
                              "array-constructor", (prf)0, 1);

        type = NTCCTcomputeType (NTCCTprf_array, info, elems);

        TYfreeType (elems);

    } else {
        ntype *scalar;

        /**
         * we are dealing with an empty array here!
         * so we use the base element information from ARRAY_ELEMTYPE
         * to construct the type.
         */
        DBUG_ASSERT (TYisArray (ARRAY_ELEMTYPE (arg_node)),
                     "found non-array type as elemtype!");

        scalar = TYgetScalar (ARRAY_ELEMTYPE (arg_node));

        DBUG_ASSERT (TUshapeKnown (ARRAY_ELEMTYPE (arg_node)),
                     "found an array constructor for an empty array with non "
                     "AKS element type!");

        /*
         * the the time being, we only build AKV empty arrays
         * for user defined types!
         */
        DBUG_EXECUTE (tmp_str1 = SHshape2String (0, ARRAY_FRAMESHAPE (arg_node));
                      tmp_str2 = TYtype2String (ARRAY_ELEMTYPE (arg_node), FALSE, 0));
        DBUG_PRINT ("computing type of empty array-constructor with outer "
                    "shape %s and element type %s",
                    tmp_str1, tmp_str2);
        DBUG_EXECUTE (tmp_str1 = MEMfree (tmp_str1); tmp_str2 = MEMfree (tmp_str2));

        if (TYisSimple (scalar)) {
            type = TYmakeAKV (TYcopyType (scalar),
                              COmakeConstant (TYgetSimpleType (scalar),
                                              SHappendShapes (ARRAY_FRAMESHAPE (arg_node),
                                                              TYgetShape (ARRAY_ELEMTYPE (
                                                                arg_node))),
                                              NULL));
        } else {
            type = TYmakeAKS (TYcopyType (scalar),
                              SHappendShapes (ARRAY_FRAMESHAPE (arg_node),
                                              TYgetShape (ARRAY_ELEMTYPE (arg_node))));
        }

        type = TYmakeProductType (1, type);

        DBUG_EXECUTE (tmp_str1 = TYtype2String (type, FALSE, 0));
        DBUG_PRINT ("yields %s", tmp_str1);
        DBUG_EXECUTE (tmp_str1 = MEMfree (tmp_str1));
    }

    INFO_TYPE (arg_info) = TYgetProductMember (type, 0);
    TYfreeTypeConstructor (type);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCid( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
NTCid (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ();

    type = AVIS_TYPE (ID_AVIS (arg_node));

    if (type == NULL) {
        CTIabort (NODE_LOCATION (arg_node),
                      "Cannot infer type for %s as it may be"
                      " used without a previous definition",
                      ID_NAME (arg_node));
    } else {
        INFO_TYPE (arg_info) = TYcopyType (type);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCglobobj( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NTCglobobj (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ();

    type = OBJDEF_TYPE (GLOBOBJ_OBJDEF (arg_node));

    DBUG_ASSERT (type != NULL, "N_objdef wo type found in NTCglobobj");

    INFO_TYPE (arg_info) = TYcopyType (type);

    DBUG_RETURN (arg_node);
}

node *
NTCnested_init (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NTCnum( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

#define NTCBASIC(name, base)                                                             \
    node *NTC##name (node *arg_node, info *arg_info)                                     \
    {                                                                                    \
        constant *cv;                                                                    \
        DBUG_ENTER ();                                                                   \
                                                                                         \
        cv = COaST2Constant (arg_node);                                                  \
        if (cv == NULL) {                                                                \
            INFO_TYPE (arg_info)                                                         \
              = TYmakeAKS (TYmakeSimpleType (base), SHcreateShape (0));                  \
        } else {                                                                         \
            INFO_TYPE (arg_info) = TYmakeAKV (TYmakeSimpleType (base), cv);              \
        }                                                                                \
        DBUG_RETURN (arg_node);                                                          \
    }

NTCBASIC (num, T_int)
NTCBASIC (numbyte, T_byte)
NTCBASIC (numshort, T_short)
NTCBASIC (numint, T_int)
NTCBASIC (numlong, T_long)
NTCBASIC (numlonglong, T_longlong)
NTCBASIC (numubyte, T_ubyte)
NTCBASIC (numushort, T_ushort)
NTCBASIC (numuint, T_uint)
NTCBASIC (numulong, T_ulong)
NTCBASIC (numulonglong, T_ulonglong)
NTCBASIC (double, T_double)
NTCBASIC (float, T_float)
NTCBASIC (floatvec, T_floatvec)
NTCBASIC (char, T_char)
NTCBASIC (bool, T_bool)

/******************************************************************************
 *
 * function:
 *   node *NTCnodeToType( node *arg_node)
 *
 * description: Convert simple scalar node type to simpletype.
 *
 * Parameter: arg_node: an N_num, etc., node.
 *
 * Result: A T_int, etc., value.
 *
 ******************************************************************************/

simpletype
NTCnodeToType (node *arg_node)
{
    DBUG_ENTER ();
    simpletype z;

    switch (NODE_TYPE (arg_node)) {
    default:
        DBUG_UNREACHABLE ("Illegal node type");
    case N_bool:
        z = T_bool;
        break;
    case N_char:
        z = T_char;
        break;
    case N_double:
        z = T_double;
        break;
    case N_float:
        z = T_float;
        break;
    case N_floatvec:
        z = T_floatvec;
        break;
    case N_num:
        z = T_int;
        break;
    case N_numint:
        z = T_int;
        break;
    case N_numbyte:
        z = T_byte;
        break;
    case N_numshort:
        z = T_short;
        break;
    case N_numlong:
        z = T_long;
        break;
    case N_numlonglong:
        z = T_longlong;
        break;
    case N_numubyte:
        z = T_ubyte;
        break;
    case N_numushort:
        z = T_ushort;
        break;
    case N_numuint:
        z = T_uint;
        break;
    case N_numulong:
        z = T_ulong;
        break;
    case N_numulonglong:
        z = T_ulonglong;
        break;
    }
    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   node *NTCstr( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCstr (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    INFO_TYPE (arg_info) = TYmakeAUD (TYmakeSimpleType (T_unknown));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NTCtype( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCtype (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    INFO_TYPE (arg_info) = TYcopyType (TYPE_TYPE (arg_node));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NTCcast( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCcast (node *arg_node, info *arg_info)
{
    te_info *info;
    ntype *type, *cast_t, *expr_t;

    DBUG_ENTER ();

    CAST_EXPR (arg_node) = TRAVdo (CAST_EXPR (arg_node), arg_info);
    expr_t = INFO_TYPE (arg_info);
    if (TYisProd (expr_t)) {
        /*
         * The expression we are dealing with here is a function application.
         * Therefore, only a single return type is legal. This one is to be extracted!
         */
        if (TYgetProductSize (expr_t) != 1) {
            CTIabort (LINE_TO_LOC (global.linenum),
                          "Cast used for a function application with %zu return values",
                          TYgetProductSize (expr_t));
        } else {
            expr_t = TYgetProductMember (expr_t, 0);
        }
    }
    cast_t = CAST_NTYPE (arg_node);

    info
      = TEmakeInfoPrf (global.linenum, global.filename, TE_prf, "type-cast", (prf)0, 1);
    type = NTCCTcomputeType (NTCCTprf_cast, info, TYmakeProductType (2, cast_t, expr_t));

    INFO_TYPE (arg_info) = TYgetProductMember (type, 0);
    TYfreeTypeConstructor (type);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCwith( node *arg_node, info *arg_info)
 *
 * description:
 *   steers the type inference of with loops and dbug prints the individual
 *   findings of the generator inference, the body inference, and the
 *   composition of them which is done in NTCNwithop.
 *
 ******************************************************************************/

node *
NTCwith (node *arg_node, info *arg_info)
{
    ntype *gen, *body, *mem_outer_accu, *mem_outer_prop_objs;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    /*
     * First, we infer the generator type
     */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    gen = TYgetProductMember (INFO_TYPE (arg_info), 0);
    TYfreeTypeConstructor (INFO_TYPE (arg_info));
    INFO_TYPE (arg_info) = NULL;

    DBUG_EXECUTE (tmp_str = TYtype2String (gen, FALSE, 0));
    DBUG_PRINT ("  WL - generator type: %s", tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    /*
     * Then, we infer the type of the WL body:
     *
     * Since this may yield an explicit accu type in INFO_EXP_ACCU and
     * a new types for propagated objects in INFO_PROP_OBJS,
     * and fold-wls / propagate-wls may be nested, we need to stack these
     * pointers before traversing the code of this WL.
     */
    mem_outer_accu = INFO_EXP_ACCU (arg_info);
    INFO_EXP_ACCU (arg_info) = NULL;
    mem_outer_prop_objs = INFO_PROP_OBJS (arg_info);
    INFO_PROP_OBJS (arg_info) = NULL;

    /**
     * we need to communicate the withops to the code as multi
     * generator WLs need to be treted differently depending
     * on whether we are dealing with fold-withloops ( where
     * we do NOT require shape conformity) and the others.
     */
    INFO_WL_OPS (arg_info) = WITH_WITHOP (arg_node);

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    body = INFO_TYPE (arg_info);
    INFO_TYPE (arg_info) = NULL;

    DBUG_ASSERT (TYisProd (body), "non-product type received for the type of a WL body");
    /**
     * needs to be kept as product type as multi-operator WL requires more
     * than one return type!
     */

    DBUG_EXECUTE (tmp_str = TYtype2String (body, FALSE, 0));
    DBUG_PRINT ("  WL - body type: %s", tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    /*
     * Finally, we compute the return type from "gen" and "body".
     * This is done in NTCNwithop. The two types are transferred via
     * INFO_GEN_TYPE and INFO_BODIES_TYPE, respectively.
     * However, since we only need one body type per withop, we use the
     * field INFO_NUM_EXPRS_SOFAR( arg_info) to indicate the acual
     * component of interest in the product type "body".
     * We also may have to deal with propagate operations. Therefore,
     * we initilise INFO_ACT_PROP_OBJ.
     */
    INFO_GEN_TYPE (arg_info) = gen;
    INFO_BODIES_TYPE (arg_info) = body;
    INFO_NUM_EXPRS_SOFAR (arg_info) = 0;
    INFO_ACT_PROP_OBJ (arg_info) = 0;
    INFO_ACT_FOLD_POS (arg_info) = 0;

    if (TYgetProductSize (body) != TCcountWithops (WITH_WITHOP (arg_node))) {
        CTIabort (LINE_TO_LOC (global.linenum),
                      "Inconsistent with loop: %zu operator(s) "
                      "but %zu value(s) specified in the body",
                      TCcountWithops (WITH_WITHOP (arg_node)), TYgetProductSize (body));
    }
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_EXECUTE (tmp_str = TYtype2String (INFO_TYPE (arg_info), FALSE, 0));
    DBUG_PRINT ("  WL - final type: %s", tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    /**
     * eventually, we need to restore a potential outer accu / prop_objs
     * for fold-wls / propagate-wls:
     */
    INFO_EXP_ACCU (arg_info) = mem_outer_accu;
    INFO_PROP_OBJS (arg_info) = mem_outer_prop_objs;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCpart( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCpart (node *arg_node, info *arg_info)
{
    node *idxs;
    ntype *idx, *this_idx, *remaining_idx;
    te_info *info;
    size_t num_ids;

    DBUG_ENTER ();

    /*
     * First, we check whether we can extract some shape info from the
     * generator variable, i.e, we check whether we do have scalar indices:
     */
    idxs = PART_IDS (arg_node);
    if (idxs != NULL) {
        num_ids = TCcountIds (idxs);
        idx = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, num_ids));
    } else {
        idx = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHcreateShape (0));
    }

    /*
     * Then, we infer the best possible type of the generator specification
     * and from the idx information gained from the Nwithid node:
     */
    INFO_TYPE (arg_info) = TYmakeProductType (1, idx);
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    /*
     * the best possible generator type is returned, so
     * we attach the generator type to the generator variable(s).
     */
    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
    DBUG_ASSERT (INFO_TYPE (arg_info) != NULL,
                 "inferred generator type corrupted in NTCNwithid");

    /*
     * Now, we check the other parts:
     */

    if ((global.ssaiv) ||
        // Not always in ssalw mode &&
        ((global.optimize.dossawl) && (global.compiler_anyphase >= PH_opt_saacyc_ssawl)
         && (global.compiler_anyphase < PH_opt_saacyc_ussawl))) {
        if (PART_NEXT (arg_node) != NULL) {
            this_idx = TYgetProductMember (INFO_TYPE (arg_info), 0);
            INFO_TYPE (arg_info) = TYfreeTypeConstructor (INFO_TYPE (arg_info));

            PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
            remaining_idx = TYgetProductMember (INFO_TYPE (arg_info), 0);
            INFO_TYPE (arg_info) = TYfreeTypeConstructor (INFO_TYPE (arg_info));

            info
              = TEmakeInfo (global.linenum, global.filename, TE_with, "multi generator");
            idx = TYmakeProductType (2, this_idx, remaining_idx);
            INFO_TYPE (arg_info) = NTCCTcomputeType (NTCCTwl_multipart, info, idx);
            ;
        }
    }

    /*
     * AND (!!) we hand the type back to NTCNwith!
     */
    DBUG_ASSERT (INFO_TYPE (arg_info) != NULL,
                 "inferred generator type corrupted in multigenerator WL");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCgenerator( node *arg_node, info *arg_info)
 *
 * description:
 *   checks compatability of the generator entries, i.e.,
 *              bounds, step, and width vectors
 *   if they are potentially conformative the most specific type possible is
 *   returned.
 *
 ******************************************************************************/

node *
NTCgenerator (node *arg_node, info *arg_info)
{
    ntype *lb, *idx, *ub, *s, *w, *gen, *res;
    te_info *info;

    DBUG_ENTER ();

    idx = TYgetProductMember (INFO_TYPE (arg_info), 0); /* generated in NTCNpart !*/
    INFO_TYPE (arg_info) = TYfreeTypeConstructor (INFO_TYPE (arg_info));

    GENERATOR_BOUND1 (arg_node) = TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);
    lb = INFO_TYPE (arg_info);
    INFO_TYPE (arg_info) = NULL;

    GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);
    ub = INFO_TYPE (arg_info);
    INFO_TYPE (arg_info) = NULL;

    if (GENERATOR_STEP (arg_node) != NULL) {
        GENERATOR_STEP (arg_node) = TRAVdo (GENERATOR_STEP (arg_node), arg_info);
        s = INFO_TYPE (arg_info);
        INFO_TYPE (arg_info) = NULL;

        if (GENERATOR_WIDTH (arg_node) != NULL) {
            GENERATOR_WIDTH (arg_node) = TRAVdo (GENERATOR_WIDTH (arg_node), arg_info);
            w = INFO_TYPE (arg_info);
            INFO_TYPE (arg_info) = NULL;

            gen = TYmakeProductType (5, lb, idx, ub, s, w);
        } else {
            gen = TYmakeProductType (4, lb, idx, ub, s);
        }
    } else {
        gen = TYmakeProductType (3, lb, idx, ub);
    }

    info = TEmakeInfo (global.linenum, global.filename, TE_generator, "generator");
    res = NTCCTcomputeType (NTCCTwl_idx, info, gen);
    TYfreeType (gen);

    INFO_TYPE (arg_info) = res;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCwithid( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCwithid (node *arg_node, info *arg_info)
{
    node *idxs, *vec;

    DBUG_ENTER ();

    idxs = WITHID_IDS (arg_node);
    while (idxs) {
        /*
         * single genvars always have to be scalar int s
         */
        AVIS_TYPE (IDS_AVIS (idxs))
          = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0));
        idxs = IDS_NEXT (idxs);
    }

    vec = WITHID_VEC (arg_node);
    if (vec != NULL) {
        /*
         * we have to leave the generator type intact, therefore, we copy it:
         */
        AVIS_TYPE (IDS_AVIS (vec))
          = TYcopyType (TYgetProductMember (INFO_TYPE (arg_info), 0));
    }

    idxs = WITHID_IDXS (arg_node);
    while (idxs) {
        /*
         * single genvars always have to be scalar int s
         */
        AVIS_TYPE (IDS_AVIS (idxs))
          = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0));
        idxs = IDS_NEXT (idxs);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *NTCcode( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NTCcode (node *arg_node, info *arg_info)
{
    ntype *remaining_blocks, *this_block, *blocks, *res_i, *res;
    size_t num_ops, i;
    te_info *info;
    node *wl_ops;

    DBUG_ENTER ();

    wl_ops = INFO_WL_OPS (arg_info);
    INFO_WL_OPS (arg_info) = NULL;

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    /**
     * traverse into further code blocks iff existant
     */
    if (CODE_NEXT (arg_node) != NULL) {
        this_block = INFO_TYPE (arg_info);
        INFO_TYPE (arg_info) = NULL;

        INFO_WL_OPS (arg_info) = wl_ops;
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
        remaining_blocks = INFO_TYPE (arg_info);
        INFO_TYPE (arg_info) = NULL;

        num_ops = TYgetProductSize (this_block);
        DBUG_ASSERT (num_ops == TYgetProductSize (remaining_blocks),
                     "number of WL-body types varies within one multi-generator WL");
        res = TYmakeEmptyProductType (num_ops);

        for (i = 0; i < num_ops; i++) {
            info
              = TEmakeInfo (global.linenum, global.filename, TE_with, "multi generator");
            blocks = TYmakeProductType (2, TYgetProductMember (this_block, i),
                                        TYgetProductMember (remaining_blocks, i));
            DBUG_ASSERT (wl_ops != NULL,
                         "number of return values does not match withloop ops");
            if (NODE_TYPE (wl_ops) == N_fold) {
                res_i = NTCCTcomputeType (NTCCTwl_multifoldcode, info, blocks);
            } else {
                res_i = NTCCTcomputeType (NTCCTwl_multicode, info, blocks);
            }
            wl_ops = WITHOP_NEXT (wl_ops);
            TYsetProductMember (res, i, TYgetProductMember (res_i, 0));
            res_i = TYfreeTypeConstructor (res_i);
        }
        this_block = TYfreeTypeConstructor (this_block);
        remaining_blocks = TYfreeTypeConstructor (remaining_blocks);
        INFO_TYPE (arg_info) = res;
    }

    DBUG_RETURN (arg_node);
}

static node *
HandleMultiOperators (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (arg_node != NULL) {
        INFO_NUM_EXPRS_SOFAR (arg_info)++;
        arg_node = TRAVdo (arg_node, arg_info);
        INFO_NUM_EXPRS_SOFAR (arg_info)--;

    } else {
        INFO_TYPE (arg_info)
          = TYmakeEmptyProductType (INFO_NUM_EXPRS_SOFAR (arg_info) + 1);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCgenarray( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NTCgenarray (node *arg_node, info *arg_info)
{
    ntype *gen, *body, *res;
    ntype *shp, *dexpr, *args;
    te_info *info;

    DBUG_ENTER ();

    gen = INFO_GEN_TYPE (arg_info);
    body
      = TYgetProductMember (INFO_BODIES_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info));

    /*
     * First, we check the shape expression:
     */
    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    shp = INFO_TYPE (arg_info);
    INFO_TYPE (arg_info) = NULL;
    /*
     * Then, we check the shape of the default value, if available:
     */
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
        dexpr = INFO_TYPE (arg_info);
        INFO_TYPE (arg_info) = NULL;
    } else {
        dexpr = TYcopyType (body);
    }

    if (TYcountNoMinAlpha (body) > 0) {
        bool ok;
        DBUG_PRINT ("inserting type dependency in potentially non-terminating WL");
        ok = SSInewTypeRel (dexpr, body);
        DBUG_ASSERT (ok, "default elem type <= body type trick failed!");
    }
    args = TYmakeProductType (4, gen, shp, body, dexpr);
    info = TEmakeInfo (global.linenum, global.filename, TE_with, "genarray");

    res = NTCCTcomputeType (NTCCTwl_gen, info, args);

    GENARRAY_NEXT (arg_node) = HandleMultiOperators (GENARRAY_NEXT (arg_node), arg_info);

    TYsetProductMember (INFO_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info),
                        TYgetProductMember (res, 0));
    res = TYfreeTypeConstructor (res);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCmodarray( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NTCmodarray (node *arg_node, info *arg_info)
{
    ntype *gen, *body, *res;
    ntype *shp, *args;
    te_info *info;

    DBUG_ENTER ();

    gen = INFO_GEN_TYPE (arg_info);
    body
      = TYgetProductMember (INFO_BODIES_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info));

    /*
     * First, we check the array expression:
     */
    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);
    shp = INFO_TYPE (arg_info);
    INFO_TYPE (arg_info) = NULL;

    args = TYmakeProductType (3, gen, shp, body);
    info = TEmakeInfo (global.linenum, global.filename, TE_with, "modarray");
    res = NTCCTcomputeType (NTCCTwl_mod, info, args);

    MODARRAY_NEXT (arg_node) = HandleMultiOperators (MODARRAY_NEXT (arg_node), arg_info);

    TYsetProductMember (INFO_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info),
                        TYgetProductMember (res, 0));
    res = TYfreeTypeConstructor (res);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCfold( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NTCfold (node *arg_node, info *arg_info)
{
    ntype *gen, *body, *res, *elems, *acc;
    ntype *neutr, *args, *pargs;
    size_t i, num_pargs, exprs_so_far;
    node *wrapper;
    te_info *info;
    bool ok;

    DBUG_ENTER ();

    gen = INFO_GEN_TYPE (arg_info);

    body
      = TYgetProductMember (INFO_BODIES_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info));

    /**
     * we are dealing with a udf-fold-wl here!
     *
     * First, we check the neutral expression:
     */
    if (FOLD_NEUTRAL (arg_node) == NULL) {
        CTIabort (LINE_TO_LOC (global.linenum),
                      "Missing neutral element for user-defined fold function");
    }
    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);
    neutr = INFO_TYPE (arg_info);
    INFO_TYPE (arg_info) = NULL;

    /*
     * Then, we compute the type of the elements to be folded:
     */
    args = TYmakeProductType (3, gen, neutr, body);
    info = TEmakeInfo (global.linenum, global.filename, TE_with, "fold");
    res = NTCCTcomputeType (NTCCTwl_fold, info, args);
    elems = TYgetProductMember (res, 0);
    res = TYfreeTypeConstructor (res);

    /*
     * Followed by a computation of the type of the fold fun:
     *
     * Since the entire fold-wl is a recursive function that in each
     * iteration applies the foldfun, we need to establish a fix-point
     * iteration here (cf. bug no.18).
     * To do so, we introduce a type variable for the accumulated parameter
     * which has to be bigger than a) the element type and b) the result
     * type.
     * As we may be dealing with explicit accumulators, this type variable may
     * exist already. If this is the case, it is contained in INFO_EXP_ACCU
     * otherwise that field is NULL.
     */
    if (INFO_EXP_ACCU (arg_info) != NULL) {
        /**
         * As the accu is explicit, we have the following situation:
         *
         *   {
         *      a = accu( ...);
         *      e = ....;
         *      val = fun( p1, ..., pn, a, e);     [ N >= 0 ]
         *   } : val;
         * Therefore, it suffices to take the alpha type of 'a' (from
         * INFO_EXP_ACCU( arg_info)), make the neutral element a
         * subtype of it (which triggers the initial approximation for
         * fun( a, e) ), and then make the type of 'val' a subtype of
         * the alpha type again in order to ensure the fix-point calculation.
         */
        acc = TYcopyType (
                  TYgetProductMember (
                      INFO_EXP_ACCU (arg_info),
                      INFO_ACT_FOLD_POS (arg_info)));
        INFO_ACT_FOLD_POS (arg_info)++;

        res = TYmakeProductType (1, elems);

        ok = SSInewTypeRel (neutr, acc);
        DBUG_ASSERT (ok, "initialization of fold-fun in fold-wl went wrong");

        ok = SSInewTypeRel (elems, acc);

    } else {
        acc = TYmakeAlphaType (NULL);
        ok = SSInewTypeRel (neutr, acc);
        DBUG_ASSERT (ok, "initialization of fold-fun in fold-wl went wrong");

        /*
         * We have to create the arguments from potentially existing partial
         * arguments and then add the types of acc and elems:
         */
        if (FOLD_ARGS (arg_node) != NULL) {
            exprs_so_far = INFO_NUM_EXPRS_SOFAR (arg_info);
            INFO_NUM_EXPRS_SOFAR (arg_info) = 0;
            FOLD_ARGS (arg_node) = TRAVdo (FOLD_ARGS (arg_node), arg_info);
            pargs = INFO_TYPE (arg_info);
            INFO_TYPE (arg_info) = NULL;
            INFO_NUM_EXPRS_SOFAR (arg_info) = exprs_so_far;

            num_pargs = TYgetProductSize (pargs);
            args = TYmakeEmptyProductType (num_pargs + 2);
            for (i = 0; i < num_pargs; i++) {
                TYsetProductMember (args, i, TYgetProductMember (pargs, i));
            }
            TYsetProductMember (args, i++, acc);
            TYsetProductMember (args, i++, elems);

            pargs = TYfreeTypeConstructor (pargs);
        } else {
            args = TYmakeProductType (2, acc, elems);
        }

        wrapper = FOLD_FUNDEF (arg_node);
        info = TEmakeInfoUdf (global.linenum, global.filename, TE_foldf,
                              NSgetName (FUNDEF_NS (wrapper)), FUNDEF_NAME (wrapper),
                              wrapper, INFO_LAST_ASSIGN (arg_info), NULL);
        res = NTCCTcomputeType ((FUNDEF_ISWRAPPERFUN (wrapper) ? NTCCTudf
                                                               : NTCCTudfDispatched),
                                info, args);

        ok = SSInewTypeRel (TYgetProductMember (res, 0), acc);
    }
    if (!ok) {
        CTIabort (LINE_TO_LOC (global.linenum), "Illegal fold function in fold with loop");
    }

    FOLD_NEXT (arg_node) = HandleMultiOperators (FOLD_NEXT (arg_node), arg_info);

    TYsetProductMember (INFO_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info),
                        TYgetProductMember (res, 0));
    res = TYfreeTypeConstructor (res);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCbreak( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NTCbreak (node *arg_node, info *arg_info)
{
    ntype *body;

    DBUG_ENTER ();
    body
      = TYgetProductMember (INFO_BODIES_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info));

    BREAK_NEXT (arg_node) = HandleMultiOperators (BREAK_NEXT (arg_node), arg_info);

    TYsetProductMember (INFO_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info),
                        TYcopyType (body));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *NTCpropagate( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NTCpropagate (node *arg_node, info *arg_info)
{
    ntype *body, *prop_obj_type;
    bool ok;

    DBUG_ENTER ();

    body
      = TYgetProductMember (INFO_BODIES_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info));

    DBUG_ASSERT (INFO_PROP_OBJS (arg_info) != NULL,
                 "propagate WL found without F_prop_obj in any body");

    prop_obj_type
      = TYgetProductMember (INFO_PROP_OBJS (arg_info), INFO_ACT_PROP_OBJ (arg_info));
    INFO_ACT_PROP_OBJ (arg_info)++;

    ok = SSInewTypeRel (body, prop_obj_type);

    if (!ok) {
        CTIabort (LINE_TO_LOC (global.linenum),
                      "Illegal object transformation in propagate with loop"
                      " body yields %s, but %s is propagated",
                      TYtype2String (body, FALSE, 0),
                      TYtype2String (AVIS_TYPE (ID_AVIS (PROPAGATE_DEFAULT (arg_node))),
                                     FALSE, 0));
    }

    if (PROPAGATE_NEXT (arg_node) == NULL) {
        TYfreeType (INFO_PROP_OBJS (arg_info));
        INFO_PROP_OBJS (arg_info) = NULL;
    }

    PROPAGATE_NEXT (arg_node)
      = HandleMultiOperators (PROPAGATE_NEXT (arg_node), arg_info);

    TYsetProductMember (INFO_TYPE (arg_info), INFO_NUM_EXPRS_SOFAR (arg_info),
                        TYcopyType (body));

    DBUG_RETURN (arg_node);
}

/* @} */

/**
 *
 * name: Module internal entry function:
 *
 * @{
 */

/******************************************************************************
 *
 * function:
 *    node *NTCtriggerTypeCheck( node *fundef)
 *
 * description:
 *   external interface for TypeCheckFunctionBody. It is used from ct_fun,
 *   where the generation of new specializations and the type inference
 *   of all potential instances of a function application is triggered.
 *
 ******************************************************************************/

node *
NTCtriggerTypeCheck (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ();

    if (FUNDEF_TCSTAT (fundef) == NTC_not_checked) {
        arg_info = MakeInfo ();
        fundef = TypeCheckFunctionBody (fundef, arg_info);
        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (fundef);
}

/* @} */
/* @} */ /* defgroup ntc */

#undef DBUG_PREFIX
