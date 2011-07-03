/*
 *
 * $Id$
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
 *      INFO_TSI_ACCESSCNT(n)
 *      INFO_TSI_ARRAYDIM(n)                 (see tree_compound.h)
 *      INFO_TSI_ARRAYSHP(n)                 (see tree_compound.h)
 *      INFO_TSI_TILESHP(n)
 *      INFO_TSI_CACHEPAR(n)
 *      INFO_TSI_WLARRAY(n)
 *
 *****************************************************************************/

#include <stdlib.h>
#include <limits.h>

#define DBUG_PREFIX "PRINT_TSI"
#include "debug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "resource.h"
#include "DupTree.h"
#include "print.h"
#include "tile_size_inference.h"

#ifndef TSI_DEACTIVATED
/*
 * INFO structure
 */
struct INFO {
    access_t *access;
    int accesscnt;
    int minline;
    int maxline;
    int feature;
    int wlcomp;
    shpseg *tileshp;
    node *indexvar;
    node *wlarray;
    int *cacheparam;
};

/*
 * INFO macros
 */
#define INFO_TSI_ACCESS(n) (n->access)
#define INFO_TSI_ACCESSCNT(n) (n->accesscnt)
#define INFO_TSI_MINLINE(n) (n->minline)
#define INFO_TSI_MAXLINE(n) (n->maxline)
#define INFO_TSI_FEATURE(n) (n->feature)
#define INFO_TSI_WLCOMP(n) (n->wlcomp)
#define INFO_TSI_TILESHP(n) (n->tileshp)
#define INFO_TSI_INDEXVAR(n) (n->indexvar)
#define INFO_TSI_WLARRAY(n) (n->wlarray)
#define INFO_TSI_CACHEPARAM(n) (n->cacheparam)

/* compound macros */
#define INFO_TSI_CACHESIZE(n) (INFO_TSI_CACHEPARAM (n)[0])
#define INFO_TSI_LINESIZE(n) (INFO_TSI_CACHEPARAM (n)[1])
#define INFO_TSI_DATATYPE(n) (INFO_TSI_CACHEPARAM (n)[2])
#define INFO_TSI_ARRAYSHP(n) VARDEC_SHPSEG (INFO_TSI_WLARRAY (n))
#define INFO_TSI_INDEXDIM(n) VARDEC_SHAPE (INFO_TSI_INDEXVAR (n), 0)
#define INFO_TSI_ARRAYDIM(n) VARDEC_DIM (INFO_TSI_WLARRAY (n))

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = Malloc (sizeof (info));

    INFO_TSI_ACCESS (result) = NULL;
    INFO_TSI_ACCESSCNT (result) = 0;
    INFO_TSI_MINLINE (result) = 0;
    INFO_TSI_MAXLINE (result) = 0;
    INFO_TSI_FEATURE (result) = 0;
    INFO_TSI_WLCOMP (result) = 0;
    INFO_TSI_TILESHP (result) = NULL;
    INFO_TSI_INDEXVAR (result) = NULL;
    INFO_TSI_WLARRAY (result) = NULL;
    INFO_TSI_CACHEPARAM (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = Free (info);

    DBUG_RETURN (info);
}

#define MINTILE 20

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
 *   list_t* InSortByCLine(void* element, list_t* list)
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
        if (elem->cline <= next->cline) {
            if (elem->vaddr == next->vaddr) {
                elem = Free (elem);
            } else {
                result = (list_t *)Malloc (sizeof (list_t));
                result->element = element;
                result->next = list;
            }
        } else {
            result->next = InsortByCLine (element, list->next);
        }
    }
    return (result);
}

