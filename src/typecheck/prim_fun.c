/*
 * $Log$
 * Revision 1.10  1995/05/03 12:26:06  hw
 * - changes range of first argument of primitive function rotate
 *   (rotate(i,n,a) : 0 <= i <= dim(a) -1 )
 * - first argument of of primitive function rotate has to be constant
 * - new error-messages if 1. argumnet of primitive function 'cat' or
 *   'rotate' isn't constant
 *
 * Revision 1.9  1995/04/13  14:38:53  hw
 * bug fixed in 'Cat' (type of concatenated array will be computed
 *  correct now)
 *
 * Revision 1.8  1995/04/13  10:00:22  hw
 * changed typecheck of primitive function cat ( types *Cat(..) )
 *
 * Revision 1.7  1995/03/28  12:17:27  hw
 * internal_lib.h included
 *
 * Revision 1.6  1995/03/08  14:03:54  hw
 * added enumerator  SxA_A, SxA_F to enum type_class
 *
 * Revision 1.5  1995/02/27  11:09:35  hw
 * bug fixed in function `Psi`
 *
 * Revision 1.4  1995/02/13  17:51:38  hw
 * added B_B to enum type_class
 *
 * Revision 1.3  1995/02/09  11:07:13  hw
 *  - added macros TT1, TT3, BOOL_A
 * - enlarged enum type_class
 *  - changed function AxA
 * - renamed function Ixf_F to Axs_f
 * - added new functions: Shp, Reshp, TakeV, DropV, Psi, TakeDropS, Rot, Cat
 *
 * Revision 1.2  1995/02/03  16:03:33  hw
 * added new functions AxA & Ixf_F
 * changed enum type_class
 *
 * Revision 1.1  1995/02/03  07:45:32  hw
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include "tree.h"
#include "prim_fun.h"
#include "dbug.h"
#include "Error.h"
#include "my_debug.h"
#include "internal_lib.h"

extern types *DuplicateTypes (types *source); /* imported form typecheck.c */

enum type_class {
    SxS_S,
    AxA_A,
    BxB_B,
    SxS_B,
    AxS_A,
    SxS_F,
    AxA_F,
    AxS_F,
    VxA_Ar,
    VxA_As,
    VxA_At,
    VxA_Ad,
    VxA_Ap,
    SxA_At,
    SxA_Ad,
    V_As,
    A_S,
    SxSxA_A,
    SxAxA_A,
    B_B,
    SxA_A,
    SxA_F
};

#define GEN_TYPE_NODE(node, type)                                                        \
    if (NULL != (node = GEN_NODE (types))) {                                             \
        node->dim = 0;                                                                   \
        node->simpletype = type;                                                         \
        node->shpseg = NULL;                                                             \
        node->next = NULL;                                                               \
        node->id = NULL;                                                                 \
        node->id_mod = NULL;                                                             \
        node->name = NULL;                                                               \
        node->name_mod = NULL;                                                           \
        DBUG_PRINT ("TYPE", ("new type" P_FORMAT, node));                                \
    } else                                                                               \
        Error ("out of memory", 1)

#define GEN_PRIM_FUN_TAB_ELEM(p_old, mod, node_p, t_tag, u_tag, p_new)                   \
    tmp = (prim_fun_tab_elem *)Malloc (sizeof (prim_fun_tab_elem));                      \
    tmp->prf = p_old;                                                                    \
    tmp->id_mod = mod;                                                                   \
    tmp->node = node_p;                                                                  \
    tmp->typed_tag = t_tag;                                                              \
    tmp->user_tag = u_tag;                                                               \
    tmp->new_prf = p_new;                                                                \
    tmp->next = NULL;

#define INT GEN_TYPE_NODE (type, T_int)
#define FLOAT GEN_TYPE_NODE (type, T_float)
#define BOOL GEN_TYPE_NODE (type, T_bool)
#define INT_A                                                                            \
    type = MakeTypes (T_int);                                                            \
    type->dim = -1;
#define FLOAT_A                                                                          \
    type = MakeTypes (T_float);                                                          \
    type->dim = -1;
