/*
 *
 * $Log$
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

#include "types.h"
#include "tree_basic.h"
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

static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *USSAids (ids *arg_ids, node *arg_info);

/* helper functions for local use */
static void USSAInitAvisFlags (node *fundef);
static node *USSARemoveUnusedVardecs (node *vardecs);

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
USSAInitAvisFlags (node *fundef)
{
    node *tmp;

    DBUG_ENTER ("USSAInitAvisFlags");

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
 *   node *USSARemoveUnusedVardecs(node *vardecs)
 *
 * description:
 *   free all vardecs marked for complete substitution.
 *
 ******************************************************************************/
static node *
USSARemoveUnusedVardecs (node *vardecs)
{
    node *avis;

    DBUG_ENTER ("USSARemoveUnusedVardecs");

    /* traverse rest of chain */
    if (VARDEC_NEXT (vardecs) != NULL) {
        VARDEC_NEXT (vardecs) = USSARemoveUnusedVardecs (VARDEC_NEXT (vardecs));
    }

    avis = VARDEC_AVIS (vardecs);

    /* check vardec for removal */
    if ((AVIS_SUBSTUSSA (avis) != NULL) && (AVIS_SUBSTUSSA (avis) != avis)) {
        DBUG_PRINT ("USSA", ("remove unused vardec %s", VARDEC_NAME (vardecs)));
        vardecs = FreeNode (vardecs);
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *  node *USSAarg(node *arg_node, node *arg_info)
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
USSAarg (node *arg_node, node *arg_info)
{
    node *tmp;
    node *vardec;

    DBUG_ENTER ("USSAarg");

    if (AVIS_SUBST (ARG_AVIS (arg_node)) != NULL) {
        DBUG_PRINT ("USSA", ("using arg %s instead of vardec %s", ARG_NAME (arg_node),
                             VARDEC_OR_ARG_NAME (
                               AVIS_VARDECORARG (AVIS_SUBST (ARG_AVIS (arg_node))))));

        /* use avis of vardec node as avis of arg node */
        vardec = AVIS_VARDECORARG (AVIS_SUBST (ARG_AVIS (arg_node)));
        tmp = ARG_AVIS (arg_node);

        ARG_AVIS (arg_node) = VARDEC_AVIS (vardec);
        AVIS_VARDECORARG (ARG_AVIS (arg_node)) = arg_node;
        AVIS_SUBSTUSSA (ARG_AVIS (arg_node)) = ARG_AVIS (arg_node); /* trigger renaming */

        VARDEC_AVIS (vardec) = tmp;
        AVIS_VARDECORARG (VARDEC_AVIS (vardec)) = vardec;
        AVIS_SUBSTUSSA (VARDEC_AVIS (vardec)) = NULL; /* no further renaming needed */
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
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
 *   node *USSAvardec(node *arg_node, node *arg_info)
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
USSAvardec (node *arg_node, node *arg_info)
{
    node *tmp;
    node *expr;

    DBUG_ENTER ("USSAvardec");
    DBUG_PRINT ("USSA",
                ("name %s: STATUS %s, ATTRIB %s, AVIS " F_PTR "\n",
                 VARDEC_NAME (arg_node), mdb_statustype[VARDEC_STATUS (arg_node)],
                 mdb_statustype[VARDEC_ATTRIB (arg_node)], VARDEC_AVIS (arg_node)));

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
            Free (VARDEC_NAME (arg_node));
            VARDEC_NAME (arg_node)
              = StringCopy (SSACNT_BASEID (AVIS_SSACOUNT (VARDEC_AVIS (arg_node))));

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

            DBUG_PRINT ("USSA", ("PHITARGET: rename %s -> %s (1)",
                                 VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (expr))),
                                 VARDEC_OR_ARG_NAME (
                                   AVIS_VARDECORARG (AVIS_SUBST (ID_AVIS (expr))))));
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

            DBUG_PRINT ("USSA", ("PHITARGET: rename %s -> %s (2)",
                                 VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (expr))),
                                 VARDEC_OR_ARG_NAME (
                                   AVIS_VARDECORARG (AVIS_SUBST (ID_AVIS (expr))))));
        }
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
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
 *   node *USSAid(node *arg_node, node *arg_info)
 *
 * description:
 *   checks for consistent back reference from N_id node to N_arg or N_vardec
 *   node together with back reference to N_avis node. Here implemented in
 *   USSAids.
 *
 ******************************************************************************/
node *
USSAid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("USSAid");

    DBUG_ASSERT ((ID_IDS (arg_node) != NULL), "missing IDS in N_id!");

    ID_IDS (arg_node) = TravIDS (ID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSAlet(node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal in ids chain.
 *
 ******************************************************************************/
node *
USSAlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("USSAlet");
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
            LET_IDS (arg_node) = TravIDS (LET_IDS (arg_node), arg_info);
        }

        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
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
 *   node *USSANwithid(node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal for ids chains in Nwithid nodes.
 *
 ******************************************************************************/
node *
USSANwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("USSANwithid");

    if (NWITHID_VEC (arg_node) != NULL) {
        NWITHID_VEC (arg_node) = TravIDS (NWITHID_VEC (arg_node), arg_info);
    }

    if (NWITHID_IDS (arg_node) != NULL) {
        NWITHID_IDS (arg_node) = TravIDS (NWITHID_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSANcode(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses code blocks of with loop and inserts unique result name for
 *   multigenerator fold-withloops.
 *
 ******************************************************************************/
node *
USSANcode (node *arg_node, node *arg_info)
{
    node *src_id;

    DBUG_ENTER ("USSANcode");

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    if (INFO_USSA_FOLDTARGET (arg_info) != NULL) {
        /* create source id node */
        src_id = MakeId_Copy (
          VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (ID_AVIS (NCODE_CEXPR (arg_node)))));
        ID_VARDEC (src_id) = AVIS_VARDECORARG (ID_AVIS (NCODE_CEXPR (arg_node)));
        ID_AVIS (src_id) = ID_AVIS (NCODE_CEXPR (arg_node));

        /*
         * append copy assignment: <fold-target> = cexprvar;
         * to block
         */
        BLOCK_INSTR (NCODE_CBLOCK (arg_node))
          = AppendAssign (BLOCK_INSTR (NCODE_CBLOCK (arg_node)),
                          MakeAssignLet (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (
                                           INFO_USSA_FOLDTARGET (arg_info)))),
                                         AVIS_VARDECORARG (
                                           INFO_USSA_FOLDTARGET (arg_info)),
                                         src_id));

        /* set new fold target as cexpr */
        DBUG_PRINT ("USSA",
                    ("set new fold target %s",
                     VARDEC_NAME (AVIS_VARDECORARG (INFO_USSA_FOLDTARGET (arg_info)))));

        ID_VARDEC (NCODE_CEXPR (arg_node))
          = AVIS_VARDECORARG (INFO_USSA_FOLDTARGET (arg_info));
        ID_AVIS (NCODE_CEXPR (arg_node)) = INFO_USSA_FOLDTARGET (arg_info);
#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        Free (ID_NAME (NCODE_CEXPR (arg_node)));
        ID_NAME (NCODE_CEXPR (arg_node))
          = StringCopy (VARDEC_NAME (AVIS_VARDECORARG (INFO_USSA_FOLDTARGET (arg_info))));
#endif
    } else {
        /* do standard traversal */
        if (NCODE_CEXPR (arg_node) != NULL) {
            NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
        }
    }

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSANwith(node *arg_node, node *arg_info)
 *
 * description:
 *   if this is a fold-withloop. we have to create a new unique result valiable
 *   for all results of a multigenerator withloop. The renaming and inserting
 *   of the necessary copy assignment is done by USSANcode.
 *
 ******************************************************************************/
node *
USSANwith (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("USSANwith");
    /* stack arg_info node, copy pointer to vardec/args lists */
    new_arg_info = MakeInfo ();
    INFO_USSA_TOPBLOCK (new_arg_info) = INFO_USSA_TOPBLOCK (arg_info);
    INFO_USSA_ARGS (new_arg_info) = INFO_USSA_ARGS (arg_info);

    /*
     * first check for fold-withloop with at least two code segments
     * (first code has a next attribute set) that needs a new
     * unique target variable
     */
    if ((NWITH_IS_FOLD (arg_node)) && (NWITH_CODE (arg_node) != NULL)
        && (NCODE_NEXT (NWITH_CODE (arg_node)) != NULL)) {
        DBUG_ASSERT ((NCODE_CEXPR (NWITH_CODE (arg_node)) != NULL),
                     "fold-withloop without target expression");
        DBUG_ASSERT ((NODE_TYPE (NCODE_CEXPR (NWITH_CODE (arg_node))) == N_id),
                     "fold-withloop without target variable");

        /* make new unique vardec as fold target and append it to vardec chain */
        BLOCK_VARDEC (INFO_USSA_TOPBLOCK (new_arg_info))
          = MakeVardec (TmpVar (),
                        DupAllTypes (VARDEC_OR_ARG_TYPE (
                          ID_VARDEC (NCODE_CEXPR (NWITH_CODE (arg_node))))),
                        BLOCK_VARDEC (INFO_USSA_TOPBLOCK (arg_info)));

        DBUG_PRINT ("USSA", ("create unique fold target %s",
                             VARDEC_NAME (BLOCK_VARDEC (INFO_USSA_TOPBLOCK (arg_info)))));

        /* set as new fold-target (will be inserted by USSANcode */
        INFO_USSA_FOLDTARGET (new_arg_info)
          = VARDEC_AVIS (BLOCK_VARDEC (INFO_USSA_TOPBLOCK (arg_info)));
    } else {
        /* no new target needed */
        INFO_USSA_FOLDTARGET (new_arg_info) = NULL;
    }

    /* now traverse all sons */
    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), new_arg_info);
    }

    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), new_arg_info);
    }

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), new_arg_info);
    }

    /* free new_arg_info node */
    FreeNode (new_arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* USSAfundef(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses arg nodes and block in this order.
 *
 ******************************************************************************/
node *
USSAfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("USSAfundef");

    USSAInitAvisFlags (arg_node);

    DBUG_PRINT ("USSA", ("\nrestoring names in function %s", FUNDEF_NAME (arg_node)));

    INFO_USSA_ARGS (arg_info) = FUNDEF_ARGS (arg_node);
    INFO_USSA_CONSTASSIGNS (arg_info) = NULL;

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* there is a block */

        /* save begin of vardec chain for later access */
        INFO_USSA_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* USSAblock(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses vardec nodes and assignments in this order.
 *
 ******************************************************************************/
node *
USSAblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("USSAblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /*
         * there are some vardecs, check for artificial ones
         */
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (INFO_USSA_ARGS (arg_info) != NULL) {
        /* traverse args for renaming args */
        INFO_USSA_ARGS (arg_info) = Trav (INFO_USSA_ARGS (arg_info), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* there is a block */
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) == NULL) {
        /* insert N_empty node in empty block */
        BLOCK_INSTR (arg_node) = MakeEmpty ();
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /* remove unused vardecs (marked for complete substitution) */
        BLOCK_VARDEC (arg_node) = USSARemoveUnusedVardecs (BLOCK_VARDEC (arg_node));
    }

    if (BLOCK_SSACOUNTER (arg_node) != NULL) {
        /* remove all ssacnt nodes - they are not needed anymore */
        BLOCK_SSACOUNTER (arg_node) = FreeTree (BLOCK_SSACOUNTER (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* USSAassign(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses instruction and removes/moves tagges assignments.
 *
 ******************************************************************************/
node *
USSAassign (node *arg_node, node *arg_info)
{
    int op;
    node *tmp;

    DBUG_ENTER ("USSAassign");
    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "missing instruction in assignment");

    INFO_USSA_OPASSIGN (arg_info) = OPASSIGN_NOOP;

    /* traverse instruction */
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /* save operation to perform on this assignment on bottom-up traveral */
    op = INFO_USSA_OPASSIGN (arg_info);
    INFO_USSA_OPASSIGN (arg_info) = OPASSIGN_NOOP;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        /* traverse next assignment */
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* insert moved constant assignments, if available */
    if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_return)
        && (INFO_USSA_CONSTASSIGNS (arg_info) != NULL)) {
        INFO_USSA_CONSTASSIGNS (arg_info)
          = AppendAssign (INFO_USSA_CONSTASSIGNS (arg_info), arg_node);
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
            tmp = FreeNode (tmp);
        } else {
            /* move assignment to temp assignment chain */
            ASSIGN_NEXT (tmp) = NULL;
            INFO_USSA_CONSTASSIGNS (arg_info)
              = AppendAssign (INFO_USSA_CONSTASSIGNS (arg_info), tmp);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSAids(node *arg_ids, node *arg_info)
 *
 * description:
 *   re-renames artificial vardecs to their original name
 *
 ******************************************************************************/
static ids *
USSAids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("USSAids");

    if (AVIS_SUBSTUSSA (IDS_AVIS (arg_ids)) != NULL) {
        DBUG_PRINT ("USSA", ("rename ids %s(" F_PTR ") in %s(" F_PTR ")",
                             VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids))),
                             IDS_AVIS (arg_ids),
                             VARDEC_OR_ARG_NAME (
                               AVIS_VARDECORARG (AVIS_SUBSTUSSA (IDS_AVIS (arg_ids)))),
                             AVIS_SUBSTUSSA (IDS_AVIS (arg_ids))));

        /* restore rename back to undo vardec */
        IDS_AVIS (arg_ids) = AVIS_SUBSTUSSA (IDS_AVIS (arg_ids));
        IDS_VARDEC (arg_ids) = AVIS_VARDECORARG (IDS_AVIS (arg_ids));

#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        Free (IDS_NAME (arg_ids));
        IDS_NAME (arg_ids) = StringCopy (VARDEC_OR_ARG_NAME (IDS_VARDEC (arg_ids)));
#endif
    }

    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   ids *TravIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *   here used for ids.
 *
 ******************************************************************************/
static ids *
TravIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = USSAids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* UndoSSATransform(node* modul)
 *
 * description:
 *   Starts traversal of AST to restore original artificial identifier.
 *
 ******************************************************************************/
node *
UndoSSATransform (node *modul)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("UndoSSATransform");

    DBUG_PRINT ("OPT", ("starting UNDO ssa transformation"));

    arg_info = MakeInfo ();

    INFO_USSA_MODUL (arg_info) = modul;

    old_tab = act_tab;
    act_tab = undossa_tab;

    modul = Trav (modul, arg_info);

    act_tab = old_tab;

    arg_info = FreeTree (arg_info);

    /* ast is no longer in ssaform */
    valid_ssaform = FALSE;

    DBUG_RETURN (modul);
}