/*****************************************************************************
 *
 * function:
 *    int MinDistance(list_t* acc_list, int mindist, node* arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/

static int
MinDistance (list_t *acc_list, int mindist, info *arg_info)
{
    int cLine, cInst, NumLines, cSize, lSize, dType, NumElems, *cacheparam;
    list_t *a_list = acc_list;
    enh_access_t *access;

    DBUG_ENTER ();

    cacheparam = INFO_TSI_CACHEPARAM (arg_info);
    cSize = cacheparam[CSIZE_INDEX];
    lSize = cacheparam[LSIZE_INDEX];
    dType = cacheparam[DTYPE_INDEX];
    NumLines = cSize / lSize;
    NumElems = lSize / dType;

    if (mindist > 0) {
        access = a_list->element;
        cLine = access->cline;
        cInst = access->cinst;
        a_list = a_list->next;
        while (a_list != NULL) {
            access = a_list->element;
            if (cInst != access->cinst) {
                mindist = MIN (mindist, NumElems * (access->cline - cLine));
                cInst = access->cinst;
            }
            cLine = access->cline;
            a_list = a_list->next;
        }
        cInst++;
        access = acc_list->element;
        if (cInst != access->cinst) {
            mindist = MIN (mindist, NumElems * (NumLines - cLine + access->cline));
        }
    }

    DBUG_RETURN (mindist);
}

/*****************************************************************************
 *
 * function:
 *    int ComputeVaddr(shpseg* access, shpseg* shape, int dim, int data)
 *
 * description:
 *
 *
 *****************************************************************************/

static int
ComputeVaddr (shpseg *access, info *arg_info)
{
    int vaddr, j, dim, dType, *cacheparam;
    shpseg *shape;

    dim = INFO_TSI_ARRAYDIM (arg_info);
    shape = INFO_TSI_ARRAYSHP (arg_info);
    cacheparam = INFO_TSI_CACHEPARAM (arg_info);
    dType = cacheparam[DTYPE_INDEX];
    /*
     *  computing virtual address [elements]:
     */
    vaddr = SHPSEG_SHAPE (access, (dim - 1));
    for (j = dim - 2; j >= 0; j--) {
        vaddr += (SHPSEG_SHAPE (shape, (j + 1))) * (SHPSEG_SHAPE (access, j));
    }
    /*
     *  computing virtual address [byte]:
     */
    vaddr *= dType;

    return (vaddr);
}

/*****************************************************************************
 *
 * function:
 *   enh_access_t* ComputeEnhAccess(int vaddr, node arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/

static enh_access_t *
ComputeEnhAccess (int vaddr, info *arg_info)
{
    int dim, cSize, lSize, dType, *cacheparam;
    int abscl, cline, cinst;
    shpseg *shape;
    enh_access_t *eaccess;

    dim = INFO_TSI_ARRAYDIM (arg_info);
    shape = INFO_TSI_ARRAYSHP (arg_info);
    cacheparam = INFO_TSI_CACHEPARAM (arg_info);
    cSize = cacheparam[CSIZE_INDEX];
    lSize = cacheparam[LSIZE_INDEX];
    dType = cacheparam[DTYPE_INDEX];

    /*
     *  computing absolute cache line, cache line and cache instance:
     */
    if ((vaddr < 0) && (lSize > dType)) {
        abscl = (vaddr - lSize + dType) / lSize;
        cline = (cSize / lSize) + (abscl % (cSize / lSize));
        /* 2nd summand is negative ! */
        cinst = (abscl + 1 - (cSize / lSize)) / (cSize / lSize);
    } else {
        abscl = vaddr / lSize;
        cline = abscl % (cSize / lSize);
        cinst = abscl / (cSize / lSize);
    }
    eaccess = (enh_access_t *)Malloc (sizeof (enh_access_t));
    eaccess->vaddr = vaddr;
    eaccess->cline = cline;
    eaccess->cinst = cinst;

    return (eaccess);
}

