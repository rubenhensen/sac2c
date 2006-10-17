/*
 *
 * $Id$
 *
 */

#include <stdio.h>
#include <string.h>
#include "elim_bottom_types.h"

#include "dbug.h"
#include "ctinfo.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "shape.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse_helper.h"

#include "new_typecheck.h"
#include "new_types.h"
#include "type_utils.h"
#include "type_errors.h"
#include "ssi.h"
#include "sig_deps.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

/*
 * This phase does NOT ONLY compute old types from the new ones but
 * IT ALSO fixes the ntypes, i.e., it eliminates type vars!!!!!!
 */

/*
 * usages of 'arg_info':
 *
 *   INFO_THENBOTTS   -   type_error let
 *   INFO_ELSEBOTTS   -   type_error let
 *   INFO_ONEFUNCTION - traverse one function only (and associated lacfuns)
 *   INFO_FUNDEF      - pointer to current fundef to prevent infinite recursion
 *   INFO_DELLACFUN   -   indicates that a call to a lacfun has been deleted
 */

/**
 * INFO structure
 */
struct INFO {
    node *then_botts;
    node *else_botts;
    bool onefunction;
    node *fundef;
    bool dellacfun;
};

/**
 * INFO macros
 */
#define INFO_THENBOTTS(n) ((n)->then_botts)
#define INFO_ELSEBOTTS(n) ((n)->else_botts)
#define INFO_ONEFUNCTION(n) ((n)->onefunction)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_DELLACFUN(n) ((n)->dellacfun)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_THENBOTTS (result) = NULL;
    INFO_ELSEBOTTS (result) = NULL;
    INFO_ONEFUNCTION (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_DELLACFUN (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

static bool
IdsContainBottom (node *ids)
{
    bool res = FALSE;

    DBUG_ENTER ("IdsContainBottom");

    while (ids != NULL) {
        res = res || TYisBottom (AVIS_TYPE (IDS_AVIS (ids)));
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (res);
}

static bool
IdsInChain (node *ids, node *chain)
{
    bool result = FALSE;

    DBUG_ENTER ("IdsInChain");

    while (!result && (chain != NULL)) {
        result = IDS_AVIS (ids) == IDS_AVIS (chain);
        chain = IDS_NEXT (chain);
    }

    DBUG_RETURN (result);
}

static node *
AddIdsToTypeError (node *ids, node *error)
{
    node *result = NULL;
    node *errorids;

    DBUG_ENTER ("AddIdsToTypeError");

    errorids = LET_IDS (ASSIGN_INSTR (error));

    while (ids != NULL) {
        if (TYisBottom (AVIS_TYPE (IDS_AVIS (ids))) || IdsInChain (ids, errorids)) {
            /*
             * it is already in there or will be added
             * later, so nothing to do
             */
            node *tmp = ids;
            ids = IDS_NEXT (ids);
            IDS_NEXT (tmp) = result;
            result = tmp;
        } else {
            /*
             * add it now.
             */
            node *tmp = ids;
            ids = IDS_NEXT (ids);
            IDS_NEXT (tmp) = errorids;
            errorids = tmp;

            AVIS_SSAASSIGN (IDS_AVIS (errorids)) = error;
            DBUG_PRINT ("EBT", ("adding ids '%s' to type error...", IDS_NAME (errorids)));
        }
    }

    LET_IDS (ASSIGN_INSTR (error)) = errorids;

    DBUG_RETURN (result);
}

static node *
AddTypeError (node *assign, node *bottom_id, ntype *other_type)
{
    node *ids;
    DBUG_ENTER ("AddTypeError");

    if (assign == NULL) {
        /**
         * No errors yet : create an assignment of the form
         *
         *   bottom_id = type_error( bottom_type);
         *
         */
        assign = TBmakeAssign (TBmakeLet (TBmakeIds (ID_AVIS (bottom_id), NULL),
                                          TCmakePrf1 (F_type_error,
                                                      TBmakeType (TYcopyType (AVIS_TYPE (
                                                        ID_AVIS (bottom_id)))))),
                               NULL);

    } else {
        /**
         * We have seen other errors before, add a new LHS
         */
        ids = LET_IDS (ASSIGN_INSTR (assign));
        ids = TBmakeIds (ID_AVIS (bottom_id), ids);
        LET_IDS (ASSIGN_INSTR (assign)) = ids;
    }
    /**
     * Finally, we change the type of bottom_id to other_type.
     */
    AVIS_TYPE (ID_AVIS (bottom_id)) = TYfreeType (AVIS_TYPE (ID_AVIS (bottom_id)));

    AVIS_TYPE (ID_AVIS (bottom_id)) = TYeliminateAKV (other_type);

    if (AVIS_DIM (ID_AVIS (bottom_id)) != NULL) {
        AVIS_DIM (ID_AVIS (bottom_id)) = FREEdoFreeNode (AVIS_DIM (ID_AVIS (bottom_id)));
    }
    if (AVIS_SHAPE (ID_AVIS (bottom_id)) != NULL) {
        AVIS_SHAPE (ID_AVIS (bottom_id))
          = FREEdoFreeNode (AVIS_SHAPE (ID_AVIS (bottom_id)));
    }

    /**
     * and we eliminate the defining N_let if it has not been
     * eliminated already while processing one of its further
     * results:
     */
    DBUG_ASSERT ((AVIS_SSAASSIGN (ID_AVIS (bottom_id)) != NULL),
                 "missing AVIS_SSAASSIGN!");
    DBUG_ASSERT ((NODE_TYPE (AVIS_SSAASSIGN (ID_AVIS (bottom_id))) == N_assign),
                 "AVIS_SSAASSIGN points to non N_assign node!");
    if (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (bottom_id))) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (bottom_id))))
                      == N_let),
                     "AVIS_SSAASSIGN does not point to an N_let assignment!");

        /*
         * as we will delete the entire let, we have to ensure that all
         * lhs ids are bound to a type error. this may not be the case if
         * only one result of the rhs function call is a bottom and the
         * others are not. If in fact they are bottom, we do not add them
         * as they would be added twice otherwise!
         */
        LET_IDS (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (bottom_id))))
          = AddIdsToTypeError (LET_IDS (
                                 ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (bottom_id)))),
                               assign);

        ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (bottom_id)))
          = FREEdoFreeTree (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (bottom_id))));
    }

    /**
     * finally set the SSAASSIGN to the type_error
     */
    AVIS_SSAASSIGN (ID_AVIS (bottom_id)) = assign;

    DBUG_RETURN (assign);
}

