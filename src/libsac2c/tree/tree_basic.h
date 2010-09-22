/*
 * $Id$
 */

#ifndef _SAC_TREE_BASIC_H_
#define _SAC_TREE_BASIC_H_

#include "types.h"

/*
 *   Global Access-Macros
 *   --------------------
 */

#define NODE_TYPE(n) ((n)->nodetype)
#define NODE_LINE(n) ((n)->lineno)
#define NODE_FILE(n) ((n)->src_file)
#define NODE_ERROR(n) ((n)->error)
#define NODE_CHECKVISITED(n) ((n)->checkvisited)
#define NODE_TEXT(n) (global.mdb_nodetype[NODE_TYPE (n)])

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

extern shpseg *TBmakeShpseg (node *num);

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
#define TYPES_TDEF(t) (t->tdef)
#define TYPES_NEXT(t) (t->next)
/* mutc old type accessors */
#define TYPES_MUTC_SCOPE(t) (t->scope)
#define TYPES_MUTC_USAGE(t) (t->usage)
#define TYPES_UNIQUE(t) (t->unique)
/*--------------------------------------------------------------------------*/

/***
 ***  NODELIST :
 ***
 ***  !!! DEPRECATED !!!
 ***
 ***  permanent attributes:
 ***
 ***    node*       NODE
 ***    void*       ATTRIB2
 ***    int         STATUS
 ***    nodelist*   NEXT    (O)
 ***/

/*
 * srs:
 * to use a nodelist in more general situations I have inserted a
 * new attribut ATTRIB2 which can store any suitable information.
 * Functions to handle a general node list can be found in tree_compound.h,
 * starting with NodeList... .
 * TBmakeNodelist(), TBmakeNodelistNode() are not needed to create the general
 * node list.
 */

extern nodelist *TBmakeNodelistNode (node *node, nodelist *next);

#define NODELIST_NODE(n) ((n)->node)
#define NODELIST_ATTRIB2(n) ((n)->attrib2)
#define NODELIST_STATUS(n) ((n)->status)
#define NODELIST_NEXT(n) ((n)->next)
#define NODELIST_INT(n) ((n)->num)

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
 ***  REUSE_CANDIDATE_T :
 ***
 ***  permanent attributes:
 ***
 ***/

extern rc_t *TBmakeReuseCandidate (node *array, int dim, rc_t *next);

#define RC_ARRAY(a) (a->array)
#define RC_ARRAYSHP(a) (a->arrayshp)
#define RC_SHARRAY(a) (a->sharray)
#define RC_SHARRAYSHP(a) (a->sharrayshp)
#define RC_DIM(a) (a->dim)
#define RC_SELFREF(a) (a->selfref)
#define RC_POSOFFSET(a, i) (a->posoffset[i])
#define RC_NEGOFFSET(a, i) (a->negoffset[i])
#define RC_REUSABLE(a) (a->reusable)
#define RC_NEXT(a) (a->next)

/*--------------------------------------------------------------------------*/

/***
 ***  INDEX_T :
 ***
 ***  permanent attributes:
 ***
 ***/

extern index_t *TBmakeIndex (unsigned int type, int coefficient, node *id, int looplevel,
                             index_t *next);

#define INDEX_TYPE(a) (a->type)
#define INDEX_COEFFICIENT(a) (a->coefficient)
#define INDEX_ID(a) (a->id)
#define INDEX_LOOPLEVEL(a) (a->looplevel)
#define INDEX_NEXT(a) (a->next)

extern index_t *TBfreeIndex (index_t *index);

/*--------------------------------------------------------------------------*/

/***
 ***  CUDA_ACCESS_INFO_T :
 ***
 ***  permanent attributes:
 ***
 ***/

extern cuda_access_info_t *TBmakeCudaAccessInfo (node *array, node *arrayshp, int dim,
                                                 int nestlevel);

#define CUAI_MATRIX(a) (a->coe_mtx)
#define CUAI_ARRAY(a) (a->array)
#define CUAI_ARRAYSHP(a) (a->arrayshp)
#define CUAI_SHARRAY(a) (a->sharray)
#define CUAI_SHARRAYSHP(a) (a->sharrayshp)
#define CUAI_DIM(a) (a->dim)
#define CUAI_NESTLEVEL(a) (a->nestlevel)
#define CUAI_INDICES(a, i) (a->indices[i])

extern cuda_access_info_t *TBfreeCudaAccessInfo (cuda_access_info_t *access_info);

/*
 * this defines the new node type
 */

#include "sons.h"
#include "attribs.h"

struct NODE {
    nodetype nodetype;         /* type of node */
    int lineno;                /* line number in source code */
    char *src_file;            /* pointer to filename or source code */
    node *error;               /* error node */
    bool checkvisited;         /* visited flag to detect illegal node sharing */
    union SONUNION sons;       /* the sons */
    union ATTRIBUNION attribs; /* the nodes attributes */
};

#include "node_basic.h"

#endif /* _SAC_TREE_BASIC_H_ */
