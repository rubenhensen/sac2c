
/*
 *
 * $Log$
 * Revision 3.60  2004/07/31 13:44:44  sah
 * removed function MakeNCodeExprs. Instead, MakeNCode now expects
 * an exprs node as its second argument!
 *
 * Revision 3.59  2004/07/30 17:25:06  sbs
 * switch between old and new tc lifted into main
 *
 * Revision 3.58  2004/07/27 11:50:20  sbs
 * PROFILE_IN_TC eliminated!
 *
 * Revision 3.57  2004/07/14 23:25:28  sah
 *  removed all old ssa optimizations and the use_ssaform flag
 *
 * Revision 3.56  2004/07/13 16:45:11  sah
 * added WO_unknown WithOpType
 *
 * Revision 3.55  2004/03/10 00:18:01  dkrHH
 * comment about float.h corrected
 *
 * Revision 3.54  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.53  2004/02/20 08:14:00  mwe
 * now functions with and without body are separated
 * changed tree traversal (added traverse of MODUL_FUNDECS)
 *
 * Revision 3.52  2003/12/23 10:58:51  khf
 * DebugAssert in BuildGenarrayWithLoop inserted.
 *
 * Revision 3.51  2003/06/13 09:25:25  ktr
 * shape_array is now checked for NULL in Types2Array
 *
 * Revision 3.50  2003/06/11 21:45:46  ktr
 * replaced calls of MakeArray with MakeFlatArray
 *
 * Revision 3.49  2003/04/14 15:06:53  sbs
 * all int prf's casted into prf's as these may be implemented as unsigned.
 *
 * Revision 3.48  2002/10/18 14:27:51  sbs
 * ID_ATTRIB accesses replaced with FLAG inspections.
 *
 * Revision 3.47  2002/10/09 22:18:17  dkr
 * macro moved from tree_compound.h to typecheck.c
 *
 * Revision 3.46  2002/09/11 23:06:41  dkr
 * printing of PRFs modified, prf_node_info.mac modified.
 *
 * Revision 3.45  2002/09/09 19:33:25  dkr
 * prf_name_str renamed into prf_name_string
 *
 * Revision 3.44  2002/09/09 17:42:51  dkr
 * F_{add,sub,mul,div}_SxS added
 *
 * Revision 3.43  2002/08/07 09:50:31  sbs
 * now, there is a life behind the new type checker....8-)
 *
 * Revision 3.42  2002/08/03 01:16:36  dkr
 * *very* dirty hack for new backend
 *
 * Revision 3.41  2002/07/03 13:02:47  sah
 * changed assert due to incorrect module compiling.
 *
 * Revision 3.40  2002/07/02 15:27:29  sah
 * added support for N_dot nodes in WL generators
 *
 * Revision 3.39  2002/06/20 15:23:41  dkr
 * signature of MakeNWithOp modified
 *
 * Revision 3.38  2002/03/05 15:53:33  sbs
 * in sbs mode print is called on the ast before exiting
 *
 * Revision 3.37  2002/02/22 11:44:20  dkr
 * works with simplified TYPES structure now :-))
 *
 * Revision 3.36  2002/02/21 18:18:18  dkr
 * access macros used for TYPES structure (finished :-)
 *
 * Revision 3.35  2002/02/21 17:46:09  dkr
 * access macros used for TYPES structure (not finished yet)
 *
 * Revision 3.34  2002/02/20 15:06:10  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.33  2001/11/19 20:34:58  dkr
 * TI() renamed into TypeInference() in order to avoid linker warning
 *
 * Revision 3.32  2001/07/18 12:57:45  cg
 * Applications of old tree construction function
 * AppendNodeChain eliminated.
 *
 * Revision 3.31  2001/07/16 08:23:11  cg
 * Old tree construction function MakeNode eliminated.
 *
 * Revision 3.30  2001/07/13 13:23:41  cg
 * Some useless DBUG_PRINTs eliminated.
 *
 * ... [eliminated] ...
 *
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <limits.h> /* for INT_MIN, INT_MAX */
#include <float.h>  /* for FLT_MAX, DBL_MAX */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "print.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "typecheck.h"
#include "Error.h"
#include "convert.h"
#include "import.h"
#include "prim_fun.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"
#include "typecheck_WL.h"
#include "gen_pseudo_fun.h"
#include "import_specialization.h"

#include "new_typecheck.h"

/*****************************************************************************/

/**
 **  ugly macros from access_macros.h included here ...
 **/

/* macros for access to elements of struct info.types */
#define DIM info.types->dim
#define SHP info.types->shpseg->shp
#define NAME info.types->name

/* macros used for N_ap nodes to get the function's name */
#define FUN_NAME info.fun_name.id
#define FUN_MOD_NAME info.fun_name.id_mod

/*****************************************************************************/

/*
 * compares two module names (name maybe NULL)
 * result: 1 - equal, 0 - not equal
 */
#define CMP_MOD(a, b) ((NULL == a) ? (NULL == b) : ((NULL == b) ? 0 : (!strcmp (a, b))))

/*****************************************************************************/

#define LOCAL_STACK_SIZE 1000 /* used for scope stack */
#define MAX_ARG_TYPE_LENGTH                                                              \
    1024 /* used in FindFun as size of char[]                                            \
          * to convert the argument-types                                                \
          * of a function call to a sting                                                \
          */

#define PUSH_VAR(name, new_node)                                                         \
    {                                                                                    \
        if (tos < stack_limit) {                                                         \
            DBUG_PRINT ("TYPE", (" push : %s" F_PTR " shpseg:" F_PTR, name, tos,         \
                                 VARDEC_OR_ARG_SHPSEG (new_node)->shp));                 \
            tos->id = name;                                                              \
            tos->node = new_node;                                                        \
            tos++;                                                                       \
        } else                                                                           \
            SYSABORT (("Overflow of local stack"))                                       \
    }

#define GEN_TYPE_NODE(node, type) node = MakeTypes1 (type)

/*
 *  The following defines are *only* used in functions TI_prf, TI_Nfoldprf.
 *  USAGES CANNOT BE FOUND IN THIS FILE (typecheck.c)!!!
 *  They are used to select the function that computes the result type
 *  of the belonging primitive function.
 *  The informtion is stored in prim_fun_tt.mac
 */
#define TT1(t, t_c, t1, res)                                                             \
    case (t):                                                                            \
        ret_type = res;                                                                  \
        break;
#define TT2(t, t_c, t1, t2, res)                                                         \
    case (t):                                                                            \
        ret_type = res;                                                                  \
        break;
#define TT3(t, t_c, t1, t2, t3, res)                                                     \
    case (t):                                                                            \
        ret_type = res;                                                                  \
        break;

/* the following defines are used as tag for the status of typechecking
 * of userdefined functions. The tag is put on the scope stack.
 */
#define NOT_CHECKED 0 /* function is not typecheched yet */
#define IS_CHECKED 1  /* function is completely typechecked */
#define CHECKING 2    /* trying to typecheck function */
#define TMP_CHECKED 3 /* function is typechecked, but not completly */
#define FAST_CHECK                                                                       \
    4 /* trying to infere type of function in only one                                   \
         branch */
#define PLEASE_CHECK                                                                     \
    5 /* try to typecheck function (is set only for                                      \
         typechecking of modules */

#define CHECK_NAME(n)                                                                    \
    ((0 == n)                                                                            \
       ? "NOT_CHECKED"                                                                   \
       : ((1 == n)                                                                       \
            ? "IS_CHECKED"                                                               \
            : ((2 == n) ? "CHECKING"                                                     \
                        : ((3 == n) ? "TMP_CHECKED"                                      \
                                    : ((4 == n) ? "FAST_CHECK"                           \
                                                : ((5 == n) ? "PLEASE CHECK" : ""))))))

/* following define is used with arg_info->node[0]->info.cint  ...*/
#define BODY 0 /* checking plane part of function, no branch */

/* macro used for type_tab access */
#define T_TYPE(i) TYPEDEF_TYPE ((tab + i)->node)
#define T_NAME(i) TYPEDEF_NAME ((tab + i)->node)
#define T_MOD(i) TYPEDEF_MOD ((tab + i)->node)

#define CMP_TYPEDEF_ID(a, b)                                                             \
    ((NULL == TYPEDEF_MOD (a))                                                           \
       ? ((!strcmp (TYPEDEF_NAME (a), TYPEDEF_NAME (b))) && (NULL == TYPEDEF_MOD (b)))   \
       : ((NULL == TYPEDEF_MOD (b))                                                      \
            ? (!strcmp (TYPEDEF_NAME (a), TYPEDEF_NAME (b)))                             \
            : ((!strcmp (TYPEDEF_NAME (a), TYPEDEF_NAME (b)))                            \
               && (!strcmp (TYPEDEF_MOD (a), TYPEDEF_MOD (b))))))

#define CMP_TYPE_NAME(a, b)                                                              \
    ((NULL == a->name_mod)                                                               \
       ? ((!strcmp (a->name, b->name)) && (NULL == b->name_mod))                         \
       : ((NULL == b->name_mod)                                                          \
            ? (!strcmp (a->name, b->name))                                               \
            : ((!strcmp (a->name, b->name)) && (!strcmp (a->name_mod, b->name_mod)))))

#define CMP_TYPE_MOD(a, b) ((NULL == a) ? 1 : ((NULL == b) ? 0 : (!strcmp (a, b))))

#define FREE_TYPES(a) FreeOneTypes (a)

#define CHECK_FUNCTION_TYPENAMECLASH(arg)                                                \
    {                                                                                    \
        if (NULL != LookupType (FUNDEF_NAME (arg), NULL, NODE_LINE (arg))) {             \
            ERROR (NODE_LINE (arg), ("Function '%s` has same name as user-defined type", \
                                     FUNDEF_NAME (arg)));                                \
        }                                                                                \
        if ((NULL != FUNDEF_PRAGMA (arg)) && (NULL != FUNDEF_LINKNAME (arg)))            \
            if (NULL != LookupType (FUNDEF_LINKNAME (arg), NULL, NODE_LINE (arg))) {     \
                ERROR (NODE_LINE (arg), ("Link name '%s` of function '%s` has same name" \
                                         " as user-defined type",                        \
                                         FUNDEF_LINKNAME (arg), FUNDEF_NAME (arg)));     \
            }                                                                            \
    }

#define CHECK_TYPENAMECLASH(arg)                                                         \
    if (NULL != LookupType (VARDEC_OR_ARG_NAME (arg), NULL, NODE_LINE (arg))) {          \
        ERROR (NODE_LINE (arg), ("Identifier '%s` has same name as user-defined type",   \
                                 VARDEC_OR_ARG_NAME (arg)));                             \
    }

/* some macros for fun_tab access */
#define END_OF_FUN_TAB(elem) (NULL == elem)

#define NEXT_FUN_TAB_ELEM(elem) elem->next

#define NEW_FUN_TAB_ELEM (fun_tab_elem *)Malloc (sizeof (fun_tab_elem))

#define OLD_INSERT_FUN(fun_p, name, mod_name, new_node, status, overload)                \
    {                                                                                    \
        fun_tab_elem *tmp;                                                               \
        tmp = NEW_FUN_TAB_ELEM;                                                          \
        NEXT_FUN_TAB_ELEM (tmp) = NEXT_FUN_TAB_ELEM (fun_p);                             \
        tmp->id = name;                                                                  \
        tmp->id_mod = mod_name;                                                          \
        tmp->tag = status;                                                               \
        tmp->n_dub = overload;                                                           \
        tmp->node = new_node;                                                            \
        NEXT_FUN_TAB_ELEM (fun_p) = tmp;                                                 \
        DBUG_PRINT ("TYPE", (" insert :" F_PTR " %s:%s", tmp,                            \
                             (NULL == tmp->id_mod) ? "" : tmp->id_mod, tmp->id));        \
    }

#define INSERT_FUN(fun_p, new_node, overload)                                            \
    if (FUNDEF_BODY (new_node) == NULL) {                                                \
        OLD_INSERT_FUN (fun_p, FUNDEF_NAME (new_node),                                   \
                        (!(FUNDEF_MOD (new_node)) ? (FUNDEF_LINKMOD (new_node))          \
                                                  : FUNDEF_MOD (new_node)),              \
                        new_node, NOT_CHECKED, overload);                                \
    } else {                                                                             \
        if (FUNDEF_STATUS (new_node) == ST_objinitfun) {                                 \
            OLD_INSERT_FUN (fun_p, FUNDEF_NAME (new_node), FUNDEF_MOD (new_node),        \
                            new_node, PLEASE_CHECK, overload);                           \
        } else {                                                                         \
            OLD_INSERT_FUN (fun_p, FUNDEF_NAME (new_node), FUNDEF_MOD (new_node),        \
                            new_node, NOT_CHECKED, overload);                            \
        }                                                                                \
    }

/*
 *  The macro INSERT_FUN simplifies applications of the macro OLD_INSERT_FUN.
 *  Functions that have status ST_objinitfun are tagged PLEASE_CHECK to
 *  ensure that they are actually typechecked. After that their status is
 *  changed to ST_regular.
 */

#define INIT_FUN_TAB_ELEM(elem)                                                          \
    elem->id = "1";                                                                      \
    elem->id_mod = "1";                                                                  \
    elem->next = NULL;                                                                   \
    elem->node = NULL

#define IS_DUPLICATED(fun_p) (-2 == fun_p->n_dub)

/* used in function ComputeNeutralElem */
#define MAKE_NEUTRAL_ELEM(Node, Simpletype, neutral)                                     \
    switch (Simpletype) {                                                                \
    case T_int:                                                                          \
        Node = MakeNum (neutral);                                                        \
        break;                                                                           \
    case T_float:                                                                        \
        Node = MakeFloat (neutral);                                                      \
        break;                                                                           \
    case T_bool:                                                                         \
        Node = MakeBool (neutral);                                                       \
        break;                                                                           \
    case T_double:                                                                       \
        Node = MakeDouble (neutral);                                                     \
        break;                                                                           \
    default:                                                                             \
        DBUG_ASSERT (0, "wrong value of simpletype");                                    \
        break;                                                                           \
    }

/* following string is used for debuging (cmp_types)
 */
#define CMP_T(n, s) s

char *cmp_types_string[] = {
#include "cmp_type.mac"
};
#undef CMP_T

/* variables to export */
file_type kind_of_file;
/* will be set in Typecheck and will be later used in prim_fun.c
 * to distinguish between compilation of a SAC-program
 * or a SAC-module implenetation
 */
char *module_name = NULL; /* name of module to typecheck;
                           * is set in function Typecheck
                           */

/* some local typedefs */
typedef struct STACK_ELEM {
    node *node;
    char *id;
} stack_elem;

typedef struct TYPE_TAB_ELEM {
    node *node;   /* N_typedef node                    */
    int id_count; /* number of typedefs with same name */
} type_tab_elem;

typedef struct FUN_TAB_ELEM {
    node *node;                /* pointer to declaration (N_fundef) */
    char *id;                  /* name of function */
    char *id_mod;              /* modul name of function */
    int tag;                   /* typecheck status */
    int n_dub;                 /* number of remaining dublications */
    struct FUN_TAB_ELEM *next; /* next fun_tab_elem node */
} fun_tab_elem;

static node *pseudo_fold_fundefs;

/*
 *  forward declarations
 */
static types *TI_prf (node *arg_node, node *arg_info);
static types *TI_ap (node *arg_node, node *arg_info);
static types *TI_cast (node *arg_node, node *arg_info);

static types *TI_Nwith (node *, node *);
static types *TI_Npart (node *, types *, node *, node *);
static types *TI_Ngenarray (node *, node *, node **);
static void ConsistencyCheckModarray (int lineno, types *array_type,
                                      types *generator_type, types *body_type);
static types *TI_Nfoldprf (node *arg_node, types *block_type, types *neutral,
                           node *arg_info);
static types *TI_Nfoldfun (node *arg_node, types *body_type, types *neutral,
                           node *arg_info);

static fun_tab_elem *DoConstantPropagation (fun_tab_elem *fun_p, nodelist *propas);
static fun_tab_elem *TryConstantPropagation (fun_tab_elem *fun_p, node *args);

/* some local variables */
static type_tab_elem *type_table = NULL; /* table for typedefs */
static int type_tab_size = 0;            /* size of type table */

static stack_elem *stack = NULL, /* bottom of stack */
  *tos = NULL,                   /* top of stack */
    *act_frame = NULL,           /* bottom of variables
                                    known in current scope */
      *stack_limit = NULL;       /* top limit of stack */

static fun_tab_elem *fun_table; /* (bottom of) table of known functions  */

static node *object_table; /* entry point to global object definitions  */
                           /* is set in function typecheck              */

static int imported_fun; /* is used to check whether to look behind a "hidden"
                          * type or not (look macros IMPORTED, etc.)
                          */

/*  following macros are used with the variable 'imported_fun'
 */
#define IMPORTED 1
#define NOT_IMPORTED 0

/******************************************************************************
 *
 * Function:
 *   node *Types2Array( types *type, types *res_type)
 *
 * Description:
 *   Creates and computes the basic-shape out of a types struct as N_array.
 *   NOTE: If the type is not the type of an array, NULL will be returned.
 *
 ******************************************************************************/

node *
Types2Array (types *type, types *res_type)
{
    node *shape_array = NULL;
    int i;

    DBUG_ENTER ("Types2Array");

    if (T_user == type->simpletype) {
        types *b_type = TYPEDEF_TYPE (LookupType (type->name, type->name_mod, 042));
        if (0 < b_type->dim + type->dim) {
            node *dummy = MakeExprs (NULL, NULL);
            shape_array = MakeFlatArray (NULL);
            ARRAY_TYPE (shape_array) = DupAllTypes (res_type);
            shape_array->node[0] = dummy;
            for (i = 0; i < type->dim - 1; i++) {
                dummy->node[0] = MakeNum (type->shpseg->shp[i]);
                dummy->node[1] = MakeExprs (NULL, NULL);
                dummy = dummy->node[1];
            }
            if (0 < type->dim) {
                dummy->node[0] = MakeNum (type->shpseg->shp[i]);
                if (0 < b_type->dim) {
                    dummy->node[1] = MakeExprs (NULL, NULL);
                    dummy = dummy->node[1];
                }
            }
            for (i = 0; i < b_type->dim - 1; i++) {
                dummy->node[0] = MakeNum (b_type->shpseg->shp[i]);
                dummy->node[1] = MakeExprs (NULL, NULL);
                dummy = dummy->node[1];
            }
            if (0 < b_type->dim) {
                dummy->node[0] = MakeNum (b_type->shpseg->shp[i]);
            }
        }
    } else {
        if (0 < type->dim) {
            node *dummy = MakeExprs (NULL, NULL);
            shape_array = MakeFlatArray (NULL);
            ARRAY_TYPE (shape_array) = DupAllTypes (res_type);
            shape_array->node[0] = dummy;
            for (i = 0; i < type->dim - 1; i++) {
                dummy->node[0] = MakeNum (type->shpseg->shp[i]);
                dummy->node[1] = MakeExprs (NULL, NULL);
                dummy = dummy->node[1];
            }
            dummy->node[0] = MakeNum (type->shpseg->shp[i]);
        }
    }

    if (shape_array != NULL)
        shape_array = AdjustVectorShape (shape_array);

    DBUG_RETURN (shape_array);
}

/******************************************************************************
 *
 * function:
 *   node *Type2Vec(types *type)
 *
 * description:
 *   builds an N_array alike shape of types
 *
 ******************************************************************************/

