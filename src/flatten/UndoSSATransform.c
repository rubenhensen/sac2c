/*
 * $Log$
 * Revision 1.14  2004/11/25 12:50:11  khf
 * SacDevCamp: COMPILES!
 *
 * Revision 1.13  2004/08/05 21:00:05  sbs
 * global variable ssaform_phase modified now.
 *
 * Revision 1.12  2004/08/04 12:04:41  ktr
 * substituted eacc by emm
 *
 * Revision 1.11  2004/07/23 15:24:04  khf
 * changed flag for explicit accumulation from ktr to eacc
 *
 * Revision 1.10  2004/07/23 13:19:26  khf
 * if explicit accumulation was applied, no identical names for
 * cexpr nodes of multigenerator fold WLs are longer necessary
 *
 * Revision 1.9  2004/07/16 18:36:55  sah
 * forgotten to allocate space for info structure;)
 *
 * Revision 1.8  2004/07/16 17:36:23  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.7  2004/05/12 12:59:40  ktr
 * Code for NCODE_EPILOGUE added
 *
 * Revision 1.6  2004/05/11 13:27:30  khf
 * Replaced NCODE_CEXPR in USSANcode() by NCODE_CEXPRS for genarray or modarray WLs
 *
 * Revision 1.5  2004/03/06 20:06:40  mwe
 * changed arguments of MakeCond in USSAfundef
 *
 * Revision 1.4  2004/03/05 19:14:27  mwe
 * representation of conditional changed
 * using N_funcond node instead of phi
 *
 * Revision 1.3  2004/02/09 15:23:53  mwe
 * PHIT_NONE assignments for PHITARGET added
 *
 * Revision 1.2  2004/02/06 14:19:33  mwe
 * condTransform added
 * replace primitive phi function with PHITARGET
 *
 * Revision 1.1  2004/01/28 16:56:18  skt
 * Initial revision
 *
 *
 *
 *
 *
 ************ Attention! ************
 * File was moved from ../tree
 * following older Revisions can be found there
 *
 *
 * Revision 1.20  2003/03/12 23:29:57  dkr
 * USSAvardec(), USSAarg(): AVIS_SSACOUNT removed since all N_ssacnt nodes
 * (BLOCK_SSACOUNTER) are removed in USSAblock().
 *
 * Revision 1.19  2002/08/05 09:52:04  sbs
 * eliminated the requirement for Nwithid nodes to always have both,
 * an IDS and a VEC attribute. This change is motivated by the requirement
 * to convert to SSA form prior to type checking. Furthermore, not being
 * restricted to the AKS case anymore, we cannot guarantee the existance
 * of the IDS attribute in all cases anymore !!!!
 *
 * Revision 1.18  2002/02/20 14:37:04  dkr
 * function DupTypes() renamed into DupAllTypes()
 *
 * Revision 1.17  2002/02/12 15:44:45  dkr
 * no changes done
 *
 * Revision 1.16  2001/06/01 16:09:06  nmw
 * removal of assignments like a=a after renaming added
 *
 * Revision 1.15  2001/05/17 11:38:19  dkr
 * FREE/MALLOC eliminated
 *
 * Revision 1.14  2001/05/09 12:31:29  nmw
 * remove unused vardecs after renaming operation
 *
 * Revision 1.13  2001/04/19 08:03:27  dkr
 * macro F_PTR used as format string for pointers
 *
 * Revision 1.12  2001/04/18 13:00:29  nmw
 * debug output for OPT traversal added
 *
 * Revision 1.11  2001/04/05 12:31:32  nmw
 * re-renaming of global objects improved (works for inlined code, too)
 *
 * Revision 1.10  2001/04/03 14:24:49  nmw
 * debug messages added
 *
 * Revision 1.9  2001/03/29 09:09:29  nmw
 * tabs2spaces done
 *
 * Revision 1.8  2001/03/22 20:01:53  dkr
 * include of tree.h eliminated
 *
 * Revision 1.7  2001/03/20 14:22:28  nmw
 * documentation added
 *
 * Revision 1.6  2001/03/19 14:23:32  nmw
 * removal of ssa phi copy assignments added
 *
 * Revision 1.5  2001/03/16 11:55:59  nmw
 * AVIS_SSAPHITARGET type changed
 *
 * Revision 1.4  2001/03/15 10:53:31  nmw
 * Undo SSA for all vardecs marked with SSAUNDOFLAG
 *
 * Revision 1.3  2001/03/12 17:20:43  nmw
 * Pointer Sharing fixed
 *
 * Revision 1.2  2001/03/12 13:41:53  nmw
 * UndoSSA creates unique result variables in multigenerator fold-withloops.
 *
 * Revision 1.1  2001/02/22 13:14:06  nmw
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   UndoSSATransform.c
 *
 * prefix: USSA
 *
 * description:
 *
 * 1. This module renames all artificial identifier to their original
 *    baseid to avoid problems with multiple global object names in the
 *    compiler backend.
 *    All identifiers marked with AVIS_SSAUNDOFLAG are re-renamed. This
 *    flag has been set by SSATransform.
 *
 * 2. All result-variables of a multigenerator fold-withloop are made identical
 *    by inserting an additional variable and corresponding assignments at
 *    the end of each Ncode CBLOCK and adjusting the CEXPR identifier to
 *    the new created variable. This adjustment is necessary for the inlining
 *    of the fold-function.
 *
 * 3. in special functions (lifted do and while loops) a removal of the
 *    phi-copy-target assignments is done by renaming different variables in
 *    then- and else-part to the single variable used in the return statement.
 *    Constant phi-copy-targets in the else-part are moved down behind the
 *    conditional. From now on the else parts of conditionals in do- and while
 *    functions are empty as it is required by the fun2lac transformation.
 *
 *    Example:
 *    int, globobj_t f_do(int i, globobj_t obj)
 *    {
 *      <body with assignments to i_SSA_1 and obj_SSA_1>
 *
 *      if (<condition>) {
 *        i_SSA_2, obj_SSA_2 = f_do(i_SSA_1, obj_SSA_1);
 *        i_SSA_3 = i_SSA_2;
 *        obj_SSA_3 = obj_SSA_2;
 *      } else {
 *        i_SSA_3 = i_SSA_1;
 *        obj_SSA_3 = obj_SSA_1;
 *      }
 *     return(i_SSA_3, obj_SSA_3);
 *    }
 *
 *    will be transformed to:
 *    int, globobj_t f_do(int i, globobj_t obj)
 *    {
 *      <body with assignments to i_SSA_3 and obj>
 *
 *      if (<condition>) {
 *        i_SSA_3, obj = f_do(i_SSA_3, obj);
 *      } else {
 *        <empty>
 *      }
 *      return(i_SSA_3, obj);
 *     }
 *
 *     where all identifiers of obj(_SSA_x) are mapped back to the one global
 *     object obj. And the different phi copy assignments for i_SSA_3 from
 *     i_SSA_2 and i_SSA_1 are made identical by renaming them to i_SSA_3;
 *
 *****************************************************************************/

