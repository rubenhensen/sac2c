/*
 *
 * $Log$
 * Revision 1.19  2004/09/30 15:12:35  sbs
 * eliminated FunTypes from ALL but wrapper functions
 * (memory concerns!)
 * Now, the function signatures of individual instances are
 * stored in the AVIS_TYPE and FUNDEF_RET_TYPE only!!!!!
 *
 * Revision 1.18  2004/07/30 17:29:21  sbs
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.17  2004/03/05 12:05:15  sbs
 * Now, the wrapper function headers are inserted in to the fundefs immediately
 *
 * Revision 1.16  2004/02/20 08:14:00  mwe
 * now functions with and without body are separated
 * changed tree traversal (added traverse of MODUL_FUNDECS)
 *
 * Revision 1.15  2003/11/18 17:21:57  dkr
 * NWITHOP_DEFAULT added
 *
 * Revision 1.14  2003/09/08 13:14:33  sbs
 * some more debug prints added
 *
 * Revision 1.13  2002/10/31 13:41:51  sbs
 * ... args are tagged IS_REFERENCE FALSE now.
 *
 * Revision 1.12  2002/10/30 12:13:38  sbs
 * handling of ... args / rets corrected..
 *
 * Revision 1.11  2002/10/28 19:04:06  dkr
 * CreateWrapperFor(): wrapper functions are never external now
 *
 * Revision 1.10  2002/10/18 14:32:04  sbs
 * create wrappers now is responsible for setting IS_REFERENCE flags
 * appropriately ! This information will be used by the object/reference
 * parameter
 * resolution in objects.c
 *
 * Revision 1.8  2002/09/06 15:16:40  sbs
 * FUNDEF_RETURN now set properly?!
 *
 * Revision 1.7  2002/08/15 18:47:11  dkr
 * uses LUT now
 *
 * Revision 1.6  2002/08/05 17:00:38  sbs
 * first alpha version of the new type checker!!
 *
 * Revision 1.5  2002/05/31 14:43:06  sbs
 * CRTWRPlet added
 *
 * Revision 1.4  2002/03/12 15:13:32  sbs
 * CRTWRPxxxx traversal function added.
 *
 * Revision 1.2  2002/03/05 15:40:40  sbs
 * CRTWRP traversal embedded.
 *
 * Revision 1.1  2002/03/05 13:59:27  sbs
 * Initial revision
 *
 */

#define NEW_INFO

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

/*******************************************************************************
 *
 */

/**
 * INFO structure
 */
struct INFO {
    LUT_t wrapperfuns;
    int exprets;
};

/**
 * INFO macros
 */

