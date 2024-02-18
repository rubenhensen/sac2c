#include "specialize.h"

#define DBUG_PREFIX "SPEC"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "free.h"
#include "ct_fun.h"
#include "create_wrappers.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "deserialize.h"
#include "add_function_body.h"
#include "new_types.h"
#include "type_utils.h"
#include "ssi.h"
#include "ctinfo.h"
#include "namespaces.h"
#include "map_call_graph.h"
#include "globals.h"
#include "traverse.h"
#include "phase.h"

/**
 *
 * @addtogroup ntc
 *
 * @{
 */

/**
 *
 * @file specialize.c
 *
 * This file provides all functions related to function specialization.
 */

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

static node *specialized_fundefs = NULL;

/******************************************************************************
 ***
 ***          local helper functions
 ***          ----------------------
 ***
 ******************************************************************************/

node *
InsertTypeConv (node *fundef, size_t pos_of_ret, ntype *spec_type)
{
    node *last_assign, *ret, *id, *avis, *new_avis;

    DBUG_ENTER ();

    last_assign = TCgetLastAssign (FUNDEF_ASSIGNS (fundef));

    DBUG_ASSERT ((last_assign != NULL)
                   && (NODE_TYPE (ASSIGN_STMT (last_assign)) == N_return),
                 "trying to insert type conv for return type into body "
                 "without return!");

    ret = ASSIGN_STMT (last_assign);
    id = TCgetNthExprsExpr (pos_of_ret, RETURN_EXPRS (ret));
    avis = ID_AVIS (id);

    DBUG_ASSERT (NODE_TYPE (id) == N_id, "non N_id node found in N_return");

    new_avis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)), TYcopyType (spec_type));
    ID_AVIS (id) = new_avis;

    /**
     * we are recycling the N_assign node here as it may either be hooked
     * up to an N_block or an N_assign node and we do not want to use
     * a traversal here!
     */

    ASSIGN_STMT (last_assign)
      = TBmakeLet (TBmakeIds (new_avis, NULL),
                   TCmakePrf2 (F_type_conv, TBmakeType (TYcopyType (spec_type)),
                               TBmakeId (avis)));
    ASSIGN_NEXT (last_assign) = TBmakeAssign (ret, NULL);

    if (PHisSAAMode ()) {
        AVIS_SSAASSIGN (new_avis) = last_assign;
    }
    FUNDEF_VARDECS (fundef) = TBmakeVardec (new_avis, FUNDEF_VARDECS (fundef));

    DBUG_RETURN (fundef);
}

node *
InsertHideInfo (node *fundef, size_t pos_of_ret, ntype *spec_type)
{
    node *last_assign, *ret, *id, *avis, *new_avis;

    DBUG_ENTER ();

    last_assign = TCgetLastAssign (FUNDEF_ASSIGNS (fundef));

    DBUG_ASSERT ((last_assign != NULL)
                   && (NODE_TYPE (ASSIGN_STMT (last_assign)) == N_return),
                 "trying to insert shape/dimension hiding for return type "
                 "into body without return!");

    ret = ASSIGN_STMT (last_assign);
    id = TCgetNthExprsExpr (pos_of_ret, RETURN_EXPRS (ret));
    avis = ID_AVIS (id);

    DBUG_ASSERT (NODE_TYPE (id) == N_id, "non N_id node found in N_return");

    new_avis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)), TYcopyType (spec_type));
    ID_AVIS (id) = new_avis;

    /**
     * we are recycling the N_assign node here as it may either be hooked
     * up to an N_block or an N_assign node and we do not want to use
     * a traversal here!
     */

    if (TYisAKD (spec_type)) {
        ASSIGN_STMT (last_assign)
          = TBmakeLet (TBmakeIds (new_avis, NULL),
                       TCmakePrf2 (F_hideShape_SxA, TBmakeNum (0), TBmakeId (avis)));
    } else {
        ASSIGN_STMT (last_assign)
          = TBmakeLet (TBmakeIds (new_avis, NULL),
                       TCmakePrf2 (F_hideDim_SxA, TBmakeNum (0), TBmakeId (avis)));
    }

    ASSIGN_NEXT (last_assign) = TBmakeAssign (ret, NULL);

    if (PHisSAAMode ()) {
        AVIS_SSAASSIGN (new_avis) = last_assign;
    }

    FUNDEF_VARDECS (fundef) = TBmakeVardec (new_avis, FUNDEF_VARDECS (fundef));

    if (TYisAUDGZ (spec_type)) {
        InsertTypeConv (fundef, pos_of_ret, spec_type);
    }

    DBUG_RETURN (fundef);
}

