/*
 * $Log$
 * Revision 1.36  1999/01/07 17:53:53  sbs
 * some breaks were missing in GenPrimTabEntries!
 * => rotate and friends couldn't be used from the stdlib!
 *
 * Revision 1.35  1999/01/07 10:50:49  cg
 * The rotate function now accepts 1. arguments (rotation dimension)
 * which are not constants. However, if a constant is provided, it
 * is checked against the dimension of the array to be rotated.
 *
 * Revision 1.34  1998/12/07 10:27:02  sbs
 * +,-,*,/ on int[.] x int[.], int[.] x int , and int x int[.]
 * now are intrinsic iff intrinsic is turned off (default)!!!
 * This enables IVE in all cases ...
 *
 * Revision 1.33  1998/11/02 17:00:36  sbs
 * intrinsic-mechanism added, i.e., only those versions of
 * primitive functions will be used that are either marked
 * intrinsic (by calling sac2c with -intrinsic) or are inherently
 * intrinsic, e.g., + on scalars, toi, etc. pp.
 *
 * Revision 1.32  1998/06/05 15:30:54  cg
 * *** empty log message ***
 *
 * Revision 1.31  1998/03/10 17:11:28  srs
 * improved orthography
 *
 * Revision 1.30  1998/03/03 14:02:09  cg
 * Last occurrence of function Error() replaced by macro SYSABORT
 *
 * Revision 1.29  1997/11/23 14:23:48  dkr
 * 2nd argument of MakeType(): NULL -> 0
 *
 * Revision 1.28  1997/11/04 11:37:59  srs
 * NEWTREE: nnode is ignored
 *
 * Revision 1.27  1997/10/29 14:34:11  srs
 * free -> FREE
 *
 * Revision 1.26  1997/10/28 18:26:25  srs
 * dead code removed
 *
 * Revision 1.25  1997/10/03 18:00:54  dkr
 * added type-classes ixi_i, i_i
 *
 * Revision 1.24  1996/02/20 17:53:43  hw
 * now primitive funktions infere types of known dimension, but unknown
 * shape (in modules only)
 *
 * Revision 1.23  1996/02/06  16:17:19  hw
 * added macros CHAR, CHAR
 * added type-class CxC_B to enum (for use with primitive functions
 *  eq, neq, ...)
 *
 * Revision 1.22  1996/02/06  13:14:14  hw
 * bug fixed in Genarray_S ( checking whether the elements of the
 *  shape-vector are konstant)
 *
 * Revision 1.21  1996/01/25  16:19:07  hw
 * added typechecking of primitive functions in modules
 * (in modules the resultung type can be also one with known dimension only
 *  (without known shape). This feature can be added by compiling this file
 *  with #define KNOWN_DIM.
 *  KNOWN_DIM isn't set in this version, but can be added later)
 *
 * Revision 1.20  1995/12/06  17:03:37  hw
 * added typecheck of primitive function 'genarray'
 *
 * Revision 1.19  1995/08/15  14:42:25  hw
 * changed typecheck of "modarray" (shapes will be compared)
 *
 * Revision 1.18  1995/08/11  17:27:32  hw
 * function Modarray inserted
 *
 * Revision 1.17  1995/07/13  15:29:48  hw
 * bug fixed in function Psi
 *
 * Revision 1.16  1995/07/07  16:22:48  hw
 * - changed functions AxA, TakeV, DropV, Psi( now they work with
 *   userdefined types, i hope ;-)
 *
 * Revision 1.15  1995/07/04  09:07:30  hw
 * - Axs_F, F2I & I2F removed
 * - ConvertType inserted
 * - enum type_class extended
 * - some DBUG_PRINTs inserted (tag: PRIM_FUN)
 *
 * Revision 1.14  1995/06/30  11:29:34  hw
 * - functions F2I & I2F inserted
 * - enlarged enum "type_class" with F_I, f_i, I_F, i_f
 *
 * Revision 1.13  1995/06/28  09:25:52  hw
 * bug fixed in Shp( userdefined types will be treated correctly)
 *
 * Revision 1.12  1995/06/23  12:36:57  hw
 * added argument to call of 'DuplicateTypes'
 *
 * Revision 1.11  1995/06/06  16:09:36  hw
 * changed errormessages
 *
 * Revision 1.10  1995/05/03  12:26:06  hw
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
#include "globals.h"
#include "tree.h"
#include "prim_fun.h"
#include "dbug.h"
#include "Error.h"
#include "my_debug.h"
#include "internal_lib.h"
#include "typecheck.h"
#include "access_macros.h"
#include "convert.h"
#include "free.h"

/* to infere types with known dimension and unknown_shape in typechecking of
 * modules KNOWN_DIM has to be set.
 * If KNOWN_DIM is not set only types with known and unknowns shapes will be
 * infered
 */
#define KNOWN_DIM

enum type_class {
    SxS_S,
    AxA_A,
    BxB_B,
    SxS_B,
    CxC_B,
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
    VxA_Ag,
    VxS_Ag,
    V_As,
    A_S,
    SxSxA_A,
    SxAxA_A,
    B_B,
    SxA_A,
    SxA_F,
    F_I,
    f_i,
    F_D,
    f_d,
    I_F,
    i_f,
    I_D,
    i_d,
    i_i,
    ixi_i,
    D_I,
    d_i,
    D_F,
    d_f,
    VxV_V,
    VxS_V,
    SxV_V,
    AxAxS_A,
    AxAxA_A
};

#ifdef OLD_GET_TYPE_NODE
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
        node->attrib = 0;                                                                \
        node->status = 0;                                                                \
        DBUG_PRINT ("PRIM_FUN", ("param: %s" P_FORMAT, type_string[type], node));        \
    } else                                                                               \
        SYSABORT (("Out of memory"))
#else
#define GEN_TYPE_NODE(types, simpletype)                                                 \
    types = MakeType (simpletype, SCALAR, NULL, NULL, NULL)
#endif /* OLD_GET_TYPE_NODE */

#define GEN_PRIM_FUN_TAB_ELEM(p_old, mod, node_p, t_tag, u_tag, p_new)                   \
    tmp = (prim_fun_tab_elem *)Malloc (sizeof (prim_fun_tab_elem));                      \
    tmp->prf = p_old;                                                                    \
    tmp->id_mod = mod;                                                                   \
    tmp->node = node_p;                                                                  \
    tmp->typed_tag = t_tag;                                                              \
    tmp->user_tag = u_tag;                                                               \
    tmp->new_prf = p_new;                                                                \
    tmp->next = NULL;                                                                    \
    DBUG_PRINT ("PRIM_FUN", ("prim_fun_elem:" P_FORMAT " %s  fun_dec:" P_FORMAT, tmp,    \
                             mdb_prf[p_new], node_p))

#define INT GEN_TYPE_NODE (type, T_int)
#define FLOAT GEN_TYPE_NODE (type, T_float)
#define BOOL GEN_TYPE_NODE (type, T_bool)
#define CHAR GEN_TYPE_NODE (type, T_char)
#define INT_A                                                                            \
    type = MakeTypes (T_int);                                                            \
    type->dim = -1;                                                                      \
    DBUG_PRINT ("PRIM_FUN", ("param: int[]" P_FORMAT, type))
#define FLOAT_A                                                                          \
    type = MakeTypes (T_float);                                                          \
    type->dim = -1;                                                                      \
    DBUG_PRINT ("PRIM_FUN", ("param: float[]" P_FORMAT, type))
#define BOOL_A                                                                           \
    type = MakeTypes (T_bool);                                                           \
    type->dim = -1;                                                                      \
    DBUG_PRINT ("PRIM_FUN", ("param: bool[]" P_FORMAT, type))
#define CHAR_A                                                                           \
    type = MakeTypes (T_char);                                                           \
    type->dim = -1;                                                                      \
    DBUG_PRINT ("PRIM_FUN", ("param: char[]" P_FORMAT, type))
#define DOUBLE GEN_TYPE_NODE (type, T_double)
#define DOUBLE_A                                                                         \
    type = MakeTypes (T_double);                                                         \
    type->dim = -1;                                                                      \
    DBUG_PRINT ("PRIM_FUN", ("param: double[]" P_FORMAT, type))

