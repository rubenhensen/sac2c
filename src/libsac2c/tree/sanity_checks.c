/*****************************************************************************
 *
 * file: sanity_checks.c
 *
 * This file implements a set of tree consistency checks. It can be used for
 * non-exhaustive checking as long as the real tree check mechanism is not
 * yet operational due to a lack of accuracy in the AST specification.
 *
 *****************************************************************************/

#include "sanity_checks.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "phase_info.h"
#include "traverse.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "globals.h"
#include "DupTree.h"

#ifdef SANITYCHECKS

static bool below_module_node = FALSE;
static node *module = NULL;

void
SANCHKdoSanityChecksPreTraversal (node *arg_node, info *arg_info, void *travstack)
{
    node *ids;

    DBUG_ENTER ();

    if (arg_node == NULL) {
        CTIerrorInternal ("Pre Traversal Sanity Check:\n"
                          "Tried to traverse into subtree NULL !");
    } else {
        if (NODE_TYPE (arg_node) > MAX_NODES) {
            CTIerrorInternal ("Pre Traversal Sanity Check:\n"
                              "Traversed into illegal node type !");
        }

        if (travstack == NULL) {
            CTIerrorInternal ("Pre Traversal Sanity Check\n"
                              "No traversal on stack!");
        }

        if ((NODE_TYPE (arg_node) == N_module)
            && (NULL != DUPgetCopiedSpecialFundefs ())) {
            CTIerrorInternal ("Pre Traversal Sanity Check:\n"
                              "Unresolved copies of special functions at beginning\n"
                              "of new traversal. Maybe a previous traversal is not\n"
                              "organized properly in the sense that it has a traversal\n"
                              "function for the N_module node.");
        }

        if (global.valid_ssaform && (NODE_TYPE (arg_node) == N_assign)
            && (NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let)) {
            ids = LET_IDS (ASSIGN_STMT (arg_node));

            while (ids != NULL) {
                if (AVIS_SSAASSIGN (IDS_AVIS (ids)) == NULL) {
                    CTIerrorInternal ("Pre Traversal Sanity Check:\n"
                                      "Missing SSA_ASSIGN link for variable %s!",
                                      AVIS_NAME (IDS_AVIS (ids)));
                } else if (AVIS_SSAASSIGN (IDS_AVIS (ids)) != arg_node) {
                    CTIerrorInternal ("Pre Traversal Sanity Check:\n"
                                      "Broken SSA_ASSIGN link for variable %s!",
                                      AVIS_NAME (IDS_AVIS (ids)));
                }

                ids = IDS_NEXT (ids);
            }
        }

        if (NODE_TYPE (arg_node) == N_fundef) {
            global.current_fundef = arg_node;
        }

        if (NODE_TYPE (arg_node) == N_module) {
            below_module_node = TRUE;
            if (module == NULL) {
                module = arg_node;
            } else {
                if (arg_node != module) {
                    CTIerrorInternal ("Pre Traversal Sanity Check:\n"
                                      "Root node of syntax tree (N_module) has changed!");
                }
            }
        } else {
            if (!below_module_node && (global.compiler_anyphase > PH_scp_prs)
                && (global.compiler_anyphase < PH_cg_frtr)) {
#if 0
        CTIerrorInternal( "Pre Traversal Sanity Check:\n"
                          "Reached %s node without having passed N_module node!\n"
                          "(Only one occurrence reported per phase)",
                          global.mdb_nodetype[NODE_TYPE( arg_node)]);
        below_module_node = TRUE;
#endif
            }
        }
    }

    DBUG_RETURN ();
}

void
SANCHKdoSanityChecksPostTraversal (node *arg_node, info *arg_info, void *travstack)
{
    node *ids;

    DBUG_ENTER ();

    if (global.valid_ssaform && (NODE_TYPE (arg_node) == N_assign)
        && (NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let)) {
        ids = LET_IDS (ASSIGN_STMT (arg_node));

        while (ids != NULL) {
            if (AVIS_SSAASSIGN (IDS_AVIS (ids)) == NULL) {
                CTIerrorInternal ("Post Traversal Sanity Check:\n"
                                  "Missing SSA_ASSIGN link for variable %s!",
                                  AVIS_NAME (IDS_AVIS (ids)));
            } else if (AVIS_SSAASSIGN (IDS_AVIS (ids)) != arg_node) {
                CTIerrorInternal ("Post Traversal Sanity Check:\n"
                                  "Broken SSA_ASSIGN link for variable %s!",
                                  AVIS_NAME (IDS_AVIS (ids)));
            }

            ids = IDS_NEXT (ids);
        }
    }

    if (NODE_TYPE (arg_node) == N_fundef) {
        global.current_fundef = NULL;
    }

    if (NODE_TYPE (arg_node) == N_module) {
        below_module_node = FALSE;
    }

    DBUG_RETURN ();
}

#endif

#undef DBUG_PREFIX
