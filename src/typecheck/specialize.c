/*
 *
 * $Log$
 * Revision 1.9  2004/02/20 15:55:49  sbs
 * one argument to MakeModul call added.
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

#include "dbug.h"
#include "specialize.h"
#include "ct_fun.h"
#include "create_wrappers.h"
#include "DupTree.h"

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
 *    ntype * SpecializationOracle( node *wrapper, node *fundef, ntype *args, DFT_res *
 *dft)
 *
 * description:
 *    for a given argument type and a given fundef the oracle computes a new
 *    type which indicates the version to be actually built (specialized).
 *    Contra-intuitively, NULL is returned iff a FULL specialization is desired!
 *
 ******************************************************************************/

static ntype *
SpecializationOracle (node *wrapper, node *fundef, ntype *args, DFT_res *dft)
{
    ntype *type, *res;
    node *arg;
    int i;

    DBUG_ENTER ("SpecializationOracle");
    if ((dft->num_deriveable_partials > 1)
        || ((dft->num_deriveable_partials == 1) && (dft->deriveable != NULL))
        || FUNDEF_IS_EXTERNAL (fundef) || (FUNDEF_SPECS (fundef) >= max_overload)) {

        arg = FUNDEF_ARGS (fundef);
        res = TYMakeEmptyProductType (CountArgs (arg));
        i = 0;
        while (arg != NULL) {
            type = AVIS_TYPE (ARG_AVIS (arg));
            if (type == NULL) {
                /* not yet converted ! */
                type = TYOldType2Type (ARG_TYPE (arg));
            } else {
                type = TYCopyType (type);
            }
            res = TYSetProductMember (res, i, type);
            arg = ARG_NEXT (arg);
            i++;
        }

    } else if (TYIsProdContainingAKV (args)) {
        res = TYEliminateAKV (args);
    } else {
        res = NULL;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    node *UpdateSignature( node *fundef, ntype *args)
 *
 * description:
 *    Here, we assume that all argument types are either array types or
 *    type variables with identical Min and Max!
 *    This function replaces the old type siganture (in the N_arg nodes)
 *    by the MINIMUM of the old type and the given argument types (arg_ts),
 *    and updates the function type ( FUNDEF_TYPE( fundef) ) as well.
 *    Such a minimum MUST exist as we want to specialize the function!!
 *    It returns the modified N_fundef node.
 *
 ******************************************************************************/

static node *
UpdateSignature (node *fundef, ntype *arg_ts)
{
    node *args;
    ntype *type, *old_type, *new_type;
    int i = 0;

    DBUG_ENTER ("UpdateSignature");
    DBUG_ASSERT ((CountArgs (FUNDEF_ARGS (fundef)) == TYGetProductSize (arg_ts)),
                 "UpdateSignature called with incompatible no of arguments!");
    DBUG_ASSERT ((TYIsProdOfArrayOrFixedAlpha (arg_ts)),
                 "UpdateSignature called with non-fixed args!");

    args = FUNDEF_ARGS (fundef);
    while (args) {
        type = TYGetProductMember (arg_ts, i);
        old_type = TYOldType2Type (ARG_TYPE (args));
        if (old_type == NULL) {
            new_type = TYCopyType (type);
        } else {
            if (TYLeTypes (type, old_type)) {
                new_type = TYCopyType (type);
                TYFreeType (old_type);
            } else {
                DBUG_ASSERT (TYLeTypes (old_type, type),
                             "UpdateSignature called with incompatible args");
                new_type = old_type;
            }
        }
        ARG_TYPE (args) = FreeOneTypes (ARG_TYPE (args));
        ARG_TYPE (args) = TYType2OldType (new_type);
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
    int i, n;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("DoSpecialize");

    DBUG_EXECUTE ("NTC", tmp_str = TYType2String (args, FALSE, 0););
    DBUG_PRINT ("NTC", ("specializing %s for %s", FUNDEF_NAME (fundef), tmp_str));
    DBUG_EXECUTE ("NTC", tmp_str = Free (tmp_str););

    /* copy the fundef to be specialized */
    res = DupNode (fundef);

    /* insert the new fundef into the specialiazed chain */
    FUNDEF_NEXT (res) = specialized_fundefs;
    specialized_fundefs = res;

    /* do actually specialize the copy !! */
    UpdateSignature (res, args);
    FUNDEF_TYPE (res) = CreateFuntype (res);

    /*
     * Finally, we make the result type variable(s) (a) subtype(s) of the
     * the original one(s)!
     */
    n = TYGetProductSize (FUNDEF_RET_TYPE (res));
    for (i = 0; i < n; i++) {
        SSINewRel (TYGetAlpha (TYGetProductMember (FUNDEF_RET_TYPE (res), i)),
                   TYGetAlpha (TYGetProductMember (FUNDEF_RET_TYPE (fundef), i)));
    }

    /* insert the new type signature into the wrapper */
    FUNDEF_TYPE (wrapper)
      = TYMakeOverloadedFunType (TYCopyType (FUNDEF_TYPE (res)), FUNDEF_TYPE (wrapper));

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
 *    DFT_res *SPECHandleDownProjections( DFT_res *dft,
 *                                          node *wrapper,
 *                                            ntype *args)
 *
 * description:
 *
 ******************************************************************************/

DFT_res *
SPECHandleDownProjections (DFT_res *dft, node *wrapper, ntype *args)
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
        dft = NTCFUNDispatchFunType (wrapper, args);
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
        dft = NTCFUNDispatchFunType (wrapper, args);
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
 *    node *SPECHandleLacFun( node *fundef, node *assign, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

node *
SPECHandleLacFun (node *fundef, node *assign, ntype *args)
{
    node *fun, *module;

    DBUG_ENTER ("SPECHandleLacFun");
    DBUG_ASSERT (FUNDEF_IS_LACFUN (fundef), "SPECHandleLacFun called with non LaC fun!");

    if (FUNDEF_USED (fundef) > 1) {
        /*
         * The function that calls this LACfun has been specialized.
         * Unfortunately, the "specialization" of LAC functions is postponed
         * until actualy found for type checking, i.e., until here:
         */
        module = MakeModul ("dummy", F_prog, NULL, NULL, NULL, NULL, NULL);
        module = CheckAndDupSpecialFundef (module, fundef, assign);
        fun = MODUL_FUNS (module);
        MODUL_FUNS (module) = NULL;
        module = FreeNode (module);

        FUNDEF_TCSTAT (fun) = 0; /* NTC_not_checked; */

        /* insert the new fundef into the specialized chain */
        FUNDEF_NEXT (fun) = specialized_fundefs;
        specialized_fundefs = fun;

    } else {
        fun = fundef;
    }
    UpdateSignature (fun, args);
    FUNDEF_TYPE (fun) = CreateFuntype (fun);

    DBUG_RETURN (fun);
}

/******************************************************************************
 *
 * function:
 *    node *SPECResetSpecChain()
 *
 * description:
 *
 ******************************************************************************/

node *
SPECResetSpecChain ()
{
    node *res;

    DBUG_ENTER ("SPECResetSpecChain");

    res = specialized_fundefs;
    specialized_fundefs = NULL;

    DBUG_RETURN (res);
}

/* @} */ /* addtogroup ntc */