#define INFO_CRTWRP_WRAPPERFUNS(n) ((n)->wrapperfuns)
#define INFO_CRTWRP_EXPRETS(n) ((n)->exprets)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_CRTWRP_WRAPPERFUNS (result) = NULL;
    INFO_CRTWRP_EXPRETS (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *CreateWrappers( node *arg_node)
 *
 * description:
 *   Creates wrapper functions for all functions found. These wrapper functions
 *     - obtain generic argument / result types (old types): _unknown_[*]
 *     - obtain overloaded function types (new types) that include all
 *       function definitions found
 *     - get a STATUS ST_wrapperfun
 *     - have a NULL BODY
 *     - are inserted into the N_fundef chain
 *   Furthermore,
 *     - all fundefs obtain a non-overloaded function type (new type)
 *     - all fundefs obtain a copy of their return type (product type) attached
 *       to the fundef node directly.
 *       (Redundant but convenient in new_typecheck.c!)
 *     - all function applications obtain a backref to the wrapper function
 *       responsible for the function dspatch
 *
 ******************************************************************************/

node *
CreateWrappers (node *arg_node)
{
    funtab *tmp_tab;
    info *info_node;

    DBUG_ENTER ("CreateWrappers");

    tmp_tab = act_tab;
    act_tab = crtwrp_tab;

    info_node = MakeInfo ();
    arg_node = Trav (arg_node, info_node);
    info_node = FreeInfo (info_node);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *FindWrapper( char *name, int num_args, int num_rets, LUT_t lut)
 *
 * description:
 *   Searches for a wrapper function in the given look-up-table (lut) that
 *   matches the given name (name), the given number of arguments (num_args),
 *   and the given number of return values (num_rets). This matching mechanism
 *   does allow for dots to be used "on both sides", i.e., for non-fixed numbers
 *   of arguments and non-fixed numbers of return values.
 *   Once a matching wrapper function is found, its N_fundef is returned.
 *   Otherwise, FindWrapper returns NULL.
 *
 *   NOTE HERE, that only the first match is returned! There is no check that
 *   ensures the "best fit"! For example, assuming a lut containing the
 *   following wrappers:
 *      ("tutu", ptr_a), where  (ptr_a)  ->  <alpha> tutu( <beta> x, ...)
 *      ("tutu", ptr_b), where  (ptr_b)  ->  <alpha> tutu( <beta> a, <gamma> b)
 *   Then,   FindWrapper( "tutu", 2, 1, lut)   yields (ptr_a)!!!
 *
 ******************************************************************************/

static node *
FindWrapper (char *name, int num_args, int num_rets, LUT_t lut)
{
    int last_parm_is_dots;
    int last_res_is_dots;
    int num_parms, num_res;
    node **wrapper_p;
    node *wrapper = NULL;
    bool found = FALSE;

    DBUG_ENTER ("FindWrapper");

    DBUG_PRINT ("CRTWRP", ("Searching for %s %d args %d rets", name, num_args, num_rets));

    /* initial search for wrapper in LUT */
    wrapper_p = (node **)SearchInLUT_S (lut, name);
    while ((wrapper_p != NULL) && (!found)) {
        wrapper = *wrapper_p;
        last_parm_is_dots = HasDotArgs (FUNDEF_ARGS (wrapper));
        last_res_is_dots = HasDotTypes (FUNDEF_TYPES (wrapper));
        num_parms = (last_parm_is_dots ? CountArgs (FUNDEF_ARGS (wrapper)) - 1
                                       : CountArgs (FUNDEF_ARGS (wrapper)));
        num_res = (last_res_is_dots ? CountTypes (FUNDEF_TYPES (wrapper)) - 1
                                    : CountTypes (FUNDEF_TYPES (wrapper)));
        DBUG_PRINT ("CRTWRP", (" ... checking %s %s%d args %s%d rets",
                               FUNDEF_NAME (wrapper), (last_parm_is_dots ? ">=" : ""),
                               num_parms, (last_res_is_dots ? ">=" : ""), num_res));
        if (((num_res == num_rets) || (last_res_is_dots && (num_res <= num_rets)))
            && ((num_parms == num_args)
                || (last_parm_is_dots && (num_parms <= num_args)))) {
            found = TRUE;
        } else {
            /* search for next wrapper in LUT */
            wrapper_p = (node **)SearchInLUT_NextS ();
        }
    }

    if (wrapper_p == NULL) {
        wrapper = NULL;
    }

    DBUG_RETURN (wrapper);
}

/******************************************************************************
 *
 * function:
 *    node *CreateWrapperFor( node *fundef)
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
                 (HasDotArgs (FUNDEF_ARGS (fundef)) ? CountArgs (FUNDEF_ARGS (fundef)) - 1
                                                    : CountArgs (FUNDEF_ARGS (fundef))),
                 CountTypes (FUNDEF_TYPES (fundef))));

    body = FUNDEF_BODY (fundef);
    FUNDEF_BODY (fundef) = NULL;
    wrapper = DupNode (fundef);
    FUNDEF_BODY (fundef) = body;

    /*
     * marking the wrapper function:
     */
    FUNDEF_STATUS (wrapper) = ST_wrapperfun;

    /*
     * wrappers of external function are not external
     *   -> remove external module name!!!
     */
    if (!strcmp (FUNDEF_MOD (wrapper), EXTERN_MOD_NAME)) {
        FUNDEF_MOD (wrapper) = MAIN_MOD_NAME;
    }

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
        FUNDEF_IMPL (wrapper) = fundef;
    }

    DBUG_RETURN (wrapper);
}

/******************************************************************************
 *
 * function:
 *    ntype *CreateFuntype( node *fundef)
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

    if ((args != NULL) && (TYPES_BASETYPE (ARG_TYPE (args)) != T_dots)) {
        res = FuntypeFromArgs (res, ARG_NEXT (args), fundef);
        res = TYMakeFunType (TYOldType2Type (ARG_TYPE (args)), res, fundef);
    }

    DBUG_RETURN (res);
}

ntype *
CreateFunRettype (types *old_ret)
{
    int num_rets;
    ntype *res, *maxtype;
    int i;

    DBUG_ENTER ("CreateFunRettype");

    num_rets = (HasDotTypes (old_ret) ? CountTypes (old_ret) - 1 : CountTypes (old_ret));

    res = TYMakeEmptyProductType (num_rets);

    for (i = 0; i < num_rets; i++) {
        maxtype = TYOldType2ScalarType (old_ret);
        if (maxtype != NULL) {
            maxtype = TYMakeAUD (maxtype);
        }
        res = TYSetProductMember (res, i, TYMakeAlphaType (maxtype));
        old_ret = TYPES_NEXT (old_ret);
    }

    DBUG_RETURN (res);
}

ntype *
CreateFuntype (node *fundef)
{
    ntype *res;

    DBUG_ENTER ("CreateFuntype");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "CreateFuntype applied to non-fundef node!");
    DBUG_ASSERT ((FUNDEF_RET_TYPE (fundef) != NULL),
                 "CreateFuntype called without FUNDEF_RET_TYPE!");

    res = FuntypeFromArgs (TYCopyType (FUNDEF_RET_TYPE (fundef)), FUNDEF_ARGS (fundef),
                           fundef);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    node *TagReferenceArgs( node *act_args, node *args)
 *
 * description:
 *    Assumes act_args to be an N_exprs chain of actual arguments and args to
 *    be an N_args chain of formal parameters of the same or less (...) length.
 *    For each param that is tagged as ST_reference, the according N_id expr
 *      is flagged as IS_REFERENCE TRUE and IS_READ_ONLY FALSE;
 *    a ST_readonly_reference param is flagged IS_REFERENCE TRUE and
 *      IS_READ_ONLY TRUE;
 *    all others are flagged IS_REFERENCE FALSE !
 *
 ******************************************************************************/

