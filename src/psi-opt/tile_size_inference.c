/*
 *
 * $Log$
 * Revision 2.10  1999/08/30 18:31:15  bs
 * Bugs fixed.
 *
 * Revision 2.9  1999/08/30 14:06:24  bs
 * TSInwith and TSIncode modified:
 * Now a #pragma wlcomp will be created in TSIncode. In TSInwith this #pragma will
 * be added to the syntax tree.
 *
 * Revision 2.8  1999/08/04 14:35:47  bs
 * reinitial revision
 *
 * Revision 2.7  1999/05/10 11:11:40  bs
 * All functions of the tsi moved to wl_access_analyze.c
 *
 * Revision 2.6  1999/05/03 15:24:14  bs
 * The TSI is printing detailed information about array accesses within a WL.
 *
 * Revision 2.5  1999/04/29 08:00:56  bs
 * print routines modified.
 *
 * Revision 2.4  1999/04/12 18:00:54  bs
 * Two functions added: TSIprintAccesses and TSIprintFeatures.
 *
 * Revision 2.3  1999/04/08 12:49:37  bs
 * The TSI is analysing withloops now.
 *
 * Revision 2.2  1999/03/15 15:49:54  bs
 * access macros changed
 *
 * Revision 2.1  1999/02/23 12:43:17  sacbase
 * new release made
 *
 * Revision 1.1  1999/01/15 15:31:06  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   tile_size_inference.c
 *
 * prefix: TSI
 *
 * description:
 *
 *   This compiler module realizes an inference scheme for the selection
 *   of appropriate tile sizes. This is used by the code generation in
 *   order to create tiled target code.
 *
 *   The following access macros are defined for the info-node:
 *
 *      INFO_TSI_ACCESSCNT(n)             (n->counter)
 *      INFO_TSI_ARRAYDIM(n)              (n->varno)
 *      INFO_TSI_ARRAYSHP(n)    ((shpseg*)(n->node[0]))
 *      INFO_TSI_BLOCKSHP(n)    ((shpseg*)(n->node[1]))
 *      INFO_TSI_CACHEPAR(n)       ((int*)(n->node[2]))
 *      INFO_TSI_WLARRAY(n)               (n->node[3])
 *
 *****************************************************************************/

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "resource.h"
#include "print.h"
#include "tile_size_inference.h"

#define TYP_IF(n, d, p, f, sz) sz

static int dtype_size[] = {
#include "type_info.mac"
};

#undef TYP_IF

typedef struct ENH_ACCESS {
    int vaddr;
    int cline;
    int cinst;
    int aline;
} enh_access_t;

typedef struct SIMPLE_LIST {
    void *element;
    struct SIMPLE_LIST *next;
} list_t;

/*****************************************************************************
 *
 * function:
 *   list_t* InSort(void* element, list_t* list)
 *
 * description:
 *
 *
 *****************************************************************************/

static list_t *
InsortByCLine (void *element, list_t *list)
{
    enh_access_t *elem = (enh_access_t *)element;
    enh_access_t *next;
    list_t *result = list;

    if (result == NULL) {
        result = (list_t *)Malloc (sizeof (list_t));
        result->element = element;
        result->next = list;
    } else {
        next = (enh_access_t *)list->element;
        if (elem->cline < next->cline) {
            result = (list_t *)Malloc (sizeof (list_t));
            result->element = element;
            result->next = list;
        } else {
            result->next = InsortByCLine (element, list->next);
        }
    }
    return (result);
}