#define INT_V                                                                            \
    type = MakeTypes (T_int);                                                            \
    type->dim = KNOWN_DIM_OFFSET - 1;                                                    \
    DBUG_PRINT ("PRIM_FUN", ("param: int[.]" P_FORMAT, type))

#ifndef NEWTREE
#define TT2(n, a, t1, t2, res)                                                           \
    tmp_node->node[1] = MakeNode (N_fundef);                                             \
    DBUG_PRINT ("PRIM_FUN", ("prim_fun_dec: " P_FORMAT, tmp_node->node[1]));             \
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
#else /* NEWTREE */
#define TT2(n, a, t1, t2, res)                                                           \
    tmp_node->node[1] = MakeNode (N_fundef);                                             \
    DBUG_PRINT ("PRIM_FUN", ("prim_fun_dec: " P_FORMAT, tmp_node->node[1]));             \
    arg1 = MakeNode (N_arg);                                                             \
    t1;                                                                                  \
    arg1->info.types = type;                                                             \
    arg2 = MakeNode (N_arg);                                                             \
    t2;                                                                                  \
    arg2->info.types = type;                                                             \
    arg1->node[0] = arg2;                                                                \
    tmp_node->node[1]->node[2] = arg1;                                                   \
    tmp_node->node[1]->info.prf_dec.tc = a;                                              \
    tmp_node->node[1]->info.prf_dec.tag = n;                                             \
    tmp_node = tmp_node->node[1];
#endif

#ifndef NEWTREE
#define TT3(n, a, t1, t2, t3, res)                                                       \
    tmp_node->node[1] = MakeNode (N_fundef);                                             \
    DBUG_PRINT ("PRIM_FUN", ("prim_fun_dec: " P_FORMAT, tmp_node->node[1]));             \
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
#else /* NEWTREE */
#define TT3(n, a, t1, t2, t3, res)                                                       \
    tmp_node->node[1] = MakeNode (N_fundef);                                             \
    DBUG_PRINT ("PRIM_FUN", ("prim_fun_dec: " P_FORMAT, tmp_node->node[1]));             \
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
    arg1->node[0] = arg2;                                                                \
    tmp_node->node[1]->node[2] = arg1;                                                   \
    tmp_node->node[1]->info.prf_dec.tc = a;                                              \
    tmp_node->node[1]->info.prf_dec.tag = n;                                             \
    tmp_node = tmp_node->node[1];
#endif

#define TT1(n, a, t1, res)                                                               \
    tmp_node->node[1] = MakeNode (N_fundef);                                             \
    DBUG_PRINT ("PRIM_FUN", ("prim_fun_dec: " P_FORMAT, tmp_node->node[1]));             \
    arg1 = MakeNode (N_arg);                                                             \
    t1;                                                                                  \
    arg1->info.types = type;                                                             \
    tmp_node->node[1]->node[2] = arg1;                                                   \
    tmp_node->node[1]->info.prf_dec.tc = a;                                              \
    DBUG_PRINT ("PRIM_FUN", ("tc: %d (%d)", tmp_node->node[1]->info.prf_dec.tc, a));     \
    tmp_node->node[1]->info.prf_dec.tag = n;                                             \
    DBUG_PRINT ("PRIM_FUN",                                                              \
                ("tc: %d (%d), tag: %d (%d)", tmp_node->node[1]->info.prf_dec.tc, a,     \
                 tmp_node->node[1]->info.prf_dec.tag, n));                               \
    tmp_node = tmp_node->node[1];

/* follwing macro is used to compare the the dimension of two types
 * especially for comparison of types with known dimension and unknown shape
 * like t[.,.] and types with known shape (and dimension) as t[2,3]
 * d1 and d2 are dimensions of types
 *
 */
#define EQUAL_DIM(d1, d2)                                                                \
    (((d1 > 0) && (d2 > 0))                                                              \
       ? (d1 == d2)                                                                      \
       : (((d1 > 0) && (d2 < KNOWN_DIM_OFFSET))                                          \
            ? (d1 == (d2 - KNOWN_DIM_OFFSET))                                            \
            : (((d1 < KNOWN_DIM_OFFSET) && (d2 > 0))                                     \
                 ? (d2 == (d1 - KNOWN_DIM_OFFSET))                                       \
                 : (((d1 < KNOWN_DIM_OFFSET) && (d2 < KNOWN_DIM_OFFSET))                 \
                      ? (d1 == d2)                                                       \
                      : (d1 == d2)))))

/* Macro to convert a know_dim (dim < KNOWN_DIM_OFFSET) to a positive Value
 */
#define POS_DIM(dim) (((-1) * dim) + KNOWN_DIM_OFFSET)

/* Macro to compare value of N_num-node is between 0 and  given integer
 * if the node is a N_num node
 */
