/*
 * $Log$
 * Revision 1.4  2000/06/14 10:46:04  mab
 * implemented APTvardec, APTarg
 * added dummies for APT ap, exprs, id, prf, fundef
 *
 * Revision 1.3  2000/06/08 11:14:14  mab
 * added functions for arg, vardec, array
 *
 * Revision 1.2  2000/05/31 16:16:58  mab
 * initial version
 *
 * Revision 1.1  2000/05/26 13:42:33  sbs
 * Initial revision
 *
 *
 */

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "DupTree.h"

#include "pad_info.h"
#include "pad_transform.h"

#include "my_debug.h"

/*****************************************************************************
 *
 * file:   pad_transform.c
 *
 * prefix: APT
 *
 * description:
 *
 *   This compiler module appplies array padding.
 *
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * function:
 *   void APtransform (node *arg_node)
 *
 * description:
 *   main function for applying array padding
 *
 *****************************************************************************/

void
APtransform (node *arg_node)
{
    node *arg_info;
    funtab *tmp_tab;

    DBUG_ENTER ("APtransform");

    DBUG_PRINT ("AP", ("Array Padding: applying transformation..."));

    tmp_tab = act_tab;
    act_tab = apt_tab;

    arg_info = MakeInfo ();

    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);
    act_tab = tmp_tab;

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   char *APTpadName(char* unpaddedName)
 *
 * description:
 *   generate new arg or vardec name with suffix '__PAD'
 *   !!! argument is set free within this function !!!
 *
 *****************************************************************************/

char *
APTpadName (char *unpaddedName)
{
    char *paddedName;

    DBUG_ENTER ("APTpadName");

    paddedName = (char *)malloc (strlen (unpaddedName) + 5 * sizeof (char));
    strcpy (paddedName, unpaddedName);
    paddedName[strlen (unpaddedName) + 0] = '_';
    paddedName[strlen (unpaddedName) + 1] = '_';
    paddedName[strlen (unpaddedName) + 2] = 'P';
    paddedName[strlen (unpaddedName) + 3] = 'A';
    paddedName[strlen (unpaddedName) + 4] = 'D';
    paddedName[strlen (unpaddedName) + 5] = '\0';

    free (unpaddedName);

    DBUG_RETURN (paddedName);
}

/*****************************************************************************
 *
 * function:
 *   void APTpadShpseg(int dim, shpseg* old, shpseg* new)
 *
 * description:
 *   copy padded shape to old shpseg (index-range: 0..dim-1)
 *
 *****************************************************************************/

void
APTpadShpseg (int dim, shpseg *old, shpseg *new)
{
    int i;
    DBUG_ENTER ("APTpadShpseg");

    i = 0;
    while (i < dim) {
        SHPSEG_SHAPE (old, i) = SHPSEG_SHAPE (new, i);
        i++;
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   node *APTarg(node *arg_node, node *arg_info)
 *
 * description:
 *   modify arg-node to padded name and shape
 *
 *****************************************************************************/

node *
APTarg (node *arg_node, node *arg_info)
{
    pad_info_t *shape_info;
    node *padded_arg;

    DBUG_ENTER ("APTarg");

    if (VARDEC_NAME (arg_node) != NULL)
        DBUG_PRINT ("AP", ("Check: %s", VARDEC_NAME (arg_node)));
    else
        DBUG_PRINT ("AP", ("Check: (NULL)"));

    shape_info = PInewShape (ARG_TYPE (arg_node));
    if (PIvalid (shape_info) == TRUE) {
        DBUG_PRINT ("AP", (" shape matched: modifying arg-node"));

        ARG_NAME (arg_node) = APTpadName (ARG_NAME (arg_node));
        APTpadShpseg (TYPES_DIM (ARG_TYPE (arg_node)), TYPES_SHPSEG (ARG_TYPE (arg_node)),
                      PIgetNewShape (shape_info));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*

extern node *MakeExprs(node* expr,
                       node* next);

#define EXPRS_EXPR(n) (n->node[0])
#define EXPRS_NEXT(n) (n->node[1])

 */

/*****************************************************************************
 *
 * function:
 *   node *APTvardec(node *arg_node, node *arg_info)
 *
 * description:
 *   duplicate vardec with matching shape -> create padded vardec
 *
 *****************************************************************************/

node *
APTvardec (node *arg_node, node *arg_info)
{

    pad_info_t *shape_info;
    node *padded_vardec;

    DBUG_ENTER ("APTvardec");

    DBUG_PRINT ("AP", ("Check: %s", VARDEC_NAME (arg_node)));

    shape_info = PInewShape (VARDEC_TYPE (arg_node));
    if (PIvalid (shape_info) == TRUE) {
        DBUG_PRINT ("AP", (" shape matched: modifying copy of vardec-node"));

        padded_vardec = DupNode (arg_node);
        VARDEC_NEXT (padded_vardec) = VARDEC_NEXT (arg_node);
        VARDEC_NAME (padded_vardec) = APTpadName (VARDEC_NAME (padded_vardec));
        APTpadShpseg (TYPES_DIM (VARDEC_TYPE (padded_vardec)),
                      TYPES_SHPSEG (VARDEC_TYPE (padded_vardec)),
                      PIgetNewShape (shape_info));

        VARDEC_NEXT (arg_node) = padded_vardec;
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTarray(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTarray");

    DBUG_PRINT ("AP", ("array-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTNwith(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/

node *
APTNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTNwith");

    DBUG_PRINT ("AP", ("Nwith-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTap(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/
node *
APTap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTap");

    DBUG_PRINT ("AP", ("ap-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTexprs(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/
node *
APTexprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTexprs");

    DBUG_PRINT ("AP", ("exprs-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTid(node *arg_node, node *arg_info)
 *
 * description:
 *  only a dummy for now
 *
 *****************************************************************************/
node *
APTid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTid");

    DBUG_PRINT ("AP", ("id-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTprf(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/
node *
APTprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTprf");

    DBUG_PRINT ("AP", ("prf-node detected"));

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *APTfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   only a dummy for now
 *
 *****************************************************************************/
node *
APTfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("APTfundef");

    DBUG_PRINT ("AP", ("fundef-node detected"));

    DBUG_RETURN (arg_node);
}