/*****************************************************************************
 *
 * function:
 *   list_t* RecreateEnhAccesslist(list_t* acc_list, node* arg_info)
 *
 * description:
 *   for all accesses a=[a_1,...,a_n] in access_list do:
 *       b = [a_1,...,a_(n-2),a_(n-1)+1,an];
 *       access_list = access_list + b;
 *   For all accesses a=[a_1,...,a_n] in the access_list
 *   where a_(n-1) <= b_(n-1) for all b=[b1,...,b_n] in access_list do:
 *       access_list = access_list - b;
 *
 *****************************************************************************/

static list_t *
RecreateEnhAccesslist (list_t *acc_list, info *arg_info)
{
    int dim, cSize, lSize, dType, *cacheparam;
    int vaddr, aline;
    list_t *new_list = NULL;
    list_t *help_list = NULL;
    shpseg *shape;
    enh_access_t *eaccess, *element;

    DBUG_ENTER ();

    dim = INFO_TSI_ARRAYDIM (arg_info);
    shape = INFO_TSI_ARRAYSHP (arg_info);
    cacheparam = INFO_TSI_CACHEPARAM (arg_info);
    cSize = cacheparam[CSIZE_INDEX];
    lSize = cacheparam[LSIZE_INDEX];
    dType = cacheparam[DTYPE_INDEX];

    while (acc_list != NULL) {

        help_list = acc_list;
        element = (enh_access_t *)acc_list->element;
        /*
         *  computing virtual address [byte]:
         */
        vaddr = element->vaddr + (dType * SHPSEG_SHAPE (shape, (dim - 1)));

        /*
         *  computing array line:
         */
        aline = (element->aline) + 1;

        /*
         *  computing absolute cache line, cache line and cache instance:
         */
        eaccess = ComputeEnhAccess (vaddr, arg_info);
        eaccess->aline = aline;
        new_list = InsortByCLine (eaccess, new_list);

        if (aline > INFO_TSI_MINLINE (arg_info)) {
            new_list = InsortByCLine (element, new_list);
        } else {
            free (element);
        }
        acc_list = acc_list->next;
        free (help_list);
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
CreateEnhAccesslist (access_t *accesses, info *arg_info)
{
    int vaddr, aline, minline, maxline, dim;
    list_t *acc_list = NULL;
    enh_access_t *eaccess;

    DBUG_ENTER ();

    minline = INT_MAX;
    maxline = INT_MIN;
    dim = INFO_TSI_ARRAYDIM (arg_info);

    DBUG_ASSERT (ACCESS_CLASS (accesses) == ACL_offset,
                 "Access without ACL_offset found !!");

    while (accesses != NULL) {

        DBUG_ASSERT (ACCESS_OFFSET (accesses) != NULL,
                     "Access with ACL_offset found, but no offset !!");
        /*
         *  computing virtual address [byte]:
         */
        vaddr = ComputeVaddr (ACCESS_OFFSET (accesses), arg_info);

        /*
         *  computing array line:
         */
        aline = SHPSEG_SHAPE (ACCESS_OFFSET (accesses), (dim - 2));

        /*
         *  computing absolute cache line, cache line and cache instance:
         */
        eaccess = ComputeEnhAccess (vaddr, arg_info);
        eaccess->aline = aline;
        minline = MIN (aline, minline);
        maxline = MAX (aline, maxline);
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
 *   int CalcTSInnerDim(access_t* accesses, node* arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/

static int
CalcTSInnerDim (list_t *acc_list, info *arg_info)
{
    int minline, maxline, i, dim, maxsize, tilesize;
    int *cacheparam, cSize, lSize, dType;
    shpseg *shape;

    DBUG_ENTER ();

    cacheparam = INFO_TSI_CACHEPARAM (arg_info);
    cSize = cacheparam[CSIZE_INDEX];
    lSize = cacheparam[LSIZE_INDEX];
    dType = cacheparam[DTYPE_INDEX];
    dim = INFO_TSI_ARRAYDIM (arg_info);
    shape = INFO_TSI_ARRAYSHP (arg_info);
    minline = INFO_TSI_MINLINE (arg_info);
    maxline = INFO_TSI_MAXLINE (arg_info);
    maxsize = MIN ((SHPSEG_SHAPE (shape, (dim - 1))), (dType * cSize / lSize));
    tilesize = MinDistance (acc_list, maxsize, arg_info);

    DBUG_EXECUTE (fprintf (stderr, "*** tilesizing ... %d\n", tilesize));

    for (i = 0; i < (maxline - minline); i++) {
        acc_list = RecreateEnhAccesslist (acc_list, arg_info);
    }
    tilesize = MinDistance (acc_list, maxsize, arg_info);

    DBUG_EXECUTE (fprintf (stderr, "*** tilesizing ... %d\n", tilesize));

    if ((tilesize >= maxsize) || (tilesize <= lSize)) {
        tilesize = SHPSEG_SHAPE (shape, (dim - 1));
    }
    if (tilesize < MINTILE) {
        tilesize = SHPSEG_SHAPE (shape, (dim - 1));
    }

    DBUG_EXECUTE (fprintf (stderr, "*** tilesize found: %d\n", tilesize));

    DBUG_RETURN (tilesize);
}

/*****************************************************************************
 *
 * function:
 *   int CalcTSOuterDims(access_t* accesses, node* arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/

static int
CalcTSOuterDims (list_t *acc_list, int index, info *arg_info)
{
    int size, dim;
    shpseg *shape;

    DBUG_ENTER ();

    dim = INFO_TSI_ARRAYDIM (arg_info);
    shape = INFO_TSI_ARRAYSHP (arg_info);
    size = SHPSEG_SHAPE (shape, index);

    DBUG_RETURN (size);
}

/*****************************************************************************
 *
 * function:
 *   int CalcTilesize(access_t* accesses, node* arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/

static int
CalcTilesize (access_t *accesses, info *arg_info)
{
    int tilesize, i, dim;
    list_t *acc_list;
    shpseg *shape, *tileshp;

    DBUG_ENTER ();

    dim = INFO_TSI_ARRAYDIM (arg_info);
    shape = INFO_TSI_ARRAYSHP (arg_info);
    tileshp = INFO_TSI_TILESHP (arg_info);

    if (dim < 2) {
        tilesize = SHPSEG_SHAPE (shape, (dim - 1));
        /*
         *  No tiling required !  No #pragma wlcomp allowed !
         */
    } else if (accesses == NULL) {
        tilesize = 0;
        /*
         *  No tiling required !  No #pragma wlcomp allowed !
         */
    } else if (INFO_TSI_INDEXDIM (arg_info) != INFO_TSI_ARRAYDIM (arg_info)) {
        tilesize = 0;
        /*
         *  No tiling required !  No #pragma wlcomp allowed !
         */
    } else {
        acc_list = CreateEnhAccesslist (accesses, arg_info);

        DBUG_ASSERT (acc_list != NULL, "Tiling without accesses ? Accesses exspected!");

        tilesize = CalcTSInnerDim (acc_list, arg_info);
        SHPSEG_SHAPE (tileshp, (dim - 1)) = tilesize;

        for (i = dim - 2; i >= 0; i--) {
            SHPSEG_SHAPE (tileshp, i) = CalcTSOuterDims (acc_list, i, arg_info);
            tilesize = MIN (tilesize, SHPSEG_SHAPE (tileshp, i));
        }
    }

    DBUG_RETURN (tilesize);
}

/*****************************************************************************
 *
 * function:
 *   node* TSIMakePragmaWLComp(node* aelems, node* arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/

static node *
TSIMakePragmaWLComp (int tilesize, info *arg_info)
{
    node *pragma, *aelems;
    char *ap_name;
    int i, dim;
    shpseg *shape;

    DBUG_ENTER ();

    dim = INFO_TSI_ARRAYDIM (arg_info);
    shape = INFO_TSI_ARRAYSHP (arg_info);
    aelems = NULL;

    if (tilesize > 0) {
        tilesize = SHPSEG_SHAPE (INFO_TSI_TILESHP (arg_info), (dim - 1));
        if (tilesize < SHPSEG_SHAPE (shape, (dim - 1))) {
            for (i = (dim - 1); i >= 0; i--) {
                tilesize = SHPSEG_SHAPE (INFO_TSI_TILESHP (arg_info), i);
                aelems = MakeExprs (MakeNum (tilesize), aelems);
            }
            pragma = MakePragma ();
            ap_name = Malloc (6 * sizeof (char));
            ap_name = strcpy (ap_name, "BvL0");
            PRAGMA_WLCOMP_APS (pragma)
              = MakeExprs (MakeAp (ap_name, NULL,
                                   MakeExprs (MakeFlatArray (aelems), NULL)),
                           NULL);
        } else {
            /*
             *  No #pragma wlcomp required.
             */
            DBUG_EXECUTE (fprintf (stderr, "*** No #pragma wlcomp required.\n"));

            pragma = NULL;
        }
    } else {
        /*
         *  No #pragma wlcomp allowed.
         */
        DBUG_EXECUTE (fprintf (stderr, "*** No #pragma wlcomp allowed!\n"));

        pragma = NULL;
    }

    DBUG_RETURN (pragma);
}

