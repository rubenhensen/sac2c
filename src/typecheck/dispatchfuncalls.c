/*
 *
 * $Log$
 * Revision 1.2  2005/07/26 14:32:08  sah
 * moved creation of special fold funs to
 * dispatchfuncall as new2old is running
 * prior to the module system which again relies
 * on the fact that no foldfuns have been
 * created, yet.
 *
 * Revision 1.1  2005/07/15 15:53:33  sah
 * Initial revision
 *
 *
 */
#include "dispatchfuncalls.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "ctinfo.h"
#include "traverse.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "type_utils.h"
#include "create_wrapper_code.h"
#include "namespaces.h"
#include "gen_pseudo_fun.h"
#include "ct_fun.h"

/*******************************************************************************
 *
 *
 */

/**
 * INFO structure
 */
struct INFO {
    node *with;
    node *let;
    node *foldfuns;
};

/**
 * INFO macros
 */
#define INFO_DFC_WITH(n) ((n)->with)
#define INFO_DFC_LASTLET(n) ((n)->let)
#define INFO_DFC_FOLDFUNS(n) ((n)->foldfuns)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_DFC_WITH (result) = NULL;
    INFO_DFC_LASTLET (result) = NULL;
    INFO_DFC_FOLDFUNS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * Function:
 *   node *DFCmodule( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFCmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if (INFO_DFC_FOLDFUNS (arg_info) != NULL) {
        MODULE_FUNS (arg_node)
          = TCappendFundef (INFO_DFC_FOLDFUNS (arg_info), MODULE_FUNS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DispatchFunCall( node *fundef, ntype *arg_types)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DispatchFunCall (node *fundef, ntype *arg_types)
{
    dft_res *dft_res;

    DBUG_ENTER ("DispatchFunCall");

    DBUG_ASSERT ((fundef != NULL), "fundef not found!");
    if (FUNDEF_ISWRAPPERFUN (fundef)) {
        /*
         * 'fundef' points to an generic function
         *   -> try to dispatch the function application statically in order to
         *      avoid superfluous wrapper function calls
         */
        DBUG_PRINT ("DFC", ("correcting fundef for %s", CTIitemName (fundef)));

        /*
         * if we have a bottom type, we cannot dispatch statically.
         * so we do not even try in this case
         */
        if (TYgetBottom (arg_types) == NULL) {
            /*
             * try to dispatch the function application statically
             */
            dft_res = NTCCTdispatchFunType (fundef, arg_types);
            if (dft_res == NULL) {
                DBUG_ASSERT ((TYgetProductSize (arg_types) == 0),
                             "illegal dispatch result found!");
                /*
                 * no args found -> static dispatch possible
                 *
                 * fundef can be found in FUNDEF_IMPL (dirty hack!)
                 */
                fundef = FUNDEF_IMPL (fundef);
                DBUG_PRINT ("DFC", ("  dispatched statically %s", CTIitemName (fundef)));
            } else if ((dft_res->num_partials == 0)
                       && (dft_res->num_deriveable_partials == 0)) {
                /*
                 * static dispatch possible
                 */
                if (dft_res->def != NULL) {
                    DBUG_ASSERT ((dft_res->deriveable == NULL),
                                 "def and deriveable found!");
                    fundef = dft_res->def;
                } else {
                    fundef = dft_res->deriveable;
                }
                DBUG_PRINT ("DFC", ("  dispatched statically %s", CTIitemName (fundef)));
            } else if (!CWChasWrapperCode (fundef)) {
                /*
                 * static dispatch impossible,
                 *    but no wrapper function could be created either!!
                 * if only a single instance is available, do the dispatch
                 * statically and give a warning message, otherwise we are stuck here!
                 */
                if ((dft_res->num_partials + dft_res->num_deriveable_partials == 1)
                    && (dft_res->def == NULL) && (dft_res->deriveable == NULL)) {
                    fundef = (dft_res->num_partials == 1)
                               ? dft_res->partials[0]
                               : dft_res->deriveable_partials[0];
                    CTIwarnLine (global.linenum,
                                 "Application of var-arg function %s found which may"
                                 " cause a type error",
                                 CTIitemName (fundef));
                    DBUG_PRINT ("DFC", ("  dispatched statically although only partial"
                                        " has been found (T_dots)!"));
                } else {
                    DBUG_ASSERT ((0), "wrapper with T_dots found which could be "
                                      "dispatched statically!");
                }
            } else {
                /*
                 * static dispatch impossible -> keep the wrapper
                 */
                DBUG_PRINT ("DFC", ("  static dispatch impossible"));
            }
        }
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *DFCfundef( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFCfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("DFCfundef");

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    if ((!FUNDEF_ISWRAPPERFUN (arg_node)) && (FUNDEF_BODY (arg_node) != NULL)) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFCap( node *arg_node, info *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFCap (node *arg_node, info *arg_info)
{
    ntype *arg_types;

    DBUG_ENTER ("DFCap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    DBUG_PRINT ("DFC",
                ("Ap of function %s::%s pointed to " F_PTR ".",
                 NSgetName (AP_NS (arg_node)), AP_NAME (arg_node), AP_FUNDEF (arg_node)));

    arg_types = TUactualArgs2Ntype (AP_ARGS (arg_node));
    AP_FUNDEF (arg_node) = DispatchFunCall (AP_FUNDEF (arg_node), arg_types);

    DBUG_PRINT ("DFC",
                ("Ap of function %s:%s now points to " F_PTR ".",
                 NSgetName (AP_NS (arg_node)), AP_NAME (arg_node), AP_FUNDEF (arg_node)));
    arg_types = TYfreeType (arg_types);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *DFCwith( node *arg_node, info *arg_info)
 *
 * @brief inserts actual with node into the info structure
 *
 ******************************************************************************/

node *
DFCwith (node *arg_node, info *arg_info)
{
    node *old_with;
    DBUG_ENTER ("DFCwith");

    old_with = INFO_DFC_WITH (arg_info);
    INFO_DFC_WITH (arg_info) = arg_node;

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    INFO_DFC_WITH (arg_info) = old_with;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFCgenarray( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFCgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DFCgenarrray");

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

static node *
buildSpecialFoldFun (node *arg_node, info *arg_info, ntype *argtype)
{
    node *foldfun;
    node *cexpr, *neutr;
    node *with;

    DBUG_ENTER ("buildSpecialFoldFun");

    with = INFO_DFC_WITH (arg_info);
    cexpr = WITH_CEXPR (with);
    neutr = FOLD_NEUTRAL (arg_node);

    DBUG_ASSERT ((neutr != NULL), "WITH_NEUTRAL does not exist");
    DBUG_ASSERT ((NODE_TYPE (neutr) == N_id), "WITH_NEUTRAL is not a N_id");
    DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "WITH_CEXPR is not a N_id");

    foldfun = GPFcreateFoldFun (argtype, FOLD_FUNDEF (arg_node), FOLD_PRF (arg_node),
                                IDS_NAME (LET_IDS (INFO_DFC_LASTLET (arg_info))),
                                ID_NAME (cexpr));
    FOLD_FUNDEF (arg_node) = foldfun;

    /*
     * append foldfun to INFO_NT2OT_FOLDFUNS
     */
    FUNDEF_NEXT (foldfun) = INFO_DFC_FOLDFUNS (arg_info);
    INFO_DFC_FOLDFUNS (arg_info) = foldfun;

    DBUG_RETURN (arg_node);
}

node *
DFCfold (node *arg_node, info *arg_info)
{
    ntype *neutr_type, *body_type;
    ntype *arg_type, *arg_types;

    DBUG_ENTER ("DFCfold");

    if (FOLD_FUN (arg_node) != NULL) {
        if (FOLD_FUNDEF (arg_node) == NULL) {
            /*
             * first dispatch the function call
             */
            FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

            neutr_type
              = TYfixAndEliminateAlpha (AVIS_TYPE (ID_AVIS (FOLD_NEUTRAL (arg_node))));
            body_type = TYfixAndEliminateAlpha (
              AVIS_TYPE (ID_AVIS (WITH_CEXPR (INFO_DFC_WITH (arg_info)))));

            arg_type = TYlubOfTypes (neutr_type, body_type);
            arg_types = TYmakeProductType (2, arg_type, TYcopyType (arg_type));

            FOLD_FUNDEF (arg_node) = DispatchFunCall (FOLD_FUNDEF (arg_node), arg_types);

            /*
             * second, create a special foldfun
             */
            arg_node = buildSpecialFoldFun (arg_node, arg_info, arg_type);

            /*
             * cleanup
             */
            arg_types = TYfreeType (arg_types);
            body_type = TYfreeType (body_type);
            neutr_type = TYfreeType (neutr_type);
        }
    } else {
        if (FOLD_NEUTRAL (arg_node) != NULL) {
            FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
DFClet (node *arg_node, info *arg_info)
{
    node *old_lastlet;

    DBUG_ENTER ("DFClet");

    old_lastlet = INFO_DFC_LASTLET (arg_info);
    INFO_DFC_LASTLET (arg_info) = arg_node;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_DFC_LASTLET (arg_info) = old_lastlet;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DFCdoDispatchFunCalls( node *ast)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
DFCdoDispatchFunCalls (node *ast)
{
    info *info_node;

    DBUG_ENTER ("DFCdoDispatchFunCalls");

    info_node = MakeInfo ();

    TRAVpush (TR_dfc);

    ast = TRAVdo (ast, info_node);

    TRAVpop ();

    info_node = FreeInfo (info_node);

    DBUG_RETURN (ast);
}