node *
AdjustReturnTypesOfSpecialization (node *fundef, ntype *rets)
{
    /* FIXME: Something seems to go wrong here when a function that takes an AKD
     * array and returns an AKD array is specialized for AKS arrays. Only the parameter
     * becomes AKS but the return type should also become AKS. Or at least a type
     * conversion should be inserted.  */
    node *ret;
    size_t i;
    size_t j;
    ntype *spec_type, *inherited_type, *new_type;

    DBUG_ENTER ();

    ret = FUNDEF_RETS (fundef);
    i = 0;
    j = 0;
    while (ret != NULL) {
        spec_type = TYgetProductMember (rets, i);
        inherited_type = SSIgetMax (TYgetAlpha (RET_TYPE (ret)));

        switch (TYcmpTypes (spec_type, inherited_type)) {
        case TY_eq:
            if (global.runtime && STReq (FUNDEF_NAME (fundef), global.rt_fun_name)) {
                if (TYisAUD (inherited_type) || TYisAUDGZ (inherited_type)
                    || TYisAKD (inherited_type)) {
                    fundef = InsertHideInfo (fundef, j, inherited_type);
                }
            }
            break;
        case TY_lt:
            /**
             * the specialisation claims to return a more specific
             * type which remains to be proved. Hence, we insert a type conv!
             */
            fundef = InsertTypeConv (fundef, j, spec_type);
            /**
             * Now, we inherit the return type:
             */
            /* Falls through. */
        case TY_gt:
            if (global.runtime && STReq (FUNDEF_NAME (fundef), global.rt_fun_name)) {
                if (TYisAUD (inherited_type) || TYisAUDGZ (inherited_type)
                    || TYisAKD (inherited_type)) {
                    fundef = InsertHideInfo (fundef, j, inherited_type);
                }
            }

            /**
             * we can potentially sharpen the return type here, as we know
             * that any specialisation can not specialize to a LESS
             * precise type!
             */
            new_type = TYcopyType (inherited_type);
            spec_type = TYfreeType (spec_type);
            rets = TYsetProductMember (rets, i, new_type);

            break;
        case TY_hcs:
        case TY_dis:
        default:
            DBUG_UNREACHABLE ("dispach should no have worked!");
        }

        ret = RET_NEXT (ret);
        i++;
        j++;
    }
    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *    ntype * SpecializationOracle( node *wrapper, node *fundef, ntype *args,
 *                                  dft_res * dft)
 *
 * description:
 *    The oracle recieves an argument type, a function that could be specialized
 *    its wrapper, and the full result of a function dispatch.
 *    From these data, it decides whether a full specialization can be made
 *    ( NULL is returned in this case !!!!!!) or - at best - a less precise
 *    approximation can be done only. In that case the best possible approximation
 *    is returned. As a consequence, returning the arg types of the fundef
 *    will prevent from any specialization!
 *    NB: The global.spec_mode flag is not handled appropriately right now. Due to
 *    the lack of an implementation for the static analysis, we cannot decide
 *    how far we should specialize in order to achieve a certain level of
 *    shape information. Nevertheless, a rude approximation is realized:
 *    In case of  SS_aud, no specialization at all will be done which definitively
 *    is correct. In all other cases we just do the best we can so far...
 *
 ******************************************************************************/

static ntype *
SpecializationOracle (node *wrapper, node *fundef, ntype *args, dft_res *dft)
{
    ntype *type, *res;
    node *arg;
    size_t i;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    DBUG_EXECUTE (tmp_str = TYtype2String (args, FALSE, 0));
    DBUG_PRINT ("spec %s for %s?", CTIitemName (fundef), tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    if ((dft->num_deriveable_partials > 1)
        || ((dft->num_deriveable_partials == 1) && (dft->deriveable != NULL))
        || FUNDEF_ISEXTERN (fundef) || (FUNDEF_SPECS (fundef) >= global.maxspec)
        || (global.spec_mode == SS_aud)) {

        arg = FUNDEF_ARGS (fundef);
        res = TYmakeEmptyProductType (TCcountArgs (arg));
        i = 0;
        while (arg != NULL) {
            type = TYcopyType (AVIS_TYPE (ARG_AVIS (arg)));
            res = TYsetProductMember (res, i,
                                      TYlubOfTypes (TYgetProductMember (args, i), type));
            type = TYfreeType (type);
            arg = ARG_NEXT (arg);
            i++;
        }
    } else if (TYisProdContainingAKV (args)) {
        res = TYeliminateAKV (args);
    } else {
        res = NULL;
    }

    DBUG_EXECUTE (tmp_str = TYtype2String (res, FALSE, 0));
    DBUG_PRINT ("oracle: %s%s!", (res == NULL ? "yes" : "try with"), tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    node *UpdateFixSignature( node *fundef, ntype *args)
 *
 * description:
 *    Here, we assume that all argument types are either array types or
 *    type variables with identical Min and Max!
 *    This function replaces the old type siganture (in the N_arg nodes)
 *    by the MINIMUM of the old type and the given argument types (arg_ts).
 *    Such a minimum MUST exist as we want to specialize the function!!
 *    Furthermore, the declared type is updated as well, as a specialisation
 *    basically is a new function declaration.
 *    It returns the modified N_fundef node.
 *
 ******************************************************************************/

static node *
UpdateFixSignature (node *fundef, ntype *arg_ts, ntype *ret_ts)
{
    node *args, *rets;
    ntype *type, *old_type, *new_type;
    size_t i = 0;

    DBUG_ENTER ();
    DBUG_ASSERT (TCcountArgs (FUNDEF_ARGS (fundef)) == TYgetProductSize (arg_ts),
                 "UpdateFixSignature called with incompatible no of arguments!");
    DBUG_ASSERT (TYisProdOfArrayOrFixedAlpha (arg_ts),
                 "UpdateFixSignature called with non-fixed args!");
    DBUG_ASSERT (((ret_ts == NULL)
                  || (TCcountRets (FUNDEF_RETS (fundef)) == TYgetProductSize (ret_ts))),
                 "UpdateFixSignature called with incompatible no of return "
                 "values!");
    DBUG_ASSERT (((ret_ts == NULL) || (TYisProdOfArrayOrFixedAlpha (ret_ts))),
                 "UpdateFixSignature called with non-fixed rets!");

    args = FUNDEF_ARGS (fundef);
    while (args) {
        DBUG_ASSERT ((!ARG_ISARTIFICIAL (args) || (ARG_OBJDEF (args) != NULL)), "BOOM!");
        type = TYgetProductMember (arg_ts, i);
        old_type = ARG_NTYPE (args);
        DBUG_ASSERT (old_type != NULL,
                     "UpdateFixSignature called on fundef w/o arg type");
        if ((TYisSimple (TYgetScalar (old_type)))
            && (TYgetSimpleType (TYgetScalar (old_type)) == T_unknown)) {
            DBUG_ASSERT (FUNDEF_ISLACFUN (fundef), "unknown arg type at non-LaC fun!");
            old_type = TYfreeType (old_type);
            new_type = TYcopyType (type);
        } else if (TYleTypes (type, old_type)) {
            TYfreeType (old_type);
            new_type = TYcopyType (type);
        } else {
            DBUG_ASSERT (TYleTypes (old_type, type),
                         "UpdateFixSignature called with incompatible args");
            new_type = old_type;
        }
        AVIS_TYPE (ARG_AVIS (args)) = new_type;
        /*
         * the declared type is set to the infered type here.
         * as these usually do not differ for args at this stage
         * of compilation, this should not be a problem. Be aware
         * that when using the specialisation system after the
         * typechecker, this assumption may not be true anymore!
         * especially after replacing udts by their basetypes, the
         * inferred type cannot be simply copied!
         */
        AVIS_DECLTYPE (ARG_AVIS (args)) = TYfreeType (AVIS_DECLTYPE (ARG_AVIS (args)));
        AVIS_DECLTYPE (ARG_AVIS (args)) = TYcopyType (new_type);

        args = ARG_NEXT (args);
        i++;
    }

    rets = FUNDEF_RETS (fundef);
    i = 0;
    while ((ret_ts != NULL) && (rets != NULL)) {
        RET_TYPE (rets) = TYfreeType (RET_TYPE (rets));
        RET_TYPE (rets) = TYcopyType (TYgetProductMember (ret_ts, i));

        rets = RET_NEXT (rets);
        i++;
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *    node *UpdateVarSignature( node *fundef, ntype *args)
 *
 * description:
 *    Here, we assume that all argument types are either array types or
 *    type variables with identical Min and Max!
 *    This function replaces the old type siganture (in the N_arg nodes)
 *    by type variables and makes the argument types subtypes of those.
 *    It returns the modified N_fundef node.
 *
 ******************************************************************************/

static node *
UpdateVarSignature (node *fundef, ntype *arg_ts)
{
    node *args;
    ntype *type, *old_type, *new_type;
    size_t i = 0;
    bool ok = TRUE;

    DBUG_ENTER ();
    DBUG_ASSERT (TCcountArgs (FUNDEF_ARGS (fundef)) == TYgetProductSize (arg_ts),
                 "UpdateVarSignature called with incompatible no of arguments!");
    DBUG_ASSERT (TYisProdOfArrayOrFixedAlpha (arg_ts),
                 "UpdateVarSignature called with non-fixed args!");

    args = FUNDEF_ARGS (fundef);
    while (args) {
        DBUG_ASSERT ((!ARG_ISARTIFICIAL (args) || (ARG_OBJDEF (args) != NULL)), "BOOM!");
        type = TYgetProductMember (arg_ts, i);
        old_type = ARG_NTYPE (args);
        if (old_type == NULL) {
            new_type = TYmakeAlphaType (NULL);
        } else if (!TYisAlpha (old_type)) {
            new_type = TYmakeAlphaType (NULL);
            if (TYisUser (TYgetScalar (old_type))
                || TYgetSimpleType (TYgetScalar (old_type)) != T_unknown) {
                ok = SSInewTypeRel (new_type, old_type);
            }
            old_type = TYfreeType (old_type);
        } else {
            new_type = old_type;
        }
        ok = ok && SSInewTypeRel (type, new_type);
        if (!ok) {
            CTIabort (LINE_TO_LOC (global.linenum),
                      "loop variable \"%s\" is being used inconsistently in function %s; "
                      "conflicting types are %s and %s",
                      ARG_NAME (args), FUNDEF_NAME (fundef),
                      TYtype2String (type, FALSE, 0),
                      TYtype2String (TYfixAndEliminateAlpha (new_type), FALSE, 0));
        }

        ARG_NTYPE (args) = new_type;

        args = ARG_NEXT (args);
        i++;
    }

    DBUG_RETURN (fundef);
}

static ntype *
checkAndRebuildWrapperType (ntype *type)
{
    ntype *result;

    DBUG_ENTER ();

    if (TYcontainsAlpha (type)) {
        DBUG_PRINT ("wrapper type seems not to be finalized");

        result = type;
    } else {
        DBUG_PRINT ("wrapper type seems to be finalized -- rebuilding");

        result = TUrebuildWrapperTypeAlphaFix (type);

        type = TYfreeType (type);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Helper functions for MCG traversal. Resets some flags of LaC-funs
 *        as they become local during specialisation.
 *
 * @param arg_node N_fundef node of a LaC-fun
 * @param arg_info NULL
 *
 * @return modified N_fundef node
 ******************************************************************************/
static node *
ResetLaCFlagsAndArgTypes (node *arg_node, info *arg_info)
{
    node *args;
    DBUG_ENTER ();

    FUNDEF_ISLOCAL (arg_node) = TRUE;
    FUNDEF_WASUSED (arg_node) = FALSE;
    FUNDEF_WASIMPORTED (arg_node) = FALSE;

    /*
     * Now, we reset all argument types to unknown[*] to avoid potential previous
     * constraints from the more generic function version to persist!
     * This fixes bug #1062.
     */
    args = FUNDEF_ARGS (arg_node);
    while (args != NULL) {
        if (TYisAlpha (AVIS_TYPE (ARG_AVIS (args)))) {
            AVIS_TYPE (ARG_AVIS (args)) = TYfreeType (AVIS_TYPE (ARG_AVIS (args)));
            AVIS_TYPE (ARG_AVIS (args)) = TYmakeAUD (TYmakeSimpleType (T_unknown));
        }
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Helper function for MCG traversal. Sets the namespace of all LaC-funs
 *        to the given namespace.
 *
 * @param arg_node N_fundef node of LaC-fun
 * @param ns namespace
 *
 * @return modified N_fundef node
 ******************************************************************************/
static node *
SetLaCNamespace (node *arg_node, namespace_t *ns)
{
    DBUG_ENTER ();

    FUNDEF_NS (arg_node) = NSfreeNamespace (FUNDEF_NS (arg_node));
    FUNDEF_NS (arg_node) = NSdupNamespace (ns);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Entry function for specialising a given function.
 *
 * @param wrapper Wrapper of the function to be specialised
 * @param fundef Instance to be specialised
 * @param args Types of arguments the instance shall be specialised for
 * @param rets Types of return arguments the instance shall be specialised for
 *             or NULL to not fix the return types.
 *
 * @return the wrapper, possibly containing the new instance
 ******************************************************************************/
static node *
DoSpecialize (node *wrapper, node *fundef, ntype *args, ntype *rets)
{
    node *res;
    node *res_ret, *fundef_ret;
    node *impl_fun, *impl_wrapper;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    DBUG_EXECUTE (tmp_str = TYtype2String (args, FALSE, 0));
    DBUG_PRINT ("specializing %s for %s", CTIitemName (fundef), tmp_str);
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    if (FUNDEF_CHECKIMPLFUNDEF (fundef) != NULL) {
        impl_fun = FUNDEF_CHECKIMPLFUNDEF (fundef);
        impl_wrapper = FUNDEF_WRAPPERFUNDEF (impl_fun);
        impl_wrapper = DoSpecialize (impl_wrapper, impl_fun, args, rets);
    }
    /*
     * in case of a function loaded from a module, the body may be missing, so
     * fetch it.
     */
    if ((FUNDEF_SYMBOLNAME (fundef) != NULL) && (FUNDEF_BODY (fundef) == NULL)) {
        fundef = AFBdoAddFunctionBody (fundef);
    }

    /* copy the fundef to be specialized */
    res = DUPdoDupNode (fundef);

    /* in case the ret-types in rets are more specific than those in res,
     * we need to insert a type conv in the body of the function and
     * can subsequently relax the rets types! (cf. bug 595)
     */
    if (rets != NULL) {
        res = AdjustReturnTypesOfSpecialization (res, rets);
    }

    DBUG_ASSERT (FUNDEF_BODY (res) != NULL, "missing body while trying to specialise!");

    /* reset the SYMBOLNAME attribute, as the function is _not_
     * the one referenced by the SYMBOLNAME anymore
     */
    if (FUNDEF_SYMBOLNAME (res) != NULL) {
        FUNDEF_SYMBOLNAME (res) = MEMfree (FUNDEF_SYMBOLNAME (res));
    }

    /*
     * reset all flags and the namespace to ensure that
     * the specialisation is handeled as a local function
     */
    FUNDEF_ISLOCAL (res) = TRUE;
    FUNDEF_WASUSED (res) = FALSE;
    FUNDEF_WASIMPORTED (res) = FALSE;
    FUNDEF_ISSPECIALISATION (res) = TRUE;

    /*
     * reset flags and arg types of contained lac funs as well
     */
    MCGdoMapCallGraph (res, ResetLaCFlagsAndArgTypes, NULL, MCGcontLacFun, NULL);

    /*
     * if it is a used function, we have to make up a new
     * namespace for the specialisations. As a function instance
     * can be used and imported at the same time, we need
     * to look at the current wrapper to find out what the current
     * context is. If that wrapper has a different namespace than
     * the current, it is a used function.
     */

    if (NSequals (FUNDEF_NS (wrapper), global.modulenamespace)) {
        /*
         * we have a local wrapper so the new specialisation should
         * become a member of the current namespace. We have to set
         * that here explicitly, as the instance this specialisation
         * was derived from, might come from another namespace
         */
        FUNDEF_NS (res) = NSfreeNamespace (FUNDEF_NS (res));
        FUNDEF_NS (res) = NSdupNamespace (FUNDEF_NS (wrapper));
    } else {
        /*
         * this must be a used instance, so we have to create a new
         * view for it. As there may be more specialisations for
         * this overloaded function and we dont want to have too
         * many views, we annotate the view at the wrapper and
         * reuse it for further specialisations.
         * Furthermore, split_wrappers uses this informations to
         * decide where to put the new wrapper.
         */
        if (FUNDEF_SPECNS (wrapper) == NULL) {
            FUNDEF_SPECNS (wrapper) = NSbuildView (FUNDEF_NS (wrapper));
        }

        FUNDEF_NS (res) = NSfreeNamespace (FUNDEF_NS (res));
        FUNDEF_NS (res) = NSdupNamespace (FUNDEF_SPECNS (wrapper));
    }

    /*
     * propagate namespace information into lac funs
     */
    MCGdoMapCallGraph (res, (travfun_p)SetLaCNamespace, NULL, MCGcontLacFun,
                       (info *)FUNDEF_NS (res));

    /*
     * store the fundef in the specchain
     */
    FUNDEF_NEXT (res) = specialized_fundefs;
    specialized_fundefs = res;

    /*
     * do actually specialize the copy !!
     * */
    UpdateFixSignature (res, args, rets);

    /*
     * convert the return type(s) into Alpha - AUDs
     */
    FUNDEF_RETS (res) = TUrettypes2alphaMax (FUNDEF_RETS (res));

    /*
     * make sure the funtype annotated at the wrapper is
     * not finalised (e.g. it still contains alphas). If it
     * already is finalised (thus it was either used, or we
     * are running in the optimizations) rebuild the wrapper
     * type from scratch.
     */
    FUNDEF_WRAPPERTYPE (wrapper)
      = checkAndRebuildWrapperType (FUNDEF_WRAPPERTYPE (wrapper));

    /*
     * Finally, we make the result type variable(s) (a) subtype(s) of the
     * original one(s)!
     */
    res_ret = FUNDEF_RETS (res);
    fundef_ret = FUNDEF_RETS (fundef);
    while (res_ret != NULL) {
        SSInewTypeRel (RET_TYPE (res_ret), RET_TYPE (fundef_ret));
        res_ret = RET_NEXT (res_ret);
        fundef_ret = RET_NEXT (fundef_ret);
    }

    /* insert the new type signature into the wrapper */
    FUNDEF_WRAPPERTYPE (wrapper)
      = TYmakeOverloadedFunType (TUcreateFuntype (res), FUNDEF_WRAPPERTYPE (wrapper));

    FUNDEF_SPECS (fundef)++;

    /*
     * we do not return the specialised fundef, as we do not want
     * to dispatch right now! this will be done lateron in
     * staticfunctiondispatch.
     */
    DBUG_RETURN (fundef);
}

/******************************************************************************
 ***
 ***       Exported functions that may trigger function specializations:
 ***       -------------------------------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    dft_res *SPEChandleDownProjections( dft_res *dft,
 *                                          node *wrapper,
 *                                            ntype *args,
 *                                            ntype *rets)
 *
 * description:
 *
 ******************************************************************************/

dft_res *
SPEChandleDownProjections (dft_res *dft, node *wrapper, ntype *args, ntype *rets)
{
    ntype *new_args;
    int i;

    DBUG_ENTER ();

    while (dft->deriveable != NULL) {
        new_args = SpecializationOracle (wrapper, dft->deriveable, args, dft);
        if (new_args == NULL) {
            DoSpecialize (wrapper, dft->deriveable, args, rets);
            for (i = 0; i < dft->num_deriveable_partials; i++) {
                DoSpecialize (wrapper, dft->deriveable_partials[i], args, rets);
            }
        } else {
            args = new_args;
        }
        dft = NTCCTdispatchFunType (wrapper, args);
    }

    while (dft->num_deriveable_partials > 0) {
        new_args = SpecializationOracle (wrapper, dft->deriveable_partials[0], args, dft);
        if (new_args == NULL) {
            for (i = 0; i < dft->num_deriveable_partials; i++) {
                DoSpecialize (wrapper, dft->deriveable_partials[i], args, rets);
            }
        } else {
            args = new_args;
        }
        dft = NTCCTdispatchFunType (wrapper, args);
    }

    /*
     * all potential specializations are done, i.e., we do not
     * have any deriveable stuff any more!
     */
    DBUG_ASSERT ((dft->deriveable == NULL) && (dft->num_deriveable_partials == 0),
                 "specialization did not eliminate all deriveables!");

    DBUG_RETURN (dft);
}

/******************************************************************************
 *
 * function:
 *    node *SPEChandleLacFun( node *fundef, node *assign, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

node *
SPEChandleLacFun (node *fundef, node *assign, ntype *args)
{
    DBUG_ENTER ();

    DBUG_ASSERT (FUNDEF_ISLACFUN (fundef), "SPEChandleLacFun called with non LaC fun!");

    /**
     * Now, we update the signature of fundef. On first thought you
     * may think that, for conditionals, it suffices to
     * directly specialize the signature, using "UpdateFixSignature".
     * Unfortunately, this is NOT true: Due to the fix point iteration
     * we may have to coarsen the argument types. An example for
     * this situation is:
     *
     * int[*] f( )
     * {
     *   if( true ) {
     *     x = f();
     *     if( false) {
     *       res = [x];
     *     } else {
     *       res = [x];
     *     }
     *   } else {
     *     res = 1;
     *   }
     *   return( res);
     * }
     * To cope properly with that situation, we need to insert variable
     * args and returns which may be coarsened afterwards.
     *
     * For loops, we usually do have more than one call. Since we
     * do not want to create more than ONE specialization, we use type vars
     * for approximating the argument types. On the very first call,
     * we create these type vars; on ALL calls, we make the actual argument
     * types minima of these, i.e., we make the overall types potentially
     * less precise. This is done by "UpdateVarSignature".
     */
    fundef = UpdateVarSignature (fundef, args);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *    node *SPECresetSpecChain()
 *
 * description:
 *
 ******************************************************************************/

node *
SPECresetSpecChain (void)
{
    node *res;

    DBUG_ENTER ();

    res = specialized_fundefs;
    specialized_fundefs = NULL;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    node *SPECinitSpecChain()
 *
 * description:
 *
 *  This function should actually be superfluous because the hook
 *  specialized_fundefs is emptied each time after specialisation may
 *  have taken place.
 *
 ******************************************************************************/

void
SPECinitSpecChain (void)
{
    DBUG_ENTER ();

    DBUG_ASSERT (specialized_fundefs == NULL,
                 "Non-empty spec chain found on initialisation");

    DBUG_RETURN ();
}

/* @} */ /* addtogroup ntc */

#undef DBUG_PREFIX
