/*
 *
 * $Log$
 * Revision 1.6  2002/08/05 17:00:38  sbs
 * first alpha version of the new type checker !!
 *
 * Revision 1.5  2002/05/31 14:43:06  sbs
 * CRTWRPlet added
 *
 * Revision 1.4  2002/03/12 15:13:32  sbs
 * CRTWRPxxxx traversal function added.
 *
 * Revision 1.3  2002/03/05 15:51:17  sbs
 * *** empty log message ***
 *
 * Revision 1.2  2002/03/05 15:40:40  sbs
 * CRTWRP traversal embedded.
 *
 * Revision 1.1  2002/03/05 13:59:27  sbs
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "internal_lib.h"
#include "traverse.h"
#include "globals.h"

#include "user_types.h"
#include "new_types.h"

#include "create_wrappers.h"

#define INFO_CRTWRP_WRAPPERS(n) (n->node[0])
#define INFO_CRTWRP_EXPRETS(n) (n->flag)

/******************************************************************************
 *
 * function:
 *    node *CreateWrappers(node *arg_node)
 *
 * description:
 *    creates wrapper functions for all functions found. These wrapper functions
 *     - obtain generic argument / result types (old types): _unknown_[*]
 *     - obtain overloaded function types (new types) that include all
 *       function definitions found
 *     - get a STATUS ST_wrapperfun
 *     - have a NULL BODY
 *     - are inserted into the N_fundef chain
 *    Furthermore,
 *     - all fundefs obtain a non-overloaded function type (new type)
 *     - all fundefs obtain a copy of their return type (product type) attached
 *       to the fundef node directly. (Redundant but convenient in new_typecheck.c!)
 *     - all function applications obtain a backref to the wrapper function
 *       responsible for the function dspatch
 *
 ******************************************************************************/

