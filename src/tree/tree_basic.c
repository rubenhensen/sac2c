/*
 *
 * $Log$
 * Revision 3.110  2004/11/23 10:05:24  sah
 * SaC DevCamp 04
 *
 * Revision 3.109  2004/11/07 15:50:02  sah
 * added CreateNums and CreateIntegerArray
 *
 * [...]
 *
 */

#include "tree_basic.h"
#include "internal_lib.h"

/*--------------------------------------------------------------------------*/
/* local macros for heap allocation                                         */
/*--------------------------------------------------------------------------*/

#define ALLOCATE(type) (type *)ILIBmalloc (sizeof (type))

/*--------------------------------------------------------------------------*/
/* local functions for node initialization                                  */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Make-functions for non-node structures                                  */
/*--------------------------------------------------------------------------*/

/*
 * attention: the given parameter chain of nums structs is set free here!!!
 */
static shpseg *
TBmakeShpseg (nums *numsp)
{
    shpseg *tmp;
    int i;
    nums *oldnumsp;

    DBUG_ENTER ("TBmakeShpseg");

    tmp = ALLOCATE (shpseg);

#ifndef DBUG_OFF
    /*
     * For debugging memory use with dbx, it is important
     * that all memory has been initialised before reading
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
            SYSABORT (("Maximum number of dimensions exceeded"));
        }

        SHPSEG_SHAPE (tmp, i) = NUMS_NUM (numsp);

        i++;
        oldnumsp = numsp;
        numsp = NUMS_NEXT (numsp);
        oldnumsp = ILIBfree (oldnumsp);
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

    tmp = ALLOCATE (types);

    TYPES_BASETYPE (tmp) = btype;
    TYPES_NAME (tmp) = name;
    TYPES_MOD (tmp) = mod;
    TYPES_SHPSEG (tmp) = shpseg;
    TYPES_DIM (tmp) = dim;
    TYPES_POLY (tmp) = FALSE;
    TYPES_STATUS (tmp) = ST_regular;

    TYPES_TDEF (tmp) = NULL;
    TYPES_NEXT (tmp) = NULL;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

nums *
TBmakeNums (int num, nums *next)
{
    nums *tmp;

    DBUG_ENTER ("TBmakeNums");

    tmp = ALLOCATE (nums);
    NUMS_NUM (tmp) = num;
    NUMS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

nums *
TBcreateNums (int size, ...)
{
    nums *result = NULL;
    int cnt;
    va_list args;

    DBUG_ENTER ("TBcreateNums");

    va_start (args, size);

    for (cnt = 0; cnt < size; cnt++) {
        result = TBmakeNums (va_arg (args, int), result);
    }

    va_end (args);

    DBUG_RETURN (result);
}

int *
TBcreateIntegerArray (int size, ...)
{
    int *result = NULL;
    int cnt;
    va_list args;

    DBUG_ENTER ("TBcreateIntegerArray");

    if (size > 0) {
        result = ILIBmalloc (sizeof (int) * size);

        va_start (args, size);
        for (cnt = 0; cnt < size; cnt++) {
            result[cnt] = va_arg (args, int);
        }
        va_end (args);
    }

    DBUG_RETURN (result);
}

/*--------------------------------------------------------------------------*/

nodelist *
TBmakeNodelist (node *node, statustype status, nodelist *next)
{
    nodelist *tmp;

    DBUG_ENTER ("TBmakeNodelist");

    tmp = ALLOCATE (nodelist);
    NODELIST_NODE (tmp) = node;
    NODELIST_STATUS (tmp) = status;
    NODELIST_NEXT (tmp) = next;

    switch (NODE_TYPE (node)) {
    case N_fundef:
        NODELIST_ATTRIB (tmp) = ST_unresolved;
        break;
    case N_objdef:
        NODELIST_ATTRIB (tmp) = ST_reference;
        break;
    case N_typedef:
        NODELIST_ATTRIB (tmp) = ST_regular;
        break;
    default:
        DBUG_ASSERT ((0), ("Wrong node type in TBmakeNodelist"));
    }

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

nodelist *
TBmakeNodelistNode (node *node, nodelist *next)
{
    nodelist *tmp;

    DBUG_ENTER ("TBmakeNodelistNode");

    tmp = ALLOCATE (nodelist);
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

    tmp = ALLOCATE (access_t);

    ACCESS_ARRAY (tmp) = array;
    ACCESS_IV (tmp) = iv;
    ACCESS_CLASS (tmp) = class;
    ACCESS_OFFSET (tmp) = offset;
    ACCESS_DIR (tmp) = direction;
    ACCESS_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

argtab_t *
TBmakeArgtab (int size)
{
    argtab_t *argtab;
    int i;

    DBUG_ENTER ("TBmakeArgtab");

    argtab = ALLOCATE (argtab_t);

    argtab->size = size;
    argtab->ptr_in = ILIBmalloc (argtab->size * sizeof (node *));
    argtab->ptr_out = ILIBmalloc (argtab->size * sizeof (types *));
    argtab->tag = ILIBmalloc (argtab->size * sizeof (argtag_t));

    for (i = 0; i < argtab->size; i++) {
        argtab->ptr_in[i] = NULL;
        argtab->ptr_out[i] = NULL;
        argtab->tag[i] = ATG_notag;
    }

    DBUG_RETURN (argtab);
}

/*--------------------------------------------------------------------------*/

dffoldmask_t *
TBmakeDfFoldMask (node *vardec, node *foldop, dffoldmask_t *next)
{
    dffoldmask_t *tmp;

    DBUG_ENTER ("TBmakeDfFoldMask");

    tmp = ALLOCATE (dffoldmask_t);

    DFFM_VARDEC (tmp) = vardec;
    DFFM_FOLDOP (tmp) = foldop;
    DFFM_NEXT (tmp) = next;

    DBUG_RETURN (tmp);
}

/* TODO: move to DupTree.c */
dffoldmask_t *
TBcopyDfFoldMask (dffoldmask_t *mask)
{
    dffoldmask_t *tmp;

    DBUG_ENTER ("TBcopyDfFoldMask");

    if (mask != NULL) {
        tmp = ALLOCATE (dffoldmask_t);

        DFFM_VARDEC (tmp) = DFFM_VARDEC (mask);
        DFFM_FOLDOP (tmp) = DFFM_FOLDOP (mask);
        DFFM_NEXT (tmp) = TBcopyDfFoldMask (DFFM_NEXT (mask));
    } else {
        tmp = NULL;
    }

    DBUG_RETURN (tmp);
}
