/*
 *
 * $Log$
 * Revision 2.3  1999/02/26 10:49:59  bs
 * BuildGenarrayWithLoop(), BuildTakeWithLoop() and BuildDropWithLoop() prepared
 * for the work with compact int. vectors.
 *
 * Revision 2.2  1999/02/25 11:07:23  bs
 * TI_Ngenarray modified. Now this function is able to work with
 * compact integer arrays.
 *
 * Revision 2.1  1999/02/23 12:40:56  sacbase
 * new release made
 *
 * Revision 1.190  1999/02/19 18:36:34  sbs
 * check on non-infereable shape-types in genarray withloop
 * in TI_Ngenarray inserted.
 *
 * Revision 1.189  1999/02/19 18:12:44  bs
 * access macros added
 *
 * Revision 1.188  1999/02/11 14:11:49  sbs
 * changed implementation of non-intrinsic cat.
 * now, you may use non-intrinsic with -noopt as well ....
 * The problem was that the generated sac-code was
 * non-flatten-compliant..
 *
 * Revision 1.187  1999/02/10 13:45:27  cg
 * Bug fixed in BuildCatWithloop1(): Now the upper border of
 * the generator is written as [...] instead of shape(...).
 * Otherwise problems arise if constant folding is switched off
 * because the backend relies on having either a variable or
 * a constant in this position.
 *
 * Revision 1.186  1999/02/09 13:41:29  sbs
 * dared to switch prim-funflag when calling CompatibleTypes
 * in several situations. Marked these clearly by sbs and
 * explained why I did so.
 *
 * Revision 1.185  1999/02/09 13:08:41  sbs
 * fixed some type-errors in TCreturn.
 *
 * Revision 1.184  1999/02/06 14:44:03  dkr
 * Bug fixed:
 *   Minimum for double/float is *not* MINDOUBLE/MINFLOAT
 *   but -MAXDOUBLE/-MAXFLOAT !!!
 *
 * Revision 1.183  1999/02/03 17:44:51  sbs
 * Due to a bug in limits.h (!!!), we had to replace INT_MIN by 1-INT_MAX!!
 *
 * Revision 1.182  1999/01/26 14:28:08  cg
 * Added implicit expansion of array-valued psi() operations
 * to equivalent with-loops.
 * All implicit expansions are now performed iff the result shape
 * has been infered completely.
 *
 * Revision 1.181  1999/01/25 11:10:53  sbs
 * MAX and MIN interchanged for default value on max-fold / min-fold!
 *
 * Revision 1.180  1999/01/21 17:02:22  sbs
 * typo "exspected" eliminated.
 *
 * Revision 1.179  1999/01/20 10:43:58  sbs
 * space inserted.
 *
 * Revision 1.178  1999/01/20 09:06:22  cg
 * Programs without any functions are now rejected.
 *
 * Revision 1.177  1999/01/19 16:18:35  cg
 * Added rudimentary support for [?] types;
 * de-activated some warnings about unknown shapes when compiling
 * a module/class implementation.
 *
 * Revision 1.176  1999/01/19 09:19:12  sbs
 * extension of drop_vec in BuildDropWithLoop added
 *
 * Revision 1.175  1999/01/15 15:22:48  cg
 * primitive function genarray is now expanded to an equivalent
 * with-loop. If not the intrinsic versions are requested, the
 * primitive functions take, drop, and cat are also expanded
 * to equivalent with-loops.
 *
 * Revision 1.174  1999/01/08 11:30:42  sbs
 * Bug in FindFun eliminated prf's where LookupPrf returns NULL
 * (like rotate when intrinsics is off!) will be searched as
 * user defined functions now as well...
 *
 * Revision 1.173  1999/01/07 15:19:32  cg
 * Some bugs fixed in typechecking og module/class implementations.
 * Warnings in the context of functions with result types of unspecified
 * shape are completely revised.
 *
 * Revision 1.172  1999/01/07 10:50:49  cg
 * Various bugs removed in the context of fold with-loops.
 *
 * Revision 1.171  1998/12/10 12:39:52  cg
 * bug fixed: if a user-defined type is used but not defined,
 * an appropriate error message is displayed and the compilation
 * process terminated.
 *
 * Revision 1.170  1998/12/03 14:19:57  sbs
 * errors typesys_0004.sac and typesys_0005.sac fixed.
 * both in TI_Nwith!
 *
 * Revision 1.169  1998/12/03 10:23:12  cg
 * Using header file values.h instead of limits.h in order to derive
 * maximum values for floating point numbers.
 * This is required for compatibility with Linux.
 *
 * Revision 1.168  1998/12/02 16:28:40  cg
 * Now, typecheck also removes superfluous function declarations in
 * addition to full function definitions.
 *
 * Revision 1.167  1998/11/18 15:09:37  srs
 * TCNcode expects N_empty node now.
 *
 * Revision 1.166  1998/11/09 20:17:07  sbs
 * DeleteFun adjusted to tree_basic.h
 * -> errors in DBUG_PRINTs in DeleteFun removed...
 *
 * Revision 1.165  1998/11/08 14:40:08  dkr
 * TCNcode: remarks added --- this part might be buggy!!
 *
 * Revision 1.164  1998/10/30 09:55:16  cg
 * Bugs fixed in generation of neutral element for fold operators.
 *
 * Revision 1.163  1998/08/07 12:00:20  srs
 * changed ABORT-message
 *
 * Revision 1.162  1998/08/06 18:33:09  srs
 * adepted TC to new semantics of flattening N_Nwith
 *
 * Revision 1.161  1998/06/08 12:38:32  cg
 * Function DuplicateTypes can now deal with type NULL
 *
 * Revision 1.160  1998/06/05 15:27:49  cg
 * global variable mod_name_con and macros MOD_NAME_CON MOD MOD_NAME MOD_CON removed
 * Now, module name and symbol name are combined correctly by ':'.
 * Only when it really comes to the generation of C code, the ':' is
 * replaced by '__'. This is done by the renaming of all identifiers
 * during the precompilation phase.
 *
 * Revision 1.159  1998/05/30 19:43:14  dkr
 * creation of PseudoFun debugged:
 *   FUNNAME is no longer shared
 *
 * Revision 1.158  1998/05/30 17:46:21  dkr
 * corrected an error-message
 *
 * Revision 1.157  1998/05/29 12:39:32  sbs
 * pseudo-fun creation debugged...
 *
 * Revision 1.156  1998/05/28 23:54:46  dkr
 * NWITHOP_FUNDEF is now set correctly after creation of pseudo-fold-fun
 *
 * Revision 1.155  1998/05/28 14:52:22  sbs
 * abstraction of pseudo-funs for WL-fold inserted!
 * this automatically converts all folds into FOLDFUN instead of FOLDPRF!
 *
 * Revision 1.154  1998/05/25 09:56:28  srs
 * removed unused parameter from functions TI_Nfold..()
 *
 * Revision 1.153  1998/05/18 07:51:46  srs
 * fixed bug in TI_Nwith()
 *
 * Revision 1.152  1998/05/17 07:26:30  srs
 * fixed bug in TI_Nwith
 *
 * Revision 1.151  1998/05/12 22:34:17  dkr
 * removed unused var
 *
 * Revision 1.150  1998/05/12 13:19:59  cg
 * bug fixed: original vardecs are now traversed in advance and
 * module names are given to user-defined types where necessary.
 *
 * Revision 1.149  1998/05/06 15:05:06  srs
 * renames INFO_ macros to INFO_TC_
 *
 * Revision 1.148  1998/04/29 14:19:52  srs
 * fixed bug in TI_Nfoldprf()
 *
 * Revision 1.147  1998/04/29 12:46:15  srs
 * added weak TC of new WLs
 *
 * Revision 1.146  1998/03/24 15:30:29  cg
 * #include "profile.h" removed since file no longer exists.
 *
 * Revision 1.145  1998/03/24 13:43:42  cg
 * The maximum number of applications of a single function is now determined.
 * This information is later used for generating profiling code.
 *
 * Revision 1.144  1998/03/03 14:02:09  cg
 * Last occurrence of function Error() replaced by macro SYSABORT
 *
 * Revision 1.143  1998/02/19 11:22:59  srs
 * fixed two bugs:
 * - uninferable array in modarray() does not lead to a segfault anymore.
 * - variable definitions in withloops (generator, body) are local now.
 *
 * Revision 1.142  1998/02/15 21:27:38  srs
 * fixed bug in typechecking the neutral elements of WL-fold
 *
 * Revision 1.141  1998/02/11 17:15:37  srs
 * changed NPART_IDX to NPART_WITHID
 *
 * Revision 1.140  1998/02/11 16:34:14  dkr
 * typedef cmp_types moved to typecheck.h (compile.c needs to import this type)
 *
 * Revision 1.139  1998/02/10 10:19:45  srs
 * fixed bug in TI_Ncode
 *
 * Revision 1.138  1998/02/10 09:59:40  srs
 * added VARDEC information to NCODE_CEXPR var in function TCNcode.
 *
 * Revision 1.137  1998/02/09 15:55:53  srs
 * added typechecking for new WLs
 *
 * Revision 1.136  1997/11/26 14:06:38  srs
 * removed use of old macros from acssass_macros.h
 *
 * Revision 1.135  1997/11/10 23:37:24  dkr
 * removed a bug with NEWTREE
 *
 * Revision 1.134  1997/11/07 11:24:08  srs
 * NEWTREE: nnode is ignored
 *
 * Revision 1.133  1997/10/29 14:28:54  srs
 * removed HAVE_MALLOC_O
 *
 * Revision 1.132  1997/10/28 18:26:50  srs
 * dead code removed
 *
 * Revision 1.131  1997/08/26 15:51:39  sbs
 * StringCopy inserted in TI_prf where N_prf is changed to N_ap
 * since we deal with an overloaded version of the primitive function!
 *
 * Revision 1.130  1997/05/28 12:35:58  sbs
 * Profiling integrated
 *
 * Revision 1.129  1997/05/16  09:55:11  sbs
 * ANALSE-TOOL extended to function-application specific timing
 *
 * Revision 1.128  1997/05/14  08:13:50  sbs
 * N_annotate's inserted around function apps
 *
 * Revision 1.127  1997/04/25  09:16:41  sbs
 * changed NULL -> 0 where appropriate
 *
 * Revision 1.126  1997/04/24  15:04:56  sbs
 * HAVE_MALLOC_O inserted.
 *
 * Revision 1.125  1997/03/19  15:31:08  cg
 * Now, module/class implementations without any functions are supported
 *
 * Revision 1.124  1997/03/11  16:30:38  cg
 * function LookupObject rewritten. Now, it should be possible to specify
 * a module name even for external modules when using a global object.
 *
 * Revision 1.123  1996/09/11  06:20:33  cg
 * Warnings because of unused functions are omitted with respect to
 * imported modules.
 *
 * Revision 1.122  1996/09/06  18:53:29  cg
 * more bugs fixed in CmpFunParams, should work now.
 *
 * Revision 1.121  1996/09/06  17:05:04  cg
 * bug fixed in function CmpFunParams.
 *
 * Revision 1.120  1996/05/23  11:25:05  sbs
 * error for imported functions that contain unknown array-shapes
 * changed to WARN ing :
 *
 *  WARN(0,("Imported function '%s` contains"
 *                      " %d types with unknown shape",
 *                      ModName(fun_node->ID_MOD, fun_node->ID),
 *                      wrong_shape));
 *
 * Revision 1.119  1996/04/04  16:34:09  hw
 * bug fixed in InitFunTable (functions will be inserted in correct order now)
 *
 * Revision 1.118  1996/04/02  19:38:18  cg
 * bug fixed in function CheckIfGOonlyCBR
 * runs now with functions with variable argument lists.
 *
 * Revision 1.117  1996/04/02  16:06:28  hw
 * - now FUNDEF_LINKMOD will be inserted in internal fun_tab,
 *   if FUNDEF_MOD is NULL (external function)
 *
 * Revision 1.116  1996/03/21  18:02:18  cg
 * bug fixed in function CheckFunctionParameters:
 * if(T_dots == ARG_BASETYPE(arg)) switched to if(T_dots != ARG_BASETYPE(arg))
 *
 * Revision 1.115  1996/03/05  15:30:15  cg
 * implemented typecheck of functions with a variable number of return
 * values (imported external declarations only)
 *
 * Revision 1.114  1996/02/27  19:04:23  hw
 * now the TC looks behind hidden types in imported functions only
 *
 * Revision 1.113  1996/02/13  18:06:12  hw
 * bug fixed in TCLet ( Comparison of types with dim ARRAY_OR_SCALAR
 *  and types with dim > SKALAR changed)
 *
 * Revision 1.112  1996/02/13  15:38:19  hw
 * DBUG_PRINT in TI_array deleted
 *
 * Revision 1.111  1996/02/12  18:11:30  hw
 * bug fixed in TI_genarray (added typeinformation to N_array node)
 *
 * Revision 1.110  1996/02/12  16:33:47  cg
 * bugs fixed in access to pragma linkname of functions
 * programs without main-function no longer cause segmentation faults.
 *
 * Revision 1.109  1996/02/12  13:31:54  asi
 * added a DBUG_ASSERT in function DuplicateTypes
 *
 * Revision 1.108  1996/02/08  18:12:15  hw
 * - changed functions CmpTypes & CompatibleTypes for use with types
 *    of known dimension, but unknown shape
 * - changed UpdateTypes to update types with known dimension, but unknown shape
 *
 * Revision 1.107  1996/02/08  08:08:11  hw
 * added error-message (if 'psi' has an argument with unknown shape
 *   and the result-type is not declared)
 *
 * Revision 1.106  1996/02/06  16:19:00  hw
 * added typecheck for chars
 *
 * Revision 1.105  1996/02/06  14:55:45  hw
 * - changed overloading of functions
 * - changed comparison of external-function-names
 *
 * Revision 1.104  1996/02/06  14:02:09  cg
 * Now, the entry id_cmod of the types structure is copied as well
 * in function DuplicateTypes (no copy of entire string)
 *
 * Revision 1.103  1996/01/25  16:25:54  hw
 * changed return-value of functions CmpTypes & CompatibleTypes
 * ( the type is a 'enum' cmp_type' which will be created from
 *   cmp_type.mac)
 * - changed all checks of values of these two functions
 * - added typechecking of functions in modules whose arguments
 *   have unknown shape
 * - and  ... (a lot) :-)
 *
 * Revision 1.102  1995/12/05  16:47:55  hw
 * bug fixed in TI_genarray (pointers of types-structure will not be
 *  shared anymore)
 *
 * Revision 1.101  1995/12/05  12:14:49  hw
 * changed UpdateTypes
 *
 * Revision 1.100  1995/12/05  11:05:33  hw
 * changed AddIdToStack ( when generating a  new vardec the name of the variable
 *  will not be shared anymore )
 *
 *
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <limits.h> /* for INT_MIN, INT_MAX */
#include <values.h> /* for MINFLOAT, MINDOUBLE, MAXFLOAT, MAXDOUBLE */

#include "tree.h"
#include "tree_basic.h"
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
#include "internal_lib.h"
#include "access_macros.h"
#include "DupTree.h"
#include "globals.h"
#include "typecheck_WL.h"
#include "gen_pseudo_fun.h"

static node *top_fundef;

#define LOCAL_STACK_SIZE 1000 /* used for scope stack */
#define MAX_ARG_TYPE_LENGTH                                                              \
    1024 /* used in FindFun as size of char[]                                            \
          * to convert the argument-types                                                \
          * of a function call to a sting                                                \
          */

#define PUSH_VAR(name, new_node)                                                         \
    {                                                                                    \
        if (tos < stack_limit) {                                                         \
            DBUG_PRINT ("TYPE", (" push : %s" P_FORMAT " shpseg:" P_FORMAT, name, tos,   \
                                 new_node->SHP));                                        \
            tos->id = name;                                                              \
            tos->node = new_node;                                                        \
            tos++;                                                                       \
        } else                                                                           \
            SYSABORT (("Overflow of local stack"))                                       \
    }

#define GEN_TYPE_NODE(node, type) node = MakeType (type, 0, NULL, NULL, NULL)

/* The following defines are only used in function TI_prf.
 * They are used to select the function that computes the result type
 * of the belonging primitive function.
 * The informtion is stored in prim_fun_tt.mac
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
#define T_TYPES(i) (tab + i)->node->info.types

#define CMP_TYPE_ID(a, b)                                                                \
    ((NULL == a->id_mod)                                                                 \
       ? ((!strcmp (a->id, b->id)) && (NULL == b->id_mod))                               \
       : ((NULL == b->id_mod)                                                            \
            ? (!strcmp (a->id, b->id))                                                   \
            : ((!strcmp (a->id, b->id)) && (!strcmp (a->id_mod, b->id_mod)))))

#define CMP_TYPE_NAME(a, b)                                                              \
    ((NULL == a->name_mod)                                                               \
       ? ((!strcmp (a->name, b->name)) && (NULL == b->name_mod))                         \
       : ((NULL == b->name_mod)                                                          \
            ? (!strcmp (a->name, b->name))                                               \
            : ((!strcmp (a->name, b->name)) && (!strcmp (a->name_mod, b->name_mod)))))

#define CMP_TYPE_HIDDEN(a, b)                                                            \
    ((NULL == a->name)                                                                   \
       ? ((NULL == b->name)                                                              \
            ? 0                                                                          \
            : (!(strcmp (a->id, b->name) && CMP_MOD (a->id_mod, b->name_mod))))          \
       : ((NULL == b->name)                                                              \
            ? (!(strcmp (a->name, b->id) && CMP_MOD (a->name_mod, b->id_mod)))           \
            : (!(strcmp (a->name, b->name) && CMP_MOD (a->name_mod, b->name_mod)))))

#define CMP_FUN_NAME(a, b) CMP_TYPE_ID (a, b)

#define CMP_TYPE_MOD(a, b) ((NULL == a) ? 1 : ((NULL == b) ? 0 : (!strcmp (a, b))))

#define FREE_TYPES(a) FreeOneTypes (a)

#define CHECK_FUNCTION_TYPENAMECLASH(arg)                                                \
    {                                                                                    \
        if (NULL != LookupType (arg->ID, NULL, NODE_LINE (arg))) {                       \
            ERROR (NODE_LINE (arg),                                                      \
                   ("Function '%s` has same name as user-defined type", arg->ID));       \
        }                                                                                \
        if ((NULL != FUNDEF_PRAGMA (arg)) && (NULL != FUNDEF_LINKNAME (arg)))            \
            if (NULL != LookupType (FUNDEF_LINKNAME (arg), NULL, NODE_LINE (arg))) {     \
                ERROR (NODE_LINE (arg), ("Link name '%s` of function '%s` has same name" \
                                         " as user-defined type",                        \
                                         FUNDEF_LINKNAME (arg), arg->ID));               \
            }                                                                            \
    }

#define CHECK_TYPENAMECLASH(arg)                                                         \
    if (NULL != LookupType (arg->ID, NULL, NODE_LINE (arg))) {                           \
        ERROR (NODE_LINE (arg),                                                          \
               ("Identifier '%s` has same name as user-defined type", arg->ID));         \
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
        DBUG_PRINT ("TYPE", (" insert :" P_FORMAT " %s:%s", tmp,                         \
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
        MAKENODE_NUM (Node, neutral);                                                    \
        break;                                                                           \
    case T_float:                                                                        \
        MAKENODE_FLOAT (Node, neutral);                                                  \
        break;                                                                           \
    case T_bool:                                                                         \
        MAKENODE_BOOL (Node, neutral);                                                   \
        break;                                                                           \
    case T_double:                                                                       \
        MAKENODE_DOUBLE (Node, neutral);                                                 \
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

/*forward declaration */
types *TI_prf (node *arg_node, node *arg_info);
types *TI_ap (node *arg_node, node *arg_info);
types *TI_array (node *arg_node, node *arg_info);
types *TI (node *arg_node, node *arg_info);
types *TI_cast (node *arg_node, node *arg_info);
types *TI_with (node *arg_node, node *arg_info);

types *TI_Nwith (node *, node *);
types *TI_Npart (node *, types *, node *);
types *TI_Ngenarray (node *, node *, node **);
void ConsistencyCheckModarray (int lineno, types *array_type, types *generator_type,
                               types *body_type);
types *TI_Nfoldprf (node *arg_node, types *block_type, types *neutral, node *arg_info);
types *TI_Nfoldfun (node *arg_node, types *body_type, types *neutral, node *arg_info);

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
    id *id;                    /* name of function */
    id *id_mod;                /* modul name of function */
    int tag;                   /* typecheck status */
    int n_dub;                 /* number of remaining dublications */
    struct FUN_TAB_ELEM *next; /* next fun_tab_elem node */
} fun_tab_elem;

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
 * function:
 *   Type2Vec(types *type)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

static node *
Type2Vec (types *type)
{
    node *res;
    int i;

    DBUG_ENTER ("Type2Vec");

    res = NULL;

    DBUG_ASSERT ((TYPES_DIM (type) > 0), "Type2Vec() called with type with dim <= 0");

    for (i = TYPES_DIM (type) - 1; i >= 0; i--) {
        res = MakeExprs (MakeNum (TYPES_SHAPE (type, i)), res);
    }

    res = MakeArray (res);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *BuildPsiWithLoop(types *restype, node *idx, node *array)
 *
 * description:
 *
 *   This function creates one large with-loop that replaces a single function
 *   application of a primitive psi() with an array as value.
 *
 *   Example:
 *
 *     idx = [M,M];
 *     A   = reshape( [N, N, N, N], [ ... ]));
 *     a   = psi( idx, A);
 *
 *   is transformed into
 *
 *     a = with ( . <= [ _tmp_4, _tmp_3 ] <= . )
 *         {
 *           _tmp_8 = [0];
 *           _tmp_6 = psi( _tmp_8, idx);
 *           _tmp_7 = [1];
 *           _tmp_5 = psi( _tmp_7, idx);
 *
 *           _tmp_2 = [ _tmp_6, _tmp_5, _tmp_4, _tmp_3 ];
 *           _tmp_1 = psi( _tmp_2, A);
 *         }
 *         genarray( [M,M], _tmp_1);
 *
 *
 ******************************************************************************/

