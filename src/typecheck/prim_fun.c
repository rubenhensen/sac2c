/*
 *
 * $Log$
 * Revision 3.13  2001/07/17 08:37:28  nmw
 * call to MakeArg() without type fixed
 *
 * Revision 3.12  2001/07/16 08:23:11  cg
 * Old tree construction function MakeNode eliminated.
 *
 * Revision 3.11  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.10  2001/05/17 11:34:07  sbs
 * return value of Free now used ...
 *
 * Revision 3.9  2001/05/17 09:20:42  sbs
 * MALLOC FREE aliminated
 *
 * Revision 3.8  2001/04/24 09:16:15  dkr
 * P_FORMAT replaced by F_PTR
 *
 * Revision 3.7  2001/03/22 20:37:49  dkr
 * no changes done
 *
 * Revision 3.6  2001/03/22 14:02:27  sbs
 * S_S added for F_abs
 *
 * Revision 3.5  2001/03/15 15:17:51  dkr
 * signature of Type2String modified
 *
 * Revision 3.4  2001/02/23 18:04:22  sbs
 * extended for negative take's and drop's in genarray
 *
 * Revision 3.3  2001/02/02 10:29:53  dkr
 * superfluous include of access_macros.h removed
 *
 * Revision 3.2  2000/11/27 13:14:43  sbs
 * warning eliminated
 *
 * Revision 3.1  2000/11/20 18:00:11  sacbase
 * new release made
 *
 * Revision 2.21  2000/11/15 14:07:44  sbs
 * additional {}'s added to please muttering gcc on ALPHA.
 *
 * Revision 2.20  2000/10/26 14:30:23  dkr
 * MakeShpseg used instead of MALLOC
 *
 * Revision 2.19  2000/10/24 11:46:55  dkr
 * MakeTypes renamed into MakeTypes1
 * MakeType renamed into MakeTypes
 *
 * Revision 2.18  2000/10/20 15:27:54  dkr
 * macro GET_DIM replaced by function GetDim()
 *
 * Revision 2.17  2000/08/08 11:50:07  dkr
 * definition of TT1, TT2, TT3 moved
 *
 * Revision 2.16  2000/08/04 17:21:15  dkr
 * NEWTREE removed
 *
 * Revision 2.15  2000/07/12 15:13:26  dkr
 * function DuplicateTypes renamed into DupTypes
 *
 * Revision 2.14  2000/07/11 10:19:27  dkr
 * code for OLD_GET_TYPE_NODE removed
 *
 * Revision 2.13  2000/02/17 17:04:17  cg
 * Prototype of function DuplicateTypes() is now included from
 * DupTree.h instead of typecheck.h.
 *
 * Revision 2.12  1999/06/28 14:44:36  cg
 * ID_ISCONST is now used instead of ID_VECLEN in order to determine
 * whether an identifier represents a constant vector or not.
 *
 * Revision 2.11  1999/06/17 14:29:22  sbs
 * patched the typechecker in order to accept programs which only require
 * the dimensionality rather than the exact shape of all arrays to be
 * known statically.
 * This only will be done in case "dynamic_shapes == 1" which can be
 * achieved by calling sac2c -ds!
 *
 * Revision 2.10  1999/05/31 16:54:54  sbs
 * bug in Genarray_S eliminated :
 * instead of checking for ID_VECLEN> SCALAR we now check for
 * ID_ISCONST(v_node).
 *
 * Revision 2.9  1999/05/18 12:06:21  jhs
 * Deleted a warning occuring while compiling -O3 (e.g. Linux),
 * but there are still some in the code ...
 *
 * Revision 2.8  1999/05/12 09:15:46  jhs
 * Adjusted macros to access constant vectors.
 *
 * Revision 2.7  1999/05/12 08:37:54  jhs
 * Deleted some warnigs occuring while compiling under Linux
 * (there are still some left).
 *
 * Revision 2.6  1999/05/11 16:14:14  jhs
 * Tried to delete some warnings occuring while compiling for
 * Linux.
 *
 * Revision 2.5  1999/05/05 09:19:40  jhs
 * TakeV and DropV modified. Both routines now handle flattened empty
 * arrays as arguments of Take and Drop.
 *
 * Revision 2.4  1999/04/16 11:47:30  jhs
 * Changes made for empty arrays.
 *
 * Revision 2.3  1999/03/15 14:02:28  bs
 * Access macros renamed (take a look at tree_basic.h).
 *
 * Revision 2.2  1999/02/25 11:02:28  bs
 * Genarray_S, Reshp, DropV and TakeV modified. Now these functions
 * are able to work with the compact propagation of constant
 * integer arrays.
 *
 * Revision 2.1  1999/02/23 12:40:48  sacbase
 * new release made
 *
 * [...]
 *
 * Revision 1.1  1995/02/03  07:45:32  hw
 * Initial revision
 *
 */

#include <stdlib.h>

#include "tree.h" /* old tree definition */
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "globals.h"
#include "prim_fun.h"
#include "dbug.h"
#include "Error.h"
#include "my_debug.h"
#include "DupTree.h"
#include "convert.h"
#include "free.h"
#include "typecheck.h"

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
    S_S,
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

#define GEN_TYPE_NODE(types, simpletype)                                                 \
    types = MakeTypes (simpletype, SCALAR, NULL, NULL, NULL)

#define GEN_PRIM_FUN_TAB_ELEM(p_old, mod, node_p, t_tag, u_tag, p_new)                   \
    tmp = (prim_fun_tab_elem *)Malloc (sizeof (prim_fun_tab_elem));                      \
    tmp->prf = p_old;                                                                    \
    tmp->id_mod = mod;                                                                   \
    tmp->node = node_p;                                                                  \
    tmp->typed_tag = t_tag;                                                              \
    tmp->user_tag = u_tag;                                                               \
    tmp->new_prf = p_new;                                                                \
    tmp->next = NULL;                                                                    \
    DBUG_PRINT ("PRIM_FUN", ("prim_fun_elem:" F_PTR " %s  fun_dec:" F_PTR, tmp,          \
                             mdb_prf[p_new], node_p))

#define INT GEN_TYPE_NODE (type, T_int)
#define FLOAT GEN_TYPE_NODE (type, T_float)
#define BOOL GEN_TYPE_NODE (type, T_bool)
#define CHAR GEN_TYPE_NODE (type, T_char)
#define INT_A                                                                            \
    type = MakeTypes1 (T_int);                                                           \
    type->dim = -1;                                                                      \
    DBUG_PRINT ("PRIM_FUN", ("param: int[]" F_PTR, type))
#define FLOAT_A                                                                          \
    type = MakeTypes1 (T_float);                                                         \
    type->dim = -1;                                                                      \
    DBUG_PRINT ("PRIM_FUN", ("param: float[]" F_PTR, type))
#define BOOL_A                                                                           \
    type = MakeTypes1 (T_bool);                                                          \
    type->dim = -1;                                                                      \
    DBUG_PRINT ("PRIM_FUN", ("param: bool[]" F_PTR, type))
#define CHAR_A                                                                           \
    type = MakeTypes1 (T_char);                                                          \
    type->dim = -1;                                                                      \
    DBUG_PRINT ("PRIM_FUN", ("param: char[]" F_PTR, type))
#define DOUBLE GEN_TYPE_NODE (type, T_double)
#define DOUBLE_A                                                                         \
    type = MakeTypes1 (T_double);                                                        \
    type->dim = -1;                                                                      \
    DBUG_PRINT ("PRIM_FUN", ("param: double[]" F_PTR, type))

