/*
 *
 * $Log$
 * Revision 1.21  2004/12/09 00:37:35  sbs
 * UpdateVarSignature debugged
 *
 * Revision 1.20  2004/12/07 16:49:30  sbs
 * FixSignature now handles unknown[*] args of LaCfuns properly
 *
 * Revision 1.19  2004/12/07 14:36:54  sbs
 * UpdateFixSignature now expects new rather than old types on the N_arg's
 *
 * Revision 1.18  2004/12/01 18:43:28  sah
 * renamed a function
 *
 * Revision 1.17  2004/11/27 02:15:19  sbs
 * *** empty log message ***
 *
 * Revision 1.16  2004/11/27 02:14:16  sbs
 * *** empty log message ***
 *
 * Revision 1.15  2004/11/25 17:52:55  sbs
 * compiles
 *
 * Revision 1.14  2004/11/23 21:56:54  sbs
 * SacDevCamp04 done
 *
 * Revision 1.13  2004/10/28 16:11:21  sah
 * added support for used functions
 * and deserialisation
 *
 * Revision 1.12  2004/10/26 17:18:50  sbs
 * added rudementary support for spec_mode:
 * in case of SS_aud, specialization will be suppressed.
 *
 * Revision 1.11  2004/09/30 15:12:35  sbs
 * eliminated FunTypes from ALL but wrapper functions
 * (memory concerns!)
 * Now, the function signatures of individual instances are
 * stored in the AVIS_TYPE and FUNDEF_RET_TYPE only!!!!!
 *
 * Revision 1.10  2004/03/05 12:09:20  sbs
 * UpdateVarSignature added. Loops are the only functions where the arguments
 * have to be type vars!!!
 *
 * Revision 1.8  2003/09/11 15:26:44  sbs
 * function specialization now bound by max_overload!
 *
 * Revision 1.7  2003/04/07 14:30:31  sbs
 * specialization oracle now does not at all specialize for AKVs!
 *
 * Revision 1.6  2003/04/01 16:37:37  sbs
 * some doxygen added.
 *
 * Revision 1.5  2002/10/18 14:28:45  sbs
 * specialization of external functions suppressed 8-))
 *
 * Revision 1.4  2002/09/04 12:59:46  sbs
 * SpecializationOracle changed so that part deriveables are not specialized anymore 8-((
 *
 * Revision 1.3  2002/09/03 15:02:53  sbs
 * bug in SPECHandleLacFun eliminated
 *
 * Revision 1.2  2002/09/03 14:41:45  sbs
 * DupTree machanism for duplicating condi funs established
 *
 * Revision 1.1  2002/08/05 16:58:37  sbs
 * Initial revision
 *
 *
 */

#include "specialize.h"
#include "dbug.h"
#include "internal_lib.h"
#include "free.h"
#include "ct_fun.h"
#include "create_wrappers.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "deserialize.h"
#include "new_types.h"
#include "type_utils.h"
#include "ssi.h"

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
    int i;

    DBUG_ENTER ("SpecializationOracle");
    if ((dft->num_deriveable_partials > 1)
        || ((dft->num_deriveable_partials == 1) && (dft->deriveable != NULL))
        || FUNDEF_ISEXTERN (fundef) || (FUNDEF_SPECS (fundef) >= global.maxspec)
        || (global.spec_mode == SS_aud)) {

        arg = FUNDEF_ARGS (fundef);
        res = TYmakeEmptyProductType (TCcountArgs (arg));
        i = 0;
        while (arg != NULL) {
            type = AVIS_TYPE (ARG_AVIS (arg));
            if (type == NULL) {
                /* not yet converted ! */
                type = TYoldType2Type (ARG_TYPE (arg));
            } else {
                type = TYcopyType (type);
            }
            res = TYsetProductMember (res, i, type);
            arg = ARG_NEXT (arg);
            i++;
        }

    } else if (TYisProdContainingAKV (args)) {
        res = TYeliminateAKV (args);
    } else {
        res = NULL;
    }

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
 *    by the MINIMUM of the old type and the given argument types (arg_ts),
 *    and updates the function type ( FUNDEF_WRAPPERTYPE( fundef) ) as well.
 *    Such a minimum MUST exist as we want to specialize the function!!
 *    It returns the modified N_fundef node.
 *
 ******************************************************************************/