#define BOOL_A                                                                           \
    type = MakeTypes (T_bool);                                                           \
    type->dim = -1;

#define TT2(n, a, t1, t2, res)                                                           \
    tmp_node->node[1] = MakeNode (N_fundef);                                             \
    arg1 = MakeNode (N_arg);                                                             \
    t1;                                                                                  \
    arg1->info.types = type;                                                             \
    arg2 = MakeNode (N_arg);                                                             \
    t2;                                                                                  \
    arg2->info.types = type;                                                             \
    arg1->node[0] = arg2;                                                                \
    arg1->nnode = 1;                                                                     \
    tmp_node->node[1]->node[2] = arg1;                                                   \
    tmp_node->node[1]->info.prf_dec.tc = a;                                              \
    tmp_node->node[1]->info.prf_dec.tag = n;                                             \
    tmp_node = tmp_node->node[1];

#define TT3(n, a, t1, t2, t3, res)                                                       \
    tmp_node->node[1] = MakeNode (N_fundef);                                             \
    arg1 = MakeNode (N_arg);                                                             \
    t1;                                                                                  \
    arg1->info.types = type;                                                             \
    arg2 = MakeNode (N_arg);                                                             \
    t2;                                                                                  \
    arg2->info.types = type;                                                             \
    arg3 = MakeNode (N_arg);                                                             \
    t3;                                                                                  \
    arg3->info.types = type;                                                             \
    arg2->node[0] = arg3;                                                                \
    arg2->nnode = 1;                                                                     \
    arg1->node[0] = arg2;                                                                \
    arg1->nnode = 1;                                                                     \
    tmp_node->node[1]->node[2] = arg1;                                                   \
    tmp_node->node[1]->info.prf_dec.tc = a;                                              \
    tmp_node->node[1]->info.prf_dec.tag = n;                                             \
    tmp_node = tmp_node->node[1];

#define TT1(n, a, t1, res)                                                               \
    tmp_node->node[1] = MakeNode (N_fundef);                                             \
    arg1 = MakeNode (N_arg);                                                             \
    t1;                                                                                  \
    arg1->info.types = type;                                                             \
    tmp_node->node[1]->node[2] = arg1;                                                   \
    tmp_node->node[1]->info.prf_dec.tc = a;                                              \
    DBUG_PRINT ("TYPE", ("tc: %d (%d)", tmp_node->node[1]->info.prf_dec.tc, a));         \
    tmp_node->node[1]->info.prf_dec.tag = n;                                             \
    DBUG_PRINT ("TYPE",                                                                  \
                ("tc: %d (%d), tag: %d (%d)", tmp_node->node[1]->info.prf_dec.tc, a,     \
                 tmp_node->node[1]->info.prf_dec.tag, n));                               \
    tmp_node = tmp_node->node[1];

#define FT(pf, tc, new_pf) GenPrimTabEntries (pf, tc, new_pf);

prim_fun_tab_elem *prim_fun_tab, *prim_fun_p;
node *prim_fun_dec;

/*
 *
 *  functionname  : GenPrimTabEntries
 *  arguments     : 1) old tag of primitive function
 *                  2) type class of primitive function
 *                  3) new tag of primitive function
 *  description   : generates an entry into the primitive function table
 *                  for each member of the type class (type_c)
 *  global vars   : prim_fun_p
 *  internal funs : ---
 *  external funs :
 *  macros        : DBUG...,GEN_PRIM_FUN_TAB_ELEM
 *
 *  remarks       :
 *
 */
