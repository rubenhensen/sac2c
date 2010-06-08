/** <!--********************************************************************-->
 *
 * @file tf_build_graph.c
 *
 * prefix: TFBDG
 *
 * description:
 *
 *****************************************************************************/

#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tf_preprocess_graph.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "tree_compound.h"

/*
 * INFO structure
 * dfwid is the depth first walk id for the nodes in the dependency
 * graph. dfwidmax is the maximum value of the dfwid of the tree
 * decendants of a node
 */
struct INFO {
    int dfwid;
};

/*
 * INFO macros
 */
#define INFO_DFWID(n) n->dfwid
#define INFO_DFWIDMAX(n) n->dfwidmax
/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));
    INFO_DFWID (result) = 1;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFPPGdoPreprocessTFGraph( node *syntax_tree)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param syntax_tree
 *   @return
 *
 *****************************************************************************/
node *
TFPPGdoPreprocessTFGraph (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("TFPPGdoPreprocessTFGraph");

    arg_info = MakeInfo ();

    TRAVpush (TR_tfppg);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFPPGtfspec( node *arg_node, info *arg_info)
 *
 *   @brief
 *   We loop through the defs in the type family specifications to
 *   identify any potential apex nodes.
 *
 *   @param arg_node
 *   @param arg_info
 *
 *   @return
 *
 *****************************************************************************/
node *
TFPPGtfspec (node *arg_node, info *arg_info)
{
    node *defs;
    DBUG_ENTER ("TFPPGtfspec");
    defs = TFSPEC_DEFS (arg_node);
    while (defs != NULL) {
        if (TFDEF_DFWID (defs) == 0 && TFDEF_SUPERS (defs) == NULL) {
            /*
             * Here could be the start of a potentially new type family
             * component since we have multiple components in one family.
             *
             * For example, \alpha and [*] both require a fresh depth first
             * walk.
             */
            if (INFO_DFWID (arg_info) != 1) {
                INFO_DFWID (arg_info) = 1;
            }
            TRAVdo (defs, arg_info);
        }
        defs = TFDEF_NEXT (defs);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFPPGtfdef( node *arg_node, info *arg_info)
 *
 *   @brief
 *   We walk through the dependency graph here. If the node has not
 *   been visited i.e. its dfwid is 0, we update the dfwid of
 *   the node. Then, we check the subs (subtypes) of the def and if
 *   they are not visited, we visit them.
 *
 *   @param arg_node
 *   @param arg_info
 *
 *   @return
 *
 *****************************************************************************/
node *
TFPPGtfdef (node *arg_node, info *arg_info)
{
    node *defs, *subs;
    DBUG_ENTER ("TFPPGtfdef");
    defs = arg_node;
    subs = TFDEF_SUBS (defs);
    TFDEF_DFWID (defs) = INFO_DFWID (arg_info)++;
    while (subs != NULL) {
        if (TFDEF_DFWID (TFSUPERSUB_TYPEFAMILY (subs)) == 0) {
            /*
             * Tree branch
             */
            TRAVdo (TFSUPERSUB_TYPEFAMILY (subs), arg_info);
        } else {
            /*
             * Cross branch here. Refer literature on Directed Acyclic
             * Graphs.
             */
        }
        subs = TFSUPERSUB_NEXT (subs);
    }
    /*
     * We have traversed all descendants of this node. Its time to
     * update the value of dfwidmax which is the maximum value of the
     * dfwid of all tree descendant of this node
     */
    TFDEF_DFWIDMAX (defs) = INFO_DFWID (arg_info);
    DBUG_RETURN (arg_node);
}
