/*      $Id$
 *
 * $Log$
 * Revision 1.8  1998/03/22 18:13:34  srs
 * splitted file to
 * - WLT.c tranformations of WLs
 * - WLI.c gather information about WLs
 * - WLF.c fold WLs
 *
 * Revision 1.7  1998/03/18 08:33:07  srs
 * first running version of WLI
 *
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
 This file organizes the withloop folding and makes some basic functions
 available.

 Withloop folding is done in 3 phases:
 1) WLT transforms WLs to apply the following phases
 1) WLI gathers information about the WLs
 2) WLF finds and folds suitable WLs.
 Assumption: We assume that all generators of a WL have the same
 shape.  Furthermore we assume that, if an N_Ncode is referenced by
 more than one generator, all these generators' indexes (vector and
 scalar) have the same names. This 'same name assumption' can even be
 expanded to all generators a WL has. This is true as a consequence of
 the folding mechanism we apply (induction): At the beginning, all WL
 have only one generator and it is obviously true. When another WL's
 body (foreign body) is folded into a this WL's body, the body is
 copied inclusive the generator indices. The foreign indices of the
 other WL are transformed (based on the origial indicies) to temp
 variables, which are applied to the substituted foreign body.

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "types.h"
#include "internal_lib.h"
#include "free.h"
#include "print.h"
#include "DupTree.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "optimize.h"
#include "ConstantFolding.h"
#include "WithloopFolding.h"
#include "WLT.h"
#include "WLI.h"
#include "WLF.h"

/******************************************************************************
 *
 * function:
 *   void DbugIndexInfo(index info *iinfo)
 *
 * description:
 *   prints history of iinfo.
 *
 *
 ******************************************************************************/