#define INT_V                                                                            \
    type = MakeTypes1 (T_int);                                                           \
    type->dim = KNOWN_DIM_OFFSET - 1;                                                    \
    DBUG_PRINT ("PRIM_FUN", ("param: int[.]" F_PTR, type))

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

    /*
     * For the time being the primitive functions take, drop, and cat
     * are left intrinsic. However, if their intrinsic implementation
     * is not explicitly requested by the -intrinsic option, applications
     * of these functions are subsequently expanded to equivalent with-loops.
     * See function TClet() in file typecheck.c for details.
     */

    switch (prf_old) {
    case F_toi:
    case F_toi_A:
    case F_tof:
    case F_tof_A:
    case F_tod:
    case F_tod_A:
        wanted = (INTRINSIC_TO & intrinsics);
        intrinsic = (wanted || (type_c == f_i) || (type_c == d_i) || (type_c == i_f)
                     || (type_c == d_f) || (type_c == i_d) || (type_c == f_d));
        break;
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
    case F_rotate:
        intrinsic = (INTRINSIC_ROT & intrinsics);
        break;
    case F_take:
        intrinsic = (INTRINSIC_TAKE & intrinsics);
        /* break; */
    case F_drop:
        intrinsic = (INTRINSIC_DROP & intrinsics);
        /* break; */
    case F_cat:
        intrinsic = (INTRINSIC_CAT & intrinsics);
        /* break; */
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

#define TT1(n, a, t1, res)                                                               \
    tmp_node->node[1] = MakeFundef (NULL, NULL, NULL, NULL, NULL, NULL);                 \
    DBUG_PRINT ("PRIM_FUN", ("prim_fun_dec: " F_PTR, tmp_node->node[1]));                \
    t1;                                                                                  \
    arg1 = MakeArg (NULL, type, ST_regular, ST_regular, NULL);                           \
    tmp_node->node[1]->node[2] = arg1;                                                   \
    tmp_node->node[1]->info.prf_dec.tc = a;                                              \
    DBUG_PRINT ("PRIM_FUN", ("tc: %d (%d)", tmp_node->node[1]->info.prf_dec.tc, a));     \
    tmp_node->node[1]->info.prf_dec.tag = n;                                             \
    DBUG_PRINT ("PRIM_FUN",                                                              \
                ("tc: %d (%d), tag: %d (%d)", tmp_node->node[1]->info.prf_dec.tc, a,     \
                 tmp_node->node[1]->info.prf_dec.tag, n));                               \
    tmp_node = tmp_node->node[1];
#define TT2(n, a, t1, t2, res)                                                           \
    tmp_node->node[1] = MakeFundef (NULL, NULL, NULL, NULL, NULL, NULL);                 \
    DBUG_PRINT ("PRIM_FUN", ("prim_fun_dec: " F_PTR, tmp_node->node[1]));                \
    t2;                                                                                  \
    arg2 = MakeArg (NULL, type, ST_regular, ST_regular, NULL);                           \
    t1;                                                                                  \
    arg1 = MakeArg (NULL, type, ST_regular, ST_regular, arg2);                           \
    tmp_node->node[1]->node[2] = arg1;                                                   \
    tmp_node->node[1]->info.prf_dec.tc = a;                                              \
    tmp_node->node[1]->info.prf_dec.tag = n;                                             \
    tmp_node = tmp_node->node[1];
#define TT3(n, a, t1, t2, t3, res)                                                       \
    tmp_node->node[1] = MakeFundef (NULL, NULL, NULL, NULL, NULL, NULL);                 \
    DBUG_PRINT ("PRIM_FUN", ("prim_fun_dec: " F_PTR, tmp_node->node[1]));                \
    t3;                                                                                  \
    arg3 = MakeArg (NULL, type, ST_regular, ST_regular, NULL);                           \
    t2;                                                                                  \
    arg2 = MakeArg (NULL, type, ST_regular, ST_regular, arg3);                           \
    t1;                                                                                  \
    arg1 = MakeArg (NULL, type, ST_regular, ST_regular, arg2);                           \
    tmp_node->node[1]->node[2] = arg1;                                                   \
    tmp_node->node[1]->info.prf_dec.tc = a;                                              \
    tmp_node->node[1]->info.prf_dec.tag = n;                                             \
    tmp_node = tmp_node->node[1];
#include "prim_fun_tt.mac"
#undef TT1
#undef TT2
#undef TT3
    tmp_node = prim_fun_dec;
    prim_fun_dec = prim_fun_dec->node[1];
    tmp_node = Free (tmp_node);

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
    prim_fun_p = Free (prim_fun_p);

    DBUG_VOID_RETURN;
}

/*
 *  MakeTypeNN (MakeTypeUnnamed)
 *
 *  Shortcut for calling MakeType with last two arguments NULL.
 *  For further documentation see MakeType.
 */