static node *
UpdateFixSignature (node *fundef, ntype *arg_ts)
{
    node *args;
    ntype *type, *old_type, *new_type;
    int i = 0;

    DBUG_ENTER ("UpdateFixSignature");
    DBUG_ASSERT ((TCcountArgs (FUNDEF_ARGS (fundef)) == TYgetProductSize (arg_ts)),
                 "UpdateFixSignature called with incompatible no of arguments!");
    DBUG_ASSERT ((TYisProdOfArrayOrFixedAlpha (arg_ts)),
                 "UpdateFixSignature called with non-fixed args!");

    args = FUNDEF_ARGS (fundef);
    while (args) {
        type = TYgetProductMember (arg_ts, i);
        old_type = ARG_NTYPE (args);
        DBUG_ASSERT (old_type != NULL,
                     "UpdateFixSignature called on fundef w/o arg type");
        if (TYgetSimpleType (TYgetScalar (old_type)) == T_unknown) {
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

        args = ARG_NEXT (args);
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
 *    It returns the modified N_fundef node.
 *
 ******************************************************************************/

static node *
UpdateVarSignature (node *fundef, ntype *arg_ts)
{
    node *args;
    ntype *type, *old_type, *new_type;
    int i = 0;
    bool ok;

    DBUG_ENTER ("UpdateVarSignature");
    DBUG_ASSERT ((TCcountArgs (FUNDEF_ARGS (fundef)) == TYgetProductSize (arg_ts)),
                 "UpdateVarSignature called with incompatible no of arguments!");
    DBUG_ASSERT ((TYisProdOfArrayOrFixedAlpha (arg_ts)),
                 "UpdateVarSignature called with non-fixed args!");

    args = FUNDEF_ARGS (fundef);
    while (args) {
        type = TYgetProductMember (arg_ts, i);
        new_type = AVIS_TYPE (ARG_AVIS (args));
        if ((new_type == NULL) || (!TYisAlpha (new_type))) {
            if (new_type != NULL) {
                new_type = TYfreeType (new_type);
            }
            new_type = TYmakeAlphaType (NULL);
            old_type = ARG_NTYPE (args);
            if (old_type != NULL) {
                ok = SSInewTypeRel (old_type, new_type);
            }
            ok = SSInewTypeRel (type, new_type);
        } else {
            DBUG_ASSERT (TYisAlpha (new_type), "UpdateVarSignature called with "
                                               "non-var argument type");
            ok = SSInewTypeRel (type, new_type);
            DBUG_ASSERT (ok, "UpdateVarSignature called with incompatible args");
        }

        AVIS_TYPE (ARG_AVIS (args)) = new_type;

        args = ARG_NEXT (args);
        i++;
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *    node *DoSpecialize(node *wrapper, node *fundef, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

static node *
DoSpecialize (node *wrapper, node *fundef, ntype *args)
{
    node *res;
    node *res_ret, *fundef_ret;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("DoSpecialize");

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (args, FALSE, 0););
    DBUG_PRINT ("NTC", ("specializing %s for %s", FUNDEF_NAME (fundef), tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = ILIBfree (tmp_str););

    /*
     * in case of a function of a module, the body is missing, so
     * fetch it
     */
    if ((FUNDEF_SYMBOLNAME (fundef) != NULL) && (FUNDEF_BODY (fundef) == NULL)) {
        fundef = DSdoDeserialize (fundef);
    }

    /* copy the fundef to be specialized */
    res = DUPdoDupNode (fundef);

    /* reset the SYMBOLNAME attribute, as the function is _not_
     * the one referenced by the SYMBOLNAME anymore
     */
    if (FUNDEF_SYMBOLNAME (res) != NULL) {
        FUNDEF_SYMBOLNAME (res) = ILIBfree (FUNDEF_SYMBOLNAME (res));
    }

    /* insert the new fundef into the specialiazed chain */
    FUNDEF_NEXT (res) = specialized_fundefs;
    specialized_fundefs = res;

    /* do actually specialize the copy !! */
    UpdateFixSignature (res, args);

    /* convert the return type(s) into Alpha - AUDs */
    FUNDEF_RETS (res) = TUrettypes2alphaAUD (FUNDEF_RETS (res));

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
      = TYmakeOverloadedFunType (CRTWRPcreateFuntype (res), FUNDEF_WRAPPERTYPE (wrapper));

    FUNDEF_SPECS (fundef)++;

    DBUG_RETURN (res);
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
 *                                            ntype *args)
 *
 * description:
 *
 ******************************************************************************/

dft_res *
SPEChandleDownProjections (dft_res *dft, node *wrapper, ntype *args)
{
    node *new_fundef;
    ntype *new_args;
    int i;

    DBUG_ENTER ("HandleDownProjections");

    while (dft->deriveable != NULL) {
        new_args = SpecializationOracle (wrapper, dft->deriveable, args, dft);
        if (new_args == NULL) {
            new_fundef = DoSpecialize (wrapper, dft->deriveable, args);
            for (i = 0; i < dft->num_deriveable_partials; i++) {
                new_fundef = DoSpecialize (wrapper, dft->deriveable_partials[i], args);
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
                new_fundef = DoSpecialize (wrapper, dft->deriveable_partials[i], args);
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
    node *fun, *module;

    DBUG_ENTER ("SPEChandleLacFun");
    DBUG_ASSERT (FUNDEF_ISLACFUN (fundef), "SPEChandleLacFun called with non LaC fun!");

    if (FUNDEF_USED (fundef) > 1) {
        /*
         * The function that calls this LACfun has been specialized.
         * Unfortunately, the "specialization" of LAC functions is postponed
         * until actualy found for type checking, i.e., until here:
         */
        module = TBmakeModule ("dummy", F_prog, NULL, NULL, NULL, NULL, NULL);
        module = DUPcheckAndDupSpecialFundef (module, fundef, assign);
        fun = MODULE_FUNS (module);
        MODULE_FUNS (module) = NULL;
        module = FREEdoFreeNode (module);

        FUNDEF_TCSTAT (fun) = 0; /* NTC_not_checked; */

        /* insert the new fundef into the specialized chain */
        FUNDEF_NEXT (fun) = specialized_fundefs;
        specialized_fundefs = fun;

    } else {
        fun = fundef;
    }
    /**
     * Now, we update the signature of fundef. In case of conditionals, we can
     * directly specialize the signature, using "UpdateFixSignature".
     * For loops, however, we usually do have more than one call. Since we
     * do not want to create more than ONE specialization, we use type vars
     * for approximating the argument types. On the very first call,
     * we create these type vars; on ALL calls, we make the actual argument
     * types minima of these, i.e., we make the overall types potentially
     * less precise. This is done by "UpdateVarSignature".
     */
    if (FUNDEF_ISCONDFUN (fun)) {
        UpdateFixSignature (fun, args);
    } else {
        UpdateVarSignature (fun, args);
    }

    FUNDEF_RETS (fun) = TUrettypes2AUD (FUNDEF_RETS (fun));

    DBUG_RETURN (fun);
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
SPECresetSpecChain ()
{
    node *res;

    DBUG_ENTER ("SPECresetSpecChain");

    res = specialized_fundefs;
    specialized_fundefs = NULL;

    DBUG_RETURN (res);
}

/* @} */ /* addtogroup ntc */
