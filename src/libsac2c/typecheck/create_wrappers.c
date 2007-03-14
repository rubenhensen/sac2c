/* $Id$ */

#include <stdio.h>
#include <string.h>
#include "create_wrappers.h"
#include "dbug.h"
#include "ctinfo.h"
#include "free.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "globals.h"
#include "namespaces.h"
#include "user_types.h"
#include "new_types.h"
#include "type_utils.h"
#include "deserialize.h"
#include "map_fun_trav.h"

/*******************************************************************************
 *
 */

/**
 * INFO structure
 */
struct INFO {
    lut_t *wrapperfuns;
    int exprets;
    node *module;
};

/**
 * INFO macros
 */

#define INFO_WRAPPERFUNS(n) ((n)->wrapperfuns)
#define INFO_EXPRETS(n) ((n)->exprets)
#define INFO_MODULE(n) ((n)->module)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WRAPPERFUNS (result) = NULL;
    INFO_EXPRETS (result) = 0;
    INFO_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *CRTWRPdoCreateWrappers( node *arg_node)
 *
 * description:
 *   Creates wrapper functions for all functions found. These wrapper functions
 *     - obtain generic argument (ARG_TYPE)/ result types(RET_TYPE) : _unknown_[*]
 *     - obtain overloaded function types (FUNDEF_WRAPPERTYPE) that include all
 *       function definitions found
 *     - have FUNDEF_ISWRAPPERFUN set
 *     - have a NULL BODY
 *     - are inserted into the N_fundef chain
 *   Furthermore,
 *     - all function applications obtain a backref to the wrapper function
 *       responsible for the function dispatch
 *     - the AP_SPNAME and AP_SPMOD is freed
 *
 *  NB: due to use's / import's there may be wrapper funs already! However,
 *      these may have to be combined (symmetric to split-wrapers!).
 *
 ******************************************************************************/