static types *
MakeTypeNN (simpletype basetype, int dim, shpseg *shpseg)
{
    types *result_type;

    DBUG_ENTER ("MakeTypeNN");

    result_type = MakeTypes (basetype, dim, shpseg, NULL, NULL);

    DBUG_RETURN (result_type);
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
                ret_type = DupTypes (array1);
            else
                ret_type = DupTypes (array2);
            ret_type->simpletype = s_type;
            break;
        case 1:
            if (1 == ret)
                ret_type = tmp_array1;
            else {
                ret_type = DupTypes (array2);
                FreeOneTypes (tmp_array1);
            }
            ret_type->simpletype = s_type;
            break;
        case 2:
            if (2 == ret)
                ret_type = tmp_array2;
            else {
                ret_type = DupTypes (array1);
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
 *  remarks       : is part of macro TT1 and is used in typecheck.c
 *
 */

types *
Shp (types *array)
{
    types *ret_type;
    int dim, n;

    DBUG_ENTER ("Shp");

    if (UNKNOWN_SHAPE == TYPES_DIM (array)) {
        /*
         * shape( A:int[]) : int[.] !
         */
        ret_type = MakeTypes (T_int, KNOWN_DIM_OFFSET - 1, NULL, NULL, NULL);
    } else if (KNOWN_DIM_OFFSET > TYPES_DIM (array)) {
        /*
         * shape( A:int[., ..., .]) : int[ n] !
         *              \---n---/
         */
        n = KNOWN_DIM_OFFSET - TYPES_DIM (array);
        ret_type = MakeTypes (T_int, 1, MakeShpseg (NULL), NULL, NULL);
        TYPES_SHAPE (ret_type, 0) = n;
    } else if (SCALAR < TYPES_DIM (array)) {
        /*
         * shape( A:int[s0, ..., sn-1]) : int[ n] !
         */
        ret_type = MakeTypes (T_int, 1, MakeShpseg (NULL), NULL, NULL);
        dim = GetDim (array);
        TYPES_SHAPE (ret_type, 0) = dim;
    } else {
        DBUG_ASSERT (0, "unknown dimension of type");
        ret_type = NULL;
    }

    DBUG_RETURN (ret_type);
}

/*
 *  functionname  : Reshp
 *  arguments     : 1) node of shape-vector
 *                  2) type of an array
 *                  3) type of shape vector
 *
 *  description   : returns the type of an array with shape 2) if number of
 *                  elements of old and new array are equal
 *                  or returns T_unkown if not
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */

types *
Reshp (node *vec, types *array, types *shp_vec)
{
    int count1, count2, i, *tmp1;
    int dim1 = 0, dim2 = 0;
    node *tmp;
    types *ret_type;

    DBUG_ENTER ("Reshp");

    if (NODE_TYPE (vec) == N_array) {
        /*
         * count number of elements of the new array  and store it in count1
         */
        DBUG_ASSERT (NODE_TYPE (vec) == N_array, " wrong node != N_array");
        tmp = ARRAY_AELEMS (vec);
        count1 = 1;
        while (tmp != NULL) {
            dim1++;
            count1 = count1 * NUM_VAL (EXPRS_EXPR (tmp));
            tmp = EXPRS_NEXT (tmp);
        }

        /*
         * count number of elements of old array 1)
         */
        count2 = 1;
        dim2 = TYPES_DIM (array);
        for (i = 0; i < dim2; i++)
            count2 = count2 * TYPES_SHAPE (array, i);

        if (dim2 <= UNKNOWN_SHAPE) {
            /*
             * TODO: replace UNKNOWN_SHAPE with KNOWN_DIM_OFFSET-dim1 later on
             */
#ifndef KNOWN_DIM
            ret_type
              = MakeTypes (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
            ret_type = MakeTypes (TYPES_BASETYPE (array), KNOWN_DIM_OFFSET - dim1, NULL,
                                  NULL, NULL);
#endif /* KNOWN_DIM */
        } else {
            if ((count1 != count2) || (dim1 > SHP_SEG_SIZE))
                GEN_TYPE_NODE (ret_type, T_unknown);
            else {
                ret_type = MakeTypes (TYPES_BASETYPE (array), dim1, MakeShpseg (NULL),
                                      NULL, NULL);
                tmp = ARRAY_AELEMS (vec);
                for (i = 0; i < dim1; i++) {
                    TYPES_SHAPE (ret_type, i) = NUM_VAL (EXPRS_EXPR (tmp));
                    tmp = EXPRS_NEXT (tmp);
                }
            }
        }
    } else {
        /*
         * if flattened :
         */
        if ((N_id == NODE_TYPE (vec)) && ID_ISCONST (vec)) {
            /*
             * count number of elements of the new array
             */
            count1 = 1;
            tmp1 = (int *)ID_CONSTVEC (vec);
            dim1 = ID_VECLEN (vec);
            for (i = 0; i < dim1; i++)
                count1 = count1 * tmp1[i];

            /*
             * count number of elements of the old array
             */
            count2 = 1;
            dim2 = TYPES_DIM (array);
            for (i = 0; i < dim2; i++)
                count2 = count2 * TYPES_SHAPE (array, i);

            if (dim2 <= UNKNOWN_SHAPE) {
                /*
                 * TODO: replace UNKNOWN_SHAPE with KNOWN_DIM_OFFSET-dim1 later on
                 */
#ifndef KNOWN_DIM
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
                ret_type = MakeTypes (TYPES_BASETYPE (array), KNOWN_DIM_OFFSET - dim1,
                                      NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            } else {
                if ((count1 != count2) || (dim1 > SHP_SEG_SIZE))
                    GEN_TYPE_NODE (ret_type, T_unknown);
                else {
                    ret_type = MakeTypes (TYPES_BASETYPE (array), dim1, MakeShpseg (NULL),
                                          NULL, NULL);
                    for (i = 0; i < dim1; i++)
                        TYPES_SHAPE (ret_type, i) = tmp1[i];
                }
            }
        } else {
            if (kind_of_file == SAC_MOD) {
                /*
                 * checking a module, so the first argument of "reshape" has not to be
                 * a constant array
                 */
                if (TYPES_DIM (shp_vec) == 1) {
                    /*
                     * we only know the dimension of the resulting type
                     */
#ifndef KNOWN_DIM
                    ret_type = MakeTypes (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL,
                                          NULL, NULL);
#else
                    ret_type = MakeTypes (TYPES_BASETYPE (array),
                                          KNOWN_DIM_OFFSET - TYPES_SHAPE (shp_vec, 0),
                                          NULL, NULL, NULL);
#endif /* KNOWN_DIM */
                } else if ((TYPES_DIM (shp_vec) == UNKNOWN_SHAPE)
                           || (TYPES_DIM (shp_vec) == (KNOWN_DIM_OFFSET - 1))) {
                    /*
                     * we don't know the dimenion and the shape of the resulting type
                     */
                    ret_type = MakeTypes (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL,
                                          NULL, NULL);
                } else {
                    ERROR2 (3,
                            ("%s, %d: 1.argument of function `reshape` "
                             " has wrong type (%s)",
                             filename, NODE_LINE (vec), Type2String (shp_vec, 0, TRUE)));
                }
            } else {
                /*
                 * checking a programm, so first argument of "reshape" has to be an
                 * constant array, but it isn't.
                 */
                ERROR2 (3, ("%s, %d: 1.argument of function `reshape` "
                            " should be a constant vector",
                            filename, NODE_LINE (vec)));
            }
        }
    }
    DBUG_RETURN (ret_type);
}

/*
 *  functionname  : TakeDropV
 *  arguments     : 1) type of shape-vector
 *                  2) type of an array
 *
 *  description   : computes the result-type of a 'take' or 'drop' operation,
 *                  whoes first argument is NOT a constant vector
 *
 */

types *
TakeDropV (types *vec_type, types *array_btype)
{
    types *ret_type;

    DBUG_ENTER ("TakeDropV");

    if (TYPES_DIM (vec_type) == 1) {
        /*
         *  access-vector is not a constant vector, but the shape is known
         */
        if (SCALAR < TYPES_DIM (array_btype)) {
            /*  the shape of the arrray is known */
            if (TYPES_SHAPE (vec_type, 0) <= TYPES_DIM (array_btype)) {
                /*  number of elements of acess-vector is equal or less than
                 *  dimension of array
                 *  (create a type with known dimension)
                 */
#ifndef KNOWN_DIM
                ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL);
#else
                ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype),
                                       KNOWN_DIM_OFFSET - TYPES_DIM (array_btype), NULL);
#endif /* KNOWN_DIM */
            } else {
                /* access-vector as to many elements */
                ret_type = MakeTypeNN (T_unknown, 0, NULL);
            }
        } else if (UNKNOWN_SHAPE == TYPES_DIM (array_btype)) {
            /* the shape of the array is unknown */
            ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL);
        } else if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype)) {
            /* only the dimension of the array is known */
            if (KNOWN_DIM_OFFSET - TYPES_SHAPE (vec_type, 0) >= TYPES_DIM (array_btype)) {
                /*  number of elements of acess-vector is equal or less than
                 *  dimension of array
                 *  (create a type with known dimension) */
#ifndef KNOWN_DIM
                ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL);
#else
                ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype),
                                       TYPES_DIM (array_btype), NULL);
#endif /* KNOWN_DIM */
            } else {
                /* access-vector as too many elements */
                ret_type = MakeTypeNN (T_unknown, 0, NULL);
            }
            ret_type = NULL;
            DBUG_ASSERT (0, "scalar arrays are not possible here!");
        } else {
            ret_type = NULL;
            DBUG_ASSERT (0, "Wrong TYPES_DIM");
            /* just filled that in to avoid warnings (jhs) */
        }
    } else if (UNKNOWN_SHAPE == TYPES_DIM (vec_type)) {
        /*  access-vector is not an constant vector.
         *  the shape of the vector is unknown */
        if (SCALAR < TYPES_DIM (array_btype)) {
            /*  the shape of the array is known, so the dimension of the result-type
             *  and the array are equal */
#ifndef KNOWN_DIM
            ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL);
#else
            ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype),
                                   KNOWN_DIM_OFFSET - TYPES_DIM (array_btype), NULL);