void
GenPrimTabEntries (prf prf_old, int type_c, prf prf_new)
{
    node *fun_dec = prim_fun_dec;
    prim_fun_tab_elem *tmp;

    DBUG_ENTER ("GenPrimTabEntries");
    while (NULL != fun_dec) {
        if (type_c == fun_dec->info.prf_dec.tc) {
            GEN_PRIM_FUN_TAB_ELEM (prf_old, NULL, fun_dec, 1, 0, prf_new);
            prim_fun_p->next = tmp;
            prim_fun_p = prim_fun_p->next;
        }
        fun_dec = fun_dec->node[1];
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InitPrimFunDeclarations
 *  arguments     :
 *  description   : generates for each entry (macro) of file "prim_fun_tt.mac"
 *                  a function declaration (N_fundef node)
 *  global vars   : prim_fun_dec
 *  internal funs : ---
 *  external funs : Malloc, free
 *  macros        : DBUG..., TT1, TT2, TT3
 *
 *  remarks       :
 *
 */
void
InitPrimFunDeclarations ()
{
    node *arg1, *arg2, *arg3, *tmp_node;
    types *type;

    DBUG_ENTER ("InitPrimFunDeclarations");
    prim_fun_dec = (node *)Malloc (sizeof (node));
    tmp_node = prim_fun_dec;

#include "prim_fun_tt.mac"
#undef TT1
#undef TT2
#undef TT3
    tmp_node = prim_fun_dec;
    prim_fun_dec = prim_fun_dec->node[1];
    free (tmp_node);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InitPrimFunTab
 *  arguments     :
 *  description   : manage the creation of a primitive function table to
 *                  lookup the argumenttype and resulttype
 *
 *  global vars   : prim_fun_tab, prim_fun_p
 *  internal funs : InitPrimFunDeclaration
 *  external funs : Malloc, free, FT
 *  macros        : DBUG...
 *
 *  remarks       : the macro FT is used to generate the entries in this
 *                  table by calling function  GenPrimTabEntries.
 *                  the information for creation is stored in the file
 *                  "prim_fun_ft.mac"
 *
 */
void
InitPrimFunTab ()
{

    DBUG_ENTER ("InitPrimFunTab");

    InitPrimFunDeclarations ();

    prim_fun_tab = (prim_fun_tab_elem *)Malloc (sizeof (prim_fun_tab_elem));
    prim_fun_p = prim_fun_tab;

#include "prim_fun_ft.mac"
#undef FT

    prim_fun_p = prim_fun_tab;
    prim_fun_tab = prim_fun_tab->next;
    free (prim_fun_p);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AxA
 *  arguments     : 1) type of array one
 *                  2) type of array two
 *                  3) simpletype of resulting type
 *  description   : compares the shape of both arrays and returns the type
 *                  which has the shape of array one and simpletype 3)
 *                  if shapes are equal
 *                  return type T_unknown if shapes are different
 *  global vars   :
 *  internal funs :
 *  external funs : DuplicateTypes
 *  macros        : DBUG..., GEN_TYPE_NODE
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */
types *
AxA (types *array1, types *array2, simpletype s_type)
{
    types *ret_type = NULL;

    DBUG_ENTER ("AxA");

    if (array1->dim == array2->dim) {
        int *shp1, *shp2, i;

        shp1 = array1->shpseg->shp;
        shp2 = array2->shpseg->shp;

        for (i = 0; i < array1->dim; i++)
            if (shp1[i] != shp2[i]) {
                GEN_TYPE_NODE (ret_type, T_unknown);
                break;
            }

        if (NULL == ret_type) {
            ret_type = DuplicateTypes (array1);
            ret_type->simpletype = s_type;
        }
    } else {
        GEN_TYPE_NODE (ret_type, T_unknown);
    }

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Axs_F
 *  arguments     : 1) type of an array
 *  description   : returns the type of an array with shape of 1) and
 *                  simpletype T_float
 *  global vars   :
 *  internal funs :
 *  external funs : DuplicateTypes
 *  macros        : DBUG...
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */
types *
Axs_F (types *array1)
{
    types *ret_type;

    DBUG_ENTER ("Axs_F");

    ret_type = DuplicateTypes (array1);
    ret_type->simpletype = T_float;

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Shp
 *  arguments     : 1) type of an array
 *                  2) N_prf node
 *  description   : returns the type of an array with shape of 1) and
 *                  simpletype T_float
 *  global vars   :
 *  internal funs :
 *  external funs : Malloc,
 *  macros        : DBUG..., GEN_TYPE_NODE
 *
 *  remarks       : is part of macro TT1 and is used in typecheck.c
 *
 */