#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "UndoSSATransform.h"
#include "DupTree.h"

#define OPASSIGN_NOOP 0
#define OPASSIGN_REMOVE 1
#define OPASSIGN_MOVE 2

/*
 * INFO structure
 */
struct INFO {
    node *args;
    node *topblock;
    node *constassigns;
    int opassign;
    node *module;
    node *phifun;
    node *fundef;
};

/*
 * INFO macros
 */
#define INFO_USSA_ARGS(n) (n->args)
#define INFO_USSA_TOPBLOCK(n) (n->topblock)
#define INFO_USSA_CONSTASSIGNS(n) (n->constassigns)
#define INFO_USSA_OPASSIGN(n) (n->opassign)
#define INFO_USSA_MODULE(n) (n->module)
#define INFO_USSA_PHIFUN(n) (n->phifun)
#define INFO_USSA_FUNDEF(n) (n->fundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_USSA_ARGS (result) = NULL;
    INFO_USSA_TOPBLOCK (result) = NULL;
    INFO_USSA_CONSTASSIGNS (result) = NULL;
    INFO_USSA_OPASSIGN (result) = 0;
    INFO_USSA_MODULE (result) = NULL;
    INFO_USSA_PHIFUN (result) = NULL;
    INFO_USSA_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/* helper functions for local use */
static void UssaInitAvisFlags (node *fundef);
static node *UssaRemoveUnusedVardecs (node *vardecs);

static node *CondTransform (node *arg_node, info *arg_info);