#endif /* KNOWN_DIM */
        } else if (UNKNOWN_SHAPE == TYPES_DIM (array_btype)) {
            /*  The shape of the array is unknown,
             *  The shape of the result-type will be unknown too. */
            ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL);
        } else if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype)) {
            /*  One only know the dimension of the array, so one can only infer
             *  the dimsion of the reslut-type */
#ifndef KNOWN_DIM
            ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL);
#else
            ret_type
              = MakeTypeNN (TYPES_BASETYPE (array_btype), TYPES_DIM (array_btype), NULL);
#endif /* KNOWN_DIM */
        } else {
            ret_type = NULL;
            DBUG_ASSERT (0, " wrong dimension of array_btype");
        }
    } else if ((KNOWN_DIM_OFFSET - 1) == TYPES_DIM (vec_type)) {
        /*  access-vector is not a constant vektor and only the dimension
         *  is known (the dimension is 1) */
        if (SCALAR < TYPES_DIM (array_btype)) {
            /*  the shape of the array is known, so the dimension of result-type
             *  and array are equal */
#ifndef KNOWN_DIM
            ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL);
#else
            ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype),
                                   KNOWN_DIM_OFFSET - TYPES_DIM (array_btype), NULL);
#endif /* KNOWN_DIM */
        } else if (UNKNOWN_SHAPE == TYPES_DIM (array_btype)) {
            /*
             * the shape of the array is unknown.
             * The shape of the resul-type will be unknown too
             */
            ret_type
              = MakeTypes (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL, NULL, NULL);
        } else if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype)) {
            /*
             *  One only knows the dimension of the array and the vector
             */
#ifndef KNOWN_DIM
            ret_type
              = MakeTypes (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
            ret_type = MakeTypes (TYPES_BASETYPE (array_btype), TYPES_DIM (array_btype),
                                  NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        } else {
            ret_type = NULL;
            DBUG_ASSERT (0, " wrong dimension of array_btype");
        }
    } else {
        /*
         * the dimension of vec_type is greater than 1.
         * This is not correct, so create T_unknown as result-type
         */
        ret_type = MakeTypeNN (T_unknown, 0, NULL);
    }
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
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */

types *
TakeV (node *vec, types *vec_type, types *array)
{
    types *ret_type, *array_btype;
    int *tmp2, ok = 1, i, dim2 = 0;
    node *tmp;

    DBUG_ENTER ("TakeV");

    DBUG_ASSERT (((NODE_TYPE (vec) == N_array) || (NODE_TYPE (vec) == N_id)),
                 "neither an N_id nor an N_array node");
    if (SAC_PRG == kind_of_file) {
        if (((NODE_TYPE (vec) != N_id) || (TYPES_BASETYPE (vec_type) != T_int)
             || (!(ID_ISCONST (vec))))
            && (NODE_TYPE (vec) != N_array))
            ERROR2 (3, ("%s, %d: 1.argument of function `take` "
                        " should be a constant vector",
                        filename, NODE_LINE (vec)));
    }

    GET_BASIC_TYPE (array_btype, array, NODE_LINE (vec));

    if (N_array == NODE_TYPE (vec)) { /*initializes ret_type */
        tmp = ARRAY_AELEMS (vec);

        if (SCALAR < TYPES_DIM (array_btype)) { /* initializes ret_type */
            /* array has known shape */

            /* check weather entries in 1) are ok */
            while ((NULL != tmp) && (1 == ok) && (dim2 < TYPES_DIM (array_btype))) {
                if ((abs (NUM_VAL (EXPRS_EXPR (tmp)))
                     > TYPES_SHAPE (array_btype, dim2)) /* SBS hack for ng take */) {
                    ok = 0;
                } else {
                    tmp = EXPRS_NEXT (tmp);
                    dim2++;
                }
            } /* while */

            if ((0 == ok) || ((NULL != tmp) && (dim2 >= array_btype->dim))) {
                GEN_TYPE_NODE (ret_type, T_unknown);
                FreeOneTypes (array_btype);
            } else {
                ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype),
                                       TYPES_DIM (array_btype), MakeShpseg (NULL));
                tmp = ARRAY_AELEMS (vec);
                /*  compute resulting shape */
                for (i = 0; i < TYPES_DIM (array_btype); i++) {
                    if (i < dim2) {
                        TYPES_SHAPE (ret_type, i) = abs (NUM_VAL (EXPRS_EXPR (tmp)));
                        tmp = EXPRS_NEXT (tmp);
                    } else {
                        TYPES_SHAPE (ret_type, i) = TYPES_SHAPE (array_btype, i);
                    }
                } /* for */
            }     /* if ... else ... */
        } else {  /* initializes ret_type */
            /*  Array has got an unknown shape */
            if (UNKNOWN_SHAPE == TYPES_DIM (array_btype)) {
                ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL);
            } else {
                if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype)) {
                    /*  one knows only the dimension of array_btype, but not the shape */
                    while (NULL != tmp) {
                        dim2 += 1;
                        tmp = EXPRS_EXPR (tmp);
                    } /* while */
                    if (KNOWN_DIM_OFFSET - dim2 >= TYPES_DIM (array_btype)) {
                        /*
                         *  the shape of 'array' is unknown, but the dimension is known
                         * and it is equal the number of elements of the constant vector.
                         *  We create a dimension known result-type, because we do not
                         * know if the components of the vector will match with the
                         * array's shape any time. (of course one could take the
                         * components of the vector as new values of the result-type, but
                         * one does not know if the access will be ok)
                         *
                         *   replace UNKNOWN_SHAPE with TYPES_DIM(array_btype)
                         */
#ifndef KNOWN_DIM
                        ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype),
                                               UNKNOWN_SHAPE, NULL);
#else
                        ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype),
                                               TYPES_DIM (array_btype), NULL);