static node *
BuildPsiWithLoop (types *restype, node *idx, node *array)
{
    int i, num_fresh_vars, var_offset;
    char **tmp_vars;
    node *aelems, *res, *assign;
    ids *iv_scalars, *last_iv_scalars;

    DBUG_ENTER ("BuildPsiWithLoop");

    DBUG_ASSERT ((NODE_TYPE (idx) == N_id), "first arg to psi() must be N_id");
    DBUG_ASSERT ((NODE_TYPE (array) == N_id), "second arg to psi() must be N_id");

    num_fresh_vars = 2 * ID_DIM (array) - TYPES_DIM (restype) + 2;

    tmp_vars = (char **)Malloc (num_fresh_vars * sizeof (char *));

    for (i = 0; i < num_fresh_vars; i++) {
        tmp_vars[i] = TmpVar ();
    }

    assign = MakeAssign (MakeLet (MakePrf2 (F_psi,
                                            MakeId (StringCopy (tmp_vars[1]), NULL,
                                                    ST_regular),
                                            array),
                                  MakeIds (tmp_vars[0], NULL, ST_regular)),
                         NULL);

    aelems = NULL;

    for (i = 2; i < ID_DIM (array) + 2; i++) {
        aelems = MakeExprs (MakeId (StringCopy (tmp_vars[i]), NULL, ST_regular), aelems);
    }

    assign
      = MakeAssign (MakeLet (MakeArray (aelems), MakeIds (tmp_vars[1], NULL, ST_regular)),
                    assign);

    var_offset = ID_DIM (array) - TYPES_DIM (restype);

    for (i = 2 + TYPES_DIM (restype); i < 2 + ID_DIM (array); i++) {
        assign
          = MakeAssign (MakeLet (MakePrf2 (F_psi,
                                           MakeId (StringCopy (tmp_vars[i + var_offset]),
                                                   NULL, ST_regular),
                                           MakeId (StringCopy (ID_NAME (idx)), NULL,
                                                   ST_regular)),
                                 MakeIds (tmp_vars[i], NULL, ST_regular)),
                        assign);

        aelems = MakeExprs (MakeNum ((var_offset - 1) - (i - (2 + TYPES_DIM (restype)))),
                            NULL);

        assign
          = MakeAssign (MakeLet (MakeArray (aelems),
                                 MakeIds (tmp_vars[i + var_offset], NULL, ST_regular)),
                        assign);
    }

    last_iv_scalars = NULL;

    for (i = 2; i < TYPES_DIM (restype) + 2; i++) {
        iv_scalars = MakeIds (tmp_vars[i], NULL, ST_regular);
        IDS_NEXT (iv_scalars) = last_iv_scalars;
        last_iv_scalars = iv_scalars;
    }

    res
      = MakeNWith (MakeNPart (MakeNWithid (NULL, iv_scalars),
                              MakeNGenerator (NULL, NULL, F_le, F_le, NULL, NULL), NULL),
                   MakeNCode (MakeBlock (assign, NULL),
                              MakeId (StringCopy (tmp_vars[0]), NULL, ST_regular)),
                   MakeNWithOp (WO_genarray));

    NWITHOP_SHAPE (NWITH_WITHOP (res)) = Type2Vec (restype);
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
 *
 *
 *
 ******************************************************************************/

static node *
BuildGenarrayWithLoop (node *shp, node *val)
{
    node *res, *tmp_node;
    int i;

    DBUG_ENTER ("BuildGenarrayWithLoop");

    res
      = MakeNWith (MakeNPart (MakeNWithid (MakeIds (TmpVar (), NULL, ST_regular), NULL),
                              MakeNGenerator (NULL, NULL, F_le, F_le, NULL, NULL), NULL),
                   MakeNCode (MAKE_EMPTY_BLOCK (), val), MakeNWithOp (WO_genarray));
    if (NODE_TYPE (shp) == N_array) {
        NWITHOP_SHAPE (NWITH_WITHOP (res)) = shp;
    } else {
        /*
         * unflatten the array
         */
        tmp_node = NULL;
        for (i = ID_ARRAYLENGTH (shp) - 1; i >= 0; i--)
            tmp_node = MakeExprs (MakeNum (ID_CONSTARRAY (shp)[i]), tmp_node);
        tmp_node = MakeArray (tmp_node);
        ARRAY_LENGTH (tmp_node) = ID_ARRAYLENGTH (shp);
        ARRAY_INTARRAY (tmp_node)
          = CopyIntArray (ID_ARRAYLENGTH (shp), ID_CONSTARRAY (shp));
        ARRAY_TYPE (tmp_node) = VARDEC_TYPE (ID_VARDEC (shp));
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

    body = MakeAssign (MakeLet (MakePrf (F_psi, MakeExprs (MakeId (StringCopy (iv), NULL,
                                                                   ST_regular),
                                                           MakeExprs (array, NULL))),
                                MakeIds (wl_body_var, NULL, ST_regular)),
                       NULL);

    res
      = MakeNWith (MakeNPart (MakeNWithid (MakeIds (iv, NULL, ST_regular), NULL),
                              MakeNGenerator (NULL, NULL, F_le, F_le, NULL, NULL), NULL),
                   MakeNCode (MakeBlock (body, NULL),
                              MakeId (StringCopy (wl_body_var), NULL, ST_regular)),
                   MakeNWithOp (WO_genarray));

    /* NWITHOP_SHAPE(NWITH_WITHOP(res)) = take_shp; */

    if (NODE_TYPE (take_shp) == N_array) {
        NWITHOP_SHAPE (NWITH_WITHOP (res)) = take_shp;
    } else {
        /*
         * unflatten the array
         */
        int i;
        node *tmp_node = NULL;
        for (i = ID_ARRAYLENGTH (take_shp) - 1; i >= 0; i--)
            tmp_node = MakeExprs (MakeNum (ID_CONSTARRAY (take_shp)[i]), tmp_node);
        tmp_node = MakeArray (tmp_node);
        ARRAY_LENGTH (tmp_node) = ID_ARRAYLENGTH (take_shp);
        ARRAY_INTARRAY (tmp_node)
          = CopyIntArray (ID_ARRAYLENGTH (take_shp), ID_CONSTARRAY (take_shp));
        ARRAY_TYPE (tmp_node) = VARDEC_TYPE (ID_VARDEC (take_shp));
        NWITHOP_SHAPE (NWITH_WITHOP (res)) = tmp_node;
    }
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
 *   node *BuildDropWithLoop(types *new_shape, node *drop_vec, node *array)
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

static node *
BuildDropWithLoop (types *new_shape, node *drop_vec, node *array)
{
    node *res, *body, *aelem;
    char *wl_body_var_vec;
    char *wl_body_var_elem;
    char *iv;
    int i, len_vec, dim_array;

    DBUG_ENTER ("BuildDropWithLoop");

    DBUG_ASSERT (((NODE_TYPE (drop_vec) == N_array)
                  || ((NODE_TYPE (drop_vec) == N_id)
                      && (ID_ARRAYLENGTH (drop_vec) > SCALAR))),
                 "First arg of drop should be a constant array!");

    wl_body_var_vec = TmpVar ();
    wl_body_var_elem = TmpVar ();
    iv = TmpVar ();

    /*
     * First, we expand the length of "drop_vec" to the rank of "array"
     * with 0's.
     */
    if (NODE_TYPE (drop_vec) == N_array) {
        len_vec = TYPES_SHAPE (ARRAY_TYPE (drop_vec), 0);
        dim_array = TYPES_DIM (new_shape); /* shape(result) == shape(array) !! */

        if (len_vec < dim_array) {
            aelem = ARRAY_AELEMS (drop_vec);
            while (EXPRS_NEXT (aelem) != NULL) {
                aelem = EXPRS_NEXT (aelem);
            }
            for (i = len_vec; i < dim_array; i++) {
                EXPRS_NEXT (aelem) = MakeExprs (MakeNum (0), NULL);
                aelem = EXPRS_NEXT (aelem);
            }
        }
    } else { /* NODE_TYPE(drop_vec) == N_id !! */
        len_vec = TYPES_SHAPE (VARDEC_TYPE (ID_VARDEC (drop_vec)), 0);
        dim_array = TYPES_DIM (new_shape); /* shape(result) == shape(array) !! */
    }

    body
      = MakeAssign (MakeLet (MakePrf (F_add, MakeExprs (MakeId (StringCopy (iv), NULL,
                                                                ST_regular),
                                                        MakeExprs (drop_vec, NULL))),
                             MakeIds (wl_body_var_vec, NULL, ST_regular)),
                    MakeAssign (MakeLet (MakePrf (F_psi,
                                                  MakeExprs (MakeId (StringCopy (
                                                                       wl_body_var_vec),
                                                                     NULL, ST_regular),
                                                             MakeExprs (array, NULL))),
                                         MakeIds (wl_body_var_elem, NULL, ST_regular)),
                                NULL));

    res
      = MakeNWith (MakeNPart (MakeNWithid (MakeIds (iv, NULL, ST_regular), NULL),
                              MakeNGenerator (NULL, NULL, F_le, F_le, NULL, NULL), NULL),
                   MakeNCode (MakeBlock (body, NULL),
                              MakeId (StringCopy (wl_body_var_elem), NULL, ST_regular)),
                   MakeNWithOp (WO_genarray));

    NWITHOP_SHAPE (NWITH_WITHOP (res)) = Type2Vec (new_shape);
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
 *   node *BuildCatWithLoop2(ids *lhs, node *arg1, node *arg2, node *arg3)
 *
 * description:
 *
 *
 *
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

    body = MakeAssign (MakeLet (MakePrf (F_psi, MakeExprs (MakeId (StringCopy (iv), NULL,
                                                                   ST_regular),
                                                           MakeExprs (array1, NULL))),
                                MakeIds (wl_body_var, NULL, ST_regular)),
                       NULL);

    res = MakeNWith (MakeNPart (MakeNWithid (MakeIds (iv, NULL, ST_regular), NULL),
                                MakeNGenerator (NULL,
                                                (ID_DIM (array1) > SCALAR)
                                                  ? Type2Vec (ID_TYPE (array1))
                                                  : MakePrf1 (F_shape,
                                                              DupTree (array1, NULL)),
                                                F_le, F_lt, NULL, NULL),
                                NULL),
                     MakeNCode (MakeBlock (body, NULL),
                                MakeId (StringCopy (wl_body_var), NULL, ST_regular)),
                     MakeNWithOp (WO_genarray));

    NWITHOP_SHAPE (NWITH_WITHOP (res)) = Type2Vec (new_shape);

    NCODE_USED (NWITH_CODE (res))++;

    /*
     * Finally, we generate the connection between the
     * (only) partition and the (only) code!
     */
    NPART_CODE (NWITH_PART (res)) = NWITH_CODE (res);

    DBUG_RETURN (res);
}

static node *
BuildCatWithLoop2 (ids *lhs, node *arg1, node *arg2, node *arg3)
{
    node *res, *body, *start_vec, *start_vec_copy;
    char *wl_body_var_vec;
    char *wl_body_var_elem;
    char *wl_body_var_offset;
    char *iv;
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
    start_vec_copy = NULL;

    for (i = ID_DIM (arg2) - 1; i > NUM_VAL (arg1); i--) {
        start_vec = MakeExprs (MakeNum (0), start_vec);
        start_vec_copy = MakeExprs (MakeNum (0), start_vec_copy);
    }

    start_vec = MakeExprs (MakeNum (ID_SHAPE (arg2, NUM_VAL (arg1))), start_vec);
    start_vec_copy
      = MakeExprs (MakeNum (ID_SHAPE (arg2, NUM_VAL (arg1))), start_vec_copy);

    for (i = NUM_VAL (arg1) - 1; i >= 0; i--) {
        start_vec = MakeExprs (MakeNum (0), start_vec);
        start_vec_copy = MakeExprs (MakeNum (0), start_vec_copy);
    }

    start_vec = MakeArray (start_vec);
    start_vec_copy = MakeArray (start_vec_copy);

    body
      = MakeAssign (MakeLet (start_vec, MakeIds (wl_body_var_offset, NULL, ST_regular)),
                    MakeAssign (MakeLet (MakePrf2 (F_sub,
                                                   MakeId (StringCopy (iv), NULL,
                                                           ST_regular),
                                                   MakeId (StringCopy (
                                                             wl_body_var_offset),
                                                           NULL, ST_regular)),
                                         MakeIds (wl_body_var_vec, NULL, ST_regular)),
                                MakeAssign (MakeLet (MakePrf2 (F_psi,
                                                               MakeId (StringCopy (
                                                                         wl_body_var_vec),
                                                                       NULL, ST_regular),
                                                               arg3),
                                                     MakeIds (wl_body_var_elem, NULL,
                                                              ST_regular)),
                                            NULL)));

    res = MakeNWith (MakeNPart (MakeNWithid (MakeIds (iv, NULL, ST_regular), NULL),
                                MakeNGenerator (start_vec_copy, NULL, F_le, F_le, NULL,
                                                NULL),
                                NULL),
                     MakeNCode (MakeBlock (body, NULL),
                                MakeId (StringCopy (wl_body_var_elem), NULL, ST_regular)),
                     MakeNWithOp (WO_modarray));

    NWITHOP_ARRAY (NWITH_WITHOP (res))
      = MakeId (StringCopy (IDS_NAME (lhs)), NULL, ST_regular);

    NCODE_USED (NWITH_CODE (res))++;

    /*
     * Finally, we generate the connection between the
     * (only) partition and the (only) code!
     */
    NPART_CODE (NWITH_PART (res)) = NWITH_CODE (res);

    DBUG_RETURN (res);
}

/*
 *
 *  functionname  : DuplicateTypes
 *  arguments     : 1) types node
 *                  2) tag to indicate whether to copy( 2)==1 ) or
 *                     to share( 2) != 1) the strings used in the struct 'types'
 *  description   : duplicates 1)
 *  global vars   :
 *  internal funs : StringCopy
 *  external funs :
 *  macros        : DBUG..., GEN_NODE, NULL
 *
 *  remarks       : DuplicateTypes  can also be used for types of
 *                  userdefined function.
 *
 */

types *
DuplicateTypes (types *source, int share)
{
    types *return_types, *tmp;
    int i;

    DBUG_ENTER ("DuplicateTypes");

    if (source == NULL) {
        return_types = NULL;
    } else {
        tmp = GEN_NODE (types);
        if (tmp == NULL)
            SYSABORT (("Out of memory"));

        return_types = tmp;

        do {
            tmp->dim = source->dim;
            TYPES_BASETYPE (tmp) = TYPES_BASETYPE (source);
            if (source->dim > 0) {
                DBUG_ASSERT ((source->dim <= SHP_SEG_SIZE), "dimension out of range");
                tmp->shpseg = (shpseg *)Malloc (sizeof (shpseg));
                DBUG_ASSERT ((NULL != source->shpseg), "types-structur without shpseg");
                for (i = 0; i < source->dim; i++)
                    tmp->shpseg->shp[i] = source->shpseg->shp[i];
                tmp->shpseg->next = NULL;
            } else
                tmp->shpseg = NULL;

            tmp->id = StringCopy (source->id);
            tmp->name = StringCopy (source->name);

            DBUG_PRINT ("TYPE", ("new type" P_FORMAT ",old " P_FORMAT, tmp, source));
            DBUG_PRINT ("TYPE",
                        ("new id" P_FORMAT ", old id" P_FORMAT, tmp->id, source->id));
            DBUG_PRINT ("TYPE", ("new name" P_FORMAT ", old name" P_FORMAT, tmp->name,
                                 source->name));

            /*
             *  Sharing of module names is common throughout sac2c,
             *  so we can do it here as well.
             */

            tmp->id_mod = source->id_mod;
            tmp->name_mod = source->name_mod;
            tmp->id_cmod = source->id_cmod;
            tmp->attrib = source->attrib;
            tmp->status = source->status;
            TYPES_TDEF (tmp) = TYPES_TDEF (source);

            if (source->next == NULL)
                tmp->next = NULL;
            else {
                tmp->next = GEN_NODE (types);
                tmp = tmp->next;
            }
            source = source->next;
        } while (source != NULL);
    }

    DBUG_RETURN (return_types);
}

/*
 *
 *  functionname  : LookupObject
 *  arguments     : 1) name of object
 *                  2) module name to look for object (maybe NULL)
 *                  3) line number (for error messages only)
 *  description   : searches for the objdef-node which defines the given
 *                  object. The pointer to this objdef node is returned,
 *                  if it's found and NULL else.
 *  global vars   : object_table
 *  internal funs : ---
 *  external funs : FindSymbolInModul, ModulePrefix, strcmp
 *  macros        : CMP_OBJ_OBJDEF
 *
 *  remarks       : Search-Strategy:
 *
 *                  If no module name is given, then the object is first
 *                  searched in the current module. If it's not found there,
 *                  it is looked for in all imported modules. An error
 *                  message occurs if more than one object matches.
 *
 *                  If a module name is given, then the object is first
 *                  searched in this particular module. If it's not found
 *                  there, then the search is continued in all modules
 *                  which are imported by this module. An error
 *                  message occurs if more than one object matches.
 *
 */

#if 0
node *LookupObject(char *name, char *mod, int line)
{
  node *tmp, *found=NULL;
  mods *mods_avail;
  char *prefix=module_name;
  
  DBUG_ENTER("LookupObject");
  
  tmp=object_table;

  if ((mod!=NULL) && (strcmp(mod, module_name)!=0))
  {
    prefix=ModulePrefix(mod);
  }

  while ((tmp!=NULL) 
         && (CMP_OBJ_OBJDEF(name, prefix, tmp)==0))
  {
    tmp=OBJDEF_NEXT(tmp);
  }
    
  if (tmp!=NULL)
  {
    found=tmp;
  }
  else  
  {
    if (mod==NULL)
    {
      tmp=object_table;

      while (tmp!=NULL)
      {
        if (strcmp(name, OBJDEF_NAME(tmp))==0)
        {
          if (found==NULL)
          {
            found=tmp;
          }
          else
          {
            ERROR(line,
                  ("Identifier '%s` matches more than one global object",
                   name));
          }
        }
        tmp=OBJDEF_NEXT(tmp);
      }
    }
    else
    {
      mods_avail=FindSymbolInModul(mod, name, 3, NULL, 1);
      while (mods_avail!=NULL)
      {
        tmp=object_table;
        while (tmp!=NULL)
        {
          if (CMP_OBJ_OBJDEF(name, mods_avail->mod->prefix, tmp)==1)
          {
            if (found==NULL)
            {
              found=tmp;
            }
            else
            {
              ERROR(line,
                    ("Identifier '%s` matches more than one global object",
                     name));
            }
          }
          tmp=OBJDEF_NEXT(tmp);
        }
        mods_avail=mods_avail->next;
      }
    }
  }
  
  DBUG_RETURN(found);
}
#endif

node *
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

/*
 *
 *  functionname  : LookupType
 *  arguments     : 1) name of type
 *                  2) name of modul, where type is defined
 *  description   : looks type up and returns pointer to corresponding
 *                  N_typedef node if found or NULL if not found
 *  global vars   : type_table, type_tab_size, filename
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., NULL, ID_MOD, ID, CMP_MOD, ABORT
 *
 *  remarks       :
 *
 */
node *
LookupType (char *type_name, char *mod_name, int line)
{
    int i, defined;
    node *ret_node = NULL;
    mods *mods[2];

    DBUG_ENTER ("LookupType");

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
                        if (NULL != mods[i])
                            if (NULL != mods[i]->next) {
                                defined += 2;
                                break;
                            } else
                                defined += 1;
                    }

                    if (0 == defined)
                        ABORT (line,
                               ("Type '%s` is not defined or imported in module '%s`",
                                type_name, mod_name));

                    if (2 <= defined)
                        ABORT (line, ("Type '%s` is defined or"
                                      " imported more than once in module '%s`",
                                      type_name, mod_name));

                    DBUG_ASSERT ((1 == defined), "defined != 1");
                    if (NULL == mods[0])
                        mod_name = mods[1]->mod->name;
                    else
                        mod_name = mods[0]->mod->name;
                } else
                    mod_name = mods[1]->mod->name;
            } else
                mod_name = mods[0]->mod->name;
        }
        for (i = 0; i < type_tab_size; i++)
            if (CMP_MOD (mod_name, (type_table + i)->node->ID_MOD)
                && (!strcmp (type_name, (type_table + i)->node->ID))) {
                ret_node = (type_table + i)->node;
                break;
            }
    } else {
        for (i = 0; i < type_tab_size; i++)
            if (!strcmp (type_name, (type_table + i)->node->ID))
                if (1 == (type_table + i)->id_count) {
                    ret_node = (type_table + i)->node;
                    break;
                } else {
                    ABORT (line, ("Type '%s` has more than one definition", type_name));
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
 *  internal funs :
 *  external funs : strcmp, Type2String
 *  macros        : DBUG..., P_FORMAT
 *
 *  remarks       :
 *
 */
stack_elem *
LookupVar (char *id)
{
    stack_elem *tmp, *bottom;
    int is_defined = 0;
#ifndef DBUG_OFF
    char *db_str;
#endif

    DBUG_ENTER ("LookupVar");

    DBUG_PRINT ("TYPE", ("looking for id: %s", id));

    bottom = act_frame;
    tmp = tos - 1;

    while ((tmp >= bottom) && !(is_defined)) {
        DBUG_PRINT ("TYPE", ("current id: %s" P_FORMAT, tmp->id, tmp));

        if (!(strcmp (tmp->id, id)))
            is_defined = 1;
        else
            tmp--;
    }

    if (is_defined) {
#ifndef DBUG_OFF
        db_str = Type2String (tmp->node->info.types, 0);
        DBUG_PRINT ("TYPE", ("found: %s %s", db_str, id));
        FREE (db_str);
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
fun_tab_elem *
LookupFun (char *fun_name, char *mod_name, node *fun_node)
{
    fun_tab_elem *tmp;

    DBUG_ENTER ("LookupFun");

    if (NULL == fun_node) {
        DBUG_PRINT ("TYPE", ("looking for function '%s'", ModName (mod_name, fun_name)));

        for (tmp = fun_table; !END_OF_FUN_TAB (tmp); tmp = NEXT_FUN_TAB_ELEM (tmp))
            if (!strcmp (fun_name, tmp->id))
                if (NULL == mod_name)
                    break;
                else if (CMP_MOD (mod_name, tmp->id_mod))
                    break;
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
        DBUG_PRINT ("TYPE", ("function found" P_FORMAT, tmp));

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
prim_fun_tab_elem *
LookupPrf (int prf, char *mod_name)
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

/*
 *
 *  functionname  : Compute NeutralElem
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs : ----
 *  external funs : ----
 *  macros        : DBUG...,
 *  remarks       : ----
 *
 */
node *
ComputeNeutralElem (prf prf_fun, types *neutral_type)
{
    node *neutral_elem = NULL;
    node *neutral_base = NULL;

    node *tmp;
    int i, length;
    simpletype stype;

    DBUG_ENTER ("ComputeNeutralElem");

    GET_LENGTH (length, neutral_type);
    GET_BASIC_SIMPLETYPE (stype, neutral_type);

    switch (prf_fun) {
    case F_add:
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

    case F_mul:
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
            neutral_base = MakeFloat (MAXFLOAT);
            break;
        case T_double:
            neutral_base = MakeDouble (MAXDOUBLE);
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
            neutral_base = MakeFloat (-MAXFLOAT);
            break;
        case T_double:
            neutral_base = MakeDouble (-MAXDOUBLE);
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
        tmp = MakeExprs (neutral_base, NULL);

        for (i = 1; i < length; i++) {
            tmp = MakeExprs (DupNode (neutral_base), tmp);
        }

        neutral_elem = MakeArray (tmp);

        GET_BASIC_TYPE (ARRAY_TYPE (neutral_elem), neutral_type, -64);
    } else {
        neutral_elem = neutral_base;
    }

    DBUG_RETURN (neutral_elem);
}

#if 0
node *ComputeNeutralElem(prf prf_fun, types *neutral_type)
{
   node *neutral_elem=NULL, *tmp;
   int  i,length, neutral_int;
   float neutral_float;
   double neutral_double;
   simpletype stype;
   
   DBUG_ENTER("ComputeNeutralElem");

   GET_LENGTH(length, neutral_type);
   GET_BASIC_SIMPLETYPE(stype, neutral_type);
      
   switch(prf_fun)
   {
   case F_add: case F_add_AxA: 
      neutral_int=0;
      break;
   case F_mul: case F_mul_AxA: 
      neutral_int=1;
      break;
   case F_and:
      neutral_int=1;
      break;
   case F_or:
      neutral_int=0;
      break;
   case F_min:
      break;
   case F_max:
      break;
   default:
      DBUG_PRINT("NEUTRAL",("prf :%s (%d)",mdb_prf[prf_fun], prf_fun));
      DBUG_ASSERT(0,"wrong N_prf-tag");
   }

   if(length>0)
   {
      neutral_elem=MakeNode(N_array);
      tmp=MakeNode(N_exprs);
      neutral_elem->node[0]=tmp;
      for(i=1; i<=length; i++)
      {
         MAKE_NEUTRAL_ELEM(tmp->node[0], stype, neutral_int);
         if(i<length)
         {
            tmp->node[1]=MakeNode(N_exprs);
            tmp=tmp->node[1];
         }
      }
      GET_BASIC_TYPE(neutral_elem->TYPES, neutral_type, -64);
   }
   else
   {
      MAKE_NEUTRAL_ELEM(neutral_elem, stype, neutral_int);
   }
   
   DBUG_RETURN(neutral_elem);
}
#endif /* 0 */

/*
 *
 *  functionname  : TI_fun
 *  arguments     : 1) argument node
 *                  2) pointer to fun_tab
 *                  3) info node
 *  description   : checks whether function ( 2) ) has to be typechecked or not
 *                  - calls Trav to typecheck function if necessary
 *                  - returns infered type of the function
 *
 *  global vars   :
 *  internal funs : DuplicateTypes
 *  external funs : Trav, Type2String, strcmp
 *  macros        : DBUG...,FREE, NULL,  BODY, ABORT, FAST_CHECK, ID_MOD,
 *                  GEN_TYPE_NODE, FUN_MOD_NAME, FUN_MOD_NAME, P_FORMAT,
 *                  IS_CHECKED, TMP_CHECKED, CHECKING, DO_NOT_SET_FUN_MOD_NAME
 *  remarks       : - arg_node->info.fun_name... contains the name and the modul
 *                    name
 *                  - the compilation-macro DO_NOT_SET_FUN_MOD_NAME can be set
 *                    while calling a c-compiler.
 *                    The module_name belonging to a function( the place where
 *                    it is defined) will not be added or updated while
 *                    typechecking, if the macro is set.
 */
types *
TI_fun (node *arg_node, fun_tab_elem *fun_p, node *arg_info)
{
    types *return_type;
#ifndef DBUG_OFF
    char *db_str;
#endif

    DBUG_ENTER ("TI_fun");

    if (fun_p->node == arg_info->node[0]->node[1])
        if (BODY == arg_info->node[0]->info.cint) {
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
                    if (0 != strcmp (arg_node->FUN_MOD_NAME, fun_p->node->ID_MOD)) {
                        FREE (arg_node->FUN_MOD_NAME);
                        arg_node->FUN_MOD_NAME = StringCopy (fun_p->node->ID_MOD);
                    }
                } else
                    arg_node->FUN_MOD_NAME = StringCopy (fun_p->node->ID_MOD);
#endif
                return_type = DuplicateTypes (fun_p->node->info.types, 1);

#ifndef DBUG_OFF
                db_str = Type2String (return_type, 0);
                DBUG_PRINT ("TYPE", ("return_type %s" P_FORMAT, db_str, return_type));
                FREE (db_str);
#endif
            }
        }
    else {
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
            || ((BODY != arg_info->node[0]->info.cint)
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
                arg_node->FUN_MOD_NAME = fun_p->node->ID_MOD;
#endif
                return_type = DuplicateTypes (fun_p->node->info.types, 1);

#ifndef DBUG_OFF
                db_str = Type2String (return_type, 0);
                DBUG_PRINT ("TYPE", ("return_type %s" P_FORMAT, db_str, return_type));
                FREE (db_str);
#endif
            } else
                ABORT (NODE_LINE (arg_node),
                       ("Type of function '%s' not inferable", arg_node->info.id));
        }
    }
    DBUG_RETURN (return_type);
}

/*
 *
 *  functionname  : LookupVardec
 *  arguments     : 1) variable to look for
 *                  2) pointer to variable declarations
 *  description   : looks for 1) in 2)
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcmp
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
LookupVardec (char *id, node *vardec)
{
    DBUG_ENTER ("LookupVardec");

    while ((vardec != NULL) && (strcmp (VARDEC_NAME (vardec), id) != 0)) {
        vardec = VARDEC_NEXT (vardec);
    }

    DBUG_RETURN (vardec);
}

/*
 *
 *  functionname  : LookupArg
 *  arguments     : 1) variable to look for
 *                  2) pointer to argument list
 *  description   : looks for 1) in 2)
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcmp
 *  macros        :
 *
 *  remarks       : equivalent to LookupVardec
 *
 */

node *
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
 */
int
CheckIds (ids *arg_ids, node *vardec, int line)
{
    ids *tmp, *not_checked_ids;
    id *current_id;
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
 *                  - formal paramters of  external functions
 *                    (NULL==info.types->id_mod) will not be checked
 *                    (only result_type)
 */
int
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
 *  global vars   : ----
 *  internal funs : ----
 *  external funs : ----
 *  macros        : DBUG..., NULL, DIM, TYPES, CMP_TYPE_ID, SIMPLETYPE
 *
 *  remarks       : ----
 *
 */
int
CmpFunParams (node *arg1, node *arg2)
{
    int i, is_equal;

    DBUG_ENTER ("CmpFunParams");

    while ((NULL != arg1) && (NULL != arg2)) {
        if (TYPES_BASETYPE (ARG_TYPE (arg1)) == TYPES_BASETYPE (ARG_TYPE (arg2))) {
            if ((TYPES_BASETYPE (ARG_TYPE (arg1)) == T_user)
                || (TYPES_BASETYPE (ARG_TYPE (arg2)) == T_hidden)) {
                is_equal = CMP_TYPE_NAME (arg1->TYPES, arg2->TYPES);
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

    if ((NULL == arg1) && (NULL == arg2)) {
        is_equal = 1;
    } else {
        is_equal = 0;
    }

    DBUG_RETURN (is_equal);
}

/*
 *
 *  functionname  : CheckKnownTypes
 *  arguments     : 1) N_fundef  node
 *  description   : - checks whether the function uses known types in
 *                    returnvalue and arguments
 *                  - if it is a userdefined type the module name where
 *                    the type is defined will be added
 *  global vars   : filename
 *  internal funs : LookupType
 *  external funs : InsertNode
 *  macros        : DBUG...,TYPES, MOD_NAME_CON, SIMPLETYPE, NAME, NAME_MOD, ID
 *                  NULL, MOD, MOD_CON, NULL, ERROR
 *
 *  remarks       : fundef node must have a body.
 *
 */
void
CheckKnownTypes (node *arg_node)
{
    types *fun_type;
    node *arg, *t_node = NULL;

    DBUG_ENTER ("CheckKnownTypes");

    /* first check whether type of function is known */
    fun_type = arg_node->TYPES;
    while (NULL != fun_type) {
        if (T_user == TYPES_BASETYPE (fun_type)) {
            node *t_node
              = LookupType (fun_type->name, fun_type->name_mod, NODE_LINE (arg_node));

            if (NULL == t_node) {
                ERROR (NODE_LINE (arg_node),
                       ("Function '%s` has"
                        " unknown type '%s` in declaration",
                        ModName (arg_node->ID_MOD, arg_node->ID),
                        ModName (fun_type->name_mod, fun_type->name)));
            } else {
                fun_type->name_mod = t_node->ID_MOD;

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
    arg = arg_node->node[2];
    while (NULL != arg) {
        if (T_user == TYPES_BASETYPE (ARG_TYPE (arg))) {
            t_node = LookupType (arg->NAME, arg->NAME_MOD, NODE_LINE (arg_node));

            if (NULL == t_node) {
                ERROR (NODE_LINE (arg_node),
                       ("Formal parameter '%s` of function '%s` "
                        "has unknown type '%s`",
                        arg->ID, ModName (arg_node->ID_MOD, arg_node->ID),
                        ModName (arg->NAME_MOD, arg->NAME)));
            } else {
                arg->NAME_MOD = t_node->ID_MOD;

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
        if (-1 == ARG_DIM (arg)) {
            FUNDEF_ATTRIB (arg_node) = ST_independent;
            DBUG_PRINT ("FUN_TAG",
                        ("Set ATTRIB-tag of function '%s' to %d"
                         " (ST_independent)",
                         ModName (FUNDEF_MOD (arg_node), FUNDEF_NAME (arg_node)),
                         FUNDEF_ATTRIB (arg_node)));
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
int
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
 *                  2) first N_fundef of the syntax_tree
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
node *
DeleteFun (node *arg_node, node *tree)
{
    node *fun_node, *last_node;

    DBUG_ENTER ("DeleteFun");
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), "DeleteFun with non-N_fundef node!");

    last_node = tree;
    if (last_node == arg_node) {
        tree = FUNDEF_NEXT (tree);
        FUNDEF_NEXT (last_node) = NULL;
        DBUG_PRINT ("TYPE", ("removed function %s", FUNDEF_NAME (last_node)));
        FreeTree (last_node);
    } else {
        fun_node = FUNDEF_NEXT (tree);
        while (fun_node != arg_node) {
            last_node = fun_node;
            fun_node = FUNDEF_NEXT (fun_node);
        }
        DBUG_ASSERT ((fun_node == arg_node), " fun not found");
        FUNDEF_NEXT (last_node) = FUNDEF_NEXT (fun_node);
        FUNDEF_NEXT (fun_node) = NULL;
        DBUG_PRINT ("TYPE", ("removed function %s", FUNDEF_NAME (fun_node)));
        FreeTree (fun_node);
    }
    if (NULL == tree) {
        ABORT (0, ("All functions deleted"));
    }

    DBUG_RETURN (tree);
}

/*
 *
 *  functionname  : CheckRest
 *  arguments     : 1) first N_fundef of the syntax_tree
 *                  2) tag: 0: check a sac-program (SAC_PRG)
 *                          1: check a sac-module (implementation)(SAC_MOD)
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
node *
CheckRest (node *arg_node, int kind)
{
    fun_tab_elem *fun_p;

    DBUG_ENTER ("CheckRest");
#ifndef DBUG_OFF
    DBUG_PRINT ("CHECK", ("tags of local functions after first run "));
    for (fun_p = fun_table; !END_OF_FUN_TAB (fun_p); fun_p = NEXT_FUN_TAB_ELEM (fun_p))
        if (NULL != fun_p->node->node[0])
            DBUG_PRINT ("CHECK",
                        ("function %s has tag :%s", fun_p->id, CHECK_NAME (fun_p->tag)));
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
                && (FUNDEF_STATUS (fun_p->node) != ST_imported)
                && (FUNDEF_STATUS (fun_p->node) != ST_objinitfun)
                && (FUNDEF_STATUS (fun_p->node) != ST_foldfun)) {
                WARN (NODE_LINE (fun_p->node),
                      ("Function '%s` is neither used nor has been specialized",
                       ModName (fun_p->id_mod, fun_p->id)));
            }
            if (FUNDEF_STATUS (fun_p->node) != ST_objinitfun) {
                arg_node = DeleteFun (fun_p->node, arg_node);
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
                arg_node = DeleteFun (fun_p->node, arg_node);
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
                arg_node = DeleteFun (fun_p->node, arg_node);
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
int
IsNameInMods (char *name, mods *mods)
{
    int found = 0;

    DBUG_ENTER ("IsNameInMods");
    if (NULL != name) {
        DBUG_PRINT ("TYPE", ("looking for module %s", name));
        if (0 == strcmp (name, (NULL != module_name ? module_name : ""))) {
            found = 1;
            DBUG_PRINT ("TYPE", ("module-name found "));
        } else if (NULL != name)
            while (NULL != mods) {
                DBUG_PRINT ("TYPE", ("current module-name: %s", mods->mod->name));
                if ((!strcmp (name, mods->mod->name))) {
                    found = 1;
                    DBUG_PRINT ("TYPE", ("module-name found "));
                    break;
                } else
                    mods = mods->next;
            }
    }
    DBUG_RETURN (found);
}

/*
 *
 *  functionname  : CheckObjdefs
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

/*
 *
 *  functionname  : InitTypeTab
 *  arguments     : 1) N_modul node
 *  description   : generates a table for userdefined types
 *  global vars   : type_tab_size, filename
 *  internal funs : Malloc
 *  external funs : Type2String, strcmp,sizeof
 *  macros        : DBUG...,T_TYPES, TYPES, ERROR, ABORT, NULL
 *
 *  remarks       : a new N_typedef chain is made too (new order)
 *
 */
type_tab_elem *
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
        if (T_user == TYPES_BASETYPE (TYPEDEF_TYPE (tmp)))
            if (NULL == TYPEDEF_NEXT (tmp))
                break;
            else {
                last_node = tmp;
                tmp = TYPEDEF_NEXT (tmp);
            }
        else {
            /* insert node into type tabel (tab) and remove it from chain */
            if (-1 != TYPEDEF_TYPE (tmp)->dim) {
                name = TYPEDEF_TYPE (tmp)->id;
                mod_name = tmp->TYPES->id_mod;
                /* look whether current type is defined more than once */
                for (k = 0; k < i; k++)
#ifdef OLD_TEST
                    if (!strcmp (name, T_TYPES (k)->id)
                        && (((NULL == mod_name) || (NULL == T_TYPES (k)->id_mod))
                              ? (mod_name == T_TYPES (k)->id_mod)
                              : (!strcmp (mod_name, T_TYPES (k)->id_mod))))
#else
                    if (!strcmp (name, T_TYPES (k)->id)
                        && CMP_TYPE_MOD (mod_name, T_TYPES (k)->id_mod))
#endif /* OLD_TEST */
                    {
                        if (NULL == tmp->TYPES->id_mod) {
                            ABORT (NODE_LINE (tmp),
                                   ("Type '%s` is defined more than once",
                                    tmp->TYPES->id));
                        } else {
                            ABORT (NODE_LINE (tmp), ("Type '%s` is defined more than"
                                                     " once in module/class '%s`",
                                                     tmp->TYPES->id, tmp->TYPES->id_mod));
                        }
                    }

                (tab + i)->node = tmp;
                DBUG_PRINT ("TYPE",
                            ("inserted %d: %s%s" P_FORMAT ", " P_FORMAT, i,
                             (NULL == tmp->TYPES->id_mod) ? "" : tmp->TYPES->id_mod,
                             tmp->TYPES->id, tmp->TYPES, tmp));
                i++;
                if (NULL == TYPEDEF_NEXT (tmp)) {
                    if (N_modul == last_node->nodetype)
                        last_node->node[1] = NULL;
                    else {
                        last_node->node[0] = NULL;
                    }
                    break;
                } else {
                    tmp = tmp->node[0];
                    if (N_modul == last_node->nodetype)
                        last_node->node[1] = tmp;
                    else {
                        DBUG_ASSERT ((N_typedef == last_node->nodetype), "wrong node");
                        last_node->node[0] = tmp;
                    }
                }
            } else {
                ABORT (NODE_LINE (tmp), ("User-defined type '%s` is built of"
                                         " type with unknown shape '%s`",
                                         ModName (tmp->TYPES->id_mod, tmp->TYPES->id),
                                         Type2String (tmp->TYPES, 0)));
            }
        }

    if (i != type_tab_size) {
        /* now insert typedefs that are constructed form userdefined types */

        int k, defined, delete = 0, diff = type_tab_size - i;
        mods *mods[2];

        last_node = modul_node;
        tmp = modul_node->node[1];

        while (i < type_tab_size) {
            name = tmp->info.types->name;
            /* name of type current type is based on*/
            mod_name = tmp->info.types->name_mod;
            /* dito, but name of module */
            if (NULL != mod_name) {
                /* now look for the module where the type is defined */
                defined = 0;
                for (k = 0; k < 2; k++) {
                    mods[k] = FindSymbolInModul (mod_name, name, k, NULL, 1);
                    if (NULL != mods[k])
                        if (NULL != mods[k]->next) {
                            defined += 2;
                            break;
                        } else
                            defined += 1;
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
                if (!strcmp (name, T_TYPES (j)->id)
                    && (((NULL == mod_name) || (NULL == T_TYPES (j)->id_mod))
                          ? (mod_name == T_TYPES (j)->id_mod)
                          : (!strcmp (mod_name, T_TYPES (j)->id_mod))))
#else
                if (!strcmp (name, T_TYPES (j)->id)
                    && CMP_TYPE_MOD (mod_name, T_TYPES (j)->id_mod))
#endif /* OLD_TEST */
                {
                    if (-1 != tmp->TYPES->dim) {
                        /* look whether current type is defined more than once */
                        for (k = 0; k < i; k++)
#ifdef OLD_TEST
                            if (!strcmp (tmp->TYPES->id, T_TYPES (k)->id)
                                && (((NULL == tmp->TYPES->id_mod)
                                     || (NULL == T_TYPES (k)->id_mod))
                                      ? (tmp->TYPES->id_mod == T_TYPES (k)->id_mod)
                                      : (!strcmp (tmp->TYPES->id_mod,
                                                  T_TYPES (k)->id_mod))))
#else
                            if (!strcmp (tmp->TYPES->id, T_TYPES (k)->id)
                                && CMP_TYPE_MOD (tmp->TYPES->id_mod, T_TYPES (k)->id_mod))
#endif /* OLD_TEST */
                            {
                                if (NULL == tmp->TYPES->id_mod) {
                                    ABORT (NODE_LINE (tmp), ("Type '%s` is "
                                                             "defined more than once",
                                                             ModName (tmp->TYPES->id_mod,
                                                                      tmp->TYPES->id)));
                                } else {
                                    ABORT (NODE_LINE (tmp),
                                           ("Type '%s' is defined more "
                                            "than once in module/class '%s`",
                                            tmp->TYPES->id, tmp->TYPES->id_mod));
                                }
                            }

                        /* insert type to 'tab' and reduce it to primitive type */
                        (tab + i)->node = tmp;
                        switch (TYPES_BASETYPE (T_TYPES (j))) {
                        case T_hidden:
                            TYPES_BASETYPE (T_TYPES (i)) = T_hidden;
                            if (NULL != T_TYPES (j)->name)
                                T_TYPES (i)->name = StringCopy (T_TYPES (j)->name);
                            if (NULL != T_TYPES (j)->name_mod)
                                T_TYPES (i)->name_mod
                                  = StringCopy (T_TYPES (j)->name_mod);
                            break;
                        case T_int:
                        case T_bool:
                        case T_float:
                        case T_str:
                            if (0 <= T_TYPES (j)->dim) {
                                TYPES_BASETYPE (T_TYPES (i))
                                  = TYPES_BASETYPE (T_TYPES (j));
                                T_TYPES (i)->name = NULL;
                                T_TYPES (i)->name_mod = NULL;
                                if ((0 < T_TYPES (j)->dim) && (0 == T_TYPES (i)->dim)) {
                                    T_TYPES (i)->shpseg
                                      = (shpseg *)Malloc (sizeof (shpseg));
                                    T_TYPES (i)->shpseg->next = NULL;
                                }

                                for (k = 0; k < T_TYPES (j)->dim; k++) {
                                    T_TYPES (i)->dim += 1;
                                    DBUG_ASSERT ((T_TYPES (i)->dim <= SHP_SEG_SIZE),
                                                 "shape out off range ");
                                    T_TYPES (i)->shpseg->shp[T_TYPES (i)->dim - 1]
                                      = T_TYPES (j)->shpseg->shp[k];
                                }
                            } else {
                                ABORT (NODE_LINE (tmp),
                                       ("User-defined type '%s` is built from type "
                                        "with unknown shape (%s)",
                                        tmp->TYPES->id, Type2String (tmp->TYPES, 0)));
                            }
                            break;
                        default: {
                            DBUG_ASSERT (0, "error in type table");
                            break;
                        }
                        }
                        DBUG_PRINT ("TYPE",
                                    ("inserted %d: %s%s" P_FORMAT ", " P_FORMAT, i,
                                     (NULL == tmp->TYPES->id_mod) ? ""
                                                                  : tmp->TYPES->id_mod,
                                     tmp->TYPES->id, tmp->TYPES, tmp));
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
                                tmp->TYPES->id, Type2String (tmp->TYPES, 0)));
                    }
                }

            if (NULL == tmp->node[0])
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
                } else
                    break; /*break while loop */
            else {
                if (1 == delete) {
                    if (N_modul == last_node->nodetype)
                        last_node->node[1] = tmp->node[0];
                    else
                        last_node->node[0] = tmp->node[0];
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
                ABORT (NODE_LINE (tmp), ("User-defined type '%s` is built of "
                                         "unknown type (%s)",
                                         tmp->TYPES->id, Type2String (tmp->TYPES, 0)));
                tmp = tmp->node[0];
            }
            if (NULL != tmp) {
                ABORT (NODE_LINE (tmp), ("User-defined type '%s` is built of "
                                         "unknown type (%s)",
                                         tmp->TYPES->id, Type2String (tmp->TYPES, 0)));
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
        name = (tab + i)->node->ID;
        (tab + i)->id_count = 0;
        for (k = 0; k < type_tab_size; k++)
            if (!strcmp ((tab + k)->node->ID, name))
                (tab + k)->id_count++;
    }

    DBUG_RETURN (tab);
}

/*
 *
 *  functionname  : InitFunTable
 *  arguments     : 1) pointer to first function declaration (N_fundef)
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
 *  macros        : DBUG..., ERROR, TYPES, ID, ID_MOD, IS_CHECKED, NOT_CHECKED,
 *                  MOD_NAME_CON, INSERT_FUN ,CMP_FUN_NAME, NULL
 *  remarks       : ----
 *
 */
void
InitFunTable (node *arg_node)
{
    node *fun_node;
    fun_tab_elem *fun_p, *last_fun_p, *dummy;
    id *fun_name, *mod_name;

    DBUG_ENTER ("InitFunTable");

    DBUG_ASSERT ((N_fundef == arg_node->nodetype), " arg_node not N_fundef");

    /* first count fundefs and check whether types of external functions are ok*/
    dummy = NEW_FUN_TAB_ELEM; /* this is only a dummy entry and will
                               * be removed later on.
                               */
    fun_table = dummy;
    INIT_FUN_TAB_ELEM (fun_table);
    fun_node = arg_node;
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
            /* fun_name=(NULL == fun_node->node[5])?fun_node->ID
                                                : (char*)fun_node->node[5]; */
            fun_name
              = (NULL == FUNDEF_PRAGMA (fun_node)
                   ? FUNDEF_NAME (fun_node)
                   : (NULL == FUNDEF_LINKNAME (fun_node) ? FUNDEF_NAME (fun_node)
                                                         : FUNDEF_LINKNAME (fun_node)));

            while (NULL != tmp_fun_node) {
                if (NULL == tmp_fun_node->node[0]) {
                    if (NULL == tmp_fun_node->ID_MOD) {
                        /* tmp_fun_name=(NULL != tmp_fun_node->node[5])
                           ?(char*)(tmp_fun_node->node[5]):tmp_fun_node->ID;
                         */
                        tmp_fun_name = (NULL == FUNDEF_PRAGMA (tmp_fun_node)
                                          ? FUNDEF_NAME (tmp_fun_node)
                                          : (NULL == FUNDEF_LINKNAME (tmp_fun_node)
                                               ? FUNDEF_NAME (tmp_fun_node)
                                               : FUNDEF_LINKNAME (tmp_fun_node)));

                        DBUG_PRINT ("CHECK", ("compare funname  %s  with %s", fun_name,
                                              tmp_fun_name));

                        if (0 == strcmp (fun_name, tmp_fun_name))
                            equal_name += 1;
                    }
                    tmp_fun_node = FUNDEF_NEXT (tmp_fun_node);
                } else
                    /* it is a local defined function, so stop checking */
                    break;
            }
            if (0 != equal_name) { /*
                                     WARN(0,("More than one external function with name
                                     '%s`", fun_name)); */
                /*
                 * This is allowed due to overloading external functions
                 * using the linkname pragma.
                 */
            }
        } else
            CheckKnownTypes (fun_node);

        /* now insert function into fun_table */
        fun_name = fun_node->ID;
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
                if (CMP_MOD (fun_p->id_mod, fun_node->ID_MOD)) {
                    if (1 == CmpFunParams (fun_p->node->node[2], fun_node->node[2])) {
                        ERROR (0, ("Function '%s` is defined more"
                                   " than once with equal domain",
                                   ModName (fun_p->id_mod, fun_p->id)));
                        break;
                    }
                }
                fun_p = NEXT_FUN_TAB_ELEM (fun_p);
            } while (END_OF_FUN_TAB (fun_p) ? 0 : (!strcmp (fun_p->id, fun_node->ID)));
            /*         INSERT_FUN(last_fun_p, fun_name, mod_name, fun_node,
                                (NULL == fun_node->node[0])?IS_CHECKED:NOT_CHECKED,
                                max_overload);*/
            INSERT_FUN (last_fun_p, fun_node, max_overload);
        }
        fun_node = fun_node->node[1];
    }
    ABORT_ON_ERROR;

    fun_table = NEXT_FUN_TAB_ELEM (fun_table);
    FREE (dummy);

#if 0   
   /* allocate memory for table */
   fun_table=(fun_tab_elem*)Malloc(sizeof(fun_tab_elem)*size);
   
   DBUG_PRINT("TYPE",("fun table from "P_FORMAT" to "P_FORMAT,
                      fun_table, fun_table+size));
   
   fun_tos=fun_table; /* set pointer to last entry in fun_table */
   
   fun_node=arg_node;
   while(NULL != fun_node)
   {
      fun_name=fun_node->ID;
      mod_name=fun_node->ID_MOD;
      if (NULL == LookupFun(NULL, NULL, fun_node))
      {
         old_tos=fun_tos;
         INSERT_FUN(fun_name, mod_name,fun_node,
                  (NULL == fun_node->node[0])?IS_CHECKED:NOT_CHECKED,
                    max_overload);
         
         /*  now look for functions with equal name (id).
          *  Insert it, if no previous function with equal name & mod_name 
          *  has same argumenttype (domain).
          */
         tmp=fun_node->node[1];
         while(NULL != tmp)
         {
            if (!strcmp(tmp->ID,fun_node->ID ))
            {
               tmp_tos=old_tos;
               while(tmp_tos < fun_tos)
               {
                  if (CMP_MOD(tmp->ID_MOD, fun_node->ID_MOD))
                  {
                     if (1==CmpFunParams(tmp_tos->node->node[2], tmp->node[2]))
                     {
                        ERROR(0, ("Function '%s` is defined more"
                                  " than once with equal domain",
                                  ModName(tmp->ID_MOD, tmp->ID)));
                        break;
                     }
                  }
                  
                  tmp_tos++;
               }
            
               if (tmp_tos == fun_tos)
               {
                  /* there isn't a function with equal name & mod_name and 
                   * equal domain. So insert function to fun_table.
                   */
                  INSERT_FUN(tmp->ID, tmp->ID_MOD, tmp, 
                           (NULL == tmp->node[0])?IS_CHECKED: NOT_CHECKED, 
                             max_overload);
               }
            }
            tmp=tmp->node[1];
         }
      }
      fun_node=fun_node->node[1];
   }
#endif

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : Typecheck
 *  arguments     : 1) syntax tree
 *  description   : initializes and starts typechecking of a sac programm
 *                  - initializes table of primitive functions
 *                  - set stack (scope stack)
 *                  - initializes table of userdefined types
 *                  - initializes table of userdefined functions
 *                  - call Trav to start checking the main function
 *                  - sets stack_limit (end of scope stack )
 *
 *  global vars   : tos, stack, act_tab type_tab, stack_limit,
 *                  filename
 *  internal funs :  Malloc, CheckRest, InitFunTable, LookupFun, InitTypeTab
 *  external funs : free, Trav, InitPrimFunTab
 *  macros        : DBUG..., MOD_NAME_CON, NULL, LOCAL_STACK_SIZE, P_FORMAT,
 *                  FREE, CHECKING, ABORT
 *
 *  remarks       : ----
 *
 */
node *
Typecheck (node *arg_node)
{
    fun_tab_elem *tmp_node;
    int i;

    DBUG_ENTER ("Typecheck");

    /*
     * initialize profile-tool !
     */
    PFfunapcntr[0] = 1; /* main-function */
    for (i = 1; i < PF_MAXFUN; i++)
        PFfunapcntr[i] = 0;

    /*
     * generate a table of primitive functions to look up their
     * types later on
     */
    InitPrimFunTab ();

    act_tab = type_tab;

    stack = (stack_elem *)Malloc (sizeof (stack_elem) * LOCAL_STACK_SIZE);
    stack_limit = LOCAL_STACK_SIZE + stack;
    DBUG_PRINT ("TYPE",
                ("local stack from " P_FORMAT "to " P_FORMAT, stack, stack_limit));
    tos = stack;

    if (N_modul == NODE_TYPE (arg_node)) {
        if (MODUL_TYPES (arg_node) != NULL) {
            type_table = InitTypeTab (arg_node);
        }

        top_fundef = MODUL_FUNS (arg_node);
        module_name = MODUL_NAME (arg_node);
        object_table = MODUL_OBJS (arg_node);
    } else {
        top_fundef = arg_node;
    }

    /*
     *  The following lines start the traversal mechanism for object
     *  definitions. The given types will be looked for in the
     *  type table and name clashes with user-defined types, functions or
     *  other global objects will be detected.
     *
     *    prerequisite:  stack != NULL
     */

    if (N_modul == arg_node->nodetype) {
        if (MODUL_OBJS (arg_node) != NULL) {
            MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), NULL);
        }
    }
    ABORT_ON_ERROR;

    if (top_fundef != NULL) {
        InitFunTable (top_fundef);

        /*   ABORT_ON_ERROR;  */

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
            top_fundef = CheckRest (top_fundef, SAC_PRG);
        } else {
            fun_tab_elem *fun_p;
            kind_of_file = SAC_MOD;

            if (tmp_node != NULL) {
                ABORT (NODE_LINE (tmp_node->node),
                       ("Function 'main' not allowed in module/"
                        "class implementations"));
            }

            for (fun_p = fun_table; !END_OF_FUN_TAB (fun_p);
                 fun_p = NEXT_FUN_TAB_ELEM (fun_p)) {
                DBUG_ASSERT (N_fundef == NODE_TYPE (fun_p->node),
                             ("wrong node in fun_table"));
                if ((FUNDEF_BODY (fun_p->node) != NULL)
                    && (FUNDEF_STATUS (fun_p->node) != ST_imported)
                    && (FUNDEF_STATUS (fun_p->node) != ST_foldfun)) {
                    /*
                     * When typechecking a module/class implementation, we cannot simply
                     * start with the main function. Aleternatively, each function that is
                     * actually implemented in this module and not imported from another
                     * one, is tagged PLEASE_CHECK and thereby acts as starting point for
                     * typechecking.
                     */
                    fun_p->tag = PLEASE_CHECK;

                    DBUG_PRINT ("CHECK", ("function %s has tag :%s", fun_p->id,
                                          CHECK_NAME (fun_p->tag)));
                }
            }

            top_fundef = CheckRest (top_fundef, SAC_MOD);
        }

        FREE (stack);
    } else {
        if (MODUL_FILETYPE (arg_node) == F_prog) {
            SYSABORT (("SAC program has no function 'main`"));
        }
    }

    if (N_modul == arg_node->nodetype)
        arg_node->node[2] = top_fundef;
    else
        arg_node = top_fundef;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CmpTypes
 *  arguments     : 1) first type
 *                  2) second type,
 *  description   : compares two type informations
 *  global vars   : ----
 *  internal funs : Type2String
 *  external funs : ----
 *  macros        : DBUG..., GEN_NODE, FREE, CMP_TYPE_ID, NULL
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
    db_str1 = Type2String (type_one, 0);
    db_str2 = Type2String (type_two, 0);
    DBUG_PRINT ("TYPE", ("compare '%s' with '%s'", db_str1, db_str2));
    FREE (db_str1);
    FREE (db_str2);
#endif

    if (TYPES_BASETYPE (type_one) == TYPES_BASETYPE (type_two)) {
        int ok = 1;

        if (T_hidden == TYPES_BASETYPE (type_one))
            ok = CMP_TYPE_ID (type_one, type_two);
        else if (T_user == TYPES_BASETYPE (type_one)) {
            if (!strcmp (type_one->name, type_two->name)) {
                if ((NULL != type_one->name_mod) && (NULL != type_two->name_mod))
                    ok = !(strcmp (type_one->name_mod, type_two->name_mod));
                else if ((NULL == type_one->name_mod) && (NULL == type_two->name_mod))
                    ok = 1;
                else {
                    char *mod_name = (NULL != type_one->name_mod) ? type_one->name_mod
                                                                  : type_two->name_mod;

                    node *t_node = LookupType (type_one->name, NULL, -064);
                    ok = !(
                      strcmp (mod_name, (NULL != t_node->ID_MOD) ? t_node->ID_MOD : ""));
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
                /* both have known shape
                 */
                int a1_n = 1, /* number of elements of array1 */
                  a2_n = 1;   /* number of elements of array2 */

                /* now check whether the arrays have the same number of
                 * elements
                 */
                for (i = 0; i < type_one->dim; i++)
                    a1_n *= type_one->shpseg->shp[i];
                for (i = 0; i < type_two->dim; i++)
                    a2_n *= type_two->shpseg->shp[i];
                if (a1_n == a2_n) {
                    if (!(type_one->dim == type_two->dim))
                        /* different dimension but equal
                         * number of  elements
                         */
                        return_value = CMP_different_dimensions;
                    else {
                        for (i = 0; i < type_one->dim; i++)
                            if (type_one->shpseg->shp[i] != type_two->shpseg->shp[i])
                                return_value = CMP_different_shapes; /* 4*/
                    }
                } else
                    /* wrong number of elements */
                    return_value = CMP_incompatible;
            } else if (((SCALAR == TYPES_DIM (type_one))
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
                     && (KNOWN_DIM_OFFSET > TYPES_DIM (type_two)))
                if ((KNOWN_DIM_OFFSET - TYPES_DIM (type_one) == TYPES_DIM (type_two)))
                    return_value = CMP_one_unknown_shape;
                else
                    return_value = CMP_incompatible;
            else if ((SCALAR < TYPES_DIM (type_two))
                     && (KNOWN_DIM_OFFSET > TYPES_DIM (type_one)))
                if ((KNOWN_DIM_OFFSET - TYPES_DIM (type_two) == TYPES_DIM (type_one)))
                    return_value = CMP_one_unknown_shape;
                else
                    return_value = CMP_incompatible;
            else if (((KNOWN_DIM_OFFSET > TYPES_DIM (type_one))
                      && (KNOWN_DIM_OFFSET > TYPES_DIM (type_two))))
                if (TYPES_DIM (type_one) == TYPES_DIM (type_two))
                    return_value = CMP_equal;
                else
                    return_value = CMP_different_dimensions;
            else if ((SCALAR == TYPES_DIM (type_one)) && (SCALAR == TYPES_DIM (type_two)))
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
 *  global vars   : filename
 *  internal funs : Type2String, Malloc, LookupType, DuplicateTypes, CmpTypes
 *  external funs :
 *  macros        : DBUG..., GEN_NODE, NULL, FREE, SHP_SEG_SIZE, TYPES,
 *                  MOD_NAME, ABORT
 *
 *  remarks       :
 */
cmp_types
CompatibleTypes (types *type_one, types *type_two, int convert_prim_type, int line)
{
    int old_dim1, old_dim2, i;
    cmp_types compare;
    node *t_node;
    types *type_1, *type_2;
    shpseg *shpseg_1 = NULL, *shpseg_2 = NULL;
#ifndef DBUG_OFF
    char *db_str1, *db_str2;

    DBUG_ENTER ("CompatibleTypes");

    db_str1 = Type2String (type_one, 0);
    db_str2 = Type2String (type_two, 0);

    DBUG_PRINT ("TYPE", ("compare %s with %s (convert_prim_type: %d", db_str1, db_str2,
                         convert_prim_type));
    FREE (db_str1);
    FREE (db_str2);
#endif

    if (T_user == TYPES_BASETYPE (type_one)) {
        t_node = LookupType (type_one->name, type_one->name_mod, line);
        if (NULL == t_node) {
            ABORT (line, ("Type '%s` is unknown",
                          ModName (type_one->name_mod, type_one->name)));
        } else {
            type_1 = DuplicateTypes (t_node->TYPES, 1);
            if (type_one->dim > 0) {
                if (type_1->dim >= 0) {
                    int dim;

                    old_dim1 = type_1->dim;
                    dim = old_dim1 + type_one->dim;
                    DBUG_ASSERT (dim <= SHP_SEG_SIZE, "shape out off range ");
                    shpseg_1 = type_1->shpseg;
                    type_1->shpseg = (shpseg *)Malloc (sizeof (shpseg));
                    type_1->shpseg->next = NULL;
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
        t_node = LookupType (type_two->name, type_two->name_mod, line);
        if (NULL == t_node) {
            ABORT (line, ("Type '%s` is unknown",
                          ModName (type_two->name_mod, type_two->name)));
        } else {
            type_2 = DuplicateTypes (t_node->TYPES, 1);
            if (type_two->dim > 0) {
                if (type_2->dim >= 0) {
                    int i, dim;

                    old_dim2 = type_2->dim;
                    dim = type_2->dim + type_two->dim;

                    DBUG_ASSERT (dim <= SHP_SEG_SIZE, "shape out off range ");
                    shpseg_2 = type_2->shpseg;
                    type_2->shpseg = (shpseg *)Malloc (sizeof (shpseg));
                    type_2->shpseg->next = NULL;
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
    } else {

        if ((((T_float == TYPES_BASETYPE (type_1)) && (T_int == TYPES_BASETYPE (type_2)))
             || ((T_double == TYPES_BASETYPE (type_1))
                 && (T_int == TYPES_BASETYPE (type_2)))
             || ((T_double == TYPES_BASETYPE (type_1))
                 && (T_float == TYPES_BASETYPE (type_2))))
            && (-1 == convert_prim_type)) {
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
         * int[?] as return type of psi in the module Array from the SAC
         * standard library.
         */
        else if ((ARRAY_OR_SCALAR == type_1->dim) && (ARRAY_OR_SCALAR == type_2->dim)
                 && (TYPES_BASETYPE (type_1) == TYPES_BASETYPE (type_2))) {
            compare = CMP_both_unknown_shape;
        } else if ((ARRAY_OR_SCALAR == type_1->dim) && (SCALAR <= type_2->dim)
                   && (TYPES_BASETYPE (type_1) == TYPES_BASETYPE (type_2))) {
            compare = CMP_one_unknown_shape;
        } else if ((ARRAY_OR_SCALAR == type_2->dim) && (SCALAR <= type_1->dim)
                   && (TYPES_BASETYPE (type_1) == TYPES_BASETYPE (type_2))) {
            compare = CMP_one_unknown_shape;
        } else {
            compare = CmpTypes (type_1, type_2);
        }
    }

    if (NULL != shpseg_1) {
        FREE (type_1->shpseg);
        type_1->shpseg = shpseg_1;
        type_1->dim = old_dim1;
    }
    if (NULL != shpseg_2) {
        FREE (type_2->shpseg);
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
 *  macros        : DBUG..., NULL, FREE, MOD_NAME, DIM, ABORT
 *
 *  remarks       : If one type is not a primitive type and one is a primitive
 *                  type, it has to be checked whether one type can be updated.
 *                  The function CompatileTypes checks it.
 *                  !!!! Call this function only after calling CmpTypes or
 *                       CompatibleTypes. !!!!
 *
 */
void
UpdateType (types *type_one, types *type_two, int line)
{
    int i;
    node *t_node;
    types *t_known = NULL, *t_unknown = NULL;

#ifndef DBUG_OFF
    char *db_str1, *db_str2;
#endif

    DBUG_ENTER ("UpdateType");

    if ((-1 >= type_one->dim) && (0 <= type_two->dim)) {
        t_unknown = type_one;
        t_known = type_two;
    } else if ((-1 >= type_two->dim) && (0 <= type_one->dim)) {
        t_unknown = type_two;
        t_known = type_one;
    }
    DBUG_ASSERT (((NULL != t_unknown) && (NULL != t_known)), "wrong types");

#ifndef DBUG_OFF
    db_str1 = Type2String (t_unknown, 0);
    db_str2 = Type2String (t_known, 0);
    DBUG_PRINT ("TYPE", ("old types :%s , %s ", db_str1, db_str2));
    FREE (db_str1);
    FREE (db_str2);
#endif

    if ((T_user != TYPES_BASETYPE (t_unknown)) && (T_user == TYPES_BASETYPE (t_known))) {
        t_node = LookupType (t_known->name, t_known->name_mod, line);
        if (NULL == t_node)
            ABORT (line, ("Type '%s` is unknown",
                          ModName (t_unknown->name_mod, t_unknown->name)));
        DBUG_ASSERT ((NULL != t_node), "t_node is NULL");
        t_unknown->dim = t_node->DIM + t_known->dim;
        DBUG_ASSERT ((t_unknown->dim <= SHP_SEG_SIZE), "dimension to large");

        if (t_unknown->dim > 0) {
            t_unknown->shpseg = (shpseg *)Malloc (sizeof (shpseg));
            t_unknown->shpseg->next = NULL;
            for (i = 0; i < t_known->dim; i++)
                t_unknown->shpseg->shp[i] = t_known->shpseg->shp[i];
            for (i = t_known->dim; i < t_unknown->dim; i++)
                t_unknown->shpseg->shp[i] = t_node->SHP[i - t_known->dim];
        }
    } else if ((T_user == TYPES_BASETYPE (t_unknown))
               && (T_user != TYPES_BASETYPE (t_known))) {
        t_node = LookupType (t_unknown->name, t_unknown->name_mod, line);
        if (NULL == t_node)
            ABORT (line,
                   ("Type '%s` is unknown", ModName (t_known->name_mod, t_known->name)));
        DBUG_ASSERT ((NULL != t_node), "t_node is NULL");
        t_unknown->dim = t_known->dim - t_node->DIM;
        if (t_unknown->dim > 0) {
            t_unknown->shpseg = (shpseg *)Malloc (sizeof (shpseg));
            t_unknown->shpseg->next = NULL;
            for (i = 0; i < t_unknown->dim; i++)
                t_unknown->shpseg->shp[i] = t_known->shpseg->shp[i];
        }
    } else {

        /* both types are arrays of compatible primitive type
         * in this case we have to copy the shape of t_known to t_unknown
         */
        DBUG_ASSERT (((TYPES_BASETYPE (t_unknown) == TYPES_BASETYPE (t_known))
                      || ((T_int == TYPES_BASETYPE (t_unknown))
                          && (T_float == TYPES_BASETYPE (t_known)))
                      || ((T_float == TYPES_BASETYPE (t_unknown))
                          && (T_int == TYPES_BASETYPE (t_known)))),
                     "wrong simpletypes ");

        t_unknown->dim = t_known->dim;
        t_unknown->shpseg = (shpseg *)Malloc (sizeof (shpseg));
        t_unknown->shpseg->next = NULL;

        for (i = 0; i < t_known->dim; i++)
            t_unknown->shpseg->shp[i] = t_known->shpseg->shp[i];
    }

#ifndef DBUG_OFF
    db_str1 = Type2String (t_unknown, 0);
    db_str2 = Type2String (t_known, 0);
    DBUG_PRINT ("TYPE", ("new types :%s , %s ", db_str1, db_str2));
    FREE (db_str1);
    FREE (db_str2);
#endif

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : DuplicateFun
 *  arguments     : 1) pointer to fun-table-entrie
 *  description   : duplicates a function(fun_p->node)(insert it into
 *                  fun-table and syntax-tree) if fun_p->n_dup >0
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., NULL, INSERT_FUN, NOT_CHECKED, ID, ID_MOD
 *
 *  remarks       :
 *
 */
fun_tab_elem *
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
        new_fun_node = DupTree (fun_p->node, NULL);
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

        /* insert copied function into fun_table */
        OLD_INSERT_FUN (fun_p, new_fun_node->ID, new_fun_node->ID_MOD, new_fun_node,
                        NOT_CHECKED, -2);
        new_fun_p = NEXT_FUN_TAB_ELEM (fun_p);
    } else
        new_fun_p = fun_p;

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
 *  macros        : DBUG..., NULL, P_FORMAT, NOTE, ID, TYPES, ERROR, ABORT,
 *                  WARN,  MOD_NAME
 *
 *  remarks       : if no function is found that matches the types of the
 *                  arguments then it will be looked for a function that has
 *                  compatible types with the arguments ( check whether
 *                  parameters and arguments have equal primitive type)
 *
 *                  - parameter-types of external functions will not be updated
 *                  - the meaning of variable 'found':
 *                     0: no function found
 *                     1: found matching userdefined function without formal
 *                        parameters of unknown shape
 *                     2: found matching userdefined function with at least
 *                        one formal paramter of unknown shape
 *                     3: found matching primitive function without formal
 *                        parameters of unknown shape
 *                     4: found matching primitive function with at least
 *                        one formal paramter of unknown shape
 *                  - the meaning of variable 'equal_types':
 *                     0: current function declaration doesn't match with
 *                        given argument-types(3)
 *                     1: current function declaration matches with given
 *                        argument_types( there isn't any unknown shape in
 *                        declaration of current function)
 *                     2: current function declaration matches given argument-
 *                        types, but there is at least one formal parameter of
 *                        unknown shape
 *
 */
void *
FindFun (char *fun_name, char *mod_name, types **arg_type, int count_args, node *arg_info,
         int line, int *prf_fun)
{
    int i, equal_types, count_param, stored_prf_fun, found = 0, once_again = 0, v_args;
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
        /* look where 'fun_name' is defined */
        mods = FindSymbolInModul (mod_name, fun_name, 2, NULL, 1);
    }

    /* now look for fitting function */
    if (IS_PRIMFUN) {
        prf_p = LookupPrf (*prf_fun, NULL);
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
                     : ((NULL != prf_p) ? *prf_fun == prf_p->prf : 0)) {
                DBUG_PRINT ("TYPE",
                            ("current fun '%s'" P_FORMAT " found:%d",
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
                        /* increase count_param only, if the type of the formal
                           parameter is not equal T_dots.
                           T_dots should be the last parameter of a function,  so we
                           only have to compare the types of the first parameter and
                           arguments.
                        */
                        if (T_dots == ARG_BASETYPE (fun_args))
                            v_args = 1;
                        else
                            count_param++;

                        DBUG_PRINT ("TYPE", ("%d. : %s" P_FORMAT, count_param,
                                             Type2String (ARG_TYPE (fun_args), 0),
                                             ARG_TYPE (fun_args)));
                        fun_args = ARG_NEXT (fun_args);
                    }

                    if ((count_args == count_param) || (1 == v_args)) {
                        if (0 != count_param) {
                            /* now compare the types */
                            fun_args = FUN_ARGS;
                            equal_types = 1;
                            for (i = 0; i < count_param;
                                 fun_args = ARG_NEXT (fun_args), i++) {
                                if (0 == once_again)
                                    is_correct
                                      = CmpTypes (arg_type[i], ARG_TYPE (fun_args));
                                else if (1 == once_again)
                                    is_correct = CompatibleTypes (ARG_TYPE (fun_args),
                                                                  arg_type[i], 0, line);
                                if (CMP_one_unknown_shape == is_correct) /* 2*/
                                    equal_types = 2;
                                else if ((CMP_both_unknown_shape == is_correct)
                                         && (SAC_MOD == kind_of_file))
                                    equal_types = 2;              /* neu am 8.12. */
                                else if (CMP_equal != is_correct) /* 1 */
                                {
                                    equal_types = 0;
                                    break;
                                }
                            }
                            switch (found) {
                            case 0:
                                if (1 == equal_types) {
                                    if (IS_PRIMFUN)
                                        found = 3;
                                    else
                                        found = 1;
                                    ret_node = FUN_P;
                                } else if (2 == equal_types) {
                                    if (IS_PRIMFUN)
                                        found = 4;
                                    else
                                        found = 2;
                                    ret_node = FUN_P;
                                }
                                break;
                            case 1:
                            case 3:
                                if (1 == equal_types) {
                                    /* there are more than one matching functions,
                                     * so an error message will be given.
                                     */
                                    ERROR (line,
                                           ("More than one "
                                            "function matches the call of '%s`",
                                            ModName (mod_name,
                                                     ((IS_PRIMFUN) ? prf_string[*prf_fun]
                                                                   : fun_name))));
                                }
                                break;

                            case 4:
                                if (0 < equal_types) {
                                    /* there are more than one matching functions,
                                     * so an error message will be given.
                                     */
                                    ERROR (line,
                                           ("More than one "
                                            "function matches the call of '%s`",
                                            ModName (mod_name,
                                                     ((IS_PRIMFUN) ? prf_string[*prf_fun]
                                                                   : fun_name))));
                                }
                                break;
                            case 2:
                                /* if((1==equal_types) && IS_DUPLICATED(fun_p)) */
                                if (1 == equal_types) {
                                    found = 1;
                                    ret_node = FUN_P;
                                } else
                                  /* if((0<equal_types) && (!IS_DUPLICATED(fun_p))) */
                                  if (1 < equal_types) {
                                    /* there are more than one matching functions,
                                     * so an error message will be given.
                                     */
                                    ERROR (line,
                                           ("More than one function "
                                            "matches the call of '%s`",
                                            ModName (mod_name,
                                                     ((IS_PRIMFUN) ? prf_string[*prf_fun]
                                                                   : fun_name))));
                                }
                                break;
                            default:
                                DBUG_ASSERT (0, "'found' has wrong value");
                            }
                        } else {
                            if (IS_PRIMFUN)
                                found = 3;
                            else
                                found = 1;
                            ret_node = FUN_P;
                        }
                    }
                }
                /* goto next function */
                if (-1 == *prf_fun)
                    fun_p = NEXT_FUN_TAB_ELEM (fun_p);
                else
                    prf_p = prf_p->next;
            }
            if (NULL == ret_node) {
                if (-1 != stored_prf_fun) {
                    /* we are looking for a primitive function */
                    if (IS_PRIMFUN) {
                        if (0 == once_again) {
                            /* now look if there is a matching userdefined funktion
                             * with name of primitive function
                             */
                            fun_name = prf_name_str[*prf_fun];
                            fun_p_store = LookupFun (fun_name, NULL, NULL);
                            if (NULL != fun_p_store)
                                *prf_fun = -1;
                            else
                                /* there isn't any matching userdefined function,
                                 * so look if there is a primitive function with
                                 * 'compatible types'
                                 */
                                once_again += 1;
                        } else if (1 == once_again)
                            if (NULL != fun_p_store)
                                *prf_fun = -1;
                            else
                                once_again += 1;
                        else
                            once_again += 1;
                    } else {
                        if (0 == once_again)
                            *prf_fun = stored_prf_fun;
                        once_again += 1;
                    }
                } else
                    once_again += 1;
            }
        } while ((once_again < 2) && (NULL == ret_node));
        ABORT_ON_ERROR;
        if (NULL == ret_node) {
            char arg_str[MAX_ARG_TYPE_LENGTH];

            arg_str[0] = '\0';
            for (i = 0; i < count_args; i++) {
                strcat (arg_str, Type2String (arg_type[i], 0));
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
                 */
                fun_p = ret_node;
                fun_args = FUN_ARGS;
                for (i = 0; i < count_args; fun_args = ARG_NEXT (fun_args), i++)
                    if ((-1 == ARG_DIM (fun_args)) && (0 < TYPES_DIM (arg_type[i]))) {
                        update = 1;
                        break;
                    } else if ((SAC_PRG == kind_of_file)
                               && (-1 == TYPES_DIM (arg_type[i]))
                               && (-1 == ARG_DIM (fun_args))) {
                        ABORT (line,
                               ("Argument %d of function '%s' has"
                                "unknown shape (%s)",
                                i + 1,
                                ModName (mod_name,
                                         (IS_PRIMFUN) ? prf_string[*prf_fun] : fun_name),
                                Type2String (arg_type[i], 0)));
                    }

                if (1 == update) {
                    fun_p = DuplicateFun (ret_node);
                    fun_args = FUN_ARGS;
                    for (i = 0; i < count_args; fun_args = ARG_NEXT (fun_args), i++)
                        if (-1 == ARG_DIM (fun_args)) {
                            /* update types only if arg_type has known shape */
                            if (-1 != TYPES_DIM (arg_type[i])) {
                                UpdateType (arg_type[i], ARG_TYPE (fun_args),
                                            NODE_LINE (fun_p->node));
#ifdef SHAPE_NOTE
                                NOTE (("%s:%d: NOTE: parameter '%s` "
                                       "has now defined shape (%s)",
                                       filename, NODE_LINE (fun_p->node),
                                       ARG_NAME (fun_args),
                                       Type2String (ARG_TYPE (fun_args), 0)));
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
 *  global vars   : filename
 *  internal funs : CompatibleTypes, UpdateType,DuplicateTypes
 *  external funs :Type2String, MakeNode, strcmp
 *  macros        : DBUG..., PUSH_VAR, ERROR,  NOTE, NULL
 *
 *  remarks       :
 *
 */
void
AddIdToStack (ids *ids, types *type, node *arg_info, int line)
{
    node *vardec_p;
    int is_defined = 0;
    char *str;

    DBUG_ENTER ("AddIdToStack");

#ifdef KNOWN_SHAPE_ONLY
    DBUG_ASSERT (-1 != type->dim, "type of unknown shape ");
#endif

    vardec_p = arg_info->node[0]->node[0]; /* pointer to var declaration */
    if (vardec_p) {
        /* looking, if ids->id is defined in variable declaration */
        while ((VARDEC_NEXT (vardec_p)) && !is_defined)
            if (!(strcmp (ids->id, vardec_p->info.types->id)))
                is_defined = 1;
            else
                vardec_p = VARDEC_NEXT (vardec_p);

        if (is_defined || (!strcmp (ids->id, vardec_p->info.types->id))) {
            /* variable ids->id is defined in variable declaration part
             * so check if typedeclaration is ok */
            is_defined = 1;
            /*
             * sbs: calling CompatibleTypes with 0 as 3rd arg makes int, float,
             * and double incompatible!!
             */
            switch (CompatibleTypes (vardec_p->info.types, type, 0, line)) {
            case CMP_equal: /* 1*/
                break;
            case CMP_incompatible: /* 0 */ {
                ABORT (line, ("Incompatible types in declaration (%s) "
                              "and  usage (%s) of variable '%s`",
                              Type2String (vardec_p->info.types, 0),
                              Type2String (type, 0), ids->id));
                break;
            }
            case CMP_one_unknown_shape: /* 2 */ {
                UpdateType (vardec_p->info.types, type, line);
                break;
            }
            case CMP_different_shapes:
            case CMP_different_dimensions: /* 3, 4 */ {
                str = Type2String (type, 0);
                WARN (line, ("Types in declaration (%s) and usage "
                             "(%s) of variable '%s` are different",
                             Type2String (vardec_p->info.types, 0), str, ids->id));
                FREE (str);
                break;
            }
            case CMP_both_unknown_shape: {
                if (kind_of_file != SAC_MOD) {
                    str = Type2String (type, 0);
                    WARN (line, ("Types in declaration (%s) and usage "
                                 "(%s) of variable '%s` have unknown shape",
                                 Type2String (vardec_p->info.types, 0), str, ids->id));
                    FREE (str);
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

        DBUG_PRINT ("REF", ("added reference" P_FORMAT " from %s to %s", ids->node,
                            ids->id, ids->node->info.types->id));

        PUSH_VAR (ids->id, vardec_p);
    } else {
        /* first use of variable ids->id
         * so one must create a new N_vardec node and put it in front of
         * the variable declaration of the current function
         */

        node *vardec;
        types *id_type;

        vardec_p = arg_info->node[0]->node[0]; /* pointer to var declaration */
        id_type = DuplicateTypes (type, 1);
        vardec = MakeNode (N_vardec);
        VARDEC_TYPE (vardec) = id_type;
        VARDEC_NAME (vardec) = StringCopy (ids->id);
        VARDEC_STATUS (vardec) = ST_used;

        if (vardec_p)
            VARDEC_NEXT (vardec) = vardec_p; /* append old variable declaration */
        /* store pointer variable declaration part */
        arg_info->node[0]->node[0] = vardec;
        DBUG_PRINT ("TYPE", ("add %s to variable declaration ", ids->id));

        /* insert reference to variable declaration */
        ids->node = vardec;
        DBUG_PRINT ("REF", ("added reference" P_FORMAT " from %s to %s", ids->node,
                            ids->id, ids->node->info.types->id));

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
 *  macros        : DBUG..., PUSH_VAR, FREE, NOT_CHECKED, NULL, CHECK_NAME
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

    DBUG_PRINT ("TYPE", ("checking function %s", arg_node->info.types->id));

    act_frame = tos; /* set new frame pointer */

    /* check whether there is a userdefined type with same name as function */
    CHECK_FUNCTION_TYPENAMECLASH (arg_node);

    /* add formal parameters to local stack */
    fun_params = arg_node->node[2];
    while (NULL != fun_params) {
        tmp_node = fun_params->node[0];
        j = i + 1;
        while (NULL != tmp_node) {
            if (0 == strcmp (tmp_node->ID, fun_params->ID)) {
                ERROR (NODE_LINE (arg_node),
                       ("%d. and %d. argument of function '%s' have same"
                        " name '%s`",
                        i, j, ModName (arg_node->ID_MOD, arg_node->ID), fun_params->ID));
                ok = 0;
            }
            j += 1;
            tmp_node = tmp_node->node[0];
        }

        CHECK_TYPENAMECLASH (fun_params);
        if (1 == ok) {
            PUSH_VAR (fun_params->info.types->id, fun_params);
        } else
            ok = 1;

        fun_params = fun_params->node[0];
        i += 1;
    }
    ABORT_ON_ERROR;

    /* set global variable 'imported_fun' */
    imported_fun = (ST_imported == FUNDEF_STATUS (arg_node)) ? IMPORTED : NOT_IMPORTED;

    info_node = MakeNode (N_info);
    info_node->node[0] = MakeNode (N_ok); /* status 0 = beginning of function */
    info_node->node[0]->info.cint = 0;

    /* pointer to variable declaration */
    info_node->node[0]->node[0] = BLOCK_VARDEC (FUNDEF_BODY (arg_node));
    info_node->node[0]->node[1] = arg_node; /* function declaration */
    info_node->info.id = arg_node->info.types->id;

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

        vardec = arg_node->node[0]->node[1];

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

                    VARDEC_TYPEDEF (vardec) = VARDEC_TDEF (vardec);
                    /*
                     * Unfortunatly, there are two links from a variable declaration
                     * to the respective type definition, one in the vardec node itself
                     * and one in the types data structure accessible by a compound
                     * macro. The former one is the way it should be since the definition
                     * obviously is an attribute of the type and not that of a variable
                     * declaration. The latter one is only maintained for compatibility
                     * concerns.
                     */

                    VARDEC_ATTRIB (vardec) = TYPEDEF_ATTRIB (VARDEC_TYPEDEF (vardec));
                }
            }

            vardec = vardec->node[0];
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
    FREE (info_node->node[0]);
    FREE (info_node);

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

void
CheckIfGOonlyCBR (node *arg, node *exprs)
{
    node *expr;

    DBUG_ENTER ("CheckIfGOonlyCBR");

    while ((arg != NULL) && (ARG_BASETYPE (arg) != T_dots)) {
        expr = EXPRS_EXPR (exprs);

        if (NODE_TYPE (expr) == N_id) {
            if (ID_ATTRIB (expr) == ST_global) {
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
 *  functionname  : TI
 *  arguments     : 1) argument node
 *  description   : returns the type of the expression behind 1) if one
 *                  can derive it
 *                  or NULL if one can't infer the type
 *  global vars   :
 *  internal funs : TI_prf, LookupVar, DuplicateTypes, TI_ap, TI_with, TI_cast
 *                  LookupObject
 *  external funs : Error, sprintf
 *  macros        : DBUG...,GEN_TYPE_NODE, NULL
 *
 *  remarks       : the result pointer has to be freed, when one does not
 *                  use it any more
 *
 */
types *
TI (node *arg_node, node *arg_info)
{
    types *return_type, *tmp_types;
    int cnt;

    DBUG_ENTER ("TI");

    switch (NODE_TYPE (arg_node)) {
    case N_prf:
        return_type = TI_prf (arg_node, arg_info);
        break;

    case N_ap:
        return_type = TI_ap (arg_node, arg_info);

        if (return_type->id != NULL) {
            FREE (return_type->id);
        }
        if (return_type->id_mod != NULL) {
            return_type->id_mod = NULL;
        }
        /*
         * Unfortunately, a function's name and module are stored in the first
         * return types structure (in the actual syntax tree, not in the virtual
         * of course). So, these have been copied by DuplicateTypes() and should
         * now be removed as they are just meaningless in this context.
         */

        cnt = 0;
        tmp_types = return_type;
        while (tmp_types != NULL) {
            if (TYPES_DIM (tmp_types) < 0)
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

    case N_with:
        return_type = TI_with (arg_node, arg_info);
        break;

    case N_Nwith:
        return_type = TI_Nwith (arg_node, arg_info);
        break;

    case N_post:
    case N_pre: {
        stack_elem *stack_p;

        if (NULL != (stack_p = LookupVar (arg_node->info.ids->id)))
            return_type = DuplicateTypes (stack_p->node->info.types, 1);
        else
            return_type = NULL;
        break;
    }
    case N_id: {
        node *odef;
        stack_elem *stack_p;

        stack_p = NULL;

        if (ID_ATTRIB (arg_node) == ST_global) {
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

            return_type = DuplicateTypes (OBJDEF_TYPE (odef), 1);
        }

        else

        {
            /*
             *  The identifier may be a local variable or a global object.
             */

            stack_p = LookupVar (arg_node->info.ids->id);

            if (stack_p) {
                return_type = DuplicateTypes (stack_p->node->info.types, 1);
                ID_VARDEC (arg_node) = stack_p->node;

                DBUG_PRINT ("REF", ("added reference" P_FORMAT " for %s to %s",
                                    arg_node->info.ids->id, arg_node->info.ids->id,
                                    arg_node->info.ids->node->info.types->id));
            } else {
                /*
                 *  If the identifier is not yet locally bound, it may be a
                 *  global object.
                 */

                odef = LookupObject (ID_NAME (arg_node), NULL, NODE_LINE (arg_node));

                if (odef != NULL) {
                    arg_node->info.ids->mod = OBJDEF_MOD (odef);
                    ID_ATTRIB (arg_node) = ST_global;
                    ID_OBJDEF (arg_node) = odef;

                    return_type = DuplicateTypes (OBJDEF_TYPE (odef), 1);
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
        ABORT (0, ("Type inference for %s isn't implemented yet",
                   mdb_nodetype[arg_node->nodetype]));
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
 *  macros        : DBUG..., PUSH_VAR, GEN_NODE, ABORT, ERROR, P_FORMAT
 *                  FREE
 *  remarks       : the reference to the variable declaration is put in
 *                  arg_node->info.ids->node
 *
 */
node *
TClet (node *arg_node, node *arg_info)
{
    ids *ids;
    types *type, *tmp, *next_type;
    stack_elem *elem;
#ifndef DBUG_OFF
    char *db_str; /* only used for debugging */
#endif

    DBUG_ENTER ("TClet");

    ids = LET_IDS (arg_node); /* left side of let-assign */
    if (ids)
        CheckIds (ids, arg_info->node[0]->node[0], NODE_LINE (arg_node));
    /*
     * look for double identifiers and name clashes with global objects
     */
    INFO_TC_LHSVARS (arg_info) = ids;

    type = TI (LET_EXPR (arg_node), arg_info); /* type of right side of let-assign */

    if (type == NULL)
        ABORT (NODE_LINE (arg_node), ("Type of right hand side of '=` is not inferable"));

    DBUG_PRINT ("STOP",
                ("arg_info->node[0]: %s", mdb_nodetype[arg_info->node[0]->nodetype]));

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_prf) {
        switch (PRF_PRF (LET_EXPR (arg_node))) {
        case F_psi:
            if (TYPES_DIM (type) == ARRAY_OR_SCALAR) {
                /*
                 * LET-expression is a call of function psi,
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

                vardec_node = LookupVardec (ids->id, arg_info->node[0]->node[0]);

                if (vardec_node) {
                    cmp_types cmp;
                    int tmp_dim = TYPES_DIM (type);

                    TYPES_DIM (type) = TYPES_DIM (VARDEC_TYPE (vardec_node));

                    if (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)))
                        TYPES_SHPSEG (type)
                          = CopyShpseg (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)));

                    /*
                     * sbs: switched the prim_fun_flag from -1 to 0 since we
                     * otherwise get an error in UpdateType for the following example:
                     *
                     *
                     *
                     */
                    cmp = CompatibleTypes (VARDEC_TYPE (vardec_node), type, 0,
                                           NODE_LINE (arg_node));

                    if (!((cmp == CMP_equal) || (cmp == CMP_both_unknown_shape)
                          || (cmp == CMP_one_unknown_shape))) {
                        TYPES_DIM (type) = tmp_dim;
                        ABORT (NODE_LINE (arg_node),
                               ("Incompatible types in declaration (%s) "
                                "and usage (%s) of variable '%s`",
                                Type2String (VARDEC_TYPE (vardec_node), 0),
                                Type2String (type, 0), ids->id));
                    }
                } else {
                    if (kind_of_file != SAC_MOD) {
                        ABORT (NODE_LINE (arg_node),
                               ("Function 'psi' is called with argument of type '%s'"
                                ", so the type of the result has to be declared",
                                Type2String (type, 0), ids->id));
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
                 * Here, we have a psi returning an array.
                 */
                if (!(intrinsics & INTRINSIC_PSI)) {
                    /*
                     * If not the intrinsic implementation of primitive function psi() is
                     * to be used, the function application is now replaced by an
                     * equialent with-loop.
                     */
                    if (NULL != LookupFun (prf_name_str[F_psi], "Array", NULL)) {
                        if (TYPES_DIM (type) > SCALAR) {
                            /*
                             * The actual replacement is only performed if the result
                             * shape has been determined.
                             */
                            node *wl_impl;
                            types *new_type;

                            wl_impl
                              = BuildPsiWithLoop (type, PRF_ARG1 (LET_EXPR (arg_node)),
                                                  PRF_ARG2 (LET_EXPR (arg_node)));
                            new_type = TI (wl_impl, arg_info);
                            FreeAllTypes (new_type);

                            /*
                             * PRF_ARG1(LET_EXPR(arg_node)) = NULL;
                             *
                             * The index argument node has not been reused by
                             * BuildPsiWithLoop(), so, it has to be freed now.
                             */

                            PRF_ARG2 (LET_EXPR (arg_node)) = NULL;
                            FreeTree (LET_EXPR (arg_node));

                            LET_EXPR (arg_node) = wl_impl;
                        }
                    } else {
                        ERROR (NODE_LINE (arg_node),
                               ("Function 'psi` undefined or applied to wrong arguments: "
                                "'%s, %s`",
                                Type2String (ID_TYPE (PRF_ARG1 (LET_EXPR (arg_node))), 0),
                                Type2String (ID_TYPE (PRF_ARG2 (LET_EXPR (arg_node))),
                                             0)));
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
                new_type = TI (wl_impl, arg_info);
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
                if (NULL != LookupFun (prf_name_str[F_take], "Array", NULL)) {
                    if (TYPES_DIM (type) > SCALAR) {
                        /*
                         * The actual replacement is only performed if the result shape
                         * has been determined.
                         */
                        node *wl_impl;
                        types *new_type;

                        wl_impl = BuildTakeWithLoop (PRF_ARG1 (LET_EXPR (arg_node)),
                                                     PRF_ARG2 (LET_EXPR (arg_node)));
                        new_type = TI (wl_impl, arg_info);
                        FreeAllTypes (new_type);

                        PRF_ARG1 (LET_EXPR (arg_node)) = NULL;
                        PRF_ARG2 (LET_EXPR (arg_node)) = NULL;
                        FreeTree (LET_EXPR (arg_node));

                        LET_EXPR (arg_node) = wl_impl;
                    }
                } else {
                    ERROR (NODE_LINE (arg_node),
                           ("Function 'take` undefined or applied to wrong arguments: "
                            "'%s, %s`",
                            Type2String ((NODE_TYPE (PRF_ARG1 (LET_EXPR (arg_node)))
                                          == N_array)
                                           ? ARRAY_TYPE (PRF_ARG1 (LET_EXPR (arg_node)))
                                           : ID_TYPE (PRF_ARG1 (LET_EXPR (arg_node))),
                                         0),
                            Type2String ((NODE_TYPE (PRF_ARG2 (LET_EXPR (arg_node)))
                                          == N_array)
                                           ? ARRAY_TYPE (PRF_ARG2 (LET_EXPR (arg_node)))
                                           : ID_TYPE (PRF_ARG2 (LET_EXPR (arg_node))),
                                         0)));
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
                if (NULL != LookupFun (prf_name_str[F_drop], "Array", NULL)) {
                    if (TYPES_DIM (type) > SCALAR) {
                        /*
                         * The actual replacement is only performed if the result shape
                         * has been determined.
                         */
                        node *wl_impl;
                        types *new_type;

                        wl_impl = BuildDropWithLoop (type, PRF_ARG1 (LET_EXPR (arg_node)),
                                                     PRF_ARG2 (LET_EXPR (arg_node)));
                        new_type = TI (wl_impl, arg_info);
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
                                         0),
                            Type2String ((NODE_TYPE (PRF_ARG2 (LET_EXPR (arg_node)))
                                          == N_array)
                                           ? ARRAY_TYPE (PRF_ARG2 (LET_EXPR (arg_node)))
                                           : ID_TYPE (PRF_ARG2 (LET_EXPR (arg_node))),
                                         0)));
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
                if (NULL != LookupFun (prf_name_str[F_cat], "Array", NULL)) {
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
                        new_type = TI (wl_impl_1, arg_info);
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
                                         0),
                            Type2String ((NODE_TYPE (PRF_ARG3 (LET_EXPR (arg_node)))
                                          == N_array)
                                           ? ARRAY_TYPE (PRF_ARG3 (LET_EXPR (arg_node)))
                                           : ID_TYPE (PRF_ARG3 (LET_EXPR (arg_node))),
                                         0)));
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
        db_str = Type2String (type, 0); /* only used while debuging */
        DBUG_PRINT ("TYPE", ("infered type : %s", db_str));
        FREE (db_str);
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
                 *
                 *
                 *
                 */
                switch (CompatibleTypes (elem->node->info.types, type, 0,
                                         NODE_LINE (arg_node))) {
                case CMP_incompatible: /* 0 */
                    ABORT (NODE_LINE (arg_node),
                           ("Incompatible types in declaration (%s) "
                            "and usage (%s) of variable '%s`",
                            Type2String (elem->node->info.types, 0),
                            Type2String (type, 0), ids->id));
                    break;
                case CMP_one_unknown_shape: /* 2*/
#ifdef KNOWN_SHAPE_ONLY
                    DBUG_ASSERT (0, "CompatibleTypes returns CMP_one_unknown_shape");
#else
                    DBUG_ASSERT (SAC_PRG != kind_of_file, "CompatibleTypes returns"
                                                          " CMP_one_unknown_shape");
#endif /* KNOWN_SHAPE_ONLY */
                    break;
                case CMP_different_shapes:
                case CMP_different_dimensions: /* 3, 4 */
                    WARN (NODE_LINE (arg_node),
                          ("Different types in declaration (%s) and usage (%s) "
                           "of variable '%s`",
                           Type2String (elem->node->info.types, 0), Type2String (type, 0),
                           ids->id));

                    /* no break missing !!!! */
                case CMP_equal: /* 1 */
                    ids->node = elem->node;
                    DBUG_PRINT ("REF", ("added reference" P_FORMAT " from %s to %s",
                                        ids->node, ids->id, ids->node->info.types->id));
                    break;
                case CMP_both_unknown_shape: /* neu 8.12. */
                    if (kind_of_file != SAC_MOD) {
                        char *str1 = Type2String (elem->node->info.types, 0);
                        char *str2 = Type2String (type, 0);
                        WARN (NODE_LINE (arg_node),
                              ("Types in declaration (%s) and usage "
                               "(%s) of variable '%s' have unknown shape",
                               str1, str2, ids->id));
                        FREE (str1);
                        FREE (str2);
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
                      = LookupVardec (IDS_NAME (ids), arg_info->node[0]->node[0]);
                    if (declaration == NULL) {
                        declaration = LookupArg (IDS_NAME (ids),
                                                 arg_info->node[0]->node[1]->node[2]);
                        if (declaration == NULL)
                            ABORT (NODE_LINE (arg_node),
                                   ("Unable to infer type of variable %s",
                                    IDS_NAME (ids)));
                    }
                    declared_type = DuplicateTypes (VARDEC_TYPE (declaration), 1);
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

        if (type && (TYPES_BASETYPE (type) == T_dots))
            type = NULL;

        if (type) {
            char *str1, *str2, *str3;
            int str3_len, i;

            str1 = Type2String (type, 0);
            str2 = Type2String (tmp, 0); /* infered type */
            str3_len
              = strlen (str2) - strlen (str1) - 2; /* -2 to delete comma and space*/
            /* in str3 the type of the left side of a let-statement will be
             * stored
             */
            str3 = (char *)Malloc (sizeof (char) * (str3_len + 1));
            for (i = 0; i < str3_len; i++)
                str3[i] = str2[i];
            str3[str3_len] = '\0';

            ABORT (NODE_LINE (arg_node), ("Different types (%s != %s)", str3, str2));
        } else if (ids) {
            char *str1;

            str1 = Type2String (tmp, 0);
            ABORT (NODE_LINE (arg_node), ("Different types (%s,... != %s)", str1, str1));
        }
        DBUG_ASSERT ((!type && !ids), "type or ids are not NULL");

        /* and now free infered types */
        /*      for(type=tmp; type != NULL; type=type->next)
         FREE_TYPES(type);
         */
        FreeAllTypes (tmp);

#ifdef TEST_GET_SHAPE
        /* to test the macro GET_SHAPE */
        if (N_prf == arg_node->node[0]->nodetype)
            if (F_shape == arg_node->node[0]->info.prf) {
                node *arg = arg_node->node[0]->node[0]->node[0];
                if (arg->nodetype == N_array)
                    SHAPE_2_ARRAY (arg_node->node[0], arg->TYPES);
                else {
                    DBUG_ASSERT (arg->nodetype == N_id, "N_id expected");
                    SHAPE_2_ARRAY (LET_EXPR (arg_node), arg->info.ids->node->TYPES);
                }
            }
#endif
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
 *  macros        : DBUG...,, ABORT, NULL, FREE
 *
 *  remarks       :- result must be freed if one does not use it any more
 *                 - if arg_node contains to a userdefined function( overloading
 *                   of primitive function) then an arg_node will be converted to
 *                   an N_ap-node
 *
 */
types *
TI_prf (node *arg_node, node *arg_info)
{
    prim_fun_tab_elem *prf_p;
    types *ret_type, **arg_type;
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
        arg_type[i] = TI (EXPRS_EXPR (current_args), arg_info);
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

    if (prf_tag == PRF_PRF (arg_node)) {
        prf_p = (prim_fun_tab_elem *)fun_p;
        type_c_tag = prf_p->node->info.prf_dec.tag;
        PRF_PRF (arg_node) = prf_p->new_prf;

        switch (type_c_tag) {
#include "prim_fun_tt.mac"
        default: {
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
    FREE (arg_type);

#ifndef DBUG_OFF
    db_str = Type2String (ret_type, 0);
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
 *  macros        : DBUG..., FREE, ERROR, ABORT, FAST_CHECK, NULL,  MOD_NAME
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

    exprs = arg_node->node[0];

#ifdef NOT_EMPTY_RETURN
    DBUG_ASSERT (NULL != exprs, (" return statement without return-value"));
#endif /* NOT_EMPTY_RETURN */
    if (NULL != exprs) {
        /* infere the types of the returnvalue */
        return_type = TI (exprs->node[0], arg_info);
        if (NULL == return_type)
            ABORT (NODE_LINE (arg_node), ("Return_type cannot be infered"));

        DBUG_ASSERT ((NULL != return_type), "return_type is NULL");
    } else
        return_type = MakeTypes (T_void);

    DBUG_PRINT ("STOP",
                ("arg_info->node[0]: %s", mdb_nodetype[arg_info->node[0]->nodetype]));

    if (NULL == arg_info->node[2]) {
        return_tmp = return_type;
        if (NULL != exprs)
            exprs = exprs->node[1];

        while (exprs) {
            return_tmp->next = TI (exprs->node[0], arg_info);
            return_tmp = return_tmp->next;
            if (NULL == return_tmp) {
                ERROR (NODE_LINE (arg_node), ("Return_type cannot be infered."));
                CONT_ERROR (("Infered so far: %s", Type2String (return_type, 0)));
                ABORT_ON_ERROR;
            }

            DBUG_ASSERT ((NULL != return_tmp), "return_tmp is NULL");
            DBUG_PRINT ("STOP", ("arg_info->node[0]: %s",
                                 mdb_nodetype[arg_info->node[0]->nodetype]));

            exprs = exprs->node[1];
        }
        return_tmp->next = NULL;

        /* now we look for function where return belongs to */
        fun_p = LookupFun (NULL, NULL, arg_info->node[0]->node[1]);
        if (NULL == fun_p) {
            ABORT (NODE_LINE (arg_node),
                   ("Function '%s` undefined",
                    ModName (arg_info->node[0]->node[1]->info.types->id_mod,
                             arg_info->node[0]->node[1]->info.types->id)));
        } else
            fun_type = fun_p->node->TYPES;

        fun_p->node->node[3] = arg_node; /* set node[3] of N_fundef to this N_return
                                          * node.
                                          */
        fun_name = fun_type->id;
        fun_mod = fun_type->id_mod;
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
                        Type2String (fun_tmp, 0), Type2String (return_tmp, 0),
                        ModName (fun_mod, fun_name)));
                break;
            }
            case CMP_equal: /* 1*/
                /* types are equal */
                break;
            case CMP_one_unknown_shape: /* 2 */
            {
                UpdateType (fun_type, return_type, NODE_LINE (arg_node));
                str1 = Type2String (fun_tmp, 0);

#ifdef SHAPE_NOTE
                NOTE (("%s:%d: NOTE: one return_type of function '%s` has now"
                       " defined shape (%s)",
                       filename, NODE_LINE (arg_node), fun_name, str1));
#endif /*SHAPE_NOTE */

                FREE (str1);
                break;
            }
            case CMP_different_dimensions: /* 3 */
            {
                str1 = Type2String (fun_tmp, 0);
                str2 = Type2String (return_type, 0);
                ERROR (NODE_LINE (arg_node),
                       ("Different types in declaration (%s) and return "
                        "value (%s) of function '%s`",
                        str1, str2, ModName (fun_mod, fun_name)));

                FREE (str1);
                FREE (str2);
                break;
            }
            case CMP_different_shapes: /* 4 */
            {
                str1 = Type2String (fun_tmp, 0);
                str2 = Type2String (return_type, 0);
                ERROR (NODE_LINE (arg_node),
                       ("Types in declaration (%s) and return value (%s) "
                        "of function '%s` have same dimension and "
                        "number of elements, but different shape",
                        str1, str2, ModName (fun_mod, fun_name)));

                FREE (str1);
                FREE (str2);
                break;
            }
            case CMP_both_unknown_shape: /* neu 8.12 */
            {
                if (kind_of_file != SAC_MOD) {
                    str1 = Type2String (fun_tmp, 0);
                    str2 = Type2String (return_type, 0);
                    WARN (NODE_LINE (arg_node),
                          ("Types with unknown shape in declaration (%s) and return "
                           "value (%s) of function '%s`",
                           str1, str2, ModName (fun_mod, fun_name)));

                    FREE (str1);
                    FREE (str2);
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
 *  macros        : DBUG..., FREE, ABORT,  NULL
 *  remarks       : return type can be T_unknown (in this case
 *                   arg_info->node[0]->nodtype should be N_stop)
 *
 */
types *
TI_ap (node *arg_node, node *arg_info)
{
    fun_tab_elem *fun_p;
    types *return_type, **arg_type;
    int count_args = 0, tmp, i;
    node *current_args;
    char *fun_name = arg_node->FUN_NAME;
    char *mod_name = arg_node->FUN_MOD_NAME;
    char *str_buff, *tmp_str;
    int str_spc = PF_MAXFUNNAMELEN - 1;
    int tmp_fun, tmp_funap, funtypemask;
    node *arg;

    DBUG_ENTER ("TI_ap");

    /* count number of current arguments */
    current_args = arg_node->node[0];
    if (NULL != current_args) {
        while (EXPRS_NEXT (current_args) != NULL) {
            count_args++;
            current_args = EXPRS_NEXT (current_args);
        }
        if ((EXPRS_NEXT (current_args) == NULL) && (EXPRS_EXPR (current_args)) != NULL)
            count_args++;
        arg_type = (types **)Malloc (sizeof (types) * count_args);
    }

    /* store type of the arguments in arg_type[] */
    current_args = arg_node->node[0];
    for (i = 0; i < count_args; i++) {
        arg_type[i] = TI (current_args->node[0], arg_info);
        if (NULL == arg_type[i])
            ABORT (NODE_LINE (arg_node), ("%d. argument of function '%s` not inferable",
                                          (i + 1), ModName (mod_name, fun_name)));
        current_args = current_args->node[1];
    }
    tmp = -1;
    fun_p = FindFun (fun_name, mod_name, arg_type, count_args, arg_info,
                     NODE_LINE (arg_node), &tmp);

    if (NULL == fun_p) {
        ABORT (NODE_LINE (arg_node),
               ("Function '%s` not defined", ModName (mod_name, fun_name)));
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
    arg_node->node[1] = fun_p->node; /* set pointer to function declaration */

    if (profileflag != 0) {
        if (AP_ATFLAG (arg_node) != 1) { /* we did not yet inspect this application! */

            AP_ATFLAG (arg_node) = 1;

            /*
             * prepare a string holding the fun-signature
             */

            str_buff = (char *)Malloc (sizeof (char) * PF_MAXFUNNAMELEN);
            str_buff[0] = '\0';
            str_buff = strncpy (str_buff, FUNDEF_NAME (fun_p->node), str_spc);
            str_spc -= strlen (FUNDEF_NAME (fun_p->node));
            str_buff = strncat (str_buff, "( ", str_spc);
            str_spc -= 2;
            arg = FUNDEF_ARGS (fun_p->node);
            while (arg != NULL) {
                tmp_str = Type2String (ARG_TYPE (arg), 1);
                str_buff = strncat (str_buff, tmp_str, str_spc);
                str_spc -= strlen (tmp_str);
                FREE (tmp_str);
                arg = ARG_NEXT (arg);
                if (arg != NULL) {
                    str_buff = strncat (str_buff, ", ", str_spc);
                    str_spc -= 2;
                }
            }
            str_buff = strncat (str_buff, " )", str_spc);
            str_spc -= 2;

            /*
             * Enumerate the actual funtion iff necessary and possible!
             */

            if (FUNDEF_FUNNO (fun_p->node) == 0) { /* this function is not yet counted! */
                if (PFfuncntr == PF_MAXFUN) {
                    SYSWARN (("\"PF_MAXFUN\" too low"));
                    CONT_WARN (("function \"%s\" will not be profiled !", str_buff));
                    FREE (str_buff);
                } else {
                    PFfunnme[PFfuncntr] = str_buff;
                    FUNDEF_FUNNO (fun_p->node) = PFfuncntr++;
                }
            }
            tmp_fun = FUNDEF_FUNNO (fun_p->node);

            if (tmp_fun != 0) { /* we want to insert profiling-macros! */

                /*
                 * Enumerate the actual function application!
                 */

                if (PFfunapcntr[FUNDEF_FUNNO (fun_p->node)] == PF_MAXFUNAP) {
                    SYSWARN (("\"PF_MAXFUNAP\" too low"));
                    CONT_WARN (
                      ("application of function \"%s\" in line %d will not "
                       "be profiled separately but be accounted to the application "
                       "in line %d !",
                       str_buff, NODE_LINE (arg_node), PFfunapline[tmp_fun][0]));
                    tmp_funap = 0;
                } else {
                    tmp_funap = PFfunapcntr[tmp_fun]++;
                    if (PFfunapcntr[tmp_fun] > PFfunapmax)
                        PFfunapmax = PFfunapcntr[tmp_fun];
                    /*
                     * The above lines allow for determining the actual maximum of
                     * applications of a single function.
                     */

                    PFfunapline[tmp_fun][tmp_funap] = NODE_LINE (arg_node);
                }

                /*
                 * insert N_assign node and N_annotate node above and below the actual
                 * N_assign:
                 * BEFORE:    INFO_TC_LASSIGN    ->
                 *            actual N_assign -> fun_p (...)
                 *            INFO_TC_NEXTASSIGN ->
                 *
                 * AFTER:     INFO_TC_LASSIGN    ->
                 *            new N_assign    -> N_annotate CALL_FUN
                 *            actual N_assign -> fun_p (...)
                 *            new N_assign    -> N_annotate RETURN_FROM_FUN
                 *            INFO_TC_NEXTASSIGN ->
                 */

                funtypemask = 0;

                if (FUNDEF_INLINE (fun_p->node) == 1)
                    funtypemask = funtypemask | INL_FUN;
                if (FUNDEF_STATUS (fun_p->node) == ST_imported)
                    funtypemask = funtypemask | LIB_FUN;
                if (FUNDEF_ATTRIB (fun_p->node) == ST_generic)
                    funtypemask = funtypemask | OVRLD_FUN;

                if (NODE_TYPE (INFO_TC_LASSIGN (arg_info)) == N_block) {
                    ASSIGN_NEXT (BLOCK_INSTR (INFO_TC_LASSIGN (arg_info)))
                      = MakeAssign (MakeAnnotate (funtypemask | RETURN_FROM_FUN, tmp_fun,
                                                  tmp_funap),
                                    INFO_TC_NEXTASSIGN (arg_info));
                    if (INFO_TC_NEXTASSIGN (arg_info) == NULL) /* ??? */
                        ASSIGN_NEXT (BLOCK_INSTR (INFO_TC_LASSIGN (arg_info)))->node[1]
                          = NULL;
                    BLOCK_INSTR (INFO_TC_LASSIGN (arg_info))
                      = MakeAssign (MakeAnnotate (funtypemask | CALL_FUN, tmp_fun,
                                                  tmp_funap),
                                    BLOCK_INSTR (INFO_TC_LASSIGN (arg_info)));
                } else {
                    ASSIGN_NEXT (ASSIGN_NEXT (INFO_TC_LASSIGN (arg_info)))
                      = MakeAssign (MakeAnnotate (funtypemask | RETURN_FROM_FUN, tmp_fun,
                                                  tmp_funap),
                                    INFO_TC_NEXTASSIGN (arg_info));
                    if (INFO_TC_NEXTASSIGN (arg_info) == NULL) /* ??? */
                        ASSIGN_NEXT (ASSIGN_NEXT (INFO_TC_LASSIGN (arg_info)))->node[1]
                          = NULL;
                    ASSIGN_NEXT (INFO_TC_LASSIGN (arg_info))
                      = MakeAssign (MakeAnnotate (funtypemask | CALL_FUN, tmp_fun,
                                                  tmp_funap),
                                    ASSIGN_NEXT (INFO_TC_LASSIGN (arg_info)));
                }
            }
        }
    }

    /* now free the infered type information */
    for (i = 0; i < count_args; i++)
        FREE_TYPES (arg_type[i]);

    if (0 < count_args) {
        FREE (arg_type);
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
 *  macros        : DBUG..., NULL, ERROR, ABORT, FREE
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

    if (NULL != ARRAY_AELEMS (arg_node)) {
        elem = ARRAY_AELEMS (arg_node);
        while (NULL != elem) {
            tmp_type = TI (EXPRS_EXPR (elem), arg_info);
            n_elem++;
            if (NULL == tmp_type) {
                ABORT (NODE_LINE (arg_node),
                       ("%d. element of array cannot be infered", n_elem));
            }

            DBUG_ASSERT ((NULL != tmp_type), "tmp_type is NULL");

            if (NULL == return_type)
                return_type = tmp_type;
            else
              /* CompatibleTypes will be called twice, to infere type of an
               * array that contains 'ints' and 'floats'
               */
              if ((CMP_equal
                   != CompatibleTypes (tmp_type, return_type, -1, NODE_LINE (arg_node)))
                  && (CMP_equal
                      != CompatibleTypes (return_type, tmp_type, -1,
                                          NODE_LINE (arg_node)))) /* 1, 1 */
            {
                str1 = Type2String (tmp_type, 0);
                str2 = Type2String (return_type, 0);
                ERROR (NODE_LINE (elem),
                       ("Array of type '%s` has element of type '%s`", str2, str1));
                FREE (str1);
                FREE (str2);
                FREE_TYPES (tmp_type);
            } else if ((T_int == TYPES_BASETYPE (return_type)
                        && T_float == TYPES_BASETYPE (tmp_type))
                       || (T_int == TYPES_BASETYPE (return_type)
                           && T_double == TYPES_BASETYPE (tmp_type))
                       || (T_float == TYPES_BASETYPE (return_type)
                           && T_double == TYPES_BASETYPE (tmp_type))) {
                FREE_TYPES (return_type);
                return_type = tmp_type;
            } else
                FREE_TYPES (tmp_type);
            elem = EXPRS_NEXT (elem);
        }
        if (NULL != return_type) {
            if (0 == return_type->dim) {
                return_type->dim = 1;
                return_type->shpseg = (shpseg *)Malloc (sizeof (shpseg));
                return_type->shpseg->next = NULL;
                return_type->shpseg->shp[0] = n_elem;
            } else if (-1 != return_type->dim) {
                DBUG_ASSERT ((return_type->dim + 1 <= SHP_SEG_SIZE), " dimension"
                                                                     " out of range");
                for (i = return_type->dim - 1; i >= 0; i--)
                    return_type->shpseg->shp[i + 1] = return_type->shpseg->shp[i];
                return_type->shpseg->shp[0] = n_elem;
                return_type->dim += 1;
            }

#ifndef DBUG_OFF
            db_str = Type2String (return_type, 0);
            DBUG_PRINT ("TYPE", ("type of array: %s", db_str));
            FREE (db_str);
#endif
            /* store type of array in arg_node->info.types */
            arg_node->info.types = DuplicateTypes (return_type, 1);
        } else {
            ABORT (NODE_LINE (arg_node), ("Type of array not inferable"));
        }
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
 *  macros        : DBUG...,CHECKING, IS_CHECKED, P_FORMAT, ERROR, ABORT,
 *                  FREE
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

    DBUG_PRINT ("STACK", ("old_tos is set to " P_FORMAT, old_tos));
    DBUG_ASSERT (N_cond == NODE_TYPE (arg_node), "wrong nodetype");

    old_status = arg_info->node[0]->nodetype;
    rest_node = INFO_TC_NEXTASSIGN (arg_info);

    expr_type = TI (arg_node->node[0], arg_info);
    if (NULL == expr_type) {
        ABORT (NODE_LINE (arg_node), ("Type not inferable"));
    }
    DBUG_ASSERT ((NULL != expr_type), "expr_type is NULL ");
    DBUG_PRINT ("STOP",
                ("arg_info->node[0]: %s", mdb_nodetype[arg_info->node[0]->nodetype]));

    if (T_bool != TYPES_BASETYPE (expr_type)) {
        ERROR (NODE_LINE (arg_node),
               ("Type of condition (%s) is not bool", Type2String (expr_type, 0)));
    }

    /* now free the infered type-information */
    FREE_TYPES (expr_type);

    fun_p = LookupFun (NULL, NULL, arg_info->node[0]->node[1]);
    DBUG_ASSERT ((NULL != fun_p), "fun_p is NULL");
    if ((CHECKING == fun_p->tag) || (IS_CHECKED == fun_p->tag)) {
        for (i = 1; i < nnode[NODE_TYPE (arg_node)]; i++) {
            tos = old_tos;
            DBUG_PRINT ("STACK", ("tos is set to " P_FORMAT, tos));
            arg_info->node[0]->info.cint += 1;
#ifndef DBUG_OFF
            if (1 == i)
                DBUG_PRINT ("TYPE", ("checking THEN part"));
            else
                DBUG_PRINT ("TYPE", ("checking ELSE part"));
#endif
            Trav (arg_node->node[i], arg_info);
            arg_info->node[0]->info.cint -= 1;
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
            DBUG_PRINT ("STACK", ("tos is set to " P_FORMAT, tos));
            arg_info->node[0]->nodetype = old_status;
            arg_info->node[0]->info.cint += 1;
#ifndef DBUG_OFF
            if (1 == check_again)
                DBUG_PRINT ("TYPE", ("checking THEN part again"));
            else
                DBUG_PRINT ("TYPE", ("checking ELSE part again"));
#endif
            Trav (arg_node->node[check_again], arg_info);
            arg_info->node[0]->info.cint -= 1;
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
            DBUG_PRINT ("STACK", ("tos is set to " P_FORMAT, tos));
            arg_info->node[0]->info.cint += 1;

            if (1 == i)
                DBUG_PRINT ("TYPE", ("checking THEN part"));
            else
                DBUG_PRINT ("TYPE", ("checking ELSE part"));

            Trav (arg_node->node[i], arg_info);
            arg_info->node[0]->info.cint -= 1;
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

    DBUG_ENTER ("TCblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }
    ABORT_ON_ERROR;

    INFO_TC_LASSIGN (arg_info) = arg_node;
    /* For the traversal of the assignments I have to
     * ignore the return-value, since TI_ap in some cases
     * inserts N_assign's with N_annotate's!
     * (compare TCassign!)
     */
    Trav (BLOCK_INSTR (arg_node), arg_info);
    INFO_TC_LASSIGN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *
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
    node *old_current_assign;

    DBUG_ENTER ("TCassign");

    DBUG_ASSERT (N_assign == NODE_TYPE (arg_node), "wrong nodetype");

    INFO_TC_NEXTASSIGN (arg_info) = ASSIGN_NEXT (arg_node);

    old_current_assign = INFO_TC_CURRENTASSIGN (arg_info);
    INFO_TC_CURRENTASSIGN (arg_info) = arg_node;
    /*
     * The current asssign back reference must be stacked here due to nested
     * N_assign nodes in the presence of with-loop bodies.
     */

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        if (N_stop != arg_info->node[0]->nodetype) {
            INFO_TC_LASSIGN (arg_info) = arg_node;
            /* For the traversal of the next assignments I have to
             * ignore the return-value, since TI_ap in some cases
             * inserts N_assign's with N_annotate's!
             * (compare TCblock!)
             */
            Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    } else
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    INFO_TC_LASSIGN (arg_info) = NULL;
    INFO_TC_NEXTASSIGN (arg_info) = NULL;
    INFO_TC_CURRENTASSIGN (arg_info) = old_current_assign;

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
 *  macros        : DBUG..., ERROR, ABORT, FREE, NULL
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
    expr_type = TI (DO_COND (arg_node), arg_info);
    if (NULL == expr_type) {
        ABORT (NODE_LINE (arg_node), ("Type not inferable"));
    } else if (T_bool != TYPES_BASETYPE (expr_type))
        ERROR (NODE_LINE (DO_COND (arg_node)),
               ("Type is '%s` but must be 'bool`", Type2String (expr_type, 0)));

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
    expr_type = TI (WHILE_COND (arg_node), arg_info);
    if (!expr_type)
        ABORT (NODE_LINE (arg_node), ("Type not inferable"));

    if (T_bool == TYPES_BASETYPE (expr_type))
        /* traverse body of while_loop */
        Trav (WHILE_BODY (arg_node), arg_info);
    else
        ERROR (NODE_LINE (WHILE_COND (arg_node)),
               ("Type is '%s` but must be 'bool`", Type2String (expr_type, 0)));

    /* restore scope stack by setting 'tos' pointer */
    tos = old_tos;

    /* now free infered type_information */
    FREE_TYPES (expr_type);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : TCunaryOp
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : checks the type of pre & post increment/decrement
 *                  used for N_post & N_pre
 *
 *  global vars   : filename
 *  internal funs : LookupVar
 *  external funs : Type2String
 *  macros        : DBUG..., ERROR, P_FORMAT
 *
 *  remarks       : the reference to the variabledeclaration
 *                  is put to arg_node->node[1], nnode is still 1
 *
 */
node *
TCunaryOp (node *arg_node, node *arg_info)
{
    stack_elem *stack_p;

    DBUG_ENTER ("TCunaryOp");

    stack_p = LookupVar (arg_node->info.ids->id);
    if (NULL == stack_p) {
        ERROR (NODE_LINE (arg_node),
               ("Identifier '%s` undefined", arg_node->info.ids->id));
    } else {
        types *type = stack_p->node->info.types;

        if ((0 != type->dim)
            || (!((T_int == TYPES_BASETYPE (type))
                  || (T_float == TYPES_BASETYPE (type))))) {
            ERROR (NODE_LINE (arg_node), ("Identifier '%s` has wrong type (%s)",
                                          arg_node->info.ids->id, Type2String (type, 0)));
        } else {
            arg_node->info.ids->node = stack_p->node;
            DBUG_PRINT ("REF", ("added reference" P_FORMAT " for %s to %s",
                                arg_node->info.ids->node, arg_node->info.ids->id,
                                arg_node->info.ids->node->info.types->id));
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : TI_cast
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : computes the type of a casted expression
 *
 *  global vars   : filename
 *  internal funs : TI, DuplicateTypes, CompatibleTypes
 *  external funs : Type2String
 *  macros        : DBUG..., ERROR, ABORT, TYPES, MOD_NAME, FUN_MOD_NAME,
 *                  FUN_NAME, FREE
 *
 *  remarks       :
 *
 */
types *
TI_cast (node *arg_node, node *arg_info)
{
    types *ret_type, *type, *tmp, *inf_type, *free_type;
    int is_comp, i = 1;
    node *t_node;

    DBUG_ENTER ("TI_cast");

    /* store casted types in ret_type */
    ret_type = DuplicateTypes (CAST_TYPE (arg_node), 1);

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

#if 0
   type=ret_type;
   cast_node=CAST_EXPR(arg_node);
   while( N_cast == NODE_TYPE(cast_node))
   {
      type->next=DuplicateTypes(cast_node->TYPES, 1);
      type=type->next;
      if( (T_user == TYPES_BASETYPE(type)) && (NULL == type->name_mod))
      {
         t_node=LookupType(type->name, NULL, NODE_LINE(cast_node));
         if(NULL == t_node)
         {
           ABORT(NODE_LINE(cast_node),
                 ("Type '%s` unknown",
                  ret_type->name));
         }
         else
         {
            type->name_mod=t_node->ID_MOD;
            cast_node->NAME_MOD=t_node->ID_MOD;
         }
      }
      cast_node=CAST_EXPR(cast_node);
   }
   
   /* get the type of casted expression */
   inf_type=TI(cast_node, arg_info);
#endif

    inf_type = TI (CAST_EXPR (arg_node), arg_info);
    if (NULL == inf_type)
        switch (arg_node->nodetype) {
        case N_id: {
            ABORT (NODE_LINE (arg_node),
                   ("Variable '%s` undefined", arg_node->info.ids->id));
            break;
        }
        case N_ap: {
            ABORT (NODE_LINE (arg_node),
                   ("Type of function '%s` not inferable",
                    ModName (arg_node->FUN_MOD_NAME, arg_node->FUN_NAME)));
            break;
        }
        default: {
            ABORT (NODE_LINE (arg_node), ("Type of expression '%s` not inferable",
                                          mdb_nodetype[arg_node->nodetype]));
            break;
        }
        }

    /* check whether casts are compatible */
    type = ret_type;
    tmp = inf_type;
    while (NULL != tmp) {
        is_comp = CompatibleTypes (type, tmp, 0, NODE_LINE (arg_node));
        if (CMP_incompatible == is_comp) /* 0 */
        {
            ERROR (NODE_LINE (arg_node), ("%d. cast has incompatible type", i));
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

    /* check whether there are to many casts */
    if ((NULL != type->next) && (NULL == tmp))
        ABORT (NODE_LINE (arg_node), ("Too many casts"))
    if (NULL != tmp)
        type->next = tmp;

#ifndef DBUG_OFF
    if (NULL != ret_type) {
        char *db_str = Type2String (ret_type, 0);
        DBUG_PRINT ("TYPE", ("%s", db_str));
    }
#endif

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : TI_genarray
 *  arguments     : 1) argument node
 *                  2) type of generator bounds
 *                  3) return type of local block
 *                  4) info_node
 *  description   : computes type of genarray and checks whether this type,
 *                  the type of the generator and the type of with_return fit
 *
 *  global vars   : filename
 *  internal funs : Malloc
 *  external funs : sizeof, Type2String
 *  macros        : DBUG..., ABORT, SHP_SEG_SIZE, GEN_TYPE_NODE
 *
 *  remarks       :
 *
 */
types *
TI_genarray (node *arg_node, types *generator_type, types *w_return_type, node *arg_info)
{
    types *ret_type;
    int dim = 0, i, length;
    node *tmp;

    DBUG_ENTER ("TI_genarray");
    if (N_array == NODE_TYPE (arg_node)) {
        /* infer type of N_array and store it in N_array-node */

        ARRAY_TYPE (arg_node) = TI (arg_node, NULL);
        /* first compute return_type */
        ret_type = DuplicateTypes (w_return_type, 1);
        TYPES_SHPSEG (ret_type) = (shpseg *)Malloc (sizeof (shpseg));
        TYPES_NEXT (ret_type) = NULL;

        tmp = ARRAY_AELEMS (arg_node);
        while (tmp)
            if (dim < SHP_SEG_SIZE)
                if (N_num == NODE_TYPE (EXPRS_EXPR (tmp))) {
                    SHAPES_SELEMS (ret_type)[dim] = NUM_VAL (EXPRS_EXPR (tmp));
                    tmp = EXPRS_NEXT (tmp);
                    dim++;
                } else {
                    ABORT (NODE_LINE (arg_node),
                           ("%d. element of shape vector not a constant", dim + 1));
                }
            else {
                ABORT (NODE_LINE (arg_node), ("Shape vector has too many elements"));
            }

        TYPES_DIM (ret_type) = dim;

        /* now check whether the computed type ret_type and the type
         * information form generator and with_return fit
         */
        length = generator_type->shpseg->shp[0];
        if ((length + w_return_type->dim) == dim)
            for (i = length; i < dim; i++) {
                if (ret_type->shpseg->shp[i] != w_return_type->shpseg->shp[i - length]) {
                    ABORT (NODE_LINE (arg_node),
                           ("Type of shape vector (%s) does not match "
                            "type of index vector (%s) and return statement (%s)",
                            Type2String (ret_type, 0), Type2String (generator_type, 0),
                            Type2String (w_return_type, 0)));
                }
            }
        else {
            ABORT (NODE_LINE (arg_node),
                   ("Type of shape vector (%s) does not match "
                    "type of index vector (%s) and return statement (%s)",
                    Type2String (ret_type, 0), Type2String (generator_type, 0),
                    Type2String (w_return_type, 0)));
        }
    } else {
        ABORT (NODE_LINE (arg_node),
               ("Argument of 'genarray` not a constant shape vector"));
    }

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : TI_modarry
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : checks whether the type of the array, the type of the
 *                  generator and the type of with_return fit.
 *                  returns type of the array
 *
 *  global vars   : filename
 *  internal funs : TI
 *  external funs : Type2String
 *  macros        : DBUG..., ABORT
 *
 *  remarks       :
 *
 */
types *
TI_modarray (node *arg_node, types *generator_type, types *w_return_type, node *arg_info)
{
    types *ret_type, *a_type;
    int length, i;

    DBUG_ENTER ("TI_modarray");

    a_type = TI (arg_node, arg_info);
    if (!a_type)
        ABORT (NODE_LINE (arg_node), ("Array component of 'modarray` not inferable"));
    ret_type = a_type;

    /* now check whether the computed type ret_type and the type
     * information form generator and with_return fit
     */
    if (TYPES_BASETYPE (ret_type) != TYPES_BASETYPE (w_return_type))
        ABORT (NODE_LINE (arg_node),
               ("Type of 'modarray` (%s) and return_type (%s) different",
                Type2String (ret_type, 0), Type2String (w_return_type, 0)))

    length = generator_type->shpseg->shp[0];
    if ((length + w_return_type->dim) == ret_type->dim)
        for (i = length; i < ret_type->dim; i++) {
            if (ret_type->shpseg->shp[i] != w_return_type->shpseg->shp[i - length])
                ABORT (NODE_LINE (arg_node),
                       ("Type of shape vector (%s) does not match "
                        "type of index vector (%s) and return statement (%s)",
                        Type2String (ret_type, 0), Type2String (generator_type, 0),
                        Type2String (w_return_type, 0)));
        }
    else
        ABORT (NODE_LINE (arg_node),
               ("Type of shape vector (%s) does not match "
                "type of index vector (%s) and return statement (%s)",
                Type2String (ret_type, 0), Type2String (generator_type, 0),
                Type2String (w_return_type, 0)));

    DBUG_RETURN (ret_type);
}
/*
 *
 *  functionname  : TI_foldprf
 *  arguments     : 1) argument node
 *                  2) type of generator
 *                  3) type of with_return
 *                  4) info_node
 *  description   :
 *  global vars   : filename
 *  internal funs :
 *  external funs : Type2String
 *  macros        : DBUG..., ABORT
 *
 *  remarks       :
 *
 */
types *
TI_foldprf (node *arg_node, types *generator_type, types *w_return_type, node *arg_info)
{
    prim_fun_tab_elem *prf_p;
    void *fun_p;
    types *ret_type, **arg_type;
    int type_c_tag, prf_tag;
    prf old_prf;

    DBUG_ENTER ("TI_foldprf");

    arg_type = (types **)Malloc (sizeof (types) * 2);
    arg_type[0] = w_return_type;
    arg_type[1] = w_return_type;
    old_prf = arg_node->info.prf;
    prf_tag = arg_node->info.prf;
    fun_p = FindFun (NULL, NULL, arg_type, 2, arg_info, NODE_LINE (arg_node), &prf_tag);
    DBUG_ASSERT (NULL != fun_p, "fun_p is NULL");
    if (prf_tag == arg_node->info.prf) {
        /* it is a primitive function */
        prf_p = (prim_fun_tab_elem *)fun_p;
        type_c_tag = prf_p->node->info.prf_dec.tag;
        arg_node->info.prf = prf_p->new_prf;
        /* now compute resulting type (ret_type) */
        switch (type_c_tag) {
#include "prim_fun_tt.mac"
        default:
            DBUG_ASSERT (0, "wrong type_class_tag");
            break;
        }
    } else {
        /* userdefined primitive function */

        /* change union 'info' of 'arg_node'. Use info.fun_name->id,
         * and info.fun_name->id_mod instead of info.prf.
         * arg_node->node[2] will point to belonging function declaration. This
         * also indicates that the union 'info' has changed.
         */
        arg_node->nodetype = N_foldfun;
        arg_node->FUN_NAME = StringCopy (((fun_tab_elem *)fun_p)->id);
        ret_type = TI_fun (arg_node, (fun_tab_elem *)fun_p, arg_info);
        /* set pointer to function declaration */
        FOLDFUN_FUNDEF (arg_node) = ((fun_tab_elem *)fun_p)->node;
    }

    if (T_unknown == TYPES_BASETYPE (ret_type)) {
        ABORT (NODE_LINE (arg_node),
               ("Primitive function '%s` is applied to wrong arguments",
                prf_string[NWITHOP_PRF (arg_node)]));
    } else if (CMP_equal
               != CompatibleTypes (w_return_type, ret_type, -1,
                                   NODE_LINE (arg_node))) /* 1*/
        if (N_foldfun == arg_node->nodetype) {
            ABORT (NODE_LINE (arg_node),
                   ("Function '%s` has return type '%s` != '%s`",
                    ModName (arg_node->FUN_MOD_NAME, arg_node->FUN_NAME),
                    Type2String (ret_type, 0), Type2String (w_return_type, 0)));
        } else
            ABORT (NODE_LINE (arg_node),
                   ("Function '%s` has return type '%s` != '%s`",
                    prf_string[arg_node->info.prf], Type2String (ret_type, 0),
                    Type2String (w_return_type, 0)));
    if (FOLDPRF_NEUTRAL (arg_node)) {
        types *neutral_type = TI (arg_node->node[1], arg_info);
        if (CMP_equal != CmpTypes (neutral_type, ret_type)) /* 1 */ {
            ABORT (NODE_LINE (arg_node),
                   ("2. argument of 'fold` has wrong type '%s` != '%s`",
                    Type2String (neutral_type, 0), Type2String (ret_type, 0)));
        }
    } else
        FOLDPRF_NEUTRAL (arg_node) = ComputeNeutralElem (old_prf, w_return_type);

    FREE (arg_type);

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : TI_foldfun
 *  arguments     : 1) argument node
 *                  2) type of generator
 *                  3) type of with_return
 *                  4) info_node
 *  description   :
 *  global vars   : filename
 *  internal funs :
 *  external funs : Type2String
 *  macros        : DBUG..., ABORT
 *
 *  remarks       :
 *
 */
types *
TI_foldfun (node *arg_node, types *generator_type, types *w_return_type, node *arg_info)
{
    fun_tab_elem *fun_p;
    types *ret_type, **arg_type;
    int tmp;

    DBUG_ENTER ("TI_foldfun");

    arg_type = (types **)Malloc (sizeof (types) * 2);
    arg_type[0] = w_return_type;
    arg_type[1] = w_return_type;

    tmp = -1;
    fun_p = (fun_tab_elem *)FindFun (arg_node->FUN_NAME, arg_node->FUN_MOD_NAME, arg_type,
                                     2, arg_info, NODE_LINE (arg_node), &tmp);

    DBUG_ASSERT (NULL != fun_p, "fun_p is NULL");

    ret_type = TI_fun (arg_node, fun_p, arg_info);

    if (T_unknown == TYPES_BASETYPE (ret_type)) {
        ABORT (NODE_LINE (arg_node),
               ("Type of function '%s` not inferable",
                ModName (arg_node->FUN_MOD_NAME, arg_node->FUN_NAME)));
    } else {
        types *neutral_type;

        if (CMP_equal != CmpTypes (w_return_type, ret_type)) /* 1*/
        {
            ABORT (NODE_LINE (arg_node),
                   ("Function '%s` has return type '%s` != '%s`",
                    ModName (arg_node->FUN_MOD_NAME, arg_node->FUN_NAME),
                    Type2String (ret_type, 0), Type2String (w_return_type, 0)));
        }
        neutral_type = TI (arg_node->node[1], arg_info);
        if (CMP_equal != CmpTypes (neutral_type, ret_type)) /* 1 */
        {
            ABORT (NODE_LINE (arg_node),
                   ("2. argument of 'fold` has wrong type '%s` != '%s`",
                    Type2String (neutral_type, 0), Type2String (ret_type, 0)));
        }
        arg_node->node[2] = fun_p->node; /* set pointer to function declararion */
    }

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : TI_generator
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : computes the type of the index-vector if types of left
 *                  and right border fit.
 *
 *  global vars   : filename
 *  internal funs : TI, LookupVar, CmpTypes, Malloc
 *  external funs : sizeof, Type2String
 *  macros        : DBUG..., ABORT
 *
 *  remarks       :
 *
 */
types *
TI_generator (node *arg_node, node *arg_info)
{
    types *ret_type, *left_type, *right_type;
    stack_elem *stack_p;
    cmp_types ok;

    DBUG_ENTER ("TI_generator");

    left_type = TI (arg_node->node[0], arg_info);
    right_type = TI (arg_node->node[1], arg_info);
    ok = CmpTypes (left_type, right_type);
    if ((CMP_equal == ok) && (T_int == TYPES_BASETYPE (left_type))
        && (1 == left_type->dim)) /* 1*/
    {
        stack_p = LookupVar (GEN_ID (arg_node));
        if (NULL != stack_p) {
            ok = CmpTypes (left_type, stack_p->node->info.types);
            if (CMP_equal == ok) /* 1*/
            {
                GEN_VARDEC (arg_node) = stack_p->node;
                DBUG_PRINT ("REF", ("added reference" P_FORMAT " for %s to %s",
                                    arg_node->info.ids->id, arg_node->info.ids->id,
                                    arg_node->info.ids->node->info.types->id));
                ret_type = left_type;
            } else
                ABORT (NODE_LINE (arg_node), ("Index vector has wrong type '%s` != '%s`",
                                              Type2String (stack_p->node->info.types, 0),
                                              Type2String (left_type, 0)));
        } else {
            AddIdToStack (GEN_IDS (arg_node), left_type, arg_info, NODE_LINE (arg_node));
            ret_type = left_type;
        }
    } else
        ABORT (NODE_LINE (arg_node),
               ("Wrong types in generator part '%s` <= .. <= '%s`",
                Type2String (left_type, 0), Type2String (right_type, 0)));

    FREE_TYPES (right_type);

    DBUG_RETURN (ret_type);
}

/*
 *
 *  functionname  : TI_with
 *  arguments     : 1) argument node
 *                  2) info_node
 *  description   : manages typeinference of with-loop
 *
 *  global vars   :
 *  internal funs : TI_modarray, TI_genarray, TI_generator
 *  external funs : Trav
 *  macros        : DBUG..., FREE
 *
 *  remarks       : puts index_vec to arg_info->node[2]
 *                  gets type of with-return in arg_info->node[2]->info.types
 */
types *
TI_with (node *arg_node, node *arg_info)
{
    stack_elem *old_tos;
    types *ret_type, *generator_type, *w_return_type;
    char *index_var;
    node *index_vec, *con_expr;
    int other_with = 0;

    DBUG_ENTER ("TI_with");

    /* save old tos to assure that variables which area defined in the WL
       (generatorvars, inside the body) are unknown afterwards. */
    old_tos = tos;

    generator_type = TI_generator (WITH_GEN (arg_node), arg_info);
    index_var = GEN_ID (WITH_GEN (arg_node));
    index_vec = MakeNode (N_arg);
    ARG_TYPE (index_vec) = generator_type;
    ARG_NAME (index_vec) = index_var;

    if (arg_info->node[2]) {
        ARG_NEXT (index_vec) = arg_info->node[2];
        other_with = 1;
    }
    arg_info->node[2] = index_vec;

    con_expr = WITH_OPERATOR (arg_node);

    if (N_genarray == NODE_TYPE (con_expr)) {
        Trav (GENARRAY_BODY (con_expr), arg_info);
        w_return_type = ARG_TYPE (arg_info->node[2]);
        ret_type = TI_genarray (GENARRAY_ARRAY (con_expr), generator_type, w_return_type,
                                arg_info);
    } else if (N_modarray == NODE_TYPE (con_expr)) {
        Trav (MODARRAY_BODY (con_expr), arg_info);
        w_return_type = arg_info->node[2]->info.types;
        ret_type = TI_modarray (MODARRAY_ARRAY (con_expr), generator_type, w_return_type,
                                arg_info);
    } else if (N_foldprf == con_expr->nodetype) {
        Trav (con_expr->node[0], arg_info);
        w_return_type = arg_info->node[2]->info.types;
        ret_type = TI_foldprf (con_expr, generator_type, w_return_type, arg_info);
    } else if (N_foldfun == con_expr->nodetype) {
        Trav (con_expr->node[0], arg_info);
        w_return_type = arg_info->node[2]->info.types;
        ret_type = TI_foldfun (con_expr, generator_type, w_return_type, arg_info);
    } else
        DBUG_ASSERT (0, "wrong nodetype");

    if (1 == other_with) {
        node *tmp = arg_info->node[2];
        arg_info->node[2] = ARG_NEXT (tmp);
        FREE_TYPES (tmp->info.types);
        FREE (tmp);
    } else {
        FREE_TYPES (arg_info->node[2]->info.types);
        FREE (arg_info->node[2]);
    }
    /*FREE_TYPES(generator_type); */

    tos = old_tos;

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

types *
TI_Nwith (node *arg_node, node *arg_info)
{
    stack_elem *old_tos;
    types *generator_type, *base_array_type, *body_type, *neutral_type;
    node *tmpn, *withop, *new_fundef;
    ids *mem_lhs;
    int i;

    DBUG_ENTER ("TI_Nwith");

    /* save old tos to assure that variables which area defined in the WL
       (generatorvars, inside the body) are unknown afterwards. */
    old_tos = tos;

    /* First we try to infere the type of the array to modify (modarray)
       or the shape of the base-array (genarray). This cannot be done
       for the withop fold. */
    base_array_type = NULL;
    neutral_type = NULL;
    if (WO_genarray == NWITHOP_TYPE (NWITH_WITHOP (arg_node)))
        base_array_type = TI_Ngenarray (NWITHOP_SHAPE (NWITH_WITHOP (arg_node)), arg_info,
                                        &(NWITHOP_SHAPE (NWITH_WITHOP (arg_node))));
    /* attention: The base type in case of genarray is infered from the
       constant argument vector. This vector if int[.] of course, but this
       is not returned. E.g. if the vector is int[2] = [2,3] the base_array_type
       is int[2,3]. Attend that ALWAYS the simpletype int is returned because
       we cannot infere the base type from the given vector. The real simpletype
       has to be infered from the CEXPR later. */
    else if (WO_modarray == NWITHOP_TYPE (NWITH_WITHOP (arg_node))) {
        base_array_type = TI (NWITHOP_ARRAY (NWITH_WITHOP (arg_node)), arg_info);
        if (!base_array_type)
            ABORT (NODE_LINE (NWITHOP_ARRAY (NWITH_WITHOP (arg_node))),
                   ("Array component of 'modarray` not inferable"));
    } else if (NWITHOP_NEUTRAL (NWITH_WITHOP (arg_node))) /* only if exists */ {
        neutral_type = TI (NWITHOP_NEUTRAL (NWITH_WITHOP (arg_node)), arg_info);
        if (!neutral_type)
            ABORT (NODE_LINE (NWITHOP_NEUTRAL (NWITH_WITHOP (arg_node))),
                   ("Neutral element of 'fold` not inferable"));
    }

    generator_type = TI_Npart (NWITH_PART (arg_node), base_array_type, arg_info);

    /* In case of genarray() check whether generator and base array fit. */
    if (WO_genarray == NWITHOP_TYPE (NWITH_WITHOP (arg_node))
        && SCALAR < TYPES_DIM (generator_type) && SCALAR < TYPES_DIM (base_array_type)
        && TYPES_SHAPE (generator_type, 0) != TYPES_DIM (base_array_type)) {
        ABORT (NODE_LINE (NWITHOP_NEUTRAL (NWITH_WITHOP (arg_node))),
               ("Generator and base shape are incompatible. "
                "Expected generator of type int[%d]",
                TYPES_DIM (base_array_type)));
    }

    /* we now have the type of the generator and, in case of genarray or
       modarray, the type of the base array, too.
       The next step is to typecheck the body (CBLOCK) and the additional
       expression (CEXPR). */
    tmpn = MakeNode (N_arg);
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
            FreeOneTypes (body_type);
        } else if (SCALAR > TYPES_DIM (body_type)
                   || SCALAR > TYPES_DIM (base_array_type)) {
            FreeOneTypes (body_type);
            body_type = MakeType (TYPES_BASETYPE (base_array_type), UNKNOWN_SHAPE, NULL,
                                  NULL, NULL);
            FreeOneTypes (base_array_type);
            base_array_type = body_type;
        } else {
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
                                              F_toi, /* this is just a dummy! */
                                              IDS_NAME (INFO_TC_LHSVARS (arg_info)),
                                              ID_NAME (NWITH_CEXPR (arg_node)));
        }

        NWITHOP_FUNDEF (withop) = new_fundef;
        NWITHOP_FUN (withop) = StringCopy (FUNDEF_NAME (new_fundef));
        NWITHOP_MOD (withop) = StringCopy (PSEUDO_MOD_FOLD);

        /* insert new function into fun_table */
        OLD_INSERT_FUN (fun_table, new_fundef->ID, new_fundef->ID_MOD, new_fundef,
                        PLEASE_CHECK, -2);

        /*
         * prepand new_fundef to the olf fundefs pointed at by top_fundef!
         * This makes sure that these dummy-funs will be compiled before
         * any other function!
         */
        FUNDEF_NEXT (new_fundef) = top_fundef;
        top_fundef = new_fundef;

        FreeOneTypes (body_type);
        FreeOneTypes (neutral_type);
        break;

    case WO_foldfun:
        base_array_type = TI_Nfoldfun (withop, body_type, neutral_type, arg_info);

        /*
         * Now, we create a pseudo function for the fold-fun iff this has
         * not been done yet. ( i.e. if the module-name does not equal PSEUDO_MOD_FOLD!).
         * The pseudo function is needed for the code-generation of the with-loop!!
         * Although it could be regarded as a kind of flatten-step
         * it has to be done here since type-declarations for functions
         * are mendatory in SAC!!
         */
        if (strcmp (NWITHOP_MOD (withop), PSEUDO_MOD_FOLD) != 0) {
            DBUG_ASSERT ((NODE_TYPE (NWITH_CEXPR (arg_node)) == N_id),
                         "Cexpr of Ncode from with-loop is expected to be an N_id node!");
            DBUG_ASSERT ((INFO_TC_LHSVARS (arg_info) != NULL),
                         "during type-checking with-loops are expected to be RHS of "
                         "let-exprs only!");

            new_fundef = CreatePseudoFoldFun (body_type, NWITHOP_FUN (withop),
                                              F_toi, /* this is just a dummy! */
                                              IDS_NAME (INFO_TC_LHSVARS (arg_info)),
                                              ID_NAME (NWITH_CEXPR (arg_node)));

            NWITHOP_FUNDEF (withop) = new_fundef;
            NWITHOP_FUN (withop) = StringCopy (FUNDEF_NAME (new_fundef));
            NWITHOP_MOD (withop) = StringCopy (PSEUDO_MOD_FOLD);

            /* insert new function into fun_table */
            OLD_INSERT_FUN (fun_table, new_fundef->ID, new_fundef->ID_MOD, new_fundef,
                            PLEASE_CHECK, -2);

            /*
             * prepand new_fundef to the olf fundefs pointed at by top_fundef!
             * This makes sure that these dummy-funs will be compiled before
             * any other function!
             */
            FUNDEF_NEXT (new_fundef) = top_fundef;
            top_fundef = new_fundef;
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
    }

    tmpn = ARG_NEXT (arg_info->node[2]);
    FREE (arg_info->node[2]); /* free N_arg-node */
    arg_info->node[2] = tmpn; /* ...and restore old one. */

    tos = old_tos;

    if (SAC_PRG == kind_of_file && SCALAR > TYPES_DIM (base_array_type))
        /* SCALAR == TYPES_DIM() possible in case of fold. */
        ABORT (NODE_LINE (arg_node), ("No constant type for withloop inferable"));

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
                            Type2String (bound, 0), Type2String (*concrete_type, 0)));   \
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
 *   types *TI_Npart(node *arg_node, node *arg_info)
 *
 * description:
 *   checks the generator of the new WL and returns the type of the lower
 *   bound.
 *   1) substitues bounds . with constant vectors
 *   2) checks if bounds/step/width have same type
 *   3) completes N_Nwithid (introduce vector AND array of scalars as index)
 *   4) checks type of index variables (vector and array of scalars)
 *
 * remark: Point 1) is not possible if the withop is a fold
 *   function because we have no array identifier (modarray) or shape
 *   vector (like genarray) on which to base the number of elements of
 *   the index vector.
 *
 ******************************************************************************/

types *
TI_Npart (node *arg_node, types *default_bound_type, node *arg_info)
{
    types *left_type, *right_type, *step_type, *width_type, *gen_type;
    types *tmpt, **concrete_type;
    node *gen, *withid, *tmpn;
    int i, add;
    ids *_ids;

    DBUG_ENTER ("TI_Npart");
    gen = NPART_GEN (arg_node);

    /* replace bounds . with a constant array. For the lower bound this is
       always possible ([0,...,0]). For the upper bound we need not only
       the dimension but the shape. This is made available by
       default_bound_shape. */

    if (!NGEN_BOUND1 (gen)) {
        /* transform lower bound .
           if the operator is < it is changed to <= and 1 is added to the
           lower bound. */
        if (!default_bound_type)
            ABORT (NODE_LINE (arg_node), ("bound . not allowed with function fold"));
        if (SCALAR < TYPES_DIM (default_bound_type)) { /* strong TC */
            tmpn = NULL;
            add = F_lt == NGEN_OP1 (gen);
            if (add)
                NGEN_OP1 (gen) = F_le;
            for (i = TYPES_DIM (default_bound_type) - 1; i >= 0; i--)
                tmpn = MakeExprs (MakeNum (add), tmpn);
            NGEN_BOUND1 (gen) = MakeArray (tmpn);
        }
    }
    if (!NGEN_BOUND2 (gen)) {
        /* transform upper bound .
            if the operator is <= it is changed to < and 1 is added to the
     upper bound. */
        if (!default_bound_type)
            ABORT (NODE_LINE (arg_node), ("bound . not allowed with function fold"));
        if (SCALAR < TYPES_DIM (default_bound_type)) { /* strong TC */
            tmpn = NULL;
            add = F_le == NGEN_OP2 (gen);
            if (add)
                NGEN_OP2 (gen) = F_lt;
            for (i = TYPES_DIM (default_bound_type) - 1; i >= 0; i--)
                tmpn = MakeExprs (MakeNum (TYPES_SHAPE (default_bound_type, i) + add - 1),
                                  tmpn);
            NGEN_BOUND2 (gen) = MakeArray (tmpn);
        }
    }

    /* Now check whether the bounds, step and width have the same type.
       First, infere all types. */
    step_type = NULL;
    width_type = NULL;

    if (NGEN_BOUND1 (gen)) {
        left_type = TI (NGEN_BOUND1 (gen), arg_info);
        if (!left_type)
            ERROR (NODE_LINE (arg_node), ("lower bound cannot be infered"));
    } else
        left_type = MakeType (T_int, KNOWN_DIM_OFFSET - 1, NULL, NULL, NULL);

    if (NGEN_BOUND2 (gen)) {
        right_type = TI (NGEN_BOUND2 (gen), arg_info);
        if (!right_type)
            ERROR (NODE_LINE (arg_node), ("upper bound cannot be infered"));
    } else
        right_type = MakeType (T_int, KNOWN_DIM_OFFSET - 1, NULL, NULL, NULL);

    if (NGEN_STEP (gen)) {
        step_type = TI (NGEN_STEP (gen), arg_info);
        if (!step_type)
            ERROR (NODE_LINE (arg_node), ("step cannot be infered"));
    }
    if (NGEN_WIDTH (gen)) {
        width_type = TI (NGEN_WIDTH (gen), arg_info);
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

    gen_type = (concrete_type) ? DuplicateTypes (*concrete_type, 1) : NULL;
    if (gen_type && TYPES_NAME (gen_type))
        FREE (TYPES_NAME (gen_type));
    if (gen_type && TYPES_MOD (gen_type))
        FREE (TYPES_MOD (gen_type));

    /* The index variable has to be either an identifier or an array of
       identifiers. If the  SAC-user only specifies one of it, the other
       part is generated here so that all generators have both for later
       use. (id (vector) and array of ids (scalars)).

       Index scalars can only be created if we have a concrete_type */

    withid = NPART_WITHID (arg_node);
    /* if we have no index vector, we craete one */
    if (!NWITHID_VEC (withid))
        NWITHID_VEC (withid) = MakeIds (TmpVar (), NULL, ST_regular);
    /* if we have no array of scalars, we create gen_type->shpseg[0] ids. */
    if (!NWITHID_IDS (withid) && gen_type) {
        _ids = MakeIds (TmpVar (), NULL, ST_regular);
        NWITHID_IDS (withid) = _ids;
        for (i = TYPES_SHAPE (gen_type, 0) - 1; i > 0; i--) {
            IDS_NEXT (_ids) = MakeIds (TmpVar (), NULL, ST_regular);
            _ids = IDS_NEXT (_ids);
        }
    }

    /* now we have to care about the ids.
       Do they have the right type? ALL index vectors and scalars are always
       given new names in flatten. So these names cannot be declared and
       we do not have to check if they already have been typed.
       First the index vector.*/
    if (gen_type) {
        /* strong TC */
        AddIdToStack (NWITHID_VEC (withid), gen_type, arg_info, NODE_LINE (arg_node));
    } else {
        /* weak TC */
        /* declare index vector as one-dimensional type.  */
        tmpt = MakeType (T_int, KNOWN_DIM_OFFSET - 1, NULL, NULL, NULL);
        AddIdToStack (NWITHID_VEC (withid), tmpt, arg_info, NODE_LINE (arg_node));
        FreeOneTypes (tmpt);
    }

    /* ...and then we check the array of scalars (int). All Id are
       introduced in flatten (or TC) like the index vector. */
    _ids = NWITHID_IDS (withid);
    tmpt = MakeType (T_int, 0, NULL, NULL, NULL);
    i = 0;
    while (_ids) {
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
        gen_type = MakeType (T_int, KNOWN_DIM_OFFSET - 1, NULL, NULL, NULL);

    if (SAC_PRG == kind_of_file && SCALAR >= TYPES_DIM (gen_type))
        ABORT (NODE_LINE (arg_node), ("No constant type for generator inferable"));

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

types *
TI_Ngenarray (node *arg_node, node *arg_info, node **replace)
{
    types *ret_type, *expr_type;
    int *tmpi, i, dim = 0;
    node *tmpn;

    DBUG_ENTER ("TI_Ngenarray");

    expr_type = TI (arg_node, NULL);
    if (expr_type == NULL) {
        ABORT (NODE_LINE (arg_node),
               ("type of shape in genarray with loop cannot be infered!"));
    }
    if (((NODE_TYPE (arg_node) != N_id) && (NODE_TYPE (arg_node) != N_array))
        || ((NODE_TYPE (arg_node) == N_id) && (ID_ARRAYLENGTH (arg_node) == SCALAR))) {
        /* weak TC */
        /* 1 dimension wanted */
        if (((TYPES_DIM (expr_type) < SCALAR)
             && (TYPES_DIM (expr_type) < KNOWN_DIM_OFFSET)
             && (TYPES_DIM (expr_type) != (KNOWN_DIM_OFFSET - 1)))
            || ((TYPES_DIM (expr_type) >= SCALAR) && (TYPES_DIM (expr_type) != 1)))
            ABORT (NODE_LINE (arg_node), ("genarray shape has to have one dimension"));

        /*
         * if the dimension of the genarray-vector is unkown or has one dimension,
         * we try to constantfold the expression to reduce it to a const array.
         * This is only possible if we have a prf shape(). Else the integration
         * or CF would be too complex (masks, mrd, name clashed when including
         * optimize.h to patch masks and mrd).
         */
        if (N_prf == NODE_TYPE (arg_node)) {
            tmpn = ReduceGenarrayShape (arg_node, expr_type);
            if (tmpn) {
                arg_node = tmpn;
                /*
                 * replace old expr in WL-syntax tree
                 */
                *replace = arg_node;
            }
        }
    }

    if (NODE_TYPE (arg_node) == N_array) {
        /*
         * infer type of N_array and store it in N_array-node
         */
        ARRAY_TYPE (arg_node) = expr_type;

        /*
         * compute return_type
         */
        ret_type = MakeType (T_int, 0, MakeShpseg (NULL), NULL, NULL);

        tmpn = ARRAY_AELEMS (arg_node);
        while (tmpn)
            if (dim < SHP_SEG_SIZE)
                if (N_num == NODE_TYPE (EXPRS_EXPR (tmpn))) {
                    TYPES_SHAPE (ret_type, dim) = NUM_VAL (EXPRS_EXPR (tmpn));
                    tmpn = EXPRS_NEXT (tmpn);
                    dim++;
                } else {
                    ABORT (NODE_LINE (arg_node),
                           ("%d. element of shape vector not a constant integer",
                            dim + 1));
                }
            else {
                ABORT (NODE_LINE (arg_node), ("Shape vector has too many elements"));
            }

        TYPES_DIM (ret_type) = dim;
        /*
         * don't free exprs_next because it is assignd to arg_node.
         */
    } else {
        if ((NODE_TYPE (arg_node) == N_id) && (ID_ARRAYLENGTH (arg_node) > SCALAR)) {

            /*
             * compute return_type
             */
            ret_type = MakeType (T_int, 0, MakeShpseg (NULL), NULL, NULL);

            tmpi = ID_CONSTARRAY (arg_node);
            dim = ID_ARRAYLENGTH (arg_node);
            if (dim < SHP_SEG_SIZE)
                for (i = 0; i < dim; i++)
                    TYPES_SHAPE (ret_type, dim) = tmpi[i];
            else
                ABORT (NODE_LINE (arg_node), ("Shape vector has too many elements"));
            TYPES_DIM (ret_type) = dim;
        } else {
            /* weak TC */
            /* return int[] or float[] ... */
            ret_type
              = MakeType (TYPES_BASETYPE (expr_type), UNKNOWN_SHAPE, NULL, NULL, NULL);
            FreeOneTypes (expr_type);
        }
    }

    if ((kind_of_file == SAC_PRG) && (TYPES_DIM (ret_type) <= SCALAR))
        ABORT (NODE_LINE (arg_node), ("No constant type for genarray shape inferable"));

    DBUG_RETURN (ret_type);
}

/******************************************************************************
 *
 * function:
 *   void ConsistencyCheckModarray(types *array_type, types *generator_type, types
 **body_type)
 *
 * description:
 *   additional checks for modarray operator
 *
 *
 ******************************************************************************/

void
ConsistencyCheckModarray (int lineno, types *array_type, types *generator_type,
                          types *body_type)
{
    int length, i;
    DBUG_ENTER (" ConsistencyCheckModarray");

    if (TYPES_BASETYPE (array_type) != TYPES_BASETYPE (body_type))
        ABORT (lineno, ("Basetypes of 'modarray` (%s) and return_type (%s) different",
                        Type2String (array_type, 0), Type2String (body_type, 0)));

    /* strong TC */
    if (SCALAR < TYPES_DIM (array_type) && SCALAR < TYPES_DIM (generator_type)
        && SCALAR <= TYPES_DIM (body_type)) {
        length = TYPES_SHAPE (generator_type, 0);
        if (length + TYPES_DIM (body_type) == TYPES_DIM (array_type))
            for (i = length; i < TYPES_DIM (array_type); i++) {
                if (TYPES_SHAPE (array_type, i) != TYPES_SHAPE (body_type, i - length))
                    ABORT (lineno,
                           ("Type of shape vector (%s) does not match "
                            "type of index vector (%s) and return statement (%s)",
                            Type2String (array_type, 0), Type2String (generator_type, 0),
                            Type2String (body_type, 0)));
            }
        else
            ABORT (lineno, ("Type of shape vector (%s) does not match "
                            "type of index vector (%s) and return statement (%s)",
                            Type2String (array_type, 0), Type2String (generator_type, 0),
                            Type2String (body_type, 0)));
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

types *
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

    if (prf_tag == NWITHOP_PRF (arg_node)) {
        /* it is a primitive function */
        prf_p = (prim_fun_tab_elem *)fun_p;
        type_c_tag = prf_p->node->info.prf_dec.tag;
        NWITHOP_PRF (arg_node) = prf_p->new_prf;
        /* now compute resulting type (ret_type) */
        switch (type_c_tag) {
#include "prim_fun_tt.mac" /* assigns ret_type */
        default:
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
               != CompatibleTypes (body_type, ret_type, -1, NODE_LINE (arg_node)))
        if (WO_foldfun == NWITHOP_TYPE (arg_node)) {
            ABORT (NODE_LINE (arg_node),
                   ("Function '%s` has return type '%s` != '%s`",
                    ModName (NWITHOP_MOD (arg_node), NWITHOP_FUN (arg_node)),
                    Type2String (ret_type, 0), Type2String (body_type, 0)));
        } else
            ABORT (NODE_LINE (arg_node),
                   ("Function '%s` has return type '%s` != '%s`",
                    prf_string[arg_node->info.prf], Type2String (ret_type, 0),
                    Type2String (body_type, 0)));

    if (NWITHOP_NEUTRAL (arg_node)) {
        if (CMP_equal != CmpTypes (neutral_type, ret_type)) /* 1 */ {
            ABORT (NODE_LINE (arg_node),
                   ("neutral element of 'fold` has wrong type '%s`. Should be '%s`",
                    Type2String (neutral_type, 0), Type2String (ret_type, 0)));
        }
    } else
        NWITHOP_NEUTRAL (arg_node) = ComputeNeutralElem (old_prf, body_type);

    FREE (arg_type);
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

types *
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
                    Type2String (ret_type, 0), Type2String (body_type, 0)));
        }
        if (CMP_equal != CmpTypes (neutral_type, body_type)) {
            ABORT (NODE_LINE (arg_node),
                   ("Neutral element of 'fold` has wrong type '%s`. Should be '%s`",
                    Type2String (neutral_type, 0), Type2String (ret_type, 0)));
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
    arg_info->node[2]->info.types = TI (NCODE_CEXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
