/*
 *
 * $Log$
 * Revision 1.5  2004/11/24 18:57:29  sbs
 * compiles
 *
 * Revision 1.4  2004/08/08 16:05:08  sah
 * fixed some includes.
 *
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
    ntype *res;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("NTCCTcomputeType");

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (args, 0, 0););
    DBUG_PRINT ("NTC", ("computing type of %s \"%s\" applied to %s", TEgetKindStr (info),
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

    if (TYisProdOfArray (args)) {
        res = CtFun (info, args);
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
    DBUG_ASSERT ((TYisProdOfArray (args)), "NTCCond called with non-fixed predicate!");

    pred = TYgetProductMember (args, 0);
    TEassureBoolS ("predicate", pred);

    cond = TEgetWrapper (err_info);
    arg_info = (info *)TEgetAssign (err_info);

#if 0
  /**
   * I'm not sure whether the sequel will work. Most liekely, 
   * we will have to make sure, that NO code is traversed more than once!!
   */
  if( TYisAKV( pred) ) {
    if( COIsTrue( TYgetValue( pred), TRUE) ) {
      DBUG_PRINT( "NTC", ("traversing then branch only..."));
      COND_THEN( cond) = TRAVdo( COND_THEN( cond), arg_info);
    } else {
      DBUG_PRINT( "NTC", ("traversing else branch only..."));
      COND_ELSE( cond) = TRAVdo( COND_ELSE( cond), arg_info);
    }
  } else {
    DBUG_PRINT( "NTC", ("traversing then branch..."));
    COND_THEN( cond) = TRAVdo( COND_THEN( cond), arg_info);
    DBUG_PRINT( "NTC", ("traversing else branch..."));
    COND_ELSE( cond) = TRAVdo( COND_ELSE( cond), arg_info);
  }
#else
    DBUG_PRINT ("NTC", ("traversing then branch..."));
    COND_THEN (cond) = TRAVdo (COND_THEN (cond), arg_info);
    DBUG_PRINT ("NTC", ("traversing else branch..."));
    COND_ELSE (cond) = TRAVdo (COND_ELSE (cond), arg_info);
#endif

    res = TYmakeProductType (0);

    DBUG_RETURN (res);
}
