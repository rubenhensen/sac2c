/*
 *
 * $Log$
 * Revision 1.4  2005/02/08 22:29:21  mwe
 * doxygen comments added
 *
 * Revision 1.3  2005/02/03 18:28:22  mwe
 * counter added
 * simplification-rules improved
 *
 * Revision 1.2  2005/02/02 21:09:25  mwe
 * corrected traversal
 *
 * Revision 1.1  2005/02/02 18:13:25  mwe
 * Initial revision
 *
 *
 */

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "new_types.h"
#include "dbug.h"
#include "internal_lib.h"
#include "optimize.h"
#include "free.h"
#include "globals.h"
#include "constants.h"
#include "shape.h"
#include "tree_compound.h"

#include "signature_simplification.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool remex;
    bool retex;
    bool isapnode;
    node *postassign;
    node *apfunrets;
    bool remassign;
};

#define INFO_SISI_FUNDEF(n) (n->fundef)
#define INFO_SISI_RETURNEXPRS(n) (n->retex)
#define INFO_SISI_REMOVEEXPRS(n) (n->remex)
#define INFO_SISI_ISAPNODE(n) (n->isapnode)
#define INFO_SISI_POSTASSIGN(n) (n->postassign)
#define INFO_SISI_APFUNRETS(n) (n->apfunrets)
#define INFO_SISI_REMOVEASSIGN(n) (n->remassign)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");
    result = ILIBmalloc (sizeof (info));

    INFO_SISI_FUNDEF (result) = NULL;
    INFO_SISI_RETURNEXPRS (result) = FALSE;
    INFO_SISI_REMOVEEXPRS (result) = FALSE;
    INFO_SISI_ISAPNODE (result) = FALSE;
    INFO_SISI_POSTASSIGN (result) = NULL;
    INFO_SISI_APFUNRETS (result) = NULL;
    INFO_SISI_REMOVEASSIGN (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIdoSignatureSimplification(node *module)
 *
 * @brief starting point of traversal
 *
 * @param module module to be traversed
 *
 * @result
 *
 ******************************************************************************/
node *
SISIdoSignatureSimplification (node *module)
{
    info *arg_info;
    DBUG_ENTER ("SISIdoSignatureSimplification");

    arg_info = MakeInfo ();

    TRAVpush (TR_sisi);
    module = TRAVdo (module, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (module);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISImodule(node *arg_node, info *arg_info)
 *
 * @brief traverses in funs
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISImodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIfundef(node *arg_node, info *arg_info)
 *
 * @brief top-down: traverse body, bottom-up: traverse rets and args
 *
 * @param arg_node fundef to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIfundef");

    INFO_SISI_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    INFO_SISI_FUNDEF (arg_info) = arg_node;

    if ((FUNDEF_RETS (arg_node) != NULL) && (!FUNDEF_ISLACFUN (arg_node))
        && (!ILIBstringCompare ("main", FUNDEF_NAME (arg_node)))
        && (!ILIBstringCompare (MAIN_MOD_NAME, FUNDEF_MOD (arg_node)))
        && (!FUNDEF_ISEXPORTED (arg_node)) && (FUNDEF_ISPROVIDED (arg_node))) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_ARGS (arg_node) != NULL)
        && (!FUNDEF_ISLACFUN (arg_node)) && (!FUNDEF_ISEXPORTED (arg_node))
        && (!FUNDEF_ISPROVIDED (arg_node))) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIarg(node *arg_node, info *arg_info)
 *
 * @brief Move scalar akv-arguments from args-chain to vardec-chain
 *    and create new assignment. Change non-scalar akv-args to aks-args.
 *
 *    Do not change signatures of exported or provided functions. Leave also
 *    LAC-functions as they are.
 *
 * @param arg_node args-chain to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIarg");

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    /*
     * now bottom-up traversal
     */

    if (TYisAKV (AVIS_TYPE (ARG_AVIS (arg_node)))) {

        if (TYgetDim (AVIS_TYPE (ARG_AVIS (arg_node))) == 0) {
            node *tmp, *assign_let;

            /*
             * we are dealing with an known scalar value
             */

            /*
             * create new assignment and new vardec
             */
            assign_let = TCmakeAssignLet (ARG_AVIS (arg_node),
                                          COconstant2AST (TYgetValue (
                                            AVIS_TYPE (ARG_AVIS (arg_node)))));
            BLOCK_VARDEC (FUNDEF_BODY (INFO_SISI_FUNDEF (arg_info)))
              = TBmakeVardec (ARG_AVIS (arg_node),
                              BLOCK_VARDEC (FUNDEF_BODY (INFO_SISI_FUNDEF (arg_info))));

            ASSIGN_NEXT (assign_let)
              = BLOCK_INSTR (FUNDEF_BODY (INFO_SISI_FUNDEF (arg_info)));
            BLOCK_INSTR (FUNDEF_BODY (INFO_SISI_FUNDEF (arg_info))) = assign_let;

            /*
             * delete argument from chain
             */
            tmp = ARG_NEXT (arg_node);
            ARG_NEXT (arg_node) = NULL;
            ARG_AVIS (arg_node) = NULL;
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = tmp;
        } else {
            /*
             * we are dealing with an known array value
             */
            ntype *old;
            old = AVIS_TYPE (ARG_AVIS (arg_node));
            AVIS_TYPE (ARG_AVIS (arg_node)) = TYmakeAKS (TYcopyType (TYgetScalar (old)),
                                                         SHcopyShape (TYgetShape (old)));
            old = TYfreeType (old);
        }
        sisi_expr++;
    }
    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIassign(node *arg_node, info *arg_info)
 *
 * @brief top-down: traverse in assignments,
 *        bottom-up: insert new assignments, remove marked assignments
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_SISI_POSTASSIGN (arg_info) = NULL;
    INFO_SISI_REMOVEASSIGN (arg_info) = FALSE;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (INFO_SISI_POSTASSIGN (arg_info) != NULL) {

        /*
         * insert removed constants into assign chain
         */
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_SISI_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
    }

    if (INFO_SISI_REMOVEASSIGN (arg_info)) {
        node *tmp;

        tmp = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = tmp;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIlet(node *arg_node, info *arg_info)
 *
 * @brief traverse first in rhs, then in lhs
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIlet");

    INFO_SISI_ISAPNODE (arg_info) = FALSE;

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    if ((INFO_SISI_ISAPNODE (arg_info)) && (LET_IDS (arg_node) != NULL)) {
        INFO_SISI_REMOVEASSIGN (arg_info) = TRUE;
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIap(node *arg_node, info *arg_info)
 *
 * @brief check fundef-args for scalar akv-args,
 *      remove corresponding exprs from args-chain
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIap (node *arg_node, info *arg_info)
{
    node *fundef, *fun_args, *curr_args, *new_args, *tmp;
    DBUG_ENTER ("SISIap");

    fundef = AP_FUNDEF (arg_node);

    if ((!FUNDEF_ISLACFUN (fundef)) && (!FUNDEF_ISPROVIDED (fundef))
        && (!FUNDEF_ISEXPORTED (fundef))) {

        INFO_SISI_APFUNRETS (arg_info) = FUNDEF_RETS (AP_FUNDEF (arg_node));
        fun_args = FUNDEF_ARGS (fundef);
        curr_args = AP_ARGS (arg_node);
        new_args = NULL;
        AP_ARGS (arg_node) = NULL;

        while (fun_args != NULL) {

            if ((!TYisAKV (AVIS_TYPE (ARG_AVIS (fun_args))))
                || (0 < TYgetDim (AVIS_TYPE (ARG_AVIS (fun_args))))) {
                if (NULL == new_args) {
                    new_args = curr_args;
                    AP_ARGS (arg_node) = new_args;
                } else {
                    EXPRS_NEXT (new_args) = curr_args;
                    new_args = EXPRS_NEXT (new_args);
                }
                curr_args = EXPRS_NEXT (curr_args);
            } else {

                /*
                 * argument is of type akv and scalar
                 */
                tmp = curr_args;
                curr_args = EXPRS_NEXT (curr_args);
                EXPRS_NEXT (tmp) = NULL;
                tmp = FREEdoFreeNode (tmp);
                if (new_args != NULL) {
                    EXPRS_NEXT (new_args) = NULL;
                }
            }
            fun_args = ARG_NEXT (fun_args);
        }

        INFO_SISI_ISAPNODE (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIids(node *arg_node, info *arg_info)
 *
 * @brief removes ids-nodes with scalar akv-types from ids-chain
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIids (node *arg_node, info *arg_info)
{
    node *assign_let, *succ, *ret;
    constant *new_co = NULL;

    DBUG_ENTER ("SISIids");

    ret = INFO_SISI_APFUNRETS (arg_info);
    if (RET_NEXT (INFO_SISI_APFUNRETS (arg_info)) != NULL) {
        INFO_SISI_APFUNRETS (arg_info) = RET_NEXT (INFO_SISI_APFUNRETS (arg_info));
    } else {
        DBUG_ASSERT ((IDS_NEXT (arg_node) == NULL), "ret and ids do not fit together");
    }
    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    /*
     * remove bottom-up ids
     */

    if ((TYisAKV (RET_TYPE (ret))) && (0 == TYgetDim (RET_TYPE (ret)))) {
        new_co = TYgetValue (RET_TYPE (ret));
    } else if ((TYisAKV (AVIS_TYPE (IDS_AVIS (arg_node))))
               && (0 == TYgetDim (AVIS_TYPE (IDS_AVIS (arg_node))))) {
        new_co = TYgetValue (AVIS_TYPE (IDS_AVIS (arg_node)));
    } else {
        /*
         * non akv-node found, expression has to remain in AST
         */

        INFO_SISI_REMOVEASSIGN (arg_info) = FALSE;
    }

    if ((TYisAKV (RET_TYPE (ret))) && (TYisAKV (AVIS_TYPE (IDS_AVIS (arg_node))))) {
        DBUG_ASSERT ((TYeqTypes (RET_TYPE (ret), AVIS_TYPE (IDS_AVIS (arg_node)))),
                     "Types are not equal!");
    }

    if (new_co != NULL) {
        assign_let = TCmakeAssignLet (IDS_AVIS (arg_node), COconstant2AST (new_co));
        /* append new copy assignment to assignment chain later */
        INFO_SISI_POSTASSIGN (arg_info)
          = TCappendAssign (INFO_SISI_POSTASSIGN (arg_info), assign_let);
        AVIS_SSAASSIGN (IDS_AVIS (arg_node)) = assign_let;

        /*
         * remove current ids from chain
         */
        succ = IDS_NEXT (arg_node);
        IDS_NEXT (arg_node) = NULL;
        IDS_AVIS (arg_node) = NULL;
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = succ;

        sisi_expr++;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIreturn(node *arg_node, info *arg_info)
 *
 * @brief traverses in exprs-chain
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIreturn");

    if ((!FUNDEF_ISLACFUN (INFO_SISI_FUNDEF (arg_info)))
        && (!ILIBstringCompare ("main", FUNDEF_NAME (INFO_SISI_FUNDEF (arg_info))))
        && (!ILIBstringCompare (MAIN_MOD_NAME, FUNDEF_MOD (INFO_SISI_FUNDEF (arg_info))))
        && (!FUNDEF_ISEXPORTED (INFO_SISI_FUNDEF (arg_info)))
        && (!FUNDEF_ISPROVIDED (INFO_SISI_FUNDEF (arg_info)))) {

        INFO_SISI_RETURNEXPRS (arg_info) = TRUE;

        if (RETURN_EXPRS (arg_node) != NULL) {
            RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
        }
        INFO_SISI_RETURNEXPRS (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIret(node *arg_node, info *arg_info)
 *
 * @brief remove scalar akv-rets
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIret");

    if (RET_NEXT (arg_node) != NULL) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }
    /*
     * remove bottom-up rets
     */

    if ((TYisAKV (RET_TYPE (arg_node))) && (0 == TYgetDim (RET_TYPE (arg_node)))) {
        node *tmp;
        tmp = RET_NEXT (arg_node);
        RET_NEXT (arg_node) = NULL;
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = tmp;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIexprs(node *arg_node, info *arg_info)
 *
 * @brief top-down: mark exprs to be removed,
 *        bottom-up: remove marked exprs
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIexprs (node *arg_node, info *arg_info)
{
    bool old_removeexpr;

    DBUG_ENTER ("SISIexprs");

    if (INFO_SISI_RETURNEXPRS (arg_info) == TRUE) {

        old_removeexpr = INFO_SISI_REMOVEEXPRS (arg_info);
        INFO_SISI_REMOVEEXPRS (arg_info) = FALSE;

        if (EXPRS_NEXT (arg_node) != NULL) {
            EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
        }
        /*
         * remove bottom-up exprs
         */

        if (INFO_SISI_REMOVEEXPRS (arg_info)) {
            node *tmp;
            tmp = EXPRS_NEXT (arg_node);
            EXPRS_EXPR (arg_node) = NULL;
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = tmp;
        }
        INFO_SISI_REMOVEEXPRS (arg_info) = old_removeexpr;
    }
    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIid(node *arg_node, info *arg_info)
 *
 * @brief sets flag if id is scalar akv-type
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
SISIid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIid");

    if ((TYisAKV (AVIS_TYPE (ID_AVIS (arg_node))))
        && (0 == TYgetDim (AVIS_TYPE (ID_AVIS (arg_node))))) {
        INFO_SISI_REMOVEEXPRS (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}
