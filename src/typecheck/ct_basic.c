/*
 *
 * $Log$
 * Revision 1.1  2002/08/05 16:57:45  sbs
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include "dbug.h"

#include "new_types.h"
#include "sig_deps.h"
#include "ct_basic.h"

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
 *    ntype *NTCCTComputeType( ct_funptr CtFun, te_info *info, ntype *args)
 *
 * description:
 *   Either computes a return type or establishes a signature dependency.
 *
 ******************************************************************************/

ntype *
NTCCTComputeType (ct_funptr CtFun, te_info *info, ntype *args)
{
    ntype *res;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("NTCCTComputeType");

    DBUG_EXECUTE ("NTC", tmp_str = TYType2String (args, 0, 0););
    DBUG_PRINT ("NTC", ("computing type of %s \"%s\" applied to %s", TEGetKindStr (info),
                        TEGetNameStr (info), tmp_str));
    DBUG_EXECUTE ("NTC", Free (tmp_str););

    /*
     * as all CtFun s  operate on array types only, we try to fix as many argument
     * types as possible:
     */
    args = TYEliminateAlpha (args);

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

    if (TYIsProdOfArray (args)) {
        res = CtFun (info, args);
    } else {
        res = SDCreateSignatureDependency (CtFun, info, args);
    }

    DBUG_EXECUTE ("NTC", tmp_str = TYType2String (res, FALSE, 0););
    DBUG_PRINT ("NTC", ("yields %s", tmp_str));
    DBUG_EXECUTE ("NTC", Free (tmp_str););

    DBUG_RETURN (res);
}
