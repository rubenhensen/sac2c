/*
 *
 * $Log$
 * Revision 1.10  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 1.9  2005/06/15 10:23:31  sbs
 * call history added to error messages.
 *
 * Revision 1.8  2005/01/10 17:27:06  cg
 * Converted error messages from Error.h to ctinfo.c
 *
 * Revision 1.7  2004/12/07 14:27:13  sbs
 * minor correction in DBUG output
 *
 * Revision 1.6  2004/11/24 18:47:53  sbs
 * compiles
 *
 * Revision 1.5  2004/10/26 10:46:29  sbs
 * FUNDEF_MOD printed as well know
 *
 * Revision 1.4  2003/09/09 14:56:11  sbs
 * extended type error reporting added
 *
 * Revision 1.3  2002/09/06 15:16:40  sbs
 * FUNDEF_RETURN now set properly?!
 *
 * Revision 1.2  2002/09/03 14:41:45  sbs
 * DupTree machanism for duplicating condi funs established
 *
 * Revision 1.1  2002/08/05 16:57:48  sbs
 * Initial revision
 *
 *
 */

#include "ct_fun.h"
#include "dbug.h"
#include "ctinfo.h"
#include "internal_lib.h"
#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "ssi.h"
#include "new_typecheck.h"
#include "type_errors.h"
#include "type_utils.h"
#include "specialize.h"
#include "namespaces.h"

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
 ***
 ***          local helper functions
 ***          ----------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    dft_res *DispatchFunType( node *wrapper, ntype *args)
 *
 * description:
 *    This function encapsulates TYDispatchFunType.
 *    It basically extracts the wrapper type and calls that function with it.
 *    Furthermore, some DBUG output may be generated.
 *
 ******************************************************************************/

static dft_res *
DispatchFunType (node *wrapper, ntype *args)
{
    dft_res *res;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("DispatchFunType");

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (args, 0, 0););
    DBUG_PRINT ("NTC", ("dispatching %s for %s", CTIitemName (wrapper), tmp_str));

    res = TYdispatchFunType (FUNDEF_WRAPPERTYPE (wrapper), args);

    DBUG_EXECUTE ("NTC", tmp_str = TYdft_res2DebugString (res););
    DBUG_PRINT ("NTC", ("%s", tmp_str));
    DBUG_EXECUTE ("NTC", ILIBfree (tmp_str););

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    void TriggerTypeChecking( dft_res *dft)
 *
 * description:
 *
 ******************************************************************************/

