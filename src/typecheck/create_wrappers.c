/*
 *
 * $Log$
 * Revision 1.4  2002/03/12 15:13:32  sbs
 * CRTWRPxxxx traversal function added.
 *
 * Revision 1.3  2002/03/05 15:51:17  sbs
 * *** empty log message ***
 *
 * Revision 1.2  2002/03/05 15:40:40  sbs
 * CRTWRP traversal embedded.
 *
 * Revision 1.1  2002/03/05 13:59:27  sbs
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "internal_lib.h"
#include "traverse.h"
#include "globals.h"

#include "user_types.h"
#include "new_types.h"

#include "create_wrappers.h"

#define INFO_CRTWRP_WRAPPERS(n) (n->node[0])

/******************************************************************************
 *
 * function:
 *    node *CreateWrappers(node *arg_node)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CreateWrappers (node *arg_node)
{
    funtab *tmp_tab;
    node *info_node;
    ntype *a, *b, *c, *d, *e;
    tvar *aa, *bb, *cc;
    bool r1, r2, r3, r4, r5;

    DBUG_ENTER ("CreateWrappers");

    tmp_tab = act_tab;
    act_tab = crtwrp_tab;

    info_node = MakeInfo ();
#if 0
  arg_node = Trav( arg_node, info_node);
#else
    a = TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (2, 3, 4));
    b = TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (1, 5));
    c = TYMakeAUD (TYMakeSimpleType (T_int));
    d = TYMakeFunType (a, c, NULL);
    e = TYMakeFunType (b, c, NULL);
    e = TYMakeOverloadedFunType (d, e);

    printf ("%s \n", TYType2String (a, TRUE, 0));
    printf ("%s \n", TYType2String (b, TRUE, 0));
    printf ("%s \n", TYType2String (c, TRUE, 0));
    printf ("%s \n", TYType2String (d, TRUE, 0));
    printf ("%s \n", TYType2String (e, TRUE, 0));

    aa = SSIMakeVariable ();
    bb = SSIMakeVariable ();
    cc = SSIMakeVariable ();

    r1 = SSINewMax (aa, a);
    if (!r1)
        printf ("Error for SSINewMax( aa, a)\n");

    r2 = SSINewMax (bb, b);
    if (!r2)
        printf ("Error for SSINewMax( bb, a)\n");

    r3 = SSINewMax (cc, c);
    if (!r3)
        printf ("Error for SSINewMax( cc, a)\n");

    r4 = SSINewRel (bb, aa);
    if (!r4)
        printf ("Error for SSINewRel( bb, aa)\n");

    printf ("%s \n", SSIVariable2DebugString (aa));
    printf ("%s \n", SSIVariable2DebugString (bb));
    printf ("%s \n", SSIVariable2DebugString (cc));

    r5 = SSINewRel (cc, bb);
    if (!r5)
        printf ("Error for SSINewRel( cc, bb)\n");

    printf ("%s \n", SSIVariable2DebugString (aa));
    printf ("%s \n", SSIVariable2DebugString (bb));
    printf ("%s \n", SSIVariable2DebugString (cc));
#endif
    info_node = FreeNode (info_node);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *FindWrapper(char *name, int num_args, node *wrappers)
 *
 * description:
 *
 *
 ******************************************************************************/

static node *
FindWrapper (char *name, int num_args, node *wrappers)
{
    int found = FALSE;
    int last_is_dots = FALSE;
    int num_parms;

    DBUG_ENTER ("FindWrapper");

    DBUG_PRINT ("CRTWRP", ("Searching for %s %d args", name, num_args));

    while (wrappers && !found) {
        num_parms = CountArgs (FUNDEF_ARGS (wrappers));
        last_is_dots = HasDotArgs (FUNDEF_ARGS (wrappers));
        DBUG_PRINT ("CRTWRP",
                    (" ... checking %s %d args", FUNDEF_NAME (wrappers), num_parms));
        if ((strcmp (name, FUNDEF_NAME (wrappers)) == 0)
            && ((num_parms == num_args) || (last_is_dots && (num_parms <= num_args)))) {
            found = TRUE;
        } else {
            wrappers = FUNDEF_NEXT (wrappers);
        }
    }

    DBUG_RETURN (wrappers);
}

