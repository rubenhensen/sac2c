/*
 *
 * $Log$
 * Revision 1.15  2002/10/20 16:57:28  sah
 * fixed bug when substituting '.' in nested WLs
 * implemented setnotation for indexvectors
 * some code-cleanup
 *
 * Revision 1.14  2002/10/02 09:39:37  sbs
 * in commission for sah; (don't blame me 8-))
 *
 * Revision 1.13  2002/09/11 23:19:42  dkr
 * prf_node_info.mac modified
 *
 * Revision 1.12  2002/09/06 12:45:56  sah
 * some minor modifications.
 *
 * Revision 1.11  2002/09/06 11:45:32  sah
 * added support for N_selwl.
 *
 * Revision 1.10  2002/09/05 13:25:50  sah
 * added first support for WL shortcut and fixed
 * some minor bugs
 *
 * Revision 1.9  2002/08/26 13:46:00  sah
 * first full implementation of multi-
 * dimensional select.
 *
 * Revision 1.8  2002/08/21 13:43:54  sah
 * fixed bug traversing N_ap nodes without arguments.
 *
 * Revision 1.7  2002/08/20 17:31:27  sah
 * fixed some bugs occuring due to new
 * scanner/parser.
 *
 * Revision 1.6  2002/08/13 12:22:56  sbs
 * GRRRR ugly error in HDap did kill N_ap nodes.....
 *
 * Revision 1.5  2002/08/13 09:54:52  sah
 * added support for new scanner/parser and
 * dot elimination in WL boundaries.
 *
 * Revision 1.4  2002/07/24 10:40:35  dkr
 * FreeDotInfo: DBUG_VOID_RETURN added
 * tabs removed
 *
 * Revision 1.3  2002/07/24 10:30:17  sah
 * first support for multidimensional
 * selects added. '...' not supported yet.
 *
 * Revision 1.2  2002/07/19 13:24:49  sah
 * added functions for traversal.
 *
 * Revision 1.1  2002/07/09 12:54:25  sbs
 * Initial revision
 *
 */

#include "handle_dots.h"
#include "traverse.h"
#include "dbug.h"
#include "free.h"
#include "Error.h"
#include "DupTree.h"
#include "internal_lib.h"
#include <strings.h>

/*
 * Structures to store all information about dots occuring in select state-
 * ments that are needed to perform the transformation. These structures are
 * built by BuildDotList.
 * All int values are counted beginning with 1. The 0 value is used as
 * flag for non-existent (false).
 */

