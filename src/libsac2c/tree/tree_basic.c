#include "tree_basic.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"
#include "free.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

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

    DBUG_ENTER ();

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

        DBUG_ASSERT (NODE_TYPE (numsp) == N_nums, "found a non numsp node as argument");

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

nodelist *
TBmakeNodelistNode (node *node, nodelist *next)
{
    nodelist *tmp;

    DBUG_ENTER ();

    tmp = (nodelist *)MEMmalloc (sizeof (nodelist));
    NODELIST_NODE (tmp) = node;
    NODELIST_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

access_t *
TBmakeAccess (node *array, node *iv, accessclass_t mclass, shape *offset,
              accessdir_t direction, access_t *next)
{
    access_t *tmp;

    DBUG_ENTER ();

    tmp = (access_t *)MEMmalloc (sizeof (access_t));

    ACCESS_ARRAY (tmp) = array;
    ACCESS_IV (tmp) = iv;
    ACCESS_CLASS (tmp) = mclass;
    ACCESS_OFFSET (tmp) = offset;
    ACCESS_DIR (tmp) = direction;
    ACCESS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

rc_t *
TBmakeReuseCandidate (node *array, size_t dim, rc_t *next)
{
    rc_t *tmp;
    int i;

    DBUG_ENTER ();

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

cuda_index_t *
TBmakeCudaIndex (unsigned int type, int coefficient, node *id, size_t looplevel,
                 cuda_index_t *next)
{
    cuda_index_t *idx;

    DBUG_ENTER ();

    idx = (cuda_index_t *)MEMmalloc (sizeof (cuda_index_t));

    CUIDX_TYPE (idx) = type;
    CUIDX_COEFFICIENT (idx) = coefficient;
    CUIDX_ID (idx) = id;
    CUIDX_LOOPLEVEL (idx) = looplevel;
    CUIDX_NEXT (idx) = next;

    DBUG_RETURN (idx);
}

cuda_index_t *
TBfreeCudaIndex (cuda_index_t *index)
{
    DBUG_ENTER ();

    if (!index)
        DBUG_RETURN ((cuda_index_t *)NULL);

    if (CUIDX_NEXT (index) != NULL) {
        CUIDX_NEXT (index) = TBfreeCudaIndex (CUIDX_NEXT (index));
    }

    index = MEMfree (index);

    DBUG_RETURN ((cuda_index_t *)NULL);
}

/*--------------------------------------------------------------------------*/

cuda_access_info_t *
TBmakeCudaAccessInfo (node *array, node *arrayshp, int dim, size_t cuwldim, size_t nestlevel)
{
    cuda_access_info_t *info;
    int i;

    DBUG_ENTER ();

    info = (cuda_access_info_t *)MEMmalloc (sizeof (cuda_access_info_t));

    CUAI_MATRIX (info) = NULL;
    CUAI_TYPE (info) = ACCTY_REUSE;
    CUAI_ARRAY (info) = array;
    CUAI_ARRAYSHP (info) = arrayshp;
    CUAI_SHARRAY (info) = NULL;
    CUAI_SHARRAYSHP_PHY (info) = NULL;
    CUAI_SHARRAYSHP_LOG (info) = NULL;
    CUAI_DIM (info) = dim;
    CUAI_NESTLEVEL (info) = nestlevel;
    CUAI_CUWLDIM (info) = cuwldim;
    CUAI_TBSHP (info) = NULL;

    for (i = 0; i < MAX_REUSE_DIM; i++) {
        CUAI_INDICES (info, i) = NULL;
        CUAI_ISCONSTANT (info, i) = TRUE;
    }

    DBUG_RETURN (info);
}

cuda_access_info_t *
TBfreeCudaAccessInfo (cuda_access_info_t *access_info)
{
    int i;

    DBUG_ENTER ();

    for (i = 0; i < MAX_REUSE_DIM; i++) {
        if (CUAI_INDICES (access_info, i) != NULL) {
            CUAI_INDICES (access_info, i)
              = TBfreeCudaIndex (CUAI_INDICES (access_info, i));
        }
    }

    access_info = MEMfree (access_info);

    DBUG_RETURN ((cuda_access_info_t *)NULL);
}

/*--------------------------------------------------------------------------*/

argtab_t *
TBmakeArgtab (size_t size)
{
    argtab_t *argtab;
    size_t i;

    DBUG_ENTER ();

    argtab = (argtab_t *)MEMmalloc (sizeof (argtab_t));

    argtab->size = size;
    argtab->ptr_in = (node **)MEMmalloc (argtab->size * sizeof (node *));
    argtab->ptr_out = (node **)MEMmalloc (argtab->size * sizeof (types *));
    argtab->tag = (argtag_t *)MEMmalloc (argtab->size * sizeof (argtag_t));

    for (i = 0; i < argtab->size; i++) {
        argtab->ptr_in[i] = NULL;
        argtab->ptr_out[i] = NULL;
        argtab->tag[i] = ATG_notag;
    }

    DBUG_RETURN (argtab);
}

#undef DBUG_PREFIX