types *
Shp (types *array)
{
    types *ret_type;

    DBUG_ENTER ("Shp");

    GEN_TYPE_NODE (ret_type, T_int);
    ret_type->dim = 1;
    ret_type->shpseg = (shpseg *)Malloc (sizeof (shpseg));
    ret_type->shpseg->shp[0] = array->dim;

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Reshp
 *  arguments     : 1) node of shape-vector
 *                  2) type of an array
 *  description   : returns the type of an array with shape 2) if number of
 *                  elements of old and new array are equal
 *                  or returns T_unkown if not
 *  global vars   :
 *  internal funs :
 *  external funs : Malloc
 *  macros        : DBUG...,  GEN_TYPE_NODE
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */
types *
Reshp (node *vec, types *array)
{
    int count1, count2, dim2 = 0, i;
    node *tmp;
    types *ret_type;

    DBUG_ENTER ("Reshp");

    /* count number of elements of the new array  and store it in count1 */
    tmp = vec->node[0];
    count1 = 1;
    while (NULL != tmp) {
        dim2++;
        count1 *= tmp->node[0]->info.cint;
        tmp = tmp->node[1];
    }

    /* count number of elements of old array 1) */
    count2 = 1;
    for (i = 0; i < array->dim; i++)
        count2 *= array->shpseg->shp[i];

    if ((count1 != count2) || (dim2 > SHP_SEG_SIZE))
        GEN_TYPE_NODE (ret_type, T_unknown);
    else {
        GEN_TYPE_NODE (ret_type, array->simpletype);
        ret_type->dim = dim2;
        ret_type->shpseg = (shpseg *)Malloc (sizeof (shpseg));
        tmp = vec->node[0];
        for (i = 0; i < dim2; i++) {
            ret_type->shpseg->shp[i] = tmp->node[0]->info.cint;
            tmp = tmp->node[1];
        }
    }

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : TakeV
 *  arguments     : 1) node of shape-vector
 *                  2) type of an array
 *
 *  description   : computes the resulttype of a 'take' operation, whoes
 *                  first argument was a constant vector
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., ERROR2, GEN_TYPE_NODE
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */
types *
TakeV (node *vec, types *array)
{
    types *ret_type;
    int ok = 1, i, dim2 = 0;
    node *tmp;

    DBUG_ENTER ("TakeV");

    if (N_array != vec->nodetype)
        ERROR2 (3, ("type error in line %d: 1.argument of function `take` "
                    " should be a constant vector",
                    vec->lineno));
    DBUG_ASSERT ((N_array == vec->nodetype), "not a N_array node");

    /* check weather entries in 1) are ok */
    tmp = vec->node[0];
    while ((NULL != tmp) && (1 == ok) && (dim2 < array->dim))
        if ((tmp->node[0]->info.cint > array->shpseg->shp[dim2])
            || (0 > tmp->node[0]->info.cint))
            ok = 0;
        else {
            tmp = tmp->node[1];
            dim2++;
        }

    if ((0 == ok) || ((NULL != tmp) && (dim2 > array->dim)))
        GEN_TYPE_NODE (ret_type, T_unknown);
    else {
        GEN_TYPE_NODE (ret_type, array->simpletype);
        ret_type->dim = array->dim;
        ret_type->shpseg = (shpseg *)Malloc (sizeof (shpseg));
        tmp = vec->node[0];
        /* compute resulting shape */
        for (i = 0; i < array->dim; i++) {
            if (i < dim2) {
                ret_type->shpseg->shp[i] = tmp->node[0]->info.cint;
                tmp = tmp->node[1];
            } else
                ret_type->shpseg->shp[i] = array->shpseg->shp[i];
        }
    }

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : DropV
 *  arguments     : 1) node of shape-vector
 *                  2) type of an array
 *
 *  description   : computes the resulttype of a 'drop' operation, whoes
 *                  first argument was a constant vector
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,  ERROR2, GEN_TYPE_NODE
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */
types *
DropV (node *vec, types *array)
{
    types *ret_type;
    node *tmp;
    int i, dim2 = 0, ok = 1;

    DBUG_ENTER ("DropV");

    if (N_array != vec->nodetype)
        ERROR2 (3, ("type error in line %d: 1.argument of function `take` "
                    " should be a constant vector",
                    vec->lineno));
    DBUG_ASSERT ((N_array == vec->nodetype), "not a N_array node");

    /* check weather the entries in 1) are ok */
    tmp = vec->node[0];
    while ((NULL != tmp) && (1 == ok) && (dim2 < array->dim))
        if ((tmp->node[0]->info.cint < array->shpseg->shp[dim2])
            && (0 <= tmp->node[0]->info.cint)) {
            dim2++;
            tmp = tmp->node[1];
        } else
            ok = 0;

    if ((0 == ok) || ((NULL != tmp) && (dim2 == array->dim)))
        GEN_TYPE_NODE (ret_type, T_unknown);
    else {
        GEN_TYPE_NODE (ret_type, array->simpletype);
        ret_type->shpseg = (shpseg *)Malloc (sizeof (shpseg));
        ret_type->dim = array->dim;
        tmp = vec->node[0];
        for (i = 0; i < array->dim; i++)
            if (i < dim2) {
                ret_type->shpseg->shp[i]
                  = array->shpseg->shp[i] - tmp->node[0]->info.cint;
                tmp = tmp->node[1];
            } else
                ret_type->shpseg->shp[i] = array->shpseg->shp[i];
    }

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Psi
 *  arguments     : 1) type of index vector
 *                  2) type of an array
 *  description   : computes the resulttype of a 'psi' operation
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., GEN_TYPE_NOD
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */
types *
Psi (types *vec, types *array)
{
    int dim, i, to_drop;
    types *ret_type;

    DBUG_ENTER ("Psi");

    if (1 == vec->dim)
        if (vec->shpseg->shp[0] <= array->dim) {
            GEN_TYPE_NODE (ret_type, array->simpletype);
            to_drop = vec->shpseg->shp[0];
            dim = array->dim - to_drop;
            ret_type->dim = dim;
            if (dim > 0) {
                ret_type->shpseg = (shpseg *)Malloc (sizeof (shpseg));
                for (i = 0; i < dim; i++)
                    ret_type->shpseg->shp[i] = array->shpseg->shp[to_drop + i];
            }
        } else
            GEN_TYPE_NODE (ret_type, T_unknown);
    else
        GEN_TYPE_NODE (ret_type, T_unknown);
    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : TakeDropS
 *  arguments     : 1) node to scalar
 *                  2) type of an array
 *                  3) tag
 *  description   : computes the resulttype of a 'take'or 'drop' operation,
 *                  whoes first argument is a constant scalar.
 *                  distinction between 'take' and 'drop' is made by 3)
 *                  3)==0 => take
 *                  3)==1 => drop
 *  global vars   :
 *  internal funs :
 *  external funs : Malloc
 *  macros        : DBUG..., GEN_TYPE_NODE, ERROR2
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */
types *
TakeDropS (node *s_node, types *array, int tag)
{
    types *ret_type;
    int i;

    DBUG_ENTER ("TakeDropS");

    if (N_num != s_node->nodetype)
        ERROR2 (3, ("type error in line %d: 1.argument of function 'drop' "
                    "is not a constant",
                    s_node->lineno));

    DBUG_ASSERT ((N_num == s_node->nodetype), "not N_num");

    if (1 == array->dim) {
        if (((1 == tag) ? (s_node->info.cint < array->shpseg->shp[0])
                        : (s_node->info.cint <= array->shpseg->shp[0]))
            && (0 <= s_node->info.cint)) {
            GEN_TYPE_NODE (ret_type, array->simpletype);
            ret_type->dim = 1;
            ret_type->shpseg = (shpseg *)Malloc (sizeof (shpseg));
            if (1 == tag)
                /* drop */
                ret_type->shpseg->shp[0] = array->shpseg->shp[0] - s_node->info.cint;
            else
                /* take */
                ret_type->shpseg->shp[0] = s_node->info.cint;
        } else
            GEN_TYPE_NODE (ret_type, T_unknown);
    } else if (1 < array->dim) {
        if (((1 == tag) ? (s_node->info.cint < array->shpseg->shp[0])
                        : (s_node->info.cint <= array->shpseg->shp[0]))
            && (0 <= s_node->info.cint)) {
            GEN_TYPE_NODE (ret_type, array->simpletype);
            ret_type->dim = array->dim;
            ret_type->shpseg = (shpseg *)Malloc (sizeof (shpseg));
            if (1 == tag)
                /* drop */
                ret_type->shpseg->shp[0] = array->shpseg->shp[0] - s_node->info.cint;
            else
                /* take */
                ret_type->shpseg->shp[0] = s_node->info.cint;

            for (i = 1; i < array->dim; i++)
                ret_type->shpseg->shp[i] = array->shpseg->shp[i];
        } else
            GEN_TYPE_NODE (ret_type, T_unknown);
    } else
        GEN_TYPE_NODE (ret_type, T_unknown);

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Rot
 *  arguments     : 1) pointer to first argument of function rotate
 *                  2) array that should be rotated
 *  description   : computes the resulttype of a 'rotate' operation
 *                  if 0 <= 1) < dim( 2) ) the type given by 2) will be returned
 *                  otherwise type T_unknown
 *  global vars   :
 *  internal funs :DuplicateTypes
 *  external funs :
 *  macros        : DBUG..., GEN_TYPE_NODE
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *                  1) has to be a N_num node
 *                  range of 1): 0 <= 1) < dim( 2)
 */