#define IS_IN_RANGE(n_node, val)                                                         \
    ((N_num == NODE_TYPE (n_node))                                                       \
       ? ((0 <= NUM_VAL (n_node)) && (NUM_VAL (n_node) < val))                           \
       : 1)

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
    int intrinsic = 0, wanted;

    DBUG_ENTER ("GenPrimTabEntries");

    /*
     * First, we check whether the particular entry is a "wanted"
     * intrinsic function!!
     */

    switch (prf_old) {
    case F_add_AxA:
    case F_add_AxS:
    case F_add_SxA:
    case F_add:
        wanted = (INTRINSIC_ADD & intrinsics);
        intrinsic
          = (type_c == SxS_S
             || (wanted && (type_c == AxA_A || type_c == SxA_A || type_c == AxS_A))
             || (!wanted && (type_c == VxV_V || type_c == VxS_V || type_c == SxV_V)));
        break;
    case F_sub_AxA:
    case F_sub_AxS:
    case F_sub_SxA:
    case F_sub:
        wanted = (INTRINSIC_SUB & intrinsics);
        intrinsic
          = (type_c == SxS_S
             || (wanted && (type_c == AxA_A || type_c == SxA_A || type_c == AxS_A))
             || (!wanted && (type_c == VxV_V || type_c == VxS_V || type_c == SxV_V)));
        break;
    case F_mul_AxA:
    case F_mul_AxS:
    case F_mul_SxA:
    case F_mul:
        wanted = (INTRINSIC_MUL & intrinsics);
        intrinsic
          = (type_c == SxS_S
             || (wanted && (type_c == AxA_A || type_c == SxA_A || type_c == AxS_A))
             || (!wanted && (type_c == VxV_V || type_c == VxS_V || type_c == SxV_V)));
        break;
    case F_div_AxA:
    case F_div_AxS:
    case F_div_SxA:
    case F_div:
        wanted = (INTRINSIC_DIV & intrinsics);
        intrinsic
          = (type_c == SxS_S
             || (wanted && (type_c == AxA_A || type_c == SxA_A || type_c == AxS_A))
             || (!wanted && (type_c == VxV_V || type_c == VxS_V || type_c == SxV_V)));
        break;
    case F_psi:
        intrinsic = (INTRINSIC_PSI & intrinsics);
        break;
    case F_take:
        intrinsic = (INTRINSIC_TAKE & intrinsics);
        break;
    case F_drop:
        intrinsic = (INTRINSIC_DROP & intrinsics);
        break;
    case F_cat:
        intrinsic = (INTRINSIC_CAT & intrinsics);
        break;
    case F_rotate:
        intrinsic = (INTRINSIC_ROT & intrinsics);
        break;
    default:
        intrinsic = 1;
    }

    if (intrinsic) {
        DBUG_PRINT ("PRIM_FUN", ("intrinsic %s added!", mdb_prf[prf_old]));
        while (NULL != fun_dec) {
            if (type_c == fun_dec->info.prf_dec.tc) {
                GEN_PRIM_FUN_TAB_ELEM (prf_old, NULL, fun_dec, 1, 0, prf_new);
                prim_fun_p->next = tmp;
                prim_fun_p = prim_fun_p->next;
            }
            fun_dec = fun_dec->node[1];
        }
    } else {
        DBUG_PRINT ("PRIM_FUN", ("intrinsic %s ommitted!", mdb_prf[prf_old]));
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
    FREE (tmp_node);

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

#define FT(pf, tc, new_pf) GenPrimTabEntries (pf, tc, new_pf);
#include "prim_fun_ft.mac"
#undef FT

    prim_fun_p = prim_fun_tab;
    prim_fun_tab = prim_fun_tab->next;
    FREE (prim_fun_p);

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
    types *ret_type = NULL, *tmp_array1 = NULL, *tmp_array2 = NULL;
    int new_type = 0;
    int ret = 1;

    DBUG_ENTER ("AxA");

    /* First compute basis_types (new types will be generated), if necessary.
     * new_type will say which types are new
     */
    if (T_user == array1->simpletype) {
        GET_BASIC_TYPE (tmp_array1, array1, -64);
        new_type = 1;
    } else
        tmp_array1 = array1;

    if (T_user == array2->simpletype) {
        GET_BASIC_TYPE (tmp_array2, array2, -64);
        new_type += 2;
    } else
        tmp_array2 = array2;

    if (EQUAL_DIM (TYPES_DIM (tmp_array1), TYPES_DIM (tmp_array2))) {
        /* set 'ret' to indicate which type is the result_type */
        if ((TYPES_DIM (tmp_array2) < KNOWN_DIM_OFFSET)
            && (TYPES_DIM (tmp_array1) > UNKNOWN_SHAPE))
            ret = 1;
        else if ((TYPES_DIM (tmp_array2) < KNOWN_DIM_OFFSET)
                 && (TYPES_DIM (tmp_array1) > UNKNOWN_SHAPE))
            ret = 2;
        else if ((TYPES_DIM (tmp_array2) < KNOWN_DIM_OFFSET)
                 && (TYPES_DIM (tmp_array1) < KNOWN_DIM_OFFSET))
            ret = 2;
        else if ((TYPES_DIM (tmp_array1) > UNKNOWN_SHAPE)
                 && (TYPES_DIM (tmp_array2) > UNKNOWN_SHAPE)) {
            int i;

            /* compare shapes */
            for (i = 0; i < TYPES_DIM (array1); i++)
                if (TYPES_SHAPE (tmp_array1, i) != TYPES_SHAPE (tmp_array2, i)) {
                    GEN_TYPE_NODE (ret_type, T_unknown);
                    break;
                }
        } else if ((TYPES_DIM (tmp_array1) == UNKNOWN_SHAPE)
                   && (TYPES_DIM (tmp_array2) == UNKNOWN_SHAPE))
            ret = 1;
        else
            DBUG_ASSERT (0, "wrong dimensions");
    } else {
        if (TYPES_DIM (tmp_array1) == UNKNOWN_SHAPE)
            ret = 2;
        else if (TYPES_DIM (tmp_array2) == UNKNOWN_SHAPE)
            ret = 1;
        else
            GEN_TYPE_NODE (ret_type, T_unknown);
    }

    /* now set return_type */
    if (NULL == ret_type) {
        switch (new_type) {
        case 0:
            if (1 == ret)
                ret_type = DuplicateTypes (array1, 0);
            else
                ret_type = DuplicateTypes (array2, 0);
            ret_type->simpletype = s_type;
            break;
        case 1:
            if (1 == ret)
                ret_type = tmp_array1;
            else {
                ret_type = DuplicateTypes (array2, 0);
                FreeOneTypes (tmp_array1);
            }
            ret_type->simpletype = s_type;
            break;
        case 2:
            if (2 == ret)
                ret_type = tmp_array2;
            else {
                ret_type = DuplicateTypes (array1, 0);
                FreeOneTypes (tmp_array2);
            }
            ret_type->simpletype = s_type;
            break;
        case 3:
            if (1 == ret) {
                ret_type = tmp_array1;
                FreeOneTypes (tmp_array2);
            } else {
                ret_type = tmp_array2;
                FreeOneTypes (tmp_array1);
            }
            ret_type->simpletype = s_type;
            break;
        default:
            DBUG_ASSERT (0, "wrong value of 'new_type'");
            break;
        }
    }

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Shp
 *  arguments     : 1) type of an array
 *                  2) N_prf node
 *  description   : returns the type of an array with shape of 1) and
 *                  simpletype T_int
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
    int dim;

    DBUG_ENTER ("Shp");

    if (UNKNOWN_SHAPE == TYPES_DIM (array))
    /* replace UNKNOWN_SHAPE with KNOWN_DIM_OFFSET-1 if infering of dimension
     * is integrated
     */
#ifndef KNOWN_DIM
        ret_type = MakeType (T_int, UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
        ret_type = MakeType (T_int, KNOWN_DIM_OFFSET - 1, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
    else if (KNOWN_DIM_OFFSET > TYPES_DIM (array))
    /* replace UNKNOWN_SHAPE with KNOWN_DIM_OFFSET-1 if infering of dimension
     * is integrated
     */
#ifndef KNOWN_DIM
        ret_type = MakeType (T_int, UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
        ret_type = MakeType (T_int, KNOWN_DIM_OFFSET - 1, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
    else if (SCALAR < TYPES_DIM (array)) {
        /* it is a array-type */
        ret_type = MakeType (T_int, 1, MakeShpseg (NULL), NULL, NULL);
        GET_DIM (dim, array);
        TYPES_SHAPE (ret_type, 0) = dim;
    } else
        DBUG_ASSERT (0, "unknown dimension of type");

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Reshp
 *  arguments     : 1) node of shape-vector
 *                  2) type of an array
 *                  3) type of shape vector
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
Reshp (node *vec, types *array, types *shp_vec)
{
    int count1, count2, i, dim2 = 0;
    node *tmp;
    types *ret_type;

    DBUG_ENTER ("Reshp");

    if (N_array == NODE_TYPE (vec)) {
        /* count number of elements of the new array  and store it in count1 */
        DBUG_ASSERT (N_array == NODE_TYPE (vec), " wrong node != N_array");
        tmp = ARRAY_AELEMS (vec);
        count1 = 1;
        while (NULL != tmp) {
            dim2++;
            count1 *= NUM_VAL (EXPRS_EXPR (tmp));
            tmp = EXPRS_NEXT (tmp);
        }

        /* count number of elements of old array 1) */
        count2 = 1;
        for (i = 0; i < TYPES_DIM (array); i++)
            count2 *= TYPES_SHAPE (array, i);

        if (UNKNOWN_SHAPE >= TYPES_DIM (array)) {
            /* TODO: replace UNKNOWN_SHAPE with KNOWN_DIM_OFFSET-dim2 later on */
#ifndef KNOWN_DIM
            ret_type = MakeType (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
            ret_type = MakeType (TYPES_BASETYPE (array), KNOWN_DIM_OFFSET - dim2, NULL,
                                 NULL, NULL);
#endif /* KNOWN_DIM */
        } else if ((count1 != count2) || (dim2 > SHP_SEG_SIZE))
            GEN_TYPE_NODE (ret_type, T_unknown);
        else {
            ret_type
              = MakeType (TYPES_BASETYPE (array), dim2, MakeShpseg (NULL), NULL, NULL);
            tmp = ARRAY_AELEMS (vec);
            for (i = 0; i < dim2; i++) {
                TYPES_SHAPE (ret_type, i) = NUM_VAL (EXPRS_EXPR (tmp));
                tmp = EXPRS_NEXT (tmp);
            }
        }
    } else if (SAC_MOD == kind_of_file) {
        /* checking a module, so the first argument of "reshape" has not to be
         * an constant array
         */
        if (1 == TYPES_DIM (shp_vec)) {
            /* we only know the dimension of the resulting type */
#ifndef KNOWN_DIM
            ret_type = MakeType (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
            ret_type
              = MakeType (TYPES_BASETYPE (array),
                          KNOWN_DIM_OFFSET - TYPES_SHAPE (shp_vec, 0), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        } else if ((UNKNOWN_SHAPE == TYPES_DIM (shp_vec))
                   || ((KNOWN_DIM_OFFSET - 1) == TYPES_DIM (shp_vec))) {
            /* we don't know the dimenion and the shape of the resulting type */
            ret_type = MakeType (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
        } else {
            ERROR2 (3, ("%s, %d: 1.argument of function `reshape` "
                        " has wrong type (%s)",
                        filename, NODE_LINE (vec), Type2String (shp_vec, 0)));
        }
    } else {
        /* checking a programm, so first argument of "reshape" has to be an
         * constant array, but it isn't.
         */
        ERROR2 (3, ("%s, %d: 1.argument of function `reshape` "
                    " should be a constant vector",
                    filename, NODE_LINE (vec)));
    }

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : TakeDropV
 *  arguments     : 1) type of shape-vector
 *                  2) type of an array
 *
 *  description   : computes the result-type of a 'take' or 'drop' operation,
 *                  whoes first argument is NOT a constant vector
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *
 *  remarks       :
 *
 */
types *
TakeDropV (types *vec_type, types *array_btype)
{
    types *ret_type;

    DBUG_ENTER ("TakeDropV");

    if (1 == TYPES_DIM (vec_type)) {
        /* access-vector is not a constant vector, but the shape is known
         */
        if (SCALAR < TYPES_DIM (array_btype))
            /* the shape of the arrray is known
             */
            if (TYPES_SHAPE (vec_type, 0) <= TYPES_DIM (array_btype))
            /* number of elements of acess-vector is equal or less than
             * dimension of array
             * (create a type with known dimension)
             */
#ifndef KNOWN_DIM
                ret_type = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL,
                                     NULL, NULL);
#else
                ret_type = MakeType (TYPES_BASETYPE (array_btype),
                                     KNOWN_DIM_OFFSET - TYPES_DIM (array_btype), NULL,
                                     NULL, NULL);
#endif /* KNOWN_DIM */
            else
                /* access-vector as to many elements */
                ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
        else if (UNKNOWN_SHAPE == TYPES_DIM (array_btype))
            /* the shape of the array is unknown */
            ret_type
              = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL, NULL, NULL);
        else if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype))
            /* only the dimension of the array is known */
            if (KNOWN_DIM_OFFSET - TYPES_SHAPE (vec_type, 0) >= TYPES_DIM (array_btype))
            /* number of elements of acess-vector is equal or less than
             * dimension of array
             * (create a type with known dimension)
             */
#ifndef KNOWN_DIM
                ret_type = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL,
                                     NULL, NULL);
#else
                ret_type = MakeType (TYPES_BASETYPE (array_btype),
                                     TYPES_DIM (array_btype), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            else
                /* access-vector as to many elements */
                ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
    } else if (UNKNOWN_SHAPE == TYPES_DIM (vec_type)) {
        /* access-vector is not an constant vector.
         * the shape of the vector is unknown
         */
        if (SCALAR < TYPES_DIM (array_btype))
        /* the shape of the array is known, so the dimension of the result-type
         * and the array are equal
         */
#ifndef KNOWN_DIM
            ret_type
              = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
            ret_type
              = MakeType (TYPES_BASETYPE (array_btype),
                          KNOWN_DIM_OFFSET - TYPES_DIM (array_btype), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        else if (UNKNOWN_SHAPE == TYPES_DIM (array_btype))
            /* the shape of the array is unknown.
             * The shape of the resul-type will be unknown too
             */
            ret_type
              = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL, NULL, NULL);

        else if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype))
        /* One only know the dimension of the array, so one can only infer
         * the dimsion of the reslut-type
         */
