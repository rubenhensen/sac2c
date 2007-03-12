/*
 *
 * $Id$
 *
 */

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

#include "dbug.h"
#include "phase_info.h"
#include "traverse.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "globals.h"
#include "DupTree.h"

static node *fundef = NULL;
static bool below_module_node = FALSE;

#ifdef SANITYCHECKS

void
SANCHKdoSanityChecksPreTraversal (node *arg_node, info *arg_info, void *travstack)
{
    node *ids;

    DBUG_ENTER ("SANCHKdoSanityChecksPreTraversal");

    if (arg_node == NULL) {
        CTIerrorInternal ("Pre Traversal Sanity Check:\n"
                          "Tried to traverse into subtree NULL !\n"
                          "Compiler phase:    %s\n"
                          "                   %s\n"
                          "Traversal:         %s\n"
                          "Function:          %s( %s)",
                          AVIS_NAME (IDS_AVIS (ids)),
                          PHIphaseName (global.compiler_anyphase),
                          PHIphaseText (global.compiler_anyphase), TRAVgetName (),
                          CTIitemName (fundef), CTIfunParams (fundef));
    } else {
        if (NODE_TYPE (arg_node) > MAX_NODES) {
            CTIerrorInternal ("Pre Traversal Sanity Check:\n"
                              "Traversed into illegal node type !\n"
                              "Compiler phase:    %s\n"
                              "                   %s\n"
                              "Traversal:         %s\n"
                              "Function:          %s( %s)",
                              AVIS_NAME (IDS_AVIS (ids)),
                              PHIphaseName (global.compiler_anyphase),
                              PHIphaseText (global.compiler_anyphase), TRAVgetName (),
                              CTIitemName (fundef), CTIfunParams (fundef));
        }

        if (travstack == NULL) {
            CTIerrorInternal ("Pre Traversal Sanity Check\n"
                              "No traversal on stack!\n"
                              "Compiler phase:    %s\n"
                              "                   %s\n"
                              "Traversal:         %s\n"
                              "Function:          %s( %s)",
                              AVIS_NAME (IDS_AVIS (ids)),
                              PHIphaseName (global.compiler_anyphase),
                              PHIphaseText (global.compiler_anyphase), TRAVgetName (),
                              CTIitemName (fundef), CTIfunParams (fundef));
        }

        if ((NODE_TYPE (arg_node) == N_module)
            && (NULL != DUPgetCopiedSpecialFundefs ())) {
            CTIerrorInternal ("Pre Traversal Sanity Check:\n"
                              "Unresolved copies of special functions at beginning\n"
                              "of new traversal. Maybe a previous traversal is not\n"
                              "organized properly in the sense that it has a traversal\n"
                              "function for the N_module node.\n"
                              "Compiler phase:    %s\n"
                              "                   %s\n"
                              "Traversal:         %s\n"
                              "Function:          %s( %s)",
                              AVIS_NAME (IDS_AVIS (ids)),
                              PHIphaseName (global.compiler_anyphase),
                              PHIphaseText (global.compiler_anyphase), TRAVgetName (),
                              CTIitemName (fundef), CTIfunParams (fundef));
        }

        if (global.valid_ssaform && (NODE_TYPE (arg_node) == N_assign)
            && (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)) {
            ids = LET_IDS (ASSIGN_INSTR (arg_node));

            while (ids != NULL) {
                if (AVIS_SSAASSIGN (IDS_AVIS (ids)) != arg_node) {
                    CTIerrorInternal ("Pre Traversal Sanity Check:\n"
                                      "Broken SSAASSIGN link for variable %s!\n"
                                      "Compiler phase:    %s\n"
                                      "                   %s\n"
                                      "Traversal:         %s\n"
                                      "Function:          %s( %s)",
                                      AVIS_NAME (IDS_AVIS (ids)),
                                      PHIphaseName (global.compiler_anyphase),
                                      PHIphaseText (global.compiler_anyphase),
                                      TRAVgetName (), CTIitemName (fundef),
                                      CTIfunParams (fundef));
                }

                ids = IDS_NEXT (ids);
            }
        }

        if (NODE_TYPE (arg_node) == N_fundef) {
            fundef = arg_node;
        }

        if (NODE_TYPE (arg_node) == N_module) {
            below_module_node = TRUE;
        } else {
            if (!below_module_node) {
                CTIerrorInternal ("Pre Traversal Sanity Check:\n"
                                  "Reached %s node without having passed N_module node!\n"
                                  "Compiler phase:    %s\n"
                                  "                   %s\n"
                                  "Traversal:         %s\n"
                                  "Function:          %s( %s)",
                                  global.mdb_nodetype[NODE_TYPE (arg_node)],
                                  PHIphaseName (global.compiler_anyphase),
                                  PHIphaseText (global.compiler_anyphase), TRAVgetName (),
                                  CTIitemName (fundef), CTIfunParams (fundef));
            }
        }
    }

    DBUG_VOID_RETURN;
}

void
SANCHKdoSanityChecksPostTraversal (node *arg_node, info *arg_info, void *travstack)
{
    node *ids;

    DBUG_ENTER ("SANCHKdoSanityChecksPostTraversal");

    if (global.valid_ssaform && (NODE_TYPE (arg_node) == N_assign)
        && (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)) {
        ids = LET_IDS (ASSIGN_INSTR (arg_node));

        while (ids != NULL) {
            if (AVIS_SSAASSIGN (IDS_AVIS (ids)) != arg_node) {
                CTIerrorInternal ("Post Traversal Sanity Check:\n"
                                  "Broken SSAASSIGN link for variable %s!\n"
                                  "Compiler phase:    %s\n"
                                  "                   %s\n"
                                  "Traversal:         %s\n"
                                  "Function:          %s( %s)",
                                  AVIS_NAME (IDS_AVIS (ids)),
                                  PHIphaseName (global.compiler_anyphase),
                                  PHIphaseText (global.compiler_anyphase), TRAVgetName (),
                                  CTIitemName (fundef), CTIfunParams (fundef));
            }
        }
    }

    if (NODE_TYPE (arg_node) == N_fundef) {
        fundef = NULL;
    }

    if (NODE_TYPE (arg_node) == N_module) {
        below_module_node = FALSE;
    }

    DBUG_VOID_RETURN;
}

#endif
