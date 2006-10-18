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

/**
 * This phase eliminates all existing bottom types.
 * To achieve that, it does:
 * - transform functions that return a bottom type into bottom-functions
 * - eliminate bottom variables by
 *   a) eliminating the respective declaration
 *   b) replacing the definition of this variable by a type_error
 *      which is assigned to the (remaining if any) non-bottom lhs
 *      variables.
 * Here an example......
 *
 * There is one exception to this scheme though which pertains to funconds.
 * As these are non-strict, we allow one of them to be bottom.
 * As a consequence, we do need a proper definition of that variable. To achieve
 * that, we transform the rhs of that definition into a type error and then
 * change its type from bottom to the corresponding non-bottom type of the
 * other branch of the conditional.
 * For example......
 *
 */

/*
 * usages of 'arg_info':
 *
 *   INFO_THENBOTTS   - at least one bottom in then branch found
 *   INFO_ELSEBOTTS   - at least one bottom in else branch found
 *   INFO_ONEFUNCTION - traverse one function only (and associated lacfuns)
 *   INFO_FUNDEF      - pointer to current fundef to prevent infinite recursion
 *   INFO_TYPEERROR   - indicates that the rhs of a let is to be transformed
 *                      into this type error
 */

/**
 * INFO structure
 */
struct INFO {
    bool then_botts;
    bool else_botts;
    bool onefunction;
    node *fundef;
    node *type_error;
};

/**
 * INFO macros
 */
#define INFO_THENBOTTS(n) ((n)->then_botts)
#define INFO_ELSEBOTTS(n) ((n)->else_botts)
#define INFO_ONEFUNCTION(n) ((n)->onefunction)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_TYPEERROR(n) ((n)->type_error)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_THENBOTTS (result) = FALSE;
    INFO_ELSEBOTTS (result) = FALSE;
    INFO_ONEFUNCTION (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_TYPEERROR (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
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

            if (FUNDEF_ISDOFUN (arg_node) && INFO_THENBOTTS (arg_info)) {
                FUNDEF_ISDOFUN (arg_node) = FALSE;
                FUNDEF_ISLACINLINE (arg_node) = TRUE;
            }
        }

        INFO_THENBOTTS (arg_info) = FALSE;
        INFO_ELSEBOTTS (arg_info) = FALSE;
    }

    if ((!INFO_ONEFUNCTION (arg_info)) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTblock( node *arg_node, info *arg_info)
 *
 * @brief ensure that the BODY is traversed BEFORE the vardecs!
 * @return original node
 *
 *****************************************************************************/

node *
EBTblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EBTblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
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
     * First we go down in order to collect those bottom funcond vars that need to
     * be created by the prf 'type_error'.
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /**
     * Now, we can go into the instruction. In case of
     * - N_cond we place the type_errors generated by the funconds into the
     *   corresponding blocks!
     */
    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTlet( node *arg_node, info *arg_info)
 *
 *   @brief if args contain bottom: convert rhs into type-error
 *          else if lhs vars contain bottom: convert rhs into type-error.
 *          Anyways do filter-out bottom lhs vars.
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
EBTlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EBTlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    if (INFO_TYPEERROR (arg_info) != NULL) {
        LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
        LET_EXPR (arg_node) = INFO_TYPEERROR (arg_info);
        INFO_TYPEERROR (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTids( node *arg_node, info *arg_info)
 *
 *   @brief filter-out lhs vars. iff INFO_TYPEERROR is NULL and lhs contains
 *          bottom vars, create type-error!
 *          iff AVIS_BOTRT is set, replace the bottom type with the specified
 *          one.
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
EBTids (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("EBTids");

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    avis = IDS_AVIS (arg_node);

    if (TYisBottom (AVIS_TYPE (avis))) {
        if (INFO_TYPEERROR (arg_info) == NULL) {
            DBUG_PRINT ("EBT",
                        ("creating type error due to bottom LHS %s", AVIS_NAME (avis)));
            INFO_TYPEERROR (arg_info)
              = TCmakePrf1 (F_type_error, TBmakeType (TYcopyType (AVIS_TYPE (avis))));
        }

        if (AVIS_BOTRT (avis) != NULL) {
            DBUG_PRINT ("EBT", ("lifting bottom LHS %s", AVIS_NAME (avis)));
            AVIS_TYPE (avis) = TYfreeType (AVIS_TYPE (avis));
            AVIS_TYPE (avis) = AVIS_BOTRT (avis);
            AVIS_BOTRT (avis) = NULL;

            /**
             * this seems to be saa-bind related stuff??!
             */
            if (AVIS_DIM (avis) != NULL) {
                AVIS_DIM (avis) = FREEdoFreeNode (AVIS_DIM (avis));
            }
            if (AVIS_SHAPE (avis) != NULL) {
                AVIS_SHAPE (avis) = FREEdoFreeNode (AVIS_SHAPE (avis));
            }

        } else {
            DBUG_PRINT ("EBT", ("eliminating bottom LHS %s", AVIS_NAME (avis)));
            arg_node = FREEdoFreeNode (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EBTap( node *arg_node, info *arg_info)
 *
 * @brief create a type error iff args contain bottom;
 *        delete LaC fun iff args contain bottom, traverse into it otherwise.
 * @return original node
 *
 *****************************************************************************/
node *
EBTap (node *arg_node, info *arg_info)
{
    ntype *argt, *bottom;

    DBUG_ENTER ("EBTap");

    arg_node = TRAVcont (arg_node, arg_info);

    argt = TUactualArgs2Ntype (AP_ARGS (arg_node));
    bottom = TYgetBottom (argt);

    if (bottom != NULL) {
        if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
            && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
            DBUG_PRINT ("EBT",
                        ("lacfun %s found...", CTIitemName (AP_FUNDEF (arg_node))));
            DBUG_PRINT ("EBT", ("deleting %s", CTIitemName (AP_FUNDEF (arg_node))));
            AP_FUNDEF (arg_node) = FREEdoFreeNode (AP_FUNDEF (arg_node));
        }
        INFO_TYPEERROR (arg_info)
          = TCmakePrf1 (F_type_error, TBmakeType (TYcopyType (bottom)));

    } else {
        if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
            && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
            DBUG_PRINT ("EBT",
                        ("lacfun %s found...", CTIitemName (AP_FUNDEF (arg_node))));
            info *new_info = MakeInfo ();
            INFO_ONEFUNCTION (new_info) = TRUE;
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_info);
            new_info = FreeInfo (new_info);
        }
    }

    argt = TYfreeType (argt);

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

    if (INFO_THENBOTTS (arg_info) && INFO_ELSEBOTTS (arg_info)) {
        /**
         * There are errors in both branches, i.e., abort
         */
        CTIabortLine (global.linenum, "Conditional with type errors in both branches");
    }
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

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
        AVIS_BOTRT (ID_AVIS (FUNCOND_THEN (arg_node))) = TYeliminateAKV (etype);
        INFO_THENBOTTS (arg_info) = TRUE;
    }
    if (TYisBottom (etype)) {
        DBUG_ASSERT (!TYisBottom (ttype), "two bottom args for funcond found");
        AVIS_BOTRT (ID_AVIS (FUNCOND_ELSE (arg_node))) = TYeliminateAKV (ttype);
        INFO_ELSEBOTTS (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}
