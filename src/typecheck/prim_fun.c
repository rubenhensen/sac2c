/*
 * $Log$
 * Revision 1.2  1995/02/03 16:03:33  hw
 * added new functions AxA & Ixf_F
 * changed enum type_class
 *
 * Revision 1.1  1995/02/03  07:45:32  hw
 * Initial revision
 *, $
 *
 */

#include <stdlib.h>
#include "tree.h"
#include "prim_fun.h"
#include "dbug.h"
#include "Error.h"
#include "my_debug.h"

extern void *Malloc (int);                    /*imported from typecheck.c */
extern types *DuplicateTypes (types *source); /* imported form typecheck.c */

enum type_class { SxS_S, AxA_A, BxB_B, SxS_B, AxS_A };

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
 *  arguments     :
 *  description   :
 *  global vars   : ---
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
 *  description   :
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Malloc, free
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
void
InitPrimFunDeclarations ()
{
    node *arg1, *arg2, *tmp_node;
    types *type;

    DBUG_ENTER ("InitPrimFunDeclarations");
    prim_fun_dec = (node *)Malloc (sizeof (node));
    tmp_node = prim_fun_dec;

#include "prim_fun_tt.mac"
#undef TT2

    tmp_node = prim_fun_dec;
    prim_fun_dec = prim_fun_dec->node[1];
    free (tmp_node);

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InitPrimFunTab
 *  arguments     :
 *  description   :
 *  global vars   : prim_fun_tab, prim_fun_p
 *  internal funs : InitPrimFunDeclaration
 *  external funs : Malloc, free
 *  macros        : DBUG...
 *
 *  remarks       :
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
 *  description   : compares the shape of both arrays and returns the type
 *                  of array one if shapes are equal
 *                  return type T_unknown if shapes are different
 *  global vars   :
 *  internal funs :
 *  external funs : DuplicateTypes
 *  macros        : DBUG...
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */
types *
AxA (types *array1, types *array2)
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

        if (NULL == ret_type)
            ret_type = DuplicateTypes (array1);
    } else {
        GEN_TYPE_NODE (ret_type, T_unknown);
    }

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Ixf_F
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
Ixf_F (types *array1)
{
    types *ret_type;

    DBUG_ENTER ("Ixf_F");

    ret_type = DuplicateTypes (array1);
    ret_type->simpletype = T_float;

    DBUG_RETURN (ret_type);
}
