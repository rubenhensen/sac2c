/*
 *
 * $Log$
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
};

#define INFO_SISI_FUNDEF(n) (n->fundef)
#define INFO_SISI_RETURNEXPRS(n) (n->retex)
#define INFO_SISI_REMOVEEXPRS(n) (n->remex)
#define INFO_SISI_ISAPNODE(n) (n->isapnode)
#define INFO_SISI_POSTASSIGN(n) (n->postassign)

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

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

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

node *
SISImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISImodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SISIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIfundef");

    INFO_SISI_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if ((FUNDEF_RETS (arg_node) != NULL) && (!FUNDEF_ISLACFUN (arg_node))) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }

    if (FUNDEF_RETS (arg_node) == NULL) {
        /*
         * TODO: create N_empty?
         */
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_ARGS (arg_node) != NULL)
        && (!FUNDEF_ISLACFUN (arg_node))) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * remove scalar akv from args-chain, insert them in assignment chain
 * change array akv to array aks
 * we are not in a lac function
 */
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
    }
    DBUG_RETURN (arg_node);
}

/*
 * set reference from arg_info to arg_node (used in args-chain)
 */
node *
SISIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIarg");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * top-down: evaluate INSTR
 */
node *
SISIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_SISI_POSTASSIGN (arg_info) = NULL;

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

    DBUG_RETURN (arg_node);
}

node *
SISIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIlet");

    INFO_SISI_ISAPNODE (arg_info) = FALSE;

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    if ((INFO_SISI_ISAPNODE (arg_info)) && (LET_IDS (arg_node) != NULL)) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SISIap (node *arg_node, info *arg_info)
{
    node *fundef, *fun_args, *curr_args, *new_args, *tmp;
    DBUG_ENTER ("SISIap");

    fundef = AP_FUNDEF (arg_node);

    if (!FUNDEF_ISLACFUN (fundef)) {

        fun_args = FUNDEF_ARGS (fundef);
        curr_args = AP_ARGS (arg_node);
        new_args = NULL;
        AP_ARGS (arg_node) = NULL;

        while (fun_args != NULL) {

            if (!TYisAKV (AVIS_TYPE (ARG_AVIS (fun_args)))) {
                if (NULL == new_args) {
                    AP_ARGS (arg_node) = curr_args;
                    new_args = curr_args;
                } else {
                    EXPRS_NEXT (new_args) = curr_args;
                }
                curr_args = EXPRS_NEXT (curr_args);
            } else {

                /*
                 * argument is of type akv
                 */
                tmp = curr_args;
                curr_args = EXPRS_NEXT (curr_args);
                EXPRS_NEXT (tmp) = NULL;
                tmp = FREEdoFreeNode (tmp);
            }
            fun_args = ARG_NEXT (fun_args);
        }

        INFO_SISI_ISAPNODE (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/*
 * remove ids with akv-types from chain
 */
node *
SISIids (node *arg_node, info *arg_info)
{
    node *assign_let, *succ;
    constant *new_co;

    DBUG_ENTER ("SISIids");

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    /*
     * remove bottom-up ids
     */
    if (TYisAKV (AVIS_TYPE (IDS_AVIS (arg_node)))) {
        new_co = TYgetValue (AVIS_TYPE (IDS_AVIS (arg_node)));

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
    }

    DBUG_RETURN (arg_node);
}

/*
 * remove exprs with akv-types from chain
 */
node *
SISIreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIreturn");

    if (!FUNDEF_ISLACFUN (INFO_SISI_FUNDEF (arg_info))) {

        INFO_SISI_RETURNEXPRS (arg_info) = TRUE;

        if (RETURN_EXPRS (arg_node) != NULL) {
            RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
        }
        INFO_SISI_RETURNEXPRS (arg_info) = FALSE;

        if (RETURN_EXPRS (arg_node) == NULL) {
            /*
             * TODO: create void node?
             */
        }
    }

    DBUG_RETURN (arg_node);
}

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

    if (TYisAKV (RET_TYPE (arg_node))) {
        node *tmp;
        tmp = RET_NEXT (arg_node);
        RET_NEXT (arg_node) = NULL;
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = tmp;
    }

    DBUG_RETURN (arg_node);
}

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

node *
SISIid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SISIid");

    if (TYisAKV (AVIS_TYPE (ID_AVIS (arg_node)))) {
        INFO_SISI_REMOVEEXPRS (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}