#endif /* KNOWN_DIM */
                    } else {
                        DBUG_ASSERT (0, " wrong dimension of array_btype");
                        ret_type = NULL;
                    }
                } else {
                    /* I added it only to be sure ret_type is set,
                     * I know noyhing further about it. jhs */
                    DBUG_ASSERT (0, "unknown cause");
                    ret_type = NULL;
                }
            } /* if ... else ... */
        }     /* if ... else ... */
    } else {
        if ((NODE_TYPE (vec) == N_id) && (ID_ISCONST (vec))
            && (TYPES_BASETYPE (vec_type) == T_int)) {
            dim2 = ID_VECLEN (vec);
            tmp2 = (int *)ID_CONSTVEC (vec);

            if (SCALAR < TYPES_DIM (array_btype)) { /* initializes ret_type */
                /* array has got a known shape */

                /* check weather the entries in 1) are ok */
                for (i = 0; i < dim2; i++) {
                    if (((tmp2[i] >= 0)
                         || (tmp2[i] < 0)) /* dirty hack of SBS to allow for neg take's */
                        && (abs (tmp2[i]) <= TYPES_SHAPE (array_btype, i))) {
                        ok = 1;
                    } else {
                        ok = 0;
                    }
                }

                if (ok == 0) {
                    GEN_TYPE_NODE (ret_type, T_unknown);
                    FreeOneTypes (array_btype);
                } else {
                    ret_type = MakeTypeNN (TYPES_BASETYPE (array_btype),
                                           TYPES_DIM (array_btype), MakeShpseg (NULL));
                    /* compute resulting shape */
                    for (i = 0; i < TYPES_DIM (array_btype); i++) {
                        if (i < dim2)
                            TYPES_SHAPE (ret_type, i) = abs (tmp2[i]);
                        else
                            TYPES_SHAPE (ret_type, i) = TYPES_SHAPE (array_btype, i);
                    }
                }
            } else {
                /* array has got an unknown shape */
                if (UNKNOWN_SHAPE == TYPES_DIM (array_btype)) {
                    ret_type
                      = MakeTypeNN (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL);
                } else {
                    if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype)) {
                        /*
                         * one knows only the dimension of array_btype, but not the shape
                         */
                        if (KNOWN_DIM_OFFSET - dim2 >= TYPES_DIM (array_btype)) {
                            /*
                             * the shape of 'array' is unknown, but the dimension is known
                             * and it is equal the number of elements of the constant
                             * vector. We create a dimension known result-type, because we
                             * do not know if the components of the vector will match with
                             * the shape of the array any time. (of course one could take
                             * the components of the vector as new values of the
                             * result-type, but one does not know if the access will be
                             * ok)
                             *
                             * replace UNKNOWN_SHAPE with TYPES_DIM(array_btype)
                             */
#ifndef KNOWN_DIM
                            ret_type = MakeTypes (TYPES_BASETYPE (array_btype),
                                                  UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
                            ret_type
                              = MakeTypes (TYPES_BASETYPE (array_btype),
                                           TYPES_DIM (array_btype), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
                        } else {
                            DBUG_ASSERT (0, "ret_type is not initialized here! ");
                            ret_type = NULL; /* this eliminates warning - but may I do
                                                it???? ASSERT will show */
                        }
                    } else {
                        DBUG_ASSERT (0, " wrong dimension of array_btype");
                        ret_type = NULL;
                    } /* if ... else ... */
                }     /* if ... else ... */
            }         /* if ... else ... */
        } else {
            ret_type = TakeDropV (vec_type, array_btype);
        } /* if ... else ... */
    }
    /* and now free some memory */
    FreeOneTypes (array_btype);

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : DropV
 *  arguments     : 1) node of shape-vector
 *                  2) type of shape-vector
 *                  3) type of an array
 *
 *  description   : computes the resulttype of a 'drop' operation, whoes
 *                  first argument was a constant vector
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */

types *
DropV (node *vec, types *vec_type, types *array)
{
    types *ret_type, *array_btype;
    node *tmp;
    int i, *tmp2, dim2 = 0, ok = 1;

    DBUG_ENTER ("DropV");

    DBUG_ASSERT (((NODE_TYPE (vec) == N_array) || (NODE_TYPE (vec) == N_id)),
                 "neither an N_id nor an N_array node");
    GET_BASIC_TYPE (array_btype, array, NODE_LINE (vec));

    if (kind_of_file == SAC_PRG) {
        if (((NODE_TYPE (vec) != N_id) || (TYPES_BASETYPE (vec_type) != T_int)
             || (!(ID_ISCONST (vec))))
            && (NODE_TYPE (vec) != N_array))
            ERROR2 (3, ("%s, %d: 1.argument of function `drop` "
                        " should be a constant integer vector",
                        filename, NODE_LINE (vec)));
    }

    if (NODE_TYPE (vec) == N_array) {
        tmp = ARRAY_AELEMS (vec);

        if (SCALAR < TYPES_DIM (array_btype)) {
            /* array has got a known shape */

            /* check weather the entries in 1) are ok */
            while ((tmp != NULL) && (ok == 1) && (dim2 < TYPES_DIM (array_btype))) {
                if ((abs (NUM_VAL (EXPRS_EXPR (tmp))) <= TYPES_SHAPE (array_btype, dim2))
                    && ((NUM_VAL (EXPRS_EXPR (tmp)) >= 0)
                        || (NUM_VAL (EXPRS_EXPR (tmp))
                            < 0))) { /* dirty hack of SBS: allow for neg drops! */
                    dim2++;
                    tmp = EXPRS_NEXT (tmp);
                } else {
                    ok = 0;
                }
            }

            if ((ok == 0) || ((tmp != NULL) && (dim2 == TYPES_DIM (array_btype)))) {
                GEN_TYPE_NODE (ret_type, T_unknown);
            } else {
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array_btype), TYPES_DIM (array_btype),
                               MakeShpseg (NULL), NULL, NULL);
                tmp = ARRAY_AELEMS (vec);
                for (i = 0; i < TYPES_DIM (array_btype); i++) {
                    if (i < dim2) {
                        SHAPES_SELEMS (ret_type)
                        [i] = SHAPES_SELEMS (array_btype)[i]
                              - abs (NUM_VAL (EXPRS_EXPR (tmp)));
                        tmp = EXPRS_NEXT (tmp);
                    } else {
                        SHAPES_SELEMS (ret_type)[i] = SHAPES_SELEMS (array_btype)[i];
                    }
                }
            }
        } else {
            /*
             * array has got an unknown shape
             */
            if (UNKNOWN_SHAPE == TYPES_DIM (array_btype)) {
                ret_type = MakeTypes (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL,
                                      NULL, NULL);
            } else {
                if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype)) {
                    /*
                     * one knows only the dimension of array_btype, but not the shape
                     */
                    while (NULL != tmp) {
                        dim2 += 1;
                        tmp = EXPRS_EXPR (tmp);
                    }
                    if (KNOWN_DIM_OFFSET - dim2 >= TYPES_DIM (array_btype)) {
                        /*
                         * the shape of 'array' is unknown, but the dimension is known and
                         * it is equal the number of elements of the constant vector. We
                         * create a dimension known result-type, because we do not know if
                         * the components of the vector will match with the array's shape
                         * any time.
                         * (of course one could take the components of the vector as new
                         * values of the result-type, but one does not know if the
                         * access will be ok)
                         *
                         * replace UNKNOWN_SHAPE with TYPES_DIM(array_btype)
                         */
#ifndef KNOWN_DIM
                        ret_type = MakeTypes (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE,
                                              NULL, NULL, NULL);
#else
                        ret_type = MakeTypes (TYPES_BASETYPE (array_btype),
                                              TYPES_DIM (array_btype), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
                    } else {
                        /*  I just filled this in to avoid warning under Linux. (jhs) */
                        ABORT (NODE_LINE (vec), ("Unknown dimension"));
                        ret_type = NULL;
                    }
                } else {
                    DBUG_ASSERT (0, " wrong dimension of array_btype");
                    ret_type = NULL;
                }
            }
        }
    } else {
        if ((NODE_TYPE (vec) == N_id) && (ID_ISCONST (vec))
            && (TYPES_BASETYPE (vec_type) == T_int)) {
            dim2 = ID_VECLEN (vec);
            tmp2 = (int *)ID_CONSTVEC (vec);

            if (SCALAR < TYPES_DIM (array_btype)) {
                /* array has got a known shape */

                /* check weather the entries in 1) are ok */
                dim2 = ID_VECLEN (vec);
                for (i = 0; i < dim2; i++) {
                    if ((tmp2[i] >= 0) && (tmp2[i] <= TYPES_SHAPE (array_btype, i)))
                        ok = 1;
                    else
                        ok = 0;
                }

                if (ok == 0)
                    GEN_TYPE_NODE (ret_type, T_unknown);
                else {
                    ret_type
                      = MakeTypes (TYPES_BASETYPE (array_btype), TYPES_DIM (array_btype),
                                   MakeShpseg (NULL), NULL, NULL);
                    for (i = 0; i < TYPES_DIM (array_btype); i++) {
                        if (i < dim2)
                            SHAPES_SELEMS (ret_type)
                            [i] = SHAPES_SELEMS (array_btype)[i] - tmp2[i];
                        else
                            SHAPES_SELEMS (ret_type)[i] = SHAPES_SELEMS (array_btype)[i];
                    }
                }
            } else {
                /*
                 * array has got an unknown shape
                 */
                if (UNKNOWN_SHAPE == TYPES_DIM (array_btype))
                    ret_type = MakeTypes (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE,
                                          NULL, NULL, NULL);
                else {
                    if (KNOWN_DIM_OFFSET > TYPES_DIM (array_btype)) {
                        /*
                         * one knows only the dimension of array_btype, but not the shape
                         */
                        if (KNOWN_DIM_OFFSET - dim2 >= TYPES_DIM (array_btype)) {
                            /*
                             * the shape of 'array' is unknown, but the dimension is known
                             * and it is equal the number of elements of the constant
                             * vector. We create a dimension known result-type, because we
                             * do not know if the components of the vector will match with
                             * the shape of the array any time. (of course one could take
                             * the components of the vector as new values of the
                             * result-type, but one does not know if the access will be
                             * ok)
                             *
                             * replace UNKNOWN_SHAPE with TYPES_DIM(array_btype)
                             */
#ifndef KNOWN_DIM
                            ret_type = MakeTypes (TYPES_BASETYPE (array_btype),
                                                  UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
                            ret_type
                              = MakeTypes (TYPES_BASETYPE (array_btype),
                                           TYPES_DIM (array_btype), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
                        } else {
                            /* Just filled tis in to avoid warnings under Linux (jhs) */
                            ABORT (NODE_LINE (vec), ("Unknown Dimension"));
                            ret_type = NULL;
                        }
                    } else {
                        DBUG_ASSERT (0, " wrong dimension of array_btype");
                        ret_type = NULL;
                    }
                }
            }
        }

        else {
            ret_type = TakeDropV (vec_type, array_btype);
        }
    }

    FreeOneTypes (array_btype);

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : Sel
 *  arguments     : 1) type of index vector
 *                  2) type of an array
 *  description   : computes the resulttype of a 'sel' operation
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */

