/*
 *
 * $Log$
 * Revision 1.3  2002/07/24 10:30:17  sah
 * first support for multidimensional
 * selects added. '...' not supported yet.
 *
 * Revision 1.1  2002/07/09 12:54:25  sbs
 * Initial revision
 *
 *
 */

#include "handle_dots.h"
#include "traverse.h"
#include "dbug.h"
#include "free.h"
#include "Error.h"
#include "DupTree.h"
#include "internal_lib.h"

typedef struct DOTLIST {
    int no;               /* number of dot counted from left */
    int position;         /* position of dot within selection */
    int dottype;          /* type of dot; 1:'.' 3: '...' */
    struct DOTLIST *next; /* for building up a list ;) */
    struct DOTLIST *prev;
} dotlist;

typedef struct DOTINFO {
    dotlist *left;  /* left end of dotlist */
    dotlist *right; /* right end */
    int dotcnt;     /* amount of dots found */
    int tripledot;  /* dotno of tripledot, 0 iff none found */
    int triplepos;  /* position of tripledot, 0 iff none found */
    int selcnt;     /* amount of selectors at all */
} dotinfo;

void
BuildDotList (node *tree, dotinfo *info)
{
    DBUG_ENTER ("BuildDotList");

    while (tree != NULL) {
        node *handle = EXPRS_EXPR (tree);
        info->selcnt++;

        if (NODE_TYPE (handle) == N_dot) {
            dotlist *newdot = Malloc (sizeof (dotlist));

            info->dotcnt++;
            newdot->position = info->selcnt;
            newdot->no = info->dotcnt;
            newdot->dottype = DOT_NUM (handle);
            newdot->next = NULL;

            if (info->right == NULL) {
                newdot->prev = NULL;
                info->left = newdot;
                info->right = newdot;
            } else {
                newdot->prev = info->right;
                newdot->prev->next = newdot;
                info->right = newdot;
            }
            if (newdot->dottype == 3) {
                if (info->tripledot == 0) {
                    info->tripledot = info->dotcnt;
                    info->triplepos = info->selcnt;
                } else {
                    /* there are multiple occurences of '...' */
                    ERROR (linenum, ("Multiple occurences of ... are not allowed in a "
                                     "single select statement."));
                }
            }
        }

        tree = EXPRS_NEXT (tree);
    }
}

dotinfo *
MakeDotInfo (node *args)
{
    dotinfo *result;

    DBUG_ENTER ("MakeDotInfo");

    result = (dotinfo *)Malloc (sizeof (dotinfo));

    result->left = NULL;
    result->right = NULL;
    result->dotcnt = 0;
    result->tripledot = 0;
    result->triplepos = 0;
    result->selcnt = 0;

    BuildDotList (args, result);

    DBUG_RETURN (result);
}

void
FreeDotInfo (dotinfo *node)
{
    DBUG_ENTER ("FreeDotInfo");

    while (node->left != NULL) {
        dotlist *tmp = node->left;
        node->left = node->left->next;
        Free (tmp);
    }

    Free (node);
}

int
LDot2Pos (int dot, dotinfo *info)
{
    dotlist *dots = info->left;
    int cnt;

    DBUG_ENTER ("LDot2Pos");

    for (cnt = 1; cnt < dot; cnt++) {
        dots = dots->next;
    }

    DBUG_RETURN (dots->position);
}

int
RDot2Pos (int dot, dotinfo *info)
{
    dotlist *dots = info->right;
    int cnt;

    DBUG_ENTER ("RDot2Pos");

    for (cnt = 1; cnt < dot; cnt++) {
        dots = dots->prev;
    }

    DBUG_RETURN (info->selcnt - dots->position + 1);
}

int
LIsDot (int dot, dotinfo *info)
{
    int result = 0;
    dotlist *list = info->left;

    DBUG_ENTER ("LIsDot");

    while ((list != NULL) && (list->position <= dot)) {
        if (list->position == dot) {
            result = list->no;
            exit;
        }

        list = list->next;
    }

    DBUG_RETURN (result);
}