types *
Rot (node *s_node, types *array)
{
    types *ret_type;

    DBUG_ENTER ("Rot");

    if (N_num == s_node->nodetype)
        if ((0 <= s_node->info.cint) && (s_node->info.cint < array->dim))
            ret_type = DuplicateTypes (array);
        else
            GEN_TYPE_NODE (ret_type, T_unknown);
    else
        ERROR2 (3, ("type error in line %d: 1.argument of function `rotate` "
                    " should be a constant",
                    s_node->lineno));

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Cat
 *  arguments     : 1) pointer to first argument of function cat
 *                  2) first array
 *                  3) second array
 *  description   : computes the resulttype of a 'cat' operation, whose
 *                  first argument is a constant scalar
 *                  if the 1) is not a scalar type T_unknown will be returned
 *
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., GEN_TYPE_NODE
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *                  the range of 1) is: 0 <= 1) < dim( 2) )
 *
 */
types *
Cat (node *s_node, types *array1, types *array2)
{
    types *ret_type;
    int dim1, dim2, axis, ok = 1, i;

    DBUG_ENTER ("Cat");

    if (N_num == s_node->nodetype) {
        dim1 = array1->dim;
        dim2 = array2->dim;
        axis = s_node->info.cint;

        if ((0 <= axis) && (axis < dim1) && (dim1 == dim2)) {
            for (i = 0; i < dim1; i++)
                if (i != axis)
                    if (array1->shpseg->shp[i] != array2->shpseg->shp[i]) {
                        ok = 0;
                        break;
                    }
            if (1 == ok) {
                GEN_TYPE_NODE (ret_type, array1->simpletype);
                ret_type->shpseg = (shpseg *)Malloc (sizeof (shpseg));
                ret_type->dim = dim1;
                for (i = 0; i < dim1; i++)
                    ret_type->shpseg->shp[i] = array1->shpseg->shp[i];
                ret_type->shpseg->shp[axis] += array2->shpseg->shp[axis];
            } else
                GEN_TYPE_NODE (ret_type, T_unknown);
        } else
            GEN_TYPE_NODE (ret_type, T_unknown);
    } else
        ERROR2 (3, ("type error in line %d: 1.argument of function `cat` "
                    " should be a constant",
                    s_node->lineno));

    DBUG_RETURN (ret_type);
}