#ifndef KNOWN_DIM
            ret_type
              = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
            ret_type = MakeType (TYPES_BASETYPE (array_btype), TYPES_DIM (array_btype),
                                 NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        else
            DBUG_ASSERT (0, " wrong dimension of array_btype");
    } else if ((KNOWN_DIM_OFFSET - 1) == TYPES_DIM (vec_type)) {
        /* access-vector is not a constant vektor and only the dimension
         * is known (the dimension is 1)
         */
        if (SCALAR < TYPES_DIM (array_btype)) {
            /* the shape of the array is known, so the dimension of the result-type
             * and the array are equal
             */
#ifndef KNOWN_DIM
            ret_type
              = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
            ret_type
              = MakeType (TYPES_BASETYPE (array_btype),
                          KNOWN_DIM_OFFSET - TYPES_DIM (array_btype), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        } else if (UNKNOWN_SHAPE == TYPES_DIM (array_btype))
            /* the shape of the array is unknown.
             * The shape of the resul-type will be unknown too
             */
            ret_type
              = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL, NULL, NULL);
        else if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype)) {
            /* One only knows the dimension of the array and the vector
             */
#ifndef KNOWN_DIM
            ret_type
              = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
            ret_type = MakeType (TYPES_BASETYPE (array_btype), TYPES_DIM (array_btype),
                                 NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        } else
            DBUG_ASSERT (0, " wrong dimension of array_btype");
    } else
        /* the dimension of vec_type is greater than 1.
         * This is not correct, so create T_unknown as result-type
         */
        ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : TakeV
 *  arguments     : 1) node of shape-vector
 *                  2) type of shape-vector
 *                  3) type of an array
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
TakeV (node *vec, types *vec_type, types *array)
{
    types *ret_type, *array_btype;
    int ok = 1, i, dim2 = 0;
    node *tmp;

    DBUG_ENTER ("TakeV");

    if (SAC_PRG == kind_of_file) {
        if (N_array != NODE_TYPE (vec))
            ERROR2 (3, ("%s, %d: 1.argument of function `take` "
                        " should be a constant vector",
                        filename, NODE_LINE (vec)));
        DBUG_ASSERT ((N_array == NODE_TYPE (vec)), "not a N_array node");
    }

    GET_BASIC_TYPE (array_btype, array, NODE_LINE (vec));

    if (N_array == NODE_TYPE (vec)) {
        tmp = ARRAY_AELEMS (vec);

        if (SCALAR < TYPES_DIM (array_btype)) {
            /* array has known shape */

            /* check weather entries in 1) are ok */
            while ((NULL != tmp) && (1 == ok) && (dim2 < TYPES_DIM (array_btype)))
                if ((NUM_VAL (EXPRS_EXPR (tmp)) > TYPES_SHAPE (array_btype, dim2))
                    || (0 >= NUM_VAL (EXPRS_EXPR (tmp))))
                    ok = 0;
                else {
                    tmp = EXPRS_NEXT (tmp);
                    dim2++;
                }

            if ((0 == ok) || ((NULL != tmp) && (dim2 >= array_btype->dim))) {
                GEN_TYPE_NODE (ret_type, T_unknown);
                FreeOneTypes (array_btype);
            } else {
                ret_type
                  = MakeType (TYPES_BASETYPE (array_btype), TYPES_DIM (array_btype),
                              MakeShpseg (NULL), NULL, NULL);
                tmp = ARRAY_AELEMS (vec);
                /* compute resulting shape */
                for (i = 0; i < TYPES_DIM (array_btype); i++)
                    if (i < dim2) {
                        TYPES_SHAPE (ret_type, i) = NUM_VAL (EXPRS_EXPR (tmp));
                        tmp = EXPRS_NEXT (tmp);
                    } else
                        TYPES_SHAPE (ret_type, i) = TYPES_SHAPE (array_btype, i);
            }
        } else if (UNKNOWN_SHAPE == TYPES_DIM (array_btype))
            ret_type
              = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL, NULL, NULL);
        else if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype)) {
            /* one knows only the dimension of array_btype, but not the shape
             */
            while (NULL != tmp) {
                dim2 += 1;
                tmp = EXPRS_EXPR (tmp);
            }
            if (KNOWN_DIM_OFFSET - dim2 == TYPES_DIM (array_btype))
            /* the shape of 'array' is unknown, but the dimension is known and it
             * is equal the number of elements of the constant vector.
             * We create a dimension known result-type, because we do not know
             * if the components of the vector will match with the array's shape
             *  any time.
             * (of course one could take the components of the vector as new
             *  values of the result-type, but one does not know if the
             *  access will be ok)
             *
             *  replace UNKNOWN_SHAPE with TYPES_DIM(array_btype)
             */
#ifndef KNOWN_DIM
                ret_type = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL,
                                     NULL, NULL);
