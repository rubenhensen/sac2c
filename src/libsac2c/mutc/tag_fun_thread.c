/** <!--********************************************************************-->
 *
 * @defgroup temp Traversal template
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * ============================================================================
 * can be called on N_module               |   -----   |  y  |       |
 * can be called on N_fundef               |   -----   |  y  |       |
 * expects LaC funs                        |   -----   |  n  |       |
 * follows N_ap to LaC funs                |   -----   |  y  |       |
 * ============================================================================
 * deals with GLF properly                 |    yes    |  y  |       |
 * ============================================================================
 * is aware of potential SAA annotations   |    yes    |  y  |       |
 * utilises SAA annotations                |   -----   |  n  |       |
 * ============================================================================
 * tolerates flattened N_array             |    yes    |  y  |       |
 * tolerates flattened Generators          |    yes    |  y  |       |
 * tolerates flattened operation parts     |    yes    |  y  |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |  y  |       |
 * ============================================================================
 * tolerates multi-operator WLs            |    yes    |  y  |       |
 * ============================================================================
 * </pre>
 *
 * Traverses the call tree of the program to see when thread mode can be
 * droped
 *
 * Trav aps to fundefs and look for the need to use thread mode tag fundef,
 * and calling fun as thread fun.
 *
 * Reasons to be in thread mode:
 * =============================
 * Have a call to a thread fun.
 * Have with3
 * Is !local or is exported or is provided or IsObjInitFun
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file tag_fun_thread.c
 *
 * Prefix: TFT
 *
 *****************************************************************************/
#include "tag_fun_thread.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "TFT"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "LookUpTable.h"
/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool thread;
    bool module;
    lut_t *funs;
};

#define INFO_THREAD(n) (n->thread)
#define INFO_MODULE(n) (n->module)
#define INFO_FUNS(n) (n->funs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_THREAD (result) = FALSE;
    INFO_MODULE (result) = TRUE;
    INFO_FUNS (result) = LUTgenerateLut ();

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_FUNS (info) = LUTremoveLut (INFO_FUNS (info));

    info = MEMfree (info);

    DBUG_RETURN (info);
}
/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *TFTdoTagFunctionsAsThreads( node *syntax_tree)
 *
 *****************************************************************************/
node *
TFTdoTagFunctionsAsThreads (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_PRINT ("Sarting tagging functions as threads traversal.");

    INFO_MODULE (info) = (NODE_TYPE (syntax_tree) == N_module);

    TRAVpush (TR_tft);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("Taging functions as threads complete.");

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *TFTfundef(node *arg_node, info *arg_info)
 *
 * @brief Tag this functions as a thread function if needed
 *
 *****************************************************************************/
node *
TFTfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* If this is a thread fun then rember in info */
    if (FUNDEF_ISEXTERN (arg_node) && !FUNDEF_ISTHREADFUN (arg_node)) {
        /* This is an external 'C' function */
        /* Do nothing */
    } else if (FUNDEF_ISTHREADFUN (arg_node) || FUNDEF_ISEXPORTED (arg_node)
               || FUNDEF_ISPROVIDED (arg_node) || FUNDEF_ISOBJINITFUN (arg_node)
               || !FUNDEF_ISLOCAL (arg_node)) {
        FUNDEF_ISTHREADFUN (arg_node) = TRUE;
        INFO_THREAD (arg_info) = TRUE;
    } else {
        /* If we have not traversed this fun before update its thread status */
        if (LUTsearchInLutP (INFO_FUNS (arg_info), arg_node) == NULL) {
            INFO_FUNS (arg_info)
              = LUTinsertIntoLutP (INFO_FUNS (arg_info), arg_node, (void *)TRUE);

            bool thread = INFO_THREAD (arg_info);

            INFO_THREAD (arg_info) = FALSE;

            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

            FUNDEF_ISTHREADFUN (arg_node) = INFO_THREAD (arg_info);

            INFO_THREAD (arg_info) = (thread || FUNDEF_ISTHREADFUN (arg_node));
        }
    }

    /* If we are in module mode process next fun */
    if (INFO_MODULE (arg_info)) {
        INFO_THREAD (arg_info) = FALSE;
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFTwith3(node *arg_node, info *arg_info)
 *
 * @brief With3 need thread mode
 *
 *****************************************************************************/
node *
TFTwith3 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_THREAD (arg_info) = TRUE;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFTap(node *arg_node, info *arg_info)
 *
 * @brief Pass down thread status if calling a thread fun
 *        trav into fun to see if it should be a thread fun.
 *
 *****************************************************************************/
node *
TFTap (node *arg_node, info *arg_info)
{
    bool module;
    bool thread;
    DBUG_ENTER ();

    thread = INFO_THREAD (arg_info);
    module = INFO_MODULE (arg_info);
    INFO_MODULE (arg_info) = FALSE;

    AP_FUNDEF (arg_node) = TRAVopt (AP_FUNDEF (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_MODULE (arg_info) = module;
    INFO_THREAD (arg_info) = (thread || INFO_THREAD (arg_info));
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
