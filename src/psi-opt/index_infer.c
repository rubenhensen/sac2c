/*
 *
 * $Id$
 *
 */

/**
 *
 * @file index_infer.c
 *
 *  This file contains code to implement the array usage analysis for IVE.
 *  It computes two attributes for all N_avis nodes:
 *   AVIS_IDXTYPES - an N_exprs chain of N_type nodes that contains all
 *                   shapes of arrays this particular identifier selects into
 *   AVIS_NEEDCOUNT - counts all non-indexing uses of the identifier
 *
 *  NB: The analysis deals with Loop functions inter-functional, i.e., it
 *      treats them as if they were inlined. This requires a
 *      fix-point iteration for each loop function!!!
 */

#include "tree_basic.h"
#include "dbug.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "index_infer.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *intap;
    node *lhs;
    int uses;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_INTAP(n) ((n)->intap)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_USES(n) ((n)->uses)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INTAP (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_USES (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

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
 *
 * @defgroup ive IVE
 * @ingroup opt
 *
 * @{
 */

/**
 *
 * @file index_infer.c
 *
 *  This file contains the implementation of the inference of uses attributes
 *  for IVE (index vector elimination).
 *
 */
/**
 *
 * @name Some utility functions:
 *
 * @{
 */
static bool
eqShapes (ntype *a, ntype *b)
{
    bool res;

    DBUG_ENTER ("eqTypeShapes");

    res = SHcompareShapes (TYgetShape (a), TYgetShape (b));

    DBUG_RETURN (res);
}

static int
CountUses (node *args)
{
    int result = 0;

    DBUG_ENTER ("CountUses");

    while (args != NULL) {
        result = result + TCcountExprs (AVIS_IDXTYPES (ARG_AVIS (args)));

        args = ARG_NEXT (args);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn  void AddTypeToIdxTypes( node *avis, ntype *type)
 *
 *   @brief checks whether the shape of the type argument exists in the
 *          set of N_type nodes hanging off the AVIS_IDXTYPES( avis).
 *          If NOT, an N_type node containing that shape is built and
 *          added to the set.
 *          NOTE here, that the ntype type is expected to be at least AKS!
 *   @param
 *   @return
 *
 *****************************************************************************/

static void
AddTypeToIdxTypes (node *avis, ntype *type)
{
    node *exprs;

    DBUG_ENTER ("AddTypeToIdxTypes");

    exprs = AVIS_IDXTYPES (avis);
    while ((exprs != NULL) && (!eqShapes (type, TYPE_TYPE (EXPRS_EXPR (exprs))))) {
        exprs = EXPRS_NEXT (exprs);
    }

    if (exprs == NULL) {
        AVIS_IDXTYPES (avis)
          = TBmakeExprs (TBmakeType (TYeliminateAKV (type)), AVIS_IDXTYPES (avis));
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn  void TranscribeIdxTypes( node *fromavis, node *toavis)
 *
 *   @brief computes the setunion of the AVIS_IDXTYPES chains and attaches
 *          the result to toavis.
 *   @param
 *   @return
 *
 *****************************************************************************/

static void
TranscribeIdxTypes (node *fromavis, node *toavis)
{
    node *exprs;

    DBUG_ENTER ("TranscribeIdxTypes");

    exprs = AVIS_IDXTYPES (fromavis);
    while (exprs != NULL) {
        AddTypeToIdxTypes (toavis, TYPE_TYPE (EXPRS_EXPR (exprs)));
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn  void TranscribeFormalToConcrete( node *args, node *exprs)
 *
 *   @brief expects args to be the formal parameter chain of a function and
 *          exprs to be the actual parameter chain of a call to that very function.
 *          For each argument, it computes the setunion of the AVIS_IDXTYPES chains
 *          from the formal parameters and the actual parameters and attaches
 *          the result to the  AVIS_IDXTYPES chains of the actual parameters.
 *          NB: this is needed for the fixpoint iteration in Loop functions!
 *   @param
 *   @return
 *
 *****************************************************************************/

static void
TranscribeFormalToConcrete (node *args, node *exprs)
{
    DBUG_ENTER ("TranscribeFormalToConcrete");

    while (args != NULL) {
        TranscribeIdxTypes (ARG_AVIS (args), ID_AVIS (EXPRS_EXPR (exprs)));
        args = ARG_NEXT (args);
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_VOID_RETURN;
}

static void
TranscribeNeedToArgs (node *args, node *exprs)
{
    DBUG_ENTER ("TranscribeNeedToArgs");

    if (args != NULL) {
        DBUG_ASSERT ((exprs != NULL), "less conrete args than formal args!");

        TranscribeNeedToArgs (ARG_NEXT (args), EXPRS_NEXT (exprs));

        AVIS_NEEDCOUNT (ID_AVIS (EXPRS_EXPR (exprs))) += AVIS_NEEDCOUNT (ARG_AVIS (args));
    }

    DBUG_VOID_RETURN;
}

/*@}*/

/** <!--*******************************************************************-->
 *
 * @name Traversal Functions for IVEI:
 *
 * @{
 ****************************************************************************/

/** <!--*******************************************************************-->
 *
 * @fn node *IVEIap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IVEIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEIap");

    if (FUNDEF_ISDOFUN (AP_FUNDEF (arg_node))) {
        /*
         * for DO funs we have to use a fix-point iteration
         * to make sure the needs generated by the recurisve
         * call are propagated properly.
         */
        if (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info)) {
            int olduses = 0, uses;

            uses = CountUses (FUNDEF_ARGS (AP_FUNDEF (arg_node)));

            do {
                AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

                /*
                 * Add all types annotated at the formal parameters to the
                 * corresponding concrete parameters
                 */
                TranscribeFormalToConcrete (FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                            AP_ARGS (arg_node));

                olduses = uses;
                uses = CountUses (FUNDEF_ARGS (AP_FUNDEF (arg_node)));
            } while (uses != olduses);

            /*
             * As this is a do function, the need for the args
             * generated within the do function has to be
             * transcribed to the concrete args. (the body of
             * lacfuns is handeled as if it had been inlined)
             */
            TranscribeNeedToArgs (FUNDEF_ARGS (AP_FUNDEF (arg_node)), AP_ARGS (arg_node));
        } else {
            INFO_INTAP (arg_info) = arg_node;

            /*
             * Add all types annotated at the formal parameters to the corresponding
             * concrete parameters
             */
            TranscribeFormalToConcrete (FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                        AP_ARGS (arg_node));

            /*
             * as this is the internal application of a recursive
             * do fun, the need is not added to the concrete args
             * here. the idea is, that if an argument is not needed
             * for the function body, but only for the recurive
             * call, it in fact is not needed at all!
             */
        }
    } else if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        /*
         * as cond funs are non-recurisve, no fixpoint iteration is
         * needed here.
         */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

        /*
         * Add all types annotated at the formal parameters to the corresponding
         * concrete parameters. furthermore, the need information is
         * transcribed, as well.
         */
        TranscribeFormalToConcrete (FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                                    AP_ARGS (arg_node));
        TranscribeNeedToArgs (FUNDEF_ARGS (AP_FUNDEF (arg_node)), AP_ARGS (arg_node));
    } else {
        /*
         * this is a regular function application. so the need inference
         * has to be done for the concrete args
         */
        if (AP_ARGS (arg_node) != NULL) {
            AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEIassign( node *arg_node, info *arg_info )
 *
 *****************************************************************************/
node *
IVEIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEIassign");

    /* Bottom up traversal!! */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IVEIfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IVEIfundef (node *arg_node, info *arg_info)
{
    node *oldfundef;

    DBUG_ENTER ("IVEIfundef");

    if ((!FUNDEF_ISLACFUN (arg_node)) || (INFO_FUNDEF (arg_info) != NULL)) {

        /*
         * reset the NeedCount for all args
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        /*
         * traverse the body. this will reset the NeedCount for all
         * vardecs.
         */
        if (FUNDEF_BODY (arg_node) != NULL) {
            oldfundef = INFO_FUNDEF (arg_info);
            INFO_FUNDEF (arg_info) = arg_node;
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            /*
             * traverse the intap to update the annotations
             */
            if (FUNDEF_ISDOFUN (arg_node)) {
                INFO_INTAP (arg_info) = TRAVdo (INFO_INTAP (arg_info), arg_info);
            }

            /*
             * now reset the current fundef pointer!
             */
            INFO_FUNDEF (arg_info) = oldfundef;
        }
    }

    if ((INFO_FUNDEF (arg_info) == NULL) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IVEIblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IVEIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEIblock");

    /*
     * first traverse vardecs to reset use counter
     */
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    /*
     * then start the bottom up traversal of the instructions
     */
    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IVEIavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IVEIavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEIavis");

    /*
     * reset the need counter to its initial value
     */
    AVIS_NEEDCOUNT (arg_node) = 0;

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IVEIid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IVEIid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEIid");

    AVIS_NEEDCOUNT (ID_AVIS (arg_node))++;

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IVEIlet( node *arg_node, info *arg_info)
 *
 ****************************************************************************/
node *
IVEIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEIlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEIprf( node *arg_node, info *arg_info )
 *
 *****************************************************************************/

node *
IVEIprf (node *arg_node, info *arg_info)
{
    node *lhs, *arg1, *arg2, *arg3;
    ntype *ltype, *type1, *type2, *type3;

    DBUG_ENTER ("IVEIprf");

    switch (PRF_PRF (arg_node)) {
    case F_sel:
        lhs = INFO_LHS (arg_info);
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);

        DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) && (NODE_TYPE (arg2) == N_id)),
                     "wrong arg in F_sel application");

        ltype = IDS_NTYPE (lhs);
        type1 = ID_NTYPE (arg1);
        type2 = ID_NTYPE (arg2);

        if (TUshapeKnown (ltype) && TUshapeKnown (type2) && TUisIntVect (type1)
            && TUshapeKnown (type1)) {
            AddTypeToIdxTypes (ID_AVIS (arg1), type2);

            /*
             * as arg1 is used as a indexvector, we traverse arg2
             * only. this will mark it as non-index-vector use
             */
            PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        } else {
            /*
             * as we cannot replace this usage of the indexvector, it
             * has to be counted as a regular use. so we do this here
             */
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
        break;

    case F_modarray:
        lhs = INFO_LHS (arg_info);
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);
        arg3 = PRF_ARG3 (arg_node);

        DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) && (NODE_TYPE (arg2) == N_id)),
                     "wrong arg in F_modarray application");
        /*
         * arg3 of F_modarray may be a constant!
         */
        ltype = IDS_NTYPE (lhs);
        type1 = ID_NTYPE (arg1);
        type2 = ID_NTYPE (arg2);
        type3 = (NODE_TYPE (arg3) == N_id) ? ID_NTYPE (arg3) : NULL;

        if (TUshapeKnown (ltype) && TUshapeKnown (type1) && TUisIntVect (type2)
            && TUshapeKnown (type2) && ((type3 == NULL) || TUshapeKnown (type3))) {
            AddTypeToIdxTypes (ID_AVIS (arg2), type1);
            /*
             * argument 1 and 3 are non index-vector uses, so we
             * have to traverse them here
             */
            PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
            PRF_ARG3 (arg_node) = TRAVdo (PRF_ARG3 (arg_node), arg_info);
        } else {
            /*
             * as we cannot replace the index vector, we have to count
             * all args as uses
             */
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
        break;

    case F_add_AxA:
    case F_add_SxA:
    case F_add_AxS:
    case F_sub_AxA:
    case F_sub_SxA:
    case F_sub_AxS:
        lhs = INFO_LHS (arg_info);
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);

        ltype = IDS_NTYPE (lhs);
        type1 = (NODE_TYPE (arg1) == N_id) ? ID_NTYPE (arg1) : NULL;
        type2 = (NODE_TYPE (arg2) == N_id) ? ID_NTYPE (arg2) : NULL;

        if ((AVIS_NEEDCOUNT (IDS_AVIS (lhs)) == 0) && TUisIntVect (ltype)
            && TUshapeKnown (ltype) && ((type1 == NULL) || TUshapeKnown (type1))
            && ((type2 == NULL) || TUshapeKnown (type2))) {
            /*
             * the result is used as an index-vector only.
             * so we forward the infered shapes to the
             * array arguments. We do not forward it to
             * scalars. These are handeled in index_optimize.
             */
            if ((NODE_TYPE (arg1) == N_id) && TYgetDim (type1) != 0) {
                TranscribeIdxTypes (IDS_AVIS (lhs), ID_AVIS (arg1));
            }
            if ((NODE_TYPE (arg2) == N_id) && TYgetDim (type2) != 0) {
                TranscribeIdxTypes (IDS_AVIS (lhs), ID_AVIS (arg2));
            }
        } else {
            /*
             * this vector has to be calculated anyways (as it
             * is needed somewhere), so we increase the use
             * counter for the two args
             */
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
        break;

    case F_mul_AxS:
    case F_mul_SxA:
        lhs = INFO_LHS (arg_info);
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);

        ltype = IDS_NTYPE (lhs);
        type1 = (NODE_TYPE (arg1) == N_id) ? ID_NTYPE (arg1) : NULL;
        type2 = (NODE_TYPE (arg2) == N_id) ? ID_NTYPE (arg2) : NULL;

        if ((AVIS_NEEDCOUNT (IDS_AVIS (lhs)) == 0) && TUisIntVect (ltype)
            && TUshapeKnown (ltype) && ((type1 == NULL) || TUshapeKnown (type1))
            && ((type2 == NULL) || TUshapeKnown (type2))) {
            /*
             * the result is used as an index-vector only.
             * so we forward the infered shapes to the
             * array argument.
             */
            if ((NODE_TYPE (arg1) == N_id) && (TYgetDim (type1) != 0)) {
                TranscribeIdxTypes (IDS_AVIS (lhs), ID_AVIS (arg1));
            }
            if ((NODE_TYPE (arg2) == N_id) && (TYgetDim (type2) != 0)) {
                TranscribeIdxTypes (IDS_AVIS (lhs), ID_AVIS (arg2));
            }
        } else {
            /*
             * this vector has to be calculated anyways (as it
             * is needed somewhere), so we increase the use
             * counter for the two args
             */
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
        break;

    default:
        /*
         * for all other prfs we traverse the arg to ensure
         * that we find uses of identifiers other than identifiers
         */
        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *IVEIdoIndexVectorEliminationInference( node *syntax_tree)
 *
 *   @brief call this function for inferring all array uses.
 *   @param part of the AST (usually the entire tree) IVE is to be applied on.
 *   @return modified AST.
 *
 *****************************************************************************/
node *
IVEIdoIndexVectorEliminationInference (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IVEIdoIndexVectorEliminationInference");

    DBUG_PRINT ("OPT", ("Starting index vector inference..."));

    TRAVpush (TR_ivei);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);
    TRAVpop ();

    DBUG_PRINT ("OPT", ("Index vector inference complete!"));

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn  node *IVEIprintPreFun( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
IVEIprintPreFun (node *arg_node, info *arg_info)
{
    node *exprs;

    DBUG_ENTER ("IVEIprintPreFun");

    switch (NODE_TYPE (arg_node)) {
    case N_avis:
        exprs = AVIS_IDXTYPES (arg_node);
        printf ("/* IVEI results");
        if (exprs != NULL) {
            while (exprs != NULL) {
                printf (":IDX(%s)",
                        SHshape2String (0, TYgetShape (TYPE_TYPE (EXPRS_EXPR (exprs)))));
                exprs = EXPRS_NEXT (exprs);
            }
        }
        printf (":NEED(%d)", AVIS_NEEDCOUNT (arg_node));
        printf ("*/");
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/*@}*/
/*@}*/ /* defgroup ive */