int
RIsDot (int dot, dotinfo *info)
{
    int result = 0;
    DBUG_ENTER ("RIsDot");

    result = LIsDot (info->selcnt - dot + 1, info);
    result = info->dotcnt - result;

    DBUG_RETURN (result);
}

node *
GetNthExpr (int n, node *exprs)
{
    int cnt;
    node *result = NULL;

    DBUG_ENTER ("GetNthExpr");

    for (cnt = 1; cnt < n; cnt++) {
        if (exprs == NULL)
            exit;
        exprs = EXPRS_NEXT (exprs);
    }

    if (exprs != NULL)
        result = EXPRS_EXPR (exprs);

    DBUG_RETURN (result);
}

node *
MakeTmpId ()
{
    node *result;

    DBUG_ENTER ("MakeTmpId");

    result = MakeId (TmpVar (), NULL, ST_regular);

    DBUG_RETURN (result);
}

/* BuildTakeDrop may work or may not ;))
 * The generated code needs the new typechecker
 * so the development is delayed until then
 */

node *
BuildTakeDrop (node *from, node *to, node *vector)
{
    node *result;
    node *iv;

    DBUG_ENTER ("BuildTakeDrop");

    iv = MakeTmpId ();

    result = MakeNWith (
      MakeNPart (MakeNWithid (DupId_Ids (iv), NULL),
                 MakeNGenerator (MakeArray (MakeExprs (from, NULL)),
                                 MakeArray (MakeExprs (to, NULL)), F_le, F_le, NULL,
                                 NULL),
                 NULL),
      MakeNCode (MAKE_EMPTY_BLOCK (),
                 MakePrf (F_sel, MakeExprs (iv, MakeExprs (DupTree (vector), NULL)))),
      MakeNWithOp (WO_genarray,
                   MakeArray (MakeExprs (
                     MakePrf (F_add,
                              MakeExprs (MakeNum (1),
                                         MakeExprs (MakePrf (F_sub,
                                                             MakeExprs (to,
                                                                        MakeExprs (from,
                                                                                   NULL))),
                                                    NULL))),
                     NULL))));

    NCODE_USED (NWITH_CODE (result))++;
    NPART_CODE (NWITH_PART (result)) = NWITH_CODE (result);

    DBUG_RETURN (result);
}

node *
BuildLeftShape (node *args, node *array, dotinfo *info)
{
    int cnt;
    int maxdot;
    node *result = NULL;

    DBUG_ENTER ("BuildLeftShape");

    if (info->tripledot == 0)
        maxdot = info->dotcnt;
    else
        maxdot = info->tripledot - 1;

    for (cnt = maxdot; cnt > 0; cnt--) {
        result
          = MakeExprs (MakePrf (F_sel,
                                MakeExprs (MakeArray (
                                             MakeExprs (MakeNum (LDot2Pos (cnt, info)
                                                                 - 1),
                                                        NULL)),
                                           MakeExprs (MakePrf (F_shape,
                                                               MakeExprs (DupTree (array),
                                                                          NULL)),
                                                      NULL))),
                       result);
    }

    DBUG_RETURN (result);
}

/* BuildMiddleShape may work or may not ;))
 * The generated code needs the new typechecker
 * so the development is delayed until then
 */

node *
BuildMiddleShape (node *args, node *array, dotinfo *info)
{
    node *result = NULL;
    node *shape = NULL;
    node *dim = NULL;
    node *from = NULL;
    node *to = NULL;

    DBUG_ENTER ("BuildMiddleShape");

    shape = MakePrf (F_shape, MakeExprs (DupTree (array), NULL));

    dim = MakePrf (F_dim, MakeExprs (DupTree (array), NULL));

    from = MakeArray (MakeExprs (MakeNum (info->triplepos), NULL));

    to = MakePrf (F_sub,
                  MakeExprs (dim,
                             MakeExprs (MakeNum (info->selcnt - info->triplepos), NULL)));

    result = BuildTakeDrop (from, to, shape);

    DBUG_RETURN (result);
}

node *
BuildShape (node *args, node *array, dotinfo *info)
{
    node *result = NULL;

    DBUG_ENTER ("BuildShape");

    result = BuildLeftShape (args, array, info);

    result = MakeArray (result);

    DBUG_RETURN (result);
}