node *
CreateWrappers (node *arg_node)
{
    funtab *tmp_tab;
    node *info_node;

    DBUG_ENTER ("CreateWrappers");

    tmp_tab = act_tab;
    act_tab = crtwrp_tab;

    info_node = MakeInfo ();

    arg_node = Trav (arg_node, info_node);

    info_node = FreeNode (info_node);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *FindWrapper(char *name, int num_args, int num_rets, node *wrappers)
 *
 * description:
 *    searches for a wrapper function in the N_fundef chain "wrappers" that
 *    matches the given name (name), the given number of arguments (num_args),
 *    and the given number of return values (num_rets). This matching mechanism
 *    does allow for dots to be used "on both sides", i.e., for non-fixed numbers
 *    of arguments and non-fixed numbers of return values.
 *    Once a matching wrapper function is found, its N_fundef is returned.
 *    Otherwise, FindWrapper returns NULL.
 *
 *    NOTE HERE, that only the first match is returned! There is no check that
 *    ensures the "best fit"! For example, assuming the following chain of wrappers:
 *      (ptr_a) ->  <alpha> tutu( <beta> x, ...)
 *      (ptr_b) ->  <alpha> tutu( <beta> a, <gamma> b)
 *
 *    FindWrapper( "tutu", 2, 1, (ptr_a))     yields (ptr_a)!!!  Only a call
 *    FindWrapper( "tutu", 2, 1, (ptr_b))     yields (ptr_b)!
 *
 ******************************************************************************/

static node *
FindWrapper (char *name, int num_args, int num_rets, node *wrappers)
{
    int found = FALSE;
    int last_parm_is_dots = FALSE;
    int last_res_is_dots = FALSE;
    int num_parms, num_res;

    DBUG_ENTER ("FindWrapper");

    DBUG_PRINT ("CRTWRP", ("Searching for %s %d args %d rets", name, num_args, num_rets));

    while (wrappers && !found) {
        num_parms = CountArgs (FUNDEF_ARGS (wrappers));
        num_res = CountTypes (FUNDEF_TYPES (wrappers));
        last_parm_is_dots = HasDotArgs (FUNDEF_ARGS (wrappers));
        last_res_is_dots = HasDotTypes (FUNDEF_TYPES (wrappers));
        DBUG_PRINT ("CRTWRP", (" ... checking %s %s%d args %s%d rets",
                               FUNDEF_NAME (wrappers), (last_parm_is_dots ? ">=" : ""),
                               num_parms, (last_res_is_dots ? ">=" : ""), num_res));
        if ((strcmp (name, FUNDEF_NAME (wrappers)) == 0)
            && ((num_res == num_rets) || (last_res_is_dots && (num_res <= num_rets)))
            && ((num_parms == num_args)
                || (last_parm_is_dots && (num_parms <= num_args)))) {
            found = TRUE;
        } else {
            wrappers = FUNDEF_NEXT (wrappers);
        }
    }

    DBUG_RETURN (wrappers);
}

/******************************************************************************
 *
 * function:
 *    node *CreateWrapperFor(node *fundef)
 *
 * description:
 *   using a given fundef (fundef) as a template, a wrapper function is
 *   created. It consists of a duplicate of the function header given
 *   whose types (old types!!) have been modified to "_unknown_[*]"
 *   unless they are "void" or "...".
 *   The STATUS of the function returned is set to ST_wrapperfun and its
 *   body is NULL!
 *
 ******************************************************************************/

static node *
CreateWrapperFor (node *fundef)
{
    node *body, *wrapper, *args;
    types *rettypes;

    DBUG_ENTER ("CreateWrapperFor");
    DBUG_PRINT ("CRTWRP",
                ("Creating wrapper for %s %s%d args %d rets", FUNDEF_NAME (fundef),
                 (HasDotArgs (FUNDEF_ARGS (fundef)) ? ">=" : ""),
                 CountArgs (FUNDEF_ARGS (fundef)), CountTypes (FUNDEF_TYPES (fundef))));

    body = FUNDEF_BODY (fundef);
    FUNDEF_BODY (fundef) = NULL;
    wrapper = DupNode (fundef);
    FUNDEF_BODY (fundef) = body;

    /*
     * marking the wrapper function:
     */
    FUNDEF_STATUS (wrapper) = ST_wrapperfun;

    /*
     * setting the wrapper function's return types to _unknown_[*]
     * unless the function turns out to be void, or their basetype
     * turns out to be T_dots:
     */
    if (TYPES_BASETYPE (FUNDEF_TYPES (wrapper)) != T_void) {
        rettypes = FUNDEF_TYPES (wrapper);
        while (rettypes) {
            if (TYPES_BASETYPE (rettypes) != T_dots) {
                TYPES_DIM (rettypes) = ARRAY_OR_SCALAR;
                TYPES_BASETYPE (rettypes) = T_unknown;
            }
            rettypes = TYPES_NEXT (rettypes);
        }
    }

    /*
     * setting the wrapper function's parameter types to _unknown_[*]
     * unless their basetype turns out to be T_dots :
     */
    args = FUNDEF_ARGS (wrapper);
    while (args) {
        if (TYPES_BASETYPE (ARG_TYPE (args)) != T_dots) {
            TYPES_DIM (ARG_TYPE (args)) = ARRAY_OR_SCALAR;
            TYPES_BASETYPE (ARG_TYPE (args)) = T_unknown;
        }
        args = ARG_NEXT (args);
    }

    /*
     * Now, we implement a dirty trick for 0 ary functions.
     * Since we do not allow them to be overloaded, we include
     * a pointer to the fundef here.
     * It is used when dispatching such functions (cf. new_types.c)!!
     */
    if (FUNDEF_ARGS (wrapper) == NULL) {
        FUNDEF_RETURN (wrapper) = fundef;
    }

    DBUG_RETURN (wrapper);
}

/******************************************************************************
 *
 * function:
 *    ntype *CreateFuntype(node *fundef)
 *
 * description:
 *    creates a function type from the given arg/return types. While doing so,
 *    a copy of the return type is put into FUNDEF_RET_TYPE( fundef) !!!!
 *    This shortcut is very useful during type inference, when the return
 *    statement is reached (cf. NTCreturn). Otherwise, it would be necessary
 *    to dig through the intersection type in order to find the return type.
 *
 ******************************************************************************/

static ntype *
FuntypeFromArgs (ntype *res, node *args, node *fundef)
{
    DBUG_ENTER ("FuntypeFromArgs");

    if (args != NULL) {
        res = FuntypeFromArgs (res, ARG_NEXT (args), fundef);
        res = TYMakeFunType (TYOldType2Type (ARG_TYPE (args)), res, fundef);
    }

    DBUG_RETURN (res);
}

ntype *
CreateFuntype (node *fundef)
{
    int num_rets;
    types *old_ret;
    ntype *res;
    int i;

    DBUG_ENTER ("CreateFuntype");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "CreateFuntype applied to non-fundef node!");

    old_ret = FUNDEF_TYPES (fundef);
    num_rets = CountTypes (old_ret);

    res = TYMakeEmptyProductType (num_rets);

    for (i = 0; i < num_rets; i++) {
        res = TYSetProductMember (res, i, TYMakeAlphaType (TYOldType2Type (old_ret)));
        old_ret = TYPES_NEXT (old_ret);
    }

    FUNDEF_RET_TYPE (fundef) = TYCopyType (res);

    res = FuntypeFromArgs (res, FUNDEF_ARGS (fundef), fundef);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPmodul(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPmodul (node *arg_node, node *arg_info)
{
    node *wrappers;

    DBUG_ENTER ("CRTWRPmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }
    wrappers = INFO_CRTWRP_WRAPPERS (arg_info);
    MODUL_FUNS (arg_node) = AppendFundef (wrappers, MODUL_FUNS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPfundef (node *arg_node, node *arg_info)
{
    node *wrapper;
    int num_args;

    DBUG_ENTER ("CRTWRPfundef");

    num_args = CountArgs (FUNDEF_ARGS (arg_node));
    wrapper = FindWrapper (FUNDEF_NAME (arg_node), num_args,
                           CountTypes (FUNDEF_TYPES (arg_node)),
                           INFO_CRTWRP_WRAPPERS (arg_info));
    if (wrapper == NULL) {
        wrapper = CreateWrapperFor (arg_node);
        FUNDEF_NEXT (wrapper) = INFO_CRTWRP_WRAPPERS (arg_info);
        INFO_CRTWRP_WRAPPERS (arg_info) = wrapper;
    }

    FUNDEF_TYPE (arg_node) = CreateFuntype (arg_node);
    FUNDEF_TYPE (wrapper) = TYMakeOverloadedFunType (TYCopyType (FUNDEF_TYPE (arg_node)),
                                                     FUNDEF_TYPE (wrapper));
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * Now, we do have wrappers for all fundefs!
     * On our way back up, we traverse the body in order to insert the backrefs
     * to the appropriate wrappers.
     */

    /*
     * Usually, we expect all functions applied in the function body to return a
     * single value. Therefore, INFO_CRTWRP_EXPRETS is set to 1.
     * Only on RHSs of N_let nodes with more than one Id on the LHS, this
     * value is changed (and reset afterwards!) -> CRTWRPlet.
     */
    INFO_CRTWRP_EXPRETS (arg_info) = 1;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPlet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPlet (node *arg_node, node *arg_info)
{
    int old_exprets;

    DBUG_ENTER ("CRTWRPlet");

    old_exprets = INFO_CRTWRP_EXPRETS (arg_info);
    INFO_CRTWRP_EXPRETS (arg_info) = CountIds (LET_IDS (arg_node));

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }
    INFO_CRTWRP_EXPRETS (arg_info) = old_exprets;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPap(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPap (node *arg_node, node *arg_info)
{
    int num_args;
    node *wrapper;

    DBUG_ENTER ("CRTWRPap");

    num_args = CountExprs (AP_ARGS (arg_node));
    wrapper = FindWrapper (AP_NAME (arg_node), num_args, INFO_CRTWRP_EXPRETS (arg_info),
                           INFO_CRTWRP_WRAPPERS (arg_info));

    if (wrapper == NULL) {
        ABORT (NODE_LINE (arg_node),
               ("No definition found for a function \"%s\" that expects %i argument(s)"
                " and yields %i return value(s)",
                AP_NAME (arg_node), num_args, INFO_CRTWRP_EXPRETS (arg_info)));
    } else {
        AP_FUNDEF (arg_node) = wrapper;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPNwithop(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPNwithop (node *arg_node, node *arg_info)
{
    int num_args;
    node *wrapper;

    DBUG_ENTER ("CRTWRPNwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        break;

    case WO_modarray:
        NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        break;
    case WO_foldfun:
        NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);

        num_args = 2;
        wrapper
          = FindWrapper (NWITHOP_FUN (arg_node), 2, 1, INFO_CRTWRP_WRAPPERS (arg_info));

        if (wrapper == NULL) {
            ABORT (NODE_LINE (arg_node),
                   ("No definition found for a function \"%s\" that expects 2 arguments"
                    " and yields 1 return value",
                    NWITHOP_FUN (arg_node)));
        } else {
            NWITHOP_FUNDEF (arg_node) = wrapper;
        }
        break;

    case WO_foldprf:
        if (NWITHOP_NEUTRAL (arg_node) != NULL) {
            NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        }
        break;
    default:
        DBUG_ASSERT (FALSE, "corrupted WL tag found");
        break;
    }

    DBUG_RETURN (arg_node);
}