#else
                ret_type = MakeType (TYPES_BASETYPE (array_btype),
                                     TYPES_DIM (array_btype), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            else if (KNOWN_DIM_OFFSET - dim2 > TYPES_DIM (array_btype))
            /* one only can infere the dimension of the result-type
             *
             * replace UNKNOWN_SHAPE with TYPES_DIM(array_btype) when infering
             * dimesnion
             */
#ifndef KNOWN_DIM
                ret_type = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL,
                                     NULL, NULL);
#else
                ret_type = MakeType (TYPES_BASETYPE (array_btype),
                                     TYPES_DIM (array_btype), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        } else
            DBUG_ASSERT (0, " wrong dimension of array_btype");
    } else
        ret_type = TakeDropV (vec_type, array_btype);

    /* and now free some memory, because we don't need it anymore */
    FreeOneTypes (array_btype);

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
 *  macros        : DBUG...,  ERROR2, GEN_TYPE_NODE, FREE,
 *                   GET_BASIC_TYPE
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */
types *
DropV (node *vec, types *vec_type, types *array)
{
    types *ret_type, *array_btype;
    node *tmp;
    int i, dim2 = 0, ok = 1;

    DBUG_ENTER ("DropV");

    GET_BASIC_TYPE (array_btype, array, vec->lineno);

    if (SAC_PRG == kind_of_file) {
        if (N_array != vec->nodetype)
            ERROR2 (3, ("%s, %d: 1.argument of function `take` "
                        " should be a constant vector",
                        filename, vec->lineno));
        DBUG_ASSERT ((N_array == vec->nodetype), "not a N_array node");
    }

    if (N_array == NODE_TYPE (vec)) {
        tmp = ARRAY_AELEMS (vec);

        if (SCALAR < TYPES_DIM (array_btype)) {
            /* array has known shape */

            /* check weather the entries in 1) are ok */
            while ((NULL != tmp) && (1 == ok) && (dim2 < TYPES_DIM (array_btype)))
                if ((NUM_VAL (EXPRS_EXPR (tmp)) < TYPES_SHAPE (array_btype, dim2))
                    && (0 <= NUM_VAL (EXPRS_EXPR (tmp)))) {
                    dim2++;
                    tmp = EXPRS_NEXT (tmp);
                } else
                    ok = 0;

            if ((0 == ok) || ((NULL != tmp) && (dim2 == TYPES_DIM (array_btype)))) {
                GEN_TYPE_NODE (ret_type, T_unknown);
            } else {
                ret_type
                  = MakeType (TYPES_BASETYPE (array_btype), TYPES_DIM (array_btype),
                              MakeShpseg (NULL), NULL, NULL);
                tmp = ARRAY_AELEMS (vec);
                for (i = 0; i < TYPES_DIM (array_btype); i++)
                    if (i < dim2) {
                        ret_type->shpseg->shp[i]
                          = array_btype->shpseg->shp[i] - tmp->node[0]->info.cint;
                        tmp = tmp->node[1];
                    } else
                        ret_type->shpseg->shp[i] = array_btype->shpseg->shp[i];
            }
        } else if (UNKNOWN_SHAPE == TYPES_DIM (array_btype))
            ret_type
              = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL, NULL, NULL);
        else if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype)) {
            /* one knows only the dimension of array_btype, but not the shape
             */
            while (NULL != tmp) {
                dim2 += 1;
                tmp = EXPRS_EXPR (tmp);
            }
            if (KNOWN_DIM_OFFSET - dim2 == TYPES_DIM (array_btype))
            /* the shape of 'array' is unknown, but the dimension is known and it
             * is equal the number of elements of the constant vector.
             * We create a dimension known result-type, because we do not know
             * if the components of the vector will match with the array's shape
             *  any time.
             * (of course one could take the components of the vector as new
             *  values of the result-type, but one does not know if the
             *  access will be ok)
             *
             *  replace UNKNOWN_SHAPE with TYPES_DIM(array_btype)
             */
#ifndef KNOWN_DIM
                ret_type = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL,
                                     NULL, NULL);
#else
                ret_type = MakeType (TYPES_BASETYPE (array_btype),
                                     TYPES_DIM (array_btype), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            else if (KNOWN_DIM_OFFSET - dim2 > TYPES_DIM (array_btype))
            /* one only can infere the dimension of the result-type
             *
             * replace UNKNOWN_SHAPE with TYPES_DIM(array_btype) when infering
             * dimesnion
             */
#ifndef KNOWN_DIM
                ret_type = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL,
                                     NULL, NULL);
