/*
 *
 * $Log$
 * Revision 1.12  2005/09/04 12:52:11  ktr
 * re-engineered the optimization cycle
 *
 * Revision 1.11  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 1.10  2005/06/02 13:42:48  mwe
 * set WASOPTIMIZED flag if some constant return values were found
 *
 * Revision 1.9  2005/06/01 11:27:04  mwe
 * some comments added
 *
 * Revision 1.8  2005/05/31 13:59:34  mwe
 * usage of ARG_ISINUSE instaed of NEEDCOUNT
 * some methods new implemented
 *
 * Revision 1.7  2005/05/31 12:28:06  mwe
 * bug fixed
 *
 * Revision 1.6  2005/02/18 22:19:02  mwe
 * bug fixed
 *
 * Revision 1.5  2005/02/11 12:10:42  mwe
 * now only arguments marked by deadcoderemoval are deleted
 *
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

/**<!--******************************************************************-->
 *
 * @file signature_simplification.c
 *
 * This module based optimization removes all unused input arguments of
 * non-exported, non-provided and non-lac functions of a module.
 * Also all constant and scalar return types of that functions are removed
 * from the function signature and inserted directly in the calling context.
 *
 * First while traversal through the function body all unused function
 * arguments are removed from the signature of the calling contexts and
 * constant scalar return values are inserted in the calling context.
 * Finally constant scalar return values are removed from the
 * return-exprs-chain.
 * After modifying all function bodies, all function signatures (args and rets)
 * are modified.
 *
 * Unused function arguments are marked by (ARG_ISINUSE == FALSE), which
 * is set in SSADeadCodeRemoval.
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "new_types.h"
#include "dbug.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "globals.h"
#include "constants.h"
#include "shape.h"
#include "namespaces.h"
#include "tree_compound.h"
#include "inferneedcounters.h"

#include "signature_simplification.h"

typedef enum { infer, simplify } travphases;

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool remex;
    bool retex;
    bool isapnode;
    node *rets;
    node *apfunrets;
    bool remassign;
    bool idslet;
    node *postassign;
    travphases travphase;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_RETURNEXPRS(n) (n->retex)
#define INFO_REMOVEEXPRS(n) (n->remex)
#define INFO_ISAPNODE(n) (n->isapnode)
#define INFO_RETS(n) (n->rets)
#define INFO_APFUNRETS(n) (n->apfunrets)
#define INFO_IDSLET(n) (n->idslet)
#define INFO_POSTASSIGN(n) (n->postassign)

#define INFO_TRAVPHASE(n) (n->travphase)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");
    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_RETURNEXPRS (result) = FALSE;
    INFO_REMOVEEXPRS (result) = FALSE;
    INFO_ISAPNODE (result) = FALSE;
    INFO_RETS (result) = NULL;
    INFO_APFUNRETS (result) = NULL;
    INFO_IDSLET (result) = FALSE;
    INFO_POSTASSIGN (result) = NULL;

    INFO_TRAVPHASE (result) = infer;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

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

    INFO_TRAVPHASE (arg_info) = infer;

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    INFO_TRAVPHASE (arg_info) = simplify;

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

    if (INFO_TRAVPHASE (arg_info) == infer) {

        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node);

    } else if (INFO_TRAVPHASE (arg_info) == simplify) {
        INFO_FUNDEF (arg_info) = arg_node;

        INFO_RETS (arg_info) = FUNDEF_RETS (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }

        INFO_FUNDEF (arg_info) = arg_node;

        if ((FUNDEF_RETS (arg_node) != NULL) && (!FUNDEF_ISLACFUN (arg_node))
            && (!STReq ("main", FUNDEF_NAME (arg_node)))
            && (!NSequals (NSgetRootNamespace (), FUNDEF_NS (arg_node)))
            && (!FUNDEF_ISEXPORTED (arg_node)) && (!FUNDEF_ISPROVIDED (arg_node))) {
            FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
        }

        if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_ARGS (arg_node) != NULL)
            && (!FUNDEF_ISLACFUN (arg_node)) && (!FUNDEF_ISEXPORTED (arg_node))
            && (!FUNDEF_ISPROVIDED (arg_node))) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
    } else {
        DBUG_ASSERT ((FALSE), "Unexpected traversal phase!");
    }
    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIarg(node *arg_node, info *arg_info)
 *
 * @brief Delete unused arguments from args-chain.
 *
 *    Do not change signatures of exported or provided functions. Leave also
 *    LAC-functions as they are (ensured in SISIfundef).
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

    if (AVIS_NEEDCOUNT (ARG_AVIS (arg_node)) == 0) {

        node *tmp;

        /*
         * we are dealing with an unused argument
         * delete argument from chain
         */
        tmp = ARG_NEXT (arg_node);
        ARG_NEXT (arg_node) = NULL;
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = tmp;

        global.optcounters.sisi_expr++;
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
 * @brief traverse in assignments, insert new assignments if necessary
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

    INFO_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }
    /* integrate post assignments after current assignment */
    ASSIGN_NEXT (arg_node)
      = TCappendAssign (INFO_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
    INFO_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
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

    INFO_ISAPNODE (arg_info) = FALSE;

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    if ((INFO_ISAPNODE (arg_info)) && (LET_IDS (arg_node) != NULL)
        && (INFO_APFUNRETS (arg_info) != NULL)) {

        INFO_IDSLET (arg_info) = TRUE;
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
        INFO_IDSLET (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIap(node *arg_node, info *arg_info)
 *
 * @brief check fundef-args for unused arguments,
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
        && (!FUNDEF_ISEXPORTED (fundef)) && (!FUNDEF_ISEXTERN (fundef))) {

        INFO_APFUNRETS (arg_info) = FUNDEF_RETS (AP_FUNDEF (arg_node));
        fun_args = FUNDEF_ARGS (fundef);
        curr_args = AP_ARGS (arg_node);
        new_args = NULL;
        AP_ARGS (arg_node) = NULL;

        while (fun_args != NULL) {

            if (AVIS_NEEDCOUNT (ARG_AVIS (fun_args)) > 0) {
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
                 * argument is marked as not needed
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

        INFO_ISAPNODE (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIids(node *arg_node, info *arg_info)
 *
 * @brief removes ids-nodes with corresponding scalar akv-types in ret-chain
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
    node *succ, *ret, *assign_let;
    constant *new_co;
    DBUG_ENTER ("SISIids");

    if (INFO_IDSLET (arg_info)) {

        ret = INFO_APFUNRETS (arg_info);
        if (RET_NEXT (INFO_APFUNRETS (arg_info)) != NULL) {
            INFO_APFUNRETS (arg_info) = RET_NEXT (INFO_APFUNRETS (arg_info));
        } else {
            DBUG_ASSERT ((IDS_NEXT (arg_node) == NULL),
                         "ret and ids do not fit together");
        }

        if (IDS_NEXT (arg_node) != NULL) {

            IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
        }

        /*
         * bottom-up traversal
         */

        if ((TYisAKV (RET_TYPE (ret))) && (0 == TYgetDim (RET_TYPE (ret)))) {
            new_co = TYgetValue (RET_TYPE (ret));

            DBUG_PRINT ("SISI", ("identifier %s marked as constant",
                                 VARDEC_OR_ARG_NAME (AVIS_DECL (IDS_AVIS (arg_node)))));

            /*
             * create one let assign for constant definition,
             * reuse old avis/vardec
             */
            assign_let = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (arg_node), NULL),
                                                  COconstant2AST (new_co)),
                                       NULL);
            AVIS_SSAASSIGN (IDS_AVIS (arg_node)) = assign_let;

            /*
             * append new copy assignment to then-part block
             */
            INFO_POSTASSIGN (arg_info)
              = TCappendAssign (INFO_POSTASSIGN (arg_info), assign_let);

            DBUG_PRINT ("SISI",
                        ("create constant assignment for %s", (IDS_NAME (arg_node))));

            /*
             * Remove current ids
             */
            succ = IDS_NEXT (arg_node);
            IDS_NEXT (arg_node) = NULL;
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = succ;

            global.optcounters.sisi_expr++;
            FUNDEF_WASOPTIMIZED (INFO_FUNDEF (arg_info)) = TRUE;
        }
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

    if ((!FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info)))
        && (!STReq ("main", FUNDEF_NAME (INFO_FUNDEF (arg_info))))
        && (!NSequals (NSgetRootNamespace (), FUNDEF_NS (INFO_FUNDEF (arg_info))))
        && (!FUNDEF_ISEXPORTED (INFO_FUNDEF (arg_info)))
        && (!FUNDEF_ISPROVIDED (INFO_FUNDEF (arg_info)))) {

        INFO_RETURNEXPRS (arg_info) = TRUE;

        if (RETURN_EXPRS (arg_node) != NULL) {
            RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
        }
        INFO_RETURNEXPRS (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *SISIret(node *arg_node, info *arg_info)
 *
 * @brief remove marked (already removed) rets
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

    if (RET_WASREMOVED (arg_node)) {
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
 * @brief top-down: find and mark exprs to be removed,
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
    bool remove;
    node *ret;
    DBUG_ENTER ("SISIexprs");

    if (INFO_RETURNEXPRS (arg_info) == TRUE) {

        INFO_REMOVEEXPRS (arg_info) = FALSE;

        ret = INFO_RETS (arg_info);

        EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);

        remove = INFO_REMOVEEXPRS (arg_info);

        if (EXPRS_NEXT (arg_node) != NULL) {
            INFO_RETS (arg_info) = RET_NEXT (INFO_RETS (arg_info));
            EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
        }
        /*
         * remove bottom-up exprs
         */

        if (remove) {
            node *tmp;
            tmp = EXPRS_NEXT (arg_node);
            EXPRS_EXPR (arg_node) = NULL;
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = tmp;

            RET_WASREMOVED (ret) = TRUE;
        }
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

        INFO_REMOVEEXPRS (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}