static node *
Type2Vec (types *type)
{
    node *res;
    int i;
    void *const_vec;
    node *tmp_num;

    DBUG_ENTER ("Type2Vec");
    DBUG_ASSERT ((TYPES_DIM (type) > 0), "Type2Vec() called with type with dim <= 0");

    res = NULL;
    const_vec = AllocConstVec (T_int, TYPES_DIM (type));
    for (i = TYPES_DIM (type) - 1; i >= 0; i--) {
        tmp_num = MakeNum (TYPES_SHAPE (type, i));
        res = MakeExprs (tmp_num, res);
        const_vec = ModConstVec (T_int, const_vec, i, tmp_num);
    }
    res = MakeFlatArray (res);
    ARRAY_ISCONST (res) = TRUE;
    ARRAY_VECTYPE (res) = T_int;
    ARRAY_VECLEN (res) = TYPES_DIM (type);
    ARRAY_CONSTVEC (res) = const_vec;

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *ArrayOfZeros (int count)
 *
 * description:
 *   Produces an Array (N_array) with count elements, which are all zero (0).
 *
 ******************************************************************************/

static node *
ArrayOfZeros (int count)
{
    node *res;
    int i;

    DBUG_ENTER ("ArrayZeros");

    res = NULL;
    for (i = count - 1; i >= 0; i--) {
        res = MakeExprs (MakeNum (0), res);
    }
    res = MakeFlatArray (res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int TypeToCount (types *type)
 *
 * description:
 *   type has to be an array-type.
 *   Calculates how many Elements are contained in an array of Type type.
 *
 ******************************************************************************/

static int
TypeToCount (types *type)
{
    int i, res;

    DBUG_ENTER ("TypeToCount");
    DBUG_ASSERT ((TYPES_DIM (type) > 0), "TypeToCount() called with type dim <= 0");

    res = 1;
    for (i = 0; i < TYPES_DIM (type); i++) {
        res = res * (TYPES_SHAPE (type, i));
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *BuildSelWithLoop(types *restype, node *idx, node *array)
 *
 * description:
 *   This function creates one large with-loop that replaces a single function
 *   application of a primitive sel() with an array as value.
 *
 *   Example:
 *
 *     idx = [M,M];
 *     A   = reshape( [N, N, N, N], [ ... ]));
 *     a   = sel( idx, A);
 *
 *   is transformed into
 *
 *     a = with ( . <= [ _tmp_4, _tmp_3 ] <= . )
 *         {
 *           _tmp_8 = [0];
 *           _tmp_6 = sel( _tmp_8, idx);
 *           _tmp_7 = [1];
 *           _tmp_5 = sel( _tmp_7, idx);
 *
 *           _tmp_2 = [ _tmp_6, _tmp_5, _tmp_4, _tmp_3 ];
 *           _tmp_1 = sel( _tmp_2, A);
 *         }
 *         genarray( [M,M], _tmp_1);
 *
 * remarks:
 *  iv_scalars is initialized only for avoiding a compiler warning
 *  "...might be used uninitialized". This should not have any bad effect
 *  since the for-loop where it will be initialized is executed anyways.
 *
 ******************************************************************************/

static node *
BuildSelWithLoop (types *restype, node *idx, node *array)
{
    int i, num_fresh_vars, var_offset;
    char **tmp_vars;
    node *aelems, *res, *assign;
    ids *iv_scalars = NULL, *last_iv_scalars;

    DBUG_ENTER ("BuildSelWithLoop");

    DBUG_ASSERT ((NODE_TYPE (idx) == N_id), "first arg to sel() must be N_id");
    DBUG_ASSERT ((NODE_TYPE (array) == N_id), "second arg to sel() must be N_id");

    num_fresh_vars = 2 * ID_DIM (array) - TYPES_DIM (restype) + 2;

    tmp_vars = (char **)Malloc (num_fresh_vars * sizeof (char *));

    for (i = 0; i < num_fresh_vars; i++) {
        tmp_vars[i] = TmpVar ();
    }

    assign = MakeAssign (MakeLet (MakePrf2 (F_sel,
                                            MakeId (StringCopy (tmp_vars[1]), NULL,
                                                    ST_regular),
                                            array),
                                  MakeIds (tmp_vars[0], NULL, ST_regular)),
                         NULL);

    aelems = NULL;

    for (i = 2; i < ID_DIM (array) + 2; i++) {
        aelems = MakeExprs (MakeId (StringCopy (tmp_vars[i]), NULL, ST_regular), aelems);
    }

    assign = MakeAssign (MakeLet (MakeFlatArray (aelems),
                                  MakeIds (tmp_vars[1], NULL, ST_regular)),
                         assign);

    var_offset = ID_DIM (array) - TYPES_DIM (restype);

    for (i = 2 + TYPES_DIM (restype); i < 2 + ID_DIM (array); i++) {
        assign
          = MakeAssign (MakeLet (MakePrf2 (F_sel,
                                           MakeId (StringCopy (tmp_vars[i + var_offset]),
                                                   NULL, ST_regular),
                                           MakeId (StringCopy (ID_NAME (idx)), NULL,
                                                   ST_regular)),
                                 MakeIds (tmp_vars[i], NULL, ST_regular)),
                        assign);

        aelems = MakeExprs (MakeNum ((var_offset - 1) - (i - (2 + TYPES_DIM (restype)))),
                            NULL);

        assign
          = MakeAssign (MakeLet (MakeFlatArray (aelems),
                                 MakeIds (tmp_vars[i + var_offset], NULL, ST_regular)),
                        assign);
    }

    last_iv_scalars = NULL;

    for (i = 2; i < TYPES_DIM (restype) + 2; i++) {
        iv_scalars = MakeIds (tmp_vars[i], NULL, ST_regular);
        IDS_NEXT (iv_scalars) = last_iv_scalars;
        last_iv_scalars = iv_scalars;
    }

    res = MakeNWith (MakeNPart (MakeNWithid (NULL, iv_scalars),
                                MakeNGenerator (MakeDot (1), MakeDot (1), F_le, F_le,
                                                NULL, NULL),
                                NULL),
                     MakeNCode (MakeBlock (assign, NULL),
                                MakeExprs (MakeId (StringCopy (tmp_vars[0]), NULL,
                                                   ST_regular),
                                           NULL)),
                     MakeNWithOp (WO_genarray, Type2Vec (restype)));

    NCODE_USED (NWITH_CODE (res))++;

    /*
     * Finally, we generate the connection between the
     * (only) partition and the (only) code!
     */
    NPART_CODE (NWITH_PART (res)) = NWITH_CODE (res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *BuildGenarrayWithLoop(node *shp, node *val)
 *
 * description:
 *
 *
 ******************************************************************************/

static node *
BuildGenarrayWithLoop (node *shp, node *val)
{
    node *res, *tmp_node;
    int i;

    DBUG_ENTER ("BuildGenarrayWithLoop");
    DBUG_ASSERT ((NODE_TYPE (val) != N_exprs),
                 "BuildGenarrayWithLoop called with N_exprs");

    res = MakeNWith (MakeNPart (MakeNWithid (MakeIds (TmpVar (), NULL, ST_regular), NULL),
                                MakeNGenerator (MakeDot (1), MakeDot (1), F_le, F_le,
                                                NULL, NULL),
                                NULL),
                     MakeNCode (MAKE_EMPTY_BLOCK (), MakeExprs (val, NULL)),
                     MakeNWithOp (WO_genarray, NULL));
    if (NODE_TYPE (shp) == N_array) {
        NWITHOP_SHAPE (NWITH_WITHOP (res)) = shp;
    } else {
        /*
         * unflatten the array
         */
        tmp_node = NULL;
        for (i = ID_VECLEN (shp) - 1; i >= 0; i--)
            tmp_node = MakeExprs (MakeNum (((int *)ID_CONSTVEC (shp))[i]), tmp_node);
        tmp_node = MakeFlatArray (tmp_node);
        ARRAY_VECTYPE (tmp_node) = ID_VECTYPE (shp);
        ARRAY_ISCONST (tmp_node) = ID_ISCONST (shp);
        ARRAY_VECLEN (tmp_node) = ID_VECLEN (shp);
        ARRAY_CONSTVEC (tmp_node)
          = CopyConstVec (ID_VECTYPE (shp), ID_VECLEN (shp), ID_CONSTVEC (shp));
        ARRAY_TYPE (tmp_node) = ID_TYPE (shp);
        NWITHOP_SHAPE (NWITH_WITHOP (res)) = tmp_node;
    }
    NCODE_USED (NWITH_CODE (res))++;
    /*
     * Finally, we generate the connection between the
     * (only) partition and the (only) code!
     */
    NPART_CODE (NWITH_PART (res)) = NWITH_CODE (res);

    if (NODE_TYPE (NCODE_CEXPR (NWITH_CODE (res))) != N_id) {
        /*
         * Still, the expression might be a constant array since no arrays are
         * flattened out of applications of primitive genarray.
         */

        char *new_var = TmpVar ();
        node *block = NCODE_CBLOCK (NWITH_CODE (res));

        /*
         * Remove N_empty node behind N_block node.
         */
        FreeTree (BLOCK_INSTR (block));

        BLOCK_INSTR (block) = MakeAssign (MakeLet (NCODE_CEXPR (NWITH_CODE (res)),
                                                   MakeIds (new_var, NULL, ST_regular)),
                                          NULL);
        NCODE_CEXPR (NWITH_CODE (res)) = MakeId (StringCopy (new_var), NULL, ST_regular);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *BuildTakeWithLoop(node *take_shp, node *array)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

static node *
BuildTakeWithLoop (node *take_shp, node *array)
{
    node *res, *body;
    char *wl_body_var;
    char *iv;

    DBUG_ENTER ("BuildTakeWithLoop");

    wl_body_var = TmpVar ();
    iv = TmpVar ();

    body = MakeAssign (MakeLet (MakePrf (F_sel, MakeExprs (MakeId (StringCopy (iv), NULL,
                                                                   ST_regular),
                                                           MakeExprs (array, NULL))),
                                MakeIds (wl_body_var, NULL, ST_regular)),
                       NULL);

    res = MakeNWith (MakeNPart (MakeNWithid (MakeIds (iv, NULL, ST_regular), NULL),
                                MakeNGenerator (MakeDot (1), MakeDot (1), F_le, F_le,
                                                NULL, NULL),
                                NULL),
                     MakeNCode (MakeBlock (body, NULL),
                                MakeExprs (MakeId (StringCopy (wl_body_var), NULL,
                                                   ST_regular),
                                           NULL)),
                     MakeNWithOp (WO_genarray, NULL));

    if (NODE_TYPE (take_shp) == N_array) {
        /*
         * since we have constant arrays flattened, we assume that this case does not
         * occur anymore.
         */
        DBUG_ASSERT (0, "First argument of take operation not flattened!");

        NWITHOP_SHAPE (NWITH_WITHOP (res)) = take_shp;
    } else {
        /*
         * unflatten the array
         */
        int i;
        node *tmp_node = NULL;
        for (i = ID_VECLEN (take_shp) - 1; i >= 0; i--)
            tmp_node = MakeExprs (MakeNum (((int *)ID_CONSTVEC (take_shp))[i]), tmp_node);
        tmp_node = MakeFlatArray (tmp_node);
        ARRAY_VECLEN (tmp_node) = ID_VECLEN (take_shp);
        ARRAY_CONSTVEC (tmp_node)
          = CopyConstVec (ID_VECTYPE (take_shp), ID_VECLEN (take_shp),
                          ID_CONSTVEC (take_shp));
        ARRAY_TYPE (tmp_node) = ID_TYPE (take_shp);
        ARRAY_VECTYPE (tmp_node) = ID_VECTYPE (take_shp);
        ARRAY_ISCONST (tmp_node) = TRUE;
        NWITHOP_SHAPE (NWITH_WITHOP (res)) = tmp_node;
    }
    (NCODE_USED (NWITH_CODE (res)))++;

    /*
     * Finally, we generate the connection between the
     * (only) partition and the (only) code!
     */
    NPART_CODE (NWITH_PART (res)) = NWITH_CODE (res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *BuildDropWithLoop(types *new_shape, node *drop_vec, node *array)
 *
 * description:
 *
 *
 ******************************************************************************/

static node *
BuildDropWithLoop (types *new_shape, node *drop_vec, node *array)
{
    node *res, *body, *aelem, *new_array, *tmp_var;
    char *wl_body_var_vec;
    char *wl_body_var_elem;
    char *new_drop_vec;
    char *iv;
    int i, len_vec, dim_array;

    DBUG_ENTER ("BuildDropWithLoop");

    DBUG_ASSERT (((NODE_TYPE (drop_vec) == N_array)
                  || ((NODE_TYPE (drop_vec) == N_id) && (ID_ISCONST (drop_vec))
                      && (VARDEC_OR_ARG_BASETYPE (ID_VARDEC (drop_vec)) == T_int))),
                 "First arg of drop should be a constant integer array!");

    wl_body_var_vec = TmpVar ();
    wl_body_var_elem = TmpVar ();
    new_drop_vec = TmpVar ();
    iv = TmpVar ();

    /*
     * First, we expand the length of "drop_vec" to the rank of "array"
     * with 0's.
     */
    tmp_var = NULL;

    if (NODE_TYPE (drop_vec) == N_array) {

        /*
         * since we have constant arrays flattened, we assume that this case does not
         * occur anymore.
         */
        DBUG_ASSERT (0, "First argument of drop operation not flattened!");

        len_vec = TYPES_SHAPE (ARRAY_TYPE (drop_vec), 0);
        dim_array = TYPES_DIM (new_shape); /* shape(result) == shape(array) !! */

        if (len_vec < dim_array) {
            aelem = ARRAY_AELEMS (drop_vec);
            /* overread given elements */
            while (EXPRS_NEXT (aelem) != NULL) {
                aelem = EXPRS_NEXT (aelem);
            }
            /* fillup len_vec to dim_array with 0 */
            for (i = len_vec; i < dim_array; i++) {
                EXPRS_NEXT (aelem) = MakeExprs (MakeNum (0), NULL);
                aelem = EXPRS_NEXT (aelem);
            }
        }
    } else { /* NODE_TYPE(drop_vec) == N_id !! */
        len_vec = ID_VECLEN (drop_vec);
        dim_array = TYPES_DIM (new_shape); /* shape(result) == shape(array) !! */

        if (len_vec < dim_array) {
            aelem = IntVec2Array (ID_VECLEN (drop_vec), ((int *)ID_CONSTVEC (drop_vec)));

            if (aelem == NULL) {
                for (i = len_vec; i < dim_array; i++) {
                    aelem = MakeExprs (MakeNum (0), aelem);
                    new_array = MakeFlatArray (aelem);
                }
            } else {
                new_array = MakeFlatArray (aelem);
                while (EXPRS_NEXT (aelem) != NULL) {
                    aelem = EXPRS_NEXT (aelem);
                }
                for (i = len_vec; i < dim_array; i++) {
                    EXPRS_NEXT (aelem) = MakeExprs (MakeNum (0), NULL);
                    aelem = EXPRS_NEXT (aelem);
                }
            }

            tmp_var = MakeLet (new_array, MakeIds (new_drop_vec, NULL, ST_regular));
            ID_NAME (drop_vec) = StringCopy (new_drop_vec);
            ID_VECLEN (drop_vec) = dim_array;
            ID_CONSTVEC (drop_vec) = Free (ID_CONSTVEC (drop_vec));
            ((int *)ID_CONSTVEC (drop_vec))
              = Array2IntVec (ARRAY_AELEMS (new_array), NULL);
            ID_VECTYPE (drop_vec) = T_int;
            ID_ISCONST (drop_vec) = TRUE;
        }
    }

    body = MakeAssign (MakeLet (MakePrf (F_sel,
                                         MakeExprs (MakeId (StringCopy (wl_body_var_vec),
                                                            NULL, ST_regular),
                                                    MakeExprs (array, NULL))),
                                MakeIds (wl_body_var_elem, NULL, ST_regular)),
                       NULL);
    body
      = MakeAssign (MakeLet (MakePrf (F_add_AxA, MakeExprs (MakeId (StringCopy (iv), NULL,
                                                                    ST_regular),
                                                            MakeExprs (drop_vec, NULL))),
                             MakeIds (wl_body_var_vec, NULL, ST_regular)),
                    body);

    if (tmp_var != NULL) {
        body = MakeAssign (tmp_var, body);
    }

    res = MakeNWith (MakeNPart (MakeNWithid (MakeIds (iv, NULL, ST_regular), NULL),
                                MakeNGenerator (MakeDot (1), MakeDot (1), F_le, F_le,
                                                NULL, NULL),
                                NULL),
                     MakeNCode (MakeBlock (body, NULL),
                                MakeExprs (MakeId (StringCopy (wl_body_var_elem), NULL,
                                                   ST_regular),
                                           NULL)),
                     MakeNWithOp (WO_genarray, Type2Vec (new_shape)));

    NCODE_USED (NWITH_CODE (res))++;

    /*
     * Finally, we generate the connection between the
     * (only) partition and the (only) code!
     */
    NPART_CODE (NWITH_PART (res)) = NWITH_CODE (res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *BuildCatWithLoop1(types *new_shape, node *array1)
 *
 * description:
 *
 *
 ******************************************************************************/

static node *
BuildCatWithLoop1 (types *new_shape, node *array1)
{
    node *res, *body;
    char *wl_body_var;
    char *iv;

    DBUG_ENTER ("BuildCatWithLoop1");

    wl_body_var = TmpVar ();
    iv = TmpVar ();

    body = MakeAssign (MakeLet (MakePrf (F_sel, MakeExprs (MakeId (StringCopy (iv), NULL,
                                                                   ST_regular),
                                                           MakeExprs (array1, NULL))),
                                MakeIds (wl_body_var, NULL, ST_regular)),
                       NULL);

    res = MakeNWith (MakeNPart (MakeNWithid (MakeIds (iv, NULL, ST_regular), NULL),
                                MakeNGenerator (MakeDot (1),
                                                (ID_DIM (array1) > SCALAR)
                                                  ? Type2Vec (ID_TYPE (array1))
                                                  : MakePrf1 (F_shape, DupTree (array1)),
                                                F_le, F_lt, NULL, NULL),
                                NULL),
                     MakeNCode (MakeBlock (body, NULL),
                                MakeExprs (MakeId (StringCopy (wl_body_var), NULL,
                                                   ST_regular),
                                           NULL)),
                     MakeNWithOp (WO_genarray, Type2Vec (new_shape)));

    NCODE_USED (NWITH_CODE (res))++;

    /*
     * Finally, we generate the connection between the
     * (only) partition and the (only) code!
     */
    NPART_CODE (NWITH_PART (res)) = NWITH_CODE (res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *BuildCatWithLoop2(ids *lhs, node *arg1, node *arg2, node *arg3)
 *
 * description:
 *
 *
 ******************************************************************************/

static node *
BuildCatWithLoop2 (ids *lhs, node *arg1, node *arg2, node *arg3)
{
    node *res, *body, *tmp_num, *start_vec, *start_vec_copy;
    char *wl_body_var_vec;
    char *wl_body_var_elem;
    char *wl_body_var_offset;
    char *iv;
    void *const_vec;
    int i;

    DBUG_ENTER ("BuildCatWithLoop2");

    wl_body_var_vec = TmpVar ();
    wl_body_var_elem = TmpVar ();
    wl_body_var_offset = TmpVar ();
    iv = TmpVar ();

    DBUG_ASSERT ((NODE_TYPE (arg1) == N_num),
                 "Illegal 1st argument to primitive function cat()");

    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                 "Illegal 2nd argument to primitive function cat()");

    DBUG_ASSERT ((ID_DIM (arg2) > 0),
                 "Type of 2nd argument to primitive function cat(): dim <= 0");

    start_vec = NULL;
    const_vec = AllocConstVec (T_int, ID_DIM (arg2));

    for (i = ID_DIM (arg2) - 1; i > NUM_VAL (arg1); i--) {
        tmp_num = MakeNum (0);
        start_vec = MakeExprs (tmp_num, start_vec);
        const_vec = ModConstVec (T_int, const_vec, i, tmp_num);
    }

    tmp_num = MakeNum (ID_SHAPE (arg2, NUM_VAL (arg1)));
    start_vec = MakeExprs (tmp_num, start_vec);
    const_vec = ModConstVec (T_int, const_vec, i, tmp_num);

    for (i = NUM_VAL (arg1) - 1; i >= 0; i--) {
        tmp_num = MakeNum (0);
        start_vec = MakeExprs (tmp_num, start_vec);
        const_vec = ModConstVec (T_int, const_vec, i, tmp_num);
    }

    start_vec = MakeFlatArray (start_vec);
    ARRAY_ISCONST (start_vec) = TRUE;
    ARRAY_VECTYPE (start_vec) = T_int;
    ARRAY_VECLEN (start_vec) = ID_DIM (arg2);
    ARRAY_CONSTVEC (start_vec) = const_vec;

    start_vec_copy = DupTree (start_vec);

    body
      = MakeAssign (MakeLet (start_vec, MakeIds (wl_body_var_offset, NULL, ST_regular)),
                    MakeAssign (MakeLet (MakePrf2 (F_sub_AxA,
                                                   MakeId (StringCopy (iv), NULL,
                                                           ST_regular),
                                                   MakeId (StringCopy (
                                                             wl_body_var_offset),
                                                           NULL, ST_regular)),
                                         MakeIds (wl_body_var_vec, NULL, ST_regular)),
                                MakeAssign (MakeLet (MakePrf2 (F_sel,
                                                               MakeId (StringCopy (
                                                                         wl_body_var_vec),
                                                                       NULL, ST_regular),
                                                               arg3),
                                                     MakeIds (wl_body_var_elem, NULL,
                                                              ST_regular)),
                                            NULL)));

    res = MakeNWith (MakeNPart (MakeNWithid (MakeIds (iv, NULL, ST_regular), NULL),
                                MakeNGenerator (start_vec_copy, MakeDot (1), F_le, F_le,
                                                NULL, NULL),
                                NULL),
                     MakeNCode (MakeBlock (body, NULL),
                                MakeExprs (MakeId (StringCopy (wl_body_var_elem), NULL,
                                                   ST_regular),
                                           NULL)),
                     MakeNWithOp (WO_modarray, MakeId (StringCopy (IDS_NAME (lhs)), NULL,
                                                       ST_regular)));

    NCODE_USED (NWITH_CODE (res))++;

    /*
     * Finally, we generate the connection between the
     * (only) partition and the (only) code!
     */
    NPART_CODE (NWITH_PART (res)) = NWITH_CODE (res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   node *LookupObject(char *name, char *mod, int line)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
LookupObject (char *name, char *mod, int line)
{
    node *tmp, *found = NULL;
    mods *mods_avail;

    DBUG_ENTER ("LookupObject");

    /*
     * First of all, we check the specified module for the object.
     * If no module is specified explicitly, we search within the current
     * module.
     */

    if (mod == NULL) {
        tmp = object_table;

        while ((tmp != NULL) && (CMP_OBJ_OBJDEF (name, module_name, tmp) == 0)) {
            tmp = OBJDEF_NEXT (tmp);
        }

        if (tmp != NULL) {
            found = tmp;
        }
    } else {
        tmp = object_table;

        while ((tmp != NULL) && (CMP_OBJ_OBJDEF (name, mod, tmp) == 0)) {
            tmp = OBJDEF_NEXT (tmp);
        }

        if (tmp != NULL) {
            found = tmp;
        }
    }

    if (found == NULL) {
        if (mod == NULL) {
            /*
             * If we didn't find the object in the current module,
             * we now have look into all imported modules. Ambiguities
             * cause errors.
             */

            tmp = object_table;

            while (tmp != NULL) {
                if (CMP_OBJ_OBJDEF (name, NULL, tmp)) {
                    if (found == NULL) {
                        found = tmp;
                    } else {
                        ERROR (line,
                               ("Identifier '%s` matches more than one global object",
                                name));
                    }
                }
                tmp = OBJDEF_NEXT (tmp);
            }
        } else {
            /*
             * If we didn't find the object in the specified module,
             * we now have look into all modules which are imported by the
             * specified module. Ambiguities cause errors.
             */

            mods_avail = FindSymbolInModul (mod, name, 3, NULL, 1);
            while (mods_avail != NULL) {
                tmp = object_table;
                while (tmp != NULL) {
                    if (CMP_OBJ_OBJDEF (name, mods_avail->mod->name, tmp)) {
                        if (found == NULL) {
                            found = tmp;
                        } else {
                            ERROR (line,
                                   ("Identifier '%s` matches more than one global object",
                                    name));
                        }
                    }
                    tmp = OBJDEF_NEXT (tmp);
                }
                mods_avail = mods_avail->next;
            }
        }
    }

    DBUG_RETURN (found);
}

/******************************************************************************
 *
 * Function:
 *   node *LookupType( char *type_name, char *mod_name, int line)
 *
 * Description:
 *   Looks type up and returns pointer to corresponding N_typedef node if
 *   found or NULL if not found.
 *
 ******************************************************************************/

node *
LookupType (char *type_name, char *mod_name, int line)
{
    int i, defined;
    node *ret_node = NULL;
    mods *mods[2];

    DBUG_ENTER ("LookupType");

    DBUG_ASSERT ((type_name != NULL), "No type name found to look up!");

    if (NULL != mod_name) {
        if (0 != strcmp (mod_name, (NULL != module_name ? module_name : ""))) {
            /* now look for the module where the type is defined */

            /* look if type is local to module */
            mods[0] = FindSymbolInModul (mod_name, type_name, 0, NULL, 0);
            if (NULL == mods[0]) {
                mods[1] = FindSymbolInModul (mod_name, type_name, 1, NULL, 0);
                if (NULL == mods[1]) {
                    /* now look for imported types as well */
                    defined = 0;
                    for (i = 0; i < 2; i++) {
                        mods[i] = FindSymbolInModul (mod_name, type_name, i, NULL, 1);
                        if (NULL != mods[i]) {
                            if (NULL != mods[i]->next) {
                                defined += 2;
                                break;
                            } else {
                                defined += 1;
                            }
                        }
                    }

                    if (0 == defined) {
                        ABORT (line,
                               ("Type '%s` is not defined or imported in module '%s`",
                                type_name, mod_name));
                    }

                    if (2 <= defined) {
                        ABORT (line, ("Type '%s` is defined or"
                                      " imported more than once in module '%s`",
                                      type_name, mod_name));
                    }

                    DBUG_ASSERT ((1 == defined), "defined != 1");
                    if (NULL == mods[0]) {
                        mod_name = mods[1]->mod->name;
                    } else {
                        mod_name = mods[0]->mod->name;
                    }
                } else {
                    mod_name = mods[1]->mod->name;
                }
            } else {
                mod_name = mods[0]->mod->name;
            }
        }
        for (i = 0; i < type_tab_size; i++) {
            if (CMP_MOD (mod_name, TYPEDEF_MOD ((type_table + i)->node))
                && (!strcmp (type_name, TYPEDEF_NAME ((type_table + i)->node)))) {
                ret_node = (type_table + i)->node;
                break;
            }
        }
    } else {
        for (i = 0; i < type_tab_size; i++) {
            if (!strcmp (type_name, TYPEDEF_NAME ((type_table + i)->node))) {
                if (1 == (type_table + i)->id_count) {
                    ret_node = (type_table + i)->node;
                    break;
                } else {
                    ABORT (line, ("Type '%s` has more than one definition", type_name));
                }
            }
        }
    }

    DBUG_RETURN (ret_node);
}

/*
 *
 *  functionname  : LookupVar
 *  arguments     : 1) identifier
 *  description   : looks for 1) on the scope stack
 *                  returns pointer to stack if found, NULL otherwise
 *  global vars   : tos, type_string, stack
 *
 */

static stack_elem *
LookupVar (char *id)
{
    stack_elem *tmp, *bottom;
    bool is_defined = FALSE;
#ifndef DBUG_OFF
    char *db_str;
#endif

    DBUG_ENTER ("LookupVar");

    DBUG_PRINT ("TYPE", ("looking for id: %s", id));

    bottom = act_frame;
    tmp = tos - 1;

    while ((tmp >= bottom) && (!is_defined)) {
        DBUG_PRINT ("TYPE", ("current id: %s" F_PTR, tmp->id, tmp));

        if (!strcmp (tmp->id, id)) {
            is_defined = TRUE;
        } else {
            tmp--;
        }
    }

    if (is_defined) {
#ifndef DBUG_OFF
        db_str = Type2String (VARDEC_OR_ARG_TYPE (tmp->node), 0, TRUE);
        DBUG_PRINT ("TYPE", ("found: %s %s", db_str, id));
        db_str = Free (db_str);
#endif
    } else {
        DBUG_PRINT ("TYPE", ("not found"));
        tmp = NULL;
    }

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : LookupFun
 *  arguments     : 1) name of function
 *                  2) name of module, where function is defined
 *                  3) N_fundef node
 *  description   : looks whether specified function is in the fun_table
 *                  returns pointer to table entry if function was found,
 *                          otherwise NULL
 *  global vars   : fun_table
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., CMP_MOD,
 *  remarks       : if 3) is not NULL then  look for this node in fun_table
 *                                          1) and 2) do not care
 *                   else  if 2) is NULL then look only 1) up
 *
 */

static fun_tab_elem *
LookupFun (char *fun_name, char *mod_name, node *fun_node)
{
    fun_tab_elem *tmp;

    DBUG_ENTER ("LookupFun");

    if (NULL == fun_node) {
        DBUG_PRINT ("TYPE", ("looking for function '%s'", ModName (mod_name, fun_name)));

        for (tmp = fun_table; !END_OF_FUN_TAB (tmp); tmp = NEXT_FUN_TAB_ELEM (tmp))
            if (!strcmp (fun_name, tmp->id)) {
                if (NULL == mod_name)
                    break;
                else if (CMP_MOD (mod_name, tmp->id_mod))
                    break;
            }
    } else {
        DBUG_PRINT ("TYPE", ("looking for function '%s%s%s'",
                             ModName (FUNDEF_MOD (fun_node), FUNDEF_NAME (fun_node))));

        for (tmp = fun_table; !END_OF_FUN_TAB (tmp); tmp = NEXT_FUN_TAB_ELEM (tmp))
            if (tmp->node == fun_node)
                break;
    }

    if (END_OF_FUN_TAB (tmp)) {
        DBUG_PRINT ("TYPE", ("function not found"));
        tmp = NULL; /* not found */
    } else
        DBUG_PRINT ("TYPE", ("function found" F_PTR, tmp));

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : LookupPrf
 *  arguments     : 1) "name" of function
 *                  2) name of module, where function is defined
 *  description   : looks wheater spezified function is in the fun_table
 *                  returns pointer to table entry if function was found,
 *                          or NULL
 *  global vars   : prim_fun_tab
 *  internal funs : ----
 *  external funs : ----
 *  macros        : DBUG...,
 *  remarks       : ----
 *
 */

static prim_fun_tab_elem *
LookupPrf (prf prf, char *mod_name)
{
    prim_fun_tab_elem *prf_p;

    DBUG_ENTER ("LookupPrf");

    DBUG_PRINT ("TYPE", ("looking for: %s", mdb_prf[prf]));

    prf_p = prim_fun_tab;
    while (NULL != prf_p)
        if (prf_p->prf == prf)
            break;
        else
            prf_p = prf_p->next;

    DBUG_RETURN (prf_p);
}

/******************************************************************************
 *
 * Function:
 *   node *ComputeNeutralElem(prf prf_fun, types *neutral_type)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
ComputeNeutralElem (prf prf_fun, types *neutral_type, node *arg_info)
{
    node *neutral_elem = NULL;
    node *neutral_base = NULL;

    int length;
    simpletype stype;

    DBUG_ENTER ("ComputeNeutralElem");

    length = GetTypesLength (neutral_type);
    stype = GetBasetype (neutral_type);

    switch (prf_fun) {
    case F_add_SxS:
    case F_add_AxA:
        switch (stype) {
        case T_int:
            neutral_base = MakeNum (0);
            break;
        case T_float:
            neutral_base = MakeFloat (0.0f);
            break;
        case T_double:
            neutral_base = MakeDouble (0.0);
            break;
        default:
            DBUG_ASSERT (0, "Fold operator '+` incompatible with operand type");
        }
        break;

    case F_mul_SxS:
    case F_mul_AxA:
        switch (stype) {
        case T_int:
            neutral_base = MakeNum (1);
            break;
        case T_float:
            neutral_base = MakeFloat (1.0f);
            break;
        case T_double:
            neutral_base = MakeDouble (1.0);
            break;
        default:
            DBUG_ASSERT (0, "Fold operator '*` incompatible with operand type");
        }
        break;

    case F_and:
        neutral_base = MakeBool (1);
        break;

    case F_or:
        neutral_base = MakeBool (0);
        break;

    case F_min:
        switch (stype) {
        case T_int:
            neutral_base = MakeNum (INT_MAX);
            break;
        case T_float:
            neutral_base = MakeFloat (FLT_MAX);
            break;
        case T_double:
            neutral_base = MakeDouble (DBL_MAX);
            break;
        default:
            DBUG_ASSERT (0, "Fold operator 'min` incompatible with operand type");
        }
        break;

    case F_max:
        switch (stype) {
        case T_int:
            /* Due to a bug in limits.h (!!!), we have to use INT_MAX here!!! */
            neutral_base = MakeNum (1 - INT_MAX);
            break;
        case T_float:
            neutral_base = MakeFloat (-FLT_MAX);
            break;
        case T_double:
            neutral_base = MakeDouble (-DBL_MAX);
            break;
        default:
            DBUG_ASSERT (0, "Fold operator 'max` incompatible with operand type");
        }
        break;

    default:
        DBUG_PRINT ("NEUTRAL", ("prf :%s (%d)", mdb_prf[prf_fun], prf_fun));
        DBUG_ASSERT (0, "Illegal fold operator");
    }

    if (length > 0) {
        /* dkr: dirty hack for new backend */
        shpseg *shape;
        int dim;

        shape = Type2Shpseg (neutral_type, &dim);
        neutral_elem
          = CreateScalarWith (dim, shape, stype, neutral_base, INFO_TC_FUNDEF (arg_info));
        if (NODE_TYPE (neutral_elem) == N_Nwith) {
            node *new_assigns = NULL;
            neutral_elem = LiftArg (neutral_elem, INFO_TC_FUNDEF (arg_info), neutral_type,
                                    FALSE, &new_assigns);
            if (FUNDEF_VARDEC (INFO_TC_FUNDEF (arg_info)) != NULL) {
                INFO_TC_VARDEC (arg_info)
                  = AppendVardec (INFO_TC_VARDEC (arg_info),
                                  FUNDEF_VARDEC (INFO_TC_FUNDEF (arg_info)));
            }
            if (new_assigns != NULL) {
                if (NODE_TYPE (INFO_TC_LASSIGN (arg_info)) == N_block) {
                    new_assigns = AppendAssign (new_assigns,
                                                BLOCK_INSTR (INFO_TC_LASSIGN (arg_info)));
                    BLOCK_INSTR (INFO_TC_LASSIGN (arg_info)) = new_assigns;
                } else {
                    new_assigns = AppendAssign (new_assigns,
                                                ASSIGN_NEXT (INFO_TC_LASSIGN (arg_info)));
                    ASSIGN_NEXT (INFO_TC_LASSIGN (arg_info)) = new_assigns;
                }
            }
        }
    } else {
        neutral_elem = neutral_base;
    }

    DBUG_RETURN (neutral_elem);
}

/******************************************************************************
 *
 * function:
 *   node *TypecheckFunctionDeclarations(node *fundef)
 *
 * description:
 *   Function declarations (N_fundef nodes without body) are usually not
 *   typechecked. However, since user-defined types now have a back reference
 *   to the corresponding N_typedef node, function declarations have to be
 *   checked for user-defined types.
 *
 *   The same is hold for pseudo-fold-functions!
 *
 ******************************************************************************/

static node *
TypecheckFunctionDeclarations (node *fundef)
{
    types *type;
    node *tdef, *arg, *fun;

    DBUG_ENTER ("TypecheckFunctionDeclarations");

    fun = fundef;

    while (fun != NULL) {
        if ((FUNDEF_BODY (fun) == NULL) || (FUNDEF_STATUS (fun) == ST_foldfun)) {
            /*
             * Pseudo-fold-functions don't have back references to N_typedef nodes
             * yet either!!
             */

            /*
             * First, the return types are checked.
             */
            type = FUNDEF_TYPES (fun);
            while (type != NULL) {
                if (TYPES_BASETYPE (type) == T_user) {
                    tdef
                      = LookupType (TYPES_NAME (type), TYPES_MOD (type), NODE_LINE (fun));
                    if (tdef == NULL) {
                        ERROR (
                          NODE_LINE (fun),
                          ("Unknown type '%s` in declaration of imported function %s",
                           ModName (TYPES_MOD (type), TYPES_NAME (type)),
                           ItemName (fun)));
                    }
                    TYPES_TDEF (type) = tdef;
                }
                type = TYPES_NEXT (type);
            }

            /*
             * Second, the argument types are checked.
             */
            arg = FUNDEF_ARGS (fun);
            while (arg != NULL) {
                if (ARG_BASETYPE (arg) == T_user) {
                    tdef = LookupType (TYPES_NAME (ARG_TYPE (arg)),
                                       TYPES_MOD (ARG_TYPE (arg)), NODE_LINE (fun));
                    if (tdef == NULL) {
                        ERROR (
                          NODE_LINE (fun),
                          ("Unknown type '%s` in declaration of imported function %s",
                           ModName (TYPES_MOD (ARG_TYPE (arg)),
                                    TYPES_NAME (ARG_TYPE (arg))),
                           ItemName (fun)));
                    }
                    TYPES_TDEF (ARG_TYPE (arg)) = tdef;
                }
                arg = ARG_NEXT (arg);
            }
        }

        fun = FUNDEF_NEXT (fun);
    }

    DBUG_RETURN (fundef);
}

/*
 *
 *  functionname  : TI_fun
 *  arguments     : 1) argument node
 *                  2) pointer to fun_tab
 *                  3) info node
 *  description   : checks whether function ( 2) ) has to be typechecked or not
 *                  - calls Trav to typecheck function if necessary
 *                  - returns infered type of the function
 *  remarks       : - arg_node->info.fun_name... contains the name and the modul
 *                    name
 *                  - the compilation-macro DO_NOT_SET_FUN_MOD_NAME can be set
 *                    while calling a c-compiler.
 *                    The module_name belonging to a function( the place where
 *                    it is defined) will not be added or updated while
 *                    typechecking, if the macro is set.
 */

static types *
TI_fun (node *arg_node, fun_tab_elem *fun_p, node *arg_info)
{
    types *return_type;
#ifndef DBUG_OFF
    char *db_str;
#endif

    DBUG_ENTER ("TI_fun");

    DBUG_ASSERT ((INFO_TC_FUNDEF (arg_info) != NULL),
                 "TI_fun(): INFO_TC_FUNDEF( arg_info) is missing!");

    if (fun_p->node == INFO_TC_FUNDEF (arg_info)) {
        if (BODY == INFO_TC_STATUS (arg_info)) {
            ABORT (NODE_LINE (arg_node),
                   ("Nonterminated recursion in function '%s`", arg_node->info.id));
        } else {
            /* recursive call in then or else part */
            if (FAST_CHECK == fun_p->tag) {
                arg_info->node[0]->nodetype = N_stop;
                GEN_TYPE_NODE (return_type, T_unknown);
                DBUG_PRINT ("TYPE", ("return_type is  unknown (tag : %d)", fun_p->tag));
            } else {
#ifndef DO_NOT_SET_FUN_MOD_NAME
                if (NULL != arg_node->FUN_MOD_NAME) {
                    if (0 != strcmp (arg_node->FUN_MOD_NAME, FUNDEF_MOD (fun_p->node))) {
                        arg_node->FUN_MOD_NAME = Free (arg_node->FUN_MOD_NAME);
                        arg_node->FUN_MOD_NAME = StringCopy (FUNDEF_MOD (fun_p->node));
                    }
                } else
                    arg_node->FUN_MOD_NAME = StringCopy (FUNDEF_MOD (fun_p->node));
#endif
                return_type = DupAllTypes (FUNDEF_TYPES (fun_p->node));

#ifndef DBUG_OFF
                db_str = Type2String (return_type, 0, TRUE);
                DBUG_PRINT ("TYPE", ("return_type %s" F_PTR, db_str, return_type));
                db_str = Free (db_str);
#endif
            }
        }
    } else {
        /* it is not a recursive call of current function */
        if ((NOT_CHECKED == fun_p->tag) || (PLEASE_CHECK == fun_p->tag)) {
            /* applied function is not typechecked yet, so do it now */
            if (FUNDEF_BODY (fun_p->node) == NULL) {
                /*
                 * For pure function declarations, the typecheck is trivial.
                 */
                fun_p->tag = IS_CHECKED;
            } else {
                fun_p->tag = FAST_CHECK; /* infer type in only one branch */
                DBUG_PRINT ("TYPE", ("set typecheck tag to %d (FAST_CHECK)", fun_p->tag));

                Trav (fun_p->node, arg_info);
            }
        }
        if ((FAST_CHECK == fun_p->tag)
            || ((BODY != INFO_TC_STATUS (arg_info))
                && ((NOT_CHECKED == fun_p->tag) || (PLEASE_CHECK == fun_p->tag)))) {
            /* if the applied function is still be checked and one checks
             * a conditional then the type can't be infered and one must
             * stop checking this branch.
             */

            arg_info->node[0]->nodetype = N_stop;
            GEN_TYPE_NODE (return_type, T_unknown);
            DBUG_PRINT ("TYPE", ("return_type is  unknown (tag : %d)", fun_p->tag));
        } else {
            DBUG_PRINT ("TYPE",
                        ("typecheck tag of function '%s' : %d", fun_p->id, fun_p->tag));

            if ((IS_CHECKED == fun_p->tag) || (TMP_CHECKED == fun_p->tag)
                || (CHECKING == fun_p->tag)) {
                /* now the applied function is typechecked , so we can return
                 * the infered type.
                 */
#ifndef DO_NOT_SET_FUN_MOD_NAME
                arg_node->FUN_MOD_NAME = FUNDEF_MOD (fun_p->node);
#endif
                return_type = DupAllTypes (FUNDEF_TYPES (fun_p->node));

#ifndef DBUG_OFF
                db_str = Type2String (return_type, 0, TRUE);
                DBUG_PRINT ("TYPE", ("return_type %s" F_PTR, db_str, return_type));
                db_str = Free (db_str);
#endif
            } else
                ABORT (NODE_LINE (arg_node),
                       ("Type of function '%s' not inferable", arg_node->info.id));
        }
    }
    DBUG_RETURN (return_type);
}

/******************************************************************************
 *
 * function:
 *   node *LookupVardec(char *id, node *vardec)
 *
 * description:
 *   Searches the next vardec (begining from given vardec itself) with
 *   VARDEC_NAME( vardec) == id.
 *
 ******************************************************************************/

static node *
LookupVardec (char *id, node *vardec)
{
    DBUG_ENTER ("LookupVardec");

    while ((vardec != NULL) && (strcmp (VARDEC_NAME (vardec), id) != 0)) {
        vardec = VARDEC_NEXT (vardec);
    }

    DBUG_RETURN (vardec);
}

/******************************************************************************
 *
 * function:
 *   node *LookupArg(char *id, node *arg)
 *
 * description:
 *   Searches the next arg (begining from given arg itself) with
 *   ARG_NAME( arg) == id.
 *
 ******************************************************************************/

static node *
LookupArg (char *id, node *arg)
{
    DBUG_ENTER ("LookupArg");

    while ((arg != NULL) && (strcmp (ARG_NAME (arg), id) != 0)) {
        arg = ARG_NEXT (arg);
    }

    DBUG_RETURN (arg);
}

/*
 *
 *  functionname  : CheckIds
 *  arguments     : 1) ids-chain
 *                : 2) vardecs
 *                  3) line number
 *  description   : checks whether the ids-chain contains identifiers with
 *                  equal name or names which are equal to a global object
 *  global vars   : filename
 *  internal funs : LookupType, LookupVar, LookupObject, LookupVardec
 *  external funs : ----
 *  macros        : DBUG..., NULL, ERROR
 *
 *  remarks       : So far, no global objects are allowed on the left
 *                  hand side of a let-construct.
 *
 *     this function seems to be used only to produce error messages,
 *     the ret_value is predefined as 1 in it's declaration,
 *     changed (but not really) only once to 1, so ...
 *                                             IT ALWAYS RETURNS 1!!!
 *                                             (or produces an error)
 *     (jhs)
 */

static int
CheckIds (ids *arg_ids, node *vardec, int line)
{
    ids *tmp, *not_checked_ids;
    char *current_id;
    int ret_value = 1;

    DBUG_ENTER ("CheckIds");

    DBUG_ASSERT ((NULL != arg_ids), "arg_ids is NULL");

    not_checked_ids = arg_ids;
    while (NULL != not_checked_ids) {
        tmp = not_checked_ids->next;
        current_id = not_checked_ids->id;
        while (NULL != tmp) {
            if (!strcmp (tmp->id, current_id)) {
                ERROR (line,
                       ("Identifier '%s` twice on left hand side of '=`", current_id));
                ret_value = 1;
                break;
            } else {
                tmp = tmp->next;
            }
        }
        if (LookupVar (current_id) == NULL) {
            if (LookupVardec (current_id, vardec) == NULL) {
                if (LookupObject (current_id, NULL, line) != NULL) {
                    ERROR (line, ("Global object '%s` not allowed on left "
                                  "hand side of '=`",
                                  current_id));
                }
            }
        }
        not_checked_ids = not_checked_ids->next;
    }

    DBUG_RETURN (ret_value);
}

/*
 *
 *  functionname  : CheckFunctionDeclaration
 *  arguments     : 1) N_fundef node of imported/extern function
 *                  2) tag: 1 => count unknown shapes in result-type also
 *                          0 => don't count unknown shapes in result-type
 *  description   : checks
 *                   - whether declaration contains arrays with unknown shape
 *                   - whether there are formal parameters with same name
 *                   - whether a formal parameter or the function has the
 *                     same name as a userdefined type
 *  global vars   : ----
 *  internal funs : ----
 *  external funs : ----
 *  macros        : DBUG..., NULL
 *
 *  remarks       : - ret_value contains the number of paramters that are
 *                    arrays with unknown shape.
 *                  - formal parameters of external functions
 *                    (FUNDEF_MOD == NULL) will not be checked
 *                    (only result_type)
 */

static int
CheckFunctionDeclaration (node *arg_node, int all)
{
    int ret_value = 0;
    types *type;
    node *arg;

    DBUG_ENTER ("CheckFunctionDeclaration");

    DBUG_ASSERT ((N_fundef == NODE_TYPE (arg_node)), "wrong node");

    /* check whether there is a userdefined type with same name as function */
    CHECK_FUNCTION_TYPENAMECLASH (arg_node);

    type = FUNDEF_TYPES (arg_node);
    while ((NULL != type) && (1 == all)) {
        if (TYPES_DIM (type) < 0) {
            /*
             * return type without exact shape specification
             */
            ret_value++;
        }
        type = TYPES_NEXT (type);
    }

    if (NULL != FUNDEF_ARGS (arg_node)) {
        int i = 1;
        int check_names = 0;
        int j;
        node *tmp_node;

        arg = FUNDEF_ARGS (arg_node);
        if (NULL != ARG_NAME (arg))
            check_names = 1;

        while (NULL != arg) {
            if (T_dots != ARG_BASETYPE (arg)) {
                if (1 == check_names) {
                    /* check whether there is a userdefined type with equal name */
                    CHECK_TYPENAMECLASH (arg);

                    /* now check whether there are formal parameters with equal name */
                    tmp_node = ARG_NEXT (arg);
                    j = i + 1;

                    while (NULL != tmp_node) {
                        if (T_dots != ARG_BASETYPE (tmp_node)) {
                            if (0 == strcmp (ARG_NAME (tmp_node), ARG_NAME (arg)))
                                ERROR (NODE_LINE (arg_node),
                                       ("%d. and %d. argument of function "
                                        "'%s` have same name '%s`",
                                        i, j,
                                        ModName (FUNDEF_MOD (arg_node),
                                                 FUNDEF_NAME (arg_node)),
                                        ARG_NAME (arg)));
                        }
                        j += 1;
                        tmp_node = ARG_NEXT (tmp_node);
                    }
                }
            }
            arg = ARG_NEXT (arg);
            i += 1;
        }
    }
    DBUG_PRINT ("CHECK", ("return: %d", ret_value));
    DBUG_RETURN (ret_value);
}

/*
 *
 *  functionname  : CmpFunParams
 *  arguments     : 1) N_arg node of one function
 *                  2) N_arg node of another function
 *  description   : checks whether the functions have equal domain
 *                  returns 1 if domain is equal, 0 else
 *
 */

static int
CmpFunParams (node *arg1, node *arg2)
{
    int i, is_equal;

    DBUG_ENTER ("CmpFunParams");

    while ((NULL != arg1) && (NULL != arg2)) {
        if (TYPES_BASETYPE (ARG_TYPE (arg1)) == TYPES_BASETYPE (ARG_TYPE (arg2))) {
            if ((TYPES_BASETYPE (ARG_TYPE (arg1)) == T_user)
                || (TYPES_BASETYPE (ARG_TYPE (arg2)) == T_hidden)) {
                is_equal = CMP_TYPE_NAME (ARG_TYPE (arg1), ARG_TYPE (arg2));
                if (0 == is_equal)
                    break;
            }
            if (arg1->DIM == arg2->DIM) {
                for (i = 0; i < arg1->DIM; i++)
                    if (arg1->SHP[i] != arg2->SHP[i])
                        break;
                if (i != arg1->DIM)
                    break;
                else {
                    arg1 = arg1->node[0];
                    arg2 = arg2->node[0];
                }
            } else
                break;
        } else
            break;
    }

    is_equal = ((arg1 == NULL) && (arg2 == NULL));

    DBUG_RETURN (is_equal);
}

/*
 *
 *  functionname  : CheckKnownTypes
 *  arguments     : 1) N_fundef node
 *  description   : - checks whether the function uses known types in
 *                    returnvalue and arguments
 *                  - if it is a userdefined type the module name where
 *                    the type is defined will be added
 *  global vars   : filename
 *  internal funs : LookupType
 *  external funs : InsertNode
 *
 *  remarks       : fundef node must have a body.
 *
 */

static void
CheckKnownTypes (node *arg_node)
{
    types *fun_type;
    node *arg, *t_node = NULL;

    DBUG_ENTER ("CheckKnownTypes");

    /* first check whether type of function is known */
    fun_type = FUNDEF_TYPES (arg_node);
    while (NULL != fun_type) {
        if (T_user == TYPES_BASETYPE (fun_type)) {
            node *t_node
              = LookupType (fun_type->name, fun_type->name_mod, NODE_LINE (arg_node));

            if (NULL == t_node) {
                ERROR (NODE_LINE (arg_node),
                       ("Function '%s` has"
                        " unknown type '%s` in declaration",
                        ModName (FUNDEF_MOD (arg_node), FUNDEF_NAME (arg_node)),
                        ModName (fun_type->name_mod, fun_type->name)));
            } else {
                TYPES_TDEF (fun_type) = t_node;
                fun_type->name_mod = TYPEDEF_MOD (t_node);

                if (FUNDEF_INLINE (arg_node)) {
                    StoreNeededNode (t_node, arg_node, ST_artificial);
                }

                /*
                 *  A reference to the typedef node is added to this function's
                 *  list of needed types if it's an inline function.
                 *  This information is used in analysis.c and sib.c
                 */
            }
        }
        fun_type = fun_type->next;
    }

    /* now check whether the formal arguments have known type */
    arg = FUNDEF_ARGS (arg_node);
    while (NULL != arg) {
        if (T_user == TYPES_BASETYPE (ARG_TYPE (arg))) {
            t_node = LookupType (arg->NAME, ARG_TMOD (arg), NODE_LINE (arg_node));

            if (NULL == t_node) {
                ERROR (NODE_LINE (arg_node),
                       ("Formal parameter '%s` of function '%s` "
                        "has unknown type '%s`",
                        ARG_NAME (arg),
                        ModName (FUNDEF_MOD (arg_node), FUNDEF_NAME (arg_node)),
                        ModName (ARG_TMOD (arg), arg->NAME)));
            } else {
                TYPES_TDEF (ARG_TYPE (arg)) = t_node;
                ARG_TMOD (arg) = TYPEDEF_MOD (t_node);

                if ((TYPEDEF_ATTRIB (t_node) == ST_unique)
                    && (ARG_ATTRIB (arg) == ST_regular)) {
                    ARG_ATTRIB (arg) = ST_unique;
                }

                if (FUNDEF_INLINE (arg_node)) {
                    StoreNeededNode (t_node, arg_node, ST_artificial);
                }

                /*
                 *  A reference to the typedef node is added to this function's
                 *  list of needed types if it's an inline function.
                 *  This information is used in analysis.c and sib.c
                 */
            }
        }

        if ((ARG_ATTRIB (arg) == ST_reference)
            || (ARG_ATTRIB (arg) == ST_readonly_reference)) {
            if (t_node == NULL) {
                ERROR (NODE_LINE (arg_node),
                       ("'%s` may not be a reference"
                        " parameter, because it's not of an object of a class",
                        ARG_NAME (arg_node)));
            } else if (TYPEDEF_ATTRIB (t_node) != ST_unique) {
                ERROR (NODE_LINE (arg_node),
                       ("'%s` may not be a reference"
                        " parameter, because it's not of an object of a class",
                        ARG_NAME (arg_node)));
            }
        }

        if ((!KNOWN_SHAPE (ARG_DIM (arg))) && KNOWN_DIMENSION (ARG_DIM (arg))
            && (FUNDEF_ATTRIB (arg_node) != ST_dim_indep)) {
            FUNDEF_ATTRIB (arg_node) = ST_shp_indep;
            DBUG_PRINT ("FUN_TAG",
                        ("Set ATTRIB-tag of function '%s' to %d",
                         ModName (FUNDEF_MOD (arg_node), FUNDEF_NAME (arg_node)),
                         mdb_statustype[FUNDEF_ATTRIB (arg_node)]));
        } else if (!KNOWN_DIMENSION (ARG_DIM (arg))) {
            FUNDEF_ATTRIB (arg_node) = ST_dim_indep;
            DBUG_PRINT ("FUN_TAG",
                        ("Set ATTRIB-tag of function '%s' to %d",
                         ModName (FUNDEF_MOD (arg_node), FUNDEF_NAME (arg_node)),
                         mdb_statustype[FUNDEF_ATTRIB (arg_node)]));
        }
        arg = arg->node[0];
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : AllChecked
 *  arguments     : ----
 *  description   : looks whether all functions have tag unequal TMP_CHECKED
 *                  returns 1 if all functions have tag unequal TMP_CHECKED
 *                          0 otherwise
 *
 *  global vars   : fun_table
 *  internal funs : ----
 *  external funs : ----
 *  macros        : DBUG..., TMP_CHECKED
 *
 *  remarks       : ----
 *
 *
 */

static int
AllChecked ()
{
    fun_tab_elem *tmp;
    int ok = 1;
    DBUG_ENTER ("AllChecked");

    for (tmp = fun_table; !END_OF_FUN_TAB (tmp); tmp = NEXT_FUN_TAB_ELEM (tmp))
        if ((TMP_CHECKED == tmp->tag) || (PLEASE_CHECK == tmp->tag))
            ok = 0;

    DBUG_RETURN (ok);
}

/*
 *  functionname  : DeleteFun
 *  arguments     : 1) N_fundef to delete
 *                  2) modul of the syntax_tree
 *  description   : deletes 1) from 2)
 *                  returns 2) without 1)
 *  global vars   : ----
 *  internal funs : ----
 *  external funs : FreeTree
 *  macros        : DBUG...,
 *
 *  remarks       : ----
 *
 */

static node *
DeleteFun (node *arg_node, node *modul)
{
    node *fun_node, *last_node, *tree;
    int i;

    DBUG_ENTER ("DeleteFun");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), "DeleteFun with non-N_fundef node!");

    tree = MODUL_FUNS (modul);
    last_node = tree;

    for (i = 0; i < 2; i++) {
        if (last_node == arg_node) {
            tree = FUNDEF_NEXT (tree);
            FUNDEF_NEXT (last_node) = NULL;
            DBUG_PRINT ("TYPE", ("removed function %s", FUNDEF_NAME (last_node)));
            FreeTree (last_node);
            /* node already found */
            if (i == 0) {
                MODUL_FUNS (modul) = tree;
            } else {
                MODUL_FUNDECS (modul) = tree;
            }
            break;

        } else {
            fun_node = FUNDEF_NEXT (tree);
            while ((fun_node != arg_node) && (fun_node != NULL)) {
                last_node = fun_node;
                fun_node = FUNDEF_NEXT (fun_node);
            }

            if (fun_node == arg_node) {
                FUNDEF_NEXT (last_node) = FUNDEF_NEXT (fun_node);
                FUNDEF_NEXT (fun_node) = NULL;
                DBUG_PRINT ("TYPE", ("removed function %s", FUNDEF_NAME (fun_node)));
                FreeTree (fun_node);
                /* node already found */
                break;
            }
            tree = MODUL_FUNDECS (modul);
            last_node = tree;
        }
    }

    DBUG_RETURN (MODUL_FUNS (modul));
}

/*
 *
 *  functionname  : CheckRest
 *  arguments     : 1) first N_fundef of the syntax_tree
 *                  2) tag: 0: check a sac-program (SAC_PRG)
 *                          1: check a sac-module (implementation)(SAC_MOD)
 *                  3) current modul node, needed in DeleteFun
 *  description   : looks whether all functions are checked
 *                  if one function is checked "fast" then check it again ,but
 *                  normally
 *
 *  global vars   : stack, filename
 *  internal funs : TCfundef, AllChecked, DeleteFun
 *  external funs : ----
 *  macros        : DBUG...,NOT_CHECKED, IS_CHECKED,TMP_CHECKED, CHECKING,
 *                  FAST_CHECK, ERROR
 *
 *  remarks       : if one function is not used it will be removed form the
 *                  syntax_tree 1)
 *
 *
 */

static node *
CheckRest (node *arg_node, int kind, node *modul)
{
    fun_tab_elem *fun_p;

    DBUG_ENTER ("CheckRest");
#ifndef DBUG_OFF
    DBUG_PRINT ("CHECK", ("tags of local functions after first run "));
    for (fun_p = fun_table; !END_OF_FUN_TAB (fun_p); fun_p = NEXT_FUN_TAB_ELEM (fun_p))
        if (NULL != fun_p->node->node[0]) {
            DBUG_PRINT ("CHECK",
                        ("function %s has tag :%s", fun_p->id, CHECK_NAME (fun_p->tag)));
        }
    DBUG_PRINT ("CHECK", (""));
#endif

    while (!AllChecked ()) {
        for (fun_p = fun_table; !END_OF_FUN_TAB (fun_p);
             fun_p = NEXT_FUN_TAB_ELEM (fun_p)) {
            if (TMP_CHECKED == fun_p->tag) {
                fun_p->tag = CHECKING;
                TCfundef (fun_p->node, NULL);
            } else {
                if (PLEASE_CHECK == fun_p->tag) {
                    if (FUNDEF_BODY (fun_p->node) == NULL) {
                        /*
                         * For pure function declarations, the typecheck is trivial.
                         */
                        fun_p->tag = IS_CHECKED;
                    } else {
                        fun_p->tag = FAST_CHECK;
                        TCfundef (fun_p->node, NULL);
                        if (PLEASE_CHECK == fun_p->tag) {
                            fun_p->tag = TMP_CHECKED;
                        }
                    }
                }
            }
        }
    }

    DBUG_PRINT ("CHECK", ("tags of functions at the end of typechecking"));
    for (fun_p = fun_table; !END_OF_FUN_TAB (fun_p); fun_p = NEXT_FUN_TAB_ELEM (fun_p)) {
        DBUG_PRINT ("CHECK", ("Tags of function '%s%': STATUS: %d, ATTRIB: %d",
                              ModName (fun_p->id_mod, fun_p->id),
                              FUNDEF_STATUS (fun_p->node), FUNDEF_ATTRIB (fun_p->node)));
        switch (fun_p->tag) {
        case NOT_CHECKED: {
            if ((fun_p->n_dub == max_overload)
                && (FUNDEF_STATUS (fun_p->node) != ST_imported_mod)
                && (FUNDEF_STATUS (fun_p->node) != ST_imported_class)
                && (FUNDEF_STATUS (fun_p->node) != ST_objinitfun)
                && (FUNDEF_STATUS (fun_p->node) != ST_foldfun)) {
                WARN (NODE_LINE (fun_p->node),
                      ("Function '%s` is neither used nor has been specialized",
                       ModName (fun_p->id_mod, fun_p->id)));
            }

            if (FUNDEF_STATUS (fun_p->node) != ST_objinitfun) {
                arg_node = DeleteFun (fun_p->node, modul);
            }

            DBUG_PRINT ("CHECK", ("function %s is not used", fun_p->id));
            break;
        }
        case IS_CHECKED:
            DBUG_PRINT ("CHECK", ("function %s is checked", fun_p->id));
            break;
        case TMP_CHECKED: {
            DBUG_PRINT ("CHECK", ("function %s is not checked completely", fun_p->id));
            if (SAC_PRG == kind) {
                ERROR (NODE_LINE (fun_p->node), ("Function '%s` cannot be typechecked",
                                                 ModName (fun_p->id_mod, fun_p->id)));
            } else {
                WARN (NODE_LINE (fun_p->node),
                      ("Function '%s` cannot be typechecked "
                       "because of unknown shapes in declaration. "
                       "Function will be removed",
                       ModName (fun_p->id_mod, fun_p->id)));
                arg_node = DeleteFun (fun_p->node, modul);
            }
            break;
        }
        case CHECKING:
            DBUG_PRINT ("CHECK", ("function %s is not checked (normal)", fun_p->id));
        case PLEASE_CHECK:
            DBUG_PRINT ("CHECK", ("function %s can not be checked", fun_p->id));

        case FAST_CHECK: {
            DBUG_PRINT ("CHECK", ("function %s is not checked (fast)", fun_p->id));

            if (SAC_PRG == kind) {
                ERROR (NODE_LINE (fun_p->node), ("Function '%s` is not typechecked",
                                                 ModName (fun_p->id_mod, fun_p->id)));
            } else {
                WARN (NODE_LINE (fun_p->node), ("Function '%s` cannot be"
                                                " typechecked and will be removed",
                                                ModName (fun_p->id_mod, fun_p->id)));
                arg_node = DeleteFun (fun_p->node, modul);
            }
            break;
        }
        default:
            DBUG_PRINT ("CHECK",
                        ("function %s has wrong tag (%d)", fun_p->id, fun_p->tag));
            DBUG_ASSERT (0, "function has wrong tag");
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IsNameInMods
 *  arguments     : 1) name looking for
 *                  2) mods looking in
 *  description   : looks whether 'name' is in 'mods'
 *                  returns 0, if name was not found in 'mods'
 *                          1, if name was found in 'mods'
 *  global vars   : module_name
 *  internal funs :
 *  external funs :  strcmp
 *  macros        : DBUG..., NULL
 *  remarks       :
 *
 *
 */

static int
IsNameInMods (char *name, mods *mods)
{
    int found = 0;

    DBUG_ENTER ("IsNameInMods");

    if (NULL != name) {
        DBUG_PRINT ("TYPE", ("looking for module %s", name));
        if (0 == strcmp (name, (NULL != module_name ? module_name : ""))) {
            found = 1;
            DBUG_PRINT ("TYPE", ("module-name found "));
        } else if (NULL != name) {
            while (NULL != mods) {
                DBUG_PRINT ("TYPE", ("current module-name: %s", mods->mod->name));
                if ((!strcmp (name, mods->mod->name))) {
                    found = 1;
                    DBUG_PRINT ("TYPE", ("module-name found "));
                    break;
                } else {
                    mods = mods->next;
                }
            }
        }
    }
    DBUG_RETURN (found);
}

/*
 *
 *  functionname  : InitTypeTab
 *  arguments     : 1) N_modul node
 *  description   : generates a table for userdefined types
 *  global vars   : type_tab_size, filename
 *
 *  remarks       : a new N_typedef chain is made too (new order)
 *
 */

static type_tab_elem *
InitTypeTab (node *modul_node)
{
    node *tmp, *last_node;
    type_tab_elem *tab;
    int i, j, k;
    char *name, *mod_name;

    DBUG_ENTER ("InitTypeTab");

    tmp = MODUL_TYPES (modul_node); /*set tmp to first N_typedef node*/
    DBUG_ASSERT ((NULL != tmp), "typedef_node is NULL");
    DBUG_ASSERT ((N_typedef == tmp->nodetype), "wrong node");

    for (type_tab_size = 0; NULL != tmp; type_tab_size++)
        tmp = TYPEDEF_NEXT (tmp);

    tab = (type_tab_elem *)Malloc (sizeof (type_tab_elem) * type_tab_size);

    last_node = modul_node;
    tmp = MODUL_TYPES (modul_node); /* first N_typedef node */
    /* first insert all typedefs that are constructed from primitive types */
    i = 0;
    while (i < type_tab_size)
        if (T_user == TYPES_BASETYPE (TYPEDEF_TYPE (tmp))) {
            if (NULL == TYPEDEF_NEXT (tmp)) {
                break;
            } else {
                last_node = tmp;
                tmp = TYPEDEF_NEXT (tmp);
            }
        } else {
            /* insert node into type tabel (tab) and remove it from chain */
            if (-1 != TYPEDEF_DIM (tmp)) {
                name = TYPEDEF_NAME (tmp);
                mod_name = TYPEDEF_MOD (tmp);
                /* look whether current type is defined more than once */
                for (k = 0; k < i; k++)
#ifdef OLD_TEST
                    if (!strcmp (name, T_NAME (k))
                        && (((NULL == mod_name) || (NULL == T_MOD (k)))
                              ? (mod_name == T_MOD (k))
                              : (!strcmp (mod_name, T_MOD (k)))))
#else
                    if (!strcmp (name, T_NAME (k)) && CMP_TYPE_MOD (mod_name, T_MOD (k)))
#endif /* OLD_TEST */
                    {
                        if (NULL == TYPEDEF_MOD (tmp)) {
                            ABORT (NODE_LINE (tmp),
                                   ("Type '%s` is defined more than once",
                                    TYPEDEF_NAME (tmp)));
                        } else {
                            ABORT (NODE_LINE (tmp),
                                   ("Type '%s` is defined more than"
                                    " once in module/class '%s`",
                                    TYPEDEF_NAME (tmp), TYPEDEF_MOD (tmp)));
                        }
                    }

                (tab + i)->node = tmp;
                DBUG_PRINT ("TYPE", ("inserted %d: %s%s" F_PTR ", " F_PTR, i,
                                     (NULL == TYPEDEF_MOD (tmp)) ? "" : TYPEDEF_MOD (tmp),
                                     TYPEDEF_NAME (tmp), TYPEDEF_TYPE (tmp), tmp));
                i++;
                if (NULL == TYPEDEF_NEXT (tmp)) {
                    if (N_modul == last_node->nodetype) {
                        last_node->node[1] = NULL;
                    } else {
                        last_node->node[0] = NULL;
                    }
                    break;
                } else {
                    tmp = tmp->node[0];
                    if (N_modul == last_node->nodetype) {
                        last_node->node[1] = tmp;
                    } else {
                        DBUG_ASSERT ((N_typedef == last_node->nodetype), "wrong node");
                        last_node->node[0] = tmp;
                    }
                }
            } else {
                ABORT (NODE_LINE (tmp), ("User-defined type '%s` is built of"
                                         " type with unknown shape '%s`",
                                         ModName (TYPEDEF_MOD (tmp), TYPEDEF_NAME (tmp)),
                                         Type2String (TYPEDEF_TYPE (tmp), 0, TRUE)));
            }
        }

    if (i != type_tab_size) {
        /* now insert typedefs that are constructed form userdefined types */

        int k, defined, delete = 0, diff = type_tab_size - i;
        mods *mods[2];

        last_node = modul_node;
        tmp = MODUL_TYPES (modul_node);

        while (i < type_tab_size) {
            name = TYPES_NAME (TYPEDEF_TYPE (tmp));
            /* name of type current type is based on*/
            mod_name = TYPES_MOD (TYPEDEF_TYPE (tmp));
            /* dito, but name of module */
            if (NULL != mod_name) {
                /* now look for the module where the type is defined */
                defined = 0;
                for (k = 0; k < 2; k++) {
                    mods[k] = FindSymbolInModul (mod_name, name, k, NULL, 1);
                    if (NULL != mods[k]) {
                        if (NULL != mods[k]->next) {
                            defined += 2;
                            break;
                        } else
                            defined += 1;
                    }
                }

                if (0 == defined)
                    ABORT (NODE_LINE (tmp), ("Type '%s` is not defined or"
                                             " imported in module/class '%s`",
                                             name, mod_name))
                else if (2 <= defined)
                    ABORT (NODE_LINE (tmp), ("Type '%s` is defined or imported "
                                             "more than once in module/class '%s`",
                                             name, mod_name))

                DBUG_ASSERT ((1 == defined), "defined != 1");
                if (NULL == mods[0])
                    mod_name = mods[1]->mod->name;
                else
                    mod_name = mods[0]->mod->name;
            }

            /* look for based on type */
            for (j = 0; j < i; j++)
#ifdef OLD_TEST
                if (!strcmp (name, T_NAME (j))
                    && (((NULL == mod_name) || (NULL == T_MOD (j)))
                          ? (mod_name == T_MOD (j))
                          : (!strcmp (mod_name, T_MOD (j)))))
#else
                if (!strcmp (name, T_NAME (j)) && CMP_TYPE_MOD (mod_name, T_MOD (j)))
#endif /* OLD_TEST */
                {
                    if (-1 != TYPEDEF_TYPE (tmp)->dim) {
                        /* look whether current type is defined more than once */
                        for (k = 0; k < i; k++)
#ifdef OLD_TEST
                            if (!strcmp (TYPEDEF_NAME (tmp), T_NAME (k))
                                && (((NULL == TYPEDEF_MOD (tmp)) || (NULL == T_MOD (k)))
                                      ? (TYPEDEF_MOD (tmp) == T_MOD (k))
                                      : (!strcmp (TYPEDEF_MOD (tmp), T_MOD (k)))))
#else
                            if (!strcmp (TYPEDEF_NAME (tmp), T_NAME (k))
                                && CMP_TYPE_MOD (TYPEDEF_MOD (tmp), T_MOD (k)))
#endif /* OLD_TEST */
                            {
                                if (NULL == TYPEDEF_MOD (tmp)) {
                                    ABORT (NODE_LINE (tmp),
                                           ("Type '%s` is "
                                            "defined more than once",
                                            ModName (TYPEDEF_MOD (tmp),
                                                     TYPEDEF_NAME (tmp))));
                                } else {
                                    ABORT (NODE_LINE (tmp),
                                           ("Type '%s' is defined more "
                                            "than once in module/class '%s`",
                                            TYPEDEF_NAME (tmp), TYPEDEF_MOD (tmp)));
                                }
                            }

                        /* insert type to 'tab' and reduce it to primitive type */
                        (tab + i)->node = tmp;
                        switch (TYPES_BASETYPE (T_TYPE (j))) {
                        case T_hidden:
                            TYPES_BASETYPE (T_TYPE (i)) = T_hidden;
                            if (NULL != T_TYPE (j)->name)
                                T_TYPE (i)->name = StringCopy (T_TYPE (j)->name);
                            if (NULL != T_TYPE (j)->name_mod)
                                T_TYPE (i)->name_mod = StringCopy (T_TYPE (j)->name_mod);
                            break;
                        case T_int:
                        case T_bool:
                        case T_float:
                        case T_str:
                            if (0 <= T_TYPE (j)->dim) {
                                TYPES_BASETYPE (T_TYPE (i)) = TYPES_BASETYPE (T_TYPE (j));
                                T_TYPE (i)->name = NULL;
                                T_TYPE (i)->name_mod = NULL;
                                if ((0 < T_TYPE (j)->dim) && (0 == T_TYPE (i)->dim)) {
                                    T_TYPE (i)->shpseg = MakeShpseg (NULL);
                                }

                                for (k = 0; k < T_TYPE (j)->dim; k++) {
                                    T_TYPE (i)->dim += 1;
                                    DBUG_ASSERT ((T_TYPE (i)->dim <= SHP_SEG_SIZE),
                                                 "shape out off range ");
                                    T_TYPE (i)->shpseg->shp[T_TYPE (i)->dim - 1]
                                      = T_TYPE (j)->shpseg->shp[k];
                                }
                            } else {
                                ABORT (NODE_LINE (tmp),
                                       ("User-defined type '%s` is built from type "
                                        "with unknown shape (%s)",
                                        TYPEDEF_NAME (tmp),
                                        Type2String (TYPEDEF_TYPE (tmp), 0, TRUE)));
                            }
                            break;
                        default: {
                            DBUG_ASSERT (0, "error in type table");
                            break;
                        }
                        }
                        DBUG_PRINT ("TYPE",
                                    ("inserted %d: %s%s" F_PTR ", " F_PTR, i,
                                     (NULL == TYPEDEF_MOD (tmp)) ? "" : TYPEDEF_MOD (tmp),
                                     TYPEDEF_NAME (tmp), TYPEDEF_TYPE (tmp), tmp));
                        i++;

                        /* now remove inserted N_typedef(tmp) form typedef-chain
                         */
                        delete = 1;
                        break; /* break for-loop */
                    } else {
                        ABORT (NODE_LINE (tmp),
                               ("User-defined type '%s` is "
                                "built from user-defined type "
                                "with unknown shape (%s)",
                                TYPEDEF_NAME (tmp),
                                Type2String (TYPEDEF_TYPE (tmp), 0, TRUE)));
                    }
                }

            if (NULL == tmp->node[0]) {
                if ((type_tab_size - i) < diff) {
                    diff = type_tab_size - i;
                    if (1 == delete) {
                        if (N_modul == last_node->nodetype) {
                            last_node->node[1] = NULL;
                            break;
                        } else {
                            last_node->node[0] = NULL;
                        }
                        delete = 0;
                    }
                    last_node = modul_node;
                    tmp = modul_node->node[1];
                } else {
                    break; /*break while loop */
                }
            } else {
                if (1 == delete) {
                    if (N_modul == last_node->nodetype) {
                        last_node->node[1] = tmp->node[0];
                    } else {
                        last_node->node[0] = tmp->node[0];
                    }
                    tmp = tmp->node[0];
                    delete = 0;
                } else {
                    last_node = tmp;
                    tmp = tmp->node[0];
                }
            }
        } /* end of while loop */

        /* now check whether all entries are done */
        if (i < type_tab_size) {
            tmp = modul_node->node[1];
            while (TYPEDEF_TYPE (tmp) != NULL) {
                ABORT (NODE_LINE (tmp),
                       ("User-defined type '%s` is built of "
                        "unknown type (%s)",
                        TYPEDEF_NAME (tmp), Type2String (TYPEDEF_TYPE (tmp), 0, TRUE)));
                tmp = tmp->node[0];
            }
            if (NULL != tmp) {
                ABORT (NODE_LINE (tmp),
                       ("User-defined type '%s` is built of "
                        "unknown type (%s)",
                        TYPEDEF_NAME (tmp), Type2String (TYPEDEF_TYPE (tmp), 0, TRUE)));
            } else
                exit (3);
        }
        DBUG_ASSERT ((i == type_tab_size), " type tabel is not filled completly");
    }

    /* now make a correct N_typedef chain */
    modul_node->node[1] = tab->node;
    if (1 < type_tab_size) {
        tmp = tab->node;
        tmp->node[0] = (tab + 1)->node;
        for (i = 2; i < type_tab_size; i++) {
            tmp = tmp->node[0];
            tmp->node[0] = tab[i].node;
        }
        tmp->node[0]->node[0] = NULL;
    }
    /* now add id_count */
    for (i = 0; i < type_tab_size; i++) {
        name = TYPEDEF_NAME ((tab + i)->node);
        (tab + i)->id_count = 0;
        for (k = 0; k < type_tab_size; k++)
            if (!strcmp (TYPEDEF_NAME ((tab + k)->node), name))
                (tab + k)->id_count++;
    }

    DBUG_RETURN (tab);
}

/*
 *
 *  functionname  : InitFunTable
 *  arguments     : 1) pointer to first FUNS declaration (N_fundef)
 *                : 2) pointer to first FUNDECS declaration (N_fundef)
 *  description   : generates a table of userdefined functions
 *                  - checks whether types of parameters are ok
 *                       - extern functions do not contain arrays with
 *                         unknown shape
 *                       - all types are known
 *                  - puts all function with equal name in a row
 *                  - inserts "checking" informations
 *                  - tests whether definitions of functions are definit
 *                  - set global variable fun_table
 *
 *  global vars   : fun_table, filename
 *  internal funs : Malloc, CheckFunctionDeclaration, LookupFun, CmpFunParam
 *  external funs : sizeof
 *  macros        : DBUG..., ERROR, TYPES, IS_CHECKED, NOT_CHECKED,
 *                  MOD_NAME_CON, INSERT_FUN, NULL
 *  remarks       : ----
 *
 */

static void
InitFunTable (node *funs, node *fundecs)
{
    node *fun_node;
    fun_tab_elem *fun_p, *last_fun_p, *dummy;
    char *fun_name, *mod_name;
    int counter;

    DBUG_ENTER ("InitFunTable");

    /*  DBUG_ASSERT((N_fundef == funs->nodetype)," funs not N_fundef");
      DBUG_ASSERT((N_fundef == fundecs->nodetype)," fundecs not N_fundef");
     */
    /* first count fundefs and check whether types of external functions are ok*/
    dummy = NEW_FUN_TAB_ELEM; /* this is only a dummy entry and will
                               * be removed later on.
                               */
    fun_table = dummy;
    INIT_FUN_TAB_ELEM (fun_table);
    fun_node = funs;

    /* execute inner loops 2 times
     * first time for funs, second time for fundecs
     */
    for (counter = 0; counter < 2; counter++) {

        while (NULL != fun_node) {
            if (NULL == fun_node->node[0]) {
                node *tmp_fun_node = fun_node->node[1];
                char *tmp_fun_name;
                int equal_name = 0;

                /* first check whether the types of the external function declaration
                 * are ok
                 */
                CheckFunctionDeclaration (fun_node, 1);
                /* check whether there are more external functions with name for
                 * extern function (node[5])
                 */
                /* fun_name=(NULL == fun_node->node[5]) ? FUNDEF_NAME( fun_node)
                                                        : (char*)fun_node->node[5]; */
                fun_name = (NULL == FUNDEF_PRAGMA (fun_node)
                              ? FUNDEF_NAME (fun_node)
                              : (NULL == FUNDEF_LINKNAME (fun_node)
                                   ? FUNDEF_NAME (fun_node)
                                   : FUNDEF_LINKNAME (fun_node)));

                while (NULL != tmp_fun_node) {
                    if (NULL == tmp_fun_node->node[0]) {
                        if (NULL == FUNDEF_MOD (tmp_fun_node)) {
                            /* tmp_fun_name=(NULL != tmp_fun_node->node[5])
                               ?(char*)(tmp_fun_node->node[5]) : FUNDEF_NAME(
                               tmp_fun_node);
                             */
                            tmp_fun_name = (NULL == FUNDEF_PRAGMA (tmp_fun_node)
                                              ? FUNDEF_NAME (tmp_fun_node)
                                              : (NULL == FUNDEF_LINKNAME (tmp_fun_node)
                                                   ? FUNDEF_NAME (tmp_fun_node)
                                                   : FUNDEF_LINKNAME (tmp_fun_node)));

                            DBUG_PRINT ("CHECK", ("compare funname  %s  with %s",
                                                  fun_name, tmp_fun_name));

                            if (0 == strcmp (fun_name, tmp_fun_name))
                                equal_name += 1;
                        }
                        tmp_fun_node = FUNDEF_NEXT (tmp_fun_node);
                    } else
                        /* it is a local defined function, so stop checking */
                        break;
                }
                if (0 != equal_name) {
                    /*
                      WARN(0,("More than one external function with name '%s`",
                      fun_name));
                    */
                    /*
                     * This is allowed due to overloading external functions
                     * using the linkname pragma.
                     */
                }
            } else
                CheckKnownTypes (fun_node);

            /* now insert function into fun_table */
            fun_name = FUNDEF_NAME (fun_node);
            mod_name = (NULL == FUNDEF_MOD (fun_node)) ? (FUNDEF_LINKMOD (fun_node))
                                                       : (FUNDEF_MOD (fun_node));
            fun_p = LookupFun (fun_name, NULL, NULL);

            if (NULL == fun_p) {
                /*         INSERT_FUN(fun_table, fun_name, mod_name, fun_node,
                           (NULL == fun_node->node[0])?IS_CHECKED:NOT_CHECKED,
                           max_overload);*/
                INSERT_FUN (fun_table, fun_node, max_overload);
            } else {
                do {
                    last_fun_p = fun_p;
                    if (CMP_MOD (fun_p->id_mod, FUNDEF_MOD (fun_node))) {
                        if (1 == CmpFunParams (fun_p->node->node[2], fun_node->node[2])) {
                            ERROR (0, ("Function '%s` is defined more"
                                       " than once with equal domain",
                                       ModName (fun_p->id_mod, fun_p->id)));
                            break;
                        }
                    }
                    fun_p = NEXT_FUN_TAB_ELEM (fun_p);
                } while (END_OF_FUN_TAB (fun_p)
                           ? 0
                           : (!strcmp (fun_p->id, FUNDEF_NAME (fun_node))));
                /*         INSERT_FUN(last_fun_p, fun_name, mod_name, fun_node,
                           (NULL == fun_node->node[0])?IS_CHECKED:NOT_CHECKED,
                           max_overload);*/
                INSERT_FUN (last_fun_p, fun_node, max_overload);
            }
            fun_node = fun_node->node[1];
        }
        fun_node = fundecs;
    }
    ABORT_ON_ERROR;

    fun_table = NEXT_FUN_TAB_ELEM (fun_table);
    dummy = Free (dummy);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *Typecheck( node *arg_node)
 *
 * Description:
 *   Initializes and starts typechecking of a SAC programm
 *     - first off all extends fundefs by specializations
 *     - initializes table of primitive functions
 *     - set stack (scope stack)
 *     - initializes table of userdefined types
 *     - initializes table of userdefined functions
 *     - calls Trav() to start checking the main function
 *     - sets stack_limit (end of scope stack)
 *
 ******************************************************************************/

node *
Typecheck (node *arg_node)
{
    fun_tab_elem *tmp_node;
    node *tmp;

    DBUG_ENTER ("Typecheck");

    /*
     * dkr: Should be a N_modul-node, right??
     */
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_modul),
                 "Typecheck() not called with N_modul node!");

    /*
     * if compiling for a c library, search for specializations
     * of functions and integrate them
     *
     */
    if (generatelibrary & GENERATELIBRARY_C) {
        arg_node = ImportSpecialization (arg_node);
    }

    /*
     * generate a table of primitive functions to look up their
     * types later on
     */
    InitPrimFunTab ();

    act_tab = type_tab;

    stack = (stack_elem *)Malloc (sizeof (stack_elem) * LOCAL_STACK_SIZE);
    stack_limit = LOCAL_STACK_SIZE + stack;
    DBUG_PRINT ("TYPE", ("local stack from " F_PTR "to " F_PTR, stack, stack_limit));
    tos = stack;

    pseudo_fold_fundefs = NULL;
    module_name = MODUL_NAME (arg_node);
    object_table = MODUL_OBJS (arg_node);

    if (MODUL_TYPES (arg_node) != NULL) {
        type_table = InitTypeTab (arg_node);
    }

    /*
     *  The following lines start the traversal mechanism for object
     *  definitions. The given types will be looked for in the
     *  type table and name clashes with user-defined types, functions or
     *  other global objects will be detected.
     *
     *  prerequisite:  stack != NULL
     */
    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), NULL);
    }

    ABORT_ON_ERROR;

    if ((MODUL_FUNS (arg_node) != NULL) || (MODUL_FUNDECS (arg_node) != NULL)) {
        InitFunTable (MODUL_FUNS (arg_node), MODUL_FUNDECS (arg_node));

        /*  ABORT_ON_ERROR;  */

        /* traverse main function */
        tmp_node = LookupFun ("main", NULL, NULL);

        if (MODUL_FILETYPE (arg_node) == F_prog) {
            if (NULL == tmp_node) {
                SYSABORT (("SAC program has no function 'main`"));
            }

            kind_of_file = SAC_PRG;
            tmp_node->tag = CHECKING; /* set tag, because function will be checked */
            Trav (tmp_node->node, NULL);

            /* now check the type of "fastchecked" functions */
            MODUL_FUNS (arg_node) = CheckRest (MODUL_FUNS (arg_node), SAC_PRG, arg_node);
            /* MODUL_FUNDECS( arg_node) = CheckRest( MODUL_FUNDECS(arg_node), SAC_PRG,
             * arg_node);*/
        } else {
            fun_tab_elem *fun_p;
            kind_of_file = SAC_MOD;

            if (tmp_node != NULL) {
                ABORT (NODE_LINE (tmp_node->node),
                       ("Function 'main' not allowed in module/class implementations"));
            }

            for (fun_p = fun_table; !END_OF_FUN_TAB (fun_p);
                 fun_p = NEXT_FUN_TAB_ELEM (fun_p)) {
                DBUG_ASSERT ((N_fundef == NODE_TYPE (fun_p->node)),
                             "wrong node in fun_table");
                if ((FUNDEF_BODY (fun_p->node) != NULL)
                    && (FUNDEF_STATUS (fun_p->node) != ST_imported_mod)
                    && (FUNDEF_STATUS (fun_p->node) != ST_imported_class)
                    && (FUNDEF_STATUS (fun_p->node) != ST_foldfun)) {

                    if (!(((generatelibrary & GENERATELIBRARY_C)
                           && ((FUNDEF_ATTRIB (fun_p->node) == ST_shp_indep)
                               || (FUNDEF_ATTRIB (fun_p->node) == ST_dim_indep)
                               || (FUNDEF_ATTRIB (fun_p->node) == ST_generic))))) {
                        /*
                         * When typechecking a module/class implementation, we cannot
                         * simply start with the main function. Aleternatively, each
                         * function that is actually implemented in this module and not
                         * imported from another one, is tagged PLEASE_CHECK and thereby
                         * acts as starting point for typechecking.
                         */
                        fun_p->tag = PLEASE_CHECK;

                        DBUG_PRINT ("CHECK", ("function %s has tag :%s", fun_p->id,
                                              CHECK_NAME (fun_p->tag)));
                    } else {
                        fun_p->tag = IS_CHECKED;
                        FUNDEF_STATUS (fun_p->node) = ST_ignore;
                        /*
                         * generic functions in modules compiled for a c-library must not
                         * be checked. There may be a conflict between specialized and
                         * generic versions the typchechecker cannot resolve. So these
                         * fundefs are tagged as CHECKED and marked to be ignored in the
                         * further compiling steps
                         */
                    }
                }
            }

            MODUL_FUNS (arg_node) = CheckRest (MODUL_FUNS (arg_node), SAC_MOD, arg_node);
            /*      MODUL_FUNDECS( arg_node) = CheckRest( MODUL_FUNDECS( arg_node),
             * SAC_MOD,arg_node);*/
        }

        stack = Free (stack);
    } else {
        if (MODUL_FILETYPE (arg_node) == F_prog) {
            SYSABORT (("SAC program has no function 'main`"));
        }
    }

    /*
     * Prepand pseudo-fold-fundefs to the olf fundefs.
     * This makes sure that these dummy-funs will be compiled before any other
     * function!
     */
    if (pseudo_fold_fundefs != NULL) {

        pseudo_fold_fundefs = TypecheckFunctionDeclarations (pseudo_fold_fundefs);

        DBUG_ASSERT ((pseudo_fold_fundefs != NULL), "pseudo_fold_fundefs is NULL");

        tmp = pseudo_fold_fundefs;
        while (FUNDEF_NEXT (tmp) != NULL) {
            tmp = FUNDEF_NEXT (tmp);
        }
        FUNDEF_NEXT (tmp) = MODUL_FUNS (arg_node);
        MODUL_FUNS (arg_node) = pseudo_fold_fundefs;
    }

    MODUL_FUNDECS (arg_node) = TypecheckFunctionDeclarations (MODUL_FUNDECS (arg_node));

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CmpTypes
 *  arguments     : 1) first type
 *                  2) second type,
 *  description   : compares two type informations
 *
 *  remarks       : return_values
 *                  0 if there is an incompatibility
 *                  1 if types are equal
 *                  2 if 1) & 2) are arrays of equal primitive type,
 *                     1) has unknown shape and 2) has known shape
 *                     or the other way around
 *                  3 equal simpletype, but different dimension
 *                  4 equal simpletype, equal dimension, equal number of
 *                    elements, but different shape
 */