types *
Sel (types *vec, types *array)
{
    int dim, i, to_drop;
    types *ret_type, *array_btype;
    shpseg *new_shpseg;

    DBUG_ENTER ("Sel");

    GET_BASIC_TYPE (array_btype, array, -64); /* -64 is a dummy argument */

    if (1 == TYPES_DIM (vec)) {
        if (TYPES_SHAPE (vec, 0) <= TYPES_DIM (array_btype)) {
            ret_type = array_btype;
            to_drop = TYPES_SHAPE (vec, 0);
            dim = TYPES_DIM (array_btype) - to_drop;
            ret_type->dim = dim;
            if (dim > 0) {
                new_shpseg = MakeShpseg (NULL);
                for (i = 0; i < dim; i++) {
                    SHPSEG_SHAPE (new_shpseg, i) = TYPES_SHAPE (array_btype, to_drop + i);
                }
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array_btype), dim, new_shpseg, NULL, NULL);
            } else
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array_btype), SCALAR, NULL, NULL, NULL);
        } else if (UNKNOWN_SHAPE == TYPES_DIM (array_btype))
            /* The result-type is an array or an scalar
             */
            ret_type = MakeTypes (TYPES_BASETYPE (array_btype), ARRAY_OR_SCALAR, NULL,
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
                ret_type = MakeTypes (TYPES_BASETYPE (array_btype), UNKNOWN_SHAPE, NULL,
                                      NULL, NULL);
#else
                ret_type = MakeTypes (TYPES_BASETYPE (array_btype),
                                      TYPES_DIM (array_btype) + TYPES_SHAPE (vec, 0),
                                      NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            else if (TYPES_SHAPE (vec, 0) == -TYPES_DIM (array_btype) + KNOWN_DIM_OFFSET)
                /* dimension of array_btype is equal to number umber of elements
                 * of the access vector.
                 * The result-type is a scalar
                 */
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array_btype), SCALAR, NULL, NULL, NULL);
            else
                ret_type = MakeTypes (T_unknown, SCALAR, NULL, NULL, NULL);
        } else {
            GEN_TYPE_NODE (ret_type, T_unknown);
        }
    } else if ((KNOWN_DIM_OFFSET - 1) == TYPES_DIM (vec)) {
        /* the dimension of vec is 1, but the shape is unknown
         * The result-type is an array or an scalar
         */
        ret_type
          = MakeTypes (TYPES_BASETYPE (array_btype), ARRAY_OR_SCALAR, NULL, NULL, NULL);

    } else if (UNKNOWN_SHAPE == TYPES_DIM (vec)) {
        /* The dimension iand the shape of vec are unknown.
         * The result-type is an array or an scalar.
         */
        ret_type
          = MakeTypes (TYPES_BASETYPE (array_btype), ARRAY_OR_SCALAR, NULL, NULL, NULL);
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
 *
 *  description   : computes the resulttype of a 'take'or 'drop' operation,
 *                  whoes first argument is a constant scalar.
 *                  distinction between 'take' and 'drop' is made by 3)
 *                  3)==0 => take
 *                  3)==1 => drop
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

        if (1 <= TYPES_DIM (array)) {
            if (((1 == tag) ? (s_node->info.cint < array->shpseg->shp[0])
                            : (s_node->info.cint <= array->shpseg->shp[0]))
                && (0 <= s_node->info.cint)) {
                GEN_TYPE_NODE (ret_type, array->simpletype);
                ret_type->dim = 1;
                ret_type->shpseg = MakeShpseg (NULL);
                if (1 == tag) {
                    /* drop */
                    ret_type->shpseg->shp[0] = array->shpseg->shp[0] - s_node->info.cint;
                } else {
                    /* take */
                    ret_type->shpseg->shp[0] = s_node->info.cint;
                }

                for (i = 1; i < array->dim; i++)
                    ret_type->shpseg->shp[i] = array->shpseg->shp[i];
            } else {
                GEN_TYPE_NODE (ret_type, T_unknown);
            }
        } else {
            /* I just filled this in to suppress warnings while compiling for
             * Linux. (jhs)
             */
            ABORT (NODE_LINE (s_node), ("Wrong dimension"));
            ret_type = NULL;
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
                    ret_type->shpseg = MakeShpseg (NULL);
                    if (1 == tag) {
                        /* drop */
                        ret_type->shpseg->shp[0]
                          = array->shpseg->shp[0] - s_node->info.cint;
                    } else {
                        /* take */
                        ret_type->shpseg->shp[0] = s_node->info.cint;
                    }

                    for (i = 1; i < array->dim; i++)
                        ret_type->shpseg->shp[i] = array->shpseg->shp[i];
                } else {
                    GEN_TYPE_NODE (ret_type, T_unknown);
                }
            } else if (KNOWN_DIM_OFFSET > TYPES_DIM (array)) {
                ret_type = MakeTypes (TYPES_BASETYPE (array), TYPES_DIM (array), NULL,
                                      NULL, NULL);
            } else if (UNKNOWN_SHAPE == TYPES_DIM (array)) {
                ret_type = MakeTypes (TYPES_BASETYPE (array), TYPES_DIM (array), NULL,
                                      NULL, NULL);
            } else {
                DBUG_ASSERT (0, "wrong dimension of second argument (array)");
                ret_type = NULL;
            }
        } else {
            /* We know (N_num != NODE_TYPE(s_mode)) ...
             *
             * We don`t know the value of the first argument of 'take' or 'drop'.
             * We can only infer the dimension, but no shape.
             */
            if (SCALAR < TYPES_DIM (array)) {
#ifndef KNOWN_DIM
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array),
                               KNOWN_DIM_OFFSET - TYPES_DIM (array), NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            } else if (KNOWN_DIM_OFFSET > TYPES_DIM (array)) {
                ret_type = MakeTypes (TYPES_BASETYPE (array), TYPES_DIM (array), NULL,
                                      NULL, NULL);
            } else if (UNKNOWN_SHAPE == TYPES_DIM (array)) {
                ret_type = MakeTypes (TYPES_BASETYPE (array), TYPES_DIM (array), NULL,
                                      NULL, NULL);
            } else {
                DBUG_ASSERT (0, "wrong dimension of second argument (array)");
                ret_type = NULL;
            }
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
            ret_type->shpseg=MakeShpseg(NULL);
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
        if (N_num == s_node->nodetype) {
            if ((0 <= s_node->info.cint) && (s_node->info.cint < array->dim)) {
                ret_type = DupTypes (array);
            } else {
                ERROR (s_node->lineno, ("1.argument of function 'rotate` is constant %d "
                                        "but rotated array has dimension %d",
                                        s_node->info.cint, array->dim));
                GEN_TYPE_NODE (ret_type, T_unknown);
            }
        } else {
            ret_type = DupTypes (array);
        }
    } else {
        /* for modules only */
        if (N_num == NODE_TYPE (s_node)) {
            if (SCALAR < TYPES_DIM (array)) {
                if ((0 <= NUM_VAL (s_node)) && (NUM_VAL (s_node) < array->dim))
                    ret_type = DupTypes (array);
                else
                    GEN_TYPE_NODE (ret_type, T_unknown);
            } else if (KNOWN_DIM_OFFSET > TYPES_DIM (array)) {
                if ((0 <= NUM_VAL (s_node))
                    && (NUM_VAL (s_node) < POS_DIM (TYPES_DIM (array))))
                    ret_type = DupTypes (array);
                else
                    GEN_TYPE_NODE (ret_type, T_unknown);
            } else
                ret_type = DupTypes (array);
        } else
            /* the type of 'array' remains as it is */
            ret_type = DupTypes (array);
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
                    ret_type = MakeTypes (TYPES_BASETYPE (array1), dim1,
                                          MakeShpseg (NULL), NULL, NULL);
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
                        ret_type = MakeTypes (TYPES_BASETYPE (array1), dim1,
                                              MakeShpseg (NULL), NULL, NULL);
                        for (i = 0; i < dim1; i++)
                            TYPES_SHAPE (ret_type, i) = TYPES_SHAPE (array1, i);
                        TYPES_SHAPE (ret_type, axis) += TYPES_SHAPE (array2, axis);
                    } else
                        GEN_TYPE_NODE (ret_type, T_unknown);
                } else
                    ret_type = MakeTypes1 (T_unknown);
            } else
