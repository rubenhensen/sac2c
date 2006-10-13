/*
 *
 * $Id$
 *
 */

#include <stdio.h>
#include <string.h>
#include "new2old.h"

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
 *   INFO_VARDECS    -   list of the generated vardecs
 *   INFO_WLIDS      -   WITHID_VEC of first partition
 *   INFO_THENBOTTS  -   type_error let
 *   INFO_ELSEBOTTS  -   type_error let
 *   INFO_LHS        -   left hand side of current let node
 *   INFO_ONEFUNCTION - traverse one function only (and associated lacfuns)
 *   INFO_FUNDEF     - pointer to current fundef to prevent infinite recursion
 *   INFO_VARDECMODE -   M_fix / M_filter
 *   INFO_DELLACFUN  -   indicates that a call to a lacfun has been deleted
 */

/**
 * INFO structure
 */
struct INFO {
    node *vardecs;
    node *wlids;
    node *then_botts;
    node *else_botts;
    node *lhs;
    bool onefunction;
    node *fundef;
    enum { M_fix, M_filter } mode;
    bool dellacfun;
};

/**
 * INFO macros
 */
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_WLIDS(n) ((n)->wlids)
#define INFO_THENBOTTS(n) ((n)->then_botts)
#define INFO_ELSEBOTTS(n) ((n)->else_botts)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_ONEFUNCTION(n) ((n)->onefunction)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECMODE(n) ((n)->mode)
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

    INFO_VARDECS (result) = NULL;
    INFO_WLIDS (result) = NULL;
    INFO_THENBOTTS (result) = NULL;
    INFO_ELSEBOTTS (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_ONEFUNCTION (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_VARDECMODE (result) = M_fix;
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
            DBUG_PRINT ("FIXNT",
                        ("adding ids '%s' to type error...", IDS_NAME (errorids)));
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
 *   node *NT2OTdoTransform( node *arg_node)
 *
 * description:
 *   adjusts all old vardec types according to the attached ntypes!
 *
 ******************************************************************************/

node *
NT2OTdoTransform (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("NT2OTdoTransform");

    TRAVpush (TR_nt2ot);

    info_node = MakeInfo ();
    arg_node = TRAVdo (arg_node, info_node);
    info_node = FreeInfo (info_node);

    TRAVpop ();

    /**
     * Since all alpha types are gone now, we may free all tvars, all
     *  sig_deps, and all te_infos:
     */
    SSIfreeAllTvars ();
    SDfreeAllSignatureDependencies ();
    TEfreeAllTypeErrorInfos ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTdoTransformOneFunction( node *arg_node)
 *
 * description:
 *   adjusts all old vardec types according to the attached ntypes!
 *
 ******************************************************************************/

node *
NT2OTdoTransformOneFunction (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("NT2OTdoTransformOneFunction");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "NT2OTdoTransformOneFunction can only be applied to fundefs");

    if (!FUNDEF_ISLACFUN (arg_node)) {
        TRAVpush (TR_nt2ot);

        info_node = MakeInfo ();
        INFO_ONEFUNCTION (info_node) = TRUE;
        arg_node = TRAVdo (arg_node, info_node);
        info_node = FreeInfo (info_node);

        TRAVpop ();

        /**
         * Since all alpha types are gone now, we may may free all tvars, all
         *  sig_deps, and all te_infos:
         */
        SSIfreeAllTvars ();
        SDfreeAllSignatureDependencies ();
        TEfreeAllTypeErrorInfos ();
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTmodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTmodule");

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
 *   node *NT2OTfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTfundef (node *arg_node, info *arg_info)
{
    ntype *otype, *ftype, *bottom;

    DBUG_ENTER ("NT2OTfundef");

    if (!FUNDEF_ISLACFUN (arg_node) || INFO_ONEFUNCTION (arg_info)) {
        INFO_FUNDEF (arg_info) = arg_node;

        DBUG_PRINT ("FIXNT", ("----> Processing function %s\n", CTIitemName (arg_node)));

        otype = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));
        DBUG_ASSERT ((otype != NULL), "FUNDEF_RET_TYPE not found!");
        ftype = TYfixAndEliminateAlpha (otype);
        FUNDEF_RETS (arg_node) = TUreplaceRetTypes (FUNDEF_RETS (arg_node), ftype);

        /* process the real function type as well */
        if (FUNDEF_WRAPPERTYPE (arg_node) != NULL) {
            ntype *funtype = TYfixAndEliminateAlpha (FUNDEF_WRAPPERTYPE (arg_node));
            FUNDEF_WRAPPERTYPE (arg_node) = TYfreeType (FUNDEF_WRAPPERTYPE (arg_node));
            FUNDEF_WRAPPERTYPE (arg_node) = funtype;
        }

        if (TYcountNoMinAlpha (ftype) > 0) {

            if (FUNDEF_ISPROVIDED (arg_node) || FUNDEF_ISEXPORTED (arg_node)) {
                CTIabortLine (NODE_LINE (arg_node),
                              "One component of inferred return type (%s) has no lower "
                              "bound;"
                              " an application of \"%s\" will not terminate",
                              TYtype2String (ftype, FALSE, 0), CTIitemName (arg_node));
            } else {
                DBUG_PRINT ("FIXNT",
                            ("eliminating function %s due to lacking result type",
                             CTIitemName (arg_node)));
                arg_node = FREEdoFreeNode (arg_node);
            }
        } else {
            bottom = TYgetBottom (ftype);
            if (bottom != NULL) {
                DBUG_PRINT ("FIXNT", ("bottom component found in function %s",
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
                     * (cf. bug no 115).However, since this bottom has been propagated
                     * into the calling site, we can simply eliminate these functions
                     * entirely!
                     */
                    if (FUNDEF_ISLACFUN (arg_node)) {
                        /* mark this fun as zombie! */
                        arg_node = FREEdoFreeNode (arg_node);

                    } else {
                        arg_node = TransformIntoTypeError (arg_node);
                    }
                }

            } else {
                DBUG_ASSERT (TYisProdOfArray (ftype), "inconsistent return type found");
                DBUG_PRINT ("FIXNT", ("ProdOfArray return type found for function %s",
                                      CTIitemName (arg_node)));

                if (FUNDEF_ARGS (arg_node) != NULL) {
                    FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
                }

                INFO_VARDECS (arg_info) = NULL;
                if (FUNDEF_BODY (arg_node) != NULL) {
                    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
                }

                if (INFO_VARDECS (arg_info) != NULL) {
                    /*
                     * if some new vardecs have been built, insert them !
                     * this has to be done here rather than in NT2OTblock since blocks
                     * in general may not be top level.
                     * AFAIK, the only place where vardecs are in fact built is the
                     * extension of withloop ids in NT2OTwithid.
                     */
                    INFO_VARDECS (arg_info) = TRAVdo (INFO_VARDECS (arg_info), arg_info);

                    FUNDEF_VARDEC (arg_node) = TCappendVardec (INFO_VARDECS (arg_info),
                                                               FUNDEF_VARDEC (arg_node));
                    INFO_VARDECS (arg_info) = NULL;
                }
                if (FUNDEF_ISDOFUN (arg_node) && INFO_THENBOTTS (arg_info) != NULL) {
                    FUNDEF_ISDOFUN (arg_node) = FALSE;
                    FUNDEF_ISLACINLINE (arg_node) = TRUE;
                }
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
 * @fn node *NT2OTap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
NT2OTap (node *arg_node, info *arg_info)
{
    ntype *argt, *bottom;

    DBUG_ENTER ("NT2OTap");

    arg_node = TRAVcont (arg_node, arg_info);

#if 0
  if ( INFO_ONEFUNCTION( arg_info) &&
       FUNDEF_ISLACFUN( AP_FUNDEF( arg_node)) &&
       ( AP_FUNDEF( arg_node) != INFO_FUNDEF( arg_info))) {
    info *new_info = MakeInfo();
    INFO_ONEFUNCTION( new_info) = TRUE;
    AP_FUNDEF( arg_node) = TRAVdo( AP_FUNDEF( arg_node), new_info);
    new_info = FreeInfo( new_info);
  }
#else
    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("FIXNT", ("lacfun %s found...", CTIitemName (AP_FUNDEF (arg_node))));
        /**
         * First, we check whether we are dealing with an application
         * that contains a bottom type. If so, we delete the lacfun!
         */
        argt = TUactualArgs2Ntype (AP_ARGS (arg_node));
        bottom = TYgetBottom (argt);
        if (bottom != NULL) {
            DBUG_PRINT ("FIXNT", ("deleting %s", CTIitemName (AP_FUNDEF (arg_node))));
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
#endif

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTavis( node *arg_node, info *arg_info)
 *
 * description:
 *   Fix (!) the ntype.
 *
 ******************************************************************************/

node *
NT2OTavis (node *arg_node, info *arg_info)
{
    ntype *type, *scalar;
#ifndef DBUG_OFF
    char *tmp_str, *tmp_str2;
#endif

    DBUG_ENTER ("NT2OTavis");

    type = AVIS_TYPE (arg_node);

    if (type != NULL) {

        DBUG_EXECUTE ("FIXNT", tmp_str = TYtype2String (type, FALSE, 0););
        DBUG_PRINT ("FIXNT", ("replacing argument/vardec %s\'s type %s by ...",
                              AVIS_NAME (arg_node), tmp_str));
        type = TYfixAndEliminateAlpha (type);
        /**
         * we try to avoid AKD(0) types and replace them by AKS([]) types
         * I'm not 100% sure whether this is the right thing to do here.
         * I think that there is a similar mechanism somewhere else in the
         * TC or in new_types.c but I cannot recall where...:-(
         * However, print( reshape([], 3)) would lead to pointer problems
         * when linking due to an AKD specialization rather than an AKS one.
         */
        if (TYisAKD (type) && (TYgetDim (type) == 0)) {
            scalar = TYgetScalar (type);
            type = TYfreeTypeConstructor (type);
            type = TYmakeAKS (scalar, SHmakeShape (0));
        }
        DBUG_EXECUTE ("FIXNT", tmp_str2 = TYtype2String (type, FALSE, 0););
#if CWC_WOULD_BE_PROPER
        AVIS_TYPE (arg_node) = TYfreeType (AVIS_TYPE (arg_node));
#endif
        AVIS_TYPE (arg_node) = type;
        DBUG_PRINT ("FIXNT", ("... %s", tmp_str2));
        DBUG_EXECUTE ("FIXNT", tmp_str = ILIBfree (tmp_str);
                      tmp_str2 = ILIBfree (tmp_str2););

        if (!(TYisArray (type) || TYisBottom (type))) {
            CTIabort ("Could not infer proper type for arg %s", AVIS_NAME (arg_node));
        }
    } else {
        /*
         * AVIS_TYPE CAN BE NULL iff nt2ot is run in the opt cycle
         * AVIS nodes without AVIS_TYPE are now equipped with a default type (int).
         * This is important as dead variables can arise during the opt cycle.
         */
        AVIS_TYPE (arg_node) = TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTarray( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
NT2OTarray (node *arg_node, info *arg_info)
{
    ntype *elemtype;
    ntype *outer;
    ntype *nested;
    ntype *arrayelem;

    DBUG_ENTER ("NT2OTarray");

    /*
     * first do the clean up in the sons
     */
    arg_node = TRAVcont (arg_node, arg_info);

    /*
     * construct the new elemtype
     */
    if (INFO_LHS (arg_info) != NULL) {
        nested = TYcopyType (AVIS_TYPE (IDS_AVIS (INFO_LHS (arg_info))));
    } else {
        nested = NTCnewTypeCheck_Expr (arg_node);
    }

    outer = TYmakeAKS (TYcopyType (TYgetScalar (nested)),
                       SHcopyShape (ARRAY_SHAPE (arg_node)));
    arrayelem = ARRAY_ELEMTYPE (arg_node);

    elemtype = TYdeNestTypeFromOuter (nested, outer);

#ifndef DBUG_OFF
    if (!TYisSimple (TYgetScalar (arrayelem))
        || (TYgetSimpleType (TYgetScalar (arrayelem)) != T_unknown)) {

        DBUG_ASSERT (TYleTypes (elemtype, arrayelem),
                     "new element type of array does not match old type!");
    }
#endif

    ARRAY_ELEMTYPE (arg_node) = TYfreeType (ARRAY_ELEMTYPE (arg_node));
    ARRAY_ELEMTYPE (arg_node) = elemtype;

    outer = TYfreeType (outer);
    nested = TYfreeType (nested);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTblock( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTblock");

    INFO_VARDECMODE (arg_info) = M_fix;
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    INFO_VARDECMODE (arg_info) = M_filter;
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTvardec( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTvardec (node *arg_node, info *arg_info)
{
    node *this_vardec;

    DBUG_ENTER ("NT2OTvardec");

    if (INFO_VARDECMODE (arg_info) == M_fix) {
        VARDEC_AVIS (arg_node) = TRAVdo (VARDEC_AVIS (arg_node), arg_info);
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    if (INFO_VARDECMODE (arg_info) == M_filter) {
        if (TYisBottom (VARDEC_NTYPE (arg_node))) {
            /**
             * eliminate this vardec completely!
             */
            DBUG_PRINT ("FIXNT",
                        ("eliminating bottom vardec for %s", VARDEC_NAME (arg_node)));
            this_vardec = arg_node;
            arg_node = VARDEC_NEXT (arg_node);
            VARDEC_NEXT (this_vardec) = NULL;
            this_vardec = FREEdoFreeTree (this_vardec);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTlet( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTlet");

    if (IdsContainBottom (LET_IDS (arg_node))) {

        DBUG_PRINT ("FIXNT", ("bottom LHS found; eliminating N_let \"%s...\"",
                              IDS_NAME (LET_IDS (arg_node))));
        arg_node = FREEdoFreeTree (arg_node);
    } else {
        INFO_LHS (arg_info) = LET_IDS (arg_node);
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
        INFO_LHS (arg_info) = NULL;

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
 * @fn node *NT2OTassign( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NT2OTassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTassign");

    /**
     * First we go down in order to collect those funcond vars that need to be
     * created by the prf 'type_error'.
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /**
     * Now, we can go into the instruction. In case of N_lets that have bottom
     * vars on their LHS, we will obtain NULL, which signals us to eliminate
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
 * @fn node *NT2OTcond( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NT2OTcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTcond");

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
 * @fn node *NT2OTfuncond( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NT2OTfuncond (node *arg_node, info *arg_info)
{
    ntype *ttype, *etype;

    DBUG_ENTER ("NT2OTfuncond");

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

/** <!--********************************************************************-->
 *
 * @fn node *NT2OTpart( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
NT2OTpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("NT2OTpart");

    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    } else {
        INFO_WLIDS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTwithid( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTwithid (node *arg_node, info *arg_info)
{
    node *new_ids, *tmp_avis;
    node *new_vardecs;
    int i, num_vars;
    ntype *vec_type;

    DBUG_ENTER ("NT2OTwithid");

    if (INFO_WLIDS (arg_info) == NULL) {
        vec_type = AVIS_TYPE (IDS_AVIS (WITHID_VEC (arg_node)));
        vec_type = TYfixAndEliminateAlpha (vec_type);

        if (WITHID_IDS (arg_node) == NULL) {
            if (TYisAKS (vec_type)) {
                DBUG_PRINT ("NT2OT", ("WITHID_IDS for %s built",
                                      IDS_NAME (WITHID_VEC (arg_node))));
                num_vars = SHgetExtent (TYgetShape (vec_type), 0);
                new_ids = NULL;
                new_vardecs = INFO_VARDECS (arg_info);
                for (i = 0; i < num_vars; i++) {
                    tmp_avis
                      = TBmakeAvis (ILIBtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                              SHcreateShape (0)));
                    new_vardecs = TBmakeVardec (tmp_avis, new_vardecs);
                    new_ids = TBmakeIds (tmp_avis, new_ids);
                }
                WITHID_IDS (arg_node) = new_ids;
                INFO_WLIDS (arg_info) = new_ids;
                INFO_VARDECS (arg_info) = new_vardecs;
            } else {
                DBUG_PRINT ("NT2OT", ("no WITHID_IDS for %s built",
                                      IDS_NAME (WITHID_VEC (arg_node))));
            }
        } else {
            INFO_WLIDS (arg_info) = WITHID_IDS (arg_node);
        }
    } else {
        if (WITHID_IDS (arg_node) == NULL) {
            /**
             * we are dealing with a default partition here
             *  => Duplicate those of the real one!
             */
            DBUG_PRINT ("NT2OT", ("duplicating withids for non-first partition"));

            WITHID_IDS (arg_node) = DUPdoDupTree (INFO_WLIDS (arg_info));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *NT2OTwith( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
NT2OTwith (node *arg_node, info *arg_info)
{
    node *oldlhs;

    DBUG_ENTER ("NT2OTwith");

    /*
     * unfortunately, withloops may contain unflattened array constructors.
     * when updating the element type inf NT2OTarray, we rely on the type
     * of the LHS ids stored in INFO_LHS if it is present.
     * As the current LHS is that of the WL and the unflattened array
     * constructors do not have any LHS they are assigned to, we set the
     * LHS to NULL here. This will trigger a typecheck of the
     * array constructor in NT2OTarray (which is the only way to obtain
     * correct type information for it)...
     */
    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = NULL;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_LHS (arg_info) = oldlhs;

    DBUG_RETURN (arg_node);
}