#else
                ret_type = MakeType (TYPES_BASETYPE (array_btype),
                                     TYPES_DIM (array_btype), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        } else
            DBUG_ASSERT (0, " wrong dimension of array_btype");
    } else
        ret_type = TakeDropV (vec_type, array_btype);

    FreeOneTypes (array_btype);

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
    types *ret_type, *array_btype;
    shpseg *new_shpseg;

    DBUG_ENTER ("Psi");

    GET_BASIC_TYPE (array_btype, array, -64); /* -64 is a dummy argument */

    if (1 == TYPES_DIM (vec)) {
        if (TYPES_SHAPE (vec, 0) <= TYPES_DIM (array_btype)) {
            ret_type = array_btype;
            to_drop = TYPES_SHAPE (vec, 0);
            dim = TYPES_DIM (array_btype) - to_drop;
            ret_type->dim = dim;
            if (dim > 0) {
                new_shpseg = MakeShpseg (NULL);
                for (i = 0; i < dim; i++)
                    SHPSEG_SHAPE (new_shpseg, i) = TYPES_SHAPE (array_btype, to_drop + i);
                ret_type
                  = MakeType (TYPES_BASETYPE (array_btype), dim, new_shpseg, NULL, NULL);
            } else
                ret_type
                  = MakeType (TYPES_BASETYPE (array_btype), SCALAR, NULL, NULL, NULL);
        } else if (UNKNOWN_SHAPE == TYPES_DIM (array_btype))
            /* The result-type is an array or an scalar
             */
            ret_type = MakeType (TYPES_BASETYPE (array_btype), ARRAY_OR_SCALAR, NULL,
                                 NULL, NULL);
        else if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype)) {
            /* only the dimension of array_btype is known
             */
            if (TYPES_SHAPE (vec, 0) < -TYPES_DIM (array_btype) + KNOWN_DIM_OFFSET)
            /* dimension of array_btype is greater than number umber of elements
             * of the access vector.
             * The result-type will be an array with known dimension
             */
#ifndef KNOWN_DIM
                ret_type = MakeType (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL,
                                     NULL, NULL);
#else
                ret_type = MakeType (TYPES_BASETYPE (array_btype),
                                     TYPES_DIM (array_btype) + TYPES_SHAPE (vec, 0), NULL,
                                     NULL, NULL);
#endif /* KNOWN_DIM */
            else if (TYPES_SHAPE (vec, 0) == -TYPES_DIM (array_btype) + KNOWN_DIM_OFFSET)
                /* dimension of array_btype is equal to number umber of elements
                 * of the access vector.
                 * The result-type is a scalar
                 */
                ret_type
                  = MakeType (TYPES_BASETYPE (array_btype), SCALAR, NULL, NULL, NULL);
            else
                ret_type = MakeType (T_unknown, SCALAR, NULL, NULL, NULL);
        } else {
            GEN_TYPE_NODE (ret_type, T_unknown);
        }
    } else if ((KNOWN_DIM_OFFSET - 1) == TYPES_DIM (vec)) {
        /* the dimension of vec is 1, but the shape is unknown
         * The result-type is an array or an scalar
         */
        ret_type
          = MakeType (TYPES_BASETYPE (array_btype), ARRAY_OR_SCALAR, NULL, NULL, NULL);

    } else if (UNKNOWN_SHAPE == TYPES_DIM (vec)) {
        /* The dimension iand the shape of vec are unknown.
         * The result-type is an array or an scalar.
         */
        ret_type
          = MakeType (TYPES_BASETYPE (array_btype), ARRAY_OR_SCALAR, NULL, NULL, NULL);
    } else {
        GEN_TYPE_NODE (ret_type, T_unknown);
    }
    FreeOneTypes (array_btype);
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

    if (SAC_PRG == kind_of_file) {
        if (N_num != NODE_TYPE (s_node))
            ERROR2 (3, ("%s, %d: 1.argument of function 'drop' "
                        "is not a constant",
                        filename, NODE_LINE (s_node)));

        DBUG_ASSERT (N_num == NODE_TYPE (s_node), "not N_num");
        if (1 <= TYPES_DIM (array)) {
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

                for (i = 1; i < array->dim; i++)
                    ret_type->shpseg->shp[i] = array->shpseg->shp[i];
            } else
                GEN_TYPE_NODE (ret_type, T_unknown);
        }
    } else {
        /* for modules only */
        if (N_num == NODE_TYPE (s_node)) {
            if (SCALAR < TYPES_DIM (array)) {
                if (((1 == tag) ? (s_node->info.cint < array->shpseg->shp[0])
                                : (s_node->info.cint <= array->shpseg->shp[0]))
                    && (0 <= s_node->info.cint)) {
                    GEN_TYPE_NODE (ret_type, array->simpletype);
                    ret_type->dim = 1;
                    ret_type->shpseg = (shpseg *)Malloc (sizeof (shpseg));
                    if (1 == tag)
                        /* drop */
                        ret_type->shpseg->shp[0]
                          = array->shpseg->shp[0] - s_node->info.cint;
                    else
                        /* take */
                        ret_type->shpseg->shp[0] = s_node->info.cint;

                    for (i = 1; i < array->dim; i++)
                        ret_type->shpseg->shp[i] = array->shpseg->shp[i];
                } else
                    GEN_TYPE_NODE (ret_type, T_unknown);
            } else if (KNOWN_DIM_OFFSET > TYPES_DIM (array))
                ret_type = MakeType (TYPES_BASETYPE (array), TYPES_DIM (array), NULL,
                                     NULL, NULL);
            else if (UNKNOWN_SHAPE == TYPES_DIM (array))
                ret_type = MakeType (TYPES_BASETYPE (array), TYPES_DIM (array), NULL,
                                     NULL, NULL);
            else
                DBUG_ASSERT (0, "wrong dimension of second argument (array)");
        } else {
            /* We don`t know the value of the first argument of 'take' or 'drop'.
             * We can only infer the dimension, but no shape.
             */
            if (SCALAR < TYPES_DIM (array))
#ifndef KNOWN_DIM
                ret_type
                  = MakeType (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
                ret_type
                  = MakeType (TYPES_BASETYPE (array),
                              KNOWN_DIM_OFFSET - TYPES_DIM (array), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            else if (KNOWN_DIM_OFFSET > TYPES_DIM (array))
                ret_type = MakeType (TYPES_BASETYPE (array), TYPES_DIM (array), NULL,
                                     NULL, NULL);
            else if (UNKNOWN_SHAPE == TYPES_DIM (array))
                ret_type = MakeType (TYPES_BASETYPE (array), TYPES_DIM (array), NULL,
                                     NULL, NULL);
            else
                DBUG_ASSERT (0, "wrong dimension of second argument (array)");
        }
    }
#if 0
   else
      if(1<array->dim)
      {
         if(((1==tag)?(s_node->info.cint < array->shpseg->shp[0])
             :(s_node->info.cint <= array->shpseg->shp[0]) ) &&
            (0 <=s_node->info.cint))
         {
            GEN_TYPE_NODE(ret_type, array->simpletype);
            ret_type->dim=array->dim;
            ret_type->shpseg=(shpseg*)Malloc(sizeof(shpseg));
            if(1==tag)
               /* drop */
               ret_type->shpseg->shp[0]=array->shpseg->shp[0]-s_node->info.cint;
            else
               /* take */
               ret_type->shpseg->shp[0]=s_node->info.cint;
            
            for(i=1;i<array->dim;i++)
               ret_type->shpseg->shp[i]=array->shpseg->shp[i];
         }
         else
            GEN_TYPE_NODE(ret_type,T_unknown);
      }
      else
         GEN_TYPE_NODE(ret_type,T_unknown);
#endif /* if 0 , kann geloescht werden */

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

    if (SAC_PRG == kind_of_file) {
        if (N_num == s_node->nodetype)
            if ((0 <= s_node->info.cint) && (s_node->info.cint < array->dim))
                ret_type = DuplicateTypes (array, 0);
            else {
                ERROR (s_node->lineno, ("1.argument of function 'rotate` is constant %d "
                                        "but rotated array has dimension %d",
                                        s_node->info.cint, array->dim));
                GEN_TYPE_NODE (ret_type, T_unknown);
            }
        else {
            ret_type = DuplicateTypes (array, 0);
        }
    } else {
        /* for modules only */
        if (N_num == NODE_TYPE (s_node)) {
            if (SCALAR < TYPES_DIM (array)) {
                if ((0 <= NUM_VAL (s_node)) && (NUM_VAL (s_node) < array->dim))
                    ret_type = DuplicateTypes (array, 0);
                else
                    GEN_TYPE_NODE (ret_type, T_unknown);
            } else if (KNOWN_DIM_OFFSET > TYPES_DIM (array)) {
                if ((0 <= NUM_VAL (s_node))
                    && (NUM_VAL (s_node) < POS_DIM (TYPES_DIM (array))))
                    ret_type = DuplicateTypes (array, 0);
                else
                    GEN_TYPE_NODE (ret_type, T_unknown);
            } else
                ret_type = DuplicateTypes (array, 0);
        } else
            /* the type of 'array' remains as it is */
            ret_type = DuplicateTypes (array, 0);
    }

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

    dim1 = TYPES_DIM (array1);
    dim2 = TYPES_DIM (array2);

    if (SAC_PRG == kind_of_file) {
        if (N_num == s_node->nodetype) {
            axis = NUM_VAL (s_node);

            if ((0 <= axis) && (axis < dim1) && (dim1 == dim2)) {
                for (i = 0; i < dim1; i++)
                    if (i != axis)
                        if (TYPES_SHAPE (array1, i) != TYPES_SHAPE (array2, i)) {
                            ok = 0;
                            break;
                        }
                if (1 == ok) {
                    ret_type = MakeType (TYPES_BASETYPE (array1), dim1, MakeShpseg (NULL),
                                         NULL, NULL);
                    for (i = 0; i < dim1; i++)
                        TYPES_SHAPE (ret_type, i) = TYPES_SHAPE (array1, i);
                    TYPES_SHAPE (ret_type, axis) += TYPES_SHAPE (array2, axis);
                } else
                    GEN_TYPE_NODE (ret_type, T_unknown);
            } else
                GEN_TYPE_NODE (ret_type, T_unknown);
        } else
            ERROR2 (3, ("%s, %d: 1.argument of function `cat` "
                        " should be a constant",
                        filename, s_node->lineno));
    } else
      /* for modules only */
      if ((SCALAR < dim1) && (SCALAR < dim2)) {
        /* both arrays have known shape */
        if (dim1 == dim2) {
            if (N_num == NODE_TYPE (s_node)) {
                axis = NUM_VAL (s_node);

                if ((0 <= axis) && (axis < dim1)) {
                    for (i = 0; i < dim1; i++)
                        if (i != axis)
                            if (TYPES_SHAPE (array1, i) != TYPES_SHAPE (array2, i)) {
                                ok = 0;
                                break;
                            }
                    if (1 == ok) {
                        ret_type = MakeType (TYPES_BASETYPE (array1), dim1,
                                             MakeShpseg (NULL), NULL, NULL);
                        for (i = 0; i < dim1; i++)
                            TYPES_SHAPE (ret_type, i) = TYPES_SHAPE (array1, i);
                        TYPES_SHAPE (ret_type, axis) += TYPES_SHAPE (array2, axis);
                    } else
                        GEN_TYPE_NODE (ret_type, T_unknown);
                } else
                    ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
            } else
#ifndef KNOWN_DIM
                ret_type
                  = MakeType (TYPES_BASETYPE (array1), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
                ret_type = MakeType (TYPES_BASETYPE (array1), KNOWN_DIM_OFFSET - dim1,
                                     NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        } else
            ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);

    } else if ((SCALAR < dim1) && (KNOWN_DIM_OFFSET > dim2)) {
        if (dim2 == (KNOWN_DIM_OFFSET - dim1))
            ret_type = MakeType (TYPES_BASETYPE (array1), dim2, NULL, NULL, NULL);
        else
            ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
    } else if ((SCALAR < dim1) && (UNKNOWN_SHAPE == dim2))
#ifndef KNOWN_DIM
        ret_type
          = MakeType (TYPES_BASETYPE (array1), KNOWN_DIM_OFFSET - dim1, NULL, NULL, NULL);
#else
        ret_type = MakeType (TYPES_BASETYPE (array1), UNKNOWN_SHAPE, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
    else if ((SCALAR < dim2) && (KNOWN_DIM_OFFSET > dim1)) {
        if ((dim1 == (KNOWN_DIM_OFFSET - dim2)) && IS_IN_RANGE (s_node, dim1))
            ret_type = MakeType (TYPES_BASETYPE (array1), dim1, NULL, NULL, NULL);
        else
            ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
    } else if ((SCALAR < dim2) && (UNKNOWN_SHAPE == dim1)) {
        if (IS_IN_RANGE (s_node, dim2))
            ret_type = MakeType (TYPES_BASETYPE (array1), dim1, NULL, NULL, NULL);
        else
            ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
    } else if ((UNKNOWN_SHAPE == dim1) && (UNKNOWN_SHAPE == dim2))
        ret_type = MakeType (TYPES_BASETYPE (array1), dim1, NULL, NULL, NULL);
    else if ((UNKNOWN_SHAPE == dim1) && (KNOWN_DIM_OFFSET > dim2)) {
        if (IS_IN_RANGE (s_node, POS_DIM (dim2)))
            ret_type = MakeType (TYPES_BASETYPE (array1), dim2, NULL, NULL, NULL);
        else
            ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
    } else if ((UNKNOWN_SHAPE == dim2) && (KNOWN_DIM_OFFSET > dim1)) {
        if (IS_IN_RANGE (s_node, POS_DIM (dim1)))
            ret_type = MakeType (TYPES_BASETYPE (array1), dim1, NULL, NULL, NULL);
        else
            ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
    } else if ((KNOWN_DIM_OFFSET > dim1) && (KNOWN_DIM_OFFSET > dim2)) {
        if ((dim1 == dim2) && (IS_IN_RANGE (s_node, POS_DIM (dim1))))
#ifndef KNOWN_DIM
            ret_type
              = MakeType (TYPES_BASETYPE (array1), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
            ret_type = MakeType (TYPES_BASETYPE (array1), dim1, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        else
            ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
    } else
        DBUG_ASSERT (0, "wrong dimensions");

    DBUG_ASSERT (NULL != ret_type, "no ret_type");

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : ConvertType
 *  arguments     : 1) type of array
 *                  2) new simpletype
 *  description   :  returns the type 2) with shape of 1)
 *  global vars   :
 *  internal funs :
 *  external funs : DuplicateTypes
 *  macros        : DBUG...,
 *
 *  remarks       : is part of macros TT1 & TT2 and is used in typecheck.c
 *
 */
types *
ConvertType (types *array1, simpletype s_type)
{
    types *ret_type = NULL;

    DBUG_ENTER ("ConvertType");

    GET_BASIC_TYPE (ret_type, array1, 0);
    ret_type->simpletype = s_type;
    ret_type->name = NULL; /* has to be set to NULL, because of output */
    ret_type->name_mod = NULL;

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Modarray
 *  arguments     : 1) type of array to modify
 *                  2) type of access vector
 *                  3) type of new value
 *  description   :  returns the type 1)
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., GET_BASIC_TYPE
 *
 *  remarks       : is part of macros TT1 & TT2 and is used in typecheck.c
 *
 */
