/* 	$Id$
 *
 * $Log$
 * Revision 1.6  1998/03/06 13:32:54  srs
 * added some WLI functions
 *
 * Revision 1.5  1998/02/27 13:38:10  srs
 * checkin with deactivated traversal
 *
 * Revision 1.4  1998/02/24 14:19:20  srs
 * *** empty log message ***
 *
 * Revision 1.3  1998/02/09 15:58:20  srs
 * *** empty log message ***
 *
 *
 */

/*******************************************************************************
 This file realizes the WL-folding for the new SAC-WLs.

 Withloop folding is done in 2 phases:
 1) WLI to gather information about the WLs
 2) WLF to find and fold suitable WLs.

 Assumption:
 We assume that all generators of a WL have the same shape.
 Furthermore we assume that, if an N_Ncode is referenced by more than one
 generator, all these generators' indexes (vector and scalar) have the same
 names.

 Usage of arg_info (WLI):
 - node[0]: store old information in nested WLs
 - node[1]: reference to base node of current WL (N_Nwith)
 - node[2]: always the last N_assign node (see WLIassign)
 - counter: nesting level of WLs.

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "types.h"
#include "internal_lib.h"
#include "free.h"
#include "print.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "optimize.h"
#include "WithloopFolding.h"

/******************************************************************************
 *
 * types, global variables
 *
 ******************************************************************************/

typedef struct INDEX_INFO {
    int vector;       /* this is an index vector (1) or a scalar (0) */
    int base_index;   /* only used if '!vector'. Number (starting */
                      /* with 0) of index scalar. */
    int *permutation; /* only used if 'vector'. Describes the */
                      /* permutation of index vars in this vector. */
    int valid;        /* this is a valid transformation (0/1) */
} index_info;

#define INDEX(n) ((index_info *)ASSIGN_INDEX (n))

/******************************************************************************
 *
 * function:
 *   node *WLIfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   we search in every function separately for withloops.
 *   The folding always happens exclusively in a function body.
 *
 *   The optimization traversal OPTTrav is included in WLI traversal to
 *   modify USE and DEF and to create MRD masks.
 *
 ******************************************************************************/

