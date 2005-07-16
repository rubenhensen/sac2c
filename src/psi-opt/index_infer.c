/*
 *
 * $Log$
 * Revision 1.1  2005/07/16 19:00:18  sbs
 * Initial revision
 *
 *
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
    shape *shp;
};
/*
 * INFO macros
 */
#define INFO_IVEI_SHP(n) (n->shp)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_IVEI_SHP (result) = NULL;

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
 * @name Some functions for handling N_vinfo nodes:
 *
 * <!--
 * node * FindVect( node *N_vinfo_chain)        : checks whether VECT is in
 *                                                  the chain of N_vinfo nodes
 * node * FindIdx( node *chain, shape *shp)     : dito for a specific shape
 *
 * node * SetVect( node *chain)                 : inserts VECT-node in chain if
 *                                                  not already present
 * node * SetIdx( node *chain, shape *shp)      : inserts IDX-node if not
 *                                                  already present
 * -->
 * @{
 */

/** <!--********************************************************************-->
 *
 * @fn node *FindVect( node *chain)
 *
 *   @brief checks whether VECT is in the chain.
 *   @param chain  chain of vinfo-nodes to be searched
 *   @return the address of the VECT-node or the address of a vinfo-node with
 *           DOLLAR-flag (= no VECT in chain)
 *
 ******************************************************************************/

node *
FindVect (node *chain)
{
    DBUG_ENTER ("FindVect");

    while (VINFO_FLAG (chain) == IDX) {
        chain = VINFO_NEXT (chain);
    }

    DBUG_RETURN (chain);
}

/** <!--********************************************************************-->
 *
 * @fn  node *FindIdx( node *chain, shape *vshape)
 *
 *   @brief  checks whether IDX(vshape) is in the chain.
 *   @param  chain
 *   @param  vshape
 *   @return NULL (= IDX(idx-shape) not in chain) or the adress of the IDX-node
 *
 ******************************************************************************/

node *
FindIdx (node *chain, shape *vshape)
{
    DBUG_ENTER ("FindIdx");

    while ((VINFO_FLAG (chain) != DOLLAR)
           && ((VINFO_FLAG (chain) != IDX)
               || !SHcompareShapes (VINFO_SHAPE (chain), vshape))) {
        chain = VINFO_NEXT (chain);
    }

    DBUG_RETURN (chain);
}

/** <!--********************************************************************-->
 *
 * @fn  node *SetVect( node *chain)
 *
 *   @brief  inserts a VECT node in the given node-chain if there exists none yet.
 *   @param  chain
 *   @return potentially extended chain
 *
 ******************************************************************************/

node *
SetVect (node *chain)
{
    DBUG_ENTER ("SetVect");

    DBUG_PRINT ("IDX", ("VECT assigned"));
    if (VINFO_FLAG (FindVect (chain)) == DOLLAR) {
        chain = TBmakeVinfo (VECT, NULL, chain, VINFO_DOLLAR (chain));
    }

    DBUG_RETURN (chain);
}

/** <!--*********************************************************************-->
 *
 * @fn  node *SetIdx( node *chain, shape *vartype)
 *
 *   @brief  inserts an IDX(shape) node in the given node-chain if there exists
 *           none yet.
 *   @param  chain
 *   @param  vartype shape to be inserted
 *   @return potentially extended chain
 *
 ******************************************************************************/

node *
SetIdx (node *chain, shape *varshape)
{
#ifndef DBUG_OFF
    char *shp_str;
#endif

    DBUG_ENTER ("SetIdx");

    if (VINFO_FLAG (FindIdx (chain, varshape)) == DOLLAR) {
        chain = TBmakeVinfo (IDX, SHcopyShape (varshape), chain, VINFO_DOLLAR (chain));
        DBUG_EXECUTE ("IDX", shp_str = SHshape2String (0, VINFO_SHAPE (chain)););
        DBUG_PRINT ("IDX", ("IDX(%s) assigned", shp_str));
        DBUG_EXECUTE ("IDX", shp_str = ILIBfree (shp_str););
    }

    DBUG_RETURN (chain);
}

/*@}*/

/******************************************************************************
 ***
 ***          Here, the traversal functions start!
 ***          ------------------------------------
 ***
 ******************************************************************************/
/**
 *
 * @name Traversal Functions for IVEI:
 *
 * @{
 */

/** <!--********************************************************************-->
 *
 * @fn node *IVEIassign( node *arg_node, info *arg_info )
 *
 ******************************************************************************/

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

/** <!--********************************************************************-->
 *
 * @fn node *IVEIprf( node *arg_node, info *arg_info )
 *
 ******************************************************************************/

node *
IVEIprf (node *arg_node, info *arg_info)
{
    node *arg1, *arg2, *arg3;
    ntype *type1, *type2;

    DBUG_ENTER ("IVEIprf");

    switch (PRF_PRF (arg_node)) {
    case F_sel:
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);
        DBUG_ASSERT (((NODE_TYPE (arg2) == N_id) || (NODE_TYPE (arg2) == N_array)),
                     "wrong arg in F_sel application");

        type1 = ID_NTYPE (arg1);
        type2 = ID_NTYPE (arg2);

        if (TUshapeKnown (type2) && TUisIntVect (type1)) {
            INFO_IVEI_SHP (arg_info) = TYgetShape (type2);
            PRF_ARG1 (arg_node) = TRAVdo (arg1, arg_info);
        } else {
            INFO_IVEI_SHP (arg_info) = NULL;
            PRF_ARG1 (arg_node) = TRAVdo (arg1, arg_info);
        }
        INFO_IVEI_SHP (arg_info) = NULL;
        PRF_ARG2 (arg_node) = TRAVdo (arg2, arg_info);
        break;

    case F_modarray:
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);
        arg3 = PRF_ARG3 (arg_node);
        DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) || (NODE_TYPE (arg1) == N_array)),
                     "wrong arg in F_modarray application");

        type1 = ID_NTYPE (arg1);
        type2 = ID_NTYPE (arg2);

        if (TUshapeKnown (type1) && TUisIntVect (type2)) {
            INFO_IVEI_SHP (arg_info) = TYgetShape (type1);
            PRF_ARG2 (arg_node) = TRAVdo (arg2, arg_info);
        } else {
            INFO_IVEI_SHP (arg_info) = NULL;
            PRF_ARG2 (arg_node) = TRAVdo (arg2, arg_info);
        }
        INFO_IVEI_SHP (arg_info) = NULL;
        PRF_ARG1 (arg_node) = TRAVdo (arg1, arg_info);
        PRF_ARG3 (arg_node) = TRAVdo (PRF_ARG3 (arg_node), arg_info);
        break;

    default:
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEIid( node *arg_node, info *arg_info )
 *
 *   @brief examines whether variable is a one-dimensional array;
 *          if so, SetVect on "N_avis" belonging to the "N_id" node.
 *
 ******************************************************************************/

