/** <!--********************************************************************-->
 *
 * @file dagbuilder.c
 *
 * prefix: TFBDG
 *
 * description: This file constructs the subtyping graph from the type relations
 * input by the user.
 *
 *****************************************************************************/

#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "tree_compound.h"
#include "dagbuilder.h"

/*
 * INFO structure
 */
struct INFO {
    char *supertag;
    char *subtag;
    node *cond;
};

/*
 * INFO macros
 */
#define INFO_SUPERTAG(n) n->supertag
#define INFO_SUBTAG(n) n->subtag
#define INFO_COND(n) n->cond

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));
    INFO_SUPERTAG (result) = NULL;
    INFO_SUBTAG (result) = NULL;
    INFO_COND (result) = NULL;

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
 * @fn node *TFBDGdoBuildTFGraph( node *syntax_tree)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TFBDGdoBuildTFGraph (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ("TFBDGdoBuildTFGraph");

    arg_info = MakeInfo ();

    TRAVpush (TR_tfbdg);

    syntax_tree = TRAVdo (syntax_tree, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *TFBDGtfspec( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TFBDGtfspec (node *arg_node, info *arg_info)
{
    node *rels;

    DBUG_ENTER ("TFBDGtfspec");

    if (TFSPEC_RELS (arg_node) != NULL) {
        rels = TFSPEC_RELS (arg_node);
        while (rels != NULL) {
            /* copy the supertag, subtag and cond pointers to the info
             * structure
             */
            INFO_SUPERTAG (arg_info) = TFREL_SUPERTAG (rels);
            INFO_SUBTAG (arg_info) = TFREL_SUBTAG (rels);
            INFO_COND (arg_info) = TFREL_COND (rels);
            if (TFSPEC_DEFS (arg_node) != NULL) {
                TRAVdo (TFSPEC_DEFS (arg_node), arg_info);
            }
            /*
             * free the relation node
             */
            TFSPEC_RELS (arg_node) = FREEdoFreeNode (TFSPEC_RELS (arg_node));
            rels = TFSPEC_RELS (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

static void
TFBDGaddEdge (node *super, node *sub, node *cond)
{

    DBUG_ENTER ("TFBDGaddEdge");

    node *itersuper, *itersub;

    itersuper = TFVERTEX_CHILDREN (super);

    if (itersuper == NULL) {

        TFVERTEX_CHILDREN (super) = TBmakeTfedge (sub, NULL, NULL);

        if (cond != NULL) {
            TFEDGE_COND (TFVERTEX_CHILDREN (super)) = DUPdoDupNode (cond);
        }

    } else {

        while (TFEDGE_NEXT (itersuper) != NULL) {
            itersuper = TFEDGE_NEXT (itersuper);
        }

        TFEDGE_NEXT (itersuper) = TBmakeTfedge (sub, NULL, NULL);

        if (cond != NULL) {
            TFEDGE_COND (itersuper) = DUPdoDupNode (cond);
        }
    }

    itersub = TFVERTEX_PARENTS (sub);

    if (itersub == NULL) {

        TFVERTEX_PARENTS (sub) = TBmakeTfedge (super, NULL, NULL);
        if (cond != NULL) {
            TFEDGE_COND (TFVERTEX_PARENTS (sub)) = DUPdoDupNode (cond);
        }

    } else {

        while (TFEDGE_NEXT (itersub) != NULL) {
            itersub = TFEDGE_NEXT (itersub);
        }

        TFEDGE_NEXT (itersub) = TBmakeTfedge (super, NULL, NULL);

        if (cond != NULL) {
            TFEDGE_COND (itersub) = DUPdoDupNode (cond);
        }
    }

    /*
     * Update the number of parents that the subtype has. This information is
     * necessary while topologically sorting the subtyping hierarchy.
     */

    TFVERTEX_NUMPARENTS (sub)++;

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn node *TFBDGtfvertex( node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
TFBDGtfvertex (node *arg_node, info *arg_info)
{
    node *defs, *super, *sub;

    DBUG_ENTER ("TFBDGtfvertex");

    super = NULL;
    sub = NULL;

    /*
     * First we need to find the super and sub nodes
     */

    defs = arg_node;

    while (defs != NULL) {

        if (super == NULL
            && STReq (INFO_SUPERTAG (arg_info), TFVERTEX_TAG (TFVERTEX_CURR (defs)))) {
            super = defs;
        }

        if (sub == NULL
            && STReq (INFO_SUBTAG (arg_info), TFVERTEX_TAG (TFVERTEX_CURR (defs)))) {
            sub = defs;
        }

        defs = TFVERTEX_NEXT (defs);
    }
    /*
     * Then we need to add the subtyping relationship to these nodes
     */
    if (super == NULL || sub == NULL) {
        CTIerror ("Sub/Super node not found for relation %s->%s",
                  INFO_SUPERTAG (arg_info), INFO_SUBTAG (arg_info));
    } else {
        TFBDGaddEdge (super, sub, INFO_COND (arg_info));
    }

    DBUG_RETURN (arg_node);
}
