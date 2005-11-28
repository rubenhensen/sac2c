/*
 * $Log$
 */

#include <stdio.h>
#include <string.h>

#include "ct_basic.h"
#include "dbug.h"
#include "internal_lib.h"

#include "new_types.h"
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
 *
 * description:
 *   Either computes a return type or establishes a signature dependency.
 *
 ******************************************************************************/

ntype *
NTCCTcomputeType (ct_funptr CtFun, te_info *info, ntype *args)
{
    ntype *res, *bottom;
    int i, num_res;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("NTCCTcomputeType");

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (args, 0, 0););
    DBUG_PRINT ("NTC",
                ("computing type of %s \"%s%s%s\" applied to %s", TEgetKindStr (info),
                 ((TEgetKind (info) == TE_udf) || (TEgetKind (info) == TE_foldf)
                    ? TEgetModStr (info)
                    : ""),
                 ((TEgetKind (info) == TE_udf) || (TEgetKind (info) == TE_foldf) ? "::"
                                                                                 : ""),
                 TEgetNameStr (info), tmp_str));
    DBUG_EXECUTE ("NTC", ILIBfree (tmp_str););

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
        if (TYisProdOfArray (args)) {
            res = CtFun (info, args);
        } else {
            /**
             * args contain bottom which needs to be propagated.
             */
            bottom = TYgetBottom (args);
            DBUG_ASSERT ((bottom != NULL), "inconsistent type in NTCCTcomputeType!");
            num_res = TEgetNumRets (info);
            res = TYmakeEmptyProductType (num_res);
            for (i = 0; i < num_res; i++) {
                res = TYsetProductMember (res, i, TYcopyType (bottom));
            }
        }
    } else {
        res = SDcreateSignatureDependency (CtFun, info, args);
    }

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (res, FALSE, 0););
    DBUG_PRINT ("NTC", ("yields %s", tmp_str));
    DBUG_EXECUTE ("NTC", ILIBfree (tmp_str););

    DBUG_RETURN (res);
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

    DBUG_ENTER ("NTCCTcond");
    DBUG_ASSERT ((TYisProdOfArray (args)), "NTCCond called with non-fixed predicate!");

    pred = TYgetProductMember (args, 0);
    TEassureBoolS ("predicate", pred);
    err_msg = TEfetchErrors ();

    if (err_msg != NULL) {
        CTIabort (err_msg);
    } else {
        res = TYmakeProductType (0);
    }

    DBUG_RETURN (res);
}