/******************************************************************************
 *
 * function:
 *   void USSAInitAvisFlags(node *fundef)
 *
 * description:
 *   inits flags AVIS_SUBST, AVIS_SUBSTUSSA needed for this module
 *   in all vardec and args.
 *
 ******************************************************************************/
static void
UssaInitAvisFlags (node *fundef)
{
    node *tmp;

    DBUG_ENTER ("UssaInitAvisFlags");

    /* process args */
    tmp = FUNDEF_ARGS (fundef);
    while (tmp != NULL) {
        AVIS_SUBST (ARG_AVIS (tmp)) = NULL;
        AVIS_SUBSTUSSA (ARG_AVIS (tmp)) = NULL;
        tmp = ARG_NEXT (tmp);
    }

    /* process vardecs */
    if (FUNDEF_BODY (fundef) != NULL) {
        tmp = BLOCK_VARDEC (FUNDEF_BODY (fundef));
        while (tmp != NULL) {
            AVIS_SUBST (VARDEC_AVIS (tmp)) = NULL;
            AVIS_SUBSTUSSA (VARDEC_AVIS (tmp)) = NULL;
            tmp = VARDEC_NEXT (tmp);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *UssaRemoveUnusedVardecs(node *vardecs)
 *
 * description:
 *   free all vardecs marked for complete substitution.
 *
 ******************************************************************************/
static node *
UssaRemoveUnusedVardecs (node *vardecs)
{
    node *avis;

    DBUG_ENTER ("UssaRemoveUnusedVardecs");

    /* traverse rest of chain */
    if (VARDEC_NEXT (vardecs) != NULL) {
        VARDEC_NEXT (vardecs) = UssaRemoveUnusedVardecs (VARDEC_NEXT (vardecs));
    }

    avis = VARDEC_AVIS (vardecs);

    /* check vardec for removal */
    if ((AVIS_SUBSTUSSA (avis) != NULL) && (AVIS_SUBSTUSSA (avis) != avis)) {
        DBUG_PRINT ("USSA", ("remove unused vardec %s", VARDEC_NAME (vardecs)));
        vardecs = FREEdoFreeNode (vardecs);
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *  node *USSATarg(node *arg_node, info *arg_info)
 *
 * description:
 *  check args for AVIS_SUBST entries.
 *  because it is not a good idea to rename args (e.g. unique identifiers,
 *  global objects) the arg node is used as target identifier in this function
 *  instead of the marked SUBST vardec. This can easily be done by exchanging
 *  the avis nodes of arg and vardec (that will not be used anymore after USSA).
 *
 ******************************************************************************/
node *
USSATarg (node *arg_node, info *arg_info)
{
    node *tmp;
    node *vardec;

    DBUG_ENTER ("USSATarg");

    if (AVIS_SUBST (ARG_AVIS (arg_node)) != NULL) {
        DBUG_PRINT ("USSA", ("using arg %s instead of vardec %s", ARG_NAME (arg_node),
                             AVIS_NAME (AVIS_SUBST (ARG_AVIS (arg_node)))));

        /* use avis of vardec node as avis of arg node */
        vardec = AVIS_DECL (AVIS_SUBST (ARG_AVIS (arg_node)));
        tmp = ARG_AVIS (arg_node);

        ARG_AVIS (arg_node) = VARDEC_AVIS (vardec);
        AVIS_DECL (ARG_AVIS (arg_node)) = arg_node;
        AVIS_SUBSTUSSA (ARG_AVIS (arg_node)) = ARG_AVIS (arg_node); /* trigger renaming */

        VARDEC_AVIS (vardec) = tmp;
        AVIS_DECL (VARDEC_AVIS (vardec)) = vardec;
        AVIS_SUBSTUSSA (VARDEC_AVIS (vardec)) = NULL; /* no further renaming needed */
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    /*
     * remove AVIS_SSACOUNT since all N_ssacnt nodes (BLOCK_SSACOUNTER) will
     * be removed in USSAblock()
     */
    AVIS_SSACOUNT (ARG_AVIS (arg_node)) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSATvardec(node *arg_node, info *arg_info)
 *
 * description:
 *
 * 1. if a vardec is marked with SSAUNDOFLAG the corresponsing original vardec
 *    or arg is searched. if the original node has been deleted by optimizations
 *    the actual node is renamed to this orginal name (stored in SSACNT_BASEID).
 *    in the following tree traversal all corresponding identifiers are renamed
 *    back to their original name.
 *
 * 2. if a vardec is marked as SSAPHITARGET, the complete copy assignment must
 *    be removed to enable the fun2lac transformation. therfore the right-side
 *    identifier has to be renamed to the SSAPHITARGET identifier (or direct to
 *    its rename target (see 1).
 *
 * 3. after traversing all vardecs, check on back traversal for different
 *    renamings in AVIS_SUBST and AVIS_SUBSTUSSA. set the correct AVIS_SUBSTUSSA
 *    entry for each vardec (AVIS_SUBSTUSSA overrides AVIS_SUBST entry).
 *
 ******************************************************************************/
node *
USSATvardec (node *arg_node, info *arg_info)
{
    node *tmp;
    node *expr;

    DBUG_ENTER ("USSATvardec");

    /* 1. SSAUNDOFLAG */
    if ((AVIS_SSAUNDOFLAG (VARDEC_AVIS (arg_node)))
        && (strcmp (SSACNT_BASEID (AVIS_SSACOUNT (VARDEC_AVIS (arg_node))),
                    VARDEC_NAME (arg_node))
            != 0)) {
        /* artificial vardec with renamed ids -> search for original vardec/arg */

        /* first search in arg chain */
        tmp = INFO_USSA_ARGS (arg_info);
        while ((tmp != NULL) && (AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node)) == NULL)) {
            if (strcmp (SSACNT_BASEID (AVIS_SSACOUNT (VARDEC_AVIS (arg_node))),
                        ARG_NAME (tmp))
                == 0) {
                AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node)) = ARG_AVIS (tmp);
            }
            tmp = ARG_NEXT (tmp);
        }

        /* then search in vardec chain */
        tmp = BLOCK_VARDEC (INFO_USSA_TOPBLOCK (arg_info));
        while ((tmp != NULL) && (AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node)) == NULL)) {
            if (strcmp (SSACNT_BASEID (AVIS_SSACOUNT (VARDEC_AVIS (arg_node))),
                        VARDEC_NAME (tmp))
                == 0) {
                AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node)) = VARDEC_AVIS (tmp);
            }
            tmp = VARDEC_NEXT (tmp);
        }

        /*
         * no original vardec found (maybe removed by optimizations)!
         * so rename this vardec to original name.
         */
        if (AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node)) == NULL) {
            ILIBfree (VARDEC_NAME (arg_node));
            VARDEC_NAME (arg_node)
              = ILIBstringCopy (SSACNT_BASEID (AVIS_SSACOUNT (VARDEC_AVIS (arg_node))));

            /* force renaming of identifier of this vardec */
            AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node)) = VARDEC_AVIS (arg_node);

            DBUG_PRINT ("USSA", ("set %s as new baseid", VARDEC_NAME (arg_node)));
        }

        DBUG_PRINT ("USSA", ("-> rename %s to %s", VARDEC_NAME (arg_node),
                             SSACNT_BASEID (AVIS_SSACOUNT (VARDEC_AVIS (arg_node)))));

        DBUG_ASSERT ((AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node)) != NULL),
                     "no matching baseid found - no re-renaming possible.");
    }

    /* 2. handle SSAPHITARGET */
    if ((AVIS_SSAPHITARGET (VARDEC_AVIS (arg_node)) == PHIT_DO)
        || (AVIS_SSAPHITARGET (VARDEC_AVIS (arg_node)) == PHIT_WHILE)) {

        DBUG_ASSERT ((AVIS_SSAASSIGN (VARDEC_AVIS (arg_node)) != NULL),
                     "missing first assignment for phitarget");
        DBUG_ASSERT ((LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (VARDEC_AVIS (arg_node))))),
                     "missing let expr in first assignment");

        expr = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (VARDEC_AVIS (arg_node))));

        if (NODE_TYPE (expr) == N_id) {
            /*
             * ssa phi target copy assignment:
             * mark right side identifier for renming to left side identifer
             */
            if (AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node)) == NULL) {
                /* no further renaming of the this vardec */
                AVIS_SUBST (ID_AVIS (expr)) = VARDEC_AVIS (arg_node);
            } else {
                /*
                 * this vardec must be renamed, so everything that will be renamed
                 * to this vardec has to be renamed to the substussa vardec
                 */
                AVIS_SUBST (ID_AVIS (expr)) = AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node));
            }

            DBUG_PRINT ("USSA",
                        ("PHITARGET: rename %s -> %s (1)", (AVIS_NAME (ID_AVIS (expr))),
                         (AVIS_NAME (AVIS_SUBST (ID_AVIS (expr))))));
        }

        DBUG_ASSERT ((AVIS_SSAASSIGN2 (VARDEC_AVIS (arg_node)) != NULL),
                     "missing second assignment for phitarget");
        DBUG_ASSERT ((LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN2 (VARDEC_AVIS (arg_node))))),
                     "missing let expr in second assignment");

        expr = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN2 (VARDEC_AVIS (arg_node))));

        if (NODE_TYPE (expr) == N_id) {
            /*
             * ssa phi target copy assignment:
             * mark right side identifier for renming to left side identifer
             */
            if (AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node)) == NULL) {
                /* no further renaming of the this vardec */
                AVIS_SUBST (ID_AVIS (expr)) = VARDEC_AVIS (arg_node);
            } else {
                /*
                 * this vardec must be renamed, so everything that will be renamed
                 * to this vardec has to be renamed to the substussa vardec
                 */
                AVIS_SUBST (ID_AVIS (expr)) = AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node));
            }

            DBUG_PRINT ("USSA",
                        ("PHITARGET: rename %s -> %s (2)", AVIS_NAME (ID_AVIS (expr)),
                         AVIS_NAME (AVIS_SUBST (ID_AVIS (expr)))));
        }
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    /* 3. check for different renamings */
    if (AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node)) == NULL) {
        /* no first level renaming -> second level renaming will be done */
        AVIS_SUBSTUSSA (VARDEC_AVIS (arg_node)) = AVIS_SUBST (VARDEC_AVIS (arg_node));
    }

    /*
     * remove AVIS_SSACOUNT since all N_ssacnt nodes (BLOCK_SSACOUNTER) will
     * be removed in USSAblock()
     */
    AVIS_SSACOUNT (VARDEC_AVIS (arg_node)) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSATids(node *arg_ids, info *arg_info)
 *
 * description:
 *   re-renames artificial vardecs to their original name
 *
 ******************************************************************************/
