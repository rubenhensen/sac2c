/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup anrp add norc prfs
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   | yes |       |
 * can be called on N_fundef               |   -----   | yes |       |
 * expects LaC funs                        |   -----   | n/a |       |
 * follows N_ap to LaC funs                |   -----   | no  |       |
 * =============================================================================
 * deals with GLF properly                 |    yes    | yes |       |
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    | no1 |       |
 * utilises SAA annotations                |   -----   | yes |       |
 * =============================================================================
 * tolerates flattened N_array             |    yes    | yes |       |
 * tolerates flattened Generators          |    yes    | yes |       |
 * tolerates flattened operation parts     |    yes    | yes |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    | yes |       |
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    | yes |       |
 * =============================================================================
 * 1) Designed to work on ast post ussa
 * </pre>
 *
 * Perform the following patten transformation
 *
 * a = with3 {
 *   ...
 *   v1 = rfv1;
 *   ...
 *   vn = rfvn;
 *   ...
 * }
 *
 * rfv1', rfv1_oldrc = _2norc_( rfv1);
 * rfvn', rfvn_oldrc = _2norc_( rfvn);
 * a = with3 {
 *   ...
 *   v1 = rfv1';
 *   ...
 *   vn = rfvn'
 *   ...
 * }
 * rfv1 = _norc2_( rfv1', rfv1_oldrc);
 * rfvn = _norc2_( rfvn', rfvn_oldrc);
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file add_norc_prf.c
 *
 * Prefix: anrp
 *
 *****************************************************************************/
#include "add_norc_prf.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool with3;
    bool args_2_prf;
    node *preassign;
    node *postassign;
    node *vardecs;
};

/**
 * A template entry in the template info structure
 */
#define INFO_WITH3(n) ((n)->with3)
#define INFO_ARGS_2_PRF(n) ((n)->args_2_prf)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_POSTASSIGN(n) ((n)->postassign)
#define INFO_VARDECS(n) ((n)->vardecs)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WITH3 (result) = FALSE;
    INFO_ARGS_2_PRF (result) = FALSE;
    INFO_PREASSIGN (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;
    INFO_VARDECS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    DBUG_ASSERT (INFO_WITH3 (info) == FALSE, "Finished traversal in with3 loop");

    DBUG_ASSERT (INFO_ARGS_2_PRF (info) == FALSE,
                 "Finished traversal while looking at with3 thread fun args");

    DBUG_ASSERT (INFO_PREASSIGN (info), "Possible memory leak");

    DBUG_ASSERT (INFO_POSTASSIGN (info), "Possible memory leak");

    DBUG_ASSERT (INFO_VARDECS (info), "Possible memory leak");

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
 * @fn node *ANRPdoAddNorcPrf( node *syntax_tree)
 *
 *****************************************************************************/
node *
ANRPdoAddNorcPrf (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ANRPdoAddNorcPrf");

    info = MakeInfo ();

    DBUG_PRINT ("ANRP", ("Starting template traversal."));

    TRAVpush (TR_anrp);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("ANRP", ("Template traversal complete."));

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
 * @fn node *ANRPwith3(node *arg_node, info *arg_info)
 *
 * @brief mark the info that we are in a with3
 *
 *****************************************************************************/
node *
ANRPwith3 (node *arg_node, info *arg_info)
{
    bool stack;
    DBUG_ENTER ("ANRPwith3");

    stack = INFO_WITH3 (arg_info);
    INFO_WITH3 (arg_info) = TRUE;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_WITH3 (arg_info) = stack;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ANRPassign(node *arg_node, info *arg_info)
 *
 * @brief Insert pre and post assign
 *
 *****************************************************************************/
node *
ANRPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANRPwith3");

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_POSTASSIGN (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGN (arg_info) = NULL;
    }

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ANRPap(node *arg_node, info *arg_info)
 *
 * @brief If this is a thread fun ap in a with3 then the arguments are the
 *        relative free variables in one of the partitions.
 *        Add pre assign and post assign to convert from and to norc mode
 *        Add needed temporary variables
 *
 *****************************************************************************/
node *
ANRPap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANRPap");

    arg_node = TRAVcont (arg_node, arg_info);

    if (FUNDEF_WASWITH3BODY (AP_FUNDEF (arg_node))) {
        bool stack = FALSE;

        DBUG_ASSERT (INFO_WITH3 (arg_info), "With3 thread function not in a with loop");

        /* Could do with check that all args are ids */

        stack = INFO_ARGS_2_PRF (arg_info);
        INFO_ARGS_2_PRF (arg_info) = TRUE;
        arg_node = TRAVcont (arg_node, arg_info);
        INFO_ARGS_2_PRF (arg_info) = stack;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ANRPid(node *arg_node, info *arg_info)
 *
 * @brief If we are in a with3 and looking at arguments of a fun ap then
 *        create pre assign to stop rc and post assign to restore the rc mode.
 *
 *****************************************************************************/
node *
ANRPid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANRPid");

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_ARGS_2_PRF (arg_info)) {
        node *avis = TBmakeAvis (TRAVtmpVar (), NULL);

        /*
         * rc avis;
         * ...
         * avis = 2norc( id);
         * ...
         * ap( id);
         * ...
         * id = restoreRc( avis);
         *
         */

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                     TBmakePrf (F_2norc, TBmakeExprs (arg_node, NULL))),
                          INFO_PREASSIGN (arg_info));

        INFO_POSTASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (arg_node, NULL),
                                     TBmakePrf (F_restorerc,
                                                TBmakeExprs (TBmakeId (avis), NULL))),
                          INFO_POSTASSIGN (arg_info));
    }

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 *
 * @fn node *ANRPfundef(node *arg_node, info *arg_info)
 *
 * @brief Save any vardecs in info into fundef vardec chain.
 *
 *****************************************************************************/
node *
ANRPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANRPfundef");

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_VARDECS (arg_info) != NULL) {
        BLOCK_VARDEC (FUNDEF_BODY (arg_node))
          = TCappendVardec (BLOCK_VARDEC (FUNDEF_BODY (arg_node)),
                            INFO_VARDECS (arg_info));
        INFO_VARDECS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