cmp_types
CmpTypes (types *type_one, types *type_two)
{
    int i;
    cmp_types return_value = CMP_equal;
#ifndef DBUG_OFF
    char *db_str1, *db_str2;
#endif

    DBUG_ENTER ("CmpTypes");
    DBUG_ASSERT (((NULL != type_one) && (NULL != type_two)), "one arguments is NULL ");
#ifndef DBUG_OFF
    db_str1 = Type2String (type_one, 0, TRUE);
    db_str2 = Type2String (type_two, 0, TRUE);
    DBUG_PRINT ("TYPE", ("compare '%s' with '%s'", db_str1, db_str2));
    db_str1 = Free (db_str1);
    db_str2 = Free (db_str2);
#endif

    if (TYPES_BASETYPE (type_one) == TYPES_BASETYPE (type_two)) {
        int ok = 1;

        if (T_hidden == TYPES_BASETYPE (type_one)) {
#if 1
            /*
             * ugly hack (dkr):
             * TYPEDEF_NAME is no longer part of the types structure
             *  -> hidden types must already be handled by CompatibleTypes()
             */
            DBUG_ASSERT ((0), "hidden type found!");
#else
            ok = CMP_TYPE_ID (type_one, type_two);
#endif
        } else if (T_user == TYPES_BASETYPE (type_one)) {
            if (!strcmp (type_one->name, type_two->name)) {
                if ((NULL != type_one->name_mod) && (NULL != type_two->name_mod))
                    ok = !(strcmp (type_one->name_mod, type_two->name_mod));
                else if ((NULL == type_one->name_mod) && (NULL == type_two->name_mod))
                    ok = 1;
                else {
                    node *t_node;

                    char *mod_name = (NULL != type_one->name_mod) ? type_one->name_mod
                                                                  : type_two->name_mod;
                    t_node = TYPES_TDEF (type_one);
                    if (t_node == NULL) {
                        t_node = LookupType (type_one->name, NULL, -064);
                    } else {
                        TYPES_TDEF (type_one) = t_node;
                    }

                    ok = !(strcmp (mod_name, (NULL != TYPEDEF_MOD (t_node))
                                               ? TYPEDEF_MOD (t_node)
                                               : ""));
                }
            } else
                ok = 0;
        } else
            ok = 1;

        if (1 == ok) {
            if ((UNKNOWN_SHAPE == type_one->dim) && (SCALAR < type_two->dim))
                return_value = CMP_one_unknown_shape;
            else if ((UNKNOWN_SHAPE == type_two->dim) && (SCALAR < type_one->dim))
                return_value = CMP_one_unknown_shape;
            else if ((SCALAR < type_one->dim) && (SCALAR < type_two->dim)) {
                /*
                 * both have known shape
                 */
                if (type_one->dim == type_two->dim) {
                    for (i = 0; i < type_one->dim; i++) {
                        if (type_one->shpseg->shp[i] != type_two->shpseg->shp[i]) {
                            return_value = CMP_incompatible; /* 4*/
                        }
                    }
                } else {
                    return_value = CMP_incompatible;
                }
            }

            else if (((SCALAR == TYPES_DIM (type_one))
                      && (SCALAR != TYPES_DIM (type_two)))
                     || ((SCALAR == TYPES_DIM (type_two))
                         && (SCALAR != TYPES_DIM (type_one)))
                     || (ARRAY_OR_SCALAR == TYPES_DIM (type_one))
                     || (ARRAY_OR_SCALAR == TYPES_DIM (type_two)))
                return_value = CMP_incompatible;
            else if ((UNKNOWN_SHAPE == TYPES_DIM (type_one))
                     && (UNKNOWN_SHAPE == TYPES_DIM (type_two)))
                /* both have same type but unknown shape */
                return_value = CMP_both_unknown_shape;
            else if ((UNKNOWN_SHAPE == TYPES_DIM (type_one))
                     && (KNOWN_DIM_OFFSET > TYPES_DIM (type_two)))
                return_value = CMP_both_unknown_shape;
            else if ((UNKNOWN_SHAPE == TYPES_DIM (type_two))
                     && (KNOWN_DIM_OFFSET > TYPES_DIM (type_one)))
                return_value = CMP_both_unknown_shape;
            else if ((SCALAR < TYPES_DIM (type_one))
                     && (KNOWN_DIM_OFFSET > TYPES_DIM (type_two))) {
                if ((KNOWN_DIM_OFFSET - TYPES_DIM (type_one) == TYPES_DIM (type_two))) {
                    return_value = CMP_one_unknown_shape;
                } else {
                    return_value = CMP_incompatible;
                }
            } else if ((SCALAR < TYPES_DIM (type_two))
                       && (KNOWN_DIM_OFFSET > TYPES_DIM (type_one))) {
                if ((KNOWN_DIM_OFFSET - TYPES_DIM (type_two) == TYPES_DIM (type_one))) {
                    return_value = CMP_one_unknown_shape;
                } else {
                    return_value = CMP_incompatible;
                }
            } else if (((KNOWN_DIM_OFFSET > TYPES_DIM (type_one))
                        && (KNOWN_DIM_OFFSET > TYPES_DIM (type_two)))) {
                if (TYPES_DIM (type_one) == TYPES_DIM (type_two)) {
                    return_value = CMP_equal;
                } else {
                    return_value = CMP_incompatible;
                }
            } else if ((SCALAR == TYPES_DIM (type_one))
                       && (SCALAR == TYPES_DIM (type_two)))
                return_value = CMP_equal;
            else
                DBUG_ASSERT (0, "wrong dimensions");
        } else
            /* different T_user or T_hidden types */
            return_value = CMP_incompatible;
    } else if (IMPORTED == imported_fun) {
        if (((TYPES_BASETYPE (type_one) == T_hidden)
             && (TYPES_BASETYPE (type_two) != T_hidden))) {

            if (NULL != TYPES_NEXT (type_one))
                return_value = CmpTypes (TYPES_NEXT (type_one), type_two);
            else
                return_value = CMP_incompatible;

        } else if ((TYPES_BASETYPE (type_two) == T_hidden)
                   && (TYPES_BASETYPE (type_one) != T_hidden)) {

            if (NULL != TYPES_NEXT (type_two))
                return_value = CmpTypes (TYPES_NEXT (type_two), type_one);
            else
                return_value = CMP_incompatible;

        } else
            return_value = CMP_incompatible;
    } else
        /* different primitive type */
        return_value = CMP_incompatible;

    DBUG_PRINT ("TYPE", ("return: %s", cmp_types_string[return_value]));

    DBUG_RETURN (return_value);
}