node *
USSATids (node *arg_ids, info *arg_info)
{
    DBUG_ENTER ("USSATids");

    if (AVIS_SUBSTUSSA (IDS_AVIS (arg_ids)) != NULL) {
        DBUG_PRINT ("USSA", ("rename ids %s(" F_PTR ") in %s(" F_PTR ")",
                             AVIS_NAME (IDS_AVIS (arg_ids)), IDS_AVIS (arg_ids),
                             AVIS_NAME (AVIS_SUBSTUSSA (IDS_AVIS (arg_ids))),
                             AVIS_SUBSTUSSA (IDS_AVIS (arg_ids))));

        /* restore rename back to undo vardec */
        IDS_AVIS (arg_ids) = AVIS_SUBSTUSSA (IDS_AVIS (arg_ids));
    }

    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TRAVdo (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node *USSATlet(node *arg_node, info *arg_info)
 *
 * description:
 *   starts traversal in ids chain.
 *
 ******************************************************************************/
node *
USSATlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSATlet");
    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "N_let with empty EXPR attribute.");

    /* special handling for removal of phi copy tragets */
    if ((LET_IDS (arg_node) != NULL)
        && ((AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (arg_node))) == PHIT_DO)
            || (AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (arg_node))) == PHIT_WHILE))) {
        if (NODE_TYPE (LET_EXPR (arg_node)) == N_id) {
            /* simple identifier copy can be removed */
            INFO_USSA_OPASSIGN (arg_info) = OPASSIGN_REMOVE;
        } else {
            /* constants will be moved down behind conditional */
            INFO_USSA_OPASSIGN (arg_info) = OPASSIGN_MOVE;
        }
    } else {
        INFO_USSA_OPASSIGN (arg_info) = OPASSIGN_NOOP;

        if (LET_IDS (arg_node) != NULL) {
            /* there are some ids */
            LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
        }

        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    /* remove assignments like a = a after renaming */
    if ((LET_IDS (arg_node) != NULL) && (LET_EXPR (arg_node) != NULL)
        && (NODE_TYPE (LET_EXPR (arg_node)) == N_id)
        && (IDS_AVIS (LET_IDS (arg_node)) == ID_AVIS (LET_EXPR (arg_node)))) {
        INFO_USSA_OPASSIGN (arg_info) = OPASSIGN_REMOVE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSATwith(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
USSATwith (node *arg_node, info *arg_info)
{
    info *new_arg_info;

    DBUG_ENTER ("USSATwith");
    /* stack arg_info node, copy pointer to vardec/args lists */
    new_arg_info = MakeInfo ();
    INFO_USSA_TOPBLOCK (new_arg_info) = INFO_USSA_TOPBLOCK (arg_info);
    INFO_USSA_ARGS (new_arg_info) = INFO_USSA_ARGS (arg_info);

    /* now traverse all sons */
    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), new_arg_info);
    }

    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), new_arg_info);
    }

    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), new_arg_info);
    }

    /* free new_arg_info node */
    FreeInfo (new_arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* USSATfundef(node *arg_node, info *arg_info)
 *
 * description:
 *  traverses arg nodes and block in this order.
 *
 ******************************************************************************/
node *
USSATfundef (node *arg_node, info *arg_info)
{

    node *block, *assign, *cond;
    DBUG_ENTER ("USSATfundef");

    /* pre-traversal to find all cond nodes */

    if ((FUNDEF_ISCONDFUN (arg_node)) || (FUNDEF_ISDOFUN (arg_node))) {

        INFO_USSA_FUNDEF (arg_info) = arg_node;

        block = FUNDEF_BODY (arg_node);
        if (block != NULL) {
            assign = BLOCK_INSTR (block);
            if ((assign != NULL)
                && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assign))) == N_funcond)) {
                /* condition function without N_cond node found */
                cond = TBmakeCond (DUPdoDupTree (EXPRS_EXPR (
                                     FUNCOND_IF (LET_EXPR (ASSIGN_INSTR (assign))))),
                                   TBmakeBlock (TBmakeEmpty (), NULL),
                                   TBmakeBlock (TBmakeEmpty (), NULL));
                assign = TBmakeAssign (cond, assign);
                BLOCK_INSTR (block) = assign;
            }
            while (assign != NULL) {
                if (NODE_TYPE (ASSIGN_INSTR (assign)) == N_cond) {
                    INFO_USSA_PHIFUN (arg_info) = assign;
                    CondTransform (ASSIGN_INSTR (assign), arg_info);
                } else if ((NODE_TYPE (ASSIGN_INSTR (assign)) == N_let)
                           && (LET_IDS (ASSIGN_INSTR (assign)) != NULL)
                           && (IDS_AVIS (LET_IDS (ASSIGN_INSTR (assign))) != NULL)) {
                    AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (ASSIGN_INSTR (assign))))
                      = PHIT_NONE;
                }
                assign = ASSIGN_NEXT (assign);
            }
        }
    }

    UssaInitAvisFlags (arg_node);

    DBUG_PRINT ("USSA", ("\nrestoring names in function %s", FUNDEF_NAME (arg_node)));

    INFO_USSA_ARGS (arg_info) = FUNDEF_ARGS (arg_node);
    INFO_USSA_CONSTASSIGNS (arg_info) = NULL;

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* there is a block */

        /* save begin of vardec chain for later access */
        INFO_USSA_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* USSATblock(node *arg_node, info *arg_info)
 *
 * description:
 *  traverses vardec nodes and assignments in this order.
 *
 ******************************************************************************/
