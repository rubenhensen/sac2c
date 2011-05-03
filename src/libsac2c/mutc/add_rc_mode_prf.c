/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup armp add rc mode prfs
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
 * rfv1_oldrc = _2norc_( rfv1);
 * rfvn_oldrc = _2norc_( rfvn);
 * a = with3 {
 *   ...
 *   v1 = rfv1;
 *   ...
 *   vn = rfvn;
 *   ...
 * }
 * rfv1 = _restorerc2_( rfv1_oldrc, rfv1);
 * rfvn = _restorerc2_( rfvn_oldrc, rfvn);
 *
 *
 * res = spawn foo( a);
 *
 * arg0 = _2ayncrc_( a);
 * res = spawn foo( arg0);
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file add_rc_mode_prf.c
 *
 * Prefix: armp
 *
 *****************************************************************************/
#include "add_rc_mode_prf.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "DupTree.h"
#include "new_types.h"
#include "shape.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
typedef enum { RCM_null, RCM_norc, RCM_async } rcmode_t;

struct INFO {
    bool with3;
    rcmode_t args_2_prf;
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
    INFO_ARGS_2_PRF (result) = RCM_null;
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

    DBUG_ASSERT (INFO_PREASSIGN (info) == NULL, "Possible memory leak");

    DBUG_ASSERT (INFO_POSTASSIGN (info) == NULL, "Possible memory leak");

    DBUG_ASSERT (INFO_VARDECS (info) == NULL, "Possible memory leak");

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
 * @fn node *ARMPdoAddRcModePrf( node *syntax_tree)
 *
 *****************************************************************************/
node *
ARMPdoAddRcModePrf (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ARMPdoAddRcModePrf");

    info = MakeInfo ();

    DBUG_PRINT ("ARMP", ("Starting Add RC Mode PRF traversal."));

    TRAVpush (TR_armp);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("ARMP", ("Add RC Mode complete."));

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
 * @fn node *ARMPwith3(node *arg_node, info *arg_info)
 *
 * @brief mark the info that we are in a with3
 *
 *****************************************************************************/
node *
ARMPwith3 (node *arg_node, info *arg_info)
{
    bool stack;
    DBUG_ENTER ("ARMPwith3");

    stack = INFO_WITH3 (arg_info);
    INFO_WITH3 (arg_info) = TRUE;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_WITH3 (arg_info) = stack;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ARMPassign(node *arg_node, info *arg_info)
 *
 * @brief Insert pre and post assign
 *
 *****************************************************************************/
node *
ARMPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ARMPwith3");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

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
 * @fn node *ARMPap(node *arg_node, info *arg_info)
 *
 * @brief If this is a thread fun ap in a with3 then the arguments are the
 *        relative free variables in one of the partitions.
 *        Add pre assign and post assign to convert from and to norc mode
 *        Add needed temporary variables
 *
 *****************************************************************************/
node *
ARMPap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ARMPap");

    arg_node = TRAVcont (arg_node, arg_info);

    if (FUNDEF_WASWITH3BODY (AP_FUNDEF (arg_node))) {
        bool stack = FALSE;

        DBUG_ASSERT (INFO_WITH3 (arg_info), "With3 thread function not in a with loop");

        /* Could do with check that all args are ids */

        stack = INFO_ARGS_2_PRF (arg_info);
        INFO_ARGS_2_PRF (arg_info) = RCM_norc;
        arg_node = TRAVcont (arg_node, arg_info);
        INFO_ARGS_2_PRF (arg_info) = stack;
    } else if (AP_ISSPAWNED (arg_node)) {
        rcmode_t stack = FALSE;
        stack = INFO_ARGS_2_PRF (arg_info);
        INFO_ARGS_2_PRF (arg_info) = RCM_async;
        arg_node = TRAVcont (arg_node, arg_info);
        INFO_ARGS_2_PRF (arg_info) = stack;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ARMPid(node *arg_node, info *arg_info)
 *
 * @brief If we are in a with3 and looking at arguments of a fun ap then
 *        create pre assign to stop rc and post assign to restore the rc mode.
 *
 *****************************************************************************/
node *
ARMPid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ARMPid");

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_ARGS_2_PRF (arg_info) == RCM_norc) {
        node *postAssign = NULL;
        node *avis = TBmakeAvis (TRAVtmpVar (),
                                 TYmakeAKS (TYmakeSimpleType (T_rc), SHmakeShape (0)));

        /*
         * rc avis;
         * ...
         * avis = 2norc( id);
         * ...
         * ap( id);
         * ...
         * id = restoreRc( avis, id);
         *
         */

        INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

        AVIS_DECL (avis) = INFO_VARDECS (arg_info);

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                     TBmakePrf (F_2norc,
                                                TBmakeExprs (DUPdoDupNode (arg_node),
                                                             NULL))),
                          INFO_PREASSIGN (arg_info));

        postAssign
          = TBmakeAssign (TBmakeLet (TBmakeIds (ID_AVIS (arg_node), NULL),
                                     TBmakePrf (F_restorerc,
                                                TBmakeExprs (TBmakeId (avis),
                                                             TBmakeExprs (TBmakeId (
                                                                            ID_AVIS (
                                                                              arg_node)),
                                                                          NULL)))),
                          NULL);
        if (INFO_POSTASSIGN (arg_info) == NULL) {
            INFO_POSTASSIGN (arg_info) = postAssign;
        } else {
            INFO_POSTASSIGN (arg_info)
              = TCappendAssign (INFO_POSTASSIGN (arg_info), postAssign);
        }

    } else if (INFO_ARGS_2_PRF (arg_info) == RCM_async) {

        node *avis
          = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (ID_AVIS (arg_node))));

        /*
         * TYPE(id) id';
         * ...
         * id' = _2asyncrc_( id);
         * ...
         * ap( id');
         *
         */

        INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                     TBmakePrf (F_2asyncrc,
                                                TBmakeExprs (arg_node, NULL))),
                          INFO_PREASSIGN (arg_info));
        /* Note old arg_node used in assign above */
        arg_node = TBmakeId (avis);
    }

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 *
 * @fn node *ARMPfundef(node *arg_node, info *arg_info)
 *
 * @brief Save any vardecs in info into fundef vardec chain.
 *
 *****************************************************************************/
node *
ARMPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ARMPfundef");

    /* Next first */
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_OBJECTS (arg_node) = TRAVopt (FUNDEF_OBJECTS (arg_node), arg_info);
    FUNDEF_AFFECTEDOBJECTS (arg_node)
      = TRAVopt (FUNDEF_AFFECTEDOBJECTS (arg_node), arg_info);
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

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