/*
 *
 *  functionname  : CompatibleTypes
 *  arguments     : 1) first type
 *                  2) second type,
 *                  3) tag to consider compatibility of int and float
 *                  4) linenumber
 *  description   : checks whether type_one  is compatible to type_two
 *                  if 3)==-1 then compatibiltiy of int and float is considered
 *                  too
 *  macros        : DBUG..., GEN_NODE, NULL, Free, SHP_SEG_SIZE, TYPES,
 *                  MOD_NAME, ABORT
 *
 */

static cmp_types
CompatibleTypes (types *type_one, types *type_two, int convert_prim_type, int line)
{
    int old_dim1 = 0, old_dim2 = 0, i;
    cmp_types compare;
    node *t_node;
    types *type_1, *type_2;
    shpseg *shpseg_1 = NULL, *shpseg_2 = NULL;
#ifndef DBUG_OFF
    char *db_str1, *db_str2;

    DBUG_ENTER ("CompatibleTypes");

    db_str1 = Type2String (type_one, 0, TRUE);
    db_str2 = Type2String (type_two, 0, TRUE);

    DBUG_PRINT ("TYPE", ("compare %s with %s (convert_prim_type: %d", db_str1, db_str2,
                         convert_prim_type));
    db_str1 = Free (db_str1);
    db_str2 = Free (db_str2);
#endif

    if (T_user == TYPES_BASETYPE (type_one)) {
        t_node = TYPES_TDEF (type_one);
        if (t_node == NULL) {
            t_node = LookupType (type_one->name, type_one->name_mod, line);
        }

        if (NULL == t_node) {
            ABORT (line, ("Type '%s` is unknown",
                          ModName (type_one->name_mod, type_one->name)));
        } else {
            TYPES_TDEF (type_one) = t_node;
            type_1 = DupAllTypes (TYPEDEF_TYPE (t_node));
            if (type_one->dim > 0) {
                if (type_1->dim >= 0) {
                    int dim;

                    old_dim1 = type_1->dim;
                    dim = old_dim1 + type_one->dim;
                    DBUG_ASSERT (dim <= SHP_SEG_SIZE, "shape out off range ");
                    shpseg_1 = type_1->shpseg;
                    type_1->shpseg = MakeShpseg (NULL);
                    for (i = 0; i < type_one->dim; i++)
                        type_1->shpseg->shp[i] = type_one->shpseg->shp[i];
                    for (i = 0; i < old_dim1; i++)
                        type_1->shpseg->shp[i + type_one->dim] = shpseg_1->shp[i];
                    type_1->dim = dim;
                }
            }
        }
    } else
        type_1 = type_one;

    if (T_user == TYPES_BASETYPE (type_two)) {
        t_node = TYPES_TDEF (type_two);
        if (t_node == NULL) {
            t_node = LookupType (type_two->name, type_two->name_mod, line);
        }

        if (NULL == t_node) {
            ABORT (line, ("Type '%s` is unknown",
                          ModName (type_two->name_mod, type_two->name)));
        } else {
            TYPES_TDEF (type_two) = t_node;
            type_2 = DupAllTypes (TYPEDEF_TYPE (t_node));
            if (type_two->dim > 0) {
                if (type_2->dim >= 0) {
                    int i, dim;

                    old_dim2 = type_2->dim;
                    dim = type_2->dim + type_two->dim;

                    DBUG_ASSERT (dim <= SHP_SEG_SIZE, "shape out off range ");
                    shpseg_2 = type_2->shpseg;
                    type_2->shpseg = MakeShpseg (NULL);
                    for (i = 0; i < type_two->dim; i++)
                        type_2->shpseg->shp[i] = type_two->shpseg->shp[i];
                    for (i = 0; i < old_dim2; i++)
                        type_2->shpseg->shp[i + type_two->dim] = shpseg_2->shp[i];
                    type_2->dim = dim;
                }
            }
        }
    } else
        type_2 = type_two;

    DBUG_ASSERT (((T_user != TYPES_BASETYPE (type_1))
                  && (T_user != TYPES_BASETYPE (type_2))),
                 " not primitive types ");

    /* now type_1 and type_2 contain primitive types */

    if ((TYPES_BASETYPE (type_1) == T_dots) || (TYPES_BASETYPE (type_2) == T_dots)) {
        compare = CMP_equal;
        /*
         *  Type '...' is compatible to every other type.
         */
    }
    /*  else if (TYPES_BASETYPE(type_2)==T_nothing)
    {
      compare=CMP_equal;

         Type T_nothing can be casted to everything.

    } */
    else {

        if ((T_nothing == TYPES_BASETYPE (type_2))
            || ((((T_float == TYPES_BASETYPE (type_1))
                  && (T_int == TYPES_BASETYPE (type_2)))
                 || ((T_double == TYPES_BASETYPE (type_1))
                     && (T_int == TYPES_BASETYPE (type_2)))
                 || ((T_double == TYPES_BASETYPE (type_1))
                     && (T_float == TYPES_BASETYPE (type_2))))
                && (-1 == convert_prim_type))) {
            if ((UNKNOWN_SHAPE == type_1->dim) && (UNKNOWN_SHAPE == type_2->dim))
                compare = CMP_both_unknown_shape;
            else if ((UNKNOWN_SHAPE == type_1->dim) || (UNKNOWN_SHAPE == type_2->dim))
                compare = CMP_one_unknown_shape;
            else {
                simpletype s_type = TYPES_BASETYPE (type_2);

                TYPES_BASETYPE (type_2) = TYPES_BASETYPE (type_1);
                compare = CmpTypes (type_1, type_2);
                TYPES_BASETYPE (type_2) = s_type;
            }
        } else if ((UNKNOWN_SHAPE == type_1->dim) && (UNKNOWN_SHAPE == type_2->dim)
                   && (TYPES_BASETYPE (type_1) == TYPES_BASETYPE (type_2))) {
            compare = CMP_both_unknown_shape;
        } else if ((UNKNOWN_SHAPE == type_1->dim) && (SCALAR < type_2->dim)
                   && (TYPES_BASETYPE (type_1) == TYPES_BASETYPE (type_2))) {
            compare = CMP_one_unknown_shape;
        } else if ((UNKNOWN_SHAPE == type_2->dim) && (SCALAR < type_1->dim)
                   && (TYPES_BASETYPE (type_1) == TYPES_BASETYPE (type_2))) {
            compare = CMP_one_unknown_shape;
        }
        /*
         * The 3 following cases represent a quick hack in order to allow
         * int[?] as return type of sel in the module Array from the SAC
         * standard library.
         */
        else if ((ARRAY_OR_SCALAR == type_1->dim) && (ARRAY_OR_SCALAR == type_2->dim)
                 && (TYPES_BASETYPE (type_1) == TYPES_BASETYPE (type_2))) {
            compare = CMP_both_unknown_shape;
        } else if ((ARRAY_OR_SCALAR == type_1->dim) && KNOWN_DIMENSION (type_2->dim)
                   && (dynamic_shapes == 1)
                   && (TYPES_BASETYPE (type_1) == TYPES_BASETYPE (type_2))) {
            compare = CMP_both_unknown_shape;
        } else if ((ARRAY_OR_SCALAR == type_2->dim) && KNOWN_DIMENSION (type_1->dim)
                   && (dynamic_shapes == 1)
                   && (TYPES_BASETYPE (type_1) == TYPES_BASETYPE (type_2))) {
            compare = CMP_both_unknown_shape;
        } else if ((ARRAY_OR_SCALAR == type_1->dim) && KNOWN_SHAPE (type_2->dim)
                   && (TYPES_BASETYPE (type_1) == TYPES_BASETYPE (type_2))) {
            compare = CMP_one_unknown_shape;
        } else if ((ARRAY_OR_SCALAR == type_2->dim) && KNOWN_SHAPE (type_1->dim)
                   && (TYPES_BASETYPE (type_1) == TYPES_BASETYPE (type_2))) {
            compare = CMP_one_unknown_shape;
        } else {
#if 1
            /*
             * ugly hack (dkr):
             * CmpTypes() cannot handle hidden types
             *  -> handle them here
             */
            node *tdef1 = NULL, *tdef2 = NULL;
            tdef1 = TYPES_TDEF (type_one);
            tdef2 = TYPES_TDEF (type_two);
            if ((tdef1 != NULL) && (tdef2 != NULL)
                && ((TYPEDEF_BASETYPE (tdef1) == T_hidden)
                    || (TYPEDEF_BASETYPE (tdef2) == T_hidden))) {
                compare = CMP_TYPEDEF_ID (tdef1, tdef2);
            } else
#endif
            {
                compare = CmpTypes (type_1, type_2);
            }
        }
    }

    /*
     * Hack to enable complex type spacialization.
     */

    if ((compare == CMP_incompatible) && (TYPES_BASETYPE (type_one) == T_user)
        && (TYPES_BASETYPE (type_two) == T_user)
        && (TYPES_TDEF (type_one) == TYPES_TDEF (type_two))) {

        if ((TYPES_DIM (type_one) == UNKNOWN_SHAPE)
            && (TYPES_DIM (type_two) == UNKNOWN_SHAPE)) {
            compare = CMP_both_unknown_shape;
        } else if ((TYPES_DIM (type_one) == UNKNOWN_SHAPE)
                   && (TYPES_DIM (type_two) > 0)) {
            compare = CMP_one_unknown_shape;
        } else if ((TYPES_DIM (type_one) > 0)
                   && (TYPES_DIM (type_two) == UNKNOWN_SHAPE)) {
            compare = CMP_one_unknown_shape;
        }
    }

    if ((compare == CMP_incompatible) && (TYPES_BASETYPE (type_one) == T_user)
        && (TYPES_BASETYPE (type_two) != T_user)
        && (TYPEDEF_BASETYPE (TYPES_TDEF (type_one)) == TYPES_BASETYPE (type_two))
        && (TYPEDEF_DIM (TYPES_TDEF (type_one)) > 0)
        && (TYPES_DIM (type_one) == UNKNOWN_SHAPE) && (TYPES_DIM (type_two) > 0)) {

        compare = CMP_one_unknown_shape;
    }

    if ((compare == CMP_incompatible) && (TYPES_BASETYPE (type_two) == T_user)
        && (TYPES_BASETYPE (type_one) != T_user)
        && (TYPEDEF_BASETYPE (TYPES_TDEF (type_two)) == TYPES_BASETYPE (type_one))
        && (TYPEDEF_DIM (TYPES_TDEF (type_two)) > 0)
        && (TYPES_DIM (type_two) == UNKNOWN_SHAPE) && (TYPES_DIM (type_one) > 0)) {

        compare = CMP_one_unknown_shape;
    }

    if (NULL != shpseg_1) {
        Free (type_1->shpseg);
        type_1->shpseg = shpseg_1;
        type_1->dim = old_dim1;
    }
    if (NULL != shpseg_2) {
        Free (type_2->shpseg);
        type_2->shpseg = shpseg_2;
        type_2->dim = old_dim2;
    }
    if (T_user == TYPES_BASETYPE (type_one))
        FREE_TYPES (type_1);
    if (T_user == TYPES_BASETYPE (type_two))
        FREE_TYPES (type_2);

    DBUG_PRINT ("TYPE", ("%d", compare));

    DBUG_RETURN (compare);
}

/*
 *
 *  functionname  : UpdateType
 *  arguments     : 1) first type
 *                  2) second type
 *                  3) linenumber
 *  description   : if one of the types is an arrays with unknown shape
 *                  the shapes are copied.
 *  global vars   : filename
 *  internal funs : Malloc, Type2String
 *  external funs : sizeof
 *  macros        : DBUG..., NULL, Free, MOD_NAME, DIM, ABORT
 *
 *  remarks       : If one type is not a primitive type and one is a primitive
 *                  type, it has to be checked whether one type can be updated.
 *                  The function CompatileTypes checks it.
 *                  !!! Call this function only after calling CmpTypes or
 *                      CompatibleTypes. !!!
 *
 */

static void
UpdateType (types *type_one, types *type_two, int line)
{
    int i;
    node *t_node;
    types *t_known = NULL, *t_unknown = NULL;

#ifndef DBUG_OFF
    char *db_str1, *db_str2;
#endif

    DBUG_ENTER ("UpdateType");

    /*
     *  One of the types has unknown shape, the other one's shape or at least its
     * dimensionality is known. NB: the latter only comes true if -ds is set since
     * this may lead to specializations such as int[] => int[.]!
     *  Which one is which? (jhs)
     */
    if ((KNOWN_SHAPE (TYPES_DIM (type_one)) && (!KNOWN_SHAPE (TYPES_DIM (type_two))))
        || ((TYPES_DIM (type_one) < KNOWN_DIM_OFFSET)
            && (TYPES_DIM (type_two) == UNKNOWN_SHAPE))
        || (TYPES_DIM (type_two) == ARRAY_OR_SCALAR)) {
        /*
         * note here that (! KNOWN_SHAPE) is not identical to UNKNOWN_SHAPE !!!
         */
        t_unknown = type_two;
        t_known = type_one;
    } else if ((KNOWN_SHAPE (TYPES_DIM (type_two))
                && (!KNOWN_SHAPE (TYPES_DIM (type_one))))
               || ((TYPES_DIM (type_two) < KNOWN_DIM_OFFSET)
                   && (TYPES_DIM (type_one) == UNKNOWN_SHAPE))
               || (TYPES_DIM (type_one) == ARRAY_OR_SCALAR)) {
        t_unknown = type_one;
        t_known = type_two;
    }

    DBUG_ASSERT (((NULL != t_unknown) && (NULL != t_known)),
                 "UpdateType: wrong types or update not needed");

#ifndef DBUG_OFF
    db_str1 = Type2String (t_unknown, 0, TRUE);
    db_str2 = Type2String (t_known, 0, TRUE);
    DBUG_PRINT ("TYPE", ("old types :%s , %s ", db_str1, db_str2));
    db_str1 = Free (db_str1);
    db_str2 = Free (db_str2);
#endif

    if ((T_user != TYPES_BASETYPE (t_unknown)) && (T_user == TYPES_BASETYPE (t_known))) {
        t_node = TYPES_TDEF (t_known);
        if (t_node == NULL) {
            t_node = LookupType (TYPES_NAME (t_known), TYPES_MOD (t_known), line);
        }

        if (NULL == t_node) {
            ABORT (line, ("Type '%s` is unknown",
                          ModName (t_unknown->name_mod, t_unknown->name)));
        }
        TYPES_TDEF (t_known) = t_node;
        DBUG_ASSERT ((NULL != t_node), "t_node is NULL");
        t_unknown->dim = t_node->DIM + t_known->dim;
        DBUG_ASSERT ((t_unknown->dim <= SHP_SEG_SIZE), "dimension to large");

        if (KNOWN_SHAPE (t_unknown->dim)) {
            t_unknown->shpseg = MakeShpseg (NULL);
            for (i = 0; i < t_known->dim; i++) {
                t_unknown->shpseg->shp[i] = t_known->shpseg->shp[i];
            }
            for (i = t_known->dim; i < t_unknown->dim; i++) {
                t_unknown->shpseg->shp[i] = t_node->SHP[i - t_known->dim];
            }
        }
    } else if ((T_user == TYPES_BASETYPE (t_unknown))
               && (T_user != TYPES_BASETYPE (t_known))) {
        t_node = TYPES_TDEF (t_unknown);
        if (t_node == NULL) {
            t_node = LookupType (TYPES_NAME (t_unknown), TYPES_MOD (t_unknown), line);
        }

        if (NULL == t_node) {
            ABORT (line,
                   ("Type '%s` is unknown", ModName (t_known->name_mod, t_known->name)));
        }
        TYPES_TDEF (t_unknown) = t_node;

        DBUG_ASSERT ((NULL != t_node), "t_node is NULL");
        t_unknown->dim = t_known->dim - t_node->DIM;
        if (KNOWN_SHAPE (t_unknown->dim)) {
            t_unknown->shpseg = MakeShpseg (NULL);
            for (i = 0; i < t_unknown->dim; i++) {
                t_unknown->shpseg->shp[i] = t_known->shpseg->shp[i];
            }
        }
    } else {
        DBUG_PRINT ("TYPE", ("array-cast"));

        /* both types are arrays of compatible primitive type
         * in this case we have to copy the shape of t_known to t_unknown
         */
        DBUG_ASSERT (((TYPES_BASETYPE (t_known) == T_nothing)
                      || (TYPES_BASETYPE (t_unknown) == TYPES_BASETYPE (t_known))
                      || ((T_int == TYPES_BASETYPE (t_unknown))
                          && (T_float == TYPES_BASETYPE (t_known)))
                      || ((T_float == TYPES_BASETYPE (t_unknown))
                          && (T_int == TYPES_BASETYPE (t_known)))
                      || ((T_int == TYPES_BASETYPE (t_unknown))
                          && (T_double == TYPES_BASETYPE (t_known)))
                      || ((T_double == TYPES_BASETYPE (t_unknown))
                          && (T_int == TYPES_BASETYPE (t_known)))
                      || ((T_double == TYPES_BASETYPE (t_unknown))
                          && (T_float == TYPES_BASETYPE (t_known)))
                      || ((T_float == TYPES_BASETYPE (t_unknown))
                          && (T_double == TYPES_BASETYPE (t_known)))),
                     "wrong simpletypes ");

        t_unknown->dim = t_known->dim;
        t_unknown->shpseg = MakeShpseg (NULL);

        for (i = 0; i < t_known->dim; i++) {
            t_unknown->shpseg->shp[i] = t_known->shpseg->shp[i];
        }
    }

#ifndef DBUG_OFF
    db_str1 = Type2String (t_unknown, 0, TRUE);
    db_str2 = Type2String (t_known, 0, TRUE);

    DBUG_PRINT ("TYPE", ("new types :%s , %s ", db_str1, db_str2));

    db_str1 = Free (db_str1);
    db_str2 = Free (db_str2);
#endif

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : DuplicateFun
 *  arguments     : 1) pointer to fun-table-entrie
 *  description   : duplicates a function(fun_p->node)(insert it into
 *                  fun-table and syntax-tree) if fun_p->n_dup >0
 *
 */

static fun_tab_elem *
DuplicateFun (fun_tab_elem *fun_p)
{
    fun_tab_elem *new_fun_p;
    node *new_fun_node, *next_fun;

    DBUG_ENTER ("DuplicateFun");

    if (fun_p->n_dub > 0) {
        fun_p->n_dub--;
        next_fun = fun_p->node->node[1]; /* store pointer to next function */
        fun_p->node->node[1] = NULL;     /* set NULL to duplicate only current
                                          * function
                                          */
        new_fun_node = DupTree (fun_p->node);
        DBUG_ASSERT (N_fundef == new_fun_node->nodetype, "nodetype != N_fundef");
        DBUG_ASSERT (NULL == new_fun_node->node[1], "next function is also copied");

        /* insert copied function into syntax-tree */
        new_fun_node->node[1] = next_fun;
        fun_p->node->node[1] = new_fun_node;

        /* set ATTRIB tag */
        FUNDEF_ATTRIB (new_fun_node) = ST_generic;

        DBUG_PRINT ("FUN_TAG",
                    ("ATTRIB-tag of function '%s' set to:%d",
                     ModName (FUNDEF_MOD (new_fun_node), FUNDEF_NAME (new_fun_node)),
                     FUNDEF_ATTRIB (new_fun_node)));

        /*
         * Insert copied function into fun_table.
         *
         *  The fun table stores functions with equal names right behind each
         *  other. Since typecheck constant propagation may lead to function
         *  specializations with different names, specialized functions are
         *  always inserted at the end of the table or between two chains of
         *  of functions with identical names.
         */

        while ((fun_p->next != NULL) && (0 == strcmp (fun_p->id, fun_p->next->id))) {
            fun_p = fun_p->next;
        }

        OLD_INSERT_FUN (fun_p, FUNDEF_NAME (new_fun_node), FUNDEF_MOD (new_fun_node),
                        new_fun_node, NOT_CHECKED, -2);
        new_fun_p = NEXT_FUN_TAB_ELEM (fun_p);
    } else {
        ABORT (NODE_LINE (fun_p->node),
               ("%d specializations already created for function %s !  "
                "Use option -maxspecialize to enable additional "
                "specializations",
                max_overload, FUNDEF_NAME (fun_p->node)));
    }

    DBUG_RETURN (new_fun_p);
}

/*
 *
 *  functionname  :FindFun
 *  arguments     : 1) name of function
 *                  2) name of function's module
 *                  3) current arguments for function
 *                  4) arg_info from the calling function
 *                  5) linenumber
 *                  6) -1  => userdefined function
 *                     >=0 => primitive function one looks for
 *                     This argument is used to indicate that a primitive
 *                     function is overloaded by a userdefined one by
 *                     returning a -1. This fact is only used in TI_prf.
 *
 *  description   : looks for matching functions (matching to name and argument
 *                  type)
 *  global vars   : filename, prf_string[]
 *  internal funs : Malloc, IsNameInMods,  LookupFun, LookupPrf, CmpTypes,
 *                  CompatibleTypes, UpdateType
 *  external funs : sizeof, FindSymbolInModul
 *
 *  remarks       : if no function is found that matches the types of the
 *                  arguments then it will be looked for a function that has
 *                  compatible types with the arguments ( check whether
 *                  parameters and arguments have equal primitive type)
 *
 *                  - parameter-types of external functions will not be updated
 *                  - the meaning of variable 'found':
 *  FOUND_NONE         0: no function found
 *  FOUND_USER         1: found matching userdefined function without formal
 *                        parameters of unknown shape
 *  FOUND_USER_UNKNOWN 2: found matching userdefined function with at least
 *                        one formal paramter of unknown shape
 *  FOUND_PRIM         3: found matching primitive function without formal
 *                        parameters of unknown shape
 *  FOUND_PRIM_UNKNOWN 4: found matching primitive function with at least
 *                        one formal paramter of unknown shape
 *                  - the meaning of variable 'equal_types':
 *  EQUAL_NO           0: current function declaration doesn't match with
 *                        given argument-types(3)
 *  EQUAL_YES          1: current function declaration matches with given
 *                        argument_types( there isn't any unknown shape in
 *                        declaration of current function)
 *  EQUAL_YES_UNKNOWN  2: current function declaration matches given argument-
 *                        types, but there is at least one formal parameter of
 *                        unknown shape
 *
 */

