/*
 *
 * $Log$
 * Revision 1.1  2002/02/21 15:12:36  sbs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   insert_vardec.c
 *
 * prefix: INSVD
 *
 * description:
 *
 *   This compiler module inserts vardecs for all identifyers that do not have
 *   one yet and adds proper backrefs to them.
 *   It is needed for running InferDFMS prior to type checking.
 *
 * usage of arg_info (INFO_INSVD_...):
 *
 *   ...VARDECS  holds the pointer to the vardec chain of the actual fundef !
 *   ...ARGS     holds the pointer to the argument chain of the actual fundef !
 *
 *****************************************************************************/

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"

#include "insert_vardec.h"

#define INFO_INSVD_VARDECS(n) (n->node[0])
#define INFO_INSVD_ARGS(n) (n->node[1])

static funtab insvd_tab_rec = {{
#define NIFinsvd(it_insvd) it_insvd
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *insvd_tab = &insvd_tab_rec;

/******************************************************************************
 *
 * function:
 *  node * SearchForName( char *name, node *vardecs)
 *
 * description:
 *   looks up the name in the vardecs. Returns the address of the vardec
 *   if found, NULL otherwise.
 *
 ******************************************************************************/

node *
SearchForName (char *name, node *vardecs)
{
    DBUG_ENTER ("SearchForName");

    while ((vardecs != NULL) && (strcmp (VARDEC_NAME (vardecs), name) != 0)) {
        vardecs = VARDEC_NEXT (vardecs);
    }
    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *  node * CheckIds( ids *idents, node *vardecs)
 *
 * description:
 *   looks up each identifyer of the idents within the vardecs. If found
 *   a propoer backref is set only, otherwise, an appropriate vardec is pre-
 *   panded to the vardecs. The (potentially enlarged) vardecs are returned.
 *
 ******************************************************************************/

node *
CheckIds (ids *idents, node *vardecs)
{
    node *vardec;

    DBUG_ENTER ("CheckIds");

    while (idents != NULL) {
        vardec = SearchForName (IDS_NAME (idents), vardecs);
        if (vardec == NULL) {
            /*
             * The identifyer we are looking for does not have a
             * vardec yet! So we allocate one and prepand it to vardecs.
             */
            vardec = MakeVardec (IDS_NAME (idents), NULL, vardecs);
            vardecs = vardec;
        }
        IDS_VARDEC (idents) = vardec;
        idents = IDS_NEXT (idents);
    }
    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDfundef( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INSVDfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_INSVD_VARDECS (arg_info) = FUNDEF_VARDEC (arg_node);
        INFO_INSVD_ARGS (arg_info) = FUNDEF_ARGS (arg_node);

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_VARDEC (arg_node) = INFO_INSVD_VARDECS (arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDlet( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INSVDlet");

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_INSVD_VARDECS (arg_info)
      = CheckIds (LET_IDS (arg_node), INFO_INSVD_VARDECS (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSVDid( node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSVDid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INSVDid");

    INFO_INSVD_VARDECS (arg_info)
      = CheckIds (ID_IDS (arg_node), INFO_INSVD_VARDECS (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node * InsertVardecs( node *syntax_tree)
 *
 * description:
 *   Traverses through all functions and inserts vardecs for all identifyers
 *   that do not have one yet. Whenever an N_id node is met, we simply look for
 *   its declaration. If found, a backref to it is inserted (ID_VARDEC).
 *   Otherwise, a new vardec is inserted and a backref is inserted.
 *   The same holds for the ids found at N_let nodes.
 *
 ******************************************************************************/

node *
InsertVardecs (node *syntax_tree)
{
    funtab *old_funtab;
    node *info_node;

    DBUG_ENTER ("InsertVardecs");

    old_funtab = act_tab;
    act_tab = insvd_tab;

    info_node = MakeInfo ();
    syntax_tree = Trav (syntax_tree, NULL);
    act_tab = old_funtab;

    info_node = FreeNode (info_node);

    DBUG_RETURN (syntax_tree);
}