node *
IVEIid (node *arg_node, info *arg_info)
{
    shape *shp;
    node *avis;

    DBUG_ENTER ("IVEIid");

    avis = ID_AVIS (arg_node);

    if (TUisIntVect (AVIS_TYPE (avis))) {
        if (INFO_IVEI_SHP (arg_info) == NULL) {
            DBUG_PRINT ("IVE", ("assigning VECT to %s:", ID_NAME (arg_node)));
            AVIS_USECHN (avis) = SetVect (AVIS_USECHN (avis));
        } else {
            shp = INFO_IVEI_SHP (arg_info);
            DBUG_PRINT ("IVE", ("assigning IDX to %s:", ID_NAME (arg_node)));
            AVIS_USECHN (avis) = SetIdx (AVIS_USECHN (avis), shp);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--*********************************************************************-->
 *
 * @fn  node *IndexVectorEliminationInference( node *syntax_tree)
 *
 *   @brief call this function for inferring all array uses.
 *   @param part of the AST (usually the entire tree) IVE is to be applied on.
 *   @return modified AST.
 *
 ******************************************************************************/

node *
IndexVectorEliminationInference (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IndexVectorEliminationInference");

    TRAVpush (TR_ivei);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/*@}*/
/*@}*/ /* defgroup ive */