static void *
FindFun (char *fun_name, char *mod_name, types **arg_type, int count_args, node *arg_info,
         int line, int *prf_fun)
{

#define FOUND_NONE 0
#define FOUND_USER 1
#define FOUND_USER_UNKNOWN 2
#define FOUND_PRIM 3
#define FOUND_PRIM_UNKNOWN 4

#define EQUAL_NO 0
#define EQUAL_YES 1
#define EQUAL_YES_UNKNOWN 2

    int i, equal_types, count_param, stored_prf_fun, found = FOUND_NONE, once_again = 0,
                                                     v_args;
    cmp_types is_correct = CMP_incompatible;
    node *fun_args;
    void *ret_node = NULL;
    mods *mods = NULL;
    fun_tab_elem *fun_p = NULL, *fun_p_store = NULL;
    prim_fun_tab_elem *prf_p = NULL, *prf_p_store = NULL;

#define FUN_P (-1 == *prf_fun) ? (void *)fun_p : (void *)prf_p
#define FUN_ARGS (-1 == *prf_fun) ? FUNDEF_ARGS (fun_p->node) : FUNDEF_ARGS (prf_p->node)
#define IS_PRIMFUN (-1 != *prf_fun)

    DBUG_ENTER ("FindFun");

    if ((NULL != mod_name) && (0 != strcmp (mod_name, PSEUDO_MOD_FOLD))) {
        /*
         *  look where 'fun_name' is defined
         */
        mods = FindSymbolInModul (mod_name, fun_name, 2, NULL, 1);
    }

    /* now look for fitting function */
    if (IS_PRIMFUN) {
        prf_p = LookupPrf ((prf) (*prf_fun), NULL);
        prf_p_store = prf_p;
    } else {
        fun_p = LookupFun (fun_name, NULL, NULL);
        fun_p_store = fun_p;
    }
    stored_prf_fun = *prf_fun;

    if (IS_PRIMFUN || (fun_p != NULL)) {
        do {
            if (IS_PRIMFUN)
                prf_p = prf_p_store;
            else
                fun_p = fun_p_store;

            while ((!IS_PRIMFUN)
                     ? (END_OF_FUN_TAB (fun_p) ? 0 : (!(strcmp (fun_name, fun_p->id))))
                     : ((NULL != prf_p) ? (prf) (*prf_fun) == prf_p->prf : 0)) {
                DBUG_PRINT ("TYPE",
                            ("current fun '%s'" F_PTR " found:%d",
                             (-1 == *prf_fun) ? ModName (FUNDEF_MOD (fun_p->node),
                                                         FUNDEF_NAME (fun_p->node))
                                              : mdb_prf[*prf_fun],
                             (-1 == *prf_fun) ? fun_p->node : prf_p->node, found));

                if ((NULL == mod_name) || (strcmp (mod_name, PSEUDO_MOD_FOLD) == 0)
                    || IsNameInMods (fun_p->id_mod, mods)) {
                    /* count number of parameters */
                    fun_args = FUN_ARGS;
                    count_param = 0;
                    v_args = 0;
                    while (NULL != fun_args) {
                        /*  increase count_param only, if the type of the formal
                         *  parameter is not equal T_dots.
                         *  T_dots should be the last parameter of a function,  so we
                         *  only have to compare the types of the first parameter and
                         *  arguments.
                         */
                        if (T_dots == ARG_BASETYPE (fun_args))
                            v_args = 1;
                        else
                            count_param++;

                        DBUG_PRINT ("TYPE", ("%d. : %s" F_PTR, count_param,
                                             Type2String (ARG_TYPE (fun_args), 0, TRUE),
                                             ARG_TYPE (fun_args)));
                        fun_args = ARG_NEXT (fun_args);
                    } /* while (NULL != funargs) */

                    if ((count_args == count_param) || (1 == v_args)) {
                        if (0 != count_param) {
                            /* now compare the types */
                            fun_args = FUN_ARGS;
                            equal_types = EQUAL_YES;
                            for (i = 0; i < count_param;
                                 fun_args = ARG_NEXT (fun_args), i++) {
                                if (0 == once_again) {
                                    is_correct
                                      = CmpTypes (arg_type[i], ARG_TYPE (fun_args));
                                } else if (1 == once_again) {
                                    is_correct
                                      = CompatibleTypes (arg_type[i], ARG_TYPE (fun_args),
                                                         0, line);
                                }

                                DBUG_PRINT ("TYPE", ("is_correct is %i", is_correct));

                                if (CMP_one_unknown_shape == is_correct) /* 2 */ {
                                    equal_types = EQUAL_YES_UNKNOWN;
                                } else if ((CMP_both_unknown_shape == is_correct)
                                           && ((SAC_MOD == kind_of_file)
                                               || (dynamic_shapes == 1))) {
                                    equal_types = EQUAL_YES_UNKNOWN; /* neu am 8.12. */
                                } else if (CMP_equal != is_correct) /* 1 */ {
                                    equal_types = EQUAL_NO;
                                    break;
                                }
                            } /* for */

                            DBUG_PRINT ("TYPE", ("Found is %i", found));

                            switch (found) {
                            case FOUND_NONE:
                                if (EQUAL_YES == equal_types) {
                                    if (IS_PRIMFUN)
                                        found = FOUND_PRIM;
                                    else
                                        found = FOUND_USER;
                                    ret_node = FUN_P;
                                } else if (EQUAL_YES_UNKNOWN == equal_types) {
                                    if (IS_PRIMFUN)
                                        found = FOUND_PRIM_UNKNOWN;
                                    else
                                        found = FOUND_USER_UNKNOWN;
                                    ret_node = FUN_P;
                                }
                                break;
                            case FOUND_USER:
                            case FOUND_PRIM:
                                if (EQUAL_YES == equal_types) {
                                    /*
                                     *  there are more than one matching functions,
                                     *  so an error message will be given.
                                     */

                                    ERROR (
                                      line,
                                      ("More than one function matches the call of '%s`",
                                       ModName (mod_name,
                                                ((IS_PRIMFUN) ? prf_string[*prf_fun]
                                                              : fun_name))));
                                }
                                break;
                            case FOUND_PRIM_UNKNOWN:
                                if (EQUAL_YES_UNKNOWN == equal_types) {
                                    /*
                                     * there are more than one matching functions,
                                     * so an error message will be given.
                                     */

                                    ERROR (
                                      line,
                                      ("More than one function matches the call of '%s`",
                                       ModName (mod_name,
                                                ((IS_PRIMFUN) ? prf_string[*prf_fun]
                                                              : fun_name))));
                                }
                                break;
                            case FOUND_USER_UNKNOWN:
                                /* if((1==equal_types) && IS_DUPLICATED(fun_p)) */
                                if (EQUAL_YES == equal_types) {
                                    /*
                                     * Until now a function with at least one unknown
                                     * shape was found, but now there is one with all
                                     * shapes known, this one will be used from now on.
                                     * (jhs)
                                     */
                                    found = FOUND_USER;
                                    ret_node = FUN_P;
                                } else
                                  /* if((0<equal_types) && (!IS_DUPLICATED(fun_p))) */
                                  if (EQUAL_YES_UNKNOWN == equal_types) {
                                    /*
                                     *  there are more than one matching functions,
                                     *  so an error message will be given.
                                     */

                                    ERROR (
                                      line,
                                      ("More than one function matches the call of '%s`",
                                       ModName (mod_name,
                                                ((IS_PRIMFUN) ? prf_string[*prf_fun]
                                                              : fun_name))));
                                }
                                break;
                            default:
                                DBUG_ASSERT (0, "'found' has wrong value");
                            } /* switch (found) */
                        } else {
                            /*
                             *  Both functions have no parameters, so it's easy. (jhs)
                             */
                            if (IS_PRIMFUN)
                                found = FOUND_PRIM;
                            else
                                found = FOUND_USER;
                            ret_node = FUN_P;
                        } /* if (0 != count_param ) ... else ... */
                    }     /* if */
                }         /* if */
                /* goto next function */
                if (-1 == *prf_fun)
                    fun_p = NEXT_FUN_TAB_ELEM (fun_p);
                else
                    prf_p = prf_p->next;
            } /* while */
            if (NULL == ret_node) {
                if (-1 != stored_prf_fun) {
                    /* we are looking for a primitive function */
                    if (IS_PRIMFUN) {
                        if (0 == once_again) {
                            /* now look if there is a matching userdefined function
                             * with name of primitive function
                             */
                            fun_name = prf_symbol[*prf_fun];
                            fun_p_store = LookupFun (fun_name, NULL, NULL);
                            if (NULL != fun_p_store)
                                *prf_fun = -1;
                            else
                                /* there isn't any matching userdefined function,
                                 * so look if there is a primitive function with
                                 * 'compatible types'
                                 */
                                once_again += 1;
                        } else {
                            if (1 == once_again) {
                                if (NULL != fun_p_store) {
                                    *prf_fun = -1;
                                } else {
                                    once_again += 1;
                                }
                            } else {
                                once_again += 1;
                            }
                        } /* if */
                    } else {
                        if (0 == once_again)
                            *prf_fun = stored_prf_fun;
                        once_again += 1;
                    } /* if */
                } else {
                    once_again += 1;
                }                                         /* if */
            }                                             /* (NULL == ret_node) */
        } while ((once_again < 2) && (NULL == ret_node)); /* do ... while (...) */
        ABORT_ON_ERROR;

        if (NULL == ret_node) {
            char arg_str[MAX_ARG_TYPE_LENGTH];

            arg_str[0] = '\0';
            for (i = 0; i < count_args; i++) {
                strcat (arg_str, Type2String (arg_type[i], 0, TRUE));
                if (i + 1 != count_args)
                    strcat (arg_str, ", ");
            }
            ABORT (line,
                   ("Function '%s` is undefined or is applied to"
                    " wrong arguments '%s`",
                    ModName (mod_name, (IS_PRIMFUN) ? prf_string[*prf_fun] : fun_name),
                    arg_str));

        } else if (2 == found)
            if (NULL != FUNDEF_BODY (((fun_tab_elem *)ret_node)->node)) {
                /* argument types and types of formal parameters
                 * of userdefined not imported function(ret_node) are compatible,
                 * but the shapes of the arguments have to be
                 * copied to the formal parameters, if it is
                 * a userdefined (local) function
                 */
                int update = 0;

                /* First check whether we have to duplicate the function.
                 * If we check a Sac-module it is possible that we don't have to
                 * duplicate the function and update the types, because
                 * arguments and formal parameters have both unknown types.
                 *
                 * We have only to duplicate and to update, if there are args
                 * with known and formal parameters with unknown type.
                 * OR (in case -ds is turned on) if the args are of known
                 * dimensionality and formal parameters with unknown shape !!!
                 */
                fun_p = ret_node;
                fun_args = FUN_ARGS;
                for (i = 0; i < count_args; fun_args = ARG_NEXT (fun_args), i++) {
                    if ((!KNOWN_SHAPE (ARG_DIM (fun_args)))
                        && (KNOWN_SHAPE (TYPES_DIM (arg_type[i]))
                            || (KNOWN_DIMENSION (TYPES_DIM (arg_type[i]))
                                && (dynamic_shapes == 1)))) {

                        if ((TYPES_DIM (arg_type[i]) < KNOWN_DIM_OFFSET)
                            && (ARG_DIM (fun_args) < KNOWN_DIM_OFFSET)) {
                            /*
                             * for the argument as well as for the formal parameter the
                             * dimension is known but the shape is unknown
                             * -> no update needed
                             */
                        } else {
                            update = 1;
                        }
                        break;
                    } else if ((SAC_PRG == kind_of_file)
                               && (!KNOWN_SHAPE (TYPES_DIM (arg_type[i])))
                               && (!KNOWN_SHAPE (ARG_DIM (fun_args)))) {
                        ABORT (line,
                               ("Argument %d of function '%s' has"
                                " unknown shape (%s)",
                                i + 1,
                                ModName (mod_name,
                                         (IS_PRIMFUN) ? prf_string[*prf_fun] : fun_name),
                                Type2String (arg_type[i], 0, TRUE)));
                    }
                }

                if (1 == update) {
                    fun_p = DuplicateFun (ret_node);
                    fun_args = FUN_ARGS;
                    for (i = 0; i < count_args; fun_args = ARG_NEXT (fun_args), i++)
                        if (!KNOWN_SHAPE (ARG_DIM (fun_args))) {
                            /* update types only if arg_type has known shape */
                            if (KNOWN_SHAPE (TYPES_DIM (arg_type[i]))
                                || (KNOWN_DIMENSION (TYPES_DIM (arg_type[i]))
                                    && (dynamic_shapes == 1))) {
                                UpdateType (arg_type[i], ARG_TYPE (fun_args),
                                            NODE_LINE (fun_p->node));
#ifdef SHAPE_NOTE
                                NOTE (("%s:%d: NOTE: parameter '%s` "
                                       "has now defined shape (%s)",
                                       filename, NODE_LINE (fun_p->node),
                                       ARG_NAME (fun_args),
                                       Type2String (ARG_TYPE (fun_args), 0, TRUE)));
#endif /* SHAPE_NOTE */
                            }
                        }
                    ret_node = FUN_P;
                }
            }
    } else {
        ABORT (line,
               ("Function '%s` undefined",
                ModName (mod_name, (IS_PRIMFUN) ? prf_string[*prf_fun] : fun_name)));
    }

    /*
     * I don't think that the following warning still does what it originally
     * was intended for.

    if(once_again >0)
       WARN(line,
            ("Missing casts in call of function '%s`",
             ModName(mod_name, (IS_PRIMFUN)?prf_string[*prf_fun]:fun_name)));
     */
#undef FUN_P
#undef FUN_ARGS
#undef IS_PRIMFUN
    DBUG_RETURN (ret_node);
}

/*
 *
 *  functionname  :AddIdToStack
 *  arguments     :1) variable to add
 *                 2) infered type of 1)
 *                 3) info_node
 *                 4) line no
 *  description   : adds id to scope stack
 *                  - looks whether id is declared in variabledeclaration
 *                    if it is, look whether types are equal ( declaration
 *                    type 'float' and id-type `int` is equal as well )
 *
 *          sbs: I don't know WHY float and int are considered equal here!!
 *               But, I do know, that this causes problems for the examples
 *               errors/fixed/typesys_0001.sac  and
 *               errors/fixed/typesys_0002.sac!!
 *               Therefore, I switched the flag for calling CompatibleTypes
 *               (see below...)
 *
 */

static void
AddIdToStack (ids *ids, types *type, node *arg_info, int line)
{
    node *vardec_p;
    int is_defined = 0;
    char *str;

    DBUG_ENTER ("AddIdToStack");

#ifdef KNOWN_SHAPE_ONLY
    DBUG_ASSERT (-1 != type->dim, "type of unknown shape ");
#endif

    vardec_p = INFO_TC_VARDEC (arg_info); /* pointer to var declaration */
    if (vardec_p) {
        /* looking, if ids->id is defined in variable declaration */
        while ((VARDEC_NEXT (vardec_p)) && !is_defined)
            if (!(strcmp (ids->id, VARDEC_NAME (vardec_p))))
                is_defined = 1;
            else
                vardec_p = VARDEC_NEXT (vardec_p);

        if (is_defined || (!strcmp (ids->id, VARDEC_NAME (vardec_p)))) {
            /* variable ids->id is defined in variable declaration part
             * so check if typedeclaration is ok
             */
            is_defined = 1;
            /*
             * sbs: calling CompatibleTypes with 0 as 3rd arg makes int, float,
             * and double incompatible!!
             */
            switch (CompatibleTypes (VARDEC_TYPE (vardec_p), type, 0, line)) {
            case CMP_equal: /* 1*/
                break;
            case CMP_incompatible: /* 0 */ {
                ABORT (line, ("Incompatible types in declaration (%s) "
                              "and  usage (%s) of variable '%s`",
                              Type2String (VARDEC_TYPE (vardec_p), 0, TRUE),
                              Type2String (type, 0, TRUE), ids->id));
                break;
            }
            case CMP_one_unknown_shape: /* 2 */ {
                UpdateType (VARDEC_TYPE (vardec_p), type, line);
                break;
            }
            case CMP_both_unknown_shape: {
                if (kind_of_file != SAC_MOD) {
                    if (dynamic_shapes == 0) {
                        str = Type2String (type, 0, TRUE);
                        WARN (line, ("Types in declaration (%s) and usage "
                                     "(%s) of variable '%s` have unknown shape",
                                     Type2String (VARDEC_TYPE (vardec_p), 0, TRUE), str,
                                     ids->id));
                        str = Free (str);
                    } else {
                        /* update int[] => int[.,...,.] ! */
                        UpdateType (VARDEC_TYPE (vardec_p), type, line);
                    }
                }
                break;
            }
            default: {
                DBUG_ASSERT (0, "unknown return value of function CompatibleTypes");
                break;
            }
            }
        }
    }

    if (1 == is_defined) {
        ids->node = vardec_p;
        VARDEC_STATUS (vardec_p) = ST_used;

        DBUG_PRINT ("REF", ("added reference" F_PTR " from %s to %s", IDS_VARDEC (ids),
                            ids->id, VARDEC_OR_ARG_NAME (IDS_VARDEC (ids))));

        PUSH_VAR (ids->id, vardec_p);
    } else {
        /* first use of variable ids->id
         * so one must create a new N_vardec node and put it in front of
         * the variable declaration of the current function
         */

        node *vardec;

        vardec_p = INFO_TC_VARDEC (arg_info); /* pointer to var declaration */
        vardec = MakeVardec (StringCopy (ids->id), DupAllTypes (type), NULL);
        VARDEC_STATUS (vardec) = ST_used;
        VARDEC_ATTRIB (vardec) = ST_regular;

        if (vardec_p)
            VARDEC_NEXT (vardec) = vardec_p; /* append old variable declaration */
        /* store pointer variable declaration part */
        INFO_TC_VARDEC (arg_info) = vardec;
        DBUG_PRINT ("TYPE", ("add %s to variable declaration ", ids->id));

        /* insert reference to variable declaration */
        ids->node = vardec;
        DBUG_PRINT ("REF", ("added reference" F_PTR " from %s to %s", IDS_VARDEC (ids),
                            ids->id, VARDEC_OR_ARG_NAME (IDS_VARDEC (ids))));

        /* add variable to local stack */
        PUSH_VAR (ids->id, vardec);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : TCfundef
 *  arguments     : 1) argument node
 *                  2) argument infomation
 *  description   :  - add formal parameters to local stack
 *                  - creates new info_node with following meaning:
 *                     - node[0]: pointer to declaration of local vaiables
 *                     - node[1]: status
 *                     - info.id: name of current function
 *                  - call Trav to traverse body of function
 *                  - checks whether there is a nameclash between a userdefined
 *                    type and the function-name or an identifier in the
 *                    declaration of variables
 *  global vars   : tos, act_frame
 *  internal funs : LookupFun
 *  external funs : MakeNode, Trav
 *  macros        : DBUG..., PUSH_VAR, Free, NOT_CHECKED, NULL, CHECK_NAME
 *
 *  remarks       : - It will be checked whether formal parameters have equal
 *                    name. If there are equal names typechecking will be
 *                    aborted after checking all formal parameters.
 *                  - following info nodes are set:
 *                    info_node->node[0] :status of function (N_ok / N_stop)
 *                    info_node->node[0]->node[0] : pointer to variable decl.
 *                    info_node->node[0]->node[1] : pointer to fun decl.
 *
 */

node *
TCfundef (node *arg_node, node *arg_info)
{
    node *tmp_node, *info_node, *fun_params;
    stack_elem *old_tos = tos, /* store tos */
      *old_frame = act_frame;  /*store act_frame */
    fun_tab_elem *fun;
    int ok = 1, i = 1, j;

    DBUG_ENTER ("TCfundef");

    DBUG_PRINT ("TYPE", ("checking function %s", FUNDEF_NAME (arg_node)));

    act_frame = tos; /* set new frame pointer */

    /* check whether there is a userdefined type with same name as function */
    CHECK_FUNCTION_TYPENAMECLASH (arg_node);

    /* add formal parameters to local stack */
    fun_params = FUNDEF_ARGS (arg_node);
    while (NULL != fun_params) {
        tmp_node = ARG_NEXT (fun_params);
        j = i + 1;
        while (NULL != tmp_node) {
            if (0 == strcmp (ARG_NAME (tmp_node), ARG_NAME (fun_params))) {
                ERROR (NODE_LINE (arg_node),
                       ("%d. and %d. argument of function '%s' have same"
                        " name '%s`",
                        i, j, ModName (FUNDEF_MOD (arg_node), FUNDEF_NAME (arg_node)),
                        ARG_NAME (fun_params)));
                ok = 0;
            }
            j += 1;
            tmp_node = ARG_NEXT (tmp_node);
        }

        CHECK_TYPENAMECLASH (fun_params);
        if (1 == ok) {
            PUSH_VAR (ARG_NAME (fun_params), fun_params);
        } else {
            ok = 1;
        }

        fun_params = ARG_NEXT (fun_params);
        i += 1;
    }
    ABORT_ON_ERROR;

    /* set global variable 'imported_fun' */
    imported_fun = ((ST_imported_mod == FUNDEF_STATUS (arg_node))
                    || (ST_imported_class == FUNDEF_STATUS (arg_node)))
                     ? IMPORTED
                     : NOT_IMPORTED;

    info_node = MakeInfo ();
    info_node->node[0] = MakeOk (); /* status 0 = beginning of function */
    INFO_TC_STATUS (info_node) = 0;

    /* pointer to variable declaration */
    INFO_TC_VARDEC (info_node) = BLOCK_VARDEC (FUNDEF_BODY (arg_node));
    INFO_TC_FUNDEF (info_node) = arg_node; /* function declaration */
    info_node->info.id = FUNDEF_NAME (arg_node);

    INFO_TC_NEXTASSIGN (info_node) = NULL;
    INFO_TC_LASSIGN (info_node) = NULL;

    Trav (FUNDEF_BODY (arg_node), info_node);

    if (NULL != info_node->node[0]->node[0]) {
        node *vardec;
        /* set new variable declaration part */
        BLOCK_VARDEC (FUNDEF_BODY (arg_node)) = info_node->node[0]->node[0];

        /*
         * Now, test whether one identifier has the same name as a userdefined
         * type.
         * For each used variable declaration of a user-defined type, look for
         * the respective typedef node and store a reference to it in the
         * vardec node. This information is used by the FunctionAnalyser.
         */

        vardec = FUNDEF_VARDEC (arg_node);
        do {
            CHECK_TYPENAMECLASH (vardec);

            if (VARDEC_STATUS (vardec) == ST_used) {
                if ((VARDEC_BASETYPE (vardec) == T_user)
                    || ((VARDEC_BASETYPE (vardec) == T_hidden)
                        && (VARDEC_TNAME (vardec) != NULL))) {
                    VARDEC_TDEF (vardec)
                      = LookupType (VARDEC_TNAME (vardec), VARDEC_TMOD (vardec), 0);

                    if (VARDEC_TDEF (vardec) == NULL) {
                        ABORT (0,
                               ("Type '%s` used but not defined",
                                ModName (VARDEC_TMOD (vardec), VARDEC_TNAME (vardec))));
                    }

                    VARDEC_ATTRIB (vardec) = TYPEDEF_ATTRIB (VARDEC_TDEF (vardec));
                }
            }

            vardec = VARDEC_NEXT (vardec);
        } while (NULL != vardec);
        ABORT_ON_ERROR;
    }

    tos = old_tos;         /* restore tos */
    act_frame = old_frame; /*restore act_frame */

    if (N_stop == info_node->node[0]->nodetype) {
        fun = LookupFun (NULL, NULL, arg_node);
        fun->tag = NOT_CHECKED;
        DBUG_PRINT ("TYPE",
                    ("set tag of function %s to %s", fun->id, CHECK_NAME (fun->tag)));
    }
    info_node = FreeTree (info_node);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CheckIfGOonlyCBR
 *  arguments     : 1) pointer to arguments of a fundef node
 *                  2) pointer to expressions (current parameters) of a
 *                     function application (ap node)
 *  description   : checks if global objects are exclusively passed to
 *                  functions using the call-by-reference mechanism
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ERROR
 *
 *  remarks       :
 *
 */

static void
CheckIfGOonlyCBR (node *arg, node *exprs)
{
    node *expr;

    DBUG_ENTER ("CheckIfGOonlyCBR");

    while ((arg != NULL) && (ARG_BASETYPE (arg) != T_dots)) {
        expr = EXPRS_EXPR (exprs);

        if (NODE_TYPE (expr) == N_id) {
            if (GET_FLAG (ID, expr, IS_GLOBAL)) {
                if ((ARG_ATTRIB (arg) != ST_reference)
                    && (ARG_ATTRIB (arg) != ST_readonly_reference)) {
                    ERROR (NODE_LINE (expr),
                           ("Global object '%s` must be passed as a "
                            "reference parameter",
                            ModName (expr->info.ids->mod, ID_NAME (expr))));
                }
            }
        }

        arg = ARG_NEXT (arg);
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : TypeInference
 *  arguments     : 1) argument node
 *  description   : returns the type of the expression behind 1) if one
 *                  can derive it
 *                  or NULL if one can't infer the type
 *  remarks       : the result pointer has to be freed, when one does not
 *                  use it any more
 *
 */

types *
TypeInference (node *arg_node, node *arg_info)
{
    types *return_type, *tmp_types;
    int cnt;

    DBUG_ENTER ("TI");

    DBUG_PRINT ("TYPE", ("Nodetype is %s.", NODE_TEXT (arg_node)));

    switch (NODE_TYPE (arg_node)) {
    case N_prf:
        return_type = TI_prf (arg_node, arg_info);
        break;

    case N_ap:
        return_type = TI_ap (arg_node, arg_info);

        cnt = 0;
        tmp_types = return_type;
        while (tmp_types != NULL) {
            if (!(KNOWN_SHAPE (TYPES_DIM (tmp_types))))
                cnt++;
            tmp_types = TYPES_NEXT (tmp_types);
        }
        if ((cnt > 0) && (kind_of_file != SAC_MOD)) {
            WARN (NODE_LINE (arg_node),
                  ("For %d return type(s) of the application of function '%s` "
                   "no exact shapes can be infered. Type inference relies on "
                   "user-supplied shape information",
                   cnt, ModName (AP_MOD (arg_node), AP_NAME (arg_node))));
        }
        break;

    case N_Nwith:
        return_type = TI_Nwith (arg_node, arg_info);
        break;

    case N_id: {
        node *odef;
        stack_elem *stack_p;

        stack_p = NULL;

        if (GET_FLAG (ID, arg_node, IS_GLOBAL)) {
            /*
             *  The identifier was given a module name by the programmer,
             *  so it must be a global object.
             */

            odef = LookupObject (ID_NAME (arg_node), arg_node->info.ids->mod,
                                 NODE_LINE (arg_node));

            if (odef == NULL) {
                ABORT (NODE_LINE (arg_node),
                       ("Global object '%s` unknown",
                        ModName (arg_node->info.ids->mod, ID_NAME (arg_node))));
            }

            arg_node->info.ids->mod = OBJDEF_MOD (odef);
            ID_OBJDEF (arg_node) = odef; /* link to object definition */

            return_type = DupAllTypes (OBJDEF_TYPE (odef));
        } else {
            /*
             *  The identifier may be a local variable or a global object.
             */

            stack_p = LookupVar (ID_NAME (arg_node));

            if (stack_p) {
                return_type = DupAllTypes (VARDEC_OR_ARG_TYPE (stack_p->node));
                ID_VARDEC (arg_node) = stack_p->node;

                DBUG_PRINT ("REF", ("added reference" F_PTR " for %s to %s",
                                    ID_NAME (arg_node), ID_NAME (arg_node),
                                    VARDEC_OR_ARG_NAME (ID_VARDEC (arg_node))));
            } else {
                /*
                 *  If the identifier is not yet locally bound, it may be a
                 *  global object.
                 */

                odef = LookupObject (ID_NAME (arg_node), NULL, NODE_LINE (arg_node));

                if (odef != NULL) {
                    arg_node->info.ids->mod = OBJDEF_MOD (odef);
                    SET_FLAG (ID, arg_node, IS_GLOBAL, TRUE);
                    ID_OBJDEF (arg_node) = odef;

                    return_type = DupAllTypes (OBJDEF_TYPE (odef));
                } else {
                    /*
                     *  The identifier is still unknown, so no type can be infered.
                     */

                    return_type = NULL;
                }
            }
        }

        break;
    }
    case N_num:
        GEN_TYPE_NODE (return_type, T_int);
        break;

    case N_bool:
        GEN_TYPE_NODE (return_type, T_bool);
        break;

    case N_float:
        GEN_TYPE_NODE (return_type, T_float);
        break;

    case N_double:
        GEN_TYPE_NODE (return_type, T_double);
        break;

    case N_array:
        return_type = TI_array (arg_node, arg_info);
        break;

    case N_str:
        GEN_TYPE_NODE (return_type, T_str);
        break;

    case N_cast:
        return_type = TI_cast (arg_node, arg_info);
        break;
    case N_char:
        GEN_TYPE_NODE (return_type, T_char);
        break;
    default: {
        return_type = NULL; /* for avoiding a compiler warning */
        ABORT (0, ("Type inference for %s isn't implemented yet", NODE_TEXT (arg_node)));
        break;
    }
    }

    DBUG_RETURN (return_type);
}

/*
 *
 *  functionname  : TClet
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : checks the types of the variables assigned  to and
 *                  the infered type of the expression on the right side
 *                  of the Let
 *  global vars   : filename
 *  internal funs : LookupVar, CompatibleTypes, CheckIds,  AddIdToStack,
 *                  CompatibleTypes
 *  external funs : Type2String
 *  macros        : DBUG..., PUSH_VAR, GEN_NODE, ABORT, ERROR, F_PTR
 *                  Free
 *  remarks       : the reference to the variable declaration is put in
 *                  arg_node->info.ids->node
 *
 */

