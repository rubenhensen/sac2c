/*
 *
 * $Log$
 * Revision 3.236  2004/11/23 10:05:24  sah
 * SaC DevCamp 04
 *
 * Revision 3.235  2004/11/21 20:31:16  sah
 * SaCDevCamp 04 ISMOP II
 *
 * Revision 3.234  2004/11/21 20:28:52  sah
 * SaCDevCamp 04 ISMOP
 *
 * Revision 3.233  2004/11/18 14:34:31  mwe
 * changed CheckAvis and chkavis to ToNewTypes and to tonewtypes
 *
 * Revision 3.232  2004/11/07 15:50:02  sah
 * added CreateNums and CreateIntegerArray
 *
 * [...]
 *
 */

#ifndef _SAC_TREE_BASIC_H_
#define _SAC_TREE_BASIC_H_

#include "types.h"
#include "attribs.h"

/*
 *   Global Access-Macros
 *   --------------------
 */

#define NODE_TYPE(n) ((n)->nodetype)
#define NODE_LINE(n) ((n)->lineno)
#define NODE_FILE(n) ((n)->src_file)

#define NODE_TEXT(n) (mdb_nodetype[NODE_TYPE (n)])

/*
 *   Non-node-structures
 *   -------------------
 */

#if 0 /* TODO: remove if no more needed */

/***
 ***  SHAPES :
 ***
 ***  permanent attributes:
 ***
 ***    int                DIM
 ***    shpseg*            SHPSEG
 ***
 ***/

#define SHAPES_DIM(s) (s->dim)
#define SHAPES_SHPSEG(s) (s->shpseg)

#endif

/*--------------------------------------------------------------------------*/

/***
 ***  SHPSEG :
 ***
 ***  permanent attributes:
 ***
 ***    int[SHP_SEG_SIZE]  SHAPE
 ***    shpseg*            NEXT
 ***
 ***/

extern shpseg *TBmakeShpseg (static nums *num);

#define SHPSEG_ELEMS(s) (s->shp)
#define SHPSEG_SHAPE(s, x) (SHPSEG_ELEMS (s)[x])
#define SHPSEG_NEXT(s) (s->next)

/*--------------------------------------------------------------------------*/

/***
 ***  TYPES :
 ***
 ***  permanent attributes:
 ***
 ***    simpletype         BASETYPE
 ***    int                DIM
 ***    bool               POLY                new TC indicates type vars!
 ***    shpseg*            SHPSEG    (O)
 ***    char*              NAME      (O)
 ***    char*              MOD       (O)
 ***    statustype         STATUS
 ***    types*             NEXT      (O)
 ***
 ***  temporary attributes:
 ***
 ***    node*              TDEF      (O)  (typecheck -> )
 ***/

/*
 * STATUS:
 *   ST_artificial : artificial return type due to the resolution of reference
 *                   parameters and global objects.
 *   ST_crettype   : return type of a function that is compiled to the actual
 *                   return type of the resulting C function.
 *   ST_regular    : otherwise
 *
 * TDEF is a reference to the defining N_typedef node of a user-defined type.
 */

extern types *TBmakeTypes1 (simpletype btype);

extern types *TBmakeTypes (simpletype btype, int dim, shpseg *shpseg, char *name,
                           char *mod);

#define TYPES_BASETYPE(t) (t->simpletype)
#define TYPES_DIM(t) (t->dim)
#define TYPES_POLY(t) (t->poly)
#define TYPES_SHPSEG(t) (t->shpseg)
#define TYPES_NAME(t) (t->name)
#define TYPES_MOD(t) (t->name_mod)
#define TYPES_STATUS(t) (t->type_status)
#define TYPES_TDEF(t) (t->tdef)
#define TYPES_NEXT(t) (t->next)

/*--------------------------------------------------------------------------*/

#if 1 /* TODO: SBS remove this as soon as parser is fixed */

/***
 ***  NUMS :
 ***
 ***  permanent attributes:
 ***
 ***    int    NUM
 ***    nums*  NEXT  (O)
 ***/