types *
Modarray (types *array, types *vec, types *value, int line)
{
    types *b_array, *b_vec, *b_value;
    types *ret_type = NULL;
    int types_ok = 1;

    DBUG_ENTER ("Modarray");

    GET_BASIC_TYPE (b_array, array, line);
    GET_BASIC_TYPE (b_vec, vec, line);
    GET_BASIC_TYPE (b_value, value, line);

    DBUG_PRINT ("PRF_TYPE", ("b_array: %s", Type2String (b_array, 0)));
    DBUG_PRINT ("PRF_TYPE", ("b_vec: %s", Type2String (b_vec, 0)));
    DBUG_PRINT ("PRF_TYPE", ("b_value: %s", Type2String (b_value, 0)));

    if (SAC_PRG == kind_of_file) {
        if (b_vec->dim != 1) {
            ERROR2 (3, ("%s, %d: 2.argument of function 'modarray' has incompatible "
                        "type ( int[x] != %s)",
                        filename, line, Type2String (vec, 0)));
        }
        DBUG_ASSERT (1 == TYPES_DIM (b_vec), " wrong access vector");

        if (b_array->dim == b_vec->shpseg->shp[0] + b_value->dim) {
            int i, j;

            for (i = b_vec->shpseg->shp[0], j = 0; i < b_array->dim; i++, j++) {
                DBUG_ASSERT (j < b_value->dim, " out of shape range");

                if (b_array->shpseg->shp[i] != b_value->shpseg->shp[j]) {
                    types_ok = 0;
                    break;
                }
            }

        } else
            types_ok = 0;

    } else {
        /* for modules only */

        if (1 == TYPES_DIM (b_vec)) {
            if (SCALAR < TYPES_DIM (b_array)) {
                if (SCALAR < TYPES_DIM (b_value)) {
                    /* now check whether the informations fit */
                    if (b_array->dim == b_vec->shpseg->shp[0] + b_value->dim) {
                        int i, j;

                        for (i = b_vec->shpseg->shp[0], j = 0; i < b_array->dim;
                             i++, j++) {
                            DBUG_ASSERT (j < b_value->dim, " out of shape range");

                            if (b_array->shpseg->shp[i] != b_value->shpseg->shp[j]) {
                                types_ok = 0;
                                break;
                            }
                        }
                    }
                } else if (SCALAR == TYPES_DIM (b_value)) {
                    if (TYPES_SHAPE (b_vec, 0) != TYPES_DIM (b_array))
                        types_ok = 0;
                } else if (UNKNOWN_SHAPE == TYPES_DIM (b_value)) {
                    if (TYPES_SHAPE (b_vec, 0) >= TYPES_DIM (b_array))
                        types_ok = 0;
                } else if (KNOWN_DIM_OFFSET > TYPES_DIM (b_value)) {
                    if ((KNOWN_DIM_OFFSET - TYPES_DIM (b_array))
                        != (TYPES_DIM (b_value) - TYPES_SHAPE (b_vec, 0)))
                        types_ok = 0;
                }
            } else if (KNOWN_DIM_OFFSET > TYPES_DIM (b_array)) {
                if (SCALAR < TYPES_DIM (b_value)) {
                    if (TYPES_DIM (b_array)
                        != (KNOWN_DIM_OFFSET
                            - (TYPES_DIM (b_value) + TYPES_SHAPE (b_vec, 1))))
                        types_ok = 0;
                } else if (UNKNOWN_SHAPE == TYPES_DIM (b_value)) {
                    if (TYPES_DIM (b_array) > (KNOWN_DIM_OFFSET - TYPES_SHAPE (b_vec, 1)))
                        types_ok = 0;
                } else if (KNOWN_DIM_OFFSET > TYPES_DIM (b_value)) {
                    if (TYPES_DIM (b_array)
                        != (TYPES_DIM (b_value) - TYPES_SHAPE (b_vec, 1)))
                        types_ok = 0;
                }
            } else if (UNKNOWN_SHAPE == TYPES_DIM (b_array)) {
                /* nothing to  check  */
            }
        } else if ((UNKNOWN_SHAPE == TYPES_DIM (b_vec))
                   || (KNOWN_DIM_OFFSET - 1 == TYPES_DIM (b_vec))) {
            /* access vector has unknown shape or its dimension 1, but shape
             * isn't known
             */
            if (SCALAR < TYPES_DIM (b_array)) {
                /* known shape of b_array */
                if (SCALAR < TYPES_DIM (b_value)) {
                    if (TYPES_DIM (b_array) > TYPES_DIM (b_value)) {
                        int i, j;

                        for (i = TYPES_DIM (b_array) - TYPES_DIM (b_value), j = 0;
                             j < TYPES_DIM (b_value); i++, j++) {
                            DBUG_ASSERT (i < TYPES_DIM (b_array), " out of shape range");

                            if (TYPES_SHAPE (b_array, i) != TYPES_SHAPE (b_value, j)) {
                                types_ok = 0;
                                break;
                            }
                        }
                    }
                } else if (KNOWN_DIM_OFFSET > TYPES_DIM (b_value)) {
                    if ((KNOWN_DIM_OFFSET - TYPES_DIM (b_array)) > TYPES_DIM (b_value))
                        types_ok = 0;
                }
            } else if (KNOWN_DIM_OFFSET > TYPES_DIM (b_array)) {
                /* known dimension only */
                if (SCALAR < TYPES_DIM (b_value)) {
                    if (TYPES_DIM (b_array) > (KNOWN_DIM_OFFSET - TYPES_DIM (b_value)))
                        types_ok = 0;
                } else if (KNOWN_DIM_OFFSET > TYPES_DIM (b_value)) {
                    if (TYPES_DIM (b_array) > TYPES_DIM (b_value))
                        types_ok = 0;
                }
                /* nothing to check for: UNKNOWN_SHAPE== TYPES_DIM(b_value)
                 *  or SCALAR == TYPES_DIM(b_value)
                 */
            }
        } else
            /* wrong dimesnison of access vector */
            types_ok = 0;
    }

    if (0 == types_ok) {
        ERROR2 (3, ("%s, %d: arguments of function 'modarray' have wrong "
                    "types ( %s, %s, %s)",
                    filename, line, Type2String (array, 0), Type2String (vec, 0),
                    Type2String (value, 0)));
    } else {
        FREE (b_vec);
        FREE (b_value);
        ret_type = b_array;
    }

    DBUG_ASSERT (ret_type != NULL, " wrong return value");

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Genarray_S
 *  arguments     : 1) node to constant array (vector)
 *                  2) type of vector 1)
 *                  3) type of an array
 *  description   : computes the resulttype of a 'genarray' operation
 *                  whoes first argument is a constant vector (only if checking
 *                  SAC_PRGs).
 *  global vars   :
 *  internal funs :
 *  external funs : Malloc
 *  macros        : DBUG...,
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */
types *
Genarray_S (node *v_node, types *vec, types *scalar)
{
    types *ret_type;
    int dim = 0;
    shpseg *shpseg_p;
    node *tmp;

    DBUG_ENTER ("Genarray_S");

    if (SAC_PRG == kind_of_file) {
        if (N_array == NODE_TYPE (v_node)) {
            int ok = 1;
            /* create shpseg of resulting type */
            shpseg_p = MakeShpseg (NULL);
            tmp = ARRAY_AELEMS (v_node);
            do {
                if (NODE_TYPE (EXPRS_EXPR (tmp)) != N_num) {
                    ok = 0;
                    break;
                } else {
                    SHPSEG_SHAPE (shpseg_p, dim) = NUM_VAL (EXPRS_EXPR (tmp));
                    tmp = EXPRS_NEXT (tmp);
                    dim++;
                }
            } while (NULL != tmp);

            /* create resulting types */
            if (1 == ok)
                ret_type
                  = MakeType (TYPES_BASETYPE (scalar), dim, shpseg_p,
                              StringCopy (TYPES_NAME (scalar)), TYPES_MOD (scalar));
            else
                ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);

        } else
            ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
        /*
         * DBUG_ASSERT(0," wrong first argument of primitive function genarray");
         */
    } else {
        /* checking in a module */
        if (N_array == NODE_TYPE (v_node)) {

            /* create shpseg of resulting type */
            shpseg_p = MakeShpseg (NULL);
            tmp = ARRAY_AELEMS (v_node);
            do {
                SHPSEG_SHAPE (shpseg_p, dim) = NUM_VAL (EXPRS_EXPR (tmp));
                tmp = EXPRS_NEXT (tmp);
                dim++;
            } while (NULL != tmp);

            /* create resulting types */
            ret_type = MakeType (TYPES_BASETYPE (scalar), dim, shpseg_p,
                                 StringCopy (TYPES_NAME (scalar)), TYPES_MOD (scalar));
        } else {
            /* there isn't a constant vector */
            if ((1 == TYPES_DIM (vec)) || ((KNOWN_DIM_OFFSET - 1) == TYPES_DIM (vec))
                || (UNKNOWN_SHAPE == TYPES_DIM (vec)))
                ret_type
                  = MakeType (TYPES_BASETYPE (scalar), UNKNOWN_SHAPE, NULL,
                              StringCopy (TYPES_NAME (scalar)), TYPES_MOD (scalar));
            else
                ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
        }
    }

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Genarray_A
 *  arguments     : 1) node to constant array (vector)
 *                  2) type of access vector (1)
 *                  3) type of an array
 *  description   : computes the resulttype of a 'genarray' operation
 *                  whoes first argument is a constant vector.
 *  global vars   :
 *  internal funs :
 *  external funs : Malloc
 *  macros        : DBUG...,
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */
types *
Genarray_A (node *v_node, types *vec, types *array)
{
    types *ret_type;
    int i, dim = 0;

    DBUG_ENTER ("Genarray_A");
    if (SAC_PRG == kind_of_file) {
        if (N_array == NODE_TYPE (v_node)) {
            ret_type = Genarray_S (v_node, vec, array);
            dim = TYPES_DIM (ret_type);
            for (i = 0; i < TYPES_DIM (array); i++) {
                DBUG_ASSERT (dim < SHP_SEG_SIZE, " shape is out of range");
                TYPES_SHAPE (ret_type, dim) = TYPES_SHAPE (array, i);
                dim++;
            }
            TYPES_DIM (ret_type) = dim;
        } else
            ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
    } else if (SAC_MOD == kind_of_file) {
        /* we are checking a funktion in a module */
        if (N_array == NODE_TYPE (v_node)) {
            if (UNKNOWN_SHAPE == TYPES_DIM (array)) {
                ret_type
                  = MakeType (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
            } else if (KNOWN_DIM_OFFSET > TYPES_DIM (array)) {
#ifdef KNOWN_DIM
                ret_type = MakeType (TYPES_BASETYPE (array),
                                     (TYPES_DIM (array) - TYPES_SHAPE (vec, 0)), NULL,
                                     NULL, NULL);
#else
                ret_type
                  = MakeType (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            } else if (SCALAR < TYPES_DIM (array)) {
                ret_type = Genarray_S (v_node, vec, array);
                dim = TYPES_DIM (ret_type);
                for (i = 0; i < TYPES_DIM (array); i++) {
                    DBUG_ASSERT (dim < SHP_SEG_SIZE, " shape is out of range");
                    TYPES_SHAPE (ret_type, dim) = TYPES_SHAPE (array, i);
                    dim++;
                }
                TYPES_DIM (ret_type) = dim;
            }
        } else if (1 == TYPES_DIM (vec)) {
            if (SCALAR < TYPES_DIM (array))
#ifdef KNOWN_DIM
                ret_type = MakeType (TYPES_BASETYPE (array),
                                     (KNOWN_DIM_OFFSET
                                      - (TYPES_SHAPE (vec, 0) + TYPES_DIM (array))),
                                     NULL, NULL, NULL);
#else
                ret_type
                  = MakeType (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            else if (KNOWN_DIM_OFFSET < TYPES_DIM (array))
#ifdef KNOWN_DIM
                ret_type = MakeType (TYPES_BASETYPE (array),
                                     (TYPES_DIM (array) - TYPES_SHAPE (vec, 0)), NULL,
                                     NULL, NULL);
#else
                ret_type
                  = MakeType (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            else if (UNKNOWN_SHAPE == TYPES_DIM (array))
                ret_type
                  = MakeType (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
            else
                ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
        } else if ((UNKNOWN_SHAPE == TYPES_DIM (vec))
                   || ((KNOWN_DIM_OFFSET - 1) == TYPES_DIM (vec)))
            ret_type = MakeType (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
        else
            ret_type = MakeType (T_unknown, 0, NULL, NULL, NULL);
    }

    DBUG_RETURN (ret_type);
}