static void
TriggerTypeChecking (dft_res *dft)
{
    int i;

    DBUG_ENTER ("TriggerTypeChecking");

    /*
     * trigger type checking of the partials:
     */
    if (dft->num_partials > 0) {
        for (i = 0; i < dft->num_partials; i++) {
            dft->partials[i] = NTCtriggerTypeCheck (dft->partials[i]);
        }
    }

    /*
     * trigger the typechecking the defining function:
     */
    if (dft->def != NULL) {
        dft->def = NTCtriggerTypeCheck (dft->def);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 ***
 ***          Exported helper functions:
 ***          --------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    dft_res *NTCCTdispatchFunType( node *wrapper, ntype *args)
 *
 * description:
 *    This function encapsulates TYDispatchFunType.
 *    It basically extracts the wrapper type and calls that function with it.
 *    Furthermore, some DBUG output may be generated.
 *
 ******************************************************************************/

dft_res *
NTCCTdispatchFunType (node *wrapper, ntype *args)
{
    dft_res *res;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("NTCCTdispatchFunType");

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (args, 0, 0););
    DBUG_PRINT ("NTC", ("dispatching %s for %s", CTIitemName (wrapper), tmp_str));

    res = TYdispatchFunType (FUNDEF_WRAPPERTYPE (wrapper), args);

    DBUG_EXECUTE ("NTC", tmp_str = TYdft_res2DebugString (res););
    DBUG_PRINT ("NTC", ("%s", tmp_str));
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
 *    ntype *NTCCTudf( te_info info, ntype *args)
 *
 * description:
 *    Here, we assume that all argument types are either array types or
 *    type variables with identical Min and Max!
 *
 ******************************************************************************/

ntype *
NTCCTudf (te_info *info, ntype *args)
{
    ntype *res, *res_mem;
    tvar *alpha;
    node *fundef, *assign;
    dft_res *dft_res;
    te_info *old_info_chn;
    char *tmp, *tmp2;
    int i;

    DBUG_ENTER ("NTCCTudf");
    DBUG_ASSERT ((TYisProdOfArray (args)), "NTCCTudf called with non-fixed args!");

    fundef = TEgetWrapper (info);
    assign = TEgetAssign (info);

    old_info_chn = global.act_info_chn;
    global.act_info_chn = info;
    DBUG_PRINT ("NTC_INFOCHN", ("global.act_info_chn set to %p", info));

    if (FUNDEF_ISLACFUN (fundef)) {
        /*
         * specialize it, trigger its typecheck, and pick its return type:
         */
        fundef = SPEChandleLacFun (fundef, assign, args);
        fundef = NTCtriggerTypeCheck (fundef);
        res = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));

    } else {
        /*
         * we are dealing with a (overloaded) udf. Therefore, we have to dispatch
         * the funap in order to find the function instances involved and the result type.
         */
        dft_res = DispatchFunType (fundef, args);

        if (dft_res == NULL) {
            /*
             * this is a 0-ary function!!
             * Therefore, the fundef we are looking for must be behind FUNDEF_IMPL
             * of the wrapper function (cf. create_wrappers.c)!!!
             * Trigger its type check, and pick its return type:
             */
            fundef = FUNDEF_IMPL (fundef);
            fundef = NTCtriggerTypeCheck (fundef);
            res = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));

        } else {
            /*
             * create specializations (if appropriate), trigger the type
             * check of all potentially involved fundefs and extract the
             * return type from the dft_res structure:
             * Note that we pass NULL for the return type as we do not want
             * to fix it but infer it.
             */
            dft_res = SPEChandleDownProjections (dft_res, fundef, args, NULL);

            if ((dft_res->def == NULL) && (dft_res->num_partials == 0)) {
                /*
                 * no match at all!
                 */
                CTIerrorLine (global.linenum,
                              "No matching definition found for the application "
                              " of \"%s\" to arguments %s",
                              CTIitemName (fundef), TYtype2String (args, FALSE, 0));
                global.act_info_chn = TEgetParent (global.act_info_chn);
                TEextendedAbort ();
            }

            TriggerTypeChecking (dft_res);

            res = TYcopyType (dft_res->type);
            DBUG_ASSERT ((res != NULL),
                         "HandleDownProjections did not return proper return type");
            TYfreeDft_res (dft_res);
        }
    }

    global.act_info_chn = old_info_chn;
    DBUG_PRINT ("NTC_INFOCHN", ("global.act_info_chn reset to %p", global.act_info_chn));

    tmp = TYtype2String (args, FALSE, 0);
    TEhandleError (global.linenum, " -- in %s::%s%s@", NSgetName (FUNDEF_NS (fundef)),
                   FUNDEF_NAME (fundef), tmp);
    tmp = ILIBfree (tmp);
    tmp2 = TEfetchErrors ();

    for (i = 0; i < TYgetProductSize (res); i++) {
        res_mem = TYgetProductMember (res, i);
        if (TYisBottom (res_mem)) {
            TYextendBottomError (res_mem, tmp2);
        } else if (TYisAlpha (res_mem)) {
            alpha = TYgetAlpha (res_mem);
            if (SSIgetMin (alpha) != NULL) {
                res_mem = SSIgetMin (alpha);
                if (TYisBottom (res_mem)) {
                    TYextendBottomError (res_mem, tmp2);
                }
            }
        }
    }

    tmp2 = ILIBfree (tmp2);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTudfDispatched( te_info info, ntype *args)
 *
 * description:
 *    Here, we assume that all argument types are either array types or
 *    type variables with identical Min and Max!
 *
 ******************************************************************************/

ntype *
NTCCTudfDispatched (te_info *info, ntype *args)
{
    node *fundef;
    ntype *res;

    DBUG_ENTER ("NTCCTudfDispatched");

    fundef = TEgetWrapper (info);
    res = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));

    DBUG_RETURN (res);
}