node *
CRTWRPdoCreateWrappers (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("CRTWRPdoCreateWrappers");

    TRAVpush (TR_crtwrp);

    info_node = MakeInfo ();
    arg_node = TRAVdo (arg_node, info_node);
    info_node = FreeInfo (info_node);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

static bool
RefArgMatch (node *arg1, node *arg2)
{
    bool result = TRUE;

    DBUG_ENTER ("RefArgMatch");

    if ((arg1 != NULL) && (arg2 != NULL)) {
        result = result && (ARG_ISREFERENCE (arg1) == ARG_ISREFERENCE (arg2))
                 && RefArgMatch (ARG_NEXT (arg1), ARG_NEXT (arg2));
    } else {
        result = (arg1 == arg2);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *FindWrapper( char *mod, char *name, int num_args,
 *                      int num_rets, lut_t *lut)
 *
 * description:
 *   Searches for a wrapper function in the given look-up-table (lut) that
 *   matches the given module name (mod), name (name), the given number of
 *   arguments (num_args), and the given number of return values (num_rets).
 *   This matching mechanism does allow for dots to be used "on both sides",
 *   i.e., for non-fixed numbers of arguments and non-fixed numbers of
 *   return values.
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
FindWrapper (namespace_t *ns, char *name, int num_args, int num_rets, lut_t *lut)
{
    bool last_parm_is_dots;
    bool last_res_is_dots;
    int num_parms, num_res;
    node **wrapper_p;
    node *wrapper = NULL;
    bool found = FALSE;

    DBUG_ENTER ("FindWrapper");

    DBUG_PRINT ("CRTWRP", ("Searching for %s:%s %d args %d rets", NSgetName (ns), name,
                           num_args, num_rets));

    /* initial search for wrapper in LUT */
    wrapper_p = (node **)LUTsearchInLutS (lut, name);
    while ((wrapper_p != NULL) && (!found)) {
        wrapper = *wrapper_p;
        last_parm_is_dots = FUNDEF_HASDOTARGS (wrapper);
        last_res_is_dots = FUNDEF_HASDOTRETS (wrapper);
        num_parms = TCcountArgs (FUNDEF_ARGS (wrapper));
        num_res = TCcountRets (FUNDEF_RETS (wrapper));
        DBUG_PRINT ("CRTWRP", (" ... checking %s %s%d args %s%d rets",
                               FUNDEF_NAME (wrapper), (last_parm_is_dots ? ">=" : ""),
                               num_parms, (last_res_is_dots ? ">=" : ""), num_res));
        if (((num_res == num_rets) || (last_res_is_dots && (num_res <= num_rets)))
            && ((num_parms == num_args) || (last_parm_is_dots && (num_parms <= num_args)))
            && NSequals (FUNDEF_NS (wrapper), ns)) {
            found = TRUE;
        } else {
            /* search for next wrapper in LUT */
            wrapper_p = (node **)LUTsearchInLutNextS ();
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
 *    node *CreateWrapperFor( node *fundef, info *info)
 *
 * description:
 *   using a given fundef (fundef) as a template, a wrapper function is
 *   created. It consists of a duplicate of the function header given
 *   whose types have been modified to "_unknown_[*]"
 *   FUNDEF_ISWRAPPERFUN is set TRUE and its body is NULL!
 *
 ******************************************************************************/

static void
ResetArgsOrRets (node *arg_node)
{
    DBUG_ENTER ("ResetArgsOrRets");

    while (arg_node != NULL) {
        switch (NODE_TYPE (arg_node)) {
        case N_arg:
            ARG_HASLINKSIGNINFO (arg_node) = FALSE;
            ARG_LINKSIGN (arg_node) = 0;
            ARG_ISREFCOUNTED (arg_node) = TRUE;
            arg_node = ARG_NEXT (arg_node);
            break;
        case N_ret:
            RET_HASLINKSIGNINFO (arg_node) = FALSE;
            RET_LINKSIGN (arg_node) = 0;
            RET_ISREFCOUNTED (arg_node) = TRUE;
            arg_node = RET_NEXT (arg_node);
            break;
        default:
            DBUG_ASSERT (FALSE, "non arg or ret argument to ResetArgsOrRets");
        }
    }

    DBUG_VOID_RETURN;
}

static node *
CreateWrapperFor (node *fundef, info *info)
{
    node *body, *wrapper;

    DBUG_ENTER ("CreateWrapperFor");
    DBUG_PRINT ("CRTWRP",
                ("Creating wrapper for %s %s%d args %d rets", CTIitemName (fundef),
                 (FUNDEF_HASDOTARGS (fundef) ? ">=" : ""),
                 (FUNDEF_HASDOTARGS (fundef) ? TCcountArgs (FUNDEF_ARGS (fundef)) - 1
                                             : TCcountArgs (FUNDEF_ARGS (fundef))),
                 TCcountRets (FUNDEF_RETS (fundef))));

    /*
     * if we have a wrapper function of a used function
     * we can just reuse it here as the new combined
     * wrapper function. We later on just add the other
     * instances.
     */
    if (FUNDEF_ISWRAPPERFUN (fundef)) {
        wrapper = fundef;

        /*
         * as this is no more the wrapper stored within
         * the module, we have to remove the symbolid. As
         * there might be some deserialisation during typechecking
         * and this prior to splitwrappers, we must make sure
         * that deserialised function calls point to the generic
         * wrapper. Thats why we have to add an aliasing prior
         * to deleting the symbolname!
         */
        DSaddAliasing (FUNDEF_SYMBOLNAME (fundef), wrapper);
        FUNDEF_SYMBOLNAME (fundef) = MEMfree (FUNDEF_SYMBOLNAME (fundef));

        /*
         * we set the wrapper to local here. In split_wrappers this
         * will be made undone if no local instances were added
         * during typechecking
         */
        FUNDEF_ISLOCAL (wrapper) = TRUE;
    } else {
        body = FUNDEF_BODY (fundef);
        FUNDEF_BODY (fundef) = NULL;
        wrapper = DUPdoDupNode (fundef);
        FUNDEF_BODY (fundef) = body;

        FUNDEF_ISWRAPPERFUN (wrapper) = TRUE;
        FUNDEF_ISLOCAL (wrapper) = TRUE;
        FUNDEF_WASUSED (wrapper) = FALSE;
        FUNDEF_WASIMPORTED (wrapper) = FALSE;
        FUNDEF_ISINLINE (wrapper) = FALSE;

        if (FUNDEF_ISEXTERN (wrapper)) {
            /* this is a wrapper for external functions.
             * the wrapper itself is not external, so
             * reset the flag and create a proper
             * linksign counting.
             */
            FUNDEF_ISEXTERN (wrapper) = FALSE;
            ResetArgsOrRets (FUNDEF_RETS (wrapper));
            ResetArgsOrRets (FUNDEF_ARGS (wrapper));
        }
        /*
         * finally, set the module name to the one of the
         * currently compiled module. Although the instance
         * might come from another module, the created
         * wrapper has to be local!
         */
        FUNDEF_NS (wrapper) = NSfreeNamespace (FUNDEF_NS (wrapper));
        FUNDEF_NS (wrapper) = NSdupNamespace (MODULE_NAMESPACE (INFO_MODULE (info)));
    }

    /*
     * setting the wrapper function's arg and return types to _unknown_[*]
     */
    FUNDEF_ARGS (wrapper) = TUargtypes2unknownAUD (FUNDEF_ARGS (wrapper));
    FUNDEF_RETS (wrapper) = TUrettypes2unknownAUD (FUNDEF_RETS (wrapper));

    /*
     * Now, we implement a dirty trick for 0 ary functions.
     * Since we do not allow them to be overloaded, we include
     * a pointer to the fundef here.
     * It is used when dispatching such functions (cf. new_types.c)!!
     * of course wo do not do so if we are copying an imported
     * wrapper! In that case, we just copy the FUNDEF_IMPL of the
     * imported wrapper.
     * However, for used or imported wrappers this is redundant as they
     * do come along with appropriately set FUNDEF_IMPL.
     */
    if ((FUNDEF_ARGS (wrapper) == NULL) && !FUNDEF_ISWRAPPERFUN (fundef)) {
        FUNDEF_IMPL (wrapper) = fundef;
    }

    DBUG_RETURN (wrapper);
}

/******************************************************************************
 *
 * function:
 *    ntype *CRTWRPcreateFuntype( node *fundef)
 *
 * description:
 *    creates a function type from the given arg/return types.
 *
 ******************************************************************************/

static ntype *
FuntypeFromArgs (ntype *res, node *args, node *fundef)
{
    DBUG_ENTER ("FuntypeFromArgs");

    if (args != NULL) {
        res = FuntypeFromArgs (res, ARG_NEXT (args), fundef);
        res = TYmakeFunType (TYcopyType (ARG_NTYPE (args)), res, fundef);
    }

    DBUG_RETURN (res);
}

ntype *
CRTWRPcreateFuntype (node *fundef)
{
    ntype *res;

    DBUG_ENTER ("CRTWRPcreateFuntype");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "CRTWRPcreateFuntype applied to non-fundef node!");

    res = FuntypeFromArgs (TUmakeProductTypeFromRets (FUNDEF_RETS (fundef)),
                           FUNDEF_ARGS (fundef), fundef);

    DBUG_RETURN (res);
}

node *
CRTWRPspecFundef (node *arg_node, info *arg_info)
{
    int num_args, num_rets;
    node *wrapper;

    DBUG_ENTER ("CRTWRPspecFundef");

    num_args = TCcountArgs (FUNDEF_ARGS (arg_node));
    num_rets = TCcountRets (FUNDEF_RETS (arg_node));

    wrapper = FindWrapper (FUNDEF_NS (arg_node), FUNDEF_NAME (arg_node), num_args,
                           num_rets, INFO_WRAPPERFUNS (arg_info));
    if (wrapper == NULL) {
        CTIabortLine (NODE_LINE (arg_node),
                      "No definition found for a function \"%s::%s\" that expects"
                      " %i argument(s) and yields %i return value(s)",
                      NSgetName (FUNDEF_NS (arg_node)), FUNDEF_NAME (arg_node), num_args,
                      num_rets);

    } else {
        FUNDEF_IMPL (arg_node) = wrapper;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPmodule(node *arg_node, info *arg_info)
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
CRTWRPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CRTWRPmodule");

    DBUG_ASSERT ((MODULE_WRAPPERFUNS (arg_node) == NULL),
                 "MODULE_WRAPPERFUNS is not NULL!");
    MODULE_WRAPPERFUNS (arg_node) = LUTgenerateLut ();
    INFO_WRAPPERFUNS (arg_info) = MODULE_WRAPPERFUNS (arg_node);

    INFO_MODULE (arg_info) = arg_node;

    /**
     * First, we traverse the external functions. As these per definition
     * DO NOT have bodies, it does not matter that not all functions have
     * been seen prior to our way back up!
     */
    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }
    /**
     * Now, we traverse the local fundefs. Once we reach the last fundef,
     * we know for sure, that we have seen ALL functions (including the externals)!
     */
    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    /**
     * Now, we set the FUNDEF_IMPL nodes for the forced specializations:
     */
    MODULE_FUNSPECS (arg_node)
      = MFTdoMapFunTrav (MODULE_FUNSPECS (arg_node), arg_info, CRTWRPspecFundef);
    /**
     * Finally, we insert the wrapper functions into the fundef chain:
     */
    MODULE_FUNS (arg_node)
      = LUTfoldLutS (INFO_WRAPPERFUNS (arg_info), MODULE_FUNS (arg_node),
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
    node *wrapper = NULL;
    int num_args, num_rets;
    bool dot_args, dot_rets;

    DBUG_ENTER ("CRTWRPfundef");

    dot_args = FUNDEF_HASDOTARGS (arg_node);
    dot_rets = FUNDEF_HASDOTRETS (arg_node);
    num_args = TCcountArgs (FUNDEF_ARGS (arg_node));
    num_rets = TCcountRets (FUNDEF_RETS (arg_node));

    DBUG_PRINT ("CRTWRP",
                ("----- Processing function %s: -----", CTIitemName (arg_node)));

    /**
     * we need to include the following functions into wrappers:
     *   - ALL local fundefs
     *   - ALL imported fundefs ( but NO imported wrappers)
     *   - ALL used wrappers ( but NO used fundefs)
     * While the former two categories obtain local wrappers,
     * the latter retain their original namespace.
     *
     * NB: imported wrappers should not occur as they should not be loaded in
     *     the first place....
     */
    DBUG_ASSERT (!(FUNDEF_WASIMPORTED (arg_node) && FUNDEF_ISWRAPPERFUN (arg_node)),
                 "imported wrapper found!");
    if (!FUNDEF_ISLOCAL (arg_node) && !FUNDEF_WASIMPORTED (arg_node)) {
        /**
         * First, we combine used wrappers. Note here, that this will only combine
         * all wrappers from ONE module with an identical amount of args / rets, i.e.,
         * its the dual to SplitWrappers!
         */
        if (FUNDEF_ISWRAPPERFUN (arg_node)) {
            wrapper = FindWrapper (FUNDEF_NS (arg_node), FUNDEF_NAME (arg_node), num_args,
                                   num_rets, INFO_WRAPPERFUNS (arg_info));
            if (wrapper == NULL) {
                /**
                 * There is no wrapper for this function yet.
                 * Therefore, CreateWrapperFor will reuse this one and generalize
                 * its arg / return types.
                 */
                wrapper = CreateWrapperFor (arg_node, arg_info);
                INFO_WRAPPERFUNS (arg_info)
                  = LUTinsertIntoLutS (INFO_WRAPPERFUNS (arg_info),
                                       FUNDEF_NAME (arg_node), wrapper);
            } else {
                /* integrate this wrapper into the existing one */

                FUNDEF_WRAPPERTYPE (wrapper)
                  = TYmakeOverloadedFunType (TYcopyType (FUNDEF_WRAPPERTYPE (arg_node)),
                                             FUNDEF_WRAPPERTYPE (wrapper));
                /*
                 * add an aliasing so calls to this wrapper will
                 * point to the correct general wrapper
                 */
                DSaddAliasing (FUNDEF_SYMBOLNAME (arg_node), wrapper);
                arg_node = FREEdoFreeNode (arg_node);
            }
        }
    } else {
        /**
         * ISLOCAL or WASIMPORTED fundef !
         */
        wrapper = FindWrapper (MODULE_NAMESPACE (INFO_MODULE (arg_info)),
                               FUNDEF_NAME (arg_node), num_args, num_rets,
                               INFO_WRAPPERFUNS (arg_info));
        if (wrapper == NULL) {
            wrapper = CreateWrapperFor (arg_node, arg_info);
            INFO_WRAPPERFUNS (arg_info)
              = LUTinsertIntoLutS (INFO_WRAPPERFUNS (arg_info), FUNDEF_NAME (arg_node),
                                   wrapper);
        } else {
            if ((dot_args != FUNDEF_HASDOTARGS (wrapper))
                || (dot_rets != FUNDEF_HASDOTRETS (wrapper))) {
                CTIabortLine (global.linenum,
                              "Trying to overload function \"%s\" that expects %s %d "
                              "argument(s) "
                              "and %s %d return value(s) with a version that expects %s "
                              "%d argument(s) "
                              "and %s %d return value(s)",
                              CTIitemName (wrapper),
                              (FUNDEF_HASDOTARGS (wrapper) ? ">=" : ""),
                              TCcountArgs (FUNDEF_ARGS (wrapper)),
                              (FUNDEF_HASDOTRETS (wrapper) ? ">=" : ""),
                              TCcountRets (FUNDEF_RETS (wrapper)), (dot_args ? ">=" : ""),
                              num_args, (dot_rets ? ">=" : ""), num_rets);
            }
        }

        /*
         * check whether the signatures match wrt reference args
         */
        if (!RefArgMatch (FUNDEF_ARGS (wrapper), FUNDEF_ARGS (arg_node))) {
            CTIabortLine (NODE_LINE (arg_node),
                          "Trying to overload function \"%s\" that expects %d "
                          "argument(s) "
                          "and %d return value(s) with a version that has a signature "
                          "differing in the number or position of reference args only.",
                          CTIitemName (wrapper), TCcountArgs (FUNDEF_ARGS (wrapper)),
                          TCcountRets (FUNDEF_RETS (wrapper)));
        }

        if (FUNDEF_ISLOCAL (arg_node) && !(FUNDEF_ISEXTERN (arg_node))) {
            FUNDEF_RETS (arg_node) = TUrettypes2alphaMax (FUNDEF_RETS (arg_node));
        } else {
            FUNDEF_RETS (arg_node) = TUrettypes2alphaFix (FUNDEF_RETS (arg_node));
        }

        FUNDEF_WRAPPERTYPE (wrapper)
          = TYmakeOverloadedFunType (CRTWRPcreateFuntype (arg_node),
                                     FUNDEF_WRAPPERTYPE (wrapper));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * Now, we do have wrappers for all fundefs!
     * On our way back up, we traverse the body in order to insert the backrefs
     * to the appropriate wrappers.
     */

    /*
     * Usually, we expect all functions applied in the function body to return a
     * single value. Therefore, INFO_EXPRETS is set to 1.
     * Only on RHSs of N_let nodes with more than one Id on the LHS, this
     * value is changed (and reset afterwards!) -> CRTWRPlet.
     */
    INFO_EXPRETS (arg_info) = 1;

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("CRTWRP",
                    ("----- Processing body of %s: -----", CTIitemName (arg_node)));
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * if we reused a wrapper-function (generated by a use statement)
     * we have to remove the wrapper from the fundef chain, as
     * it is now stored in the wrapper LUT.
     * when the LUT is merged into the ast, thsi wrapper will be
     * merged in again!
     */
    if (wrapper == arg_node) {
        arg_node = FUNDEF_NEXT (arg_node);
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

    old_exprets = INFO_EXPRETS (arg_info);
    INFO_EXPRETS (arg_info) = TCcountIds (LET_IDS (arg_node));

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }
    INFO_EXPRETS (arg_info) = old_exprets;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPspap(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPspap (node *arg_node, info *arg_info)
{
    int num_args;
    node *wrapper;
    node *new_node = NULL;

    DBUG_ENTER ("CRTWRPspap");

    num_args = TCcountExprs (SPAP_ARGS (arg_node));
    wrapper = FindWrapper (SPAP_NS (arg_node), SPAP_NAME (arg_node), num_args,
                           INFO_EXPRETS (arg_info), INFO_WRAPPERFUNS (arg_info));

    DBUG_PRINT ("CRTWRP",
                ("Adding backreference to %s:%s as " F_PTR ".",
                 NSgetName (SPAP_NS (arg_node)), SPAP_NAME (arg_node), wrapper));

    if (wrapper == NULL) {
        CTIabortLine (NODE_LINE (arg_node),
                      "No definition found for a function \"%s::%s\" that expects"
                      " %i argument(s) and yields %i return value(s)",
                      NSgetName (SPAP_NS (arg_node)), SPAP_NAME (arg_node), num_args,
                      INFO_EXPRETS (arg_info));
    } else {
        /*
         * as the function is dispatched now, we can create a real
         * function application now and free the spap node
         */
        new_node = TBmakeAp (wrapper, SPAP_ARGS (arg_node));
        SPAP_ARGS (arg_node) = NULL;
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (new_node);
}

/** <!--********************************************************************-->
 *
 * @fn node  *CRTWRPgenarray( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
CRTWRPgenarray (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CRTWRPgenarray");

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node  *CRTWRPspfold( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
CRTWRPspfold (node *arg_node, info *arg_info)
{
    int num_args;
    node *wrapper;
    node *new_node = NULL;

    DBUG_ENTER ("CRTWRPspfold");

    DBUG_ASSERT (SPFOLD_FUN (arg_node) != NULL, "N_spfold node wo FUN");
    DBUG_ASSERT (SPFOLD_NEUTRAL (arg_node) != NULL, "N_spfold node wo NEUTRAL");

    SPFOLD_NEUTRAL (arg_node) = TRAVdo (SPFOLD_NEUTRAL (arg_node), arg_info);

    num_args = 2;
    wrapper = FindWrapper (SPFOLD_NS (arg_node), SPFOLD_FUN (arg_node), 2, 1,
                           INFO_WRAPPERFUNS (arg_info));

    if (wrapper == NULL) {
        CTIabortLine (NODE_LINE (arg_node),
                      "No definition found for a function \"%s::%s\" that expects"
                      " 2 arguments and yields 1 return value",
                      NSgetName (SPFOLD_NS (arg_node)), SPFOLD_FUN (arg_node));
    } else {
        new_node = TBmakeFold (wrapper, SPFOLD_NEUTRAL (arg_node));
        FOLD_GUARD (new_node) = SPFOLD_GUARD (arg_node);

        SPFOLD_NEUTRAL (arg_node) = NULL;
        SPFOLD_GUARD (arg_node) = NULL;
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (new_node);
}