#ifndef KNOWN_DIM
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array1), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
                ret_type = MakeTypes (TYPES_BASETYPE (array1), KNOWN_DIM_OFFSET - dim1,
                                      NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        } else
            ret_type = MakeTypes1 (T_unknown);

    } else if ((SCALAR < dim1) && (KNOWN_DIM_OFFSET > dim2)) {
        if (dim2 == (KNOWN_DIM_OFFSET - dim1))
            ret_type = MakeTypes (TYPES_BASETYPE (array1), dim2, NULL, NULL, NULL);
        else
            ret_type = MakeTypes1 (T_unknown);
    } else if ((SCALAR < dim1) && (UNKNOWN_SHAPE == dim2))
#ifndef KNOWN_DIM
        ret_type = MakeTypes (TYPES_BASETYPE (array1), KNOWN_DIM_OFFSET - dim1, NULL,
                              NULL, NULL);
#else
        ret_type = MakeTypes (TYPES_BASETYPE (array1), UNKNOWN_SHAPE, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
    else if ((SCALAR < dim2) && (KNOWN_DIM_OFFSET > dim1)) {
        if ((dim1 == (KNOWN_DIM_OFFSET - dim2)) && IS_IN_RANGE (s_node, dim1))
            ret_type = MakeTypes (TYPES_BASETYPE (array1), dim1, NULL, NULL, NULL);
        else
            ret_type = MakeTypes1 (T_unknown);
    } else if ((SCALAR < dim2) && (UNKNOWN_SHAPE == dim1)) {
        if (IS_IN_RANGE (s_node, dim2))
            ret_type = MakeTypes (TYPES_BASETYPE (array1), dim1, NULL, NULL, NULL);
        else
            ret_type = MakeTypes1 (T_unknown);
    } else if ((UNKNOWN_SHAPE == dim1) && (UNKNOWN_SHAPE == dim2))
        ret_type = MakeTypes (TYPES_BASETYPE (array1), dim1, NULL, NULL, NULL);
    else if ((UNKNOWN_SHAPE == dim1) && (KNOWN_DIM_OFFSET > dim2)) {
        if (IS_IN_RANGE (s_node, POS_DIM (dim2)))
            ret_type = MakeTypes (TYPES_BASETYPE (array1), dim2, NULL, NULL, NULL);
        else
            ret_type = MakeTypes1 (T_unknown);
    } else if ((UNKNOWN_SHAPE == dim2) && (KNOWN_DIM_OFFSET > dim1)) {
        if (IS_IN_RANGE (s_node, POS_DIM (dim1)))
            ret_type = MakeTypes (TYPES_BASETYPE (array1), dim1, NULL, NULL, NULL);
        else
            ret_type = MakeTypes1 (T_unknown);
    } else if ((KNOWN_DIM_OFFSET > dim1) && (KNOWN_DIM_OFFSET > dim2)) {
        if ((dim1 == dim2) && (IS_IN_RANGE (s_node, POS_DIM (dim1))))
#ifndef KNOWN_DIM
            ret_type
              = MakeTypes (TYPES_BASETYPE (array1), UNKNOWN_SHAPE, NULL, NULL, NULL);
#else
            ret_type = MakeTypes (TYPES_BASETYPE (array1), dim1, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
        else
            ret_type = MakeTypes1 (T_unknown);
    } else {
        DBUG_ASSERT (0, "wrong dimensions");
        ret_type = NULL;
    }

    DBUG_ASSERT (NULL != ret_type, "no ret_type");

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : ConvertType
 *  arguments     : 1) type of array
 *                  2) new simpletype
 *  description   : returns the type 2) with shape of 1)
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
 *  description   : returns the type 1)
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

    DBUG_PRINT ("PRF_TYPE", ("b_array: %s", Type2String (b_array, 0, TRUE)));
    DBUG_PRINT ("PRF_TYPE", ("b_vec: %s", Type2String (b_vec, 0, TRUE)));
    DBUG_PRINT ("PRF_TYPE", ("b_value: %s", Type2String (b_value, 0, TRUE)));

    if ((SAC_PRG == kind_of_file) && (dynamic_shapes == 0)) {
        if (b_vec->dim != 1) {
            ERROR2 (3, ("%s, %d: 2.argument of function 'modarray' has incompatible "
                        "type ( int[x] != %s)",
                        filename, line, Type2String (vec, 0, TRUE)));
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
                    filename, line, Type2String (array, 0, TRUE),
                    Type2String (vec, 0, TRUE), Type2String (value, 0, TRUE)));
    } else {
        b_vec = Free (b_vec);
        b_value = Free (b_value);
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
 *
 *  remarks       : is part of macro TT2 and is used in typecheck.c
 *
 */

types *
Genarray_S (node *v_node, types *vec, types *scalar)
{
    types *ret_type;
    int *tmp2, i, dim = 0;
    shpseg *shpseg_p;
    node *tmp;

    DBUG_ENTER ("Genarray_S");

    if (SAC_PRG == kind_of_file) {

        if (NODE_TYPE (v_node) == N_array) {
            int ok = 1;
            /*
             * create shpseg of resulting type
             */
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
            } while (tmp != NULL); /* end of loop */

            /*
             * create resulting types
             */
            if (ok == 1)
                ret_type
                  = MakeTypes (TYPES_BASETYPE (scalar), dim, shpseg_p,
                               StringCopy (TYPES_NAME (scalar)), TYPES_MOD (scalar));
            else
                ret_type = MakeTypes1 (T_unknown);

        } else if ((NODE_TYPE (v_node) == N_id) && ID_ISCONST (v_node)) {
            /*
             * create shpseg of resulting type
             */
            shpseg_p = MakeShpseg (NULL);
            tmp2 = (int *)ID_CONSTVEC (v_node);
            dim = ID_VECLEN (v_node);
            for (i = 0; i < dim; i++)
                SHPSEG_SHAPE (shpseg_p, i) = tmp2[i];

            /*
             * create resulting types
             */
            ret_type = MakeTypes (TYPES_BASETYPE (scalar), dim, shpseg_p,
                                  StringCopy (TYPES_NAME (scalar)), TYPES_MOD (scalar));
        } else {
            ret_type = MakeTypes1 (T_unknown);
            /* DBUG_ASSERT(0," wrong first argument of primitive function genarray"); */
        }
    }

    else {
        /*
         * checking in a module
         */
        if (NODE_TYPE (v_node) == N_array) {
            /*
             * create shpseg of resulting type
             */
            shpseg_p = MakeShpseg (NULL);
            tmp = ARRAY_AELEMS (v_node);
            do {
                SHPSEG_SHAPE (shpseg_p, dim) = NUM_VAL (EXPRS_EXPR (tmp));
                tmp = EXPRS_NEXT (tmp);
                dim++;
            } while (NULL != tmp);

            /*
             * create resulting types
             */
            ret_type = MakeTypes (TYPES_BASETYPE (scalar), dim, shpseg_p,
                                  StringCopy (TYPES_NAME (scalar)), TYPES_MOD (scalar));
        } else {
            if ((NODE_TYPE (v_node) == N_id) && ID_ISCONST (v_node)) {
                /*
                 * create shpseg of resulting type
                 */
                shpseg_p = MakeShpseg (NULL);
                tmp2 = (int *)ID_CONSTVEC (v_node);
                dim = ID_VECLEN (v_node);
                for (i = 0; i < dim; i++) {
                    SHPSEG_SHAPE (shpseg_p, i) = tmp2[i];
                }

                /*
                 * create resulting types
                 */
                ret_type
                  = MakeTypes (TYPES_BASETYPE (scalar), dim, shpseg_p,
                               StringCopy (TYPES_NAME (scalar)), TYPES_MOD (scalar));
            } else {
                /*
                 * there isn't a constant vector
                 */
                if ((1 == TYPES_DIM (vec)) || ((KNOWN_DIM_OFFSET - 1) == TYPES_DIM (vec))
                    || (UNKNOWN_SHAPE == TYPES_DIM (vec)))
                    ret_type
                      = MakeTypes (TYPES_BASETYPE (scalar), UNKNOWN_SHAPE, NULL,
                                   StringCopy (TYPES_NAME (scalar)), TYPES_MOD (scalar));
                else
                    ret_type = MakeTypes1 (T_unknown);
            }
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
    if (kind_of_file == SAC_PRG) {
        if ((NODE_TYPE (v_node) == N_array)
            || ((NODE_TYPE (v_node) == N_id) && ID_ISCONST (v_node))) {
            ret_type = Genarray_S (v_node, vec, array);
            dim = TYPES_DIM (ret_type);
            for (i = 0; i < TYPES_DIM (array); i++) {
                DBUG_ASSERT (dim < SHP_SEG_SIZE, " shape is out of range");
                TYPES_SHAPE (ret_type, dim) = TYPES_SHAPE (array, i);
                dim++;
            }
            TYPES_DIM (ret_type) = dim;
        } else {
            ret_type = MakeTypes1 (T_unknown);
        }
    } else if (kind_of_file == SAC_MOD) {
        /*
         *  we are checking a function in a module
         */
        if ((NODE_TYPE (v_node) == N_array)
            || ((NODE_TYPE (v_node) == N_id) && ID_ISCONST (v_node))) {
            if (TYPES_DIM (array) == UNKNOWN_SHAPE) {
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
            } else if (TYPES_DIM (array) < KNOWN_DIM_OFFSET) {
#ifdef KNOWN_DIM
                ret_type = MakeTypes (TYPES_BASETYPE (array),
                                      (TYPES_DIM (array) - TYPES_SHAPE (vec, 0)), NULL,
                                      NULL, NULL);
#else
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            } else if (TYPES_DIM (array) > SCALAR) {
                ret_type = Genarray_S (v_node, vec, array);
                dim = TYPES_DIM (ret_type);
                for (i = 0; i < TYPES_DIM (array); i++) {
                    DBUG_ASSERT (dim < SHP_SEG_SIZE, " shape is out of range");
                    TYPES_SHAPE (ret_type, dim) = TYPES_SHAPE (array, i);
                    dim++;
                }
                TYPES_DIM (ret_type) = dim;
            } else {
                /*  this case was left open here. Just filled in the following
                 *  to suppress warnings while compiling for Linux. (jhs)
                 */
                ABORT (NODE_LINE (v_node), ("Wrong TYPES_DIM"));
                ret_type = NULL;
            }
        } else if (TYPES_DIM (vec) == 1) {
            if (TYPES_DIM (array) > SCALAR) {
#ifdef KNOWN_DIM
                ret_type = MakeTypes (TYPES_BASETYPE (array),
                                      (KNOWN_DIM_OFFSET
                                       - (TYPES_SHAPE (vec, 0) + TYPES_DIM (array))),
                                      NULL, NULL, NULL);
#else
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            } else if (TYPES_DIM (array) > KNOWN_DIM_OFFSET) {
#ifdef KNOWN_DIM
                ret_type = MakeTypes (TYPES_BASETYPE (array),
                                      (TYPES_DIM (array) - TYPES_SHAPE (vec, 0)), NULL,
                                      NULL, NULL);
#else
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
#endif /* KNOWN_DIM */
            } else if (TYPES_DIM (array) == UNKNOWN_SHAPE) {
                ret_type
                  = MakeTypes (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
            } else {
                ret_type = MakeTypes1 (T_unknown);
            }
        } else if ((TYPES_DIM (vec) == UNKNOWN_SHAPE)
                   || ((KNOWN_DIM_OFFSET - 1) == TYPES_DIM (vec))) {
            ret_type
              = MakeTypes (TYPES_BASETYPE (array), UNKNOWN_SHAPE, NULL, NULL, NULL);
        } else {
            ret_type = MakeTypes1 (T_unknown);
        }
    } else {
        /*  this case was left open here. Just filled in the following
         *  to suppress warnings while compiling for Linux. (jhs)
         */
        ABORT (NODE_LINE (v_node), ("Unknown modul type"));
        ret_type = NULL;
    }

    DBUG_RETURN (ret_type);
}