extern nums *TBmakeNums (int num, nums *next);
extern nums *TBcreateNums (int size, ...);
extern int *TBcreateIntegerArray (int size, ...);

#define NUMS_NUM(n) (n->num)
#define NUMS_NEXT(n) (n->next)

#endif

/*--------------------------------------------------------------------------*/

/***
 ***  NODELIST :
 ***
 ***  !!! DEPRECATED !!!
 ***
 ***  permanent attributes:
 ***
 ***    node*       NODE
 ***    statustype  ATTRIB
 ***    void*       ATTRIB2
 ***    statustype  STATUS
 ***    nodelist*   NEXT    (O)
 ***/

/*
 * STATUS:
 *   ST_regular
 *   ST_artificial
 *
 * ATTRIB: (in function node lists)
 *   ST_resolved
 *   ST_unresolved
 *
 * ATTRIB: (in object node lists)
 *   ST_reference
 *   ST_readonly_reference
 *
 * ATTRIB: (in typedef node lists)
 *   ST_regular
 */

/*
 * srs:
 * to use a nodelist in more general situations I have inserted a
 * new attribut ATTRIB2 which can store any suitable information.
 * Functions to handle a general node list can be found in tree_compound.h,
 * starting with NodeList... .
 * TBmakeNodelist(), TBmakeNodelistNode() are not needed to create the general
 * node list.
 */

extern nodelist *TBmakeNodelist (node *node, statustype status, nodelist *next);
extern nodelist *TBmakeNodelistNode (node *node, nodelist *next);

#define NODELIST_NODE(n) (n->node)
#define NODELIST_ATTRIB(n) (n->attrib)
#define NODELIST_ATTRIB2(n) (n->attrib2)
#define NODELIST_STATUS(n) (n->status)
#define NODELIST_NEXT(n) (n->next)
#define NODELIST_INT(n) ((int)(n->attrib))

/*--------------------------------------------------------------------------*/

/***
 ***  ARGTAB :
 ***
 ***  permanent attributes:
 ***
 ***    int*       SIZE
 ***    node*      PTR_IN[]     (N_arg, N_exprs)
 ***    void*      PTR_OUT[]    (TYPES, IDS)
 ***    argtag_t   TAG[]
 ***/

extern argtab_t *TBmakeArgtab (int size);

/*--------------------------------------------------------------------------*/

/***
 ***  ACCESS_T :
 ***
 ***  permanent attributes:
 ***
 ***    node*         ARRAY         (N_vardec/N_arg)
 ***    node*         IV            (N_vardec/N_arg)
 ***    accessclass_t CLASS
 ***    shpseg*       OFFSET  (O)
 ***    access_t*     NEXT    (O)
 ***/

extern access_t *TBmakeAccess (node *array, node *iv, accessclass_t class, shpseg *offset,
                               accessdir_t direction, access_t *next);

#define ACCESS_ARRAY(a) (a->array_vardec)
#define ACCESS_IV(a) (a->iv_vardec)
#define ACCESS_CLASS(a) (a->accessclass)
#define ACCESS_OFFSET(a) (a->offset)
#define ACCESS_DIR(a) (a->direction)
#define ACCESS_NEXT(a) (a->next)

/*--------------------------------------------------------------------------*/

/***
 *** dffoldmask_t :
 ***
 ***/

extern dffoldmask_t *TBmakeDfFoldMask (node *vardec, node *foldop, dffoldmask_t *next);

/* TODO: move to DupTree.c */
extern dffoldmask_t *TBcopyDfFoldMask (dffoldmask_t *mask);

#define DFFM_VARDEC(n) (n->vardec)
#define DFFM_FOLDOP(n) (n->foldop)
#define DFFM_NEXT(n) (n->next)

/*
 * this defines the new node type
 */
struct NODE {
    nodetype nodetype;           /* type of node */
    int lineno;                  /* line number in source code */
    char *src_file;              /* pointer to filename or source code */
    struct NODE *node[MAX_SONS]; /* pointers to child nodes */
    union ATTRIBUNION attribs;   /* the nodes attributes */
};

#include "node_basic.h"

#endif /* _SAC_TREE_BASIC_H_ */
