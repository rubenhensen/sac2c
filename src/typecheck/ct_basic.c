/*
 *
 * $Log$
 * Revision 1.3  2004/07/30 17:25:29  sbs
 * UGLY trick for smuggling through info * node instead of node * node:
 * casted. Compare UGLY counterpart in NTCcond (new_typecheck.c).
 *
 * Revision 1.2  2004/03/05 12:06:44  sbs
 * NTCCond added.
 *
 * Revision 1.1  2002/08/05 16:57:45  sbs
 * Initial revision
 *
 *
 */

#define NEW_INFO

#include <stdio.h>
#include <string.h>
#include "dbug.h"

#include "new_types.h"
#include "sig_deps.h"
#include "ct_basic.h"
#include "traverse.h"

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

/******************************************************************************
 ***
 ***          Type computation for user defined functions:
 ***          --------------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    ntype *NTCCond( te_info info, ntype *args)
 *
 * description:
 *    Here, we assume that the argument types (i.e. ONLY the predicate type!!)
 *    are either array types or type variables with identical Min and Max!
 *
 ******************************************************************************/

ntype *
NTCCond (te_info *err_info, ntype *args)
{
    ntype *pred, *res;
    node *cond;
    info *arg_info;

    DBUG_ENTER ("NTCCond");
    DBUG_ASSERT ((TYIsProdOfArray (args)), "NTCCond called with non-fixed predicate!");

    pred = TYGetProductMember (args, 0);
    TEAssureBoolS ("predicate", pred);

    cond = TEGetWrapper (err_info);
    arg_info = (info *)TEGetAssign (err_info);

#if 0
  /**
   * I'm not sure whether the sequel will work. Most liekely, 
   * we will have to make sure, that NO code is traversed more than once!!
   */
  if( TYIsAKV( pred) ) {
    if( COIsTrue( TYGetValue( pred), TRUE) ) {
      DBUG_PRINT( "NTC", ("traversing then branch only..."));
      COND_THEN( cond) = Trav( COND_THEN( cond), arg_info);
    } else {
      DBUG_PRINT( "NTC", ("traversing else branch only..."));
      COND_ELSE( cond) = Trav( COND_ELSE( cond), arg_info);
    }
  } else {
    DBUG_PRINT( "NTC", ("traversing then branch..."));
    COND_THEN( cond) = Trav( COND_THEN( cond), arg_info);
    DBUG_PRINT( "NTC", ("traversing else branch..."));
    COND_ELSE( cond) = Trav( COND_ELSE( cond), arg_info);
  }
#else
    DBUG_PRINT ("NTC", ("traversing then branch..."));
    COND_THEN (cond) = Trav (COND_THEN (cond), arg_info);
    DBUG_PRINT ("NTC", ("traversing else branch..."));
    COND_ELSE (cond) = Trav (COND_ELSE (cond), arg_info);
#endif

    res = TYMakeProductType (0);

    DBUG_RETURN (res);
}