typedef struct DOTLIST {
    int no;               /* number of dots counted from left */
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

/*
 * Structures to store ids and shapes during shape-scan. Filled during
 * traversal in HD_sacn mode.
 */

typedef enum TRAVSTATE { HD_sel, HD_scan } travstate;
typedef enum IDTYPE { ID_vector, ID_scalar } idtype;

typedef struct SHPCHAIN {
    node *shape;
    struct SHPCHAIN *next;
} shpchain;

typedef struct IDTABLE {
    char *id;
    idtype type;
    shpchain *shapes;
    struct IDTABLE *next;
} idtable;

/*
 * arg_info in this file:
 * DOTSHAPE:    this field is used in order to transport the generic shape
 * (node[0])    from Nwithid (via Nwith) to Ngenerator, where it may be used
 *              in order to replace . generator boundaries.
 * TRAVSTATE:   this field is used to determine the current traversalmode
 * (flag)       HD_sel in normal mode (eliminate dots)
 *              HD_scan in shape scanning mode
 * IDTABLE:     used to reference the current idtable.
 * (info2)
 * ASSIGNS:     stores any assigns that have to be inserted prior to
 * (node[1])    the current one. Used to build shape for WLs.
 */

#define INFO_HD_DOTSHAPE(n) n->node[0]
#define INFO_HD_TRAVSTATE(n) ((travstate)n->flag)
#define INFO_HD_IDTABLE(n) ((idtable *)n->info2)
#define INFO_HD_ASSIGNS(n) n->node[1]

/******************************************************************************
 *
 * Function:
 *   void BuildDotList(node* tree, dotinfo* info)
 *
 * Description:
 *
 *
 ******************************************************************************/

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

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   dotinfo* MakeDotInfo(node* args)
 *
 * Description:
 *
 *
 ******************************************************************************/

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

/******************************************************************************
 *
 * Function:
 *   void FreeDotInfo(dotinfo* node)
 *
 * Description:
 *
 *
 ******************************************************************************/

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

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   int LDot2Pos(int dot, dotinfo* info)
 *
 * Description:
 *
 *
 ******************************************************************************/

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

    if (result != 0) {
        result = info->dotcnt - result + 1;
    }

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

node *
BuildTakeDrop (node *from, node *to, node *vector)
{
    node *result;
    node *iv;

    DBUG_ENTER ("BuildTakeDrop");

    iv = MakeTmpId ();

    result = MakeNWith (MakeNPart (MakeNWithid (DupId_Ids (iv), NULL),
                                   MakeNGenerator (MakeArray (MakeExprs (from, NULL)),
                                                   MakeArray (MakeExprs (to, NULL)), F_le,
                                                   F_le, NULL, NULL),
                                   NULL),
                        MakeNCode (MAKE_EMPTY_BLOCK (),
                                   MAKE_BIN_PRF (F_sel, iv, DupTree (vector))),
                        MakeNWithOp (WO_genarray,
                                     MakeArray (
                                       MakeExprs (MAKE_BIN_PRF (F_add_SxS, MakeNum (1),
                                                                MAKE_BIN_PRF (F_sub_SxS,
                                                                              to, from)),
                                                  NULL))));

    NCODE_USED (NWITH_CODE (result))++;
    NPART_CODE (NWITH_PART (result)) = NWITH_CODE (result);

    DBUG_RETURN (result);
}

node *
BuildConcat (node *a, node *b)
{
    node *result;
    node *iv;

    DBUG_ENTER ("BuildConcat");

    if (a == NULL)
        DBUG_RETURN (b);
    if (b == NULL)
        DBUG_RETURN (a);

    iv = MakeTmpId ();

    result = MakeNWith (
      MakeNPart (MakeNWithid (DupId_Ids (iv), NULL),
                 MakeNGenerator (
                   MakeArray (MakeExprs (MakeNum (0), NULL)),
                   MakeArray (
                     MakeExprs (MAKE_BIN_PRF (F_add_SxA, MakeNum (-1),
                                              MAKE_BIN_PRF (F_add_AxA,
                                                            MakePrf (F_dim,
                                                                     MakeExprs (DupTree (
                                                                                  a),
                                                                                NULL)),
                                                            MakePrf (F_dim,
                                                                     MakeExprs (DupTree (
                                                                                  b),
                                                                                NULL)))),
                                NULL)),
                   F_le, F_le, NULL, NULL),
                 NULL),
      MakeNCode (MAKE_EMPTY_BLOCK (),
                 MakeCond (MAKE_BIN_PRF (F_lt, DupTree (iv),
                                         MakePrf (F_dim, MakeExprs (DupTree (a), NULL))),
                           MAKE_BIN_PRF (F_sel,
                                         MakeArray (MakeExprs (DupTree (iv), NULL)),
                                         DupTree (a)),
                           MAKE_BIN_PRF (F_sel,
                                         MakeArray (MakeExprs (
                                           MAKE_BIN_PRF (F_sub_SxS, iv,
                                                         MakePrf (F_dim,
                                                                  MakeExprs (DupTree (a),
                                                                             NULL))),
                                           NULL)),
                                         DupTree (b)))),
      MakeNWithOp (WO_genarray,
                   MakeArray (
                     MakeExprs (MAKE_BIN_PRF (F_add_SxS,
                                              MakePrf (F_dim,
                                                       MakeExprs (DupTree (a), NULL)),
                                              MakePrf (F_dim,
                                                       MakeExprs (DupTree (b), NULL))),
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
        result = MakeExprs (MAKE_BIN_PRF (F_sel,
                                          MakeArray (
                                            MakeExprs (MakeNum (LDot2Pos (cnt, info) - 1),
                                                       NULL)),
                                          MakePrf (F_shape,
                                                   MakeExprs (DupTree (array), NULL))),
                            result);
    }

    result = MakeArray (result);

    DBUG_RETURN (result);
}

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

    from = MakeNum (info->triplepos - 1);

    to = MakePrf (F_sub_SxS,
                  MakeExprs (dim, MakeExprs (MakeNum (info->selcnt - info->triplepos + 1),
                                             NULL)));

    result = BuildTakeDrop (from, to, shape);

    DBUG_RETURN (result);
}

node *
BuildRightShape (node *args, node *array, dotinfo *info)
{
    int cnt;
    int maxdot;
    node *result = NULL;

    DBUG_ENTER ("BuildRighthape");

    maxdot = info->dotcnt - info->tripledot;

    for (cnt = 1; cnt <= maxdot; cnt++) {
        result = MakeExprs (
          MAKE_BIN_PRF (F_sel,
                        MAKE_BIN_PRF (F_sub_SxS,
                                      MakePrf (F_dim,
                                               MakeExprs (MakePrf (F_shape,
                                                                   MakeExprs (DupTree (
                                                                                array),
                                                                              NULL)),
                                                          NULL)),
                                      MakeNum (RDot2Pos (cnt, info))),
                        MakePrf (F_shape, MakeExprs (DupTree (array), NULL))),
          result);
    }

    result = MakeArray (result);

    DBUG_RETURN (result);
}

node *
MakeAssignLetNV (char *var_name, node *let_expr)
{
    ids *tmp_ids;
    node *tmp_node;

    DBUG_ENTER ("MakeAssignLet");

    tmp_ids = MakeIds (var_name, NULL, ST_regular);
    tmp_node = MakeLet (let_expr, tmp_ids);
    tmp_node = MakeAssign (tmp_node, NULL);

    DBUG_RETURN (tmp_node);
}

node *
BuildShape (node *args, node *array, node **assigns, dotinfo *info)
{
    node *leftshape = NULL;
    node *leftid = NULL;
    node *middleshape = NULL;
    node *middleid = NULL;
    node *rightshape = NULL;
    node *rightid = NULL;

    DBUG_ENTER ("BuildShape");

    if (info->triplepos != 1) {
        leftshape = BuildLeftShape (args, array, info);
        leftid = MakeTmpId ();
        *assigns = AppendAssign (*assigns, MakeAssignLetNV (StringCopy (ID_NAME (leftid)),
                                                            leftshape));
    }

    if (info->triplepos != 0) {
        middleshape = BuildMiddleShape (args, array, info);
        middleid = MakeTmpId ();
        *assigns
          = AppendAssign (*assigns,
                          MakeAssignLetNV (StringCopy (ID_NAME (middleid)), middleshape));
    }

    if ((info->triplepos != 0) && (info->triplepos != info->selcnt)) {
        rightshape = BuildRightShape (args, array, info);
        rightid = MakeTmpId ();
        *assigns
          = AppendAssign (*assigns,
                          MakeAssignLetNV (StringCopy (ID_NAME (rightid)), rightshape));
    }

    if (rightid != NULL) {
        node *tmpid = NULL;

        tmpid = MakeTmpId ();

        *assigns
          = AppendAssign (*assigns, MakeAssignLetNV (StringCopy (ID_NAME (tmpid)),
                                                     BuildConcat (middleid, rightid)));

        middleid = tmpid;
        rightid = NULL;
    }

    if (middleid != NULL) {
        if (leftid == NULL) {
            leftid = middleid;
            middleid = NULL;
        } else {
            node *tmpid = NULL;

            tmpid = MakeTmpId ();

            *assigns
              = AppendAssign (*assigns, MakeAssignLetNV (StringCopy (ID_NAME (tmpid)),
                                                         BuildConcat (leftid, middleid)));

            leftid = tmpid;
            middleid = NULL;
        }
    }

    DBUG_ASSERT ((leftid != NULL), "error building shape: the shape is empty!");

    DBUG_RETURN (leftid);
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
            /* Make selection iv[ldot(cnt)-1] */
            result
              = MakeExprs (MAKE_BIN_PRF (F_sel,
                                         MakeArray (
                                           MakeExprs (MakeNum (LIsDot (cnt, info) - 1),
                                                      NULL)),
                                         DupTree (iv)),
                           result);
        } else {
            result = MakeExprs (DupTree (GetNthExpr (cnt, args)), result);
        }
    }

    result = MakeArray (result);

    DBUG_RETURN (result);
}

node *
BuildMiddleIndex (node *args, node *iv, dotinfo *info)
{
    node *result = NULL;
    node *from = NULL;
    node *to = NULL;

    DBUG_ENTER ("BuildMiddleIndex");

    from = MakeNum (LDot2Pos (info->triplepos, info) - 1);
    to = MakeNum (info->selcnt - RDot2Pos (info->triplepos, info));

    result = BuildTakeDrop (from, to, DupTree (iv));

    DBUG_RETURN (result);
}

node *
BuildRightIndex (node *args, node *iv, dotinfo *info)
{
    int cnt;
    int maxcnt;
    node *result = NULL;

    DBUG_ENTER ("BuildRightIndex");

    maxcnt = info->selcnt - info->triplepos;

    for (cnt = 1; cnt <= maxcnt; cnt++) {
        if (RIsDot (cnt, info)) {
            /* Make selection iv[selcnt - rdot(cnt)] */
            result
              = MakeExprs (MAKE_BIN_PRF (F_sel,
                                         MakeArray (MakeExprs (
                                           MAKE_BIN_PRF (F_sub_SxS,
                                                         MakePrf (F_dim,
                                                                  MakeExprs (DupTree (iv),
                                                                             NULL)),
                                                         MakeNum (RIsDot (cnt, info))),
                                           NULL)),
                                         DupTree (iv)),
                           result);
        } else {
            result
              = MakeExprs (DupTree (GetNthExpr (info->selcnt - cnt + 1, args)), result);
        }
    }

    result = MakeArray (result);

    DBUG_RETURN (result);
}

node *
BuildIndex (node *args, node *iv, node *block, dotinfo *info)
{
    node *leftindex = NULL;
    node *leftid = NULL;
    node *middleindex = NULL;
    node *middleid = NULL;
    node *rightindex = NULL;
    node *rightid = NULL;

    DBUG_ENTER ("BuildIndex");

    if (info->triplepos != 1) {
        leftindex = BuildLeftIndex (args, iv, info);
        leftid = MakeTmpId ();
        BLOCK_INSTR (block)
          = AppendAssign (BLOCK_INSTR (block),
                          MakeAssignLetNV (StringCopy (ID_NAME (leftid)), leftindex));
    }

    if (info->triplepos != 0) {
        middleindex = BuildMiddleIndex (args, iv, info);
        middleid = MakeTmpId ();
        BLOCK_INSTR (block)
          = AppendAssign (BLOCK_INSTR (block),
                          MakeAssignLetNV (StringCopy (ID_NAME (middleid)), middleindex));
    }

    if ((info->triplepos != 0) && (info->triplepos != info->selcnt)) {
        rightindex = BuildRightIndex (args, iv, info);
        rightid = MakeTmpId ();
        BLOCK_INSTR (block)
          = AppendAssign (BLOCK_INSTR (block),
                          MakeAssignLetNV (StringCopy (ID_NAME (rightid)), rightindex));
    }

    if (rightid != NULL) {
        node *tmpid = NULL;

        tmpid = MakeTmpId ();

        BLOCK_INSTR (block)
          = AppendAssign (BLOCK_INSTR (block),
                          MakeAssignLetNV (StringCopy (ID_NAME (tmpid)),
                                           BuildConcat (middleid, rightid)));

        middleid = tmpid;
        rightid = NULL;
    }

    if (middleid != NULL) {
        if (leftid == NULL) {
            leftid = middleid;
            middleid = NULL;
        } else {
            node *tmpid = NULL;

            tmpid = MakeTmpId ();

            BLOCK_INSTR (block)
              = AppendAssign (BLOCK_INSTR (block),
                              MakeAssignLetNV (StringCopy (ID_NAME (tmpid)),
                                               BuildConcat (leftid, middleid)));

            leftid = tmpid;
            middleid = NULL;
        }
    }

    DBUG_ASSERT (leftid != NULL, "error building index: the index is empty!");

    DBUG_RETURN (leftid);
}

node *
BuildWithLoop (node *shape, node *iv, node *array, node *index, node *block)
{
    node *result;

    DBUG_ENTER ("BuildWithLoop");

    result = MakeNWith (MakeNPart (MakeNWithid (DupId_Ids (iv), NULL),
                                   MakeNGenerator (MakeDot (1), MakeDot (1), F_le, F_le,
                                                   NULL, NULL),
                                   NULL),
                        MakeNCode (block, MakeAp2 (StringCopy ("sel"), NULL, index,
                                                   DupTree (array))),
                        MakeNWithOp (WO_genarray, shape));

    NCODE_USED (NWITH_CODE (result))++;
    NPART_CODE (NWITH_PART (result)) = NWITH_CODE (result);

    DBUG_RETURN (result);
}

idtable *
BuildIdTable (node *ids, idtable *appendto)
{
    idtable *result = appendto;

    DBUG_ENTER ("BuildIdTable");

    if (NODE_TYPE (ids) == N_exprs) {
        while (ids != NULL) {
            node *id = EXPRS_EXPR (ids);
            idtable *newtab = Malloc (sizeof (idtable));

            if (NODE_TYPE (id) != N_id) {
                ERROR (linenum, ("found non-id as index in WL set notation"));

                /* we create a dummy entry within the idtable in order */
                /* to go on and search for further errors.             */
                newtab->id = StringCopy ("_non_id_expr");
            } else
                newtab->id = StringCopy (ID_NAME (id));

            newtab->type = ID_scalar;
            newtab->shapes = NULL;
            newtab->next = result;
            result = newtab;
            ids = EXPRS_NEXT (ids);
        }
    } else {
        idtable *newtab = Malloc (sizeof (idtable));
        newtab->id = StringCopy (ID_NAME (ids));
        newtab->type = ID_vector;
        newtab->shapes = NULL;
        newtab->next = result;
        result = newtab;
    }

    DBUG_RETURN (result);
}

void
FreeIdTable (idtable *table, idtable *until)
{
    DBUG_ENTER ("FreeIdTable");

    while (table != until) {
        idtable *next = table->next;

        /* free shape-chain but NOT shapes itself */
        while (table->shapes != NULL) {
            shpchain *next = table->shapes->next;
            Free (table->shapes);
            table->shapes = next;
        }

        /* free table */
        Free (table->id);
        Free (table);
        table = next;
    }

    DBUG_VOID_RETURN;
}

void
ScanVector (node *vector, node *array, idtable *ids)
{
    int poscnt = 0;

    DBUG_ENTER ("ScanVector");

    while (vector != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (vector)) == N_id) {
            idtable *handle = ids;

            while (handle != NULL) {
                if ((handle->type == ID_scalar)
                    && (strcmp (handle->id, ID_NAME (EXPRS_EXPR (vector))) == 0)) {
                    node *shape
                      = MAKE_BIN_PRF (F_sel,
                                      MakeArray (MakeExprs (MakeNum (poscnt), NULL)),
                                      MakePrf (F_shape,
                                               MakeExprs (DupTree (array), NULL)));
                    shpchain *chain = Malloc (sizeof (shpchain));

                    chain->shape = shape;
                    chain->next = handle->shapes;
                    handle->shapes = chain;

                    break;
                }

                handle = handle->next;
            }
        }

        poscnt++;
        vector = EXPRS_NEXT (vector);
    }

    DBUG_VOID_RETURN;
}