/*****************************************************************************
 *
 * function:
 *    int MinDistance(list_t* acc_list, int tilesize, node* arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/

static int
MinDistance (list_t *acc_list, int tilesize, node *arg_info)
{
    int cLine, cInst, NumLines, cSize, lSize, *cacheparam;
    list_t *a_list = acc_list;
    enh_access_t *access;

    DBUG_ENTER ("MinDistance");

    cacheparam = INFO_TSI_CACHEPARAM (arg_info);
    cSize = cacheparam[CSIZE_INDEX];
    lSize = cacheparam[LSIZE_INDEX];

    if (tilesize > 0) {
        NumLines = cSize / lSize;
        access = a_list->element;
        cLine = access->cline;
        cInst = access->cinst;
        a_list = a_list->next;
        while (a_list != NULL) {
            access = a_list->element;
            if (cInst != access->cinst) {
                tilesize = MIN (tilesize, (access->cline - cLine));
                cInst = access->cinst;
            }
            cLine = access->cline;
            a_list = a_list->next;
        }
        cInst++;
        access = acc_list->element;
        if (cInst != access->cinst) {
            tilesize = MIN (tilesize, (NumLines - cLine + access->cline));
        }
    }

    DBUG_RETURN (tilesize);
}

/*****************************************************************************
 *
 * function:
 *   list_t* RecreateEnhAccesslist(list_t* acc_list, node* arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/

static list_t *
RecreateEnhAccesslist (list_t *acc_list, node *arg_info)
{
    int dim, cSize, lSize, dType, *cacheparam;
    int vaddr, abscl, cline, cinst, aline;
    list_t *new_list = NULL;
    shpseg *shape;
    enh_access_t *eaccess, *element;

    DBUG_ENTER ("RecreateEnhAccesslist");

    dim = INFO_TSI_ARRAYDIM (arg_info);
    shape = INFO_TSI_ARRAYSHP (arg_info);
    cacheparam = INFO_TSI_CACHEPARAM (arg_info);
    cSize = cacheparam[CSIZE_INDEX];
    lSize = cacheparam[LSIZE_INDEX];
    dType = cacheparam[DTYPE_INDEX];

    if (acc_list != NULL) {
        element = (enh_access_t *)acc_list->element;
        vaddr = element->vaddr + (dType * SHPSEG_SHAPE (shape, (dim - 1)));
        if (vaddr < 0) {
            if (lSize > dType) {
                abscl = (vaddr - lSize + dType) / lSize;
            } else {
                abscl = vaddr / lSize;
            }
            cline = (cSize / lSize) + (abscl % (cSize / lSize));
        } else {
            abscl = vaddr / lSize;
            cline = abscl % (cSize / lSize);
        }
        cinst = abscl / (cSize / lSize);
        aline = element->aline;
        eaccess = (enh_access_t *)Malloc (sizeof (enh_access_t));
        eaccess->vaddr = vaddr;
        eaccess->cline = cline;
        eaccess->cinst = cinst;
        eaccess->aline = aline + 1;
        new_list = RecreateEnhAccesslist (acc_list->next, arg_info);

        new_list = InsortByCLine (eaccess, new_list);
        if (aline > INFO_TSI_MINLINE (arg_info)) {
            new_list = InsortByCLine (element, new_list);
        }
        free (acc_list);
    }

    DBUG_RETURN (new_list);
}

/*****************************************************************************
 *
 * function:
 *   list_t* CreateEnhAccesslist(access_t* accesses, node* arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/

static list_t *
CreateEnhAccesslist (access_t *accesses, node *arg_info)
{
    int dim, cSize, lSize, dType, *cacheparam;
    int vaddr, abscl, cline, cinst, aline;
    int minline, maxline, j;
    shpseg *shape;
    list_t *acc_list = NULL;
    enh_access_t *eaccess;

    DBUG_ENTER ("CreateEnhAccesslist");

    dim = INFO_TSI_ARRAYDIM (arg_info);
    shape = INFO_TSI_ARRAYSHP (arg_info);
    cacheparam = INFO_TSI_CACHEPARAM (arg_info);
    cSize = cacheparam[CSIZE_INDEX];
    lSize = cacheparam[LSIZE_INDEX];
    dType = cacheparam[DTYPE_INDEX];
    minline = INFO_TSI_MINLINE (arg_info);
    maxline = INFO_TSI_MAXLINE (arg_info);

    while (accesses != NULL) {
        vaddr = SHPSEG_SHAPE (ACCESS_OFFSET (accesses), (dim - 1));
        for (j = dim - 2; j >= 0; j--) {
            vaddr += SHPSEG_SHAPE (shape, (SHPSEG_SHAPE (ACCESS_OFFSET (accesses), j)
                                           * (j + 1)));
        }
        vaddr *= dType;
        if ((vaddr < 0) && (lSize > dType)) {
            abscl = (vaddr - lSize + dType) / lSize;
            cline = (cSize / lSize) - (abscl % (cSize / lSize));
        } else {
            abscl = vaddr / lSize;
            cline = abscl % (cSize / lSize);
        }
        cinst = abscl / (cSize / lSize);
        aline = SHPSEG_SHAPE (ACCESS_OFFSET (accesses), (dim - 2));
        minline = MIN (aline, minline);
        maxline = MAX (aline, maxline);
        eaccess = (enh_access_t *)Malloc (sizeof (enh_access_t));
        eaccess->vaddr = vaddr;
        eaccess->cline = cline;
        eaccess->cinst = cinst;
        eaccess->aline = aline;
        acc_list = InsortByCLine (eaccess, acc_list);
        accesses = ACCESS_NEXT (accesses);
    }
    INFO_TSI_MINLINE (arg_info) = minline;
    INFO_TSI_MAXLINE (arg_info) = maxline;

    DBUG_RETURN (acc_list);
}

/*****************************************************************************
 *
 * function:
 *   int CalcTilesizeInnerDim(access_t* accesses, node* arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/

static int
CalcTilesizeInnerDim (access_t *accesses, node *arg_info)
{
    int tilesize, minline, maxline, i, dim;
    list_t *acc_list;
    shpseg *shape;

    DBUG_ENTER ("CalcTilesizeInnerDim");

    INFO_TSI_MINLINE (arg_info) = INT_MAX;
    INFO_TSI_MAXLINE (arg_info) = 0;
    dim = INFO_TSI_ARRAYDIM (arg_info);
    shape = INFO_TSI_ARRAYSHP (arg_info);
    tilesize = SHPSEG_SHAPE (INFO_TSI_ARRAYSHP (arg_info), (dim - 1));
    if (dim < 2) {
        tilesize = 0;
        /* No tiling required !  No #pragma wlcomp allowed ! */
    } else if (accesses == NULL) {
        tilesize = 0;
        /* No tiling required !  No #pragma wlcomp allowed ! */
    } else if (INFO_TSI_INDEXDIM (arg_info) != INFO_TSI_ARRAYDIM (arg_info)) {
        tilesize = 0;
        /* No tiling required !  No #pragma wlcomp allowed ! */
    } else {
        acc_list = CreateEnhAccesslist (accesses, arg_info);
        DBUG_ASSERT ((acc_list != NULL),
                     ("Tiling without accesses ? Accesses exspected!"));

        tilesize = MinDistance (acc_list, tilesize, arg_info);
        minline = INFO_TSI_MINLINE (arg_info);
        maxline = INFO_TSI_MAXLINE (arg_info);
        for (i = 0; i < (maxline - minline); i++) {
            fprintf (stderr, "*** tilesizing ... %d\n", tilesize);
            acc_list = RecreateEnhAccesslist (acc_list, arg_info);
            tilesize = MinDistance (acc_list, tilesize, arg_info);
        }
        if ((tilesize == (INFO_TSI_CACHESIZE (arg_info) / INFO_TSI_LINESIZE (arg_info)))
            || (tilesize >= SHPSEG_SHAPE (shape, (dim - 1))) || (tilesize < 1))
            tilesize = 1;
        fprintf (stderr, "*** tilesize found: %d\n", tilesize);
    }

    DBUG_RETURN (tilesize);
}