node *
TClet (node *arg_node, node *arg_info)
{
    ids *ids;
    types *type, *tmp, *next_type, *type_arg1, *type_arg2;
    stack_elem *elem;
#ifndef DBUG_OFF
    char *db_str; /* only used for debugging */
#endif

    DBUG_ENTER ("TClet");

    ids = LET_IDS (arg_node); /* left side of let-assign */
    if (ids)
        CheckIds (ids, INFO_TC_VARDEC (arg_info), NODE_LINE (arg_node));
    /*
     * look for double identifiers and name clashes with global objects
     */
    INFO_TC_LHSVARS (arg_info) = ids;

    type = TypeInference (LET_EXPR (arg_node),
                          arg_info); /* type of right side of let-assign */

    if (type == NULL)
        ABORT (NODE_LINE (arg_node), ("Type of right hand side of '=` is not inferable"));

    if (TYPES_BASETYPE (type) == T_nothing)
        ABORT (NODE_LINE (arg_node), ("Type of right hand side of '=` is inferable,"
                                      " but illegal (e.g. an untyped empty array)"));

    DBUG_PRINT ("STOP", ("arg_info->node[0]: %s", NODE_TEXT (arg_info->node[0])));

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_prf) {
        switch (PRF_PRF (LET_EXPR (arg_node))) {
        case F_sel:
            if (TYPES_DIM (type) == ARRAY_OR_SCALAR) {
                /*
                 * LET-expression is a call of function sel,
                 * and the infered type is SCALAR_OR_ARRAY .
                 * There has to be a declaration
                 * of the belonging variable
                 */

                node *vardec_node = NULL;

                DBUG_ASSERT (NULL != ids,
                             " there are no variables on the left side of a LET");
                DBUG_ASSERT (NULL != arg_info, "arg_info is NULL");
                DBUG_ASSERT (NULL != arg_info->node[0], "arg_info->node[0] is NULL");
                DBUG_ASSERT (NULL != arg_info->node[0]->node[0], "poiter to vardec is"
                                                                 "mising in arg_info");

                vardec_node = LookupVardec (ids->id, INFO_TC_VARDEC (arg_info));

                if (vardec_node) {
                    cmp_types cmp;
                    int tmp_dim = TYPES_DIM (type);

                    TYPES_DIM (type) = TYPES_DIM (VARDEC_TYPE (vardec_node));

                    if (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)))
                        TYPES_SHPSEG (type)
                          = DupShpseg (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)));

                    /*
                     * sbs: switched the prim_fun_flag from -1 to 0 since we
                     * otherwise get an error in UpdateType for the following example:
                     */
                    cmp = CompatibleTypes (VARDEC_TYPE (vardec_node), type, 0,
                                           NODE_LINE (arg_node));

                    if (!((cmp == CMP_equal) || (cmp == CMP_both_unknown_shape)
                          || (cmp == CMP_one_unknown_shape))) {
                        TYPES_DIM (type) = tmp_dim;
                        ABORT (NODE_LINE (arg_node),
                               ("Incompatible types in declaration (%s) "
                                "and usage (%s) of variable '%s`",
                                Type2String (VARDEC_TYPE (vardec_node), 0, TRUE),
                                Type2String (type, 0, TRUE), ids->id));
                    }
                } else {
                    if (kind_of_file != SAC_MOD) {
                        ABORT (NODE_LINE (arg_node),
                               ("Function 'sel' is called with argument of type '%s'"
                                ", so the type of the result has to be declared",
                                Type2String (type, 0, TRUE), ids->id));
                    }
                }
            }

            /*
             * REMARK to the implicit replacement of applications of primitive functions
             * by with-loops:
             *
             * What is done here is pretty tricky and brings some problems with respect
             * to the module system.
             *
             * Example:
             *   Module A is compiled with intrinsics off and the Array library imported.
             *   Program B imports A but not the Array library. So, sac2c will complain
             *   about a missing take() although B does not contain any take()
             * application.
             *
             * The best way out of this is a more accurate imitation of the specialization
             * mechanism, i.e. we have to create a new function definition for each
             * application of take, drop, etc. These may later be inlined by inlining and
             * the function definitions may themselves be removed by dead function
             * removal.
             */

            if (TYPES_DIM (type) != SCALAR) {
                /*
                 * Here, we have a sel returning an array.
                 */
                if (!(intrinsics & INTRINSIC_SEL)) {
                    /*
                     * If not the intrinsic implementation of primitive function sel() is
                     * to be used, the function application is now replaced by an
                     * equialent with-loop.
                     */
                    type_arg1 = ID_OR_CAST_TYPE (PRF_ARG1 (LET_EXPR (arg_node)));
                    type_arg2 = ID_OR_CAST_TYPE (PRF_ARG2 (LET_EXPR (arg_node)));

                    if (NULL != LookupFun (prf_symbol[F_sel], "Array", NULL)) {
                        if ((TYPES_DIM (type) > SCALAR)
                            && (TYPES_DIM (type_arg1) > SCALAR)
                            && (TYPES_DIM (type_arg2) > SCALAR)) {
                            /*
                             * The actual replacement is only performed if the result
                             * shape has been determined.
                             */
                            node *wl_impl;
                            types *new_type;
                            node *arg1, *arg2;

                            arg1 = PRF_ARG1 (LET_EXPR (arg_node));
                            arg2 = PRF_ARG2 (LET_EXPR (arg_node));
                            wl_impl = BuildSelWithLoop (type, arg1, arg2);
                            new_type = TypeInference (wl_impl, arg_info);
                            FreeAllTypes (new_type);

                            /*
                             * PRF_ARG1(LET_EXPR(arg_node)) = NULL;
                             *
                             * The index argument node has not been reused by
                             * BuildSelWithLoop(), so, it has to be freed now.
                             */

                            PRF_ARG2 (LET_EXPR (arg_node)) = NULL;
                            FreeTree (LET_EXPR (arg_node));

                            LET_EXPR (arg_node) = wl_impl;
                        }
                    } else {
                        ERROR (NODE_LINE (arg_node),
                               ("Function 'sel` undefined or applied to wrong arguments: "
                                "'%s, %s`",
                                Type2String (ID_OR_CAST_TYPE (
                                               PRF_ARG1 (LET_EXPR (arg_node))),
                                             0, TRUE),
                                Type2String (ID_OR_CAST_TYPE (
                                               PRF_ARG2 (LET_EXPR (arg_node))),
                                             0, TRUE)));
                    }
                }
            }

            break;

        case F_genarray: {
            if (TYPES_DIM (type) > SCALAR) {
                /*
                 * The primitive function genarray is not fully implemented as a primitive
                 * function, but is now converted into an equivalent with-loop.
                 */
                node *wl_impl;
                types *new_type;

                wl_impl = BuildGenarrayWithLoop (PRF_ARG1 (LET_EXPR (arg_node)),
                                                 PRF_ARG2 (LET_EXPR (arg_node)));
                new_type = TypeInference (wl_impl, arg_info);
                FreeAllTypes (new_type);

                PRF_ARG1 (LET_EXPR (arg_node)) = NULL;
                PRF_ARG2 (LET_EXPR (arg_node)) = NULL;
                FreeTree (LET_EXPR (arg_node));

                LET_EXPR (arg_node) = wl_impl;
            }

            break;
        }
        case F_take:
            if (!(intrinsics & INTRINSIC_TAKE)) {
                /*
                 * If not the intrinsic implementation of primitive function take() is to
                 * be used, the function application is now replaced by an equivalent
                 * with-loop.
                 */
                if (NULL != LookupFun (prf_symbol[F_take], "Array", NULL)) {
                    if (TYPES_DIM (type) > SCALAR) {
                        /*
                         * The actual replacement is only performed if the result shape
                         * has been determined.
                         */
                        node *wl_impl;
                        types *new_type;

                        wl_impl = BuildTakeWithLoop (PRF_ARG1 (LET_EXPR (arg_node)),
                                                     PRF_ARG2 (LET_EXPR (arg_node)));
                        PRF_ARG1 (LET_EXPR (arg_node)) = NULL;
                        PRF_ARG2 (LET_EXPR (arg_node)) = NULL;
                        FreeTree (LET_EXPR (arg_node));

                        LET_EXPR (arg_node) = wl_impl;

                        new_type = TypeInference (wl_impl, arg_info);
                        FreeAllTypes (new_type);
                    }
                } else {
                    ERROR (NODE_LINE (arg_node),
                           ("Function 'take` undefined or applied to wrong arguments: "
                            "'%s, %s`",
                            Type2String ((NODE_TYPE (PRF_ARG1 (LET_EXPR (arg_node)))
                                          == N_array)
                                           ? ARRAY_TYPE (PRF_ARG1 (LET_EXPR (arg_node)))
                                           : ID_TYPE (PRF_ARG1 (LET_EXPR (arg_node))),
                                         0, TRUE),
                            Type2String ((NODE_TYPE (PRF_ARG2 (LET_EXPR (arg_node)))
                                          == N_array)
                                           ? ARRAY_TYPE (PRF_ARG2 (LET_EXPR (arg_node)))
                                           : ID_TYPE (PRF_ARG2 (LET_EXPR (arg_node))),
                                         0, TRUE)));
                }
            }

            break;

        case F_drop:
            if (!(intrinsics & INTRINSIC_DROP)) {
                /*
                 * If not the intrinsic implementation of primitive function drop() is to
                 * be used, the function application is now replaced by an equialent
                 * with-loop.
                 */
                if (NULL != LookupFun (prf_symbol[F_drop], "Array", NULL)) {
                    if (TYPES_DIM (type) > SCALAR) {
                        /*
                         * The actual replacement is only performed if the result shape
                         * has been determined.
                         */
                        node *wl_impl;
                        types *new_type;

                        wl_impl = BuildDropWithLoop (type, PRF_ARG1 (LET_EXPR (arg_node)),
                                                     PRF_ARG2 (LET_EXPR (arg_node)));
                        new_type = TypeInference (wl_impl, arg_info);
                        FreeAllTypes (new_type);

                        PRF_ARG1 (LET_EXPR (arg_node)) = NULL;
                        PRF_ARG2 (LET_EXPR (arg_node)) = NULL;
                        FreeTree (LET_EXPR (arg_node));

                        LET_EXPR (arg_node) = wl_impl;
                    }
                } else {
                    ERROR (NODE_LINE (arg_node),
                           ("Function 'drop` undefined or applied to wrong arguments: "
                            "'%s, %s`",
                            Type2String ((NODE_TYPE (PRF_ARG1 (LET_EXPR (arg_node)))
                                          == N_array)
                                           ? ARRAY_TYPE (PRF_ARG1 (LET_EXPR (arg_node)))
                                           : ID_TYPE (PRF_ARG1 (LET_EXPR (arg_node))),
                                         0, TRUE),
                            Type2String ((NODE_TYPE (PRF_ARG2 (LET_EXPR (arg_node)))
                                          == N_array)
                                           ? ARRAY_TYPE (PRF_ARG2 (LET_EXPR (arg_node)))
                                           : ID_TYPE (PRF_ARG2 (LET_EXPR (arg_node))),
                                         0, TRUE)));
                }
            }

            break;

        case F_cat:
            if (!(intrinsics & INTRINSIC_CAT)) {
                /*
                 * If not the intrinsic implementation of primitive function cat() is to
                 * be used, the function application is now replaced by 2 equivalent
                 * with-loops.
                 */
                if (NULL != LookupFun (prf_symbol[F_cat], "Array", NULL)) {
                    if (TYPES_DIM (type) > SCALAR) {
                        /*
                         * The actual replacement is only performed if the result shape
                         * has been determined.
                         */
                        node *wl_impl_1, *wl_impl_2;
                        types *new_type;
                        char *new_var;

                        wl_impl_1
                          = BuildCatWithLoop1 (type, PRF_ARG2 (LET_EXPR (arg_node)));
                        new_type = TypeInference (wl_impl_1, arg_info);
                        FreeAllTypes (new_type);

                        wl_impl_2 = BuildCatWithLoop2 (LET_IDS (arg_node),
                                                       PRF_ARG1 (LET_EXPR (arg_node)),
                                                       PRF_ARG2 (LET_EXPR (arg_node)),
                                                       PRF_ARG3 (LET_EXPR (arg_node)));
                        /*
                         * The second new with-loop is inserted into the assignment chain
                         * behind the current position, so it will be typechecked
                         * automatically.
                         */

                        PRF_ARG2 (LET_EXPR (arg_node)) = NULL;
                        PRF_ARG3 (LET_EXPR (arg_node)) = NULL;
                        FreeTree (LET_EXPR (arg_node));

                        LET_EXPR (arg_node) = wl_impl_1;

                        new_var = TmpVar ();
                        ASSIGN_NEXT (INFO_TC_CURRENTASSIGN (arg_info))
                          = MakeAssign (MakeLet (wl_impl_2,
                                                 MakeIds (new_var, NULL, ST_regular)),
                                        MakeAssign (MakeLet (MakeId (StringCopy (new_var),
                                                                     NULL, ST_regular),
                                                             MakeIds (StringCopy (
                                                                        IDS_NAME (
                                                                          LET_IDS (
                                                                            arg_node))),
                                                                      NULL, ST_regular)),
                                                    ASSIGN_NEXT (
                                                      INFO_TC_CURRENTASSIGN (arg_info))));
                    }
                } else {
                    ERROR (NODE_LINE (arg_node),
                           ("Function 'cat` undefined or applied to wrong arguments: "
                            "'int, %s, %s`",
                            Type2String ((NODE_TYPE (PRF_ARG2 (LET_EXPR (arg_node)))
                                          == N_array)
                                           ? ARRAY_TYPE (PRF_ARG2 (LET_EXPR (arg_node)))
                                           : ID_TYPE (PRF_ARG2 (LET_EXPR (arg_node))),
                                         0, TRUE),
                            Type2String ((NODE_TYPE (PRF_ARG3 (LET_EXPR (arg_node)))
                                          == N_array)
                                           ? ARRAY_TYPE (PRF_ARG3 (LET_EXPR (arg_node)))
                                           : ID_TYPE (PRF_ARG3 (LET_EXPR (arg_node))),
                                         0, TRUE)));
                }
            }

            break;

        default:
            /* do nothing */
            break;
        }
    }

    if (N_ok == NODE_TYPE (arg_info->node[0])) {
#ifndef DBUG_OFF
        db_str = Type2String (type, 0, TRUE); /* only used while debuging */
        DBUG_PRINT ("TYPE", ("infered type : %s", db_str));
        db_str = Free (db_str);
#endif

        tmp = type;
        if (NULL == ids && T_void == TYPES_BASETYPE (type))
            type = type->next;

        while (type && ids) {
            next_type = type->next;
            type->next = NULL; /* this has to be done, because we'll check and
                                * add only this one and not the following too
                                */

            elem = LookupVar (ids->id);
            if (elem) {
                /* ids->id was defined before, so compare infered and stored
                 * (declared) type
                 */
                /*
                 * sbs: switched the prim_fun_flag from -1 to 0 since we
                 * otherwise get an error in UpdateType for the following example:
                 */
                switch (CompatibleTypes (VARDEC_OR_ARG_TYPE (elem->node), type, 0,
                                         NODE_LINE (arg_node))) {
                case CMP_incompatible: /* 0 */
                    ABORT (NODE_LINE (arg_node),
                           ("Incompatible types in declaration (%s) "
                            "and usage (%s) of variable '%s`",
                            Type2String (VARDEC_OR_ARG_TYPE (elem->node), 0, TRUE),
                            Type2String (type, 0, TRUE), ids->id));
                    break;
                case CMP_one_unknown_shape: /* 2*/
#ifdef KNOWN_SHAPE_ONLY
                    DBUG_ASSERT (0, "CompatibleTypes returns CMP_one_unknown_shape");
#else
                    DBUG_ASSERT (SAC_PRG != kind_of_file, "CompatibleTypes returns"
                                                          " CMP_one_unknown_shape");
#endif /* KNOWN_SHAPE_ONLY */
                    break;
                case CMP_equal: /* 1 */
                    ids->node = elem->node;
                    DBUG_PRINT ("REF", ("added reference" F_PTR " from %s to %s",
                                        IDS_VARDEC (ids), ids->id,
                                        VARDEC_OR_ARG_NAME (IDS_VARDEC (ids))));
                    break;
                case CMP_both_unknown_shape: /* neu 8.12. */
                    if (kind_of_file != SAC_MOD) {
                        char *str1
                          = Type2String (VARDEC_OR_ARG_TYPE (elem->node), 0, TRUE);
                        char *str2 = Type2String (type, 0, TRUE);
                        WARN (NODE_LINE (arg_node),
                              ("Types in declaration (%s) and usage "
                               "(%s) of variable '%s' have unknown shape",
                               str1, str2, ids->id));
                        str1 = Free (str1);
                        str2 = Free (str2);
                    }
                    break;
                default:
                    DBUG_ASSERT (0, "unknown return value of function CompatibleTypes");
                    break;
                }
            } else {
                if (TYPES_BASETYPE (type) == T_dots) {
                    node *declaration;
                    types *declared_type;

                    declaration
                      = LookupVardec (IDS_NAME (ids), INFO_TC_VARDEC (arg_info));
                    if (declaration == NULL) {
                        declaration = LookupArg (IDS_NAME (ids),
                                                 INFO_TC_FUNDEF (arg_info)->node[2]);
                        if (declaration == NULL)
                            ABORT (NODE_LINE (arg_node),
                                   ("Unable to infer type of variable %s",
                                    IDS_NAME (ids)));
                    }
                    declared_type = DupAllTypes (VARDEC_TYPE (declaration));
                    AddIdToStack (ids, declared_type, arg_info, NODE_LINE (arg_node));
                } else {
                    AddIdToStack (ids, type, arg_info, NODE_LINE (arg_node));
                }
            }

            type->next = next_type; /* we don't want to destroy the infered types,
                                     * so we have to put back this pointer */
            ids = ids->next;
            if (TYPES_BASETYPE (type) != T_dots)
                type = next_type;
        } /* while */

        if (type && (TYPES_BASETYPE (type) == T_dots)) {
            type = NULL;
        }

        if (type) {
            char *str1, *str2, *str3;
            int str3_len, i;

            str1 = Type2String (type, 0, TRUE);
            str2 = Type2String (tmp, 0, TRUE); /* infered type */
            str3_len
              = strlen (str2) - strlen (str1) - 2; /* -2 to delete comma and space */
            /* in str3 the type of the left side of a let-statement will be
             * stored
             */
            if (str3_len <= 0) {
                /* dkr: 'str3_len' is < 0 if the LHS is void!! */
                str3 = "void";
            } else {
                str3 = (char *)Malloc (sizeof (char) * (str3_len + 1));
                for (i = 0; i < str3_len; i++)
                    str3[i] = str2[i];
                str3[str3_len] = '\0';
            }

            ABORT (NODE_LINE (arg_node), ("Different types (%s != %s)", str3, str2));
        } else if (ids) {
            char *str1;

            str1 = Type2String (tmp, 0, TRUE);
            ABORT (NODE_LINE (arg_node), ("Different types (%s,... != %s)", str1, str1));
        }
        DBUG_ASSERT ((!type && !ids), "type or ids are not NULL");

        /* and now free infered types */
        /*      for(type=tmp; type != NULL; type=type->next)
         FREE_TYPES(type);
         */
        FreeAllTypes (tmp);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : TI_prf
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : returns the type of the expression behind 1) if one
 *                  can derive it, NULL otherwise
 *  global vars   : filename
 *  internal funs : TI , Malloc, FindFun
 *  external funs : Type2String
 *  macros        : DBUG...,, ABORT, NULL, Free
 *
 *  remarks       :- result must be freed if one does not use it any more
 *                 - if arg_node contains to a userdefined function( overloading
 *                   of primitive function) then an arg_node will be converted to
 *                   an N_ap-node
 *
 */