void
ScanId (node *id, node *array, idtable *ids)
{
    DBUG_ENTER ("ScanId");

    while (ids != NULL) {
        if ((ids->type == ID_vector) && (strcmp (ids->id, ID_NAME (id)) == 0)) {
            node *shape = MakePrf (F_shape, MakeExprs (DupTree (array), NULL));
            shpchain *chain = Malloc (sizeof (shpchain));

            chain->shape = shape;
            chain->next = ids->shapes;
            ids->shapes = chain;

            break;
        }

        ids = ids->next;
    }

    DBUG_VOID_RETURN;
}

node *
BuildShapeVectorMin (shpchain *vectors)
{
    node *result = NULL;
    node *index = MakeTmpId ();
    node *shape = NULL;
    node *expr = NULL;

    DBUG_ENTER ("BuildVectorMin");

    shape = MakePrf (F_shape, MakeExprs (vectors->shape, NULL));

    expr = MAKE_BIN_PRF (F_sel, DupTree (index), vectors->shape);

    vectors = vectors->next;

    while (vectors != NULL) {
        expr = MAKE_BIN_PRF (F_min, MAKE_BIN_PRF (F_sel, DupTree (index), vectors->shape),
                             expr);
        vectors = vectors->next;
    }

    result = MakeNWith (MakeNPart (MakeNWithid (DupId_Ids (index), NULL),
                                   MakeNGenerator (MakeDot (1), MakeDot (1), F_le, F_le,
                                                   NULL, NULL),
                                   NULL),
                        MakeNCode (MAKE_EMPTY_BLOCK (), expr),
                        MakeNWithOp (WO_genarray, shape));

    NCODE_USED (NWITH_CODE (result))++;
    NPART_CODE (NWITH_PART (result)) = NWITH_CODE (result);

    FreeTree (index);

    DBUG_RETURN (result);
}

