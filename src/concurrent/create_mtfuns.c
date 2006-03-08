/*****************************************************************************
 *
 * $Id:$
 *
 * file:   create_mtfuns.c
 *
 * prefix: CMTF
 *
 * description:
 *
 *   This file initiates creates for each exported or provided function of
 *   a module a copy designated for (replicated) multithreaded execution.
 *
 *****************************************************************************/

#include "dbug.h"
#include "tree_basic.h"
#include "traverse.h"
#include "DupTree.h"
#include "internal_lib.h"

#include "create_mtfuns.h"

/******************************************************************************
 *
 * @fn CMTFdoCreateMtFuns
 *
 *  @brief initiates CMTF traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
CMTFdoCreateMtFuns (node *syntax_tree)
{
    DBUG_ENTER ("CMTFdoCreateMtFuns");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!!!");

    TRAVpush (TR_cmtf);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * @fn CMTFmodule
 *
 *  @brief CMTF traversal function for N_module node
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
CMTFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CMTFmodule");

    if ((MODULE_FILETYPE (arg_node) == F_modimp)
        || (MODULE_FILETYPE (arg_node) == F_classimp)) {
        if (MODULE_FUNS (arg_node) != NULL) {
            MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
        }
        if (MODULE_FUNDECS (arg_node) != NULL) {
            MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn CMTFfundef
 *
 *  @brief CMTF traversal function for N_fundef node
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
CMTFfundef (node *arg_node, info *arg_info)
{
    node *mtvariant;
    char *funname, *mtname;

    DBUG_ENTER ("CMTFfundef");

    if (FUNDEF_ISEXPORTED (arg_node) || FUNDEF_ISPROVIDED (arg_node)) {
        if (FUNDEF_MTVARIANT (arg_node) == NULL) {
            /*
             * The MT variant has not yet been created.
             */
            mtvariant = DUPdoDupNode (arg_node);
            FUNDEF_MTVARIANT (arg_node) = mtvariant;

            funname = FUNDEF_NAME (mtvariant);
            mtname = ILIBstringConcat (funname, "__MT");
            ILIBfree (funname);
            FUNDEF_NAME (mtvariant) = mtname;
            FUNDEF_ISMTFUN (mtvariant) = TRUE;

            FUNDEF_NEXT (mtvariant) = arg_node;
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (mtvariant);
}