static node *
TagReferenceArgs (node *act_args, node *args)
{
    node *exprs;

    DBUG_ENTER ("TagReferenceArgs");

    exprs = act_args;
    while ((args != NULL) && (TYPES_BASETYPE (ARG_TYPE (args)) != T_dots)) {
        DBUG_ASSERT ((exprs != NULL), "TagReferenceArgs called with act_args and args of "
                                      "different length");
        if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_id) {
            if (ARG_ATTRIB (args) == ST_reference) {
                SET_FLAG (ID, EXPRS_EXPR (exprs), IS_REFERENCE, TRUE);
                SET_FLAG (ID, EXPRS_EXPR (exprs), IS_READ_ONLY, FALSE);
            } else if (ARG_ATTRIB (args) == ST_readonly_reference) {
                SET_FLAG (ID, EXPRS_EXPR (exprs), IS_REFERENCE, TRUE);
                SET_FLAG (ID, EXPRS_EXPR (exprs), IS_READ_ONLY, TRUE);
            } else {
                SET_FLAG (ID, EXPRS_EXPR (exprs), IS_REFERENCE, FALSE);
            }
        }
        args = ARG_NEXT (args);
        exprs = EXPRS_NEXT (exprs);
    }
    /*
     * Finally, all surplus arguments ( in case of '...')
     * have to be tagged IS_REFERENCE = FALSE
     */
    while (exprs != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_id) {
            SET_FLAG (ID, EXPRS_EXPR (exprs), IS_REFERENCE, FALSE);
        }
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (act_args);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPmodul(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

static node *
ConsFundefs (node *fundefs, node *fundef)
{
    DBUG_ENTER ("ConsFundefs");

    FUNDEF_NEXT (fundef) = fundefs;

    DBUG_RETURN (fundef);
}

node *
CRTWRPmodul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CRTWRPmodul");

    DBUG_ASSERT ((MODUL_WRAPPERFUNS (arg_node) == NULL),
                 "MODUL_WRAPPERFUNS is not NULL!");
    INFO_CRTWRP_WRAPPERFUNS (arg_info) = MODUL_WRAPPERFUNS (arg_node) = GenerateLUT ();

    /**
     * First, we traverse the external functions. As these per definition
     * DO NOT have bodies, it does not matter that not all functions have
     * been seen prior to our way back up!
     */
    if (MODUL_FUNDECS (arg_node) != NULL) {
        MODUL_FUNDECS (arg_node) = Trav (MODUL_FUNDECS (arg_node), arg_info);
    }
    /**
     * Now, we traverse the local fundefs. Once we reach the last fundef,
     * we know for sure, that we have seen ALL functions (including the externals)!
     */
    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }
    /**
     * Finally, we insert the wrapper functions into the fundef chain:
     */
    MODUL_FUNS (arg_node)
      = FoldLUT_S (INFO_CRTWRP_WRAPPERFUNS (arg_info), MODUL_FUNS (arg_node),
                   (void *(*)(void *, void *))ConsFundefs);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPfundef(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPfundef (node *arg_node, info *arg_info)
{
    node *wrapper;
    int num_args, num_rets;
    bool dot_args, dot_rets;
    int num_wr_args, num_wr_rets;

    DBUG_ENTER ("CRTWRPfundef");

    dot_args = HasDotArgs (FUNDEF_ARGS (arg_node));
    dot_rets = HasDotTypes (FUNDEF_TYPES (arg_node));
    num_args = (dot_args ? CountArgs (FUNDEF_ARGS (arg_node)) - 1
                         : CountArgs (FUNDEF_ARGS (arg_node)));
    num_rets = (dot_rets ? CountTypes (FUNDEF_TYPES (arg_node)) - 1
                         : CountTypes (FUNDEF_TYPES (arg_node)));
    DBUG_PRINT ("CRTWRP", ("----- Processing function %s:%s: -----",
                           FUNDEF_MOD (arg_node), FUNDEF_NAME (arg_node)));
    wrapper = FindWrapper (FUNDEF_NAME (arg_node), num_args, num_rets,
                           INFO_CRTWRP_WRAPPERFUNS (arg_info));
    if (wrapper == NULL) {
        wrapper = CreateWrapperFor (arg_node);
        INFO_CRTWRP_WRAPPERFUNS (arg_info)
          = InsertIntoLUT_S (INFO_CRTWRP_WRAPPERFUNS (arg_info), FUNDEF_NAME (arg_node),
                             wrapper);
    } else {
        if (dot_args || HasDotArgs (FUNDEF_ARGS (wrapper)) || dot_rets
            || HasDotTypes (FUNDEF_TYPES (wrapper))) {
            num_wr_args = (HasDotArgs (FUNDEF_ARGS (wrapper))
                             ? CountArgs (FUNDEF_ARGS (wrapper)) - 1
                             : CountArgs (FUNDEF_ARGS (wrapper)));
            num_wr_rets = (HasDotTypes (FUNDEF_TYPES (wrapper))
                             ? CountTypes (FUNDEF_TYPES (wrapper)) - 1
                             : CountTypes (FUNDEF_TYPES (wrapper)));
            if ((num_args != num_wr_args) || (num_rets != num_wr_rets)) {
                ABORT (
                  linenum,
                  ("trying to overload function \"%s\" that expects %s %d argument(s) "
                   "and %s %d return value(s) with a version that expects %s %d "
                   "argument(s) "
                   "and %s %d return value(s)",
                   FUNDEF_NAME (arg_node),
                   (HasDotArgs (FUNDEF_ARGS (wrapper)) ? ">=" : ""), num_wr_args,
                   (HasDotTypes (FUNDEF_TYPES (wrapper)) ? ">=" : ""), num_wr_rets,
                   (dot_args ? ">=" : ""), num_args, (dot_rets ? ">=" : ""), num_rets));
            }
        }
    }

    FUNDEF_RET_TYPE (arg_node) = CreateFunRettype (FUNDEF_TYPES (arg_node));
    FUNDEF_TYPE (wrapper)
      = TYMakeOverloadedFunType (CreateFuntype (arg_node), FUNDEF_TYPE (wrapper));

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
        DBUG_PRINT ("CRTWRP", ("----- Processing body of %s:%s: -----",
                               FUNDEF_MOD (arg_node), FUNDEF_NAME (arg_node)));
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPlet(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPlet (node *arg_node, info *arg_info)
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
 *    node *CRTWRPap(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPap (node *arg_node, info *arg_info)
{
    int num_args;
    node *wrapper;

    DBUG_ENTER ("CRTWRPap");

    num_args = CountExprs (AP_ARGS (arg_node));
    wrapper = FindWrapper (AP_NAME (arg_node), num_args, INFO_CRTWRP_EXPRETS (arg_info),
                           INFO_CRTWRP_WRAPPERFUNS (arg_info));

    if (wrapper == NULL) {
        ABORT (NODE_LINE (arg_node),
               ("No definition found for a function \"%s\" that expects"
                " %i argument(s) and yields %i return value(s)",
                AP_NAME (arg_node), num_args, INFO_CRTWRP_EXPRETS (arg_info)));
    } else {
        AP_FUNDEF (arg_node) = wrapper;
        AP_ARGS (arg_node) = TagReferenceArgs (AP_ARGS (arg_node), FUNDEF_ARGS (wrapper));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPid(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CRTWRPid");

    SET_FLAG (ID, arg_node, IS_REFERENCE, FALSE);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPNwithop( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPNwithop (node *arg_node, info *arg_info)
{
    int num_args;
    node *wrapper;

    DBUG_ENTER ("CRTWRPNwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        if (NWITHOP_DEFAULT (arg_node) != NULL) {
            NWITHOP_DEFAULT (arg_node) = Trav (NWITHOP_DEFAULT (arg_node), arg_info);
        }
        break;

    case WO_modarray:
        NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        break;

    case WO_foldfun:
        NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);

        num_args = 2;
        wrapper = FindWrapper (NWITHOP_FUN (arg_node), 2, 1,
                               INFO_CRTWRP_WRAPPERFUNS (arg_info));

        if (wrapper == NULL) {
            ABORT (NODE_LINE (arg_node),
                   ("No definition found for a function \"%s\" that expects"
                    " 2 arguments and yields 1 return value",
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