static node *
TransformIntoTypeError (node *fundef)
{
    DBUG_ENTER ("TransformIntoTypeError");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "cannot transform non fundef node");

    DBUG_ASSERT (TUretsContainBottom (FUNDEF_RETS (fundef)),
                 "cannot transform a fundef without bottom return types!");

    if (FUNDEF_BODY (fundef) != NULL) {
        FUNDEF_BODY (fundef) = FREEdoFreeNode (FUNDEF_BODY (fundef));
    }

    /*
     * we mark the function as a special type error function. this
     * is done to ease the detection of these functions. otherwise
     * one would have to traverse through all the return types to
     * check, which would be a noticeable performance drawback.
     */
    FUNDEF_ISTYPEERROR (fundef) = TRUE;

    /*
     * furthermore we set the function to non-inline. as it has no
     * body, it cannot be inlined anyways but better make sure.
     */
    FUNDEF_ISINLINE (fundef) = FALSE;

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *EBTdoEliminateBottomTypes( node *arg_node)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EBTdoEliminateBottomTypes (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("EBTdoEliminateBottomTypes");

    TRAVpush (TR_ebt);

    info_node = MakeInfo ();
    arg_node = TRAVdo (arg_node, info_node);
    info_node = FreeInfo (info_node);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EBTdoEliminateBottomTypesOneFunction( node *arg_node)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EBTdoEliminateBottomTypesOneFunction (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("EBTdoEliminateBottomTypesOneFunction");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "EBTdoEliminateBottomTypesOneFunction can only be applied to fundefs");

    if (!FUNDEF_ISLACFUN (arg_node)) {
        TRAVpush (TR_ebt);

        info_node = MakeInfo ();
        INFO_ONEFUNCTION (info_node) = TRUE;
        arg_node = TRAVdo (arg_node, info_node);
        info_node = FreeInfo (info_node);

        TRAVpop ();
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EBTmodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EBTmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EBTmodule");

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EBTfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EBTfundef (node *arg_node, info *arg_info)
{
    ntype *ftype, *bottom;

    DBUG_ENTER ("EBTfundef");

    if (!FUNDEF_ISLACFUN (arg_node) || INFO_ONEFUNCTION (arg_info)) {
        INFO_FUNDEF (arg_info) = arg_node;

        DBUG_PRINT ("EBT", ("----> Processing function %s\n", CTIitemName (arg_node)));

        ftype = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));

        bottom = TYgetBottom (ftype);
        if (bottom != NULL) {
            DBUG_PRINT ("EBT", ("bottom component found in function %s",
                                CTIitemName (arg_node)));
            if (FUNDEF_ISPROVIDED (arg_node) || FUNDEF_ISEXPORTED (arg_node)) {
                CTIabortOnBottom (TYgetBottomError (bottom));
            } else {
                /**
                 * we transform the entire body into one 'type error' assignment
                 *
                 * In case of LaC funs this is more difficult. Since the signatures
                 * of these funs are not user specified, they do not have an upper
                 * limit which prevents from a successfull type error generation.
                 * (cf. bug no 115).However, since this bottom has been propagated into
                 * the calling site, we can simply eliminate these functions entirely!
                 */
                arg_node = TransformIntoTypeError (arg_node);
            }

        } else {
            DBUG_ASSERT (TYisProdOfArray (ftype), "inconsistent return type found");
            DBUG_PRINT ("EBT", ("ProdOfArray return type found for function %s",
                                CTIitemName (arg_node)));

            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
            }

            if (FUNDEF_BODY (arg_node) != NULL) {
                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            }

            if (FUNDEF_ISDOFUN (arg_node) && INFO_THENBOTTS (arg_info) != NULL) {
                FUNDEF_ISDOFUN (arg_node) = FALSE;
                FUNDEF_ISLACINLINE (arg_node) = TRUE;
            }
        }

        INFO_THENBOTTS (arg_info) = NULL;
        INFO_ELSEBOTTS (arg_info) = NULL;
    }

    if ((!INFO_ONEFUNCTION (arg_info)) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EBTap (node *arg_node, info *arg_info)
{
    ntype *argt, *bottom;

    DBUG_ENTER ("EBTap");

    arg_node = TRAVcont (arg_node, arg_info);

    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("EBT", ("lacfun %s found...", CTIitemName (AP_FUNDEF (arg_node))));
        /**
         * First, we check whether we are dealing with an application
         * that contains a bottom type. If so, we delete the lacfun!
         */
        argt = TUactualArgs2Ntype (AP_ARGS (arg_node));
        bottom = TYgetBottom (argt);
        if (bottom != NULL) {
            DBUG_PRINT ("EBT", ("deleting %s", CTIitemName (AP_FUNDEF (arg_node))));
            INFO_DELLACFUN (arg_info) = TRUE;
            AP_FUNDEF (arg_node) = FREEdoFreeNode (AP_FUNDEF (arg_node));
        } else {
            info *new_info = MakeInfo ();
            INFO_ONEFUNCTION (new_info) = TRUE;
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_info);
            new_info = FreeInfo (new_info);
        }
        argt = TYfreeType (argt);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EBTvardec( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EBTvardec (node *arg_node, info *arg_info)
{
    node *this_vardec;

    DBUG_ENTER ("EBTvardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    if (TYisBottom (VARDEC_NTYPE (arg_node))) {
        /**
         * eliminate this vardec completely!
         */
        DBUG_PRINT ("EBT", ("eliminating bottom vardec for %s", VARDEC_NAME (arg_node)));
        this_vardec = arg_node;
        arg_node = VARDEC_NEXT (arg_node);
        VARDEC_NEXT (this_vardec) = NULL;
        this_vardec = FREEdoFreeTree (this_vardec);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EBTlet( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EBTlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EBTlet");

    if (IdsContainBottom (LET_IDS (arg_node))) {

        DBUG_PRINT ("EBT", ("bottom LHS found; eliminating N_let \"%s...\"",
                            IDS_NAME (LET_IDS (arg_node))));
        arg_node = FREEdoFreeTree (arg_node);
    } else {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        if (INFO_DELLACFUN (arg_info)) {
            /*
             * the LaC function has been deleted as it contains bottoms.
             * Thus this let is needed no more. Flag this to the outer
             * assign by deleting the let as well.
             */
            INFO_DELLACFUN (arg_info) = FALSE;
            arg_node = FREEdoFreeNode (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTassign( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
EBTassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EBTassign");

    /**
     * First we go down in order to collect those funcond vars that need to be
     * created by the prf 'type_error'.
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /**
     * Now, we can go into the instruction. In case of N_lets that have bottom
     * vars on their LHS, we may obtain NULL, which signals us to eliminate
     * this very N_assign.
     */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_INSTR (arg_node) == NULL) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTcond( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
EBTcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EBTcond");

    if ((INFO_THENBOTTS (arg_info) != NULL) && (INFO_ELSEBOTTS (arg_info) != NULL)) {
        /**
         * There are errors in both branches, i.e., abort
         */
        CTIabortLine (global.linenum, "Conditional with type errors in both branches");
    }
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    if (INFO_THENBOTTS (arg_info) != NULL) {
        INFO_THENBOTTS (arg_info) = TCappendAssign (INFO_THENBOTTS (arg_info),
                                                    BLOCK_INSTR (COND_THEN (arg_node)));

        BLOCK_INSTR (COND_THEN (arg_node)) = INFO_THENBOTTS (arg_info);
    }
    if (INFO_ELSEBOTTS (arg_info) != NULL) {
        INFO_ELSEBOTTS (arg_info) = TCappendAssign (INFO_ELSEBOTTS (arg_info),
                                                    BLOCK_INSTR (COND_ELSE (arg_node)));

        BLOCK_INSTR (COND_ELSE (arg_node)) = INFO_ELSEBOTTS (arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTfuncond( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
EBTfuncond (node *arg_node, info *arg_info)
{
    ntype *ttype, *etype;

    DBUG_ENTER ("EBTfuncond");

    ttype = AVIS_TYPE (ID_AVIS (FUNCOND_THEN (arg_node)));
    etype = AVIS_TYPE (ID_AVIS (FUNCOND_ELSE (arg_node)));

    if (TYisBottom (ttype)) {
        DBUG_ASSERT (!TYisBottom (etype), "two bottom args for funcond found");
        INFO_THENBOTTS (arg_info)
          = AddTypeError (INFO_THENBOTTS (arg_info), FUNCOND_THEN (arg_node), etype);
    }
    if (TYisBottom (etype)) {
        DBUG_ASSERT (!TYisBottom (ttype), "two bottom args for funcond found");
        INFO_ELSEBOTTS (arg_info)
          = AddTypeError (INFO_ELSEBOTTS (arg_info), FUNCOND_ELSE (arg_node), ttype);
    }

    DBUG_RETURN (arg_node);
}