node *
WLIfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIfundef");

    INFO_WLI_WL (arg_info) = NULL;

    FUNDEF_INSTR (arg_node) = OPTTrav (FUNDEF_INSTR (arg_node), arg_info, arg_node);
    FUNDEF_NEXT (arg_node) = OPTTrav (FUNDEF_NEXT (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIassign(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIassign");

    INFO_WLI_ASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = OPTTrav (ASSIGN_INSTR (arg_node), arg_info, arg_node);
    ASSIGN_NEXT (arg_node) = OPTTrav (ASSIGN_NEXT (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIcond(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIcond");

    /* we do not need to traverse the condition
       COND_COND(arg_node) = OPTTrav(COND_COND(arg_node), arg_info, arg_node);
    */

    /* traverse bodies. MRDs are build locally in the bodies. The DEF mask is
       evaluated in the superior OPTTrav to modify the actual MRD list. */
    COND_THENINSTR (arg_node) = OPTTrav (COND_THENINSTR (arg_node), arg_info, arg_node);
    COND_ELSEINSTR (arg_node) = OPTTrav (COND_ELSEINSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIdo(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIdo");

    DO_INSTR (arg_node) = OPTTrav (DO_INSTR (arg_node), arg_info, arg_node);
    DO_COND (arg_node) = OPTTrav (DO_COND (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIwhile(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIwhile");

    WHILE_COND (arg_node) = OPTTrav (WHILE_COND (arg_node), arg_info, arg_node);
    WHILE_INSTR (arg_node) = OPTTrav (WHILE_INSTR (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIwith(node *arg_node, node *arg_info)
 *
 * description:
 *   only needed to apply OPTTrav
 *
 *
 ******************************************************************************/

node *
WLIwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLIwith");

    WITH_GEN (arg_node) = OPTTrav (WITH_GEN (arg_node), arg_info, arg_node);
    switch (NODE_TYPE (WITH_OPERATOR (arg_node))) {
    case N_genarray:
        BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (GENARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_modarray:
        BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (MODARRAY_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldprf:
        BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDPRF_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    case N_foldfun:
        BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node)))
          = OPTTrav (BLOCK_INSTR (FOLDFUN_BODY (WITH_OPERATOR (arg_node))), arg_info,
                     arg_node);
        break;
    default:
        DBUG_ASSERT (0, "Operator not implemented for with_node");
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIlet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLIlet (node *arg_node, node *arg_info)
{
    node *tmpn, *idn, *assignn;

    DBUG_ENTER ("WLIlet");

#if 0
  /* if we are inside a WL we have to search for valid index transformations. */
  if (INFO_WLI_WL(arg_info)) {
    assignn = INFO_WLI_ASSIGN(arg_info);
    INDEX(assignn) = Malloc(sizeof(index_info));
    INDEX(assignn)->valid = 0;	/* invalid by default */
    
    tmpn = LET_EXPR(arg_node);
  
    if (N_prf == NODE_TYPE(tmpn)) {
      /* this may ba an assignment which calculates an index for an
	 array to fold. Exactly one argument has to be an integer constant.*/
      idn = NULL;
      if (N_num == NODE_TYPE(PRF_ARG1(tmpn)))
	idn = PRF_ARG2(tmpn);
      if (N_num == NODE_TYPE(PRF_ARG2(tmpn)))
	idn = PRF_ARG1(tmpn);
    
/*       if (idn && N_id == NODE_TYPE(idn)) { */
	/* arguments of prf: one constant and one id. */
	/* is this an index var? */
/* 	if (MRD(ID_VARNO(idn))) */
    }
    
  }
  DBU
#endif

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLIid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLIid (node *arg_node, node *arg_info)
{
    node *tmpn;

    DBUG_ENTER (" *WLIid");

    /*   tmpn = MRD(ID_VARNO(arg_node)); */
    /*   printf("NAME: %s   ZEILE %d    MRD Zeile %d\n", */
    /* 	 ID_NAME(arg_node),NODE_LINE(arg_node), */
    /* 	 (NULL == tmpn) ? -1 : NODE_LINE(tmpn)); */
    /*   if (tmpn) PrintNodeTree(tmpn); */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLINwith(node *arg_node, node *arg_info)
 *
 * description:
 *   start gathering information for this WL.
 *   First some initialisations in the WL structure are done, then the
 *   N_Nparts are traversed (which call the appropriate N_Ncode subtrees).
 *
 ******************************************************************************/

node *
WLINwith (node *arg_node, node *arg_info)
{
    node *tmpn;

    DBUG_ENTER ("WLINwith");

    /* inside the body of this WL we may find another WL. So we better
       save the old arg_info information. */
    tmpn = MakeInfo ();
    tmpn->mask[0] = arg_info->mask[0]; /* DEF and USE information have */
    tmpn->mask[1] = arg_info->mask[1]; /* to be identical. */
    INFO_WLI_NEXT (tmpn) = arg_info;
    arg_info = tmpn;

    /* initialize WL traversal */
    INFO_WLI_WL (arg_info) = arg_node; /* store the current node for later */
    tmpn = NWITH_CODE (arg_node);
    while (tmpn) { /* reset traversal flag for each code */
        NCODE_FLAG (tmpn) = 0;
        tmpn = NCODE_NEXT (tmpn);
    }
    NWITH_REFERENCED (arg_node) = 0;
    NWITH_REFERENCED_FOLD (arg_node) = 0;
    NWITH_COMPLEX (arg_node) = 0;
    NWITH_FOLDABLE (arg_node) = 1;

    /* traverse all parts (and implicitely bodies).
       It is not possible that WLINpart calls the NPART_NEXT node because
       the superior OPTTrav mechanism has to finisch before calling the
       next part. Else modified USE and DEF masks will case errors. */
    tmpn = NWITH_PART (arg_node);
    while (tmpn) {
        tmpn = OPTTrav (tmpn, arg_info, arg_node);
        tmpn = NPART_NEXT (tmpn);
    }

    /* restore arg_info */
    tmpn = arg_info;
    arg_info = INFO_WLI_NEXT (arg_info);
    FREE (tmpn);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLINpart(node *arg_node, node *arg_info)
 *
 * description:
 *   constant bounds, step and width vectors are substituted into the
 *   generator. Then the appropriate body is traversed.
 *
 *
 ******************************************************************************/

node *
WLINpart (node *arg_node, node *arg_info)
{
    node *tmpn, *bound;
    int i;

    DBUG_ENTER ("WLINpart");

    /* Can we replace any identifyer in the generator with a constant vector? */
    for (i = 1; i <= 4; i++) {
        switch (i) {
        case 1:
            bound = NGEN_BOUND1 (NPART_GEN (arg_node));
            break;
        case 2:
            bound = NGEN_BOUND2 (NPART_GEN (arg_node));
            break;
        case 3:
            bound = NGEN_STEP (NPART_GEN (arg_node));
            break;
        case 4:
            bound = NGEN_WIDTH (NPART_GEN (arg_node));
            break;
        }

        if (bound && N_id == NODE_TYPE (bound)) {
            tmpn = MRD (ID_VARNO (bound));
        }

        /*     if (!IsConstArray(bound)) */
    }

    /* traverse code. But do this only once, even if there are more than
       one referencing generators.
       This is just a cross reference, so just traverse, so not assign the
       resulting node.*/
    if (!NCODE_FLAG (NPART_CODE (arg_node)))
        OPTTrav (NPART_CODE (arg_node), arg_info, INFO_WLI_WL (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLINcode(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLINcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLINcode");

    NCODE_FLAG (arg_node) = 1; /* this body has been traversed and all
                          information has been gathered. */

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    NCODE_NEXT (arg_node) = OPTTrav (NPART_NEXT (arg_node), arg_info, arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFNwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLFNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFNwith");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFWithloopFolding(node *arg_node, node* arg_info)
 *
 * description:
 *   starting point for the withloop folding.
 *
 *
 ******************************************************************************/

node *
WLFWithloopFolding (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFWithloopFolding");

    DBUG_ASSERT (!arg_info, ("at the beginning of WLF: arg_info != NULL"));
    arg_info = MakeInfo ();

    /* WLI traversal: search information */
    /*   act_tab = wli_tab; */
    /*   Trav(arg_node,arg_info); */

    /* WLF traversal: fold WLs */
    /*   act_tab = wlf_tab; */
    /*   Trav(arg_node,arg_info); */

    FREE (arg_info);

    DBUG_RETURN (arg_node);

    /* srs: remarks
       ============

       remember to free additional information in ASSIGN_INDEX

  */
}