/*****************************************************************************/

/*****************************************************************************
 *
 * function:
 *   node *TileSizeInference(node *arg_node, info *arg_info)
 *
 * description:
 *
 *   This function initiates the tile size inference scheme, i.e.
 *   act_tab is set to tsi_tab and the traversal mechanism is started.
 *   Just as the other optimization schemes, tile size selection is performed
 *   on single function definitions rather than on the entire syntax tree.
 *
 *****************************************************************************/
#endif /* TSI_DEACTIVATED*/

node *
TSIdoTileSizeInference (node *arg_node)
{
    DBUG_ENTER ();

#ifndef TSI_DEACTIVATED

    info *arg_info;

    DBUG_PRINT_TAG ("TSI", "TSIdoTileSizeInference");

    arg_info = MakeInfo ();

    if ((config.cache1_size > 0) && (config.cache1_line > 0)) {
        TRAVpush (TR_tsi);
        arg_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();
    } else {
        /*
         *   If there's no target specified, it's not possibile to infere a
         *   tilesize, because the TSI have to know the cache parameters.
         */
        DBUG_PRINT ("No target specified. No TSI possible!");
    }

    arg_info = FreeInfo (arg_info);

#endif /* TSI_DEACTIVATED */

    DBUG_RETURN (arg_node);
}