node *
BuildLeftIndex (node *args, node *iv, dotinfo *info)
{
    int cnt;
    int maxcnt;
    node *result = NULL;

    DBUG_ENTER ("BuildLeftIndex");

    if (info->tripledot == 0) {
        maxcnt = info->selcnt;
    } else {
        maxcnt = info->triplepos - 1;
    }

    for (cnt = maxcnt; cnt > 0; cnt--) {
        if (LIsDot (cnt, info)) {
            /* Make selection iv[ldot-1(cnt)] */
            result
              = MakeExprs (MakePrf (F_sel,
                                    MakeExprs (MakeArray (
                                                 MakeExprs (MakeNum (LIsDot (cnt, info)
                                                                     - 1),
                                                            NULL)),
                                               MakeExprs (DupTree (iv), NULL))),
                           result);
        } else {
            result = MakeExprs (DupTree (GetNthExpr (cnt, args)), result);
        }
    }

    DBUG_RETURN (result);
}

node *
BuildIndex (node *args, node *iv, dotinfo *info)
{
    node *result = NULL;

    DBUG_ENTER ("BuildIndex");

    result = BuildLeftIndex (args, iv, info);

    result = MakeArray (result);

    DBUG_RETURN (result);
}

node *
BuildWithLoop (node *shape, node *iv, node *array, node *index)
{
    node *result;

    DBUG_ENTER ("BuildWithLoop");

    result = MakeNWith (MakeNPart (MakeNWithid (DupId_Ids (iv), NULL),
                                   MakeNGenerator (MakeDot (1), MakeDot (1), F_le, F_le,
                                                   NULL, NULL),
                                   NULL),
                        MakeNCode (MAKE_EMPTY_BLOCK (),
                                   MakePrf (F_sel,
                                            MakeExprs (index, MakeExprs (DupTree (array),
                                                                         NULL)))),
                        MakeNWithOp (WO_genarray, shape));

    NCODE_USED (NWITH_CODE (result))++;
    NPART_CODE (NWITH_PART (result)) = NWITH_CODE (result);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *    node * EliminateSelDots( node * arg_node);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
EliminateSelDots (node *arg_node)
{
    funtab *tmp_tab;
    node *info_node;

    DBUG_ENTER ("EliminateSelDots");

    tmp_tab = act_tab;
    act_tab = hd_tab;

    info_node = MakeInfo ();

    arg_node = Trav (arg_node, info_node);

    info_node = FreeNode (info_node);

    act_tab = tmp_tab;

    ABORT_ON_ERROR;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node * HDdot( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDdot (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("HDdot");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node * HDprf( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDprf (node *arg_node, node *arg_info)
{
    node *result = arg_node;

    DBUG_ENTER ("HDprf");

    /* At first we traverse through all sons, to make sure all  */
    /* sel statements therein are handled                       */

    Trav (PRF_ARGS (arg_node), arg_info);

    /* only sel statements are of interest here, so just return */
    /* on anything else                                         */
    /* besides ARG1 must be an array. because otherwise there   */
    /* is no possibility to find any dot...                     */

    if ((PRF_PRF (arg_node) == F_sel) && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_array)) {
        dotinfo *info = MakeDotInfo (ARRAY_AELEMS (PRF_ARG1 (arg_node)));

        /* for now, ... is not supported anyway, so check for that*/

        if (info->tripledot != 0) {
            ERROR (linenum, ("... is not supported yet."));
        }

        if (info->dotcnt != 0) {
            node *shape;
            node *iv;
            node *index;

            iv = MakeTmpId ();
            shape = BuildShape (ARRAY_AELEMS (PRF_ARG1 (arg_node)), PRF_ARG2 (arg_node),
                                info);
            index = BuildIndex (ARRAY_AELEMS (PRF_ARG1 (arg_node)), iv, info);

            result = BuildWithLoop (shape, iv, PRF_ARG2 (arg_node), index);

            FreeTree (arg_node);
            FreeNode (iv);
        }

        FreeDotInfo (info);
    }

    DBUG_RETURN (result);
}
