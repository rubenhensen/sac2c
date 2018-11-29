#include <stdio.h>

#include "ct_basic.h"

#define DBUG_PREFIX "NTC"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#include "constants.h"
#include "new_types.h"
#include "type_utils.h"
#include "sig_deps.h"
#include "traverse.h"
#include "tree_basic.h"

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

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTcomputeType( ct_funptr CtFun, te_info *info, ntype *args)
 *    ntype *NTCCTcomputeTypeNonStrict( ct_funptr CtFun, te_info *info,
 *                                      ntype *args)
 *
 * description:
 *   Either computes a return type or establishes a signature dependency.
 *   The non-strict version creates a non-strict signature dependency
 *   rather than a strict one.
 *
 ******************************************************************************/

static ntype *
ComputeType (ct_funptr CtFun, te_info *info, ntype *args, bool strict)
{
    ntype *res, *bottom;
    size_t i, num_res;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_EXECUTE (tmp_str = TYtype2String (args, 0, 0));
    DBUG_PRINT ("computing type of %s \"%s%s%s\" applied to %s", TEgetKindStr (info),
                TEgetKind (info) == TE_udf || TEgetKind (info) == TE_foldf
                  ? TEgetModStr (info)
                  : "",
                TEgetKind (info) == TE_udf || TEgetKind (info) == TE_foldf ? "::" : "",
                TEgetNameStr (info), tmp_str);
    DBUG_EXECUTE (MEMfree (tmp_str));

    /*
     * as all CtFun s  operate on array types only, we try to fix as many argument
     * types as possible:
     */
    args = TYeliminateAlpha (args);

    /*
     * At this point, we have to distinguish two fundamentally different
     * situations:
     * -  either all argument types are fixed (i.e., they are array types);
     *    then we simply compute the result type from the argument type(s)!!
     * -  or at least one argument type is not yet fixed! In this case we
     *    are dealing with a recursive call, which means that we have to
     *    postpone the type computation and to establish a signature
     *    dependency which implicitly introduces new type variables for
     *    all result types!
     */

    if (TYcountNonFixedAlpha (args) == 0) {
        if (TYisProdOfArray (args) || !strict) {
            res = CtFun (info, args);
        } else {
            /**
             * args contain bottom which needs to be propagated.
             */
            bottom = TYgetBottom (args);
            DBUG_ASSERT (bottom != NULL, "inconsistent type in NTCCTcomputeType!");
            num_res = TEgetNumRets (info);
            res = TYmakeEmptyProductType (num_res);
            for (i = 0; i < num_res; i++) {
                res = TYsetProductMember (res, i, TYcopyType (bottom));
            }
        }
    } else {
        res = SDcreateSignatureDependency (CtFun, info, args, strict);
    }

    DBUG_EXECUTE (tmp_str = TYtype2String (res, FALSE, 0));
    DBUG_PRINT ("yields %s", tmp_str);
    DBUG_EXECUTE (MEMfree (tmp_str));

    return (res);
}

ntype *
NTCCTcomputeType (ct_funptr CtFun, te_info *info, ntype *args)
{
    DBUG_ENTER ();
    DBUG_RETURN (ComputeType (CtFun, info, args, TRUE));
}

ntype *
NTCCTcomputeTypeNonStrict (ct_funptr CtFun, te_info *info, ntype *args)
{
    DBUG_ENTER ();
    DBUG_RETURN (ComputeType (CtFun, info, args, FALSE));
}

/******************************************************************************
 ***
 ***          Type computation for user defined functions:
 ***          --------------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTcond( te_info info, ntype *args)
 *
 * description:
 *    Here, we assume that the argument types (i.e. ONLY the predicate type!!)
 *    are either array types or type variables with identical Min and Max!
 *
 ******************************************************************************/

ntype *
NTCCTcond (te_info *err_info, ntype *args)
{
    ntype *pred;
    ntype *res = NULL;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYisProdOfArray (args), "NTCCond called with non-fixed predicate!");

    pred = TYgetProductMember (args, 0);
    TEassureBoolS ("predicate", pred);
    err_msg = TEfetchErrors ();

    if (err_msg != NULL) {
        CTIabort ("%s", err_msg);
    } else {
        res = TYmakeProductType (0);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTfuncond( te_info info, ntype *args)
 *
 * description:
 *    Here, we assume that the argument types (i.e. ONLY the predicate type!!)
 *    are either array types or type variables with identical Min and Max!
 *
 ******************************************************************************/

ntype *
NTCCTfuncond (te_info *err_info, ntype *args)
{
    ntype *pred, *rhs1, *rhs2;
    ntype *res = NULL;
    char *err_msg;
    constant *pred_val;
    ntype *non_alpha = NULL;

    DBUG_ENTER ();

    pred = TYgetProductMember (args, 0);
    rhs1 = TYgetProductMember (args, 1);
    rhs2 = TYgetProductMember (args, 2);

    if (TYisArray (pred)) {
        TEassureBoolS ("predicate", pred);
        err_msg = TEfetchErrors ();

        if (err_msg != NULL) {
            CTIabort ("%s", err_msg);
        }

        if (TYisAlpha (rhs1)) {
            rhs1 = TYmakeBottomType ("then branch computation does not terminate");
            non_alpha = rhs2;
        }
        if (TYisAlpha (rhs2)) {
            rhs2 = TYmakeBottomType ("else branch computation does not terminate");
            if (non_alpha == NULL) {
                non_alpha = rhs1;
            } else {
                non_alpha = NULL;
            }
        }
        if (TYisArray (rhs1) && TYisArray (rhs2)) {
            TEassureSameScalarType ("then branch", rhs1, "else branch", rhs2);
            err_msg = TEfetchErrors ();

            if (err_msg != NULL) {
                CTIabort ("%s", err_msg);
            }
        }

        if (TYisAKV (pred)) {
            pred_val = TYgetValue (pred);
            if (COisTrue (pred_val, FALSE) && !TYisBottom (rhs1)) {
                res = TYmakeProductType (1, TYcopyType (rhs1));
            } else if (COisFalse (pred_val, FALSE) && !TYisBottom (rhs2)) {
                res = TYmakeProductType (1, TYcopyType (rhs2));
            } else {
                /**
                 * This ensures that we tolerate non-terminating recursions
                 * if at least a terminating branch exists!
                 * I.e.: if( true) { rec_call(); } else { }; is ok!
                 *       if( ...) { rec_call(); } else { rec_call();} is not ok!
                 */
                res = TYmakeProductType (1, TYlubOfTypes (rhs1, rhs2));
            }
        } else {
            /**
             *  As bug 445 shows, we cannot just take the lub of rhs1 and rhs2 here!
             *  The problem is that lub( bottom, int{2}) = int, i.e., the value
             *  information gets lost!
             *  Hence, we have to return the non-alpha type in case exactly one
             *  had been alpha!
             */
            if (non_alpha == NULL) {
                res = TYmakeProductType (1, TYlubOfTypes (rhs1, rhs2));
            } else {
                res = TYmakeProductType (1, TYcopyType (non_alpha));
            }
        }

    } else if (TYisBottom (pred)) {
        res = TYmakeProductType (1, TYcopyType (pred));
    } else {
        res = TYmakeProductType (1, TYmakeAlphaType (NULL));
    }

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