#ifndef TSI_DEACTIVATED

/******************************************************************************
 *
 * function:
 *   node *TSIfundef(node *arg_node, info *arg_info)
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
TSIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("TSI", "TSIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        /*
         * Nodetype of FUNDEF_BODY(arg_node) is N_block.
         */
    }
#if 1
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }
#endif

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIblock(node *arg_node, info *arg_info)
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
TSIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("TSI", "TSIblock");

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
 *   node *TSInwith(node *arg_node, info *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_Nwith node.
 *
 *
 ******************************************************************************/

node *
TSInwith (node *arg_node, info *arg_info)
{
    node *pragma;
    int *cacheparam;

    DBUG_ENTER ();

    DBUG_PRINT_TAG ("TSI", "TSInwith");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_Nwith,
                 "Tile size selection not initiated on N_Nwith level");

    cacheparam = (int *)Malloc (NUM_OF_CACHEPARAM * sizeof (int));
    cacheparam[CSIZE_INDEX] = 1024 * config.cache1_size;
    cacheparam[LSIZE_INDEX] = config.cache1_line;
    INFO_TSI_CACHEPARAM (arg_info) = (void *)cacheparam;

    /* DBUG_EXECUTE("TSI_INFO",PrintNodeTree(arg_node);); */
    if ((NWITHOP_TYPE (NWITH_WITHOP (arg_node)) == WO_genarray)
        || (NWITHOP_TYPE (NWITH_WITHOP (arg_node)) == WO_modarray)) {

        INFO_TSI_WLCOMP (arg_info) = INT_MAX;

        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

        pragma = TSIMakePragmaWLComp (INFO_TSI_WLCOMP (arg_info), arg_info);

        if (NWITH_PRAGMA (arg_node) != NULL) {
            DBUG_EXECUTE (printf ("*** Tiling, manually set:"));
            DBUG_EXECUTE (Print (NWITH_PRAGMA (arg_node)));
            DBUG_EXECUTE (printf ("*** Tiling, tsi-proposal:"));
            if (pragma != NULL) {
                DBUG_EXECUTE (Print (pragma));
                FreePragma (pragma, NULL);
            } else {
                DBUG_EXECUTE (printf ("\n*** #pragma wlcomp already exists.\n\n"));
            }
        } else {
            NWITH_PRAGMA (arg_node) = pragma;
            DBUG_EXECUTE (printf ("*** Tiling, tsi-proposal:"));
            if (pragma != NULL) {
                DBUG_EXECUTE (Print (pragma));
            } else {
                DBUG_EXECUTE (printf ("\n*** No tsi-proposal possible.\n\n"));
            }
        }
    }
    cacheparam = Free (cacheparam);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TSIncode(node *arg_node, info *arg_info)
 *
 * description:
 *
 *   Syntax tree traversal function for N_Ncode node.
 *
 *
 ******************************************************************************/

node *
TSIncode (node *arg_node, info *arg_info)
{
    int *cacheparam;

    DBUG_ENTER ();

    DBUG_PRINT_TAG ("TSI", "TSIncode");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_Ncode,
                 "Tile size selection not initiated on N_Ncode level");

    cacheparam = (int *)INFO_TSI_CACHEPARAM (arg_info);
    cacheparam[DTYPE_INDEX]
      = basetype_size[TYPES_BASETYPE (VARDEC_TYPE (NCODE_WLAA_WLARRAY (arg_node)))];
    INFO_TSI_WLARRAY (arg_info) = NCODE_WLAA_WLARRAY (arg_node);
    INFO_TSI_INDEXVAR (arg_info) = NCODE_WLAA_INDEXVAR (arg_node);
    INFO_TSI_ACCESS (arg_info) = NCODE_WLAA_ACCESS (arg_node);
    INFO_TSI_FEATURE (arg_info) = NCODE_WLAA_FEATURE (arg_node);
    INFO_TSI_ACCESSCNT (arg_info) = NCODE_WLAA_ACCESSCNT (arg_node);
    INFO_TSI_TILESHP (arg_info) = DupShpseg (INFO_TSI_ARRAYSHP (arg_info));

    DBUG_ASSERT (INFO_TSI_ARRAYSHP (arg_info) != NULL, "Array without shape!");

    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    if ((INFO_TSI_FEATURE (arg_info) == FEATURE_NONE)
        && (INFO_TSI_ACCESSCNT (arg_info) > 2)) {
        /*
         *  If the number of accesses is less than 3, then there can only be one or
         *  no read-access. In this case tiling cannot rise performance.
         */
        INFO_TSI_WLCOMP (arg_info)
          = CalcTilesize (NCODE_WLAA_ACCESS (arg_node), arg_info);
        NCODE_TSI_TILESHP (arg_node) = INFO_TSI_TILESHP (arg_info);
    } else {
        INFO_TSI_WLCOMP (arg_info) = 0;
        NCODE_TSI_TILESHP (arg_node) = NULL;
    }

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

#endif /* TSI_DEACTIVATED */

#undef DBUG_PREFIX