node *
USSATblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSATblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /*
         * there are some vardecs, check for artificial ones
         */
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (INFO_USSA_ARGS (arg_info) != NULL) {
        /* traverse args for renaming args */
        INFO_USSA_ARGS (arg_info) = TRAVdo (INFO_USSA_ARGS (arg_info), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* there is a block */
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) == NULL) {
        /* insert N_empty node in empty block */
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /* remove unused vardecs (marked for complete substitution) */
        BLOCK_VARDEC (arg_node) = UssaRemoveUnusedVardecs (BLOCK_VARDEC (arg_node));
    }

    if (BLOCK_SSACOUNTER (arg_node) != NULL) {
        /* remove all ssacnt nodes - they are not needed anymore */
        BLOCK_SSACOUNTER (arg_node) = FREEdoFreeTree (BLOCK_SSACOUNTER (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* USSATassign(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses instruction and removes/moves tagges assignments.
 *
 ******************************************************************************/
node *
USSATassign (node *arg_node, info *arg_info)
{
    int op;
    node *tmp;

    DBUG_ENTER ("USSATassign");
    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "missing instruction in assignment");

    INFO_USSA_OPASSIGN (arg_info) = OPASSIGN_NOOP;

    /* save pointer to current assignment node */
    /* needed to remove possible phi functions */

    INFO_USSA_PHIFUN (arg_info) = arg_node;

    /* traverse instruction */
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* save operation to perform on this assignment on bottom-up traveral */
    op = INFO_USSA_OPASSIGN (arg_info);
    INFO_USSA_OPASSIGN (arg_info) = OPASSIGN_NOOP;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        /* traverse next assignment */
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* insert moved constant assignments, if available */
    if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_return)
        && (INFO_USSA_CONSTASSIGNS (arg_info) != NULL)) {
        INFO_USSA_CONSTASSIGNS (arg_info)
          = TCappendAssign (INFO_USSA_CONSTASSIGNS (arg_info), arg_node);
        arg_node = INFO_USSA_CONSTASSIGNS (arg_info);
        INFO_USSA_CONSTASSIGNS (arg_info) = NULL;
    }

    /* in bottom up traversal remove marked assignments from chain */
    if ((op == OPASSIGN_REMOVE) || (op == OPASSIGN_MOVE)) {
        /* remove this assignment from assignment chain, return NEXT */
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);

        if (op == OPASSIGN_REMOVE) {
            /* remove whole assignment */
            tmp = FREEdoFreeNode (tmp);
        } else {
            /* move assignment to temp assignment chain */
            ASSIGN_NEXT (tmp) = NULL;
            INFO_USSA_CONSTASSIGNS (arg_info)
              = TCappendAssign (INFO_USSA_CONSTASSIGNS (arg_info), tmp);
        }
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *CondTransform(node *arg_node, info *arg_info)
 *
 * description:
 *   This method is executed at the begining of the traversal for each
 *   fundef-node for all conditionals which are childs of that fundef.
 *   After execution of this method all primitive phi functions are
 *   replaced by PHITARGET assignments inside the conditionals.
 *   This is the old representation of conditionals in SAC, which was
 *   used when UndoSSATransform was written.
 *   To reuse the old code, the old representation has to be used.
 *
 * example:
 *
 *   ...                                           ...
 *   if (<cond>){                                  if (<cond>){
 *     ...                                           ...
 *     a = ...;                                      a = ...;
 *     ...                                           ...
 *   }                                               result = a;
 *   else {                   is transformed to    }
 *     ...                                         else {
 *     b = ...;                                      ...
 *     ...                                           b = ...;
 *   }                                               ...
 *   result = phi(a, b);                             result = b;
 *   return(result);                               }
 *                                                 return(result);
 *
 ****************************************************************************/
static node *
CondTransform (node *arg_node, info *arg_info)
{

    node *then_node, *else_node;
    node *phifun, *old_assign, *then_assign, *else_assign, *old_cond;
    node *left_ids;
    node *return_node;

    DBUG_ENTER ("CondTransform");

    /* traverse to last assignment in then block*/
    then_node = BLOCK_INSTR (COND_THEN (arg_node));
    while ((then_node != NULL) && (NODE_TYPE (then_node) != N_empty)
           && (ASSIGN_NEXT (then_node) != NULL))
        then_node = ASSIGN_NEXT (then_node);

    /* traverse to last assignment in else block*/
    else_node = BLOCK_INSTR (COND_ELSE (arg_node));
    while ((else_node != NULL) && (NODE_TYPE (else_node) != N_empty)
           && (ASSIGN_NEXT (else_node) != NULL))
        else_node = ASSIGN_NEXT (else_node);

    /* first phi function*/
    phifun = ASSIGN_NEXT (INFO_USSA_PHIFUN (arg_info));

    /* find assignment node with return node*/
    return_node = phifun;
    while (NODE_TYPE (ASSIGN_INSTR (return_node)) != N_return)
        return_node = ASSIGN_NEXT (return_node);

    /* when phifun points to the return node, then every phi function was removed */
    while (NODE_TYPE (ASSIGN_INSTR (phifun)) != N_return) {

        /* check for expected node type */
        DBUG_ASSERT (NODE_TYPE (ASSIGN_INSTR (phifun)) == N_let, "let node expected");

        old_assign = phifun;

        /* traverse to next node in assignment chain*/
        phifun = ASSIGN_NEXT (phifun);

        ASSIGN_NEXT (old_assign) = NULL;

        /* pointer to ids */
        left_ids = LET_IDS (ASSIGN_INSTR (old_assign));

        /* separate funcond subtree from assignment */
        old_cond = LET_EXPR (ASSIGN_INSTR (old_assign));
        LET_EXPR (ASSIGN_INSTR (old_assign)) = NULL;

        /* reuse original phi-function assign node for then block */
        then_assign = old_assign;

        /* create let assign for else part with same left side as then part
           (right side from old funcond subtree) */
        else_assign
          = TCmakeAssignLet (IDS_AVIS (left_ids), EXPRS_EXPR (FUNCOND_ELSE (old_cond)));

        /* add correct id node from funcond subtree to assignment for then block */
        LET_EXPR (ASSIGN_INSTR (then_assign)) = EXPRS_EXPR (FUNCOND_THEN (old_cond));

        /* delete unused prf subtree */
        EXPRS_EXPR (FUNCOND_THEN (old_cond)) = NULL;
        EXPRS_EXPR (FUNCOND_ELSE (old_cond)) = NULL;
        FREEdoFreeTree (old_cond);

        /* append then_assignment to then block */
        if (then_node != NULL) {
            if (NODE_TYPE (then_node) != N_empty) {
                ASSIGN_NEXT (then_node) = then_assign;
                then_node = ASSIGN_NEXT (then_node);
            } else {
                FREEdoFreeTree (then_node);
                BLOCK_INSTR (COND_THEN (arg_node)) = then_assign;
                then_node = then_assign;
            }
        } else {
            BLOCK_INSTR (COND_THEN (arg_node)) = then_assign;
            then_node = then_assign;
        }

        /* append else_assignment to else block */
        if (else_node != NULL) {
            if (NODE_TYPE (else_node) != N_empty) {
                ASSIGN_NEXT (else_node) = else_assign;
                else_node = ASSIGN_NEXT (else_node);
            } else {
                FREEdoFreeTree (else_node);
                BLOCK_INSTR (COND_ELSE (arg_node)) = else_assign;
                else_node = else_assign;
            }
        } else {
            BLOCK_INSTR (COND_ELSE (arg_node)) = else_assign;
            else_node = else_assign;
        }

        /* update AVIS/VARDEC node */
        AVIS_SSAASSIGN (IDS_AVIS (left_ids)) = then_assign;
        AVIS_SSAASSIGN2 (IDS_AVIS (left_ids)) = else_assign;

        /* set flag if conditional is used inside a loop */

        if (FUNDEF_ISCONDFUN (INFO_USSA_FUNDEF (arg_info))) {
            AVIS_SSAPHITARGET (IDS_AVIS (left_ids)) = PHIT_COND;
        } else if (FUNDEF_ISDOFUN (INFO_USSA_FUNDEF (arg_info))) {
            AVIS_SSAPHITARGET (IDS_AVIS (left_ids)) = PHIT_DO;
        }
    }

    /* append return statement behind conditional */
    ASSIGN_NEXT (INFO_USSA_PHIFUN (arg_info)) = phifun;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* USSATdoUndoSsaTransform(node* module)
 *
 * description:
 *   Starts traversal of AST to restore original artificial identifier.
 *
 ******************************************************************************/
node *
USSATdoUndoSsaTransform (node *module)
{
    info *arg_info;

    DBUG_ENTER ("USSATdoUndoSSATransform");

    DBUG_PRINT ("OPT", ("starting UNDO ssa transformation"));

    arg_info = MakeInfo ();

    INFO_USSA_MODULE (arg_info) = module;

    TRAVpush (TR_ussat);
    module = TRAVdo (module, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    /* ast is no longer in ssaform */
    global.valid_ssaform = FALSE;

    /*
     * finally, we increase the ssaform_phase counter, in order to avoid
     * name clashes if the ast is transformed into ssa-form at a later stage
     * of the compiler again. cf. SSANewVardec in SSATransform.c !
     */
    global.ssaform_phase++;

    DBUG_RETURN (module);
}
