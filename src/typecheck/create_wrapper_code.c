/*
 *
 * $Log$
 * Revision 1.4  2002/08/13 15:59:09  dkr
 * some more cwc stuff added (not finished yet)
 *
 * Revision 1.3  2002/08/09 14:50:53  dkr
 * CWCap added
 *
 * Revision 1.2  2002/08/09 13:15:20  dkr
 * CWCmodul, CWCfundef added
 *
 * Revision 1.1  2002/08/09 13:00:02  dkr
 * Initial revision
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "new_types.h"

#define INFO_CWC_TRAVNO(n) n->flag

/******************************************************************************
 *
 * Function:
 *   node *CWCmodul( node *arg_node, node *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CWCmodul");

    /*
     * create separate wrapper function for all base type constellations
     */
    INFO_CWC_TRAVNO (arg_info) = 1;
    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    /*
     * adjust AP_FUNDEF pointers
     */
    INFO_CWC_TRAVNO (arg_info) = 2;
    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    /*
     * create separate wrapper function for all base type constellations
     */
    INFO_CWC_TRAVNO (arg_info) = 3;
    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *SplitWrapper( node *fundef)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
SplitWrapper (node *fundef)
{
    ntype *old_type, *tmp_type;
    ntype *new_type;
    bool finished;
    node *new_fundef;
    node *new_fundefs = NULL;

    DBUG_ENTER ("SplitWrapper");

    old_type = FUNDEF_TYPE (fundef);
    tmp_type = TYCopyType (old_type);
    FUNDEF_TYPE (fundef) = NULL;
    do {
        new_fundef = DupNode (fundef);
        new_type = TYSplitWrapperType (tmp_type, &finished);
        FUNDEF_TYPE (new_fundef) = new_type;
        FUNDEF_RET_TYPE (new_fundef) = TYGetWrapperRetType (new_type);
        FUNDEF_ARGS (new_fundef)
          = TYCorrectWrapperArgTypes (FUNDEF_ARGS (new_fundef), new_type);
        FUNDEF_NEXT (new_fundef) = new_fundefs;
        new_fundefs = new_fundef;
    } while (!finished);
    FUNDEF_TYPE (fundef) = old_type;
    tmp_type = TYFreeType (tmp_type);

    DBUG_RETURN (new_fundefs);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertWrapperCode( node *fundef)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InsertWrapperCode (node *fundef)
{
    node *ret_exprs;
    node *assigns;
    node *vardec;
    node *vardecs;

    DBUG_ENTER ("InsertWrapperCode");

    /*
     * generate wrapper code together with the needed vardecs
     */
    vardecs = TYCreateWrapperVardecs (fundef);
    assigns = TYCreateWrapperCode (fundef, vardecs);

    /*
     * vardecs -> return exprs
     */
    ret_exprs = NULL;
    vardec = vardecs;
    while (vardec != NULL) {
        node *ret_expr;
        node *id_node = MakeId_Copy (VARDEC_NAME (vardec));
        ID_VARDEC (id_node) = vardec;
        ID_AVIS (id_node) = VARDEC_AVIS (vardec);

        if (ret_exprs == NULL) {
            ret_expr = ret_exprs = MakeExprs (id_node, NULL);
        } else {
            ret_expr = EXPRS_NEXT (ret_expr) = MakeExprs (id_node, NULL);
        }

        vardec = VARDEC_NEXT (vardec);
    }

    /*
     * append return statement to assignments
     */
    assigns = AppendAssign (assigns, MakeAssign (MakeReturn (ret_exprs), NULL));

    /*
     * insert function body
     */
    FUNDEF_BODY (fundef) = MakeBlock (assigns, vardecs);

    /*
     * mark wrapper function as a inline function
     */
    FUNDEF_INLINE (fundef) = TRUE;

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCfundef( node *arg_node, node *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCfundef (node *arg_node, node *arg_info)
{
    node *new_fundef;
    node *new_fundefs;

    DBUG_ENTER ("CWCfundef");

    if (INFO_CWC_TRAVNO (arg_info) == 1) {
        /*
         * first traversal -> build wrapper functions and there bodies
         */

        if (FUNDEF_STATUS (arg_node) == ST_wrapperfun) {
            DBUG_ASSERT ((FUNDEF_BODY (arg_node) == NULL),
                         "wrapper function has already a body!");

            /*
             * build a separate fundef for each base type constellation
             */
            new_fundefs = SplitWrapper (arg_node);

            /*
             * build code for all wrapper functions
             */
            new_fundef = new_fundefs;
            DBUG_ASSERT ((new_fundef != NULL), "no wrapper functions found!");
            do {
                new_fundef = InsertWrapperCode (new_fundef);
                new_fundef = FUNDEF_NEXT (new_fundef);
            } while (new_fundef != NULL);

            if (FUNDEF_NEXT (arg_node) != NULL) {
                FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
            }

            /*
             * insert new wrapper functions behind the original one and
             * free original wrapper function (-> zombie function)
             */
            new_fundefs = AppendFundef (new_fundefs, FUNDEF_NEXT (arg_node));
            arg_node = FreeNode (arg_node);
            DBUG_ASSERT (((arg_node != NULL)
                          && (FUNDEF_STATUS (arg_node) == ST_zombiefun)),
                         "zombie fundef not found!");
            FUNDEF_NEXT (arg_node) = new_fundefs;
        }
    } else if (INFO_CWC_TRAVNO (arg_info) == 2) {
        /*
         * second traversal -> adjust all AP_FUNDEF pointers
         *
         * This is needed if the original wrapper function was valid for more than
         * a single base type.
         */
        if ((FUNDEF_STATUS (arg_node) != ST_wrapperfun)
            && (FUNDEF_BODY (arg_node) != NULL)) {
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }
    } else {
        DBUG_ASSERT ((INFO_CWC_TRAVNO (arg_info) == 3), "illegal INFO_CWC_TRAVNO found!");
        /*
         * third traversal -> remove zombie functions
         */

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }

        if (FUNDEF_STATUS (arg_node) == ST_zombiefun) {
            arg_node = FreeZombie (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCap( node *arg_node, node *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCap (node *arg_node, node *arg_info)
{
    node *fundef;

    DBUG_ENTER ("CWCap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    fundef = AP_FUNDEF (arg_node);
    DBUG_ASSERT ((fundef != NULL), "AP_FUNDEF not found!");
    if (FUNDEF_STATUS (fundef) == ST_zombiefun) {
        do {
            fundef = FUNDEF_NEXT (fundef);
            DBUG_ASSERT ((fundef != NULL), "no appropriate wrapper function found!");
        } while (0); /* !!! not implemented yet !!! */
        DBUG_ASSERT ((!strcmp (AP_NAME (arg_node), FUNDEF_NAME (fundef))),
                     "no appropriate wrapper function found!");
        AP_FUNDEF (arg_node) = fundef;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateWrapperCode( node *ast)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CreateWrapperCode (node *ast)
{
    funtab *tmp_tab;
    node *info_node;

    DBUG_ENTER ("CreateWrapperCode");

    tmp_tab = act_tab;
    act_tab = cwc_tab;

    info_node = MakeInfo ();
    ast = Trav (ast, info_node);
    info_node = FreeNode (info_node);

    act_tab = tmp_tab;

    DBUG_RETURN (ast);
}
