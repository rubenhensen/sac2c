/*
 *
 * $Id$
 *
 */

#include <stdio.h>
#include <string.h>
#include "elim_alpha_types.h"

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
 * This phase eliminates all type variables and replaces them by their last
 * approximation.
 *
 * Furthermore, we replace AKD(0) types by AKS([]) types.
 *
 * Furthermore, we insert withids whenever possible.
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
    node *lhs;
    bool onefunction;
    node *fundef;
};

/**
 * INFO macros
 */
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_WLIDS(n) ((n)->wlids)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_ONEFUNCTION(n) ((n)->onefunction)
#define INFO_FUNDEF(n) ((n)->fundef)

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
    INFO_LHS (result) = NULL;
    INFO_ONEFUNCTION (result) = FALSE;
    INFO_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   node *EATdoEliminateAlphaTypes( node *arg_node)
 *
 * description:
 *   replaces all type vars by their last approximation!
 *
 ******************************************************************************/

node *
EATdoEliminateAlphaTypes (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("EATdoEliminateAlphaTypes");

    TRAVpush (TR_eat);

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
 *   node *EATdoEliminateAlphaTypesOneFunction( node *arg_node)
 *
 * description:
 *   adjusts all old vardec types according to the attached ntypes!
 *
 ******************************************************************************/

node *
EATdoEliminateAlphaTypesOneFunction (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("EATdoEliminateAlphaTypesOneFunction");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "EATdoEliminateAlphaTypesOneFunction can only be applied to fundefs");

    TRAVpush (TR_eat);

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

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EATmodule( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EATmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EATmodule");

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
 *   node *EATfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EATfundef (node *arg_node, info *arg_info)
{
    ntype *otype, *ftype;

    DBUG_ENTER ("EATfundef");

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
                 * this has to be done here rather than in EATblock since blocks
                 * in general may not be top level.
                 * AFAIK, the only place where vardecs are in fact built is the
                 * extension of withloop ids in EATwithid.
                 */
                INFO_VARDECS (arg_info) = TRAVdo (INFO_VARDECS (arg_info), arg_info);

                FUNDEF_VARDEC (arg_node)
                  = TCappendVardec (INFO_VARDECS (arg_info), FUNDEF_VARDEC (arg_node));
                INFO_VARDECS (arg_info) = NULL;
            }
        }
    }

    if ((!INFO_ONEFUNCTION (arg_info)) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EATap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EATap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EATap");

    arg_node = TRAVcont (arg_node, arg_info);

    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("FIXNT", ("lacfun %s found...", CTIitemName (AP_FUNDEF (arg_node))));

        info *new_info = MakeInfo ();
        INFO_ONEFUNCTION (new_info) = TRUE;
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_info);
        new_info = FreeInfo (new_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EATavis( node *arg_node, info *arg_info)
 *
 * description:
 *   Fix (!) the ntype.
 *
 ******************************************************************************/

node *
EATavis (node *arg_node, info *arg_info)
{
    ntype *type, *scalar;
#ifndef DBUG_OFF
    char *tmp_str, *tmp_str2;
#endif

    DBUG_ENTER ("EATavis");

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
 *   node *EATarray( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
EATarray (node *arg_node, info *arg_info)
{
    ntype *elemtype;
    ntype *outer;
    ntype *nested;
    ntype *arrayelem;

    DBUG_ENTER ("EATarray");

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
 *   node *EATblock( node *arg_node, info *arg_info)
 *
 * description:
 *   FIXME: vardec insertion should happen here rather than in fundef!
 *
 ******************************************************************************/

node *
EATblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EATblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EATlet( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EATlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EATlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EATpart( node *arg_node, info *arg_info)
 *
 *   @brief adding missing withids!
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
EATpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EATpart");

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
 *   node *EATwithid( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EATwithid (node *arg_node, info *arg_info)
{
    node *new_ids, *tmp_avis;
    node *new_vardecs;
    int i, num_vars;
    ntype *vec_type;

    DBUG_ENTER ("EATwithid");

    if (INFO_WLIDS (arg_info) == NULL) {
        vec_type = AVIS_TYPE (IDS_AVIS (WITHID_VEC (arg_node)));
        vec_type = TYfixAndEliminateAlpha (vec_type);

        if (WITHID_IDS (arg_node) == NULL) {
            if (TYisAKS (vec_type)) {
                DBUG_PRINT ("EAT", ("WITHID_IDS for %s built",
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
                DBUG_PRINT ("EAT", ("no WITHID_IDS for %s built",
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
            DBUG_PRINT ("EAT", ("duplicating withids for non-first partition"));

            WITHID_IDS (arg_node) = DUPdoDupTree (INFO_WLIDS (arg_info));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *EATwith( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
EATwith (node *arg_node, info *arg_info)
{
    node *oldlhs;

    DBUG_ENTER ("EATwith");

    /*
     * unfortunately, withloops may contain unflattened array constructors.
     * when updating the element type inf EATarray, we rely on the type
     * of the LHS ids stored in INFO_LHS if it is present.
     * As the current LHS is that of the WL and the unflattened array
     * constructors do not have any LHS they are assigned to, we set the
     * LHS to NULL here. This will trigger a typecheck of the
     * array constructor in EATarray (which is the only way to obtain
     * correct type information for it)...
     */
    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = NULL;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_LHS (arg_info) = oldlhs;

    DBUG_RETURN (arg_node);
}