void
DbugIndexInfo (index_info *iinfo)
{
    int i, sel;
    index_info *tmpii;

    DBUG_ENTER ("DbugIndexInfo");

    if (!iinfo)
        printf ("\nNULL\n");
    else if (iinfo->vector) {
        printf ("\nVECTOR shape [%d]:\n", iinfo->vector);
        for (i = 0; i < iinfo->vector; i++) {
            printf ("---%d---\n", i);

            if (!iinfo->permutation[i]) { /* constant */
                printf ("  constant %d\n", iinfo->const_arg[i]);
                continue;
            }

            printf ("  base %d\n", iinfo->permutation[i]);
            tmpii = iinfo;
            while (tmpii) {
                sel = tmpii->vector ? i : 0;
                if (tmpii->arg_no) {
                    if (1 == tmpii->arg_no)
                        printf ("   %d%s. ", tmpii->const_arg[sel],
                                prf_string[tmpii->prf]);
                    else
                        printf ("   .%s%d ", prf_string[tmpii->prf],
                                tmpii->const_arg[sel]);
                } else
                    printf ("   no prf ");
                printf ("(p:%d, v:%d)\n", tmpii->permutation[sel], tmpii->vector);
                tmpii = tmpii->last[sel];
            }
        }
    } else {
        printf ("\nSCALAR:\n");
        printf ("  base %d\n", iinfo->permutation[0]);
        tmpii = iinfo;
        sel = 0;
        if (tmpii->arg_no) {
            if (1 == tmpii->arg_no)
                printf ("   %d%s. ", tmpii->const_arg[sel], prf_string[tmpii->prf]);
            else
                printf ("   %s%d. ", prf_string[tmpii->prf], tmpii->const_arg[sel]);
            printf ("(p:%d, v:%d)\n", tmpii->permutation[sel], tmpii->vector);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   index_info *CreateIndex(int vector)
 *
 * description:
 *   create an incarnation of INDEX_INFO.
 *
 * remark:
 *   the argument 'vector' behaves like the component 'vector' of INDEX_INFO.
 *
 * reference:
 *   FREE_INDEX
 *
 ******************************************************************************/

index_info *
CreateIndex (int vector)
{
    index_info *pindex;
    DBUG_ENTER ("CreateInfoInfo");

    pindex = Malloc (sizeof (index_info));
    pindex->vector = vector;

    if (!vector)
        vector = 1;
    pindex->permutation = Malloc (sizeof (int) * vector);
    pindex->last = Malloc (sizeof (index_info *) * vector);
    pindex->const_arg = Malloc (sizeof (int) * vector);

    pindex->arg_no = 0;

    DBUG_RETURN (pindex);
}

/******************************************************************************
 *
 * function:
 *   index_info *DuplicateIndexInfo(index_info *iinfo)
 *
 * description:
 *   duplicates struct
 *
 *
 ******************************************************************************/

index_info *
DuplicateIndexInfo (index_info *iinfo)
{
    index_info *new;
    int i, to;
    DBUG_ENTER ("DuplicateIndexInfo");
    DBUG_ASSERT (iinfo, ("parameter NULL"));

    new = CreateIndex (iinfo->vector);

    to = iinfo->vector ? iinfo->vector : 1;
    for (i = 0; i < to; i++) {
        new->permutation[i] = iinfo->permutation[i];
        new->last[i] = iinfo->last[i];
        new->const_arg[i] = iinfo->const_arg[i];
    }

    new->prf = iinfo->prf;
    new->arg_no = iinfo->arg_no;

    DBUG_RETURN (new);
}

/******************************************************************************
 *
 * function:
 *   node *ValicLocalId(node *idn)
 *
 * description:
 *   returns pointer to index_info if Id (idn) is a valid variable within
 *   the WL body (index vars are excluded). Returns NULL otherwise.
 *
 ******************************************************************************/

index_info *
ValidLocalId (node *idn)
{
    index_info *iinfo;

    DBUG_ENTER ("ValicLocalId");
    DBUG_ASSERT (N_id == NODE_TYPE (idn), ("not an id node"));

    idn = MRD (ID_VARNO (idn));
    if (idn)
        iinfo = INDEX (idn);
    else
        iinfo = NULL;

    DBUG_RETURN (iinfo);
}

/******************************************************************************
 *
 * function:
 *   int LocateIndexVar(node *idn, node* wln)
 *
 * description:
 *   Searches for the Id (idn) in the WL generator (index var).
 *   The N_with node has to be available to find the index vars.
 *
 * return:
 *   -1: Id is the index vector
 *    0: Id not found
 *    x with x gt 0: Id is the x'th scalar index variable.
 *
 * remark:
 *   we exploit here that the index variables of all N_Nwithid nodes in
 *   one WL have the same names.
 *
 ******************************************************************************/

int
LocateIndexVar (node *idn, node *wln)
{
    ids *_ids;
    int result = 0, i;

    DBUG_ENTER ("LocateIndexVar");
    DBUG_ASSERT (N_Nwith == NODE_TYPE (wln), ("wln is not N_Nwith node"));

    wln = NPART_WITHID (NWITH_PART (wln));
    _ids = NWITHID_VEC (wln);

    if (!strcmp (IDS_NAME (_ids), ID_NAME (idn)))
        result = -1;

    i = 1;
    _ids = NWITHID_IDS (wln);
    while (_ids && !result) {
        if (!strcmp (IDS_NAME (_ids), ID_NAME (idn)))
            result = i;
        i++;
        _ids = IDS_NEXT (_ids);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *WithloopFolding(node *arg_node, node* arg_info)
 *
 * description:
 *   starting point for the withloop folding.
 *
 *
 ******************************************************************************/

node *
WithloopFolding (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFWithloopFolding");

    DBUG_ASSERT (!arg_info, ("at the beginning of WLF: arg_info != NULL"));
    arg_info = MakeInfo ();

    /* WLT traversal: transform WLs */
    DBUG_EXECUTE ("WLx", NOTE_OPTIMIZER_PHASE ("  WLT"););
    act_tab = wlt_tab;
    arg_node = Trav (arg_node, arg_info);

    /* rebuild mask which is necessary because of the transformations in WLT. */
    DBUG_EXECUTE ("WLx", NOTE_OPTIMIZER_PHASE ("  GenerateMasks"););
    arg_node = GenerateMasks (arg_node, NULL);

    /* WLI traversal: search information */
    DBUG_EXECUTE ("WLx", NOTE_OPTIMIZER_PHASE ("  WLI"););
    act_tab = wli_tab;
    arg_node = Trav (arg_node, arg_info);

    /* WLF traversal: fold WLs */
    DBUG_EXECUTE ("WLx", NOTE_OPTIMIZER_PHASE ("  WLF"););
    act_tab = wlf_tab;
    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);

    DBUG_RETURN (arg_node);
}