static types *
TI_prf (node *arg_node, node *arg_info)
{
    prim_fun_tab_elem *prf_p;
    types *ret_type, **arg_type = NULL;
    node *current_args;
    int i, type_c_tag, prf_tag, count_args = 0;
    void *fun_p;

#ifndef DBUG_OFF
    char *db_str;
#endif

    DBUG_ENTER ("TI_prf");
    DBUG_ASSERT (N_prf == NODE_TYPE (arg_node), "wrong node in TI_prf");

    /* count number of current arguments */
    current_args = PRF_ARGS (arg_node);
    if (NULL != current_args) {
        while (EXPRS_NEXT (current_args) != NULL) {
            count_args++;
            current_args = EXPRS_NEXT (current_args);
        }
        if ((EXPRS_NEXT (current_args) == NULL) && (EXPRS_EXPR (current_args) != NULL))
            count_args++;
        arg_type = (types **)Malloc (sizeof (types) * count_args);
    }

    /* store type of the arguments in arg_type[] */
    current_args = PRF_ARGS (arg_node);
    for (i = 0; i < count_args; i++) {
        arg_type[i] = TypeInference (EXPRS_EXPR (current_args), arg_info);
        if (NULL == arg_type[i]) {
            ABORT (NODE_LINE (arg_node), ("%d. argument of function '%s` "
                                          "is not inferable",
                                          (i + 1), mdb_prf[arg_node->info.prf]));
        }
        current_args = EXPRS_NEXT (current_args);
    }

    prf_tag = PRF_PRF (arg_node);
    fun_p = FindFun (NULL, NULL, arg_type, count_args, arg_info, NODE_LINE (arg_node),
                     &prf_tag);
    DBUG_ASSERT (NULL != fun_p, "fun_p is NULL");

    if ((prf)prf_tag == PRF_PRF (arg_node)) {
        prf_p = (prim_fun_tab_elem *)fun_p;
        type_c_tag = prf_p->node->info.prf_dec.tag;
        PRF_PRF (arg_node) = prf_p->new_prf;

        switch (type_c_tag) {
#include "prim_fun_tt.mac"
        default: {
            ret_type = NULL; /* just to please the compiler 8-) */
            DBUG_ASSERT (0, "wrong type_class_tag");
            break;
        }
        }

        if (TYPES_BASETYPE (ret_type) == T_unknown) {
            ABORT (NODE_LINE (arg_node), ("Primitive function '%s` is applied "
                                          "to wrong arguments",
                                          mdb_prf[arg_node->info.prf]));
        }
    } else {
        /*
         * userdefined primitive function
         * convert N_prf to N_ap
         */
        NODE_TYPE (arg_node) = N_ap;
        AP_NAME (arg_node) = StringCopy (((fun_tab_elem *)fun_p)->id);
        ret_type = TI_fun (arg_node, (fun_tab_elem *)fun_p, arg_info);
        /* set pointer to function declaration */
        AP_FUNDEF (arg_node) = ((fun_tab_elem *)fun_p)->node;
    }

    /* now free the infered typeinformation */
    for (i = 0; i < count_args; i++)
        FREE_TYPES (arg_type[i]);
    arg_type = Free (arg_type);

#ifndef DBUG_OFF
    db_str = Type2String (ret_type, 0, TRUE);
#endif
    DBUG_PRINT ("TYPE", ("return type : %s", db_str));

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : TCreturn
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : checks the infered types of the returnvalue  and the
 *                  the type of current function if it is a return of a function
 *                  puts the infered return_type to
 *                   arg_info->node[2]->info.types, if return containts to with
 *                   loop.
 *
 *  global vars   : filename
 *  internal funs : CompatibleTypes,  UpdateType, TI
 *  external funs : Type2String
 *  macros        : DBUG..., Free, ERROR, ABORT, FAST_CHECK, NULL,  MOD_NAME
 *
 *  remarks       : it is also used for with-loop-return
 *                  if N_return is contains to a function,
 *                   node[3] of corresponding N_fundef is set to this N_return
 *                   node. This is done for use while compilation.
 */

node *
TCreturn (node *arg_node, node *arg_info)
{
    node *exprs;
    types *return_type = NULL, *return_tmp, *fun_tmp, *fun_type;
    fun_tab_elem *fun_p;
    int is_compatible = 1;
    char *fun_name, *fun_mod, *str1, *str2;

    DBUG_ENTER ("TCreturn");
    DBUG_PRINT ("TYPE", ("return of line %d", NODE_LINE (arg_node)));

    exprs = RETURN_EXPRS (arg_node);

#ifdef NOT_EMPTY_RETURN
    DBUG_ASSERT (NULL != exprs, (" return statement without return-value"));
#endif /* NOT_EMPTY_RETURN */
    if (NULL != exprs) {
        /* infer the types of the returnvalue */
        return_type = TypeInference (exprs->node[0], arg_info);
        if (NULL == return_type) {
            ABORT (NODE_LINE (arg_node), ("Return_type cannot be infered"));
        }
        DBUG_ASSERT ((NULL != return_type), "return_type is NULL");
    } else {
        return_type = MakeTypes1 (T_void);
    }

    DBUG_PRINT ("STOP", ("arg_info->node[0]: %s", NODE_TEXT (arg_info->node[0])));

    if (NULL == arg_info->node[2]) {
        return_tmp = return_type;
        if (NULL != exprs)
            exprs = exprs->node[1];

        while (exprs) {
            return_tmp->next = TypeInference (exprs->node[0], arg_info);
            return_tmp = return_tmp->next;
            if (NULL == return_tmp) {
                ERROR (NODE_LINE (arg_node), ("Return_type cannot be infered."));
                CONT_ERROR (("Infered so far: %s", Type2String (return_type, 0, TRUE)));
                ABORT_ON_ERROR;
            }

            DBUG_ASSERT ((NULL != return_tmp), "return_tmp is NULL");
            DBUG_PRINT ("STOP", ("arg_info->node[0]: %s", NODE_TEXT (arg_info->node[0])));

            exprs = exprs->node[1];
        }
        return_tmp->next = NULL;

        /* now we look for function where return belongs to */
        fun_p = LookupFun (NULL, NULL, INFO_TC_FUNDEF (arg_info));
        if (NULL == fun_p) {
            ABORT (NODE_LINE (arg_node),
                   ("Function '%s` undefined",
                    ModName (FUNDEF_MOD (INFO_TC_FUNDEF (arg_info)),
                             FUNDEF_NAME (INFO_TC_FUNDEF (arg_info)))));
        } else {
            fun_type = FUNDEF_TYPES (fun_p->node);
            fun_name = FUNDEF_NAME (fun_p->node);
            fun_mod = FUNDEF_MOD (fun_p->node);
        }

        fun_p->node->node[3] = arg_node; /* set node[3] of N_fundef to this N_return
                                          * node.
                                          */

        fun_tmp = fun_type;
        return_tmp = return_type;
        while (fun_type && return_type && is_compatible) {
            /* sbs: switched the prim_fun_flag from -1 to 0 since we
             * otherwise get an error in UpdateType for the following example:
             *
             * int, double[] f()
             * { return(2, [1,2,3]); }
             */
            is_compatible
              = CompatibleTypes (fun_type, return_type, 0, NODE_LINE (arg_node));
            switch (is_compatible) {
            case CMP_incompatible: /* 0 */
            {
                ABORT (NODE_LINE (arg_node),
                       ("Incompatible types in declaration (%s) "
                        "and  return value (%s) of function '%s`",
                        Type2String (fun_tmp, 0, TRUE), Type2String (return_tmp, 0, TRUE),
                        ModName (fun_mod, fun_name)));
                break;
            }
            case CMP_equal: /* 1*/
                /* types are equal */
                break;
            case CMP_one_unknown_shape: /* 2 */
            {
                UpdateType (fun_type, return_type, NODE_LINE (arg_node));
                str1 = Type2String (fun_tmp, 0, TRUE);

#ifdef SHAPE_NOTE
                NOTE (("%s:%d: NOTE: one return_type of function '%s` has now"
                       " defined shape (%s)",
                       filename, NODE_LINE (arg_node), fun_name, str1));
#endif /*SHAPE_NOTE */

                str1 = Free (str1);
                break;
            }
            case CMP_both_unknown_shape: /* neu 8.12 */
            {
                if (kind_of_file != SAC_MOD) {
                    if (dynamic_shapes == 1) {
                        /*
                         * this is needed for specializing int[] into int[.],...
                         * therefore, it only occurs when compiling non-modules with -ds !
                         */
                        UpdateType (fun_type, return_type, NODE_LINE (arg_node));
                        str1 = Type2String (fun_tmp, 0, TRUE);
#ifdef SHAPE_NOTE
                        NOTE (("%s:%d: NOTE: one return_type of function '%s` has now"
                               " defined shape (%s)",
                               filename, NODE_LINE (arg_node), fun_name, str1));
#endif /*SHAPE_NOTE */
                        str1 = Free (str1);

                    } else {
                        str1 = Type2String (fun_tmp, 0, TRUE);
                        str2 = Type2String (return_type, 0, TRUE);
                        WARN (NODE_LINE (arg_node),
                              ("Types with unknown shape in declaration (%s) and return "
                               "value (%s) of function '%s`",
                               str1, str2, ModName (fun_mod, fun_name)));
                        str1 = Free (str1);
                        str2 = Free (str2);
                    }
                }
                break;
            }
            default: {
                DBUG_ASSERT (0, "unknown return value of function CompatibleTypes");
                break;
            }
            }
            return_type = return_type->next;
            fun_type = fun_type->next;
        }

        if ((return_type != NULL) || (fun_type != NULL)) {
            ABORT (NODE_LINE (arg_node), ("Number of return values does not match "
                                          "declaration of function '%s`",
                                          ModName (fun_mod, fun_name)));
        }

        DBUG_ASSERT ((0 != is_compatible), " is_compatible == 0");

        /* type of current function is now inferred and inserted */
        if (FAST_CHECK == fun_p->tag) {
            fun_p->tag = TMP_CHECKED;
            DBUG_PRINT ("TYPE",
                        ("set typecheck tag of function '%s' to %d"
                         " (TMP_CHECKED)",
                         ModName (FUNDEF_MOD (fun_p->node), FUNDEF_NAME (fun_p->node)),
                         fun_p->tag));
        } else {
            fun_p->tag = IS_CHECKED;
            DBUG_PRINT ("TYPE",
                        ("set typecheck tag of function '%s' to %d "
                         "(IS_CHECKED)",
                         ModName (FUNDEF_MOD (fun_p->node), FUNDEF_NAME (fun_p->node)),
                         fun_p->tag));
        }

        return_type = return_tmp;
        while (NULL != return_type) {
            return_tmp = return_type->next;
            FREE_TYPES (return_type);
            return_type = return_tmp;
        }
    } else {
        /* this return statement contains to a with-loop */
        arg_info->node[2]->info.types = return_type;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : TI_ap
 *  arguments     : 1) argument node (N_ap)
 *                  2) info_node
 *  description   : looks for applied functions and returns the return type
 *                  of the function
 *  global vars   : filename
 *  internal funs : TI, FindFun,  TI_fun
 *  external funs : Malloc, sizeof
 *  macros        : DBUG..., Free, ABORT,  NULL
 *  remarks       : return type can be T_unknown (in this case
 *                   arg_info->node[0]->nodtype should be N_stop)
 *
 */

static types *
TI_ap (node *arg_node, node *arg_info)
{
    fun_tab_elem *fun_p;
    types *return_type, **arg_type = NULL;
    int count_args = 0;
    int tmp, i;
    node *current_args;
    char *fun_name = arg_node->FUN_NAME;
    char *mod_name = arg_node->FUN_MOD_NAME;

    DBUG_ENTER ("TI_ap");

    /* count number of current arguments */
    current_args = AP_ARGS (arg_node);
    if (NULL != current_args) {
        while (EXPRS_NEXT (current_args) != NULL) {
            count_args++;
            current_args = EXPRS_NEXT (current_args);
        }
        if ((EXPRS_NEXT (current_args) == NULL) && (EXPRS_EXPR (current_args)) != NULL) {
            count_args++;
        }
        arg_type = (types **)Malloc (sizeof (types) * count_args);
    }

    /* store type of the arguments in arg_type[] */
    current_args = AP_ARGS (arg_node);
    for (i = 0; i < count_args; i++) {
        arg_type[i] = TypeInference (current_args->node[0], arg_info);
        if (NULL == arg_type[i]) {
            ABORT (NODE_LINE (arg_node), ("%d. argument of function '%s` not inferable",
                                          (i + 1), ModName (mod_name, fun_name)));
        }
        current_args = current_args->node[1];
    }
    tmp = -1;
    fun_p = FindFun (fun_name, mod_name, arg_type, count_args, arg_info,
                     NODE_LINE (arg_node), &tmp);

    if (NULL == fun_p) {
        ABORT (NODE_LINE (arg_node),
               ("Function '%s` not defined", ModName (mod_name, fun_name)));
    }

    if (FUNDEF_BODY (fun_p->node) != NULL) {
        fun_p = TryConstantPropagation (fun_p, AP_ARGS (arg_node));

        if (0 != strcmp (fun_p->id, AP_NAME (arg_node))) {
            /*
             * Constant propagation successfull
             */
            Free (AP_NAME (arg_node));
            AP_NAME (arg_node) = StringCopy (fun_p->id);
            AP_MOD (arg_node) = fun_p->id_mod;
        }
    }

    CheckIfGOonlyCBR (FUNDEF_ARGS (fun_p->node), AP_ARGS (arg_node));
    /*
     * Check if global objects as current arguments in a function application
     * are used for formal reference parameters only.
     */

    DBUG_PRINT ("TYPE",
                ("applied function '%s' has tag %d", arg_node->info.id, fun_p->tag));
    return_type = TI_fun (arg_node, fun_p, arg_info);
    /* return_type can be T_unknown( for example if it is a recursive call and
     * one checks the functions fast (fun_p->tag == FAST_CHECK))
     */
    AP_FUNDEF (arg_node) = fun_p->node; /* set pointer to function declaration */

    /* now free the infered type information */
    for (i = 0; i < count_args; i++) {
        FREE_TYPES (arg_type[i]);
    }

    if (0 < count_args) {
        arg_type = Free (arg_type);
    }

    DBUG_RETURN (return_type);
}

/*
 *
 *  functionname  : TI_array
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : computes the type of an array (N_array).
 *
 *  global vars   : filename
 *  internal funs : Malloc, CompatibleTypes
 *  external funs : Type2String
 *  macros        : DBUG..., NULL, ERROR, ABORT, Free
 *
 *  remarks       : the type_information is stored in arg_node->info.types.
 *
 */

types *
TI_array (node *arg_node, node *arg_info)
{
    int i, n_elem = 0; /* number of array_elements */
    types *return_type, *tmp_type;
    node *elem;
    char *str1, *str2;

#ifndef DBUG_OFF
    char *db_str; /* only used for debugging */
#endif

    DBUG_ENTER ("TI_array");

    return_type = NULL;

    elem = ARRAY_AELEMS (arg_node);
    while (NULL != elem) {
        n_elem++;
        tmp_type = TypeInference (EXPRS_EXPR (elem), arg_info);
        if (NULL == tmp_type) {
            ABORT (NODE_LINE (arg_node),
                   ("%d. element of array cannot be infered", n_elem));
        }

        DBUG_ASSERT ((NULL != tmp_type), "tmp_type is NULL");

        if (NULL == return_type) {
            return_type = tmp_type;
        } else {
            /* CompatibleTypes will be called twice, to infere type of an
             * array that contains 'ints' and 'floats'
             */
            if ((CMP_equal
                 != CompatibleTypes (tmp_type, return_type, -1, NODE_LINE (arg_node)))
                && (CMP_equal
                    != CompatibleTypes (return_type, tmp_type, -1,
                                        NODE_LINE (arg_node)))) /* 1, 1 */ {
                /*
                 *  Elements of Array have incompatible types, result ist an Error. (jhs)
                 */

                str1 = Type2String (tmp_type, 0, TRUE);
                str2 = Type2String (return_type, 0, TRUE);
                ERROR (NODE_LINE (elem),
                       ("Array of type '%s` has element of type '%s`", str2, str1));
                str1 = Free (str1);
                str2 = Free (str2);
                FREE_TYPES (tmp_type);
            } else if ((T_int == TYPES_BASETYPE (return_type)
                        && T_float == TYPES_BASETYPE (tmp_type))
                       || (T_int == TYPES_BASETYPE (return_type)
                           && T_double == TYPES_BASETYPE (tmp_type))
                       || (T_float == TYPES_BASETYPE (return_type)
                           && T_double == TYPES_BASETYPE (tmp_type))) {
                /*
                 *  Elements of Array have compatible types (until now), and the tmp_type
                 * of the last Element viewed covers the return_type. Where doubles cover
                 * floats and int, floats cover ints. (jhs)
                 */
                FREE_TYPES (return_type);
                return_type = tmp_type;
            } else {
                /*
                 *  Elements of Array have compatible types (until now), and the tmp_type
                 * of the last Element viewed is covered by the return_type. Where doubles
                 * cover floats and int, floats cover ints. (jhs)
                 */
                FREE_TYPES (tmp_type);
            }
        }
        elem = EXPRS_NEXT (elem);
    } /* while */

    if (n_elem == 0) {
        /*
         *  This produces a virtual type for the not existing arguments of this empty
         * array.
         */
        DBUG_PRINT ("TYPE", ("empty array found"));
        return_type = MakeTypes1 (T_nothing);
    }

    if (NULL != return_type) {
        if (0 == TYPES_DIM (return_type)) {
            TYPES_DIM (return_type) = 1;
            TYPES_SHPSEG (return_type) = MakeShpseg (NULL);
            SHPSEG_SHAPE (TYPES_SHPSEG (return_type), 0) = n_elem;
        } else if (-1 != TYPES_DIM (return_type)) {
            DBUG_ASSERT ((TYPES_DIM (return_type) + 1 <= SHP_SEG_SIZE), " dimension"
                                                                        " out of range");
            for (i = TYPES_DIM (return_type) - 1; i >= 0; i--) {
                SHPSEG_SHAPE (TYPES_SHPSEG (return_type), i + 1)
                  = SHPSEG_SHAPE (TYPES_SHPSEG (return_type), i);
            }
            SHPSEG_SHAPE (TYPES_SHPSEG (return_type), 0) = n_elem;
            TYPES_DIM (return_type) += 1;
        }
#ifndef DBUG_OFF
        db_str = Type2String (return_type, 0, TRUE);
        DBUG_PRINT ("TYPE", ("type of array: %s", db_str));
        db_str = Free (db_str);
#endif
        /* store type of array in ARRAY_TYPE(arg_node) */
        ARRAY_TYPE (arg_node) = DupAllTypes (return_type);
    } else {
        ABORT (NODE_LINE (arg_node), ("Type of array not inferable"));
    }

    DBUG_RETURN (return_type);
}

/*
 *
 *  functionname  : TCcond
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : manages typechecking of a conditional
 *                  tests the type of the condition
 *
 *  global vars   : tos,filename
 *  internal funs : TI, LookupFun
 *  external funs : Trav
 *  macros        : DBUG...,CHECKING, IS_CHECKED, F_PTR, ERROR, ABORT,
 *                  Free
 *
 *  remarks       :
 *
 */

node *
TCcond (node *arg_node, node *arg_info)
{
    int i, check_again = 0;
    stack_elem *old_tos;
    fun_tab_elem *fun_p;
    node *rest_node;
    nodetype old_status;
    types *expr_type;

    DBUG_ENTER ("TCcond");

    old_tos = tos;

    DBUG_ASSERT (N_cond == NODE_TYPE (arg_node), "wrong nodetype");

    old_status = arg_info->node[0]->nodetype;
    rest_node = INFO_TC_NEXTASSIGN (arg_info);

    expr_type = TypeInference (COND_COND (arg_node), arg_info);
    if (NULL == expr_type) {
        ABORT (NODE_LINE (arg_node), ("Type not inferable"));
    }
    DBUG_ASSERT ((NULL != expr_type), "expr_type is NULL ");
    DBUG_PRINT ("STOP", ("arg_info->node[0]: %s", NODE_TEXT (arg_info->node[0])));

    if ((T_bool != TYPES_BASETYPE (expr_type)) || (TYPES_DIM (expr_type) != SCALAR)) {
        ERROR (NODE_LINE (arg_node),
               ("Type of condition (%s) is not bool", Type2String (expr_type, 0, TRUE)));
    }

    /* now free the infered type-information */
    FREE_TYPES (expr_type);

    fun_p = LookupFun (NULL, NULL, INFO_TC_FUNDEF (arg_info));
    DBUG_ASSERT ((NULL != fun_p), "fun_p is NULL");
    if ((CHECKING == fun_p->tag) || (IS_CHECKED == fun_p->tag)) {
        for (i = 1; i < nnode[NODE_TYPE (arg_node)]; i++) {
            tos = old_tos;
            INFO_TC_STATUS (arg_info) += 1;
#ifndef DBUG_OFF
            if (1 == i) {
                DBUG_PRINT ("TYPE", ("checking THEN part"));
            } else {
                DBUG_PRINT ("TYPE", ("checking ELSE part"));
            }
#endif
            Trav (arg_node->node[i], arg_info);
            INFO_TC_STATUS (arg_info) -= 1;
            if (N_stop == arg_info->node[0]->nodetype) {
                check_again += i;
                arg_info->node[0]->nodetype = old_status;
            } else if ((NULL != rest_node) && ((1 == check_again) || (1 == i))) {
                DBUG_PRINT ("TYPE", ("checking rest"));
                Trav (rest_node, arg_info);
            }
        }
        if ((1 == check_again) || (2 == check_again)) {
            tos = old_tos;
            arg_info->node[0]->nodetype = old_status;
            INFO_TC_STATUS (arg_info) += 1;
#ifndef DBUG_OFF
            if (1 == check_again) {
                DBUG_PRINT ("TYPE", ("checking THEN part again"));
            } else {
                DBUG_PRINT ("TYPE", ("checking ELSE part again"));
            }
#endif
            Trav (arg_node->node[check_again], arg_info);
            INFO_TC_STATUS (arg_info) -= 1;
            if (N_stop == arg_info->node[0]->nodetype) {
                ABORT (NODE_LINE (arg_node),
                       ("Uninferable functions in then or else part"));
            }
        } else if (3 == check_again) {
            ABORT (NODE_LINE (arg_node), ("Uninferable functions in then or else part"));
        }
    } else {
        for (i = 1; i < nnode[NODE_TYPE (arg_node)]; i++) {
            tos = old_tos;
            INFO_TC_STATUS (arg_info) += 1;

            if (1 == i) {
                DBUG_PRINT ("TYPE", ("checking THEN part"));
            } else {
                DBUG_PRINT ("TYPE", ("checking ELSE part"));
            }

            Trav (arg_node->node[i], arg_info);
            INFO_TC_STATUS (arg_info) -= 1;
            if (N_stop == arg_info->node[0]->nodetype) {
                check_again += i;
                arg_info->node[0]->nodetype = old_status;
            } else
                break;
        }
        if (3 == check_again)
            ABORT (NODE_LINE (arg_node), ("Uninferable functions in then or else part"));
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : TCblock
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : traverses the body of an N_block node
 *                  also traverses the vardecs in order to lookup user-defined
 *                  types.
 *
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,
 *
 *  remarks       : instanciates INFO_TC_LASSIGN( arg_info) with pointer
 *                  to N_block
 *
 */

node *
TCblock (node *arg_node, node *arg_info)
{
    node *old_last_assign, *old_current_assign, *old_next_assign;

    DBUG_ENTER ("TCblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }
    ABORT_ON_ERROR;

    /* store last, current and next assignment for reentrant calls. (jhs) */
    old_last_assign = INFO_TC_LASSIGN (arg_info);
    old_current_assign = INFO_TC_CURRENTASSIGN (arg_info);
    old_next_assign = INFO_TC_NEXTASSIGN (arg_info);

    /*
     *  LASSIGN must be set here, because the next assignment (the first one
     *  of the block, does not know it's parent block.
     */
    INFO_TC_LASSIGN (arg_info) = arg_node;
    /* For the traversal of the assignments I have to
     * ignore the return-value, since TI_ap in some cases
     * inserts N_assign's with N_annotate's!
     * (compare TCassign!)
     */
    Trav (BLOCK_INSTR (arg_node), arg_info);

    /* restore last, current and next, see comment above */
    INFO_TC_LASSIGN (arg_info) = old_last_assign;
    INFO_TC_CURRENTASSIGN (arg_info) = old_current_assign;
    INFO_TC_NEXTASSIGN (arg_info) = old_next_assign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *TCvardec(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

node *
TCvardec (node *arg_node, node *arg_info)
{
    node *t_def;

    DBUG_ENTER ("TCvardec");

    if ((VARDEC_BASETYPE (arg_node) == T_user)
        || (VARDEC_BASETYPE (arg_node) == T_hidden)) {
        t_def = LookupType (VARDEC_TNAME (arg_node), VARDEC_TMOD (arg_node),
                            NODE_LINE (arg_node));
        if (t_def == NULL) {
            if (VARDEC_TMOD (arg_node) == NULL) {
                ERROR (NODE_LINE (arg_node),
                       ("No definition of type '%s`", VARDEC_TNAME (arg_node)));
            } else {
                ERROR (NODE_LINE (arg_node),
                       ("No definition of type '%s:%s`", VARDEC_TMOD (arg_node),
                        VARDEC_TNAME (arg_node)));
            }
        } else {
            VARDEC_TMOD (arg_node) = TYPEDEF_MOD (t_def);
            VARDEC_TDEF (arg_node) = t_def;
        }
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : TCassign
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : manages traversing of children nodes of N_assign node
 *                  - if arg_info->node[0] is N_stop, traversing will be
 *                    stopped
 *
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,
 *
 *  remarks       : info_node[2] is set to next N_assign while traversing
 *                  arg_node->node[0] , if arg_node->node[1] != NULL
 *
 */
node *
TCassign (node *arg_node, node *arg_info)
{
    node *old_last_assign, *old_current_assign, *old_next_assign;

    DBUG_ENTER ("TCassign");
    DBUG_ASSERT (N_assign == NODE_TYPE (arg_node), "wrong nodetype");

    /*
     *  secure last, current and next assign, so this routine can be called reentrant
     *  without deleting the context of calling functions. (jhs)
     *
     *  an older comment on this was:
     *  The current asssign back reference must be stacked here due to nested
     *  N_assign nodes in the presence of with-loop bodies.
     */
    old_last_assign = INFO_TC_LASSIGN (arg_info);
    old_current_assign = INFO_TC_CURRENTASSIGN (arg_info);
    old_next_assign = INFO_TC_NEXTASSIGN (arg_info);

    /* LASSIGN comes from calling function, may be NULL! */
    INFO_TC_CURRENTASSIGN (arg_info) = arg_node;
    INFO_TC_NEXTASSIGN (arg_info) = ASSIGN_NEXT (arg_node);

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        if (N_stop != arg_info->node[0]->nodetype) {
            INFO_TC_LASSIGN (arg_info) = arg_node;
            /* For the traversal of the next assignments I have to
             * ignore the return-value, since TI_ap in some cases
             * inserts N_assign's with N_annotate's!
             * (compare TCblock!)
             */
            Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    /* restore the xxxASSIGN of arg_info, see comment while storing */
    INFO_TC_LASSIGN (arg_info) = old_last_assign;
    INFO_TC_CURRENTASSIGN (arg_info) = old_current_assign;
    INFO_TC_NEXTASSIGN (arg_info) = old_next_assign;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : TCdo
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : manages typechecking of do-loop
 *                  tests type of termination condition
 *
 *  global vars   : filename
 *  internal funs : TI
 *  external funs : Trav, Type2String
 *  macros        : DBUG..., ERROR, ABORT, Free, NULL
 *
 *  remarks       :
 *
 */

node *
TCdo (node *arg_node, node *arg_info)
{
    types *expr_type;

    DBUG_ENTER ("TCdo");

    /* traverse the body of do-loop */
    Trav (DO_BODY (arg_node), arg_info);

    /* check type of termination condition of do-loop */
    expr_type = TypeInference (DO_COND (arg_node), arg_info);
    if (NULL == expr_type) {
        ABORT (NODE_LINE (arg_node), ("Type not inferable"));
    } else if ((T_bool != TYPES_BASETYPE (expr_type))
               || (TYPES_DIM (expr_type) != SCALAR))
        ERROR (NODE_LINE (DO_COND (arg_node)),
               ("Type of loop condition is '%s` but must be 'bool`",
                Type2String (expr_type, 0, TRUE)));

    /* now free infered type_information */
    FREE_TYPES (expr_type);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : TCwhile
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : manages typechecking of while-loop
 *                  tests termination condition of while-loop
 *
 *  global vars   : filename
 *  internal funs : TI
 *  external funs : Trav, Type2String
 *  macros        : DBUG..., ERROR
 *
 *  remarks       :
 *
 */

node *
TCwhile (node *arg_node, node *arg_info)
{
    stack_elem *old_tos;
    types *expr_type;

    DBUG_ENTER ("TCwhile");

    /* store scope stack pointer */
    old_tos = tos;

    /* check the termination condition of while-loop */
    expr_type = TypeInference (WHILE_COND (arg_node), arg_info);
    if (!expr_type)
        ABORT (NODE_LINE (arg_node), ("Type not inferable"));

    if ((T_bool == TYPES_BASETYPE (expr_type)) && (TYPES_DIM (expr_type) == SCALAR))
        /* traverse body of while_loop */
        Trav (WHILE_BODY (arg_node), arg_info);
    else
        ERROR (NODE_LINE (WHILE_COND (arg_node)),
               ("Type of loop condition is '%s` but must be 'bool`",
                Type2String (expr_type, 0, TRUE)));

    /* restore scope stack by setting 'tos' pointer */
    tos = old_tos;

    /* now free infered type_information */
    FREE_TYPES (expr_type);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : TI_cast
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : computes the type of a casted expression
 *
 */

static types *
TI_cast (node *arg_node, node *arg_info)
{
    types *ret_type, *type, *tmp, *inf_type, *free_type;
    int is_comp, i = 1;
    node *t_node;

    DBUG_ENTER ("TI_cast");

    /* store casted types in ret_type */
    ret_type = DupAllTypes (CAST_TYPE (arg_node));

    if (TYPES_BASETYPE (ret_type) == T_user) {
        t_node = LookupType (TYPES_NAME (ret_type), TYPES_MOD (ret_type),
                             NODE_LINE (arg_node));

        if (NULL == t_node) {
            if (TYPES_MOD (ret_type) == NULL) {
                ABORT (NODE_LINE (arg_node),
                       ("No definition of type '%s`", TYPES_NAME (ret_type)));
            } else {
                ABORT (NODE_LINE (arg_node),
                       ("No definition of type '%s:%s`", TYPES_MOD (ret_type),
                        TYPES_NAME (ret_type)));
            }
        } else {
            TYPES_MOD (ret_type) = TYPEDEF_MOD (t_node);
            CAST_TMOD (arg_node) = TYPEDEF_MOD (t_node);
            CAST_TDEF (arg_node) = t_node;
        }
    }

    inf_type = TypeInference (CAST_EXPR (arg_node), arg_info);
    if (NULL == inf_type)
        switch (arg_node->nodetype) {
        case N_id: {
            ABORT (NODE_LINE (arg_node), ("Variable '%s` undefined", ID_NAME (arg_node)));
            break;
        }
        case N_ap: {
            ABORT (NODE_LINE (arg_node),
                   ("Type of function '%s` not inferable",
                    ModName (arg_node->FUN_MOD_NAME, arg_node->FUN_NAME)));
            break;
        }
        default: {
            ABORT (NODE_LINE (arg_node),
                   ("Type of expression '%s` not inferable", NODE_TEXT (arg_node)));
            break;
        }
        }

    /* check whether casts are compatible */
    type = ret_type;
    tmp = inf_type;
    while (NULL != tmp) {
        is_comp = CompatibleTypes (type, tmp, 0, NODE_LINE (arg_node));
        if (CMP_incompatible == is_comp) /* 0 */ {
            ERROR (NODE_LINE (arg_node),
                   ("%d. cast has incompatible type: inferred type: %s", i,
                    Type2String (tmp, 0, TRUE)));
        } else if (CMP_one_unknown_shape == is_comp) /* 2 */ {
            /* change type for return  (infered type of N_cast) */
            UpdateType (type, tmp, NODE_LINE (arg_node));
            /* change type in N_cast */
            UpdateType (CAST_TYPE (arg_node), tmp, NODE_LINE (arg_node));
        }
        if (NULL != type->next)
            type = type->next;
        else {
            free_type = tmp;
            tmp = tmp->next;
            FREE_TYPES (free_type);
            break;
        }
        free_type = tmp;
        arg_node = CAST_EXPR (arg_node);
        tmp = tmp->next;
        FREE_TYPES (free_type);
        i++;
    }
    /* check whether errors occure before */
    ABORT_ON_ERROR;

    {
        node *sbs_tmp;

        sbs_tmp = NodeBehindCast (arg_node);
        if ((NODE_TYPE (sbs_tmp) == N_array) && (ARRAY_BASETYPE (sbs_tmp) == T_nothing)) {
            /*
             * This is a cast of an empty array. So we have to update
             * ARRAY_TYPE( sbs_tmp) with a duplicate of type!
             */
            FreeOneTypes (ARRAY_TYPE (sbs_tmp));
            ARRAY_TYPE (sbs_tmp) = DupAllTypes (ret_type);
        }
    }

    /* check whether there are to many casts */
    if ((NULL != type->next) && (NULL == tmp))
        ABORT (NODE_LINE (arg_node), ("Too many casts"))
    if (NULL != tmp)
        type->next = tmp;

#ifndef DBUG_OFF
    if (NULL != ret_type) {
        char *db_str = Type2String (ret_type, 0, TRUE);
        DBUG_PRINT ("TYPE", ("%s", db_str));
    }
#endif

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : TCobjdef
 *  arguments     : 1) pointer to objdef node
 *                  2) arg_info unused
 *  description   : typecheck for object definitions
 *                  -checks name clashes with types, functions, other objects
 *                  -searches for type in type table
 *                  -checks if type is user-defined and unique
 *                  -the module name of the object is set correctly
 *  global vars   :
 *  internal funs : LookupType, LookupFun
 *  external funs :
 *  macros        : CMP_OBJ_OBJDEF, ERROR, ABORT
 *
 *  remarks       :
 *
 */

node *
TCobjdef (node *arg_node, node *arg_info)
{
    node *tdef, *check_types, *check_objs;
    fun_tab_elem *check_funs;

    DBUG_ENTER ("TCobjdef");

    /*
     *  First, we check if there's a user-defined type with the same name
     *  as this global object.
     */

    check_types = LookupType (OBJDEF_NAME (arg_node), NULL, NODE_LINE (arg_node));
    if (check_types != NULL) {
        ERROR (NODE_LINE (arg_node),
               ("Global object and user-defined type have same name '%s`",
                OBJDEF_NAME (arg_node)));
    }

    /*
     *  Second, we check if there's a function with the same name as this
     *  global object.
     */

    check_funs = LookupFun (OBJDEF_NAME (arg_node), NULL, NULL);
    if (check_funs != NULL) {
        ERROR (NODE_LINE (arg_node), ("Global object and function have same name '%s`",
                                      OBJDEF_NAME (arg_node)));
    }

    /*
     *  Third, we check if there's another global object with the same name
     *  as this global object.
     */

    check_objs = OBJDEF_NEXT (arg_node);
    while (check_objs != NULL) {
        if (CMP_OBJ_OBJDEF (OBJDEF_NAME (arg_node), OBJDEF_MOD (arg_node), check_objs)
            == 1) {
            ERROR (NODE_LINE (arg_node),
                   ("Global object '%s` defined twice",
                    ModName (OBJDEF_MOD (arg_node), OBJDEF_NAME (arg_node))));
        }
        check_objs = OBJDEF_NEXT (check_objs);
    }

    /*
     *  Now, we check if the type of the global object is an existing
     *  user-defined and unique type.
     */

    if ((OBJDEF_BASETYPE (arg_node) != T_user) || (OBJDEF_DIM (arg_node) != 0)) {
        ABORT (NODE_LINE (arg_node),
               ("Type of global object '%s` must be a class",
                ModName (OBJDEF_MOD (arg_node), OBJDEF_NAME (arg_node))));
    } else {
        tdef = LookupType (OBJDEF_TNAME (arg_node), OBJDEF_TMOD (arg_node),
                           NODE_LINE (arg_node));
        if (tdef == NULL) {
            ABORT (NODE_LINE (arg_node),
                   ("Type of global object '%s` unknown",
                    ModName (OBJDEF_MOD (arg_node), OBJDEF_NAME (arg_node))));
        } else {
            if (TYPEDEF_ATTRIB (tdef) != ST_unique) {
                ABORT (NODE_LINE (arg_node),
                       ("Type of global object '%s` must be a class",
                        ModName (OBJDEF_MOD (arg_node), OBJDEF_NAME (arg_node))));
            }
            TYPES_TDEF (OBJDEF_TYPE (arg_node)) = tdef;
        }
    }

    /*
     *  The module name of the object type is set correctly.
     */

    OBJDEF_TMOD (arg_node) = TYPEDEF_MOD (tdef);

    /*
     *  At last, we recursively traverse the remaining object definitions.
     */

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 *  function:
 *    int isBoundEmpty (node *arg_node, node *bound_node)
 *
 *  description:
 *    checks whether a bound (specified by bound_node) of a withloop is an
 *    empty array, constants TRUE or FALSE are returned.
 *    arg_node is needed only for the line number in abort.
 *
 ******************************************************************************/

static int
isBoundEmpty (node *arg_node, node *bound_node)
{
    int i, elements;
    types *ltypes;

    DBUG_ASSERT (((NODE_TYPE (bound_node) == N_id) || (NODE_TYPE (bound_node) == N_array)
                  || (NODE_TYPE (bound_node) == N_cast)),
                 ("bound_node is neither N_id, N_array nor N_cast"));

    if (NODE_TYPE (bound_node) == N_id) {
        /* N_id node => elements are eventually annotated */
        if (ID_ISCONST (bound_node) && (ID_VECLEN (bound_node) == 0)) {
            return (TRUE);
        } else {
            /* They are not annotated but perhaps the shape is empty?
             * Therefore count elements. */
            ltypes = ID_TYPE (bound_node);
            elements = 1;
            for (i = 0; i < TYPES_DIM (ltypes); i++) {
                elements = elements * SHPSEG_SHAPE (TYPES_SHPSEG (ltypes), i);
            }
            /* Shape empty? */
            if (elements == 0) {
                return (TRUE);
            } else {
                return (FALSE);
            }
        }
    } else if (NODE_TYPE (bound_node) == N_array) {
        /* iff there are no elements in array, it must be empty */
        if (ARRAY_AELEMS (bound_node) == NULL) {
            return (TRUE);
        } else {
            return (FALSE);
        }
    } else if (NODE_TYPE (bound_node) == N_cast) {
        /* behind the cast must be an array somewhere,
         * so we call this by recursion, until we reach
         * type of another, we can tell something about. */
        return (isBoundEmpty (arg_node, CAST_EXPR (bound_node)));
    } else {
        return (FALSE);
    }
}

/******************************************************************************
 *
 * function:
 *   types *TI_Nwith(node *arg_node, node *arg_info)
 *
 * description:
 *   this is the starting point for typechecking the new with-loops.
 *   First, the generator and the body are checked. Then, dependent on the
 *   withop, consistency checks are done and the return type is computed.
 *
 * remarks:
 *   The base array (modarray/genarray) resp. the neutral elements (fold)
 *   must not be typechecked within the scope of the WL body. So these
 *   types are infered first.
 *
 ******************************************************************************/

static types *
TI_Nwith (node *arg_node, node *arg_info)
{
    stack_elem *old_tos;
    types *generator_type, *base_array_type, *body_type, *neutral_type;
    node *tmpn, *withop, *new_fundef, *new_expr, *new_shp = NULL;
    node *tmpass;
    ids *mem_lhs;
    int i;

    node *generator, *lowerbound, *upperbound, *assignment;
    int lowerbound_empty,
      upperbound_empty; /* to be used as boolean, telling whether the bounds are empty
                           arrays or not */

    DBUG_ENTER ("TI_Nwith");

    /* save old tos to assure that variables which area defined in the WL
       (generatorvars, inside the body) are unknown afterwards. */
    old_tos = tos;

    /* First we try to infere the type of the array to modify (modarray)
       or the shape of the base-array (genarray). This cannot be done
       for the withop fold. */
    base_array_type = NULL;
    neutral_type = NULL;
    if (WO_genarray == NWITHOP_TYPE (NWITH_WITHOP (arg_node))) {
        base_array_type = TI_Ngenarray (NWITHOP_SHAPE (NWITH_WITHOP (arg_node)), arg_info,
                                        &(NWITHOP_SHAPE (NWITH_WITHOP (arg_node))));
        /*
         * attention: The base type in case of genarray is infered from the
         * constant argument vector. This vector if int[.] of course, but this
         * is not returned. E.g. if the vector is int[2] = [2,3] the base_array_type
         * is int[2,3]. Attend that ALWAYS the simpletype int is returned because
         * we cannot infere the base type from the given vector. The real simpletype
         * has to be infered from the CEXPR later.
         */
        if (!KNOWN_SHAPE (TYPES_DIM (base_array_type))
            && (DOT_ISSINGLE (NWITH_BOUND2 (arg_node)))) {
            /* new_shp is needed for substituting "." as upper bound only!! */
            new_shp = DupTree (NWITHOP_SHAPE (NWITH_WITHOP (arg_node)));
        }
    } else if (WO_modarray == NWITHOP_TYPE (NWITH_WITHOP (arg_node))) {
        base_array_type
          = TypeInference (NWITHOP_ARRAY (NWITH_WITHOP (arg_node)), arg_info);
        if (!base_array_type)
            ABORT (NODE_LINE (NWITHOP_ARRAY (NWITH_WITHOP (arg_node))),
                   ("Array component of 'modarray` not inferable"));
        if (!KNOWN_SHAPE (TYPES_DIM (base_array_type))
            && (DOT_ISSINGLE (NWITH_BOUND2 (arg_node)))) {
            /* new_shp is needed for substituting "." as upper bound only!! */
            new_shp
              = MakePrf (F_shape,
                         MakeExprs (DupTree (NWITHOP_ARRAY (NWITH_WITHOP (arg_node))),
                                    NULL));
        }
    } else if (NWITHOP_NEUTRAL (NWITH_WITHOP (arg_node))) /* only if exists */ {
        neutral_type
          = TypeInference (NWITHOP_NEUTRAL (NWITH_WITHOP (arg_node)), arg_info);
        if (!neutral_type)
            ABORT (NODE_LINE (NWITHOP_NEUTRAL (NWITH_WITHOP (arg_node))),
                   ("Neutral element of 'fold` not inferable"));
    }

    /*
     *  Among other things: last step of normalization to (lb <= iv < ub).
     *  While for not . combined operators this was done in flatten the rest
     *  is done now.
     */
    generator_type = TI_Npart (NWITH_PART (arg_node), base_array_type, new_shp, arg_info);

    /*
     *  If compiling a program file, there now should be no . in any generator boundary.
     *  So check for that.
     */

    DBUG_ASSERT (((!DOT_ISSINGLE (NWITH_BOUND1 (arg_node))) || (kind_of_file != F_prog)),
                 "lower bound is still a N_dot node!");
    DBUG_ASSERT (((!DOT_ISSINGLE (NWITH_BOUND2 (arg_node))) || (kind_of_file != F_prog)),
                 "upper bound is still a N_dot node!");

    /* In case of genarray() check whether generator and base array fit. */
    if (WO_genarray == NWITHOP_TYPE (NWITH_WITHOP (arg_node))
        && SCALAR < TYPES_DIM (generator_type) && SCALAR < TYPES_DIM (base_array_type)
        && TYPES_SHAPE (generator_type, 0) != TYPES_DIM (base_array_type)) {
        ABORT (NODE_LINE (NWITHOP_NEUTRAL (NWITH_WITHOP (arg_node))),
               ("Generator and base shape are incompatible. "
                "Expected generator of type int[%d]",
                TYPES_DIM (base_array_type)));
    }

    /*  we now have the type of the generator and, in case of genarray or
     *  modarray, the type of the base array, too.
     *  The next step is to typecheck the body (CBLOCK) and the additional
     *  expression (CEXPR).
     */
    tmpn = MakeArg (NULL, NULL, ST_regular, ST_regular, NULL);
    if (arg_info->node[2])
        ARG_NEXT (tmpn) = arg_info->node[2];
    arg_info->node[2] = tmpn;

    mem_lhs = INFO_TC_LHSVARS (arg_info);
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    INFO_TC_LHSVARS (arg_info) = mem_lhs;
    /* the return type of the code block is in arg_info->node[2]->info.types.
       This is a pointer to the VARDEC-type, so don't free it resp.
       duplicate it if necessary. */

    withop = NWITH_WITHOP (arg_node);
    body_type = arg_info->node[2]->info.types; /* This is a copy of the CEXPR type. */
    if (NULL == body_type)
        ABORT (NODE_LINE (arg_node), ("Type of with loop body is not inferable"));

    switch (NWITHOP_TYPE (withop)) {
    case WO_genarray:
        TYPES_BASETYPE (base_array_type) = TYPES_BASETYPE (body_type);
        if (SCALAR == TYPES_DIM (body_type)) {
            TYPES_NAME (base_array_type) = StringCopy (TYPES_NAME (body_type));
            TYPES_MOD (base_array_type) = TYPES_MOD (body_type);
            FreeOneTypes (body_type);
        } else if (SCALAR > TYPES_DIM (body_type)
                   || SCALAR > TYPES_DIM (base_array_type)) {
            FreeOneTypes (body_type);
            body_type = MakeTypes (TYPES_BASETYPE (base_array_type), UNKNOWN_SHAPE, NULL,
                                   NULL, NULL);
            FreeOneTypes (base_array_type);
            base_array_type = body_type;
        } else {
            TYPES_NAME (base_array_type) = StringCopy (TYPES_NAME (body_type));
            TYPES_MOD (base_array_type) = TYPES_MOD (body_type);
            /* if type of body is non scalar: extend base_array_type */
            for (i = 0; i < TYPES_DIM (body_type); i++) {
                TYPES_SHAPE (base_array_type, TYPES_DIM (base_array_type))
                  = TYPES_SHAPE (body_type, i);
                (TYPES_DIM (base_array_type))++;
            }
            FreeOneTypes (body_type);
        }
        break;
    case WO_modarray:
        ConsistencyCheckModarray (NODE_LINE (withop), base_array_type, generator_type,
                                  body_type);
        FreeOneTypes (body_type);
        break;
    case WO_foldprf:
        base_array_type = TI_Nfoldprf (withop, body_type, neutral_type, arg_info);
        /*
         * Now, we create a pseudo function for the fold-fun.
         * It is needed for the code-generation of the with-loop!!
         * Although it could be regarded as a kind of flatten-step
         * it has to be done here since type-declarations for functions
         * are mendatory in SAC!!
         */
        DBUG_ASSERT ((NODE_TYPE (NWITH_CEXPR (arg_node)) == N_id),
                     "Cexpr of Ncode from with-loop is expected to be an N_id node!");
        DBUG_ASSERT ((INFO_TC_LHSVARS (arg_info) != NULL), "during type-checking "
                                                           "with-loops are expected to "
                                                           "be RHS of let-exprs only!");

        if (NWITHOP_TYPE (withop) == WO_foldprf) {
            new_fundef = CreatePseudoFoldFun (body_type, NULL, NWITHOP_PRF (withop),
                                              IDS_NAME (INFO_TC_LHSVARS (arg_info)),
                                              ID_NAME (NWITH_CEXPR (arg_node)));
            NWITHOP_TYPE (withop) = WO_foldfun;
        } else {
            /*
             * This case happens iff a prf is used in fold, but after
             * inferring the arg-types an overloaded non-prf version
             * has to be used.
             */
            new_fundef = CreatePseudoFoldFun (body_type, NWITHOP_FUN (withop),
                                              F_toi_A, /* this is just a dummy! */
                                              IDS_NAME (INFO_TC_LHSVARS (arg_info)),
                                              ID_NAME (NWITH_CEXPR (arg_node)));
        }

        NWITHOP_FUNDEF (withop) = new_fundef;
        NWITHOP_FUN (withop) = StringCopy (FUNDEF_NAME (new_fundef));
        NWITHOP_MOD (withop) = StringCopy (PSEUDO_MOD_FOLD);

        /* insert new function into fun_table */
        OLD_INSERT_FUN (fun_table, FUNDEF_NAME (new_fundef), FUNDEF_MOD (new_fundef),
                        new_fundef, PLEASE_CHECK, -2);

        /*
         * insert 'new_fundef' into fundef-chain 'pseudo_fold_fundefs'
         */
        FUNDEF_NEXT (new_fundef) = pseudo_fold_fundefs;
        pseudo_fold_fundefs = new_fundef;

        FreeOneTypes (body_type);
        FreeOneTypes (neutral_type);
        break;
    case WO_foldfun:
        base_array_type = TI_Nfoldfun (withop, body_type, neutral_type, arg_info);
        /*
         *  Now, we create a pseudo function for the fold-fun iff this has
         *  not been done yet. ( i.e. if the module-name does not equal PSEUDO_MOD_FOLD!).
         *  The pseudo function is needed for the code-generation of the with-loop!!
         *  Although it could be regarded as a kind of flatten-step
         *  it has to be done here since type-declarations for functions
         *  are mendatory in SAC!!
         */
        if (strcmp (NWITHOP_MOD (withop), PSEUDO_MOD_FOLD) != 0) {
            DBUG_ASSERT ((NODE_TYPE (NWITH_CEXPR (arg_node)) == N_id),
                         "Cexpr of Ncode from with-loop is expected to be an N_id node!");
            DBUG_ASSERT ((INFO_TC_LHSVARS (arg_info) != NULL),
                         "during type-checking with-loops are expected to be RHS of "
                         "let-exprs only!");

            new_fundef = CreatePseudoFoldFun (body_type, NWITHOP_FUN (withop),
                                              F_toi_A, /* this is just a dummy! */
                                              IDS_NAME (INFO_TC_LHSVARS (arg_info)),
                                              ID_NAME (NWITH_CEXPR (arg_node)));

            NWITHOP_FUNDEF (withop) = new_fundef;
            NWITHOP_FUN (withop) = StringCopy (FUNDEF_NAME (new_fundef));
            NWITHOP_MOD (withop) = StringCopy (PSEUDO_MOD_FOLD);

            /* insert new function into fun_table */
            OLD_INSERT_FUN (fun_table, FUNDEF_NAME (new_fundef), FUNDEF_MOD (new_fundef),
                            new_fundef, PLEASE_CHECK, -2);

            /*
             * insert 'new_fundef' into fundef-chain 'pseudo_fold_fundefs'
             */
            FUNDEF_NEXT (new_fundef) = pseudo_fold_fundefs;
            pseudo_fold_fundefs = new_fundef;
        } else {
            /*
             * Here, we face a with-loop which has been imported from a module via
             * the SIB. As a consequence, the fold application already has been lifted
             * to a separate N_fundef node. So, we don't have to do this again.
             * However, we have to search for it and store a reference within the
             * N_Nwithop node.
             */
            fun_tab_elem *fun_p
              = LookupFun (NWITHOP_FUN (withop), NWITHOP_MOD (withop), NULL);
            DBUG_ASSERT ((fun_p != NULL), "Imported fold operation doesn't exist.");
            DBUG_ASSERT ((FUNDEF_BODY (fun_p->node) != NULL),
                         "Imported fold operation has no body.");
            NWITHOP_FUNDEF (withop) = fun_p->node;
            fun_p->tag = PLEASE_CHECK;
            FUNDEF_STATUS (NWITHOP_FUNDEF (withop)) = ST_foldfun;
        }

        FreeOneTypes (body_type);
        FreeOneTypes (neutral_type);
        break;
    case WO_unknown:
        DBUG_ASSERT (FALSE, "non initialised WithOpType found.");
        break;
    }

    tmpn = ARG_NEXT (arg_info->node[2]);
    Free (arg_info->node[2]); /* free N_arg-node */
    arg_info->node[2] = tmpn; /* ...and restore old one. */

    tos = old_tos;

    if ((SAC_PRG == kind_of_file) && (SCALAR > TYPES_DIM (base_array_type))
        && (dynamic_shapes == 0)) {
        /* SCALAR == TYPES_DIM() possible in case of fold. */
        ABORT (NODE_LINE (arg_node), ("No constant type for withloop inferable"));
    }

    if ((SAC_PRG == kind_of_file) && (TYPES_DIM (base_array_type) >= SCALAR)) {
        /*  exchange withloops on empty array iv with their shortcut pendant. (jhs)
         *
         *  current assign has to be a let
         */

        DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (INFO_TC_CURRENTASSIGN (arg_info)))
                      == N_let),
                     "current assign is not a let!");

        /* if last assign is NULL, code for this case has to be added */
        DBUG_ASSERT ((INFO_TC_LASSIGN (arg_info) != NULL), "last assign is NULL");

        generator = NPART_GEN (NWITH_PART (arg_node));

        lowerbound = NGEN_BOUND1 (generator);
        upperbound = NGEN_BOUND2 (generator);

        lowerbound_empty = isBoundEmpty (arg_node, lowerbound);
        upperbound_empty = isBoundEmpty (arg_node, upperbound);

        if (lowerbound_empty && upperbound_empty) {

            if ((NWITHOP_TYPE (withop) == WO_modarray)
                || (NWITHOP_TYPE (withop) == WO_genarray)
                || (NWITHOP_TYPE (withop) == WO_foldfun)) {
                if ((NGEN_OP1_ORIG (generator) == F_le)
                    && (NGEN_OP2_ORIG (generator) == F_le)) {
                    /*  replace
                     *    "with ([] <= iv <= []) modarray (a, iv, b)"
                     *  or
                     *    "with ([] <= iv <= []) genarray ([], b)"
                     *  simply by:
                     *    "b"
                     */

                    tmpass
                      = MakeAssign (MakeLet (MakeCast (MakeFlatArray (NULL),
                                                       MakeTypes (T_int, UNKNOWN_SHAPE,
                                                                  NULL, NULL, NULL)),
                                             DupAllIds (NWITH_VEC (arg_node))),
                                    BLOCK_INSTR (
                                      NCODE_CBLOCK (NPART_CODE (NWITH_PART (arg_node)))));

                    /*  insert block into assignments, if block is
                     *  empty it will be replaced by CEXPR.
                     */
                    L_BLOCK_INSTR_OR_ASSIGN_NEXT (INFO_TC_LASSIGN (arg_info), tmpass);
                    /* BLOCK_INSTR(NCODE_CBLOCK(NPART_CODE(NWITH_PART(arg_node))))); */
                    /*  step to last assigment in chain and look forward,
                     *  concat end of block with current assignment  */
                    assignment = INFO_TC_LASSIGN (arg_info);
                    while ((BLOCK_INSTR_OR_ASSIGN_NEXT (assignment) != NULL)
                           && (NODE_TYPE (BLOCK_INSTR_OR_ASSIGN_NEXT (assignment))
                               != N_empty)) {
                        assignment = BLOCK_INSTR_OR_ASSIGN_NEXT (assignment);
                    } /* while */
                    ASSIGN_NEXT (assignment) = INFO_TC_CURRENTASSIGN (arg_info);
                    /* place CEXPR instead of withloop */
                    LET_EXPR (ASSIGN_INSTR (INFO_TC_CURRENTASSIGN (arg_info)))
                      = NCODE_CEXPR (NPART_CODE (NWITH_PART (arg_node)));

                    /* Cleanup the arg_node, that is no longer needed,
                     * but keep replaces elements. */
                    BLOCK_INSTR (NCODE_CBLOCK (NPART_CODE (NWITH_PART (arg_node))))
                      = NULL;
                    NCODE_CEXPR (NPART_CODE (NWITH_PART (arg_node))) = NULL;
                    arg_node = FreeTree (arg_node);
                } else {
                    /*  replace
                     *    "with ([] cmp1 iv cmp2 []) modarray (a, iv, b)"
                     *  or
                     *    "with ([] cmp1 iv cmp2 []) genarray ([], b)"
                     *  simply by default
                     *
                     *  whereby: (cmp1 in { "<", "<=" }) and
                     *           (cmp2 in { "<", "<=" }) and
                     *           ((cmp1 = "<") or (cmp2 = "<"))
                     *  default: in case of arrays : [0, ..., 0]
                     *             with shape of base_array_type respectively with
                     *             as many elements as described by the shape.
                     *           in case of scalars: 0 */

                    /*  body can be ignored, because only default values are returned
                     * here.
                     */

                    /*  evaluate new_expr, to be hanged into let.
                     */
                    if (TYPES_DIM (base_array_type) > SCALAR) {
                        if (NWITHOP_TYPE (withop) == WO_genarray) {
                            new_expr = ArrayOfZeros ((TypeToCount (base_array_type)));
                        } else if (NWITHOP_TYPE (withop) == WO_modarray) {
                            new_expr = NWITHOP_ARRAY (withop);
                        } else if (NWITHOP_TYPE (withop) == WO_foldfun) {
                            new_expr = NWITHOP_NEUTRAL (withop);
                        } else {
                            ABORT (NODE_LINE (arg_node),
                                   ("Wrong type of withloop applied to empty "
                                    "indexvectors"));
                        }
                    } else if (TYPES_DIM (base_array_type) == SCALAR) {
                        if (NWITHOP_TYPE (withop) == WO_genarray) {
                            new_expr = MakeNum (0);
                        } else if (NWITHOP_TYPE (withop) == WO_modarray) {
                            new_expr = NWITHOP_ARRAY (withop);
                        } else if (NWITHOP_TYPE (withop) == WO_foldfun) {
                            new_expr = NWITHOP_NEUTRAL (withop);

/*              ABORT(NODE_LINE(arg_node), ("Withloop on scalars applied to empty indexvectors"));
 */           } else {
    ABORT (NODE_LINE (arg_node),
           ("Wrong type of withloop applied to empty indexvectors"));
}
                    } else {
                        ABORT (NODE_LINE (arg_node), ("Wrong dimension of type!"));
                    }
                    LET_EXPR (ASSIGN_INSTR (INFO_TC_CURRENTASSIGN (arg_info))) = new_expr;

                    /* Cleanup the arg_node, no parts are reused here!
                     * So we can cleanup the whole arg_node. */
#if 0
          arg_node = FreeTree (arg_node);
#endif
                } /* if (NGEN_OP1_ORIG(...) ...) else ... */

                /* foldprf & foldfun missing here */

            } else {
#if 0
       ABORT( NODE_LINE(arg_node),
              ("Wrong type of withloop applied to empty indexvectors"));
#endif
            } /* if (NWITHOP_TYPE(...) ...) else ... */
        }     /* if (lowerbound_empty && upperbound_empty) ... */
    }

    DBUG_RETURN (base_array_type);
}

/******************************************************************************/

#define TI_NPART_HELP(bound, nodetext)                                                   \
    if (bound) {                                                                         \
        if (T_int != TYPES_BASETYPE (bound))                                             \
            ABORT (NODE_LINE (arg_node), (nodetext " is not an integer vector"));        \
        if (SCALAR > TYPES_DIM (bound)) {                                                \
            /* weak TC */                                                                \
            if (KNOWN_DIM_OFFSET > TYPES_DIM (bound)                                     \
                && KNOWN_DIM_OFFSET - 1 != TYPES_DIM (bound))                            \
                ABORT (NODE_LINE (arg_node), (nodetext " has to have one dimension"));   \
        } else {                                                                         \
            /* strong TC */                                                              \
            if (concrete_type) { /* 2 concrete types available */                        \
                if (CMP_equal != CmpTypes (*concrete_type, bound))                       \
                    ABORT (NODE_LINE (arg_node),                                         \
                           ("Wrong " nodetext " type: '%s`. Should be '%s`",             \
                            Type2String (bound, 0, TRUE),                                \
                            Type2String (*concrete_type, 0, TRUE)));                     \
            } else {                                                                     \
                if (1 != TYPES_DIM (bound))                                              \
                    ABORT (NODE_LINE (arg_node),                                         \
                           (nodetext " has to have one dimension"));                     \
                concrete_type = &bound;                                                  \
            }                                                                            \
        }                                                                                \
    }

/******************************************************************************
 *
 * function:
 *   types *TI_Npart(node *arg_node, types *default_bound_type,
 *                     node * new_shp, node *arg_info)
 *
 * description:
 *   checks the generator of the new WL and returns the type of the lower
 *   bound.
 *   1) substitues bounds . with constant vectors
 *      and also normalizes the operators combined with . to
 *      <= (in case of first operator) and < (in case of second operator)
 *   2) checks if bounds/step/width have same type
 *   3) completes N_Nwithid (introduce vector AND array of scalars as index)
 *   4) checks type of index variables (vector and array of scalars)
 *
 * remark: Point 1) is not possible if the withop is a fold
 *   function because we have no array identifier (modarray) or shape
 *   vector (like genarray) on which to base the number of elements of
 *   the index vector.
 *   In case of statically unknown shape, new_shp is required iff . is used
 *   as upper index vector bound!
 *
 ******************************************************************************/

static types *
TI_Npart (node *arg_node, types *default_bound_type, node *new_shp, node *arg_info)
{
    types *left_type, *right_type, *step_type, *width_type, *gen_type;
    types *tmpt, **concrete_type;
    node *gen, *withid, *tmpn, *tmp_num;
    int i, add, dim;
    ids *_ids;
    void *const_vec;

    DBUG_ENTER ("TI_Npart");
    gen = NPART_GEN (arg_node);

    /*  replace bounds . with a constant array. For the lower bound this is
     *  always possible ([0,...,0]). For the upper bound we need not only
     *  the dimension but the shape. This is made available by
     *  default_bound_shape.
     */

    if (DOT_ISSINGLE (NGEN_BOUND1 (gen))) {
        /*
         *  transform lower bound .
         *  if the operator is < it is changed to <= and 1 is added to the lower bound
         */
        if (!default_bound_type)
            ABORT (NODE_LINE (arg_node), ("bound . not allowed with function fold"));
        if (KNOWN_DIMENSION (TYPES_DIM (default_bound_type))) {
            if (TYPES_DIM (default_bound_type) > SCALAR)
                dim = TYPES_DIM (default_bound_type);
            else
                dim = KNOWN_DIM_OFFSET - TYPES_DIM (default_bound_type);
            tmpn = NULL;
            const_vec = AllocConstVec (T_int, dim);
            add = F_lt == NGEN_OP1 (gen);
            if (add)
                NGEN_OP1 (gen) = F_le;
            for (i = dim - 1; i >= 0; i--) {
                tmp_num = MakeNum (add);
                tmpn = MakeExprs (tmp_num, tmpn);
                const_vec = ModConstVec (T_int, const_vec, i, tmp_num);
            }
            tmpn = MakeFlatArray (tmpn);
            ARRAY_ISCONST (tmpn) = TRUE;
            ARRAY_VECTYPE (tmpn) = T_int;
            ARRAY_VECLEN (tmpn) = dim;
            ARRAY_CONSTVEC (tmpn) = const_vec;
            if (TYPES_DIM (default_bound_type) == SCALAR) {
                NGEN_BOUND1 (gen)
                  = MakeCast (tmpn, MakeTypes (T_int, 1, MakeShpseg (MakeNums (0, NULL)),
                                               NULL, NULL));
            } else {
                NGEN_BOUND1 (gen) = tmpn;
            }
        }
    }
    if (DOT_ISSINGLE (NGEN_BOUND2 (gen))) {
        /*
         *  transform upper bound .
         *  if the operator is <= it is changed to < and 1 is added to the upper bound.
         */
        if (!default_bound_type)
            ABORT (NODE_LINE (arg_node), ("bound . not allowed with function fold"));
        if (SCALAR <= TYPES_DIM (default_bound_type)) {
            tmpn = NULL;
            const_vec = AllocConstVec (T_int, TYPES_DIM (default_bound_type));
            add = F_le == NGEN_OP2 (gen);
            if (add)
                NGEN_OP2 (gen) = F_lt;
            for (i = TYPES_DIM (default_bound_type) - 1; i >= 0; i--) {
                tmp_num = MakeNum (TYPES_SHAPE (default_bound_type, i) + add - 1);
                tmpn = MakeExprs (tmp_num, tmpn);
                const_vec = ModConstVec (T_int, const_vec, i, tmp_num);
            }
            tmpn = MakeFlatArray (tmpn);
            ARRAY_ISCONST (tmpn) = TRUE;
            ARRAY_VECTYPE (tmpn) = T_int;
            ARRAY_VECLEN (tmpn) = TYPES_DIM (default_bound_type);
            ARRAY_CONSTVEC (tmpn) = const_vec;
            if (TYPES_DIM (default_bound_type) == SCALAR) {
                NGEN_BOUND2 (gen)
                  = MakeCast (tmpn, MakeTypes (T_int, 1, MakeShpseg (MakeNums (0, NULL)),
                                               NULL, NULL));
            } else {
                NGEN_BOUND2 (gen) = tmpn;
            }
        } else {
            /*
             * here, we have to use new-shape or new_shape-1, respectively.
             */
            if (SAC_MOD != kind_of_file) {
                if (F_le == NGEN_OP2 (gen)) {
                    NGEN_OP2 (gen) = F_lt;
                    NGEN_BOUND2 (gen) = new_shp;
                } else {
                    NGEN_BOUND2 (gen)
                      = MakePrf (F_sub_AxA,
                                 MakeExprs (new_shp, MakeExprs (MakeNum (1), NULL)));
                }
            }
        }
    }

    /*
     *  Now check whether the bounds, step and width have the same type.
     *  First, infere all types.
     */
    step_type = NULL;
    width_type = NULL;

    if (!(DOT_ISSINGLE (NGEN_BOUND1 (gen)))) {
        left_type = TypeInference (NGEN_BOUND1 (gen), arg_info);
        if (!left_type)
            ERROR (NODE_LINE (arg_node), ("lower bound cannot be infered"));
    } else {
        left_type = MakeTypes (T_int, KNOWN_DIM_OFFSET - 1, NULL, NULL, NULL);
    }

    if (!(DOT_ISSINGLE (NGEN_BOUND2 (gen)))) {
        right_type = TypeInference (NGEN_BOUND2 (gen), arg_info);
        if (!right_type)
            ERROR (NODE_LINE (arg_node), ("upper bound cannot be infered"));
    } else {
        right_type = MakeTypes (T_int, KNOWN_DIM_OFFSET - 1, NULL, NULL, NULL);
    }

    if (NGEN_STEP (gen)) {
        step_type = TypeInference (NGEN_STEP (gen), arg_info);
        if (!step_type)
            ERROR (NODE_LINE (arg_node), ("step cannot be infered"));
    }
    if (NGEN_WIDTH (gen)) {
        width_type = TypeInference (NGEN_WIDTH (gen), arg_info);
        if (!width_type)
            ERROR (NODE_LINE (arg_node), ("width cannot be infered"));
    }
    ABORT_ON_ERROR;

    /* check if all types fit to each other. */
    concrete_type = NULL;
    TI_NPART_HELP (left_type, "lower bound");
    TI_NPART_HELP (right_type, "upper bound");
    TI_NPART_HELP (step_type, "step");
    TI_NPART_HELP (width_type, "width");

    gen_type = (concrete_type) ? DupAllTypes (*concrete_type) : NULL;
    if (gen_type && TYPES_NAME (gen_type))
        TYPES_NAME (gen_type) = Free (TYPES_NAME (gen_type));
    if (gen_type && TYPES_MOD (gen_type))
        TYPES_MOD (gen_type) = Free (TYPES_MOD (gen_type));

    /* The index variable has to be either an identifier or an array of
     * identifiers. If the  SAC-user only specifies one of it, the other
     * part is generated here so that all generators have both for later
     * use. (id (vector) and array of ids (scalars)).
     *
     * Index scalars can only be created if we have a concrete_type */

    withid = NPART_WITHID (arg_node);
    /*
     *  if we have no index vector, we create one
     */
    if (!NWITHID_VEC (withid))
        NWITHID_VEC (withid) = MakeIds (TmpVar (), NULL, ST_regular);
    /*
     *  if we have no array of scalars, we create TYPES_SHAPE(gen_type,0) ids.
     *  because our index vector has dimension one, so TYPES_SHAPE(gen_type,0)
     *  is also the number of counter-variables needed.
     */
    if (!NWITHID_IDS (withid) && gen_type) {
        if (TYPES_SHAPE (gen_type, 0)) {
            _ids = MakeIds (TmpVar (), NULL, ST_regular);
            NWITHID_IDS (withid) = _ids;
            for (i = TYPES_SHAPE (gen_type, 0) - 1; i > 0; i--) {
                IDS_NEXT (_ids) = MakeIds (TmpVar (), NULL, ST_regular);
                _ids = IDS_NEXT (_ids);
            }
        } else {
            NWITHID_IDS (withid) = NULL;
        }
    }

    /* now we have to care about the ids.
     * Do they have the right type? ALL index vectors and scalars are always
     * given new names in flatten. So these names cannot be declared and
     * we do not have to check if they already have been typed.
     * First the index vector.*/
    if (gen_type) {
        /* strong TC */
        AddIdToStack (NWITHID_VEC (withid), gen_type, arg_info, NODE_LINE (arg_node));
    } else {
        /* weak TC */
        /* declare index vector as one-dimensional type.  */
        tmpt = MakeTypes (T_int, KNOWN_DIM_OFFSET - 1, NULL, NULL, NULL);
        AddIdToStack (NWITHID_VEC (withid), tmpt, arg_info, NODE_LINE (arg_node));
        FreeOneTypes (tmpt);
    }

    /* ...and then we check the array of scalars (int). All Id are
     * introduced in flatten (or TC) like the index vector. */
    _ids = NWITHID_IDS (withid);
    tmpt = MakeTypes1 (T_int);
    i = 0;
    while (_ids != NULL) {
        i++;
        AddIdToStack (_ids, tmpt, arg_info, NODE_LINE (arg_node));
        _ids = IDS_NEXT (_ids);
    }
    FreeOneTypes (tmpt);

    if (concrete_type && i != TYPES_SHAPE ((*concrete_type), 0))
        ABORT (NODE_LINE (arg_node), ("withloop index has %i element(s). %i expected", i,
                                      TYPES_SHAPE ((*concrete_type), 0)));

    /* return one type, free the others */
    FreeOneTypes (left_type);
    FreeOneTypes (right_type);
    FreeOneTypes (step_type);
    FreeOneTypes (width_type);

    if (!gen_type)
        gen_type = MakeTypes (T_int, KNOWN_DIM_OFFSET - 1, NULL, NULL, NULL);

    if ((SAC_PRG == kind_of_file) && (SCALAR >= TYPES_DIM (gen_type))
        && (dynamic_shapes == 0))
        ABORT (NODE_LINE (arg_node),
               ("No constant type for generator inferable (TYPES_DIM is %i)",
                TYPES_DIM (gen_type)));

    DBUG_RETURN (gen_type);
}

/******************************************************************************
 *
 * function:
 *   types *TI_Ngenarray(node *arg_node, node *arg_info)
 *
 * description:
 *   Checks the shape-argument of the genarray function. The return
 *   value is NOT the type of the shape but the type of the resulting
 *   base array. E.g. dimension 3 and shpseg [2,3,4].
 *
 * remark:
 *   the shape vector has to be a constant vector.
 *   If not, weak TC is applied and int[] is returned
 ******************************************************************************/

static types *
TI_Ngenarray (node *arg_node, node *arg_info, node **replace)
{
    types *ret_type, *expr_type;
    int dim = 0;
    node *tmpn;

    DBUG_ENTER ("TI_Ngenarray");

    expr_type = TypeInference (arg_node, arg_info);

    if (expr_type == NULL) {
        ABORT (NODE_LINE (arg_node),
               ("type of shape in genarray with loop cannot be infered!"));
    } else if (((TYPES_DIM (expr_type) != (KNOWN_DIM_OFFSET - 1))
                && (TYPES_DIM (expr_type) != 1)
                && (TYPES_DIM (expr_type) != UNKNOWN_SHAPE)
                && (TYPES_DIM (expr_type) != ARRAY_OR_SCALAR))
               || (TYPES_BASETYPE (expr_type) != T_int)) {
        ABORT (NODE_LINE (arg_node),
               ("type of shape in genarray with loop is %s which does not match int[.]!",
                Type2String (expr_type, 3, TRUE)));
    }
    /*
     * Now, we know the type of the shape-argument from the genarray WL,
     * but it is not yet obvious whether the exact value is statically
     * available!
     */

    /*
     * First we can through away any pre-ceeding casts, since we have made
     * sure that we are dealing with int[.] or int[x].
     */
    while (NODE_TYPE (arg_node) == N_cast) {
        arg_node = CAST_EXPR (arg_node);
        *replace = arg_node;
    }

    /*
     * Then, we try to apply constant-folding to reduce it to a const array.
     * This is only possible if we have a prf shape(). Else the integration
     * or CF would be too complex (masks, mrd, name clashed when including
     * optimize.h to patch masks and mrd).
     */
    tmpn = ReduceGenarrayShape (arg_node, arg_info, expr_type);
    if (tmpn) {
        arg_node = tmpn;
        /*
         * replace old expr in WL-syntax tree
         */
        *replace = arg_node;
    }

    if (NODE_TYPE (arg_node) == N_array) {
        /*
         * infer type of N_array and store it in N_array-node
         */
        ARRAY_TYPE (arg_node) = expr_type;

        /*
         * compute return_type
         */
        ret_type = MakeTypes (T_int, 0, MakeShpseg (NULL), NULL, NULL);

        tmpn = ARRAY_AELEMS (arg_node);
        while (tmpn)
            if (dim < SHP_SEG_SIZE) {
                if (N_num == NODE_TYPE (EXPRS_EXPR (tmpn))) {
                    TYPES_SHAPE (ret_type, dim) = NUM_VAL (EXPRS_EXPR (tmpn));
                    tmpn = EXPRS_NEXT (tmpn);
                    dim++;
                } else {
                    /*
                     * the shape is not constant! The dim+1 th element of the array
                     * is not a constant!
                     * Therefore, we have to set the return-type to int[].
                     * To get this done, we force an exit of the loop (tmpn = NULL)
                     * and set dim to UNKNOWN_SHAPE!
                     */
                    TYPES_SHPSEG (ret_type) = Free (TYPES_SHPSEG (ret_type));
                    dim = UNKNOWN_SHAPE;
                    tmpn = NULL;
                }
            } else {
                ABORT (NODE_LINE (arg_node), ("Shape vector has too many elements"));
            }

        TYPES_DIM (ret_type) = dim;
        /*
         * don't free exprs_type because it is assignd to arg_node.
         */
    } else {
        /* weak TC */
        if (TYPES_DIM (expr_type) == 1) {
            /*
             * we do know the exact shape of the shape-arg
             * => we know the dim of the result
             * => return int[ ., ..., .] ...
             */
            ret_type = MakeTypes (T_int, KNOWN_DIM_OFFSET - TYPES_SHAPE (expr_type, 0),
                                  NULL, NULL, NULL);
        } else {
            /* return int[] ... */
            ret_type = MakeTypes (T_int, UNKNOWN_SHAPE, NULL, NULL, NULL);
        }
        FreeOneTypes (expr_type);
    }

    if ((kind_of_file == SAC_PRG) && (TYPES_DIM (ret_type) < SCALAR)
        && (dynamic_shapes == 0))
        ABORT (NODE_LINE (arg_node), ("No constant type for genarray shape inferable"));

    DBUG_RETURN (ret_type);
}

/******************************************************************************
 *
 * function:
 *   void ConsistencyCheckModarray( int lineno,
 *                                  types *array_type, types *generator_type,
 *                                  types *body_type)
 *
 * description:
 *   additional checks for modarray operator
 *
 ******************************************************************************/

static void
ConsistencyCheckModarray (int lineno, types *array_type, types *generator_type,
                          types *body_type)
{
    int length, i;
    DBUG_ENTER (" ConsistencyCheckModarray");

    if (TYPES_BASETYPE (array_type) != TYPES_BASETYPE (body_type))
        ABORT (lineno,
               ("Basetypes of 'modarray` (%s) and return_type (%s) different",
                Type2String (array_type, 0, TRUE), Type2String (body_type, 0, TRUE)));

    /* strong TC */
    if (SCALAR < TYPES_DIM (array_type) && SCALAR < TYPES_DIM (generator_type)
        && SCALAR <= TYPES_DIM (body_type)) {
        length = TYPES_SHAPE (generator_type, 0);
        if (length + TYPES_DIM (body_type) == TYPES_DIM (array_type))
            for (i = length; i < TYPES_DIM (array_type); i++) {
                if (TYPES_SHAPE (array_type, i) != TYPES_SHAPE (body_type, i - length))
                    ABORT (lineno, ("Type of shape vector (%s) does not match "
                                    "type of index vector (%s) and return statement (%s)",
                                    Type2String (array_type, 0, TRUE),
                                    Type2String (generator_type, 0, TRUE),
                                    Type2String (body_type, 0, TRUE)));
            }
        else
            ABORT (lineno, ("Type of shape vector (%s) does not match "
                            "type of index vector (%s) and return statement (%s)",
                            Type2String (array_type, 0, TRUE),
                            Type2String (generator_type, 0, TRUE),
                            Type2String (body_type, 0, TRUE)));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   types *TI_Nfoldprf(...)
 *
 * description:
 *   this is a copy of TI_foldprf, modified so that it can be used
 *   by the new WLs.
 *
 * parameters:
 *   arg_node is the N_Nwithop node.
 *
 ******************************************************************************/

static types *
TI_Nfoldprf (node *arg_node, types *body_type, types *neutral_type, node *arg_info)
{
    prim_fun_tab_elem *prf_p;
    void *fun_p;
    types *ret_type, **arg_type;
    int type_c_tag, prf_tag;
    prf old_prf;

    DBUG_ENTER ("TI_Nfoldprf");
    DBUG_ASSERT (NWITHOP_TYPE (arg_node) == WO_foldprf, "Wrong withop type");

    arg_type = (types **)Malloc (sizeof (types) * 2);
    arg_type[0] = body_type;
    arg_type[1] = body_type;
    old_prf = NWITHOP_PRF (arg_node);
    prf_tag = NWITHOP_PRF (arg_node);
    fun_p = FindFun (NULL, NULL, arg_type, 2, arg_info, NODE_LINE (arg_node), &prf_tag);

    DBUG_ASSERT (fun_p, "fun_p is NULL");

    if ((prf)prf_tag == NWITHOP_PRF (arg_node)) {
        /* it is a primitive function */
        prf_p = (prim_fun_tab_elem *)fun_p;
        type_c_tag = prf_p->node->info.prf_dec.tag;
        NWITHOP_PRF (arg_node) = prf_p->new_prf;
        /* now compute resulting type (ret_type) */
        switch (type_c_tag) {
#include "prim_fun_tt.mac" /* assigns ret_type */
        default:
            ret_type = NULL; /* just to please the compiler 8-) */
            DBUG_ASSERT (0, "wrong type_class_tag");
            break;
        }
    } else {
        /* userdefined primitive function */
        NWITHOP_TYPE (arg_node) = WO_foldfun;
        NWITHOP_FUN (arg_node) = StringCopy (((fun_tab_elem *)fun_p)->id);
        ret_type = TI_fun (arg_node, (fun_tab_elem *)fun_p, arg_info);

/*
 * not necessary here, because this done in 'TI_Nwith' after call of
 *  'CreatePseudoFoldFun' !!
 */
#if 0
    /* set pointer to function declaration */
    NWITHOP_FUNDEF(arg_node)=((fun_tab_elem*)fun_p)->node;
#endif
    }

    if (T_unknown == TYPES_BASETYPE (ret_type)) {
        ABORT (NODE_LINE (arg_node),
               ("Primitive function '%s` is applied to wrong arguments",
                prf_string[NWITHOP_PRF (arg_node)]));
    } else if (CMP_equal
               != CompatibleTypes (body_type, ret_type, -1, NODE_LINE (arg_node))) {
        if (WO_foldfun == NWITHOP_TYPE (arg_node)) {
            ABORT (NODE_LINE (arg_node),
                   ("Function '%s` has return type '%s` != '%s`",
                    ModName (NWITHOP_MOD (arg_node), NWITHOP_FUN (arg_node)),
                    Type2String (ret_type, 0, TRUE), Type2String (body_type, 0, TRUE)));
        } else {
            ABORT (NODE_LINE (arg_node),
                   ("Function '%s` has return type '%s` != '%s`",
                    prf_string[arg_node->info.prf], Type2String (ret_type, 0, TRUE),
                    Type2String (body_type, 0, TRUE)));
        }
    }
    if (NWITHOP_NEUTRAL (arg_node)) {
        if (CMP_equal != CmpTypes (neutral_type, ret_type)) /* 1 */ {
            ABORT (NODE_LINE (arg_node),
                   ("neutral element of 'fold` has wrong type '%s`. Should be '%s`",
                    Type2String (neutral_type, 0, TRUE),
                    Type2String (ret_type, 0, TRUE)));
        }
    } else {
        NWITHOP_NEUTRAL (arg_node) = ComputeNeutralElem (old_prf, body_type, arg_info);
    }

    arg_type = Free (arg_type);
    DBUG_RETURN (ret_type);
}

/******************************************************************************
 *
 * function:
 *   types *TI_Nfoldfun(...)
 *
 * description:
 *   Checks whether neutral element and type of WL body fit to the function's
 *   arguments and its return type.
 *
 ******************************************************************************/

static types *
TI_Nfoldfun (node *arg_node, types *body_type, types *neutral_type, node *arg_info)
{
    fun_tab_elem *fun_p;
    types *ret_type, **arg_type;
    int tmp;

    DBUG_ENTER ("TI_Nfoldfun");

    /* infere return type of function NWITHOP_FUN() if applied to two
       arguments of type body_type. */
    arg_type = (types **)Malloc (sizeof (types) * 2);
    arg_type[0] = body_type;
    arg_type[1] = body_type;

    tmp = -1;
    fun_p = (fun_tab_elem *)FindFun (NWITHOP_FUN (arg_node), NWITHOP_MOD (arg_node),
                                     arg_type, 2, arg_info, NODE_LINE (arg_node), &tmp);
    DBUG_ASSERT (fun_p, "fun_p is NULL");

    ret_type = TI_fun (arg_node, fun_p, arg_info);

    /* check consistency of ret_type, body_type and neutral_type */
    if (T_unknown == TYPES_BASETYPE (ret_type)) {
        ABORT (NODE_LINE (arg_node),
               ("Type of function '%s` not inferable",
                ModName (NWITHOP_MOD (arg_node), NWITHOP_FUN (arg_node))));
    } else {
        if (CMP_equal != CmpTypes (body_type, ret_type)) {
            ABORT (NODE_LINE (arg_node),
                   ("Function '%s` has return type '%s` != '%s`",
                    ModName (NWITHOP_MOD (arg_node), NWITHOP_FUN (arg_node)),
                    Type2String (ret_type, 0, TRUE), Type2String (body_type, 0, TRUE)));
        }
        if (CMP_equal != CmpTypes (neutral_type, body_type)) {
            ABORT (NODE_LINE (arg_node),
                   ("Neutral element of 'fold` has wrong type '%s`. Should be '%s`",
                    Type2String (neutral_type, 0, TRUE),
                    Type2String (ret_type, 0, TRUE)));
        }

/*
 * not necessary here, because this done in 'TI_Nwith' after call of
 *  'CreatePseudoFoldFun' !!
 */
#if 0
    /* set pointer to function declararion */
    NWITHOP_FUNDEF(arg_node)=fun_p->node;
#endif
    }

    DBUG_RETURN (ret_type);
}

/******************************************************************************
 *
 * function:
 *   node *TCNcode(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses the Ncode node.
 *
 ******************************************************************************/

node *
TCNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TCNcode");

    if (N_empty != NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (arg_node)))) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    /* now we have to find the vardec for CEXPR and return it in arg_info. */
    arg_info->node[2]->info.types = TypeInference (NCODE_CEXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   fun_tab_elem *TryConstantPropagation(fun_tab_elem *fun_p, node *args)
 *   fun_tab_elem *DoConstantPropagation(fun_tab_elem *fun_p, nodelist *propas)
 *
 * description:
 *
 *   This function represents a more or less quick hack to realize the
 *   propagation of integer vector and scalar constants across function
 *   definitions in order to allow typechecking of functions like
 *
 *    int[] create_array(int[.] shp)
 *
 *
 *   This works about as follows:
 *    1. At function application, check whether constant int vectors
 *       or scalars are available among the function arguments.
 *    2. If so, the function is duplicated
 *    3. It is traversed with a list parameter-constant mappings.
 *    4. Additonal assignments of constants to parameters are inserted
 *       into the assignment chain.
 *    5. At each occurrence of a parameter the constant vector information
 *       is annotated.
 *    6. At each parameter occurrence in the specification of the result
 *       shape of a genarray-withloop, the identifier is replaced by the
 *       constant vector or scalar.
 *    7. If (6) never happened, the specialized function is removed again,
 *       i.e., its body is freed immediately whereas the function prototype
 *       is eliminated by subsequent dead function removal.
 *
 ******************************************************************************/

static fun_tab_elem *
TryConstantPropagation (fun_tab_elem *fun_p, node *args)
{
    node *arg, *param, *propnode;
    nodelist *propnodes;

    DBUG_ENTER ("TryConstantPropagation");

    arg = args;
    param = FUNDEF_ARGS (fun_p->node);
    propnodes = NULL;

    DBUG_PRINT ("TCCP", ("Checking tccp for function %s", FUNDEF_NAME (fun_p->node)));

    while ((param != NULL) && (arg != NULL)) {
        if ((NODE_TYPE (EXPRS_EXPR (arg)) == N_id) && (ID_ISCONST (EXPRS_EXPR (arg)))
            && (ID_VECTYPE (EXPRS_EXPR (arg)) == T_int)) {
            propnode = DupTree (EXPRS_EXPR (arg));
            Free (ID_NAME (propnode));
            ID_NAME (propnode) = StringCopy (ARG_NAME (param));

            propnodes = MakeNodelistNode (propnode, propnodes);
        } else {
            if (NODE_TYPE (EXPRS_EXPR (arg)) == N_num) {
                propnode = MakeId (StringCopy (ARG_NAME (param)), NULL, ST_regular);
                ID_ISCONST (propnode) = 1;
                ID_VECLEN (propnode) = 0;
                ID_NUM (propnode) = NUM_VAL (EXPRS_EXPR (arg));

                propnodes = MakeNodelistNode (propnode, propnodes);
            }
        }

        arg = EXPRS_NEXT (arg);
        param = ARG_NEXT (param);
    }

    if (propnodes != NULL) {
        fun_p = DoConstantPropagation (fun_p, propnodes);
        FreeNodelist (propnodes);
    } else {
        DBUG_PRINT ("TCCP", ("No tccp for function %s", FUNDEF_NAME (fun_p->node)));
    }

    DBUG_RETURN (fun_p);
}

static fun_tab_elem *
DoConstantPropagation (fun_tab_elem *fun_p, nodelist *propas)
{
    funtab *old_tab;
    node *arg_info;
    int n_dub;
    fun_tab_elem *new_fun_p, *res_fun_p;
    char *newname;

    DBUG_ENTER ("DoConstantPropagation");

    DBUG_PRINT ("TCCP", ("Trying tccp for function %s", FUNDEF_NAME (fun_p->node)));

    n_dub = fun_p->n_dub;
    fun_p->n_dub = 1;

    new_fun_p = DuplicateFun (fun_p);

    fun_p->n_dub = n_dub;
    /*
     * The manipulation of the specialization counter is necessary because we may
     * want to further specialize already type-specialized functions. However, their
     * counter is set to -2 which is subsequently used for identification of
     * specialized functions.
     * Unfortunately, this implies that there is no limit on the number of
     * function specializations resulting from constant propagation.
     */

    newname = TmpVarName (FUNDEF_NAME (new_fun_p->node));
    Free (FUNDEF_NAME (new_fun_p->node));
    FUNDEF_NAME (new_fun_p->node) = newname;
    FUNDEF_MOD (new_fun_p->node) = module_name;
    /*
     * The module name must be set to current module's name because the function
     * itself is renamed during constant propagation specialization, othwerwise
     * the symbol is searched for in the original module and, hence, found to be
     * non-existant.
     */
    new_fun_p->id = FUNDEF_NAME (new_fun_p->node);
    new_fun_p->id_mod = FUNDEF_MOD (new_fun_p->node);
    /*
     * The entry in the fun table must be updated accordingly.
     * However, it shares the name with the "real" synatax tree.
     */

    old_tab = act_tab;
    act_tab = tccp_tab;

    arg_info = MakeInfo ();

    INFO_TC_TCCP (arg_info) = propas;
    INFO_TC_TCCPSUCCESS (arg_info) = 0;
    INFO_TC_ISGWLSHAPE (arg_info) = 0;

    new_fun_p->node = Trav (new_fun_p->node, arg_info);

    if (INFO_TC_TCCPSUCCESS (arg_info)) {
        res_fun_p = new_fun_p;
        DBUG_PRINT ("TCCP", ("TCCP successful"));
    } else {
        res_fun_p = fun_p;
        DBUG_PRINT ("TCCP", ("TCCP rejected"));
    }

    arg_info = Free (arg_info);

    act_tab = old_tab;

    DBUG_RETURN (res_fun_p);
}

static node *
CreateConstLet (node *id)
{
    node *new;

    DBUG_ENTER ("CreateConstLet");

    if (ID_VECLEN (id) == 0) {
        new = MakeLet (MakeNum (ID_NUM (id)),
                       MakeIds (StringCopy (ID_NAME (id)), NULL, ST_regular));
    } else {
        new = MakeLet (MakeFlatArray (IntVec2Array (ID_VECLEN (id), ID_CONSTVEC (id))),
                       MakeIds (StringCopy (ID_NAME (id)), NULL, ST_regular));
    }

    DBUG_RETURN (new);
}

/******************************************************************************
 *
 * function:
 *   node *TCCPfundef(node *arg_node, node *arg_info)
 *   node *TCCPblock(node *arg_node, node *arg_info)
 *   node *TCCPassign(node *arg_node, node *arg_info)
 *   node *TCCPlet(node *arg_node, node *arg_info)
 *   node *TCCPnwithop(node *arg_node, node *arg_info)
 *   node *TCCPid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   Traversal routines for typecheck constant propagation.
 *
 *
 *
 ******************************************************************************/

node *
TCCPfundef (node *arg_node, node *arg_info)
{
    nodelist *param;
    node *new_assigns, *last_assign;

    DBUG_ENTER ("TCCPfundef");

    /*
     * After  the traversal, parts of the information may be destroyed.
     * Therefore, we set up the new assignments right now and add them
     * to the chain if necessary.
     */

    param = INFO_TC_TCCP (arg_info);

    new_assigns = MakeAssign (CreateConstLet (NODELIST_NODE (param)), NULL);
    last_assign = new_assigns;
    param = NODELIST_NEXT (param);

    while (param != NULL) {
        new_assigns = MakeAssign (CreateConstLet (NODELIST_NODE (param)), new_assigns);
        param = NODELIST_NEXT (param);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (INFO_TC_TCCPSUCCESS (arg_info)) {
        ASSIGN_NEXT (last_assign) = BLOCK_INSTR (FUNDEF_BODY (arg_node));
        BLOCK_INSTR (FUNDEF_BODY (arg_node)) = new_assigns;
    } else {
        /*
         * Constant propagation does not help typechecking, so we omit it.
         */
        FUNDEF_BODY (arg_node) = FreeTree (FUNDEF_BODY (arg_node));
        new_assigns = FreeTree (new_assigns);
    }

    DBUG_RETURN (arg_node);
}

node *
TCCPblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TCCPblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
TCCPassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TCCPassign");

    if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_do)
        && (NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_while)
        && (NODE_TYPE (ASSIGN_INSTR (arg_node)) != N_return)) {

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
TCCPlet (node *arg_node, node *arg_info)
{
    ids *ids;
    nodelist *param;

    DBUG_ENTER ("TCCPlet");

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    ids = LET_IDS (arg_node);

    while (ids != NULL) {
        param = INFO_TC_TCCP (arg_info);

        while (param != NULL) {
            if (0 == strcmp (IDS_NAME (ids), ID_NAME (NODELIST_NODE (param)))) {
                Free (ID_NAME (NODELIST_NODE (param)));
                ID_NAME (NODELIST_NODE (param)) = StringCopy ("_");
                /*
                 * Parameters which are shadowed by assignments are cleared from
                 * the list of parameter-constant mappings to avoid false bindings.
                 */
            }
            param = NODELIST_NEXT (param);
        }

        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (arg_node);
}

node *
TCCPnwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("TCCPnwithop");

    if (NWITHOP_TYPE (arg_node) == WO_genarray) {
        INFO_TC_ISGWLSHAPE (arg_info) = 1;
        NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        INFO_TC_ISGWLSHAPE (arg_info) = 0;
    }

    DBUG_RETURN (arg_node);
}

node *
TCCPid (node *arg_node, node *arg_info)
{
    nodelist *param;

    DBUG_ENTER ("TCCPid");

    param = INFO_TC_TCCP (arg_info);

    while (param != NULL) {
        if (0 == strcmp (ID_NAME (arg_node), ID_NAME (NODELIST_NODE (param)))) {
            if (INFO_TC_ISGWLSHAPE (arg_info)) {
                /*
                 * Here, we are in the result shape specification of a
                 * genarray-withloop.
                 *
                 * Both scalars and vectors are replaced by the respective constants.
                 */
                arg_node = FreeTree (arg_node);
                if (ID_VECLEN (NODELIST_NODE (param)) == 0) {
                    arg_node = MakeNum (ID_NUM (NODELIST_NODE (param)));
                } else {
                    arg_node = MakeFlatArray (
                      IntVec2Array (ID_VECLEN (NODELIST_NODE (param)),
                                    ID_CONSTVEC (NODELIST_NODE (param))));
                    ARRAY_ISCONST (arg_node) = 1;
                    ARRAY_VECLEN (arg_node) = ID_VECLEN (NODELIST_NODE (param));
                    ARRAY_VECTYPE (arg_node) = ID_VECTYPE (NODELIST_NODE (param));
                    ARRAY_CONSTVEC (arg_node)
                      = CopyConstVec (ID_VECTYPE (NODELIST_NODE (param)),
                                      ID_VECLEN (NODELIST_NODE (param)),
                                      ID_CONSTVEC (NODELIST_NODE (param)));
                }

                INFO_TC_TCCPSUCCESS (arg_info) = 1;
            } else {
                /*
                 * Here, we are NOT in the result shape specification of a
                 * genarray-withloop.
                 */
                if (ID_VECLEN (NODELIST_NODE (param)) == 0) {
                    arg_node = FreeTree (arg_node);
                    arg_node = MakeNum (ID_NUM (NODELIST_NODE (param)));
                    /*
                     * Scalars are replaced by the respective constants.
                     */
                } else {
                    ID_ISCONST (arg_node) = 1;
                    ID_VECLEN (arg_node) = ID_VECLEN (NODELIST_NODE (param));
                    ID_VECTYPE (arg_node) = ID_VECTYPE (NODELIST_NODE (param));
                    ID_CONSTVEC (arg_node)
                      = CopyConstVec (ID_VECTYPE (NODELIST_NODE (param)),
                                      ID_VECLEN (NODELIST_NODE (param)),
                                      ID_CONSTVEC (NODELIST_NODE (param)));
                    /*
                     * Constant vectors are annotated.
                     */
                }
            }

            break;
        }
        param = NODELIST_NEXT (param);
    }

    DBUG_RETURN (arg_node);
}