/*****************************************************************************
 *
 * function:
 *   node *TileSizeInference(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   This function initiates the tile size inference scheme, i.e.
 *   act_tab is set to tsi_tab and the traversal mechanism is started.
 *   Just as the other optimization schemes, tile size selection is performed
 *   on single function definitions rather than on the entire syntax tree.
 *
 *****************************************************************************/

node *
TileSizeInference (node *arg_node)
{
    node *arg_info;
    int *cacheparam;
    funptr *tmp_tab;

    DBUG_ENTER ("TileSizeInference");

    tmp_tab = act_tab;
    act_tab = tsi_tab;

    arg_info = MakeInfo ();

    if ((config.cache1_size > 0) && (config.cache1_line > 0)) {

        cacheparam = (int *)Malloc (NUM_OF_CACHEPARAM * sizeof (int));
        cacheparam[CSIZE_INDEX] = 1024 * config.cache1_size;
        cacheparam[LSIZE_INDEX] = config.cache1_line;
        INFO_TSI_CACHEPARAM (arg_info) = cacheparam;

        arg_node = Trav (arg_node, arg_info);

        FREE (cacheparam);
    } else {
        /*
         *   If there's no target specified, it's not possibile to infere a tilesize,
         *   because the TSI have to know the cache parameters.
         */
        SYSWARN (("No target specified. No TSI possible!"));
    }

    FREE (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_fundef node.
 *
 *   The traversal is limited to the function body, arguments and remaining
 *   functions are not traversed.
 *
 ******************************************************************************/

node *
TSIfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        /*
         * Nodetype of FUNDEF_BODY(arg_node) is N_block.
         */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIblock(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_block node.
 *
 *   The traversal is limited to the assignments chain, variable declarations
 *   are not traversed.
 *
 *
 ******************************************************************************/

node *
TSIblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSIblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
        /*
         * Nodetype of BLOCK_INSTR(arg_node) is N_assign or N_empty.
         */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSInwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_Nwith node.
 *
 *
 ******************************************************************************/

node *
TSInwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TSInwith");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Nwith),
                 "Tile size selection not initiated on N_Nwith level");

    /* DBUG_EXECUTE("TSI_INFO",PrintNodeTree(arg_node);); */
    if ((NWITHOP_TYPE (NWITH_WITHOP (arg_node)) == WO_genarray)
        || (NWITHOP_TYPE (NWITH_WITHOP (arg_node)) == WO_modarray)) {

        INFO_TSI_WLCOMP (arg_info) = NULL;
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
        if (NWITH_PRAGMA (arg_node) == NULL) {
            NWITH_PRAGMA (arg_node) = INFO_TSI_WLCOMP (arg_info);
            INFO_TSI_WLCOMP (arg_info) = NULL;
        } else {
            printf ("*** Tiling, manually set:");
            Print (NWITH_PRAGMA (arg_node));
            printf ("*** Tiling, tsi-proposal:");
            Print (INFO_TSI_WLCOMP (arg_info));
            FreePragma (INFO_TSI_WLCOMP (arg_info), NULL);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIncode(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_Ncode node.
 *
 *
 ******************************************************************************/

node *
TSIncode (node *arg_node, node *arg_info)
{
    int i, tilesize, *cacheparam;
    char *ap_name;
    node *pragma, *aelems;

    DBUG_ENTER ("TSIncode");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Ncode),
                 "Tile size selection not initiated on N_Ncode level");

    cacheparam = INFO_TSI_CACHEPARAM (arg_info);
    cacheparam[DTYPE_INDEX]
      = dtype_size[TYPES_BASETYPE (VARDEC_TYPE (NCODE_WLAA_WLARRAY (arg_node)))];
    INFO_TSI_WLARRAY (arg_info) = NCODE_WLAA_WLARRAY (arg_node);
    INFO_TSI_INDEXVAR (arg_info) = NCODE_WLAA_INDEXVAR (arg_node);
    INFO_TSI_ARRAYDIM (arg_info) = VARDEC_DIM (NCODE_WLAA_WLARRAY (arg_node));
    INFO_TSI_ACCESS (arg_info) = NCODE_WLAA_ACCESS (arg_node);
    INFO_TSI_ACCESSCNT (arg_info) = NCODE_WLAA_ACCESSCNT (arg_node);
    INFO_TSI_ARRAYSHP (arg_info) = VARDEC_SHPSEG (NCODE_WLAA_WLARRAY (arg_node));

    DBUG_ASSERT ((INFO_TSI_ARRAYSHP (arg_info) != NULL), ("Array without shape!"));

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    tilesize = CalcTilesizeInnerDim (NCODE_WLAA_ACCESS (arg_node), arg_info);

    if (tilesize > 0) {
        aelems = NULL;
        for (i = INFO_TSI_ARRAYDIM (arg_info) - 1; i >= 0; i--) {
            aelems = MakeExprs (MakeNum (tilesize), aelems);
        }
        pragma = MakePragma ();
        ap_name = Malloc (6 * sizeof (char));
        ap_name = strcpy (ap_name, "BvL0");
        PRAGMA_WLCOMP_APS (pragma)
          = MakeExprs (MakeAp (ap_name, NULL, MakeExprs (MakeArray (aelems), NULL)),
                       NULL);
    } else {
        /*
         *  No #pragma wlcomp required or allowed.
         */
        pragma = NULL;
    }

    INFO_TSI_WLCOMP (arg_info) = pragma;

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