/******************************************************************************
 *
 * function:
 *    node *CreateWrapperFor(node *fundef)
 *
 * description:
 *
 *
 ******************************************************************************/

static node *
CreateWrapperFor (node *fundef)
{
    node *body, *wrapper, *args;
    types *rettypes;

    DBUG_ENTER ("CreateWrapperFor");

    body = FUNDEF_BODY (fundef);
    FUNDEF_BODY (fundef) = NULL;
    wrapper = DupNode (fundef);
    FUNDEF_BODY (fundef) = body;

    /*
     * marking the wrapper function:
     */
    FUNDEF_STATUS (wrapper) = ST_wrapperfun;

    /*
     * setting the wrapper function's return types to _unknown_[*] :
     */
    rettypes = FUNDEF_TYPES (wrapper);
    while (rettypes) {
        TYPES_DIM (rettypes) = ARRAY_OR_SCALAR;
        TYPES_BASETYPE (rettypes) = T_unknown;
        rettypes = TYPES_NEXT (rettypes);
    }

    /*
     * setting the wrapper function's parameter types to _unknown_[*]
     * unless their basetype turns out to be T_dots :
     */
    args = FUNDEF_ARGS (wrapper);
    while (args) {
        if (TYPES_BASETYPE (ARG_TYPE (args)) != T_dots) {
            TYPES_DIM (ARG_TYPE (args)) = ARRAY_OR_SCALAR;
            TYPES_BASETYPE (ARG_TYPE (args)) = T_unknown;
        }
        args = ARG_NEXT (args);
    }

    DBUG_RETURN (wrapper);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPmodul(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPmodul (node *arg_node, node *arg_info)
{
    node *wrappers;

    DBUG_ENTER ("CRTWRPmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }
    wrappers = INFO_CRTWRP_WRAPPERS (arg_info);
    MODUL_FUNS (arg_node) = AppendFundef (wrappers, MODUL_FUNS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPfundef (node *arg_node, node *arg_info)
{
    ntype *funtype;
    node *wrapper;

    DBUG_ENTER ("CRTWRPfundef");

    wrapper = FindWrapper (FUNDEF_NAME (arg_node), CountArgs (FUNDEF_ARGS (arg_node)),
                           INFO_CRTWRP_WRAPPERS (arg_info));
    if (wrapper == NULL) {
        wrapper = CreateWrapperFor (arg_node);
        FUNDEF_NEXT (wrapper) = INFO_CRTWRP_WRAPPERS (arg_info);
        INFO_CRTWRP_WRAPPERS (arg_info) = wrapper;
    }
#if 0
  funtype = CreateFuntype( arg_node);
  InsertFuntype( funtype, wrapper);
#endif

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * Now, we do have wrappers for all fundefs!
     * On our way back up, we traverse the body in order to insert the backrefs
     * to the appropriate wrappers, AND we infer the "down-projections".
     */

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /* Infer Down-Projections still missing */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPap(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPap (node *arg_node, node *arg_info)
{
    int num_args;
    node *wrapper;

    DBUG_ENTER ("CRTWRPap");

    num_args = CountExprs (AP_ARGS (arg_node));
    wrapper = FindWrapper (AP_NAME (arg_node), num_args, INFO_CRTWRP_WRAPPERS (arg_info));

    if (wrapper == NULL) {
        ABORT (NODE_LINE (arg_node),
               ("No definition found for a function \"%s\" that expects %i argument(s)",
                AP_NAME (arg_node), num_args));
    } else {
        AP_FUNDEF (arg_node) = wrapper;
    }

    DBUG_RETURN (arg_node);
}