node *
BuildWLShape (idtable *table, idtable *end)
{
    node *result = NULL;

    DBUG_ENTER ("BuildWLShape");

    if (table->type == ID_scalar) {
        while (table != end) {
            node *shape = NULL;
            shpchain *handle = table->shapes;

            if (handle == NULL) {
                ERROR (linenum, ("no shape information found for %s", table->id));
            } else {
                shape = handle->shape;
                handle = handle->next;

                while (handle != NULL) {
                    shape = MAKE_BIN_PRF (F_min, shape, handle->shape);
                    handle = handle->next;
                }
            }

            result = MakeExprs (shape, result);
            table = table->next;
        }

        result = MakeArray (result);
    }

    if (table->type == ID_vector) {
        if (table->shapes == NULL) {
            ERROR (linenum, ("no shape information found for %s", table->id));
        } else {
            result = BuildShapeVectorMin (table->shapes);
        }
    }

    DBUG_RETURN (result);
}

ids *
Exprs2Ids (node *exprs)
{
    ids *result = NULL;
    ids *handle = NULL;

    DBUG_ENTER ("Exprs2Ids");

    while (exprs != NULL) {
        ids *newid = NULL;

        if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_id)
            newid = DupId_Ids (EXPRS_EXPR (exprs));
        else {
            /* create dummy id in order to go on until end of phase */
            ERROR (linenum, ("found non-id expression in index vector"));
            newid = MakeIds (StringCopy ("unknown_id"), NULL, ST_regular);
        }

        if (handle == NULL) {
            result = newid;
            handle = newid;
        } else {
            IDS_NEXT (handle) = newid;
            handle = newid;
        }

        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *    node* EliminateSelDots( node *arg_node);
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
    INFO_HD_TRAVSTATE (info_node) = HD_sel;

    arg_node = Trav (arg_node, info_node);

    info_node = FreeNode (info_node);

    act_tab = tmp_tab;

    ABORT_ON_ERROR;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *HDwith( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDwith (node *arg_node, node *arg_info)
{
    /* INFO_HD_DOTSHAPE is used for '.'-substitution in WLgenerators */
    /* in order to handle nested WLs correct, olddotshape stores not */
    /* processed shapes until this (maybe inner) WL is processed.    */

    node *olddotshape = INFO_HD_DOTSHAPE (arg_info);

    DBUG_ENTER ("HDwith");

    /*
     * by default (TravSons), withop would be traversed last, but
     * some information from withop is needed in order to traverse
     * the rest, so the withop is handeled first.
     */

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    INFO_HD_DOTSHAPE (arg_info) = olddotshape;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *HDwithop( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("HDWithop");

    if (INFO_HD_TRAVSTATE (arg_info) == HD_sel) {
        switch (NWITHOP_TYPE (arg_node)) {
        case WO_modarray:
            if (sbs == 1) {
                INFO_HD_DOTSHAPE (arg_info)
                  = MakeAp1 (StringCopy ("shape"), NULL,
                             DupTree (NWITHOP_ARRAY (arg_node)));
            } else {
                INFO_HD_DOTSHAPE (arg_info) = NULL;
            }
            break;

        case WO_genarray:
            if (sbs == 1) {
                INFO_HD_DOTSHAPE (arg_info) = DupTree (NWITHOP_SHAPE (arg_node));
            } else {
                INFO_HD_DOTSHAPE (arg_info) = NULL;
            }
            break;

        case WO_foldfun:
            /* here is no break missing! */
        case WO_foldprf:
            INFO_HD_DOTSHAPE (arg_info) = NULL;
            break;

        default:
            DBUG_ASSERT ((0), "wrong withop tag in N_Nwithop node!");
            break;
        }
    }

    TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *HDgenerator( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDgenerator (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("HDGenerator");

    if ((sbs == 1) && (INFO_HD_TRAVSTATE (arg_info) == HD_sel)) {
        /*
         * Dots are replaced by the "shape" expressions, that are imported via
         * INFO_HD_DOTSHAPE( arg_info)    (cf. HDWithop),
         * and the bounds are adjusted so that the operator can be
         * "normalized" to:   bound1 <= iv = [...] < bound2     .
         */

        if ((INFO_HD_DOTSHAPE (arg_info) == NULL)
            && (DOT_ISSINGLE (NGEN_BOUND1 (arg_node))
                || DOT_ISSINGLE (NGEN_BOUND2 (arg_node)))) {
            ABORT (linenum, ("dot notation is not allowed in fold with loops"));
        }

        if (DOT_ISSINGLE (NGEN_BOUND1 (arg_node))) {
            /* replace "." by "0 * shp" */
            NGEN_BOUND1 (arg_node) = FreeTree (NGEN_BOUND1 (arg_node));
            NGEN_BOUND1 (arg_node)
              = MakePrf2 (F_mul_SxA, MakeNum (0), DupTree (INFO_HD_DOTSHAPE (arg_info)));
        }

        if (NGEN_OP1 (arg_node) == F_lt) {
            /* make <= from < and add 1 to bound */
            NGEN_OP1 (arg_node) = F_le;
            NGEN_BOUND1 (arg_node)
              = MakePrf2 (F_add_AxS, NGEN_BOUND1 (arg_node), MakeNum (1));
        }

        if (DOT_ISSINGLE (NGEN_BOUND2 (arg_node))) {
            if (NGEN_OP2 (arg_node) == F_le) {
                /* make < from <= and replace "." by "shp"  */
                NGEN_OP2 (arg_node) = F_lt;
                NGEN_BOUND2 (arg_node) = FreeTree (NGEN_BOUND2 (arg_node));
                NGEN_BOUND2 (arg_node) = INFO_HD_DOTSHAPE (arg_info);
            } else {
                /* replace "." by "shp - 1"  */
                NGEN_BOUND2 (arg_node) = FreeTree (NGEN_BOUND2 (arg_node));
                NGEN_BOUND2 (arg_node)
                  = MakePrf2 (F_sub_AxS, INFO_HD_DOTSHAPE (arg_info), MakeNum (1));
            }
            INFO_HD_DOTSHAPE (arg_info) = NULL; /* has been consumed ! */
        } else {
            if (NGEN_OP2 (arg_node) == F_le) {
                /* make < from <= and add 1 to bound */
                NGEN_OP2 (arg_node) = F_lt;
                NGEN_BOUND2 (arg_node)
                  = MakePrf2 (F_add_AxS, NGEN_BOUND2 (arg_node), MakeNum (1));
            }
            if (INFO_HD_DOTSHAPE (arg_info) != NULL) {
                INFO_HD_DOTSHAPE (arg_info) = FreeTree (INFO_HD_DOTSHAPE (arg_info));
            }
        }
    }

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *HDdot( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDdot (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("HDdot");

    if ((INFO_HD_TRAVSTATE (arg_info) == HD_sel) && sbs)
        ERROR (linenum, ("'.' or '...' not allowed here."));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *HDap( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDap (node *arg_node, node *arg_info)
{
    node *result = arg_node;

    DBUG_ENTER ("HDap");

    /* only sel statements are of interest here, so just return */
    /* on anything else                                         */
    /* besides ARG1 must be an array. because otherwise there   */
    /* is no possibility to find any dot...                     */

    if ((INFO_HD_TRAVSTATE (arg_info) == HD_sel)
        && (strcmp (AP_NAME (arg_node), "sel") == 0)
        && (NODE_TYPE (AP_ARG1 (arg_node)) == N_array))

    {
        dotinfo *info = MakeDotInfo (ARRAY_AELEMS (AP_ARG1 (arg_node)));

        if (info->dotcnt != 0) {
            node *shape;
            node *iv;
            node *index;
            node *block;
            node *assigns = MakeEmpty ();

            iv = MakeTmpId ();
            block = MAKE_EMPTY_BLOCK ();
            shape = BuildShape (ARRAY_AELEMS (AP_ARG1 (arg_node)), AP_ARG2 (arg_node),
                                &assigns, info);

            if (INFO_HD_ASSIGNS (arg_info) == NULL) {
                INFO_HD_ASSIGNS (arg_info) = assigns;
            } else {
                INFO_HD_ASSIGNS (arg_info)
                  = AppendAssign (INFO_HD_ASSIGNS (arg_info), assigns);
            }

            index = BuildIndex (ARRAY_AELEMS (AP_ARG1 (arg_node)), iv, block, info);

            result = BuildWithLoop (shape, iv, AP_ARG2 (arg_node), index, block);

            FreeTree (arg_node);
            FreeNode (iv);
        }

        FreeDotInfo (info);
    }

    /* if in HD_scan mode, scan for shapes */

    if ((INFO_HD_TRAVSTATE (arg_info) == HD_scan)
        && (strcmp (AP_NAME (arg_node), "sel") == 0)) {
        if (NODE_TYPE (AP_ARG1 (arg_node)) == N_array) {
            ScanVector (ARRAY_AELEMS (AP_ARG1 (arg_node)), AP_ARG2 (arg_node),
                        INFO_HD_IDTABLE (arg_info));
        } else if (NODE_TYPE (AP_ARG1 (arg_node)) == N_id) {
            ScanId (AP_ARG1 (arg_node), AP_ARG2 (arg_node), INFO_HD_IDTABLE (arg_info));
        }
    }

    /* Now we traverse our result in order to handle any */
    /* dots inside.                                      */

    if (NODE_TYPE (result) == N_ap) {
        if (AP_ARGS (result) != NULL)
            AP_ARGS (result) = Trav (AP_ARGS (result), arg_info);
    } else
        result = Trav (result, arg_info);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *    node *HDprf( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("HDprf");

    /* if in HD_scan mode, scan for shapes */

    if ((INFO_HD_TRAVSTATE (arg_info) == HD_scan) && (PRF_PRF (arg_info) == F_sel)) {
        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_array) {
            ScanVector (ARRAY_AELEMS (PRF_ARG1 (arg_node)), PRF_ARG2 (arg_node),
                        INFO_HD_IDTABLE (arg_info));
        } else if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id) {
            ScanId (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), INFO_HD_IDTABLE (arg_info));
        }
    }

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *HDassign( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDassign (node *arg_node, node *arg_info)
{
    node *result = arg_node;
    node *oldassigns = INFO_HD_ASSIGNS (arg_info);

    DBUG_ENTER ("HDassign");

    /* first traverse sons with a fresh assigns-chain */

    INFO_HD_ASSIGNS (arg_info) = NULL;
    result = TravSons (arg_node, arg_info);

    /* check for assigns that are to be inserted */

    if (INFO_HD_ASSIGNS (arg_info) != NULL) {
        result = INFO_HD_ASSIGNS (arg_info);
        AppendAssign (result, arg_node);
    }

    /* reinstall old assigns-chain */

    INFO_HD_ASSIGNS (arg_info) = oldassigns;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *    node *HDsetwl( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDsetwl (node *arg_node, node *arg_info)
{
    node *result = NULL;
    travstate oldstate = INFO_HD_TRAVSTATE (arg_info);
    idtable *oldtable = INFO_HD_IDTABLE (arg_info);
    node *shape = NULL;

    DBUG_ENTER ("HDsetwl");

    INFO_HD_TRAVSTATE (arg_info) = HD_scan;
    INFO_HD_IDTABLE (arg_info)
      = BuildIdTable (SETWL_IDS (arg_node), INFO_HD_IDTABLE (arg_info));

    TravSons (arg_node, arg_info);

    shape = BuildWLShape (INFO_HD_IDTABLE (arg_info), oldtable);

    if (INFO_HD_IDTABLE (arg_info)->type == ID_scalar) {
        result
          = MakeNWith (MakeNPart (MakeNWithid (NULL, Exprs2Ids (SETWL_IDS (arg_node))),
                                  MakeNGenerator (MakeDot (1), MakeDot (1), F_le, F_le,
                                                  NULL, NULL),
                                  NULL),
                       MakeNCode (MAKE_EMPTY_BLOCK (), DupTree (SETWL_EXPR (arg_node))),
                       MakeNWithOp (WO_genarray, shape));
    } else {
        result
          = MakeNWith (MakeNPart (MakeNWithid (DupId_Ids (SETWL_IDS (arg_node)), NULL),
                                  MakeNGenerator (MakeDot (1), MakeDot (1), F_le, F_le,
                                                  NULL, NULL),
                                  NULL),
                       MakeNCode (MAKE_EMPTY_BLOCK (), DupTree (SETWL_EXPR (arg_node))),
                       MakeNWithOp (WO_genarray, shape));
    }

    FreeTree (arg_node);

    NCODE_USED (NWITH_CODE (result))++;
    NPART_CODE (NWITH_PART (result)) = NWITH_CODE (result);

    FreeIdTable (INFO_HD_IDTABLE (arg_info), oldtable);

    INFO_HD_IDTABLE (arg_info) = oldtable;
    INFO_HD_TRAVSTATE (arg_info) = oldstate;

    result = Trav (result, arg_info);

    DBUG_RETURN (result);
}
