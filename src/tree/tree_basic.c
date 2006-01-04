/*
 *
 * $Log$
 * Revision 3.113  2005/01/11 14:06:14  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 3.112  2004/11/24 19:41:24  sah
 * COMPILES!
 *
 * Revision 3.111  2004/11/23 10:30:03  sah
 * SaC DevCamp DK
 *
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
#include "ctinfo.h"
#include "free.h"

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

    tmp = (shpseg *)ILIBmalloc (sizeof (shpseg));

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

    tmp = (types *)ILIBmalloc (sizeof (types));

    TYPES_BASETYPE (tmp) = btype;
    TYPES_NAME (tmp) = name;
    TYPES_MOD (tmp) = mod;
    TYPES_SHPSEG (tmp) = shpseg;
    TYPES_DIM (tmp) = dim;
    TYPES_POLY (tmp) = FALSE;

    TYPES_TDEF (tmp) = NULL;
    TYPES_NEXT (tmp) = NULL;

    DBUG_RETURN (tmp);
}

/*--------------------------------------------------------------------------*/

nodelist *
TBmakeNodelistNode (node *node, nodelist *next)
{
    nodelist *tmp;

    DBUG_ENTER ("TBmakeNodelistNode");

    tmp = (nodelist *)ILIBmalloc (sizeof (nodelist));
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

    tmp = (access_t *)ILIBmalloc (sizeof (access_t));

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

    argtab = (argtab_t *)ILIBmalloc (sizeof (argtab_t));

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
