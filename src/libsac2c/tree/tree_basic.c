/*
 *
 * $Id$
 *
 */

#include "tree_basic.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"
#include "free.h"
#include "dbug.h"

/*--------------------------------------------------------------------------*/
/*  Make-functions for non-node structures                                  */
/*--------------------------------------------------------------------------*/

/*
 * attention: the given parameter chain of nums structs is set free here!!!
 */
shpseg *
TBmakeShpseg (node *numsp)
{
    shpseg *tmp;
    int i;
    node *oldnumsp;

    DBUG_ENTER ("TBmakeShpseg");

    tmp = (shpseg *)MEMmalloc (sizeof (shpseg));

#ifndef DBUG_OFF
    /*
     * For debugging memory use with dbx, it is important
     * that all "memory.has been initialised before reading
     * from it. As the Shpseg is allocated in a fixed size
     * which may not be entirely filled afterwards, we
     * have to write an initial value! Otherwise dbx will
     * complain that for example in DupTree uninitialised
     * data is read.
     */
    for (i = 0; i < SHP_SEG_SIZE; i++) {
        SHPSEG_SHAPE (tmp, i) = -1;
    }
#endif

    i = 0;
    while (numsp != NULL) {
        if (i >= SHP_SEG_SIZE) {
            CTIabort ("Maximum number of dimensions exceeded");
        }

        DBUG_ASSERT ((NODE_TYPE (numsp) == N_nums), "found a non numsp node as argument");

        SHPSEG_SHAPE (tmp, i) = NUMS_VAL (numsp);

        i++;
        oldnumsp = numsp;
        numsp = NUMS_NEXT (numsp);
        oldnumsp = FREEdoFreeNode (oldnumsp);
    }

    SHPSEG_NEXT (tmp) = NULL;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

types *
TBmakeTypes1 (simpletype btype)
{
    types *tmp;

    DBUG_ENTER ("TBmakeTypes1");

    tmp = TBmakeTypes (btype, 0, NULL, NULL, NULL);

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

types *
TBmakeTypes (simpletype btype, int dim, shpseg *shpseg, char *name, char *mod)
{
    types *tmp;

    DBUG_ENTER ("TBmakeTypes");

    tmp = (types *)MEMmalloc (sizeof (types));

    TYPES_BASETYPE (tmp) = btype;
    TYPES_NAME (tmp) = name;
    TYPES_MOD (tmp) = mod;
    TYPES_SHPSEG (tmp) = shpseg;
    TYPES_DIM (tmp) = dim;
    TYPES_POLY (tmp) = FALSE;

    TYPES_MUTC_SCOPE (tmp) = MUTC_GLOBAL;
    TYPES_MUTC_USAGE (tmp) = MUTC_US_DEFAULT;

    TYPES_TDEF (tmp) = NULL;
    TYPES_NEXT (tmp) = NULL;

    TYPES_MUTC_SCOPE (tmp) = MUTC_GLOBAL;
    TYPES_MUTC_USAGE (tmp) = MUTC_US_DEFAULT;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

nodelist *
TBmakeNodelistNode (node *node, nodelist *next)
{
    nodelist *tmp;

    DBUG_ENTER ("TBmakeNodelistNode");

    tmp = (nodelist *)MEMmalloc (sizeof (nodelist));
    NODELIST_NODE (tmp) = node;
    NODELIST_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

access_t *
TBmakeAccess (node *array, node *iv, accessclass_t class, shpseg *offset,
              accessdir_t direction, access_t *next)
{
    access_t *tmp;

    DBUG_ENTER ("TBmakeAccess");

    tmp = (access_t *)MEMmalloc (sizeof (access_t));

    ACCESS_ARRAY (tmp) = array;
    ACCESS_IV (tmp) = iv;
    ACCESS_CLASS (tmp) = class;
    ACCESS_OFFSET (tmp) = offset;
    ACCESS_DIR (tmp) = direction;
    ACCESS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

rc_t *
TBmakeReuseCandidate (node *array, int dim, rc_t *next)
{
    rc_t *tmp;
    int i;

    DBUG_ENTER ("TBmakeReuseCandidate");

    tmp = (rc_t *)MEMmalloc (sizeof (rc_t));

    RC_ARRAY (tmp) = array;
    RC_ARRAYSHP (tmp) = NULL;
    RC_SHARRAY (tmp) = NULL;
    RC_SHARRAYSHP (tmp) = NULL;
    RC_DIM (tmp) = dim;
    RC_SELFREF (tmp) = FALSE;
    RC_NEXT (tmp) = next;
    RC_REUSABLE (tmp) = FALSE;

    for (i = 0; i < SHP_SEG_SIZE; i++) {
        RC_NEGOFFSET (tmp, i) = 0;
        RC_POSOFFSET (tmp, i) = 0;
    }

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

index_t *
TBmakeIndex (unsigned int type, int coefficient, node *id, index_t *next)
{
    index_t *idx;

    DBUG_ENTER ("TBmakeIndex");

    idx = (index_t *)MEMmalloc (sizeof (index_t));

    INDEX_TYPE (idx) = type;
    INDEX_COEFFICIENT (idx) = coefficient;
    INDEX_ID (idx) = id;
    INDEX_NEXT (idx) = next;

    DBUG_RETURN (idx);
}

index_t *
TBfreeIndex (index_t *index)
{
    DBUG_ENTER ("TBfreeIndex");

    if (INDEX_NEXT (index) != NULL) {
        INDEX_NEXT (index) = TBfreeIndex (INDEX_NEXT (index));
    }

    index = MEMfree (index);

    DBUG_RETURN (NULL);
}

/*--------------------------------------------------------------------------*/

cuda_access_info_t *
TBmakeCudaAccessInfo (node *array, int dim, int nestlevel)
{
    cuda_access_info_t *info;
    int i;

    DBUG_ENTER ("TBmakeCudaAccessInfo");

    info = (cuda_access_info_t *)MEMmalloc (sizeof (cuda_access_info_t));

    CUAI_MATRIX (info) = NULL;
    CUAI_ARRAY (info) = array;
    CUAI_DIM (info) = dim;
    CUAI_NESTLEVEL (info) = nestlevel;

    for (i = 0; i < MAX_REUSE_DIM; i++) {
        CUAI_INDICES (info, i) = NULL;
    }

    DBUG_RETURN (info);
}

cuda_access_info_t *
TBfreeCudaAccessInfo (cuda_access_info_t *access_info)
{
    int i;

    DBUG_ENTER ("TBfreeCudaAccessInfo");

    for (i = 0; i < MAX_REUSE_DIM; i++) {
        if (CUAI_INDICES (access_info, i) != NULL) {
            CUAI_INDICES (access_info, i) = TBfreeIndex (CUAI_INDICES (access_info, i));
        }
    }

    access_info = MEMfree (access_info);

    DBUG_RETURN (NULL);
}

/*--------------------------------------------------------------------------*/

argtab_t *
TBmakeArgtab (int size)
{
    argtab_t *argtab;
    int i;

    DBUG_ENTER ("TBmakeArgtab");

    argtab = (argtab_t *)MEMmalloc (sizeof (argtab_t));

    argtab->size = size;
    argtab->ptr_in = MEMmalloc (argtab->size * sizeof (node *));
    argtab->ptr_out = MEMmalloc (argtab->size * sizeof (types *));
    argtab->tag = MEMmalloc (argtab->size * sizeof (argtag_t));

    for (i = 0; i < argtab->size; i++) {
        argtab->ptr_in[i] = NULL;
        argtab->ptr_out[i] = NULL;
        argtab->tag[i] = ATG_notag;
    }

    DBUG_RETURN (argtab);
}
