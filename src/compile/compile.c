/*
 *
 * $Log$
 * Revision 1.104  1997/11/22 23:07:52  dkr
 * removed a bug in CompPrf() with F_{add,sub,...}_AxA, arg{1,2}->refcnt==-1
 *   - previous N_let replaced by ND_ALLOC_ARRAY (reuse)
 *   - old_arg_node now correctly set (-> FREE(old_arg_node) !!)
 *
 * Revision 1.103  1997/11/11 00:43:13  dkr
 * removed a bug with NEWTREE in CompTypedef
 *
 * Revision 1.102  1997/11/10 16:09:47  dkr
 * removed a bug with DBUG_ASSERT in GOTO_LAST_BUT_LEAST_N_EXPRS (with defined NEWTREE)
 *
 * Revision 1.101  1997/11/07 14:48:25  dkr
 * removed a bug with nnode-elimiation
 *
 * Revision 1.100  1997/11/05 09:39:34  dkr
 * CompCond() now uses the new array nnode[]
 *
 * Revision 1.99  1997/11/03 16:22:14  dkr
 * removed a bug with the nnode-elimination
 *
 * Revision 1.98  1997/11/02 13:57:57  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.97  1997/11/02 13:01:23  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.96  1997/10/29 14:11:15  srs
 * removed HAVE_MALLOC_O
 *
 * Revision 1.95  1997/10/13 21:01:38  dkr
 * removed a bug in CompWithReturn() with nnode.
 *
 * Revision 1.94  1997/10/12 19:29:37  dkr
 * In RenameReturn(): changed the 3rd-argument of MakeId, MakeIds from NULL to 0;
 * in general NULL is NOT compatible with enum-type
 * (e.g. '#define NULL (void *)0').
 *
 * Revision 1.93  1997/10/09 13:58:56  srs
 * modified CompPrf() to compile F_min and F_max
 *
 * Revision 1.92  1997/08/29 09:58:41  sbs
 * Compilation of F_cat changed.
 * the refcnt of the result is set at the array allocation only.
 * No more INC_RC after copying the elements!
 *
 * Revision 1.91  1997/05/29 13:41:44  sbs
 * ND_IDX_MODARRAY... added
 *
 * Revision 1.90  1997/05/02  09:30:51  cg
 * CompVardec: Variable declarations are now produced for arrays with unknown
 * shape and dimension as well as arrays with known dimension but unknown shape.
 *
 * Revision 1.89  1997/04/25  09:25:44  sbs
 * dummy-decls in CompConvert and CompPrf commented out in case
 * of NOFREE.
 *
 * Revision 1.88  1997/04/24  14:57:25  sbs
 * HAVE_MALLOC_O included
 *
 * Revision 1.87  1996/09/11  06:27:38  cg
 * Some very dirty bugs fixed.
 *
 * Revision 1.86  1996/08/29  17:45:27  sbs
 * res_stype in CmpPrf was not initialised at all!!
 *
 * Revision 1.85  1996/08/04  14:40:12  sbs
 * modarray_AxVxA with and without reuse check inserted.
 *
 * Revision 1.84  1996/08/01  09:53:18  cg
 * bug fixed in compiling applications of functions with a
 * variable number of return values.
 *
 * Revision 1.83  1996/05/29  16:47:48  sbs
 * -Inserted tree-macros in Compile, CompBlock, CompAssign
 *
 * expanded macros CHECK_REUSE__ALLOC_ARRAY_ND and DEC_OR_FREE_RC_ND
 * for the cases when refcnt == -1. This should only happen iff opt_rco == 1
 * (default). If opt_rco == 1, then refcnt for all ids in arg-pos of
 * a prf should be 1 or -1; nothing else!!
 *
 * changed some direct calls of DEC_RC_FREE_ND in DEC_OR_FREE_RC_ND...
 *
 * Revision 1.82  1996/03/12  17:01:48  hw
 * bug fixed in function AddVardec (now using macro APPEND_VARDECS )
 *
 * Revision 1.81  1996/03/05  15:31:22  cg
 * bug fixed in handling of functions with variable argument list
 *
 * Revision 1.80  1996/03/05  09:27:42  cg
 * bug fixed in CompAp
 *
 * Revision 1.79  1996/02/27  15:22:04  hw
 * bug fixed in adding new vardecs
 *
 * Revision 1.78  1996/02/12  16:37:44  cg
 * bug fixed in usage of pragma refcounting in CompArg,
 * macro DEC_OR_FREE_RC_ND now used with correct refcount information
 * in primitive functions shape and dim,
 *
 * Revision 1.77  1996/02/06  18:59:17  cg
 * fixed bug in CompId
 *
 * Revision 1.76  1996/01/26  15:34:29  cg
 * CompId now inserts the right icms to copy arays and hidden objects
 * where necessary at the positions of former class conversion functions
 *
 * Revision 1.75  1996/01/25  15:00:00  cg
 * renamed some icm macros
 *
 * Revision 1.74  1996/01/23  09:02:25  cg
 * Now, the init expressions of object definitions are always removed.
 *
 * Revision 1.73  1996/01/22  17:33:22  cg
 * IsBoxed and IsUnique moved to refcount.c
 *
 * Revision 1.72  1996/01/21  14:01:03  cg
 * implemented refcounting of external implicit types, functions
 * which do not do the refcounting on their own, pragmas refcounting
 * and linksign.
 *
 * Revision 1.71  1995/12/30  17:01:23  cg
 *  removed external declaration of 'filename'
 *
 * Revision 1.70  1995/12/21  15:09:15  cg
 * converted from nums to array representation of pragma linksign.
 *
 * Revision 1.69  1995/12/18  18:28:55  cg
 * compilation of objdef nodes modified, now ICMs for global arrays
 * will be printed correctly.
 *
 * Revision 1.68  1995/12/18  16:21:18  cg
 * major modifications in CompFundef, CompAp and CompReturn.
 * new function ReorganizeParameters to rearrange icm args in function
 * definitions, applications, and return-statements.
 * parameter tags changed: in_a -> in_rc, etc.
 * because arrays are no longer the only refcounted data objects.
 * ATTENTION: some ICMs are renamed, check with icm2c.[ch]
 * void functions and reference parameters are now supported (no arrays)
 *
 * Revision 1.67  1995/12/04  16:08:19  hw
 * added compilation of type-converting-functions F_toi ...
 * added function CompConvert
 *
 * Revision 1.66  1995/12/01  17:22:16  cg
 * moved renaming of functions to precompiler
 * moved generation of additional extern declarations for mutual
 * recursive functions to print.c
 *
 * Revision 1.65  1995/11/01  07:25:06  sbs
 * dirty hack: sharing info.types in forward declarations
 * in Compile !
 *
 * Revision 1.64  1995/10/08  11:22:36  sbs
 * some bugs fixed
 *
 * Revision 1.63  1995/09/06  16:13:35  hw
 * changed compilation of idx_psi
 *
 * Revision 1.62  1995/09/05  15:21:34  hw
 * bug fixed in compilation of "idx_psi"
 * (changed condition of DBUG_ASSERT)
 *
 * Revision 1.61  1995/08/15  14:34:52  hw
 * - enlarged DBUG_ASSERT in compilation of idx_psi
 * - added compilation of modarray, but only following cases:
 *   modarry(id, vec, id)
 *
 * Revision 1.60  1995/07/24  09:16:02  hw
 * changed test for SAC-function "main" in function Compile
 *
 * Revision 1.59  1995/07/19  12:19:16  hw
 * changed RenameFunName( the name of a userdefined "external" primitive
 *  function will be set to the one specified in the module or
 *  class declartion ( it is stored in node[5] of N_fundef))
 *
 * Revision 1.58  1995/07/14  12:05:37  hw
 * - changed macro CHECK_REUSE__ALLOC_ARRAY_ND( reuse only if ol and new array
 *   have equal basic-simpletypes)
 *
 * Revision 1.57  1995/07/13  16:26:13  hw
 * - changed compilation of N_foldprf & N_foldfun
 * - changed secound argument of GET_LENGTH
 * - moved compilation of a with-loop-N_return to new function
 *   CompWithReturn
 * - changed CompFundec & CompArg to compile function declarations
 *   without formal paramter names( only imported functions)
 *
 * Revision 1.56  1995/07/04  09:24:06  hw
 * compilation of primitive functions itod, ftod, dtoi & dtof inserted
 *
 * Revision 1.55  1995/06/30  12:14:14  hw
 * - renamed macro GET_BASIC_TYPE to GET_BASIC_SIMPLETYPE
 * - changed function CompVardec
 * - compilation of primitive functions itof & ftoi inserted
 *
 * Revision 1.54  1995/06/26  14:23:47  hw
 * - deleted some unused variables
 * - remove #if 0 .. #endif parts
 *
 * Revision 1.53  1995/06/26  14:05:51  hw
 * moved some macros to compile.h and tree.h
 *
 * Revision 1.52  1995/06/26  12:01:37  hw
 * compilation of idx_psi added
 *
 * Revision 1.51  1995/06/26  09:27:07  sbs
 * insertion of idx_psi initiated :->
 *
 * Revision 1.50  1995/06/23  13:13:53  hw
 * - functions will be renamed now, so overloading of userdefined
 *   functions can be done.
 *   ( new function RenameFunName inserted)
 * -  added argument to call of 'DuplicateTypes'
 *
 * Revision 1.49  1995/06/16  15:29:11  hw
 * bug fixed in CompVardec( rmoveing of vardec-nodes)
 *
 * Revision 1.48  1995/06/14  15:32:25  hw
 * changed Compile( "extern"-declaration for each not imported function inserted)
 *
 * Revision 1.47  1995/06/14  14:24:48  hw
 * bug fixed in CompPrf ( new variable declaration will be inserted
 *  now in all cases )
 *
 * Revision 1.46  1995/06/14  12:41:55  hw
 * - N_cast nodes in argument position of primitive functions
 *   will be deleted
 * - bug fixed in creation of ND_KD_PSI_VxA_S & ND_KD_PSI_CxA_S
 * - renamed macro CONST_ARRAY to CREATE_TMP_CONST_ARRAY and
 *   changed it (removed creation of ND_REUSE)
 *
 * Revision 1.45  1995/06/13  15:11:08  hw
 * - changed RenameVar (added new parameter)
 * - added function RenameReturn
 * inserted renameing of varaibles of a return_statement  and things
 * beloning to it
 * - cast in a return_statement will be deleted
 *
 * Revision 1.44  1995/06/12  09:19:17  hw
 * bug fixed in CompVardec ( arg_node->nnode is set correctly)
 *
 * Revision 1.43  1995/06/09  17:35:53  hw
 * -bug fixed in CompLoop (look whether next there is a assignment ater loop)
 *
 * Revision 1.42  1995/06/09  15:30:42  hw
 * changed N_icms ND_KD_PSI_... (linenumber inserted)
 *
 * Revision 1.41  1995/06/08  17:48:57  hw
 * CompTypedef inserted
 *
 * Revision 1.40  1995/06/07  13:33:32  hw
 * exchanges N_icm ND_CREATE_CONST_ARRAY with ND_CREATE_CONST_ARRAY_S
 * N_icm ND_CREATE_CONST_ARRAY_A (array out of arrays) inserted
 *
 * Revision 1.39  1995/06/06  15:54:30  hw
 * changed CompVardec ( a declaration of an array with unknown shape
 *   will be removed )
 *
 * Revision 1.38  1995/06/06  11:39:02  hw
 * changed CompPrf (F_take/F_drop) (first argument can be a scalar too, now)
 *
 * Revision 1.37  1995/06/02  08:43:34  hw
 * - renamed N_icm ND_END_FOLDPRF to ND_END_FOLD
 * - compilation of N_foldfun inserted
 *
 * Revision 1.36  1995/05/24  15:28:37  hw
 * bug fixed in creation of N_icm ND_END_MODARRAY_S\A ( last arg is
 *  a N_exprs node now ;-)) )
 *
 * Revision 1.35  1995/05/24  13:45:24  hw
 * - changed args of N_icm FUN_DEC ( back to rev 1.33 )
 *   and added N_icm ND_KS_DECL_ARRAY_ARG instead ( this N_icm will be put
 *    at beginning of the block of a function)
 *    ( changes are done in CompArg only)
 * - added compilation of "fold_prf", but it does not work correctly :-((
 *   there has to be done some work on it
 *
 * Revision 1.34  1995/05/22  16:51:27  hw
 * - deleted some Check_reuse N_icms
 * - changed args of FUN_DEC N_icm
 *
 * Revision 1.33  1995/05/22  10:10:46  hw
 * - added function "CompCast" to delete N_cast nodes
 *  - remove N_cast nodes while compilation in functions ( CompReturn,
 *    CompPrf, CompAp )
 *
 * Revision 1.32  1995/05/19  13:33:30  hw
 * - bug fixed in CompPrf ( added ND_DEC_... while compilation of F_psi (case:
 *     ND_KD_PSI_VxA_S))
 * - bug fixed in CompAp (refcounts of arrays which are returned from a function
 *     will be set and inserted correctly)
 * - changed CompReturn ( if return contains to a with_loop, following refcounts
 *    will be decremented: -index_vector, left and right border,
 *                           array of modarray-part, all used arrays
 * - bug fixed in CompLoop ( - create only ND_DEC_RC_FREE macros
 *                           - put ND_LABEL(..) at the right place (after DEC's
 *                              but before INC's )
 *
 * Revision 1.31  1995/05/11  08:25:16  hw
 * changed setting of refcount at beginning of userdefined functions
 *
 * Revision 1.30  1995/05/10  13:54:38  hw
 * changed increasing of refcounts of used variables in loops
 *
 * Revision 1.29  1995/05/09  16:39:35  hw
 * bug fixed in CompAssign ( arg_info->node[0] will be set correctly now )
 *
 * Revision 1.28  1995/05/08  15:45:53  hw
 * added CompBlock
 * compilation (renaming) of expressions like `a=a+a` added
 *
 * Revision 1.27  1995/05/04  14:59:46  hw
 * -changed compilation of 'dim' & 'shape' (refcount will be decremented now)
 * - changed parameters of N_icm ND_REUSE
 * - changed setting of refcount behind a loop
 * - changed setting of refcount at beginning of a function
 *
 * Revision 1.26  1995/05/03  13:20:52  hw
 * - deleted ND_KD_ROT_SxSxA_A
 * - bug fixed in compilation of primitive function 'rotate'
 *   ( added N_icm ND_ALLOC_ARRAY for result of 'rotate' )
 * - refcounts of results of userdefined functions will be increased
 *
 * Revision 1.25  1995/05/02  10:11:52  hw
 *  bug fixed in CompReturn ( ND_END_MODARRAY and  ND_END_GENARRAY have correct
 *   parameters now)
 *
 * Revision 1.24  1995/05/02  09:32:58  hw
 * bug fixed in CompReturn
 *
 * Revision 1.23  1995/04/28  17:29:30  hw
 * - changed compilation of with_loop:
 *   - N_icm for allocation of index_vector added
 *   - refcounts of  used before defined variables (only arrays) of a
 *     with-loop will be incremented
 *   - refcount of index_vector will be decremented after with_loop
 *
 * Revision 1.22  1995/04/28  09:02:38  hw
 * bug fixed in creation of N_icm ND_BEGIN_MODARRAY ( now 3. argument is set
 *  correctly)
 *
 * Revision 1.21  1995/04/27  14:44:10  hw
 * bug fixed in CompWith (renamed N_icm for genarray)
 *
 * Revision 1.20  1995/04/27  14:24:48  hw
 * added compilation of with-loop (modarray and genarray)
 *
 * Revision 1.19  1995/04/24  18:08:06  hw
 * - renamed CompWhile to CompLoop
 * -compilation of N_do done
 * - refcount after KS_ASSIGN_ARRAY is set corectly
 *
 * Revision 1.18  1995/04/24  14:18:53  hw
 * - CompWhile & CompCond inserted
 * - changed refcount-parameter of ND_ALLOC_ARRAY if it is used with
 *   ND_CHECK_REUSE
 *
 * Revision 1.17  1995/04/19  14:30:32  hw
 * bug fixed in compilation of 'rotate'
 *
 * Revision 1.16  1995/04/19  14:13:18  hw
 * changed compilation of primitive function 'rotate'
 *  ( 3. argument of `rotate' will not be reused )
 *
 * Revision 1.15  1995/04/19  13:50:47  hw
 * changed arguments of N_icm ND_KD_PSI_CxA_S &  ND_KD_PSI_VxA_S
 *
 * Revision 1.14  1995/04/19  13:08:28  hw
 * bug fixed in compilation of primitive function 'cat'
 *
 * Revision 1.13  1995/04/19  12:51:59  hw
 * changed parameters of N_icm ND_ALLOC_ARRAY & ND_CREATE_CONST_ARRAY
 *
 * Revision 1.12  1995/04/13  17:21:28  hw
 * compilation of primitive function 'cat' completed
 * compilation of primitive function 'rotate' inserted
 *
 * Revision 1.11  1995/04/13  10:04:09  hw
 * compilation of primitive function 'cat' inserted, but not completly
 *
 * Revision 1.10  1995/04/11  15:11:55  hw
 * compilation of N_fundef, N_ap, N_return done
 *
 * Revision 1.9  1995/04/07  11:30:18  hw
 * changed N_icm name ND_KS_ASSIGN -> ND_KS_ASSIGN_ARRAY
 *
 * Revision 1.8  1995/04/07  10:01:29  hw
 * - added one argument to N_icm of primitive functions take, drop and psi
 * - changed compilation of primitive function reshape
 *
 * Revision 1.7  1995/04/06  08:48:58  hw
 * compilation of F_dim and F_shape added
 *
 * Revision 1.6  1995/04/05  15:26:33  hw
 * F_..AxA_A will be compiled now
 *
 * Revision 1.5  1995/04/04  08:31:30  hw
 * changed CompFundef
 *
 * Revision 1.4  1995/04/03  16:35:29  hw
 * - CompFundef inserted
 * - changed CompArg (ICM for increasing refcounts inserted)
 * - F_psi will work on constant arrays
 *
 * Revision 1.3  1995/04/03  13:27:29  hw
 * bug fixed in creation of N_icm ND_CREATE_CONST_ARRAY.
 * primitive functions F_...SxA, F_..AxS, F_take can have constant
 *    arrays as argument
 *
 * Revision 1.2  1995/03/31  15:49:16  hw
 * added macros to create and insert 'N_icm'
 * added CompArray, CompId, CompReturn
 * changed CompPrf (now more functions will be compiled)
 *
 * Revision 1.1  1995/03/29  12:38:10  hw
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "print.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"
#include "Error.h"
#include "traverse.h"
#include "compile.h"
#include "convert.h"
#include "refcount.h"
#include "typecheck.h" /* to use LookupType */
#include "free.h"

extern int CmpTypes (types *type_one, types *type_two);
/* from typecheck.c because typecheck.h is currently locked */

#define TYP_IF(n, d, p, f, sz) sz

int basetype_size[] = {
#include "type_info.mac"
};

#undef TYP_IF

/*
 *  The following macros are originally from access_macros.h
 *  They are copied to allow their incremtal replacement by the new
 *  macros from tree_basic.h and tree_compound.h
 *  If there's no occurrence of one or the other old macro left
 *  within this file it should be removed.
 */

/* to avoid warnings in connection with new virtual syntax tree */
#undef IDS_DEF
#undef ID_MOD
#undef IDS_VARNO

/* macros for access to elements of struct info.types */
#define TYPES info.types
#define SIMPLETYPE TYPES->simpletype
#define DIM TYPES->dim
#define ID TYPES->id
#define ID_MOD TYPES->id_mod
#define SHP TYPES->shpseg->shp
#define NAME TYPES->name
#define NAME_MOD TYPES->name_mod
#define ATTRIB TYPES->attrib
#define STATUS TYPES->status

/* macros used for N_ap nodes to get the function's name */
#define FUN_NAME info.fun_name.id
#define FUN_MOD_NAME info.fun_name.id_mod

/* macros for access to elements of struct info.ids */
#define IDS info.ids
#define IDS_ID IDS->id
#define IDS_NODE IDS->node
#define IDS_DEF IDS->def
#define IDS_VARNO IDS->node->varno
/* #define IDS_REFCNT IDS->refcnt */
/* #define IDS_NEXT   IDS->next   */
#define IDS_ATTR IDS->attrib
#define IDS_STAT IDS->status

/* macros for access arguments of a ap or prf - node */
#define ARG1 node[0]->node[0]
#define ARG2 node[0]->node[1]->node[0]
#define ARG3 node[0]->node[1]->node[1]->node[0]

/*
 *  end of old macros from access_macros.h
 */

#define DUMMY_NAME "__OUT_"
#define LABEL_NAME "__Label" /* basic-name for goto label */

#ifndef DBUG_OFF

#define MDB_VAR(var) var
#define MDB_DECLARE(type, var) type var
#define MDB_ASSIGN(var, value) var = value

#else

#define MDB_VAR(var)
#define MDB_DECLARE(type, var)
#define MDB_ASSIGN(var, value)

#endif /* DBUG_OFF */

/* the following macros are while generation of N_icms */

#define MAKE_IDNODE(id) MakeId (id, NULL, ST_regular);

#define MUST_REFCOUNT(item, arg) (item##_REFCNT (arg) >= 0)

#define TYPE_REFCNT(type) (IsArray (type) + IsNonUniqueHidden (type) - 1)

#define FUN_DOES_REFCOUNT(fundef, i)                                                     \
    (FUNDEF_STATUS (fundef) != ST_Cfun)                                                  \
      ? 1                                                                                \
      : (FUNDEF_PRAGMA (fundef) == NULL)                                                 \
          ? 0                                                                            \
          : (FUNDEF_REFCOUNTING (fundef) == NULL)                                        \
              ? 0                                                                        \
              : (i >= PRAGMA_NUMPARAMS (FUNDEF_PRAGMA (fundef)))                         \
                  ? 0                                                                    \
                  : FUNDEF_REFCOUNTING (fundef)[i]

#define ICMPARAM_TAG(expr) ID_NAME (EXPRS_EXPR (expr))
#define ICMPARAM_ARG1(expr) EXPRS_EXPR (EXPRS_NEXT (expr))
#define ICMPARAM_ARG2(expr) EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (expr)))

#define SHIFT_EXPRS_PTR(var, num)                                                        \
    {                                                                                    \
        int i;                                                                           \
        for (i = 0; i < num; i++)                                                        \
            var = EXPRS_NEXT (var);                                                      \
    }

#define EQUAL_SIMPLETYPES(stype1, stype2) (stype1 == stype2)

#define SET_VARS_FOR_MORE_ICMS                                                           \
    first_assign = LAST_ASSIGN (arg_info);                                               \
    old_arg_node = arg_node;                                                             \
    last_assign = NEXT_ASSIGN (arg_info);                                                \
    arg_node = arg_info->node[1]->node[0]

#define INSERT_ASSIGN                                                                    \
    if (NULL != last_assign) {                                                           \
        DBUG_PRINT ("COMP", ("last :" P_FORMAT "(%s " P_FORMAT ")", last_assign,         \
                             mdb_nodetype[last_assign->node[0]->nodetype],               \
                             last_assign->node[0]));                                     \
        DBUG_PRINT ("COMP", ("first: " P_FORMAT "(%s " P_FORMAT ")", first_assign,       \
                             mdb_nodetype[first_assign->node[0]->nodetype],              \
                             first_assign->node[0]));                                    \
        APPEND_ASSIGNS (first_assign, last_assign);                                      \
    }

#define CHECK_REUSE(test)                                                                \
    BIN_ICM_REUSE (arg_info->node[1], "ND_CHECK_REUSE_ARRAY", test, res);                \
    SET_VARS_FOR_MORE_ICMS

#define CHECK_REUSE__ALLOC_ARRAY_ND(new, stype_new, old, stype_old)                      \
    if ((1 == old->refcnt)                                                               \
        && (EQUAL_SIMPLETYPES (stype_new,                                                \
                               stype_old))) { /* create ND_CHECK_REUSE_ARRAY  */         \
        node *num;                                                                       \
        BIN_ICM_REUSE (arg_info->node[1], "ND_CHECK_REUSE_ARRAY", old, new);             \
        first_assign = LAST_ASSIGN (arg_info);                                           \
        DBUG_PRINT ("COMP", ("first:" P_FORMAT, first_assign));                          \
        old_arg_node = arg_node;                                                         \
        arg_node = arg_info->node[1]->node[0];                                           \
        /* create ND_ALLOC_ARRAY */                                                      \
        MAKENODE_NUM (num, 0);                                                           \
        CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, new, num);        \
        APPEND_ASSIGNS (first_assign, next_assign);                                      \
    } else { /* create ND_ALLOC_ARRAY */                                                 \
        node *num;                                                                       \
        DBUG_ASSERT (((old->refcnt == -1) || (opt_rco == 0)),                            \
                     "Refcnt of prf-arg neither -1 nor 1 !");                            \
        MAKENODE_NUM (num, 0);                                                           \
        BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, new);          \
        MAKE_NEXT_ICM_ARG (icm_arg, num);                                                \
        first_assign = LAST_ASSIGN (arg_info);                                           \
        DBUG_PRINT ("COMP", ("first:" P_FORMAT, first_assign));                          \
        old_arg_node = arg_node;                                                         \
        arg_node = arg_info->node[1]->node[0];                                           \
    }

#define DEC_OR_FREE_RC_ND(array, num_node)                                               \
    if (1 < array->refcnt) { /* create ND_DEC_RC */                                      \
        DEC_RC_ND (array, num_node);                                                     \
    } else {                                                                             \
        if (array->refcnt > 0) {                                                         \
            DEC_RC_FREE_ND (array, num_node);                                            \
        } else {                                                                         \
        }                                                                                \
    }

#define DEC_RC_ND(array, num_node)                                                       \
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC", array, num_node);                        \
    APPEND_ASSIGNS (first_assign, next_assign)

#define DEC_RC_FREE_ND(array, num_node) /* create ND_DEC_RC_FREE_ARRAY */                \
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY", array, num_node);             \
    APPEND_ASSIGNS (first_assign, next_assign)

#define INC_RC_ND(array, num_node) /* create ND_INC_RC */                                \
    CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", array, num_node);                        \
    APPEND_ASSIGNS (first_assign, next_assign)

#define SET_RC_ND(array, num_node) /* create ND_SET_RC */                                \
    CREATE_2_ARY_ICM (next_assign, "ND_SET_RC", array, num_node);                        \
    APPEND_ASSIGNS (first_assign, next_assign)

#define CREATE_TMP_CONST_ARRAY(array, rc)                                                \
    array_is_const = 1;                                                                  \
    old_arg_node = arg_node;                                                             \
    tmp = array->node[0];                                                                \
    n_elems = 0;                                                                         \
    do {                                                                                 \
        n_elems += 1;                                                                    \
        tmp = tmp->node[1];                                                              \
    } while (NULL != tmp);                                                               \
    MAKENODE_NUM (n_node, n_elems);                                                      \
    MAKENODE_NUM (n_node1, 1);                                                           \
    MAKENODE_ID (tmp_array1, "__TMP");                                                   \
    arg_info->node[1]->nodetype = N_block; /* reuse previous N_let*/                     \
    CREATE_4_ARY_ICM (first_assign, "ND_KS_DECL_ARRAY", type_id_node, tmp_array1,        \
                      n_node1, n_node);                                                  \
    arg_node = first_assign;                                                             \
    CREATE_CONST_ARRAY (array, tmp_array1, type_id_node, rc);                            \
    array = tmp_array1 /* set array to __TMP */

#ifndef NEWTREE
#define CREATE_CONST_ARRAY(array, name, type, rc)                                        \
    CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type, name, rc);                    \
    APPEND_ASSIGNS (first_assign, next_assign);                                          \
    COUNT_ELEMS (n_elems, array->node[0]);                                               \
    MAKENODE_NUM (n_node, n_elems);                                                      \
    CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", name, n_node);             \
    icm_arg->node[1] = array->node[0];                                                   \
    icm_arg->nnode = 2;                                                                  \
    APPEND_ASSIGNS (first_assign, next_assign)
#else /* NEWTREE */
#define CREATE_CONST_ARRAY(array, name, type, rc)                                        \
    CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type, name, rc);                    \
    APPEND_ASSIGNS (first_assign, next_assign);                                          \
    COUNT_ELEMS (n_elems, array->node[0]);                                               \
    MAKENODE_NUM (n_node, n_elems);                                                      \
    CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", name, n_node);             \
    icm_arg->node[1] = array->node[0];                                                   \
    APPEND_ASSIGNS (first_assign, next_assign)
#endif /* NEWTREE */

#define DECL_ARRAY(assign, Node, var_str, var_str_node)                                  \
    COUNT_ELEMS (n_elems, Node);                                                         \
    MAKENODE_NUM (n_elems_node, n_elems);                                                \
    MAKENODE_NUM (n_node1, 1);                                                           \
    MAKENODE_ID (var_str_node, var_str);                                                 \
    CREATE_4_ARY_ICM (assign, "ND_KS_DECL_ARRAY", type_id_node, var_str_node, n_node1,   \
                      n_elems_node)

#ifndef NEWTREE
#define INSERT_ID_NODE(no, last, str)                                                    \
    tmp = MakeNode (N_exprs);                                                            \
    MAKENODE_ID (tmp->node[0], str);                                                     \
    tmp->node[1] = no;                                                                   \
    tmp->nnode = 2;                                                                      \
    last->node[1] = tmp;                                                                 \
    last->nnode = 2
#else /* NEWTREE */
#define INSERT_ID_NODE(no, last, str)                                                    \
    tmp = MakeNode (N_exprs);                                                            \
    MAKENODE_ID (tmp->node[0], str);                                                     \
    tmp->node[1] = no;                                                                   \
    last->node[1] = tmp;
#endif /* NEWTREE */

/* following macros are used to compute last but one or next N_assign form
 * a 'arg_info' node
 */

#define NEXT_ASSIGN(arg_info)                                                            \
    (N_block == arg_info->node[0]->nodetype) ? arg_info->node[0]->node[0]->node[1]       \
                                             : arg_info->node[0]->node[1]->node[1]

#define LAST_ASSIGN(arg_info)                                                            \
    (N_block == arg_info->node[0]->nodetype) ? arg_info->node[0]->node[0]                \
                                             : arg_info->node[0]->node[1]

#define INSERT_BEFORE(arg_info, Node)                                                    \
    if (N_block == NODE_TYPE (INFO_LASTASSIGN (arg_info))) {                             \
        DBUG_PRINT ("COMP",                                                              \
                    ("insert node:" P_FORMAT " before:" P_FORMAT " after:" P_FORMAT,     \
                     Node, arg_info->node[0]->node[0], arg_info->node[0]));              \
        BLOCK_INSTR (INFO_LASTASSIGN (arg_info)) = Node;                                 \
    } else {                                                                             \
        DBUG_PRINT ("COMP",                                                              \
                    ("insert node:" P_FORMAT " before:" P_FORMAT " after:" P_FORMAT,     \
                     Node, arg_info->node[0]->node[1], arg_info->node[0]));              \
        ASSIGN_NEXT (INFO_LASTASSIGN (arg_info)) = Node;                                 \
    }

#define ELIMINATE_CAST(exprs)                                                            \
    if (N_cast == exprs->node[0]->nodetype) {                                            \
        tmp = exprs->node[0]->node[0];                                                   \
        FREE_TYPE (exprs->node[0]->TYPES);                                               \
        FREE (exprs->node[0]);                                                           \
        exprs->node[0] = tmp;                                                            \
    }

#ifndef NEWTREE
#define GOTO_LAST_N_EXPRS(exprs, last_exprs)                                             \
    last_exprs = exprs;                                                                  \
    while (2 == last_exprs->nnode) {                                                     \
        DBUG_ASSERT (N_exprs == last_exprs->nodetype, " nodetype  != N_expr");           \
        last_exprs = last_exprs->node[1];                                                \
    }
#else /* NEWTREE */
#define GOTO_LAST_N_EXPRS(exprs, last_exprs)                                             \
    last_exprs = exprs;                                                                  \
    while (last_exprs->node[1] != NULL) {                                                \
        DBUG_ASSERT (N_exprs == last_exprs->nodetype, " nodetype  != N_expr");           \
        last_exprs = last_exprs->node[1];                                                \
    }
#endif /* NEWTREE */

#ifndef NEWTREE
#define GOTO_LAST_BUT_LEAST_N_EXPRS(exprs, lbl_exprs)                                    \
    {                                                                                    \
        node *tmp;                                                                       \
        lbl_exprs = exprs;                                                               \
        DBUG_ASSERT (N_exprs == lbl_exprs->nodetype, " nodetype  != N_expr");            \
        DBUG_ASSERT (2 == lbl_exprs->nnode, "there is NO N_exprs-chain contains only"    \
                                            " one element");                             \
        tmp = lbl_exprs->node[1];                                                        \
        while (tmp->nnode == 2) {                                                        \
            DBUG_ASSERT (N_exprs == tmp->nodetype, " nodetype  != N_expr");              \
            lbl_exprs = tmp;                                                             \
            tmp = tmp->node[1];                                                          \
        }                                                                                \
    }
#else /* NEWTREE */
#define GOTO_LAST_BUT_LEAST_N_EXPRS(exprs, lbl_exprs)                                    \
    {                                                                                    \
        node *tmp;                                                                       \
        lbl_exprs = exprs;                                                               \
        DBUG_ASSERT (N_exprs == lbl_exprs->nodetype, " nodetype  != N_expr");            \
        DBUG_ASSERT (EXPRS_NEXT (lbl_exprs) != NULL,                                     \
                     "there is NO N_exprs-chain contains only"                           \
                     " one element");                                                    \
        tmp = lbl_exprs->node[1];                                                        \
        while (tmp->node[1] != NULL) {                                                   \
            DBUG_ASSERT (N_exprs == tmp->nodetype, " nodetype  != N_expr");              \
            lbl_exprs = tmp;                                                             \
            tmp = tmp->node[1];                                                          \
        }                                                                                \
    }
#endif /* NEWTREE */

#define FREE_TYPE(a)                                                                     \
    if (NULL != a->shpseg)                                                               \
        FREE (a->shpseg)                                                                 \
    FREE (a)

#define FREE_VARDEC(a)                                                                   \
    FREE_TYPE (a->TYPES)                                                                 \
    FREE (a)

#define FREE_TREE(a) FreeTree (a)

#define FREE_IDS(a)                                                                      \
    {                                                                                    \
        DBUG_PRINT ("FREE", (P_FORMAT, a));                                              \
        FreeAllIds (a);                                                                  \
    }

static int label_nr = 0;

/*
 *
 *  functionname  :
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
 *  functionname  :
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
 *  functionname  : AddVardec
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
node *
AddVardec (node *vardec, types *type, char *name)
{
    node *tmp = vardec;
    int insert = 1;

    DBUG_ENTER ("AddVardec");

    /* look if there is matching vardec */
    if (NULL != tmp) {
        while ((NULL != VARDEC_NEXT (tmp)) && (1 == insert)) {
            if (!strcmp (VARDEC_NAME (tmp), name))
                insert = 0;
            tmp = VARDEC_NEXT (tmp);
        }
    }

    /* now insert new vardec node */
    if ((1 == insert) ? ((NULL != tmp) ? strcmp (VARDEC_NAME (tmp), name) : 1) : 0) {
        types *new_type = DuplicateTypes (type, 0);
        node *new_vardec = MakeVardec (StringCopy (name), new_type, NULL);

        DBUG_ASSERT ((NULL != tmp) ? (NULL == VARDEC_NEXT (tmp)) : 1,
                     "VARDEC_NEXT(tmp) != NULL");

        APPEND_VARDECS (tmp, new_vardec);
    }

    DBUG_RETURN (vardec);
}

/*
 *
 *  functionname  : AdjustAddedAssigns
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

void
AdjustAddedAssigns (node *before, node *after)
{
    char *new_id, *old_id;
    node *tmp, *last;

    DBUG_ENTER ("AdjustAddedAssigns");

    before = ASSIGN_NEXT (before);

    while (before != NULL) {
        if (NODE_TYPE (ASSIGN_INSTR (before)) == N_icm) {
            if ((0
                 == strcmp (ICM_NAME (ASSIGN_INSTR (before)), "ND_KS_MAKE_UNIQUE_ARRAY"))
                || (0
                    == strcmp (ICM_NAME (ASSIGN_INSTR (before)),
                               "ND_MAKE_UNIQUE_HIDDEN"))) {
                tmp = ASSIGN_NEXT (after);
                last = after;
                old_id = ID_NAME (ICM_ARG1 (ASSIGN_INSTR (before)));
                new_id = ID_NAME (ICM_ARG2 (ASSIGN_INSTR (before)));

                while (tmp != NULL) {
                    if (NODE_TYPE (ASSIGN_INSTR (tmp)) == N_icm) {
                        if (((0
                              == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                         "ND_DEC_RC_FREE_HIDDEN"))
                             && (0
                                 == strcmp (ID_NAME (ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                            old_id)))
                            || ((0
                                 == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                            "ND_DEC_RC_FREE_ARRAY"))
                                && (0
                                    == strcmp (ID_NAME (ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                               old_id)))
                            || ((0
                                 == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)), "ND_ALLOC_RC"))
                                && (0
                                    == strcmp (ID_NAME (ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                               new_id))))

                        {
                            ASSIGN_NEXT (last) = ASSIGN_NEXT (tmp);
#ifndef NEWTREE
                            if (ASSIGN_NEXT (last) == NULL) {
                                last->nnode = 1;
                            }
#endif
                        } else {
                            if ((0
                                 == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                            "ND_NO_RC_FREE_ARRAY"))
                                && (0
                                    == strcmp (ID_NAME (ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                               new_id))) {
                                ICM_NAME (ASSIGN_INSTR (before))
                                  = "ND_KS_NO_RC_MAKE_UNIQUE_ARRAY";
                            } else {
                                if ((0
                                     == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                                "ND_NO_RC_FREE_HIDDEN"))
                                    && (0
                                        == strcmp (ID_NAME (
                                                     ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                                   new_id))) {
                                    ICM_NAME (ASSIGN_INSTR (before))
                                      = "ND_NO_RC_MAKE_UNIQUE_HIDDEN";
                                } else {
                                    if (((0
                                          == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                                     "ND_NO_RC_ASSIGN_HIDDEN"))
                                         || (0
                                             == strcmp (ICM_NAME (ASSIGN_INSTR (tmp)),
                                                        "ND_KS_NO_RC_ASSIGN_ARRAY")))
                                        && (0
                                            == strcmp (ID_NAME (
                                                         ICM_ARG1 (ASSIGN_INSTR (tmp))),
                                                       new_id))
                                        && (0
                                            == strcmp (ID_NAME (
                                                         ICM_ARG2 (ASSIGN_INSTR (tmp))),
                                                       old_id))) {
                                        new_id = old_id;
                                    }
                                }
                            }
                        }
                    }

                    last = tmp;
                    tmp = ASSIGN_NEXT (tmp);
                }
            }
        }

        before = ASSIGN_NEXT (before);
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : BasetypeSize
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

int
BasetypeSize (types *type)
{
    int ret;
    node *tdef;

    DBUG_ENTER ("BasetypeSize");

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = LookupType (TYPES_NAME (type), TYPES_MOD (type), 042);
        DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

        ret = basetype_size[TYPEDEF_BASETYPE (tdef)];
    } else {
        ret = basetype_size[TYPES_BASETYPE (type)];
    }

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : GenericFun
 *  arguments     : 1) type of a N_vardec or N_arg node
 *                  2) index to specify generic function:
 *                     0: copyfun
 *                     1: freefun
 *  description   : returns the name of the specified generic function
 *                  of the given type
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : LookupType
 *  macros        : DBUG, TREE
 *
 *  remarks       :
 *
 */

char *
GenericFun (int which, types *type)
{
    char *ret = NULL;
    node *tdef;

    DBUG_ENTER ("GenericFun");

    DBUG_PRINT ("COMP", ("Looking for generic fun %d (0==copy/1==free)"));

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = LookupType (TYPES_NAME (type), TYPES_MOD (type), 042);
        /* 042 is only a dummy argument */
        DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

        if ((TYPEDEF_BASETYPE (tdef) == T_hidden)
            || (TYPEDEF_BASETYPE (tdef) == T_user)) {
            if (TYPEDEF_TNAME (tdef) != NULL) {
                tdef = LookupType (TYPEDEF_TNAME (tdef), TYPEDEF_TMOD (tdef), 042);
                DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");
            }

            switch (which) {
            case 0:
                ret = TYPEDEF_COPYFUN (tdef);
                break;
            case 1:
                ret = TYPEDEF_FREEFUN (tdef);
                break;
            default:
                DBUG_ASSERT (0, "Unknown kind if generic function requested");
            }
        }
    }

    DBUG_PRINT ("COMP", ("Found generic fun %s", ret));

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : MakeTypeString
 *  arguments     : 1) types structure
 *  description   : converts type to the respective string used for
 *                  compilation, which means the basetype as string
 *                  followed by * in the case of arrays.
 *  global vars   : type_string
 *  internal funs : ---
 *  external funs : Malloc, strcat, strcpy, strlen
 *  macros        : DBUG, TREE, GET_BASIC_TYPE
 *
 *  remarks       :
 *
 *
 */

char *
MakeTypeString (types *fulltype)
{
    char *ret;

    DBUG_ENTER ("MakeTypeString");

    if (TYPES_DIM (fulltype) != 0) {
        ret = (char *)Malloc (sizeof (char)
                              * (strlen (type_string[TYPES_BASETYPE (fulltype)]) + 3));
        strcpy (ret, type_string[TYPES_BASETYPE (fulltype)]);
        strcat (ret, " *");
    } else {
        ret = type_string[TYPES_BASETYPE (fulltype)];
    }

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : MergeIcmsAp
 *  arguments     : 1) icm for out-parameter which is already situated
 *                     in the table
 *                  2) icm for in-parameter which was to be added to the
 *                     table when the mapping was detected.
 *                  3) type of parameter
 *                  4) refcount of variable
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

node *
MergeIcmsAp (node *out_icm, node *in_icm, types *type, int rc)
{
    node *new_assign, *icm_arg;

    DBUG_ENTER ("MergeIcmsAp");

    if (IsBoxed (type)) {
        ICMPARAM_TAG (out_icm) = "upd_bx";

        if (IsArray (type)) {
            if (IsUnique (type)) {
                DBUG_PRINT ("COMP", ("Merging ICM-args: unique array %s - %s",
                                     ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                     ID_NAME (ICMPARAM_ARG1 (out_icm))));

                if (0
                    != strcmp (ID_NAME (ICMPARAM_ARG1 (in_icm)),
                               ID_NAME (ICMPARAM_ARG1 (out_icm)))) {
                    CREATE_2_ARY_ICM (new_assign, "ND_KS_NO_RC_ASSIGN_ARRAY",
                                      MAKE_IDNODE (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                                      MAKE_IDNODE (
                                        StringCopy (ID_NAME (ICMPARAM_ARG1 (out_icm)))));
                } else {
                    new_assign = NULL;
                }
            } else {
                if (rc == 1) {
                    DBUG_PRINT ("COMP", ("Merging ICM-args non-unique array with rc==1"
                                         " %s - %s",
                                         ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                         ID_NAME (ICMPARAM_ARG1 (out_icm))));

                    CREATE_3_ARY_ICM (new_assign, "ND_KS_MAKE_UNIQUE_ARRAY",
                                      MAKE_IDNODE (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                                      MAKE_IDNODE (
                                        StringCopy (ID_NAME (ICMPARAM_ARG1 (out_icm)))),
                                      MakeNum (BasetypeSize (type)));
                } else {
                    DBUG_PRINT ("COMP", ("Merging ICM-args non-unique array with rc>1"
                                         " %s - %s",
                                         ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                         ID_NAME (ICMPARAM_ARG1 (out_icm))));

                    CREATE_3_ARY_ICM (new_assign, "ND_KS_COPY_ARRAY",
                                      MAKE_IDNODE (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                                      MAKE_IDNODE (
                                        StringCopy (ID_NAME (ICMPARAM_ARG1 (out_icm)))),
                                      MakeNum (BasetypeSize (type)));
                }
            }
        } else {
            if (IsUnique (type)) {
                DBUG_PRINT ("COMP", ("Merging ICM-args unique hidden %s - %s",
                                     ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                     ID_NAME (ICMPARAM_ARG1 (out_icm))));

                if (0
                    != strcmp (ID_NAME (ICMPARAM_ARG1 (in_icm)),
                               ID_NAME (ICMPARAM_ARG1 (out_icm)))) {
                    CREATE_2_ARY_ICM (new_assign, "ND_NO_RC_ASSIGN_HIDDEN",
                                      MAKE_IDNODE (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                                      MAKE_IDNODE (
                                        StringCopy (ID_NAME (ICMPARAM_ARG1 (out_icm)))));
                } else {
                    new_assign = NULL;
                }
            } else {
                if (rc == 1) {
                    DBUG_PRINT ("COMP", ("Merging ICM-args non-unique hidden %s - %s"
                                         " with rc==1",
                                         ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                         ID_NAME (ICMPARAM_ARG1 (out_icm))));

                    CREATE_3_ARY_ICM (new_assign, "ND_MAKE_UNIQUE_HIDDEN",
                                      MAKE_IDNODE (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                                      MAKE_IDNODE (
                                        StringCopy (ID_NAME (ICMPARAM_ARG1 (out_icm)))),
                                      MAKE_IDNODE (GenericFun (0, type)));
                } else {
                    DBUG_PRINT ("COMP", ("Merging ICM-args non-unique hidden %s - %s"
                                         " with rc>1",
                                         ID_NAME (ICMPARAM_ARG1 (in_icm)),
                                         ID_NAME (ICMPARAM_ARG1 (out_icm))));

                    CREATE_3_ARY_ICM (new_assign, "ND_KS_COPY_HIDDEN",
                                      MAKE_IDNODE (ID_NAME (ICMPARAM_ARG1 (in_icm))),
                                      MAKE_IDNODE (
                                        StringCopy (ID_NAME (ICMPARAM_ARG1 (out_icm)))),
                                      MAKE_IDNODE (GenericFun (0, type)));
                }
            }
        }
    } else {
        ICMPARAM_TAG (out_icm) = "upd";

        DBUG_PRINT ("COMP",
                    ("Merging ICM-args unboxed %s", ID_NAME (ICMPARAM_ARG1 (out_icm))));

        if ((NODE_TYPE (ICMPARAM_ARG1 (in_icm)) == N_id)
            && (0
                == strcmp (ID_NAME (ICMPARAM_ARG1 (in_icm)),
                           ID_NAME (ICMPARAM_ARG1 (out_icm))))) {
            new_assign = NULL;
        } else {
            new_assign = MakeAssign (MakeLet (ICMPARAM_ARG1 (in_icm),
                                              MakeIds (StringCopy (ID_NAME (
                                                         ICMPARAM_ARG1 (out_icm))),
                                                       NULL, ST_regular)),
                                     NULL);
#ifndef NEWTREE
            new_assign->nnode = 1;
#endif
        }
    }

    DBUG_PRINT ("COMP", ("Merging icm args of \"ND_FUN_AP\", new tag=\"%s\"",
                         ID_NAME (EXPRS_EXPR (out_icm))));

    DBUG_RETURN (new_assign);
}

/*
 *
 *  functionname  : MergeIcmsFundef
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

void
MergeIcmsFundef (node *out_icm, node *in_icm, types *out_type, types *in_type, int line)
{
    DBUG_ENTER ("MergeIcmsFundef");

    if ((0 != strcmp ("out", ID_NAME (EXPRS_EXPR (out_icm))))
        || (0 != strcmp ("in", ID_NAME (EXPRS_EXPR (in_icm))))) {
        ERROR (line, ("Pragma 'linksign` illegal"));
        CONT_ERROR (("Mappings allowed exclusively between one parameter"
                     " and one return value on which both the function"
                     " does no refcounting !"));
        ABORT_ON_ERROR;
    }

    if (1 == CmpTypes (out_type, in_type)) {
        if (IsBoxed (out_type)) {
            ID_NAME (EXPRS_EXPR (out_icm)) = "upd_bx";
        } else {
            ID_NAME (EXPRS_EXPR (out_icm)) = "upd";
        }

        DBUG_PRINT ("COMP", ("Merging icm args of \"ND_FUN_DEC\", new tag=\"%s\"",
                             ID_NAME (EXPRS_EXPR (out_icm))));
    } else {
        ERROR (line, ("Pragma 'linksign` illegal"));
        CONT_ERROR (("Mappings allowed exclusively between parameters"
                     " with identical types !"));
        ABORT_ON_ERROR;
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : ReorganizeReturnIcm
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

node *
ReorganizeReturnIcm (node *icm_arg)
{
    node *tmp, *last;
    int first_out = 0, cnt = 0;

    DBUG_ENTER ("ReorganizeReturnIcm");

    if (icm_arg != NULL) {
        tmp = EXPRS_NEXT (EXPRS_NEXT (icm_arg));
        last = EXPRS_NEXT (icm_arg);

        while (tmp != NULL) {
            if ((!first_out) && (0 == strcmp ("out", ID_NAME (EXPRS_EXPR (tmp))))) {
                first_out = 1;
                ID_NAME (EXPRS_EXPR (icm_arg)) = ID_NAME (EXPRS_EXPR (EXPRS_NEXT (tmp)));
                tmp = EXPRS_NEXT (EXPRS_NEXT (tmp));
                EXPRS_NEXT (last) = tmp;
            } else {
                last = EXPRS_NEXT (EXPRS_NEXT (last));
                tmp = EXPRS_NEXT (EXPRS_NEXT (tmp));
                cnt += 1;
            }
        }

/*********************************/
#ifndef NEWTREE
        last->nnode = 1;
#endif
        /*********************************/

        NUM_VAL (EXPRS_EXPR (EXPRS_NEXT (icm_arg))) = cnt;
    }

    DBUG_RETURN (icm_arg);
}

/*
 *
 *  functionname  : CreateApIcm
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       : last let-node is reused for icm node
 *
 */

node *
CreateApIcm (node *icm, char *name, node **icm_tab, int tab_size)
{
    int i, cnt_icm = 0;
    node *icm_arg, *tmp;

    DBUG_ENTER ("CreateApIcm");

    DBUG_PRINT ("COMP", ("Creating ICM \"ND_FUN_AP\""));

    NODE_TYPE (icm) = N_icm;
    MAKE_ICM_NAME (icm, "ND_FUN_AP");

    /* put function_name at beginning of ICM_ARGS */

    MAKE_ICM_ARG (icm_arg, MAKE_IDNODE (StringCopy (name)));
    ICM_ARGS (icm) = icm_arg;

    if (icm_tab[1] == NULL) {
        MAKE_NEXT_ICM_ARG (icm_arg, MAKE_IDNODE (""));
    } else {
        APPEND_ICM_ARG (icm_arg, EXPRS_NEXT (icm_tab[1]));
    }

    /*
     *  Now, the number of icm arguments is counted.
     */

    for (i = 2; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            cnt_icm += 1;
        }
    }

    tmp = icm_tab[0];

    while (tmp != NULL) {
        cnt_icm += 1;
        tmp = EXPRS_NEXT (EXPRS_NEXT (tmp));
    }

    /*
     *  The number od icm arguments is stored within the icm.
     */

    MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (cnt_icm));

    /*
     *  The icm arguments are extracted from the table and concatenated
     *  to a list.
     */

    for (i = 2; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            APPEND_ICM_ARG (icm_arg, icm_tab[i]);
            icm_arg = EXPRS_NEXT (icm_arg);
        }
    }

    if (icm_tab[0] != NULL) {
        APPEND_ICM_ARG (icm_arg, icm_tab[0]);
    }

    DBUG_RETURN (icm);
}

/*
 *
 *  functionname  : CreateFundefIcm
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

node *
CreateFundefIcm (char *name, node **icm_tab, int tab_size)
{
    node *icm, *icm_arg;
    int cnt_icm = 0, i;

    DBUG_ENTER ("CreateFundefIcm");

    DBUG_PRINT ("COMP", ("Creating ICM \"ND_FUN_DEC\""));

    icm = MakeNode (N_icm);
    MAKE_ICM_NAME (icm, "ND_FUN_DEC");

    MAKE_ICM_ARG (icm_arg, MAKE_IDNODE (StringCopy (name)));
    ICM_ARGS (icm) = icm_arg;

    if (icm_tab[1] == NULL) {
        MAKE_NEXT_ICM_ARG (icm_arg, MAKE_IDNODE ("void"));
    } else {
        APPEND_ICM_ARG (icm_arg, EXPRS_NEXT (icm_tab[1]));
    }

    for (i = 0; i < tab_size; i++) {
        if ((i != 1) && (icm_tab[i] != NULL)) {
            cnt_icm += 1;
        }
    }

    MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (cnt_icm));

    for (i = 2; i < tab_size; i++) {
        if (icm_tab[i] != NULL) {
            APPEND_ICM_ARG (icm_arg, icm_tab[i]);
            icm_arg = EXPRS_NEXT (EXPRS_NEXT (icm_arg));
        }
    }

    if (icm_tab[0] != NULL) {
        APPEND_ICM_ARG (icm_arg, icm_tab[0]);
    }

    DBUG_RETURN (icm);
}

/*
 *
 *  functionname  : InsertApDotsParam
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

void
InsertApDotsParam (node **icm_tab, node *icm_arg)
{
    DBUG_ENTER ("InsertApDotsParam");

    DBUG_PRINT ("COMP", ("Inserting ...-param in application"));

    icm_tab[0] = icm_arg;

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertApArgParam
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

void
InsertApArgParam (node **icm_tab, node *icm_arg, types *type, int rc,
                  node *collect_assigns, int *linksign, int cnt_param)
{
    node *new_assign = NULL;

    DBUG_ENTER ("InsertApArgParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_arg))));

    if (linksign == NULL) {
        /*
         *  create standard icm table
         */

        icm_tab[cnt_param + 2] = icm_arg;

    } else {
        /*
         *  create special icm table depending on pragma liksign
         */

        if (icm_tab[linksign[cnt_param] + 1] == NULL) {
            icm_tab[linksign[cnt_param] + 1] = icm_arg;
        } else {
            new_assign
              = MergeIcmsAp (icm_tab[linksign[cnt_param] + 1], icm_arg, type, rc);

            if (new_assign != NULL) {
                ASSIGN_NEXT (new_assign) = ASSIGN_NEXT (collect_assigns);

#ifndef NEWTREE
                if (ASSIGN_NEXT (new_assign) == NULL) {
                    new_assign->nnode = 1;
                } else {
                    new_assign->nnode = 2;
                }
#endif

                ASSIGN_NEXT (collect_assigns) = new_assign;
            }
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertApReturnParam
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

void
InsertApReturnParam (node **icm_tab, node *icm_arg, types *type, int *linksign,
                     int cnt_param)
{
    DBUG_ENTER ("InsertApReturnParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_arg))));

    if (linksign == NULL) {
        /*
         *  create standard icm table
         */

        if ((0 == strcmp (ID_NAME (EXPRS_EXPR (icm_arg)), "out"))
            && (icm_tab[1] == NULL)) {
            icm_tab[1] = icm_arg;
        } else {
            icm_tab[cnt_param + 2] = icm_arg;
        }
    } else {
        /*
         *  create special icm table depending on pragma liksign
         */

        icm_tab[linksign[cnt_param] + 1] = icm_arg;
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertDefDotsParam
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

void
InsertDefDotsParam (node **icm_tab, node *icm_arg)
{
    DBUG_ENTER ("InsertDefDotsParam");

    DBUG_PRINT ("COMP", ("Inserting ...-param in function definition"));

    icm_tab[0] = icm_arg;

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertDefArgParam
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

void
InsertDefArgParam (node **icm_tab, node *icm_arg, types **type_tab, types *type_arg,
                   int *linksign, int cnt_param, int line)
{
    DBUG_ENTER ("InsertDefArgParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_arg))));

    if (linksign == NULL) {
        /*
         *  create standard icm table
         */

        icm_tab[cnt_param + 2] = icm_arg;

    } else {
        /*
         *  create special icm table depending on pragma liksign
         */

        if (linksign[cnt_param] == 0) {
            ERROR (line, ("Pragma 'linksign` illegal"));
            CONT_ERROR (("Argument parameter cannot be mapped to return position"));
            ABORT_ON_ERROR;
        }

        if (icm_tab[linksign[cnt_param] + 1] == NULL) {
            icm_tab[linksign[cnt_param] + 1] = icm_arg;
        } else {
            MergeIcmsFundef (icm_tab[linksign[cnt_param] + 1], icm_arg,
                             type_tab[linksign[cnt_param] + 1], type_arg, line);
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertDefReturnParam
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

void
InsertDefReturnParam (node **icm_tab, node *icm_arg, types **type_tab, types *type_arg,
                      int *linksign, int cnt_param, int line)
{
    DBUG_ENTER ("InsertDefReturnParam");

    DBUG_PRINT ("COMP", ("Inserting arg #%d, tag=\"%s\"", cnt_param,
                         ID_NAME (EXPRS_EXPR (icm_arg))));

    if (linksign == NULL) {
        /*
         *  create standard icm table
         */

        if ((0 == strcmp (ID_NAME (EXPRS_EXPR (icm_arg)), "out"))
            && (icm_tab[1] == NULL)) {
            icm_tab[1] = icm_arg;
        } else {
            icm_tab[cnt_param + 2] = icm_arg;
        }
    } else {
        /*
         *  create special icm table depending on pragma liksign
         */

        if (icm_tab[linksign[cnt_param] + 1] == NULL) {
            icm_tab[linksign[cnt_param] + 1] = icm_arg;
            type_tab[linksign[cnt_param] + 1] = type_arg;
        } else {
            ERROR (line, ("Pragma 'linksign` illegal"));
            CONT_ERROR (("2 return parameters mapped to same position !"));
            ABORT_ON_ERROR;
        }
    }

    DBUG_VOID_RETURN;
}

#if 0
/*
 *
 *  functionname  : GenStandardIcmTab
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

node **GenStandardIcmTab(node *vars_arg_start, int n_vars, int varblock_length)
{
  node *tmp;
  int i;
  
  DBUG_ENTER("GenStandardIcmTab");

  icm_arg_tab=(node**)Malloc(sizeof(node*)*(n_vars+1));

  for (i=0; i<=n_vars; i++)
  {
    icm_arg_tab[i]=NULL;
  }
  
  /*
   *  A table for variable icm args is created and initialized.
   */

  tmp=vars_arg_start;
  i=1;
    
  while (tmp!=NULL)
  {
    /*
     *  All variable icm args are inserted into the table.
     *  The first "out" parameter is inserted at position 0, all other
     *  parameters are inserted in their original sequence starting with
     *  position 1.
     */

    if (0==strcmp(ID_NAME(EXPRS_EXPR(tmp)), "out_bx"))
    {
      ID_NAME(EXPRS_EXPR(tmp))="out";
    }
    else
    {
      if (0==strcmp(ID_NAME(EXPRS_EXPR(tmp)), "in_bx"))
      {
        ID_NAME(EXPRS_EXPR(tmp))="in";
      }
    }
    
    if ((0==strcmp(ID_NAME(EXPRS_EXPR(tmp)), "out"))
        && (icm_arg_tab[0]==NULL))
    {
      icm_arg_tab[0]=tmp;

      DBUG_PRINT("COMP",
                 ("First OUT parameter moved to special position"));
    }
    else
    {
      icm_arg_tab[i]=tmp;
      i+=1;
    }
      
    SHIFT_EXPRS_PTR(tmp, varblock_length);
  }

  DBUG_RETURN(icm_arg_tab);
}


/*
 *
 *  functionname  : GenSpecialIcmTabFundef
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

node **GenSpecialIcmTabFundef(node *vars_arg_start, int n_vars,
                              node *fundef, int *mapping_info)
{
  int varblock_length=3, *linksign;
  node **icm_arg_tab, *current_arg;
  types **arg_type_tab, *current_type;
  
  DBUG_ENTER("GenSpecialIcmTabFundef");
  
  linksign=PRAGMA_LINKSIGN(FUNDEF_PRAGMA(fundef));
  
  /*
   *  A table for variable icm args is created and initialized.
   */

  icm_arg_tab=(node**)Malloc(sizeof(node*)*(n_vars+1));
  arg_type_tab=(types**)Malloc(sizeof(types*)*(n_vars+1));

  for (i=0; i<=n_vars; i++)
  {
    icm_arg_tab[i]=NULL;
    arg_type_tab[i]=NULL;
  }
  
  
  /*
   *  First, the local variables current_arg and current_type are
   *  initialized. These are traversed simultaniously to the icms
   *  in order to know each parameter's type.
   */

  if (FUNDEF_BASETYPE(fundef)==T_void)
  {
    current_arg=FUNDEF_ARGS(fundef);
    if (current_arg!=NULL)
    {
      current_type=ARG_TYPE(current_arg);
    }
  }
  else
  {
    current_arg=NULL;
    current_type=FUNDEF_TYPES(fundef);
  }

  tmp=vars_arg_start;
  i=0;
  
  while (tmp!=NULL)
  {
    /*
     *  All variable icm args are inserted into the table
     *  depending on pragma linksign.
     */

    if (icm_arg_tab[linksign[i]]==NULL)
    {
      if (0==strcmp(ID_NAME(EXPRS_EXPR(tmp)), "out_bx"))
      {
        ID_NAME(EXPRS_EXPR(tmp))="out";
      }
      else
      {
        if (0==strcmp(ID_NAME(EXPRS_EXPR(tmp)), "in_bx"))
        {
          ID_NAME(EXPRS_EXPR(tmp))="in";
        }    
      }
      
      icm_arg_tab[linksign[i]]=tmp;
      arg_type_tab[linksign[i]]=current_type;

      DBUG_PRINT("COMP",
                 ("Parameter #%d moved to link position #%d",
                  i, linksign[i]));
    }
    else
    {
      MergeIcmsFundef(icm_arg_tab[linksign[i]], tmp, 
                      arg_type_tab[linksign[i]], current_type,
                      NODE_LINE(fundef), mapping_info[2*i]);
    }      
      
    i+=1;
    SHIFT_EXPRS_PTR(tmp, varblock_length);

    /*
     *  Finally, current_type and current_arg are shifted.
     */

    if (current_arg==NULL)
    {
      if (TYPES_NEXT(current_type)==NULL)
      {
        current_arg=FUNDEF_ARGS(fundef);
        if (current_arg!=NULL)
        {
          current_type=ARG_TYPE(current_arg);
        }
      }
      else
      {
        current_type=TYPES_NEXT(current_type);
      }
    }
    else
    {
      current_arg=ARG_NEXT(current_arg);
      if (current_arg!=NULL)
      {
        current_type=ARG_TYPE(current_arg);
      }
    }
  }
        
  FREE(arg_type_tab);
  FREE(mapping_info);
  
  DBUG_RETURN(icm_arg_tab);
}


/*
 *
 *  functionname  : GenSpecialIcmTabNonFundef
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

node **GenSpecialIcmTabNonFundef(node *vars_arg_start, int n_vars,
                                 int *linksign, node *collect_assigns)
{
  int varblock_length=2, i;
  node **icm_arg_tab, *tmp;
  
  DBUG_ENTER("GenSpecialIcmTabNonFundef");


  /*
   *  A table for variable icm args is created and initialized.
   */

  icm_arg_tab=(node**)Malloc(sizeof(node*)*(n_vars+1));

  for (i=0; i<=n_vars; i++)
  {
    icm_arg_tab[i]=NULL;
  }
  

  tmp=vars_arg_start;
  i=0;
  
  while (tmp!=NULL)
  {
    /*
     *  All variable icm args are inserted into the table
     *  depending on pragma linksign.
     */

    if (icm_arg_tab[linksign[i]]==NULL)
    {
      if (0==strcmp(ID_NAME(EXPRS_EXPR(tmp)), "out_bx"))
      {
        ID_NAME(EXPRS_EXPR(tmp))="out";
      }
      else
      {
        if (0==strcmp(ID_NAME(EXPRS_EXPR(tmp)), "in_bx"))
        {
          ID_NAME(EXPRS_EXPR(tmp))="in";
        }    
      }
      
      icm_arg_tab[linksign[i]]=tmp;

      DBUG_PRINT("COMP",
                 ("Parameter #%d moved to link position #%d",
                  i, linksign[i]));
    }
    else
    {
      new_assign=MergeIcmsAp(icm_arg_tab[linksign[i]], tmp, 
                             mapping_info[2*i], mapping_info[2*i+1]);

      if (new_assign!=NULL)
      {
        ASSIGN_NEXT(new_assign)=ASSIGN_NEXT(collect_assigns);
        ASSIGN_NEXT(collect_assigns)=new_assign;
      }
    }      
      
    i+=1;
    SHIFT_EXPRS_PTR(tmp, varblock_length);
  }

  DBUG_RETURN(icm_arg_tab);
}
  

/*
 *
 *  functionname  : ReorganizeParameters
 *  arguments     : 1) tag which identifies the ICM to be reorganized:
 *                     1: ND_FUN_DEC
 *                     2: ND_FUN_RET
 *                     3: ND_FUN_AP
 *                  2) ICM args to reorganize
 *                  3) fundef node
 *  description   : 
 *  global vars   : 
 *  internal funs : 
 *  external funs : 
 *  macros        : DBUG, TREE, SHIFT_EXPRS_PTR
 *
 *  remarks       : 
 *
 */

node *ReorganizeParameters(int tag, node *icm_args, node *fundef,
                           int *mapping_info)
{
  int header_length, varblock_length, i, n_vars;
  node *tmp, *n_vars_arg, *vars_arg_start, *ret_arg, *last_header_arg,
       *last, **icm_arg_tab, *collect_assigns=NULL;
  int *linksign;
  types *current_type, **arg_type_tab;
  
  DBUG_ENTER("ReorganizeParameters");
  
  if (FUNDEF_PRAGMA(fundef)==NULL)
  {
    linksign=NULL;
  }
  else
  {
    linksign=PRAGMA_LINKSIGN(FUNDEF_PRAGMA(fundef));
  }
  
  header_length=(tag==2)?2:3;
    /* number of static icm args (before ICM_VAR) */
 
  varblock_length=(tag==1)?3:2;
    /* number of dynamic icm args which belong together */
  
  DBUG_PRINT("COMP",
             ("Reorganizing parameters (tag=%d, pragma=%d)", tag, linksign));
  
  if (tag==2)
  {
    tmp=icm_args;           /* ND_FUN_RET */
  }
  else
  {
    tmp=EXPRS_NEXT(icm_args);
  }
  
  ret_arg=EXPRS_EXPR(tmp);
  tmp=EXPRS_NEXT(tmp);

  last_header_arg=tmp;
  vars_arg_start=EXPRS_NEXT(tmp);
  n_vars_arg=EXPRS_EXPR(tmp);

  /*
   *  ret_arg points to the special return identifier or return type
   *  respectively.
   *
   *  n_vars_arg points to that icm arg which stores the number of 
   *  following variable arguments.
   *
   *  vars_arg_start points to the list of variable icm args, whose
   *  number is stored in the icm arg before.
   */

  tmp=vars_arg_start;
  n_vars=0;
  
  while (tmp!=NULL)
  {
    n_vars++;
    SHIFT_EXPRS_PTR(tmp, varblock_length);
  }

  DBUG_PRINT("COMP", ("%d icm args found", n_vars));
  
  /*
   *  The number of following variable icm args is counted 
   *  and stored in n_vars.
   */


  if (linksign==NULL)
  {
    icm_arg_tab=GenStandardIcmTab(vars_arg_start, n_vars, varblock_length);
  }
  else
  {
    if (tag==1)
    {
      icm_arg_tab=GenSpecialIcmTabFundef(vars_arg_start, n_vars, fundef,
                                         mapping_info);
    }
    else
    {
      collect_assigns=MakeAssign(NULL, NULL);
      icm_arg_tab=GenSpecialIcmTabNonFundef(vars_arg_start, n_vars,
                                            linksign, collect_assigns,
                                            mapping_info);
    }
  }
  
  
  /*
   *  The real return parameter is moved to its special position.
   */

  if (icm_arg_tab[0]!=NULL)
  {
    ID_NAME(ret_arg)=ID_NAME(EXPRS_EXPR(EXPRS_NEXT(icm_arg_tab[0])));
    NUM_VAL(n_vars_arg)=n_vars-1;
  }
  else
  {
    NUM_VAL(n_vars_arg)=n_vars;
  }
    
  /*
   *  All other parameters are put together into a chain again.
   */

  last=last_header_arg;
  
  for (i=1; i<=NUM_VAL(n_vars_arg); i++)
  {
    if (icm_arg_tab[i]==NULL)
    {
      ERROR(NODE_LINE(fundef), ("Pragma 'linksign` illegal"));
      CONT_ERROR(("No SAC parameter mapped to C parameter %d", i));
    }
    else
    {
      EXPRS_NEXT(last)=icm_arg_tab[i];

#ifndef NEWTREE
      last->nnode=2;
#endif /* NEWTREE */

      SHIFT_EXPRS_PTR(last, varblock_length);
    }
  }

#ifndef NEWTREE
  last->nnode=1;
#endif /* NEWTREE */
 
  EXPRS_NEXT(last)=NULL;

  if (collect_assigns!=NULL)
  {
    tmp=collect_assigns;
    collect_assigns=ASSIGN_NEXT(collect_assigns);
    FREE(tmp);
  }
    
  DBUG_RETURN(collect_assigns);
}
#endif /* 0 */

/*
 *
 *  functionname  : RenameVar
 *  arguments     : 1) name of variable
 *  description   : puts '__' in front of 1)
 *  global vars   :
 *  internal funs :
 *  external funs : Malloc, strlen, sprintf, sizeof
 *  macros        : DBUG...,
 *
 *  remarks       :
 *
 */
char *
RenameVar (char *string, int i)
{
    char *new_name;
    DBUG_ENTER ("RenameVar");
    if (0 == i) {
        new_name = (char *)Malloc (sizeof (char) * (strlen (string) + 3));
        sprintf (new_name, "__%s", string);
    } else {
        new_name = (char *)Malloc (sizeof (char) * (strlen (string) + 10));
        sprintf (new_name, "__%s_%d", string, i);
    }

    DBUG_RETURN (new_name);
}

/*
 *
 *  functionname  : RenameReturn
 *  arguments     : 1) N_return node
 *                  2) arg_info
 *  description   : - renames the variables in a return_sttatement,
 *                  - adds variable declaration
 *                  - inserts new assignments (after LAST_ASSIGN(arg_info))
 *  global vars   :
 *  internal funs : RenameVar,
 *  external funs : MakeNode, StringCopy, strcmp, DuplicateTypes,
 *  macros        : DBUG..., ELIMINATE_CAST, IDS_ID, ISD_NODE, ID, TYPES,
 *                  FREE, MAKENODE_ID, LAST_ASSIGN, NULL
 *
 *  remarks       : - pointer to variable declaration is stored in
 *                    arg_info->node[3]
 *                  - returns N_let of last inserted new assign if
 *                     there renameing had to be done
 *                    returns N_return if no renaming had to be done
 *                  - puts new assignments after LAST_ASSIGN(arg_info).
 *                    node[0] of LAST_ASSIGN(arg_info) will be set in
 *                    last CompAssign (return value of CompReturn)) again
 *
 */
node *
RenameReturn (node *return_node, node *arg_info)
{
    node *exprs, *tmp_exprs, *assign, *let, *next_assign, *vardec, *tmp;
    int i;
    char *old_id, *new_id;

    DBUG_ENTER ("RenameReturn");

    exprs = return_node->node[0];
    next_assign = MakeNode (N_assign);
    next_assign->node[0] = return_node;
    ;
#ifndef NEWTREE
    next_assign->nnode = 1;
#endif
    vardec = arg_info->node[3];

    while (NULL != exprs) {
        tmp_exprs = exprs->node[1];
        i = 1;
        ELIMINATE_CAST (exprs);
        old_id = exprs->node[0]->IDS_ID;
        while (NULL != tmp_exprs) {
            ELIMINATE_CAST (tmp_exprs);
            if (0 == strcmp (tmp_exprs->node[0]->IDS_ID, old_id)) {
                /* generates new nodes */
                new_id = RenameVar (old_id, i);
                let = MakeLet (MakeId (ID_NAME (EXPRS_EXPR (tmp_exprs)), NULL, 0),
                               MakeIds (new_id, NULL, 0));
                assign = MakeAssign (let, next_assign);
                next_assign = assign;
                vardec = AddVardec (vardec, ID_TYPE (EXPRS_EXPR (tmp_exprs)), new_id);

                /* rename variable in return-statement */
                ID_NAME (EXPRS_EXPR (tmp_exprs)) = StringCopy (new_id);
            }
            tmp_exprs = EXPRS_NEXT (tmp_exprs);
            i += 1;
        }
        exprs = exprs->node[1];
    }
    if (next_assign->node[0] != return_node) {
        /* new nodes have been inserted */
        node *last_assign = LAST_ASSIGN (arg_info);
        last_assign->node[0] = assign->node[0];
        last_assign->node[1] = assign->node[1];
#ifndef NEWTREE
        last_assign->nnode = 2;
#endif
        arg_info->node[3] = vardec;

        return_node = assign->node[0];
    }

    DBUG_RETURN (return_node);
}

/*
 *
 *  functionname  : GenName
 *  arguments     : 1) number of variable
 *                  2) name
 *  description   : allocates memory for a new string , copies name to this
 *                  string and appends a number 1) to it
 *  global vars   :
 *  internal funs :
 *  external funs : strlen, sizeof, Malloc, sprintf
 *  macros        : DBUG..., NULL
 *  remarks       : 1) <= 99
 *
 */
char *
GenName (int i, char *name)
{
    char *new_name;

    DBUG_ENTER ("GenName");

    new_name = (char *)Malloc (sizeof (char) * (strlen (name) + 3));
    sprintf (new_name, "%s%d", name, i);

    DBUG_RETURN (new_name);
}

#if 0
/*  it seems as if this function is no longer used */

/*
 *
 *  functionname  : GenFunName
 *  arguments     : 1) fun_name
 *                  2) mod_name
 *  description   : allocates memory for a new string and makes a new function
 *                  name out of 1), MOD_NAME_CON and 2)
 *                   
 *  global vars   : 
 *  internal funs : 
 *  external funs : strlen, sizeof, Malloc
 *  macros        : DBUG..., NULL, MOD_NAME_CON
 *  remarks       : ----
 *
 */
char *GenFunName(char *fun_name, char *mod_name)
{
   int str_length;
   char *new_name;
   
   DBUG_ENTER("GenFunName");

   if(NULL == mod_name)
      new_name=fun_name;
   else
   {
      str_length=strlen(fun_name)+strlen(mod_name);
      new_name=(char*)Malloc(sizeof(char)*(str_length+3));
      strcat(new_name, mod_name);
      strcat(new_name, MOD_NAME_CON);
      strcat(new_name, fun_name);
   }
   DBUG_RETURN(new_name);
}
#endif

/*
 *
 *  functionname  : ShapeToArray
 *  arguments     : 1) N_vardec node
 *  description   : computes the shape of corresponding type and stores it
 *                  as N_exprs - chain
 *
 *  global vars   :
 *  internal funs :
 *  external funs : LookupType, MakeNode
 *  macros        : DBUG..., NULL, SIMPLETYPE, DIM, SHP, MAKENODE_NUM, NAME,
 *                  NAME_MOD
 *  remarks       : ----
 *
 */
node *
ShapeToArray (node *vardec_node)
{
    node *ret_node = NULL, *tmp, *basic_type_node;
    int i;

    DBUG_ENTER ("ShapeToArray");
    if (T_user != vardec_node->SIMPLETYPE) {
        ret_node = MakeNode (N_exprs);
        MAKENODE_NUM (ret_node->node[0], vardec_node->SHP[0]);
        tmp = ret_node;
        for (i = 1; i < vardec_node->DIM; i++) {
#ifndef NEWTREE
            tmp->nnode = 2;
#endif
            tmp->node[1] = MakeNode (N_exprs);
            MAKENODE_NUM (tmp->node[1]->node[0], vardec_node->SHP[i]);
#ifndef NEWTREE
            tmp->node[1]->nnode = 1;
#endif
            tmp = tmp->node[1];
        }
    } else {
        basic_type_node = LookupType (vardec_node->NAME, vardec_node->NAME_MOD,
                                      042); /* 042 is dummy argument */
        if (1 <= vardec_node->DIM) {
            ret_node = MakeNode (N_exprs);
            MAKENODE_NUM (ret_node->node[0], vardec_node->SHP[0]);
            tmp = ret_node;
            for (i = 1; i < vardec_node->DIM; i++) {
#ifndef NEWTREE
                tmp->nnode = 2;
#endif
                tmp->node[1] = MakeNode (N_exprs);
                MAKENODE_NUM (tmp->node[1]->node[0], vardec_node->SHP[i]);
#ifndef NEWTREE
                tmp->node[1]->nnode = 1;
#endif
                tmp = tmp->node[1];
            }
            for (i = 0; i < basic_type_node->DIM; i++) {
#ifndef NEWTREE
                tmp->nnode = 2;
#endif
                tmp->node[1] = MakeNode (N_exprs);
                MAKENODE_NUM (tmp->node[1]->node[0], basic_type_node->SHP[i]);
#ifndef NEWTREE
                tmp->node[1]->nnode = 1;
#endif
                tmp = tmp->node[1];
            }
        } else {
            ret_node = MakeNode (N_exprs);
            MAKENODE_NUM (ret_node->node[0], basic_type_node->SHP[0]);
            tmp = ret_node;
            for (i = 1; i < basic_type_node->DIM; i++) {
#ifndef NEWTREE
                tmp->nnode = 2;
#endif
                tmp->node[1] = MakeNode (N_exprs);
                MAKENODE_NUM (tmp->node[1]->node[0], basic_type_node->SHP[i]);
#ifndef NEWTREE
                tmp->node[1]->nnode = 1;
#endif
                tmp = tmp->node[1];
            }
        }
    }

    DBUG_RETURN (ret_node);
}

/*
 *
 *  functionname  : Compile
 *  arguments     : 1) syntax tree
 *  description   : starts compilation  and initializes act_tab
 *
 *  global vars   : act_tab
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,
 *  remarks       : ----
 *
 */
node *
Compile (node *arg_node)
{
    node *info;

    DBUG_ENTER ("Compile");

    act_tab = comp_tab; /* set new function-table for traverse */
    info = MakeInfo ();
    if (NODE_TYPE (arg_node) == N_modul) {
        if (MODUL_OBJS (arg_node) != NULL) {
            MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), NULL);
        }
        if (MODUL_FUNS (arg_node) != NULL) {
            MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), info);
        }
        if (MODUL_TYPES (arg_node) != NULL) {
            MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), info);
        }
    } else {
        DBUG_ASSERT ((N_fundef == arg_node->nodetype), "wrong node");
        arg_node = Trav (arg_node, info);
    }
    FREE (info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompFundef
 *  arguments     : 1) N_fundef node
 *                  2) info node
 *  description   : traverses child-nodes,
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :-sets arg_info->node[0] to first N_assign of function before
 *                 - traversing the function's arguments
 *                 - sets arg_info->node[3] to variable declaration
 *                 - calls Trav to `compile` varbiable declarations
 *
 */

node *
CompFundef (node *arg_node, node *arg_info)
{
    node *return_node, *icm_arg, *type_id_node, *var_name_node, *tag_node, **icm_tab,
      *icm_tab_entry;
    types *rettypes, *fulltype, **type_tab;
    int cnt_param, tab_size, i;

    DBUG_ENTER ("CompFundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_VARDECS (arg_info) = FUNDEF_VARDEC (arg_node);
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        if (FUNDEF_VARDEC (arg_node) != NULL) {
            FUNDEF_VARDEC (arg_node) = Trav (FUNDEF_VARDEC (arg_node), arg_info);
/*********************************/
#ifndef NEWTREE
            arg_node->node[0]->nnode = 2;
#endif
            /*********************************/
        }
    }

    rettypes = FUNDEF_TYPES (arg_node);

    if ((NULL != FUNDEF_RETURN (arg_node)) && (TYPES_BASETYPE (rettypes) != T_void)) {
        /*
         * arg_node->node[3] points to a N_icm (ND_FUN_RET).
         * return_node will point to the first N_exprs belonging to a
         * return_value. This exists only for functions with at least
         * one return value.
         */

        return_node = arg_node->node[3]->node[0]->node[1]->node[1]->node[1];
    }

    cnt_param = 0;

    tab_size = CountFunctionParams (arg_node) + 2;
    icm_tab = (node **)Malloc (sizeof (node *) * (tab_size));
    type_tab = (types **)Malloc (sizeof (types *) * (tab_size));

    for (i = 0; i < tab_size; i++) {
        icm_tab[i] = NULL;
        type_tab[i] = NULL;
    }

    while ((NULL != rettypes) && (TYPES_BASETYPE (rettypes) != T_void)
           && (TYPES_BASETYPE (rettypes) != T_dots)) {
        if ((MUST_REFCOUNT (TYPE, rettypes))
            && (FUN_DOES_REFCOUNT (arg_node, cnt_param))) {
            MAKENODE_ID (tag_node, "out_rc");
        } else {
            MAKENODE_ID (tag_node, "out");
        }

        MAKE_ICM_ARG (icm_arg, tag_node);
        icm_tab_entry = icm_arg;

        GET_BASIC_TYPE (fulltype, rettypes, 042);

        MAKENODE_ID (type_id_node, MakeTypeString (fulltype));
        MAKE_NEXT_ICM_ARG (icm_arg, type_id_node);

        if (NULL == arg_node->node[0]) {
            /* it is an extern declaration */
#ifdef IMPORTED_WITH_NAME
            MAKENODE_ID (var_name_node, GenName (i, DUMMY_NAME));
#else
            MAKENODE_ID (var_name_node, "");
#endif

        } else {
            DBUG_ASSERT (N_id == return_node->node[0]->nodetype,
                         "wrong nodetype != N_id");
            MAKENODE_ID_REUSE_IDS (var_name_node, return_node->node[0]->IDS);
            if (return_node->node[1] != NULL)
                /* put return_node to next N_exprs where a function return_value
                 * is behind
                 */
                return_node = return_node->node[1]->node[1];
        }

        MAKE_NEXT_ICM_ARG (icm_arg, var_name_node);

        InsertDefReturnParam (icm_tab, icm_tab_entry, type_tab, rettypes,
                              FUNDEF_PRAGMA (arg_node) == NULL
                                ? NULL
                                : FUNDEF_LINKSIGN (arg_node),
                              cnt_param, NODE_LINE (arg_node));

        rettypes = rettypes->next;
        cnt_param++;
    }

    if ((rettypes != NULL) && (TYPES_BASETYPE (rettypes) == T_dots)) {
        MAKENODE_ID (tag_node, "in");
        MAKE_ICM_ARG (icm_arg, tag_node);
        icm_tab_entry = icm_arg;
        MAKENODE_ID (type_id_node, MakeTypeString (rettypes));
        MAKE_NEXT_ICM_ARG (icm_arg, type_id_node);
        MAKENODE_ID (var_name_node, "");
        MAKE_NEXT_ICM_ARG (icm_arg, var_name_node);

        InsertDefDotsParam (icm_tab, icm_tab_entry);
    }

    if (NULL != FUNDEF_ARGS (arg_node)) {
        if (NULL != arg_node->node[0]) {
            /* first assign of body */
            INFO_FIRSTASSIGN (arg_info) = arg_node->node[0]->node[0];
        }

        INFO_CNTPARAM (arg_info) = (cnt_param == 0) ? 1 : cnt_param;
        INFO_FUNDEF (arg_info) = arg_node;
        INFO_ICMTAB (arg_info) = icm_tab;
        INFO_TYPETAB (arg_info) = type_tab;

        /*
         *  the arg_node is needed while compiling args as argument for
         *  FUN_DOES_REFCOUNT
         */

        /* traverse formal parameters (N_arg) */
        Trav (FUNDEF_ARGS (arg_node), arg_info);

        if (NULL != FUNDEF_BODY (arg_node)) {
            /* new first assign of body */
            arg_node->node[0]->node[0] = arg_info->node[0];
            arg_info->node[0] = NULL;
        }

        INFO_CNTPARAM (arg_info) = 0;
        INFO_FUNDEF (arg_info) = NULL;
        INFO_ICMTAB (arg_info) = NULL;
        INFO_TYPETAB (arg_info) = NULL;
    }

    if ((FUNDEF_RETURN (arg_node) != NULL)
        && (ICM_ARGS (FUNDEF_RETURN (arg_node)) != NULL)) {
        ReorganizeReturnIcm (ICM_ARGS (FUNDEF_RETURN (arg_node)));
    }

    FUNDEF_ICM (arg_node) = CreateFundefIcm (FUNDEF_NAME (arg_node), icm_tab, tab_size);

    /*
     * From now on fundef->node[3] points to N_icm instead of function's
     * return-statement.
     */

    FREE (icm_tab);
    FREE (type_tab);

    /* traverse next function if any */

    if (NULL != FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    if (FUNDEF_PRAGMA (arg_node) != NULL) {
        FUNDEF_PRAGMA (arg_node) = FreeNode (FUNDEF_PRAGMA (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompBlock
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   : stacks INFO_LASTASSIGN and sets it to this node while traversal
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :
 *                  arg_info->node[0] contains pointer to node before last
 *                    assign_node (use macro INFO_LASTASSIGN or NEXT_ASSIGN to
 *                     get these nodes)
 *
 */
node *
CompBlock (node *arg_node, node *arg_info)
{
    node *old_info;

    DBUG_ENTER ("CompBlock");
    old_info = INFO_LASTASSIGN (arg_info); /* stakcing of old info ! (nested blocks)*/

    INFO_LASTASSIGN (arg_info) = arg_node;
    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    INFO_LASTASSIGN (arg_info) = old_info; /* restoring old info ! (nested blocks)*/

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompAssign
 *  arguments     : 1) N_assign node
 *                  2) N_info-node with VARDECS of fun and LASTASSIGN
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->node[0] contains pointer to previous assign_node
 *                  or N_block node
 *
 */

node *
CompAssign (node *arg_node, node *arg_info)
{
    node *old_next_assign;
    node *old_last_assign;
    node *ret_node;

    DBUG_ENTER ("CompAssign");

    DBUG_PRINT ("COMP", ("current:" P_FORMAT " next:" P_FORMAT, arg_node,
                         ASSIGN_NEXT (arg_node)));
    DBUG_PRINT ("COMP", ("last:" P_FORMAT "next:" P_FORMAT, INFO_LASTASSIGN (arg_info),
                         BLOCK_INSTR_OR_ASSIGN_NEXT (INFO_LASTASSIGN (arg_info))));

    old_last_assign = INFO_LASTASSIGN (arg_info);
    if (ASSIGN_NEXT (arg_node) == NULL) {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    } else {
        old_next_assign = ASSIGN_NEXT (arg_node);
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        INFO_LASTASSIGN (arg_info) = arg_node;

        /*
         * Now, we skip all those assigns that were inserted during the
         * traversal of arg_node!
         */
        while (old_next_assign
               != BLOCK_INSTR_OR_ASSIGN_NEXT (INFO_LASTASSIGN (arg_info))) {
            INFO_LASTASSIGN (arg_info)
              = BLOCK_INSTR_OR_ASSIGN_NEXT (INFO_LASTASSIGN (arg_info));
        }

        DBUG_PRINT ("COMP", ("set INFO_LASTASSIGN to :" P_FORMAT " with next:" P_FORMAT,
                             INFO_LASTASSIGN (arg_info),
                             BLOCK_INSTR_OR_ASSIGN_NEXT (INFO_LASTASSIGN (arg_info))));

        old_next_assign = Trav (old_next_assign, arg_info);
    }

    /*
     * although the new assigns are allready inserted correctly into
     * the chain of assignments, we have to return the correct pointer,
     * since the normal insertion mechanism probably is used!!!!!!!
     */
    ret_node = BLOCK_INSTR_OR_ASSIGN_NEXT (old_last_assign);
    DBUG_RETURN (ret_node);
}

/*
 *
 *  functionname  : CompLet
 *  arguments     : 1) N_Let node
 *                  2) N_info-node with VARDECS of fun and LASTASSIGN
 *  description   : set INFO_LASTLET and INFO_LASTIDS while traversing the expr
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer node before
 *                   last assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *
 */
node *
CompLet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CompLet");

    INFO_LASTLET (arg_info) = arg_node;
    INFO_LASTIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    INFO_LASTLET (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompVardec
 *  arguments     : 1) N_vardec node
 *                  2) NULL
 *  description   : transforms N_vardec to N_icm node if it is the declaration
 *                  of an array
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,GET_BASIC_TYPES, TYPES, DIM,
 *  remarks       : if it is a declaration of an array, a N_assign
 *                  node will be inserted to get the new N_icm node into the
 *                  chain of N_vardecs
 *
 */
node *
CompVardec (node *arg_node, node *arg_info)
{
    node *assign, *id_node, *n_node, *n_dim, *id_type, *icm_arg;
    int i;

    types *full_type;

    DBUG_ENTER ("CompVardec");

    GET_BASIC_TYPE (full_type, arg_node->TYPES, 0);

    if (TYPES_DIM (full_type) > SCALAR) {
        /*
         * full_type is an array with known shape.
         */

        MAKENODE_ID (id_type, type_string[TYPES_BASETYPE (full_type)]);
        /* declared type */
        MAKENODE_ID (id_node, arg_node->ID); /* name of variable */
        MAKENODE_NUM (n_dim, full_type->dim);

        /* now create N_icm */
        MAKE_ICM (assign);
        MAKE_ICM_NAME (assign->node[0], "ND_KS_DECL_ARRAY");

        if (NULL != arg_node->node[0])
            assign->node[1] = arg_node->node[0];

        MAKE_ICM_ARG (assign->node[0]->node[0], id_type);
        icm_arg = assign->node[0]->node[0];
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);
        MAKE_NEXT_ICM_ARG (icm_arg, n_dim);

        for (i = 0; i < full_type->dim; i++) {
            /* the shape information will be converted & added */
            MAKENODE_NUM (n_node, full_type->shpseg->shp[i]);
            MAKE_NEXT_ICM_ARG (icm_arg, n_node);
        }

        /* now free some nodes */
        if (T_user == arg_node->SIMPLETYPE) {
            FREE_TYPE (full_type);
        }

        FREE_VARDEC (arg_node);
        arg_node = assign; /* set arg_node, because this node will be returned */

        if (NULL != arg_node->node[1]) {
            arg_node->node[1] = Trav (arg_node->node[1], NULL);
#ifndef NEWTREE
            if (NULL == arg_node->node[1])
                arg_node->nnode = 1;
            else
                arg_node->nnode = 2;
#endif /* NEWTREE */
        }
    } else if (TYPES_DIM (full_type) == UNKNOWN_SHAPE) {
        /*
         *  full_type is an array with unknown shape and dimension.
         */

        MAKENODE_ID (id_type, type_string[TYPES_BASETYPE (full_type)]);
        /* declared type */
        MAKENODE_ID (id_node, arg_node->ID); /* name of variable */

        /* now create N_icm */
        MAKE_ICM (assign);
        MAKE_ICM_NAME (assign->node[0], "ND_DECL_ARRAY");

        if (NULL != arg_node->node[0])
            assign->node[1] = arg_node->node[0];

        MAKE_ICM_ARG (assign->node[0]->node[0], id_type);
        icm_arg = assign->node[0]->node[0];
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);

        /* now free some nodes */
        if (T_user == arg_node->SIMPLETYPE) {
            FREE_TYPE (full_type);
        }

        FREE_VARDEC (arg_node);
        arg_node = assign; /* set arg_node, because this node will be returned */

        if (NULL != arg_node->node[1]) {
            arg_node->node[1] = Trav (arg_node->node[1], NULL);
#ifndef NEWTREE
            if (NULL == arg_node->node[1])
                arg_node->nnode = 1;
            else
                arg_node->nnode = 2;
#endif /* NEWTREE */
        }
    } else if (TYPES_DIM (full_type) < KNOWN_DIM_OFFSET) {
        /*
         *  full_type is an array with unknown shape and known dimension.
         */

        MAKENODE_ID (id_type, type_string[TYPES_BASETYPE (full_type)]);
        /* declared type */
        MAKENODE_ID (id_node, arg_node->ID); /* name of variable */
        MAKENODE_NUM (n_dim, KNOWN_DIM_OFFSET - full_type->dim);

        /* now create N_icm */
        MAKE_ICM (assign);
        MAKE_ICM_NAME (assign->node[0], "ND_KD_DECL_ARRAY");

        if (NULL != arg_node->node[0])
            assign->node[1] = arg_node->node[0];

        MAKE_ICM_ARG (assign->node[0]->node[0], id_type);
        icm_arg = assign->node[0]->node[0];
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);
        MAKE_NEXT_ICM_ARG (icm_arg, n_dim);

        /* now free some nodes */
        if (T_user == arg_node->SIMPLETYPE) {
            FREE_TYPE (full_type);
        }

        FREE_VARDEC (arg_node);
        arg_node = assign; /* set arg_node, because this node will be returned */

        if (NULL != arg_node->node[1]) {
            arg_node->node[1] = Trav (arg_node->node[1], NULL);
#ifndef NEWTREE
            if (NULL == arg_node->node[1])
                arg_node->nnode = 1;
            else
                arg_node->nnode = 2;
#endif /* NEWTREE */
        }
    } else {
        if (IsNonUniqueHidden (VARDEC_TYPE (arg_node))) {
            CREATE_2_ARY_ICM (assign, "ND_DECL_RC", MAKE_IDNODE ("void*"),
                              MAKE_IDNODE (StringCopy (VARDEC_NAME (arg_node))));

            if (VARDEC_NEXT (arg_node) != NULL) {
                ASSIGN_NEXT (assign) = Trav (VARDEC_NEXT (arg_node), NULL);

#ifndef NEWTREE
                if (ASSIGN_NEXT (assign) == NULL)
                    assign->nnode = 1;
                else
                    assign->nnode = 2;
#endif /* NEWTREE */
            }
#ifndef NEWTREE
            else
                assign->nnode = 1;
#endif /* NEWTREE */
            FREE_VARDEC (arg_node);
            arg_node = assign;
        } else {
            if (arg_node->DIM < 0) {
                /* current vardec-node has unknown shape and will be removed */
                node *tmp;
                tmp = arg_node;
#ifndef NEWTREE
                if (1 == arg_node->nnode)
#else  /* NEWTREE */
                if (arg_node->node[0] != NULL)
#endif /* NEWTREE */
                    arg_node = Trav (arg_node->node[0], NULL);
                else
                    arg_node = NULL;
                FREE_VARDEC (tmp);
            } else {
                /* traverse next N_vardec node if any */
#ifndef NEWTREE
                if (1 == arg_node->nnode)
#else  /* NEWTREE */
                if (arg_node->node[0] != NULL)
#endif /* NEWTREE */
                {
                    arg_node->node[0] = Trav (arg_node->node[0], NULL);
#ifndef NEWTREE
                    if (NULL == arg_node->node[0])
                        arg_node->nnode = 0;
#endif /* NEWTREE */
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompPrfModarray
 *  arguments     : 1) N_prf node
 *                  2)
 *  description   : transforms N_prf node F_modarray to N_icm nodes
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to node before
 *                   last assign_node (to get last or next assign node use
 *                                     macros LAST_ASSIGN or NEXT_ASSIGN )
 *                  arg_info->node[0] is used to insert new assign_nodes
 *                   in front of or after last assign node
 *                  arg_info->node[1] contains pointer to last N_let
 */
node *
CompPrfModarray (node *arg_node, node *arg_info)
{
    node *res, *length, *n_node, *res_ref, *type_id_node, *dim_res, *line, *first_assign,
      *next_assign, *icm_arg, *old_arg_node, *last_assign;
    node *arg1 = PRF_ARG1 (arg_node);
    node *arg2 = PRF_ARG2 (arg_node);
    node *arg3 = PRF_ARG3 (arg_node);

    int n, dim;
    simpletype s_type;

    DBUG_ENTER ("CompPrfModarray");

    MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* store dimension of result */
    GET_DIM (dim, arg_info->IDS_NODE->TYPES);
    MAKENODE_NUM (dim_res, dim);

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));

    /* store line of prf function */
    MAKENODE_NUM (line, arg_node->lineno);

    if (NODE_TYPE (arg2) == N_array) /* index is constant! */
    {
        if (NODE_TYPE (arg3) == N_array) { /* value is constant! */
            DBUG_ASSERT (0, "sorry compilation of ND_PRF_MODARRAY_AxCxC not yet done");
        } else {
            COUNT_ELEMS (n, ARRAY_AELEMS (arg2));
            MAKENODE_NUM (length, n);

            if ((N_id == arg3->nodetype) ? (1 == IsArray (arg3->IDS_NODE->TYPES)) : 0) {
                char *icm_name;

                if (1 == arg1->refcnt)
                    icm_name = "ND_PRF_MODARRAY_AxCxA_CHECK_REUSE";
                else
                    icm_name = "ND_PRF_MODARRAY_AxCxA";

                BIN_ICM_REUSE (arg_info->node[1], icm_name, line, type_id_node);
                MAKE_NEXT_ICM_ARG (icm_arg, dim_res);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                MAKE_NEXT_ICM_ARG (icm_arg, arg3);
                MAKE_NEXT_ICM_ARG (icm_arg, length);
                icm_arg->node[1] = ARRAY_AELEMS (arg2);
#ifndef NEWTREE
                icm_arg->nnode = 2;
#endif
                SET_VARS_FOR_MORE_ICMS;
                MAKENODE_NUM (n_node, 1);
                DEC_OR_FREE_RC_ND (arg3, n_node);
            } else {
                char *icm_name;

                if (1 == arg1->refcnt)
                    icm_name = "ND_PRF_MODARRAY_AxCxS_CHECK_REUSE";
                else
                    icm_name = "ND_PRF_MODARRAY_AxCxS";

                BIN_ICM_REUSE (arg_info->node[1], icm_name, line, type_id_node);
                MAKE_NEXT_ICM_ARG (icm_arg, dim_res);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                MAKE_NEXT_ICM_ARG (icm_arg, arg3);
                MAKE_NEXT_ICM_ARG (icm_arg, length);
                icm_arg->node[1] = ARRAY_AELEMS (arg2);
#ifndef NEWTREE
                icm_arg->nnode = 2;
#endif
                SET_VARS_FOR_MORE_ICMS;
            }
            MAKENODE_NUM (n_node, 1);
            INC_RC_ND (res, res_ref);
            DEC_OR_FREE_RC_ND (arg1, n_node);
            INSERT_ASSIGN;
        }
    } else { /* index is a variable ! */
        DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                     "wrong 2nd arg of modarray (neither N_array nor N_id!");

        if (NODE_TYPE (arg3) == N_array) { /* value is constant! */
            DBUG_ASSERT (0, "sorry compilation of ND_PRF_MODARRAY_AxVxC not yet done");
        } else {
            DBUG_ASSERT (((TYPES_DIM (ID_TYPE (arg2)) == 1)
                          && (TYPES_BASETYPE (ID_TYPE (arg2)) == T_int)),
                         "indexing var of wrong type as 2nd arg of modarray!");

            length = MakeNum (TYPES_SHAPE (ID_TYPE (arg2), 0));

            if ((N_id == NODE_TYPE (arg3)) ? (1 == IsArray (ID_TYPE (arg3))) : 0) {
                char *icm_name;

                if (1 == arg1->refcnt)
                    icm_name = "ND_PRF_MODARRAY_AxVxA_CHECK_REUSE";
                else
                    icm_name = "ND_PRF_MODARRAY_AxVxA";

                BIN_ICM_REUSE (arg_info->node[1], icm_name, line, type_id_node);
                MAKE_NEXT_ICM_ARG (icm_arg, dim_res);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                MAKE_NEXT_ICM_ARG (icm_arg, arg3);
                MAKE_NEXT_ICM_ARG (icm_arg, length);
                MAKE_NEXT_ICM_ARG (icm_arg, arg2);
                SET_VARS_FOR_MORE_ICMS;
                MAKENODE_NUM (n_node, 1);
                DEC_OR_FREE_RC_ND (arg3, n_node);
            } else {
                char *icm_name;

                if (1 == arg1->refcnt)
                    icm_name = "ND_PRF_MODARRAY_AxVxS_CHECK_REUSE";
                else
                    icm_name = "ND_PRF_MODARRAY_AxVxS";

                BIN_ICM_REUSE (arg_info->node[1], icm_name, line, type_id_node);
                MAKE_NEXT_ICM_ARG (icm_arg, dim_res);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                MAKE_NEXT_ICM_ARG (icm_arg, arg3);
                MAKE_NEXT_ICM_ARG (icm_arg, length);
                MAKE_NEXT_ICM_ARG (icm_arg, arg2);
                SET_VARS_FOR_MORE_ICMS;
            }
            MAKENODE_NUM (n_node, 1);
            INC_RC_ND (res, res_ref);
            DEC_OR_FREE_RC_ND (arg1, n_node);
            INSERT_ASSIGN;
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompIdxModarray
 *  arguments     : 1) N_prf node
 *                  2)
 *  description   : transforms N_prf node F_idx_modarray to N_icm nodes
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to node before
 *                   last assign_node (to get last or next assign node use
 *                                     macros LAST_ASSIGN or NEXT_ASSIGN )
 *                  arg_info->node[0] is used to insert new assign_nodes
 *                   in front of or after last assign node
 *                  arg_info->node[1] contains pointer to last N_let
 */
node *
CompIdxModarray (node *arg_node, node *arg_info)
{
    node *res, *n_node, *res_ref, *type_id_node, *line, *first_assign, *next_assign,
      *icm_arg, *old_arg_node, *last_assign;
    node *arg1 = PRF_ARG1 (arg_node);
    node *arg2 = PRF_ARG2 (arg_node);
    node *arg3 = PRF_ARG3 (arg_node);

    simpletype s_type;

    DBUG_ENTER ("CompIdxModarray");

    MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));

    /* store line of prf function */
    MAKENODE_NUM (line, arg_node->lineno);

    /* index is a variable ! */
    DBUG_ASSERT ((NODE_TYPE (arg2) == N_id),
                 "wrong 2nd arg of modarray (neither N_array nor N_id!");

    if (NODE_TYPE (arg3) == N_array) { /* value is constant! */
        DBUG_ASSERT (0, "sorry compilation of ND_IDX_MODARRAY_AxVxC not yet done");
    } else {
        DBUG_ASSERT (((TYPES_DIM (ID_TYPE (arg2)) == 0)
                      && (TYPES_BASETYPE (ID_TYPE (arg2)) == T_int)),
                     "indexing var of wrong type as 2nd arg of modarray!");

        if ((N_id == NODE_TYPE (arg3)) ? (1 == IsArray (ID_TYPE (arg3))) : 0) {
            char *icm_name;

            if (1 == arg1->refcnt)
                icm_name = "ND_IDX_MODARRAY_AxVxA_CHECK_REUSE";
            else
                icm_name = "ND_IDX_MODARRAY_AxVxA";

            BIN_ICM_REUSE (arg_info->node[1], icm_name, line, type_id_node);
            MAKE_NEXT_ICM_ARG (icm_arg, res);
            MAKE_NEXT_ICM_ARG (icm_arg, arg1);
            MAKE_NEXT_ICM_ARG (icm_arg, arg2);
            MAKE_NEXT_ICM_ARG (icm_arg, arg3);
            SET_VARS_FOR_MORE_ICMS;
            MAKENODE_NUM (n_node, 1);
            DEC_OR_FREE_RC_ND (arg3, n_node);
        } else {
            char *icm_name;

            if (1 == arg1->refcnt)
                icm_name = "ND_IDX_MODARRAY_AxVxS_CHECK_REUSE";
            else
                icm_name = "ND_IDX_MODARRAY_AxVxS";

            BIN_ICM_REUSE (arg_info->node[1], icm_name, line, type_id_node);
            MAKE_NEXT_ICM_ARG (icm_arg, res);
            MAKE_NEXT_ICM_ARG (icm_arg, arg1);
            MAKE_NEXT_ICM_ARG (icm_arg, arg2);
            MAKE_NEXT_ICM_ARG (icm_arg, arg3);
            SET_VARS_FOR_MORE_ICMS;
        }
        MAKENODE_NUM (n_node, 1);
        INC_RC_ND (res, res_ref);
        DEC_OR_FREE_RC_ND (arg1, n_node);
        INSERT_ASSIGN;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompConvert
 *  arguments     : 1) N_prf nodei (F_toi, F_tod, F_tof, F_toi_A, F_tof_A,
 *                                  F_tod_A)
 *                  2) NULL
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 */
node *
CompConvert (node *arg_node, node *arg_info)
{
    int convert = 0;

    DBUG_ENTER ("CompConvert");

    switch (PRF_PRF (arg_node)) {
    case F_toi:
    case F_tod:
    case F_tof: {
#ifndef NOFREE
        node *dummy = arg_node;
#endif /* NOFREE */

        /* return argument of ftoi */
        arg_node = PRF_ARG1 (arg_node);
        FREE (dummy->node[0]); /* free N_exprs node */
        FREE (dummy);          /* free N_prf node */
        break;
    }
    case F_tof_A:
        convert = 1;
        /* here is NO break missing !! */
    case F_tod_A:
        convert = 2;
        /* here is NO break missing !! */
    case F_toi_A: {
        int length;
        node *res_rc, *n_length, *first_assign, *next_assign, *type_id_node,
          *old_arg_node, *last_assign, *res, *icm_arg, *arg1, *n_node;
        simpletype s_type;

        arg1 = PRF_ARG1 (arg_node);
        MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
        /* compute basic type */
        GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
        MAKENODE_ID (type_id_node, type_string[s_type]);
        MAKENODE_NUM (res_rc, IDS_REFCNT (INFO_LASTIDS (arg_info)));
        BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
        MAKE_NEXT_ICM_ARG (icm_arg, res_rc);
        SET_VARS_FOR_MORE_ICMS;

        if (N_id == arg1->nodetype) {
            switch (convert) {
            case 0:
                CREATE_2_ARY_ICM (next_assign, "ND_2I_A", arg1, res);
                break;
            case 1:
                CREATE_2_ARY_ICM (next_assign, "ND_2F_A", arg1, res);
                break;
            case 2:
                CREATE_2_ARY_ICM (next_assign, "ND_2D_A", arg1, res);
                break;
            default:
                DBUG_ASSERT (0, "wrong tag (convert)");
                break;
            }
            APPEND_ASSIGNS (first_assign, next_assign);

            MAKENODE_NUM (n_node, 1);
            DEC_OR_FREE_RC_ND (arg1, n_node);
            INSERT_ASSIGN;
        } else {
            DBUG_ASSERT (N_array == arg1->nodetype, "wrong node != N_array");
            DBUG_ASSERT (NULL != arg1->TYPES, " info.types is NULL");
            COUNT_ELEMS (length, arg1->node[0]);
            MAKENODE_NUM (n_node, length);
            if (1 < arg1->DIM) {
                node *dummy;
                /* it is an array of arrays, so we have to use
                 * ND_CREATE_CONST_ARRAY_A
                 */
                DBUG_ASSERT (N_id == arg1->node[0]->node[0]->nodetype,
                             "wrong node != N_id");
                GET_LENGTH (length, arg1->node[0]->node[0]->IDS_NODE->TYPES);
                MAKENODE_NUM (n_length, length);

                CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_A", res, n_length,
                                  n_node);
                icm_arg->node[1] = arg1->node[0];
#ifndef NEWTREE
                icm_arg->nnode = 2;
#endif
                APPEND_ASSIGNS (first_assign, next_assign);

                /* now decrement refcount of the arrays */
                dummy = arg1->node[0];
                MAKENODE_NUM (n_node, 1);
                while (NULL != dummy) {
                    DBUG_ASSERT (N_id == dummy->node[0]->nodetype,
                                 "wrong nodetype != N_id");
                    DEC_OR_FREE_RC_ND (dummy->node[0], n_node);
                    dummy = dummy->node[1];
                }
            } else {
                /* it is an array out of scalar values */
                CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res, n_node);
                icm_arg->node[1] = arg1->node[0];
#ifndef NEWTREE
                icm_arg->nnode = 2;
#endif
                APPEND_ASSIGNS (first_assign, next_assign);
            }
            INSERT_ASSIGN;
        }
        break;
    }
    default:
        /*   DBUG_ASSERT(0,"wrong prf"); */
        break;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompPrf
 *  arguments     : 1) N_prf node
 *                  2) NULL
 *  description   : transforms N_prf node to N_icm nodes if prf works on
 *                  arrays
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG..., ELIMINATE_CAST
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to node before
 *                   last assign_node (to get last or next assign node use
 *                                     macros LAST_ASSIGN or NEXT_ASSIGN )
 *                  arg_info->node[0] is used to insert new assign_nodes
 *                   in front of or after last assign node
 *                  arg_info->node[1] contains pointer to last N_let
 */
node *
CompPrf (node *arg_node, node *arg_info)
{
    node *array, *scalar, *tmp, *res, *res_ref, *n_node, *icm_arg, *prf_id_node,
      *type_id_node, *arg1, *arg2, *arg3, *n_node1, *n_elems_node, *first_assign,
      *next_assign, *last_assign, *length_node, *tmp_array1, *tmp_array2, *dim_node,
      *tmp_rc, *exprs;
    node *old_arg_node;
    simpletype res_stype = LET_BASETYPE (arg_info->node[1]);
    int dim, is_SxA = 0, n_elems = 0, is_drop = 0, array_is_const = 0, convert = 0;
    simpletype s_type;

    DBUG_ENTER ("CompPrf");

    DBUG_PRINT ("COMP", ("%s line: %d", mdb_prf[arg_node->info.prf], arg_node->lineno));

    /* first eliminate N_cast node form arguments of primitive function */
    exprs = arg_node->node[0];

    do {
        ELIMINATE_CAST (exprs);
        exprs = exprs->node[1];
    } while (NULL != exprs);

    /* NOTE :  F_neq should be the last "function enumerator" that hasn't
     *         arrays as arguments.
     */
    if (arg_node->info.prf > F_neq) {
        ids *let_ids = arg_info->IDS;
        node *new_name, *new_assign, *old_name;
        int insert_assign = 0;

        exprs = PRF_ARGS (arg_node);
        /* test whether an identifier occures on the right and left side of a
         * let. In this case rename the one on the rigth side ,assign old and new
         * variable and add vardec for the new vaibale.
         * (e.g: A=A+1 => __A=A; A=__A+1; )
         */
        while (NULL != exprs) {
            if (N_id == exprs->node[0]->nodetype)
                if (0 == strcmp (let_ids->id, exprs->node[0]->IDS_ID)) {
                    if (0 == insert_assign) {
                        MAKENODE_ID (new_name, RenameVar (let_ids->id, 0));
                        MAKENODE_ID_REUSE_IDS (old_name, let_ids);
                        CREATE_2_ARY_ICM (new_assign, "ND_KS_ASSIGN_ARRAY", old_name,
                                          new_name);
                        new_assign->node[1] = LAST_ASSIGN (arg_info);
#ifndef NEWTREE
                        new_assign->nnode = 2;
#endif
                        INSERT_BEFORE (arg_info, new_assign);

                        /* set info_node to right node (update info_node )*/
                        arg_info->node[0] = new_assign;

                        insert_assign = 1;

                        /* now insert vardec if necessary */
                        arg_info->node[3]
                          = AddVardec (arg_info->node[3], IDS_VARDEC_TYPE (let_ids),
                                       ID_NAME (new_name));
                    }

                    /* now rename N_id */
                    FREE (exprs->node[0]->IDS_ID);
                    exprs->node[0]->IDS_ID = RenameVar (let_ids->id, 0);
                }

            exprs = exprs->node[1];
        }
        switch (arg_node->info.prf) {
        case F_modarray:
            arg_node = CompPrfModarray (arg_node, arg_info);
            break;
        case F_idx_modarray:
            arg_node = CompIdxModarray (arg_node, arg_info);
            break;
        case F_add_SxA:
        case F_div_SxA:
        case F_sub_SxA:
        case F_mul_SxA: {
            is_SxA = 1;
            /* here is NO break missing */
        }
        case F_add_AxS:
        case F_div_AxS:
        case F_sub_AxS:
        case F_mul_AxS: {
            simpletype array_stype;

            /* store arguments and result (as N_id)  */
            if (0 == is_SxA) {
                array = arg_node->node[0]->node[0];
                GET_BASIC_SIMPLETYPE_OF_NODE (array_stype, array);
                scalar = arg_node->node[0]->node[1]->node[0];
            } else {
                array = arg_node->node[0]->node[1]->node[0];
                GET_BASIC_SIMPLETYPE_OF_NODE (array_stype, array);
                scalar = arg_node->node[0]->node[0];
            }
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            /* store prf as N_id */
            MAKENODE_ID (prf_id_node, prf_string[arg_node->info.prf]);

            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));

            if (N_id == array->nodetype) {
                last_assign = NEXT_ASSIGN (arg_info);

                CHECK_REUSE__ALLOC_ARRAY_ND (res, res_stype, array, array_stype);
            } else {
                /* array is constant, so make a block , declare a temporary
                 * variable __TMP and create a constant array
                 */

                array_is_const = 1;
                old_arg_node = arg_node;

                /* count number of elements */
                tmp = array->node[0];
                while (NULL != tmp) {
                    n_elems += 1;
                    tmp = tmp->node[1];
                }

                MAKENODE_NUM (n_node, n_elems);
                MAKENODE_NUM (n_node1, 1);
                MAKENODE_ID (tmp_array1, "__TMP");

                arg_info->node[1]->nodetype = N_block; /* reuse previous N_let*/
                CREATE_4_ARY_ICM (first_assign, "ND_KS_DECL_ARRAY", type_id_node,
                                  tmp_array1, n_node1, n_node);
                arg_node = first_assign;

                /* create const array */
                CREATE_CONST_ARRAY (array, tmp_array1, type_id_node, res_ref);

                /* reuse temporary array __TMP */
                CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array1, res);
                APPEND_ASSIGNS (first_assign, next_assign);

                array = tmp_array1; /* set array to __TMP */
            }

            if (0 == is_SxA) {
                /* create ND_BINOP_AxS_A */
                CREATE_4_ARY_ICM (next_assign, "ND_BINOP_AxS_A", prf_id_node, array,
                                  scalar, res);
            } else {
                /* create ND_BINOP_SxA_A */
                CREATE_4_ARY_ICM (next_assign, "ND_BINOP_SxA_A", prf_id_node, scalar,
                                  array, res);
            }
            APPEND_ASSIGNS (first_assign, next_assign);

            if (0 == array_is_const) {
                if (0 < res_ref->info.cint) {
                    /* create ND_INC_RC */
                    CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", res, res_ref);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* create ND_DEC_RC */
                    MAKENODE_NUM (n_node, 1);
                    DEC_OR_FREE_RC_ND (array, n_node);
                } else {
                    /* create ND_DEC_RC_FREE_ARRAY */
                    MAKENODE_NUM (n_node, 1);
                    DEC_OR_FREE_RC_ND (array, n_node);
                }
            }

            if (0 == array_is_const) {
                INSERT_ASSIGN;
            }
            FREE (old_arg_node);
            break;
        }
        case F_add_AxA:
        case F_sub_AxA:
        case F_mul_AxA:
        case F_div_AxA: {
            simpletype arg1_stype, arg2_stype;

            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            /* store prf as N_id */
            MAKENODE_ID (prf_id_node, prf_string[arg_node->info.prf]);

            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* compute basic_type of arg1 and arg2 */
            GET_BASIC_SIMPLETYPE_OF_NODE (arg1_stype, arg1);
            GET_BASIC_SIMPLETYPE_OF_NODE (arg2_stype, arg2);

            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));

            if ((N_id == arg1->nodetype) && (N_id == arg2->nodetype)) {
                last_assign = NEXT_ASSIGN (arg_info);
                if ((1 == arg1->refcnt) && (1 == arg2->refcnt)) {
                    node *num;

                    CHECK_REUSE (arg1);
                    CREATE_2_ARY_ICM (next_assign, "ND_CHECK_REUSE_ARRAY", arg2, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    MAKENODE_NUM (num, 0);
                    CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res,
                                      num);
                    APPEND_ASSIGNS (first_assign, next_assign);
                } else if (1 == arg1->refcnt) {
                    CHECK_REUSE__ALLOC_ARRAY_ND (res, res_stype, arg1, arg1_stype);
                } else if (1 == arg2->refcnt) {
                    CHECK_REUSE__ALLOC_ARRAY_ND (res, res_stype, arg2, arg2_stype);
                } else {
                    node *num;

#if 0
	      first_assign=LAST_ASSIGN(arg_info);
#else
                    MAKENODE_NUM (num, 0);
                    TRI_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res,
                                   num);
                    first_assign = LAST_ASSIGN (arg_info);
                    old_arg_node = arg_node;
                    last_assign = NEXT_ASSIGN (arg_info);
                    arg_node = arg_info->node[1]->node[0];

#endif
                    DBUG_ASSERT ((((-1 == arg1->refcnt) && (-1 == arg2->refcnt))
                                  || (opt_rco == 0)),
                                 "Refcnt of BINOP_A_A arg neither -1 nor 1 !");
                }
            } else {
                arg_info->node[1]->nodetype = N_block; /*  reuse previous N_let*/
                old_arg_node = arg_node;
                if ((N_array == arg1->nodetype) && (N_array == arg2->nodetype)) {
                    array_is_const = 3;
                    DECL_ARRAY (first_assign, arg1->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign; /* set new arg_node */
                    DECL_ARRAY (next_assign, arg2->node[0], "__TMP2", tmp_array2);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    CREATE_CONST_ARRAY (arg1, tmp_array1, type_id_node, res_ref);
                    MAKENODE_NUM (n_node1, 0);
                    CREATE_CONST_ARRAY (arg2, tmp_array2, type_id_node, n_node1);
                    CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array1, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* set arg1 and arg2 for later use as parameters of BIN_OP */
                    arg1 = tmp_array1;
                    arg2 = tmp_array2;
                } else if (N_array == arg1->nodetype) {
                    array_is_const = 1;
                    DECL_ARRAY (first_assign, arg1->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg1, tmp_array1, type_id_node, res_ref);
                    CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array1, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* set arg1 for later use as parameters of BIN_OP */
                    arg1 = tmp_array1;
                } else {
                    array_is_const = 2;
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP2", tmp_array2);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg2, tmp_array2, type_id_node, res_ref);
                    CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array2, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    /* set arg2 for later use as parameters of BIN_OP */
                    arg2 = tmp_array2;
                }
            }
            CREATE_4_ARY_ICM (next_assign, "ND_BINOP_AxA_A", prf_id_node, arg1, arg2,
                              res);
            APPEND_ASSIGNS (first_assign, next_assign);

            MAKENODE_NUM (n_node, 1);
            switch (array_is_const) {
            case 0:
                CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", res, res_ref);
                APPEND_ASSIGNS (first_assign, next_assign);
                DEC_OR_FREE_RC_ND (arg2, n_node);
                DEC_OR_FREE_RC_ND (arg1, n_node);
                break;
            case 1:
                CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY", arg2, n_node);
                APPEND_ASSIGNS (first_assign, next_assign);
                break;
            case 2:
                CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY", arg1, n_node);
                APPEND_ASSIGNS (first_assign, next_assign);
                break;
            case 3:
                CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY", arg2, n_node);
                APPEND_ASSIGNS (first_assign, next_assign);
                break;
            default:
                DBUG_ASSERT (0, "array_is_const is out of range");
                break;
            }

            if (0 == array_is_const) {
                INSERT_ASSIGN;
            }
            FREE (old_arg_node);

            break;
        }
        case F_drop:
            is_drop = 1;
            /* here is NO break missing */

        case F_take: {
            node *num;

            /* store arguments and result (res contains refcount and pointer to
             * vardec ( don't free arg_info->IDS !!! )
             *
             * if first argument of prf is a scalar (N_um), it will be compiled
             * like an vector (array) with one element
             */
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            DBUG_ASSERT (((N_array == arg1->nodetype) || (N_num == arg1->nodetype)),
                         "first argument of take/drop isn't an array or scalar");

            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));

            MAKENODE_NUM (num, 0);
            if (N_id == arg2->nodetype) {
                GET_DIM (dim, arg2->IDS_NODE->TYPES);
                MAKENODE_NUM (dim_node, dim); /* store dimension of argument-array */
                BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, num);
                SET_VARS_FOR_MORE_ICMS;
            } else {
                DBUG_ASSERT ((N_array == arg2->nodetype), "wrong nodetype");

                MAKENODE_NUM (dim_node, 1); /* store dimension of argument-array */
                CREATE_TMP_CONST_ARRAY (arg2, res_ref);
                CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res, num);
                APPEND_ASSIGNS (first_assign, next_assign);
            }

            if (N_array == arg1->nodetype) {
                exprs = arg1->node[0];
                n_elems = 0;
                do {
                    n_elems += 1;
                    exprs = exprs->node[1];
                } while (NULL != exprs);
            } else
                n_elems = 1;

            MAKENODE_NUM (n_node, n_elems);
            if (1 == is_drop) {
                CREATE_4_ARY_ICM (next_assign, "ND_KD_DROP_CxA_A", dim_node, arg2, res,
                                  n_node);
            } else {
                CREATE_4_ARY_ICM (next_assign, "ND_KD_TAKE_CxA_A", dim_node, arg2, res,
                                  n_node);
            }
            if (N_num == arg1->nodetype) {
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
            } else {
                icm_arg->node[1] = arg1->node[0];
#ifndef NEWTREE
                icm_arg->nnode = 2;
#endif
            }
            APPEND_ASSIGNS (first_assign, next_assign);

            MAKENODE_NUM (n_node, 1);
            if (0 == array_is_const) {
                DEC_OR_FREE_RC_ND (arg2, n_node);
                INC_RC_ND (res, res_ref);
            }
            if (0 == array_is_const) {
                INSERT_ASSIGN;
            }
            FREE (old_arg_node);
            break;
        }
        case F_reshape: {
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            FREE_TREE (arg1);
            FREE (arg_node->node[0]);
            if (N_array == arg2->nodetype) {
                arg_node = CompArray (arg2, arg_info);
            } else {
                DBUG_ASSERT ((N_id == arg2->nodetype), "wrong nodetype");
                if (0 == strcmp (arg2->IDS_ID, arg_info->IDS_ID)) {
                    FREE_IDS (arg2->IDS);
                    FREE (arg2);
                    FREE (arg_node->node[0]->node[1]);
                    arg_node = NULL;
                    arg_info->node[1]->nodetype = N_icm;
#ifndef NEWTREE
                    arg_info->node[1]->nnode = 0;
#else
                    /*
                    for (int i=0; i<MAX_SONS; i++)
                      arg_info->node[1]->node[i] = NULL;
                    */
#endif
                    /* don't free  arg_info->node[1]->IDS, because ..->IDS_ID
                       is shared with vardec  FREE_IDS(arg_info->node[1]->IDS);
                       */
                    MAKE_ICM_NAME (arg_info->node[1], "NOOP");
                } else {
                    MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
                    BIN_ICM_REUSE (arg_info->node[1], "ND_KS_ASSIGN_ARRAY", arg2, res);
                    SET_VARS_FOR_MORE_ICMS;
                    if (ID_REFCNT (res) > 1) {
                        MAKENODE_NUM (n_node, ID_REFCNT (res) - 1);
                        INC_RC_ND (res, n_node);
                        INSERT_ASSIGN;
                    } else if (0 == ID_REFCNT (res)) {
                        MAKENODE_NUM (n_node, 1);
                        DEC_OR_FREE_RC_ND (res, n_node);
                        INSERT_ASSIGN;
                    }
                    FREE (old_arg_node);
                }
            }
            break;
        }
        case F_psi: {
            /* store arguments and result (res contains refcount and pointer to
             * vardec ( don't free arg_info->IDS !!! )
             */
            node *line, *arg1_length;

            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
            MAKENODE_NUM (line, arg_node->lineno);

            last_assign = NEXT_ASSIGN (arg_info);

            /* compute basic type */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* compute length of arg1 */
            if (N_id == arg1->nodetype) {
                GET_LENGTH (n_elems, arg1->IDS_NODE->TYPES);
                MAKENODE_NUM (arg1_length, n_elems);
            } else {
                n_elems = 0;
                tmp = arg1->node[0];
                do {
                    n_elems += 1;
                    tmp = tmp->node[1];
                } while (NULL != tmp);
                MAKENODE_NUM (arg1_length, n_elems);
            }

            if (0 == IsArray (res->IDS_NODE->TYPES)) {
                if (N_id == arg2->nodetype) {
                    if (N_id == arg1->nodetype) {
                        BIN_ICM_REUSE (arg_info->node[1], "ND_KD_PSI_VxA_S", line, arg2);
                        MAKE_NEXT_ICM_ARG (icm_arg, res);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1_length);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                    } else {
                        BIN_ICM_REUSE (arg_info->node[1], "ND_KD_PSI_CxA_S", line, arg2);
                        MAKE_NEXT_ICM_ARG (icm_arg, res);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1_length);
                        icm_arg->node[1] = arg1->node[0];
#ifndef NEWTREE
                        icm_arg->nnode = 2;
#endif
                        /*
                         * FREE(arg1);
                         *
                         * arg1 cannot be freed here since it is used 6 lines
                         * farther.
                         */
                    }
                    first_assign = LAST_ASSIGN (arg_info);
                    old_arg_node = arg_node;
                    arg_node = arg_info->node[1]->node[0];
                    MAKENODE_NUM (n_node, 1);
                    DEC_OR_FREE_RC_ND (arg2, n_node);
                    if (N_id == arg1->nodetype) {
                        DEC_OR_FREE_RC_ND (arg1, n_node);
                    }

                    INSERT_ASSIGN;

                    FREE (old_arg_node);
                } else {
                    /* arg2 is a constant array */
                    DBUG_ASSERT ((N_array == arg2->nodetype), "nodetype != N_array");
                    MAKENODE_NUM (tmp_rc, 0);
                    CREATE_TMP_CONST_ARRAY (arg2, tmp_rc);
                    if (N_id == arg1->nodetype) {
                        CREATE_5_ARY_ICM (next_assign, "ND_KD_PSI_VxA_S", line, arg2, res,
                                          arg1_length, arg1);
                    } else {
                        CREATE_4_ARY_ICM (next_assign, "ND_KD_PSI_CxA_S", line, arg2, res,
                                          arg1_length);
                        icm_arg->node[1] = arg1->node[0];
#ifndef NEWTREE
                        icm_arg->nnode = 2;
#endif
                        FREE (arg1);
                    }
                    APPEND_ASSIGNS (first_assign, next_assign);
                }
            } else {
                node *num;

                DBUG_ASSERT ((N_id == arg2->nodetype), "arg2 != N_id");
                GET_DIM (dim, arg2->IDS_NODE->TYPES);
                MAKENODE_NUM (dim_node, dim);
                /* store refcount of res as N_num */
                MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));

                MAKENODE_NUM (num, 0);
                BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, num);
                SET_VARS_FOR_MORE_ICMS;

                if (N_id == arg1->nodetype) {
                    CREATE_6_ARY_ICM (next_assign, "ND_KD_PSI_VxA_A", line, dim_node,
                                      arg2, res, arg1_length, arg1);
                } else {
                    CREATE_5_ARY_ICM (next_assign, "ND_KD_PSI_CxA_A", line, dim_node,
                                      arg2, res, arg1_length);
                    icm_arg->node[1] = arg1->node[0];
#ifndef NEWTREE
                    icm_arg->nnode = 2;
#endif
                    FREE (arg1);
                }
                APPEND_ASSIGNS (first_assign, next_assign);

                MAKENODE_NUM (n_node, 1);
                DEC_OR_FREE_RC_ND (arg2, n_node);
                INC_RC_ND (res, res_ref);
                INSERT_ASSIGN;
                FREE (old_arg_node);
            }
            break;
        }
        case F_idx_psi: {
            node *arg2_ref;
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            /*
                    DBUG_ASSERT((N_id == arg1->nodetype ||
                                 N_num == arg1->nodetype),"wrong first arg of idx_psi");
            */
            DBUG_ASSERT (N_id == arg2->nodetype, "wrong second arg of idx_psi");

#ifdef OLD_IDX_PSI
            /* reuse last N_let node */
            arg_info->node[1]->nodetype = N_icm;
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
            if (1 == IsArray (arg_info->IDS_NODE->TYPES)) {
                /*
                   types *b_type;
                   int length, i;
                   node *next_icm_arg;

                   if( T_user == arg_info->IDS_NODE->SIMPLETYPE)
                   {
                     GET_BASIC_TYPE(b_type,arg_info->IDS_NODE->TYPES, 0);
                   }
                   else
                     b_type=arg_info->IDS_NODE->TYPES;

                   length=1;
                   for(i=0; i< b_type->dim; i++)
                     length*=b_type->shpseg->shp[i];
                   MAKENODE_NUM(n_node, length);
                */
                MAKE_ICM_NAME (arg_info->node[1], "ND_IDX_PSI_A");

                MAKE_ICM_ARG (icm_arg, res);
                /*
                   next_icm_arg=icm_arg;
                   MAKE_NEXT_ICM_ARG(next_icm_arg, n_node);
                */
            } else {
                MAKE_ICM_NAME (arg_info->node[1], "ND_IDX_PSI_S");
                MAKE_ICM_ARG (icm_arg, res);
            }
            /* append res to arguments of current node  */
            arg_node->node[0]->node[1]->node[1] = icm_arg;
#ifndef NEWTREE
            arg_node->node[0]->node[1]->nnode = 2;
#endif

            /* set arg_node, because arg_node will be returned */
            old_arg_node = arg_node;
            arg_node = arg_node->node[0];
            FREE (old_arg_node);
            break;
#endif
            MAKENODE_NUM (arg2_ref, 1);
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
            if (1 == IsArray (arg_info->IDS_NODE->TYPES)) {
                MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));
                GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
                MAKENODE_ID (type_id_node, type_string[s_type]);
                BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
                SET_VARS_FOR_MORE_ICMS;
                MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
                CREATE_3_ARY_ICM (next_assign, "ND_IDX_PSI_A", arg1, arg2, res);
                APPEND_ASSIGNS (first_assign, next_assign);
            } else {
                BIN_ICM_REUSE (arg_info->node[1], "ND_IDX_PSI_S", arg1, arg2);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                SET_VARS_FOR_MORE_ICMS;
            }
            DEC_OR_FREE_RC_ND (arg2, arg2_ref);
            INSERT_ASSIGN;
            break;
        }
        case F_dim: {
            arg1 = arg_node->node[0]->node[0];
            arg_node->nodetype = N_num;
            if (N_array == arg1->nodetype)
                arg_node->info.cint = 1;
            else {
                node *id_node;

                GET_DIM (arg_node->info.cint, arg1->IDS_NODE->TYPES);
                first_assign = LAST_ASSIGN (arg_info);
                last_assign = NEXT_ASSIGN (arg_info);
                MAKENODE_NUM (n_node, 1);
                MAKENODE_ID (id_node, arg1->IDS_ID);
                ID_REFCNT (id_node) = ID_REFCNT (arg1);
                DEC_OR_FREE_RC_ND (id_node, n_node);
                INSERT_ASSIGN;
            }
#ifndef NEWTREE
            arg_node->nnode = 0;
#endif
            FREE_TREE (arg_node->node[0]);
            break;
        }
        case F_shape: {
            int dim;
            arg1 = arg_node->node[0]->node[0];
            MAKENODE_ID (type_id_node, "int");          /* store type of new array */
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS); /* store name of new array */
            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));
            BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
            MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
            SET_VARS_FOR_MORE_ICMS;
            if (N_id == arg1->nodetype) {
                GET_DIM (dim, arg1->IDS_NODE->TYPES);
                tmp_array1 = ShapeToArray (arg1->IDS_NODE);
            } else {
                DBUG_ASSERT ((N_array == arg1->nodetype), "wrong nodetype");
                COUNT_ELEMS (n_elems, arg1->node[0]);
                tmp_array1 = MakeNode (N_exprs);
                MAKENODE_NUM (tmp_array1->node[0], n_elems);
#ifndef NEWTREE
                tmp_array1->nnode = 1;
#endif
                dim = 1;
            }
            MAKENODE_NUM (length_node, dim); /* store length of shape_vector */
            CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res, length_node);
            icm_arg->node[1] = tmp_array1; /* append shape_vector */
#ifndef NEWTREE
            icm_arg->nnode = 2;
#endif
            APPEND_ASSIGNS (first_assign, next_assign);
            if (N_id == arg1->nodetype) {
                node *id_node;
                MAKENODE_NUM (n_node, 1);
                MAKENODE_ID (id_node, arg1->IDS_ID);
                ID_REFCNT (id_node) = ID_REFCNT (arg1);

                DEC_OR_FREE_RC_ND (id_node, n_node);
            }
            FREE_TREE (old_arg_node);
            INSERT_ASSIGN;
            break;
        }
        case F_cat: {
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            arg3 = arg_node->node[0]->node[1]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);
            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));

            if ((N_id == arg2->nodetype) && (N_id == arg3->nodetype)) {
                GET_DIM (dim, arg2->IDS_NODE->TYPES);
                MAKENODE_NUM (dim_node, dim);
                BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
                SET_VARS_FOR_MORE_ICMS;
                CREATE_5_ARY_ICM (next_assign, "ND_KD_CAT_SxAxA_A", dim_node, arg2, arg3,
                                  res, arg1);
                APPEND_ASSIGNS (first_assign, next_assign);
                MAKENODE_NUM (n_node, 1);

                DEC_OR_FREE_RC_ND (arg2, n_node);
                DEC_OR_FREE_RC_ND (arg3, n_node);
                INSERT_ASSIGN;

            } else {
                arg_info->node[1]->nodetype = N_block; /*  reuse previous N_let*/
                old_arg_node = arg_node;
                MAKENODE_NUM (tmp_rc, 0);
                if ((N_array == arg2->nodetype) && (N_array == arg3->nodetype)) {
                    array_is_const = 3;
                    GET_DIM (dim, arg2->TYPES);
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign; /* set new arg_node */
                    DECL_ARRAY (next_assign, arg3->node[0], "__TMP2", tmp_array2);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    CREATE_CONST_ARRAY (arg2, tmp_array1, type_id_node, tmp_rc);
                    CREATE_CONST_ARRAY (arg3, tmp_array2, type_id_node, tmp_rc);
                    arg2 = tmp_array1;
                    arg3 = tmp_array2;
                } else if (N_array == arg2->nodetype) {
                    array_is_const = 1;
                    GET_DIM (dim, arg2->TYPES);
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg2, tmp_array1, type_id_node, tmp_rc);
                    /* set arg2 for later use as parameters of ND_KD_CAT*/
                    arg2 = tmp_array1;
                } else {
                    array_is_const = 2;
                    GET_DIM (dim, arg3->TYPES);
                    DECL_ARRAY (first_assign, arg3->node[0], "__TMP2", tmp_array2);
                    arg_node = first_assign;
                    CREATE_CONST_ARRAY (arg3, tmp_array2, type_id_node, tmp_rc);
                    /* set arg3 for later use as parameters of ND_KD_CAT*/
                    arg3 = tmp_array2;
                }
                CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res,
                                  res_ref);
                APPEND_ASSIGNS (first_assign, next_assign);
                MAKENODE_NUM (dim_node, dim);
                CREATE_5_ARY_ICM (next_assign, "ND_KD_CAT_SxAxA_A", dim_node, arg2, arg3,
                                  res, arg1);
                APPEND_ASSIGNS (first_assign, next_assign);
                MAKENODE_NUM (n_node, 1);
                switch (array_is_const) {
                case 1: {
                    DEC_OR_FREE_RC_ND (arg3, n_node);
                    DEC_RC_FREE_ND (arg2, n_node);
                    break;
                }
                case 2: {
                    DEC_OR_FREE_RC_ND (arg3, n_node);
                    DEC_RC_FREE_ND (arg3, n_node);
                    break;
                }
                case 3: {
                    DEC_RC_FREE_ND (arg2, n_node);
                    DEC_RC_FREE_ND (arg3, n_node);
                    break;
                }
                default:
                    DBUG_ASSERT (0, "array_is_const is out of range");
                    break;
                }
                FREE (old_arg_node);
            }
            break;
        }
        case F_rotate: {
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            arg3 = arg_node->node[0]->node[1]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);
            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));
            MAKENODE_NUM (n_node, 1);

            if (N_id == arg3->nodetype) {
                GET_DIM (dim, arg3->IDS_NODE->TYPES); /* dim will be used later */
                BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
                SET_VARS_FOR_MORE_ICMS;
            } else {
                array_is_const = 1;
                arg_info->node[1]->nodetype = N_block; /*  reuse previous N_let*/
                old_arg_node = arg_node;
                GET_DIM (dim, arg3->TYPES); /* dim will be used later */
                DECL_ARRAY (first_assign, arg3->node[0], "__TMP1", tmp_array1);
                arg_node = first_assign;
                CREATE_CONST_ARRAY (arg3, tmp_array1, type_id_node, n_node);
                arg3 = tmp_array1;
                CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res,
                                  res_ref);
                APPEND_ASSIGNS (first_assign, next_assign);
            }
            /* store dimension of arg3 */
            MAKENODE_NUM (dim_node, dim);
            DBUG_ASSERT ((N_num == arg1->nodetype), "wrong 1.arg of 'rotate'");
            CREATE_5_ARY_ICM (next_assign, "ND_KD_ROT_CxSxA_A", arg1, arg2, dim_node,
                              arg3, res);
            APPEND_ASSIGNS (first_assign, next_assign);
            if (0 == array_is_const) {

                DEC_OR_FREE_RC_ND (arg3, n_node);
                INSERT_ASSIGN;
            } else {
                DEC_RC_FREE_ND (arg3, n_node);
                FREE (old_arg_node);
            }
            break;
        }
        case F_itof_A:
            convert = 2;
            /* here is NO break missing !! */
        case F_itod_A:
            convert = 3;
            /* here is NO break missing !! */
        case F_ftod_A:
            convert = 2;
            /* here is NO break missing !! */
        case F_dtoi_A:
            convert = 4;
            /* here is NO break missing !! */
        case F_dtof_A:
            convert = 5;
            /* here is NO break missing !! */
        case F_ftoi_A: {
            int length;
            node *res_rc, *n_length;

            arg1 = arg_node->node[0]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
            /* compute basic type */
            GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
            MAKENODE_ID (type_id_node, type_string[s_type]);
            MAKENODE_NUM (res_rc, IDS_REFCNT (INFO_LASTIDS (arg_info)));
            BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
            MAKE_NEXT_ICM_ARG (icm_arg, res_rc);
            SET_VARS_FOR_MORE_ICMS;

            if (N_id == arg1->nodetype) {
                switch (convert) {
                case 0:
                    CREATE_2_ARY_ICM (next_assign, "ND_F2I_A", arg1, res);
                    break;
                case 1:
                    CREATE_2_ARY_ICM (next_assign, "ND_F2D_A", arg1, res);
                    break;
                case 2:
                    CREATE_2_ARY_ICM (next_assign, "ND_I2F_A", arg1, res);
                    break;
                case 3:
                    CREATE_2_ARY_ICM (next_assign, "ND_I2D_A", arg1, res);
                    break;
                case 4:
                    CREATE_2_ARY_ICM (next_assign, "ND_D2I_A", arg1, res);
                    break;
                case 5:
                    CREATE_2_ARY_ICM (next_assign, "ND_D2F_A", arg1, res);
                    break;
                default:
                    DBUG_ASSERT (0, "wrong tag (convert)");
                    break;
                }
                APPEND_ASSIGNS (first_assign, next_assign);

                MAKENODE_NUM (n_node, 1);
                DEC_OR_FREE_RC_ND (arg1, n_node);
                INSERT_ASSIGN;
            } else {
                DBUG_ASSERT (N_array == arg1->nodetype, "wrong node != N_array");
                DBUG_ASSERT (NULL != arg1->TYPES, " info.types is NULL");
                COUNT_ELEMS (length, arg1->node[0]);
                MAKENODE_NUM (n_node, length);
                if (1 < arg1->DIM) {
                    node *dummy;
                    /* it is an array of arrays, so we have to use
                     * ND_CREATE_CONST_ARRAY_A
                     */
                    DBUG_ASSERT (N_id == arg1->node[0]->node[0]->nodetype,
                                 "wrong node != N_id");
                    GET_LENGTH (length, arg1->node[0]->node[0]->IDS_NODE->TYPES);
                    MAKENODE_NUM (n_length, length);

                    CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_A", res,
                                      n_length, n_node);
                    icm_arg->node[1] = arg1->node[0];
#ifndef NEWTREE
                    icm_arg->nnode = 2;
#endif
                    APPEND_ASSIGNS (first_assign, next_assign);

                    /* now decrement refcount of the arrays */
                    dummy = arg1->node[0];
                    MAKENODE_NUM (n_node, 1);
                    while (NULL != dummy) {
                        DBUG_ASSERT (N_id == dummy->node[0]->nodetype,
                                     "wrong nodetype != N_id");
                        DEC_OR_FREE_RC_ND (dummy->node[0], n_node);
                        dummy = dummy->node[1];
                    }
                } else {
                    /* it is an array out of scalar values */
                    CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res,
                                      n_node);
                    icm_arg->node[1] = arg1->node[0];
#ifndef NEWTREE
                    icm_arg->nnode = 2;
#endif
                    APPEND_ASSIGNS (first_assign, next_assign);
                }
                INSERT_ASSIGN;
            }
            break;
        }
        case F_toi_A:
        case F_tod_A:
        case F_tof_A: {
            arg_node = CompConvert (arg_node, arg_info);
            break;
        }
        default:
            /*   DBUG_ASSERT(0,"wrong prf"); */
            break;
        }
    } else /* (arg_node->info.prf > F_neq) */
      if ((arg_node->info.prf == F_ftoi) || (arg_node->info.prf == F_ftod)
          || (arg_node->info.prf == F_itof) || (arg_node->info.prf == F_itod)
          || (arg_node->info.prf == F_dtof) || (arg_node->info.prf == F_dtoi)) {
#ifndef NOFREE
        node *dummy = arg_node;
#endif /* NOFREE */

        /* return argument of ftoi */
        arg_node = arg_node->node[0]->node[0];
        FREE (dummy->node[0]); /* free N_exprs node */
        FREE (dummy);          /* free N_prf node */
    } else if ((PRF_PRF (arg_node) == F_toi) || (PRF_PRF (arg_node) == F_tof)
               || (PRF_PRF (arg_node) == F_tod))
        arg_node = CompConvert (arg_node, arg_info);

    else if ((PRF_PRF (arg_node) == F_min) || (PRF_PRF (arg_node) == F_max)) {
        /* srs: replace N_prf with icm-macros for min() and max() */
        NODE_TYPE (arg_node) = N_icm;
        ICM_NAME (arg_node) = PRF_PRF (arg_node) == F_min ? "ND_MIN" : "ND_MAX";
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompArray
 *  arguments     : 1) N_arrray node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to node before
 *                    last assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *
 */
node *
CompArray (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *exprs, *res, *type_id_node, *old_arg_node,
      *icm_arg, *n_node, *res_ref, *last_assign;
    int n_elems = 0, icm_created = 0;
    simpletype s_type;

    DBUG_ENTER ("CompArray");

    /* store next assign */
    last_assign = NEXT_ASSIGN (arg_info);

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* store result as N_id */
    MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));

    /* create ND_ALLOC_ARRAY */
    BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
    MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
    first_assign = LAST_ASSIGN (arg_info);
    old_arg_node = arg_node;
    arg_node = arg_info->node[1]->node[0];

    /* create ND_CREATE_CONST_ARRAY */
    exprs = old_arg_node->node[0];
    do {
        n_elems += 1;
        exprs = exprs->node[1];
    } while (NULL != exprs);
    MAKENODE_NUM (n_node, n_elems);
    DBUG_ASSERT (NULL != old_arg_node->node[0], " NULL pointer ");

    if (N_id == old_arg_node->node[0]->node[0]->nodetype) {
        if (1 == IsArray (old_arg_node->node[0]->node[0]->IDS_NODE->TYPES)) {
            node *length;
            int len;
            GET_LENGTH (len, old_arg_node->node[0]->node[0]->IDS_NODE->TYPES);
            MAKENODE_NUM (length, len);
            CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_A", res, length,
                              n_node);
            icm_created = 1;
        } else {
            if (IsNonUniqueHidden (old_arg_node->node[0]->node[0]->IDS_NODE->TYPES)) {
                CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_H", res,
                                  MAKE_IDNODE (
                                    StringCopy (GenericFun (0, old_arg_node->node[0]
                                                                 ->node[0]
                                                                 ->IDS_NODE->TYPES))),
                                  n_node);
                icm_created = 1;
            }
        }
    }

    if (0 == icm_created) {
        CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res, n_node);
    }

    /* now append the elements of the array to the last N_icm */
    icm_arg->node[1] = old_arg_node->node[0];
#ifndef NEWTREE
    icm_arg->nnode = 2;
#endif
    APPEND_ASSIGNS (first_assign, next_assign);

    INSERT_ASSIGN;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompId
 *  arguments     : 1) N_id node
 *                  2) N_info with VARDECS of fun, LASTASSIGN, LASTLET
 *                     and LASTIDS!
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to to node before
 *                   last assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *
 */
node *
CompId (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *last_assign, *old_arg_node, *icm_arg, *res,
      *n_node, *icm_node;

    DBUG_ENTER ("CompId");

    if (MUST_REFCOUNT (ID, arg_node)) {
        if (NULL != arg_info) {
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            if (ID_MAKEUNIQUE (arg_node)) {
                if (ID_REFCNT (arg_node) == 1) {
                    if (IsArray (VARDEC_TYPE (ID_VARDEC (arg_node)))) {
                        TRI_ICM_REUSE (arg_info->node[1], "ND_KS_MAKE_UNIQUE_ARRAY",
                                       arg_node, res,
                                       MakeNum (BasetypeSize (
                                         VARDEC_TYPE (ID_VARDEC (arg_node)))));

                        SET_VARS_FOR_MORE_ICMS;
                        SET_RC_ND (res, MakeNum (IDS_REFCNT (INFO_LASTIDS (arg_info))));
                        INSERT_ASSIGN;
                    } else {
                        TRI_ICM_REUSE (arg_info->node[1], "ND_NO_RC_MAKE_UNIQUE_HIDDEN",
                                       arg_node, res,
                                       MAKE_IDNODE (StringCopy (
                                         GenericFun (0, VARDEC_TYPE (
                                                          ID_VARDEC (arg_node))))));
                    }
                } else {
                    if (IsArray (VARDEC_TYPE (ID_VARDEC (arg_node)))) {
                        TRI_ICM_REUSE (arg_info->node[1], "ND_KS_COPY_ARRAY", arg_node,
                                       res,
                                       MakeNum (BasetypeSize (
                                         VARDEC_TYPE (ID_VARDEC (arg_node)))));

                        SET_VARS_FOR_MORE_ICMS;

                        CREATE_1_ARY_ICM (next_assign, "ND_ALLOC_RC",
                                          MAKE_IDNODE (StringCopy (ID_NAME (res))));
                        APPEND_ASSIGNS (first_assign, next_assign);

                        SET_RC_ND (res, MakeNum (IDS_REFCNT (INFO_LASTIDS (arg_info))));
                        DEC_RC_ND (MAKE_IDNODE (StringCopy (ID_NAME (old_arg_node))),
                                   MakeNum (1));

                        INSERT_ASSIGN;
                    } else {
                        TRI_ICM_REUSE (arg_info->node[1], "ND_COPY_HIDDEN", arg_node, res,
                                       MAKE_IDNODE (StringCopy (
                                         GenericFun (0, VARDEC_TYPE (
                                                          ID_VARDEC (arg_node))))));

                        SET_VARS_FOR_MORE_ICMS;

                        DEC_RC_ND (MAKE_IDNODE (StringCopy (ID_NAME (old_arg_node))),
                                   MakeNum (1));

                        INSERT_ASSIGN;
                    }
                }
            } else {
                if (IsArray (VARDEC_TYPE (ID_VARDEC (arg_node)))) {
                    BIN_ICM_REUSE (arg_info->node[1], "ND_KS_ASSIGN_ARRAY", arg_node,
                                   res);
                } else {
                    BIN_ICM_REUSE (arg_info->node[1], "ND_ASSIGN_HIDDEN", arg_node, res);
                }

                SET_VARS_FOR_MORE_ICMS;

                if (0 == IDS_REFCNT (INFO_LASTIDS (arg_info))) {
                    MAKENODE_NUM (n_node, 1);
                    DEC_RC_FREE_ND (res, n_node);
                } else {
                    if (1 < IDS_REFCNT (INFO_LASTIDS (arg_info))) {
                        MAKENODE_NUM (n_node, IDS_REFCNT (INFO_LASTIDS (arg_info)) - 1);
                        INC_RC_ND (res, n_node);
                    }
                }

                INSERT_ASSIGN;
            }
        } else {
            DBUG_ASSERT ((arg_info != NULL), "corrupted arg-info");

            icm_node = MakeNode (N_icm);
            MAKE_ICM_NAME (icm_node, "ND_KS_RET_ARRAY");
            MAKE_ICM_ARG (icm_node->node[0], arg_node);
#ifndef NEWTREE
            icm_node->nnode = 1;
#endif
            arg_node = icm_node;
        }
    }

    DBUG_RETURN (arg_node);
}
/*
 *
 *  functionname  : CompAp
 *  arguments     : 1) N_Ap node
 *                  2) info node
 *  description   : - creates N_icm for function application
 *                  - insert N_icm to decrement the refcount of functions
 *                    arguments
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to node before last
 *                    assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *                  arg_info->node[3] contains pointer to vardecs
 *
 */
node *
CompAp (node *arg_node, node *arg_info)
{
    node *tmp, *next, *exprs, *icm_arg, *save_icm_arg, *id_node, *tag_node, *next_assign,
      *first_assign, *add_assigns_before, *fundef_args, *add_assigns_after, *last_assign,
      **icm_tab, *icm_tab_entry;
    ids *ids;
    int i, cnt_param, tab_size, ids_for_dots = 0;
    types *fundef_rettypes;
    char *tag;

    DBUG_ENTER ("CompAp");

    ids = LET_IDS (arg_info->node[1]);
    fundef_rettypes = FUNDEF_TYPES (AP_FUNDEF (arg_node));

    add_assigns_before = MakeNode (N_assign);
    add_assigns_after = MakeNode (N_assign);
    /* will be used to store N_icms
     * for incrementation and decrementation
     * of refcounts
     */

    /*
     * Now, insert N_icms for incrementation of refcounts of function results
     * if any
     */
    first_assign = add_assigns_after;

    cnt_param = 0;

    tab_size = CountFunctionParams (AP_FUNDEF (arg_node)) + 2;
    icm_tab = (node **)Malloc (sizeof (node *) * (tab_size));

    for (i = 0; i < tab_size; i++)
        icm_tab[i] = NULL;

    DBUG_PRINT ("COMP", ("Compiling application of function %s",
                         ItemName (AP_FUNDEF (arg_node))));

    while (ids != NULL) {
        /*
         * First, we check if this variable is used as function argument
         * as well.
         */

        DBUG_PRINT ("COMP", ("Handling return value bound to %s", IDS_NAME (ids)));

        tmp = AP_ARGS (arg_node);

        while (tmp != NULL) {
            if ((NODE_TYPE (EXPRS_EXPR (tmp)) == N_id)
                && (0 == strcmp (ID_NAME (EXPRS_EXPR (tmp)), IDS_NAME (ids)))) {
                break;
            }

            tmp = EXPRS_NEXT (tmp);
        }

        if (MUST_REFCOUNT (IDS, ids)) {
            if (FUN_DOES_REFCOUNT (AP_FUNDEF (arg_node), cnt_param)) {
                MAKENODE_ID_REUSE_IDS (id_node, ids);
                MAKENODE_ID (tag_node, "out_rc");

                if (1 < IDS_REFCNT (ids)) {
                    /* create N_icm to increment refcount of function result.
                     * It will be stored in refs_node->node[1]->.. and will be
                     * inserted later.
                     */

                    INC_RC_ND (id_node, MakeNum (ids->refcnt - 1));
                } else {
                    if (0 == IDS_REFCNT (ids)) {
                        if (IsNonUniqueHidden (VARDEC_TYPE (IDS_VARDEC (ids)))) {
                            CREATE_2_ARY_ICM (next_assign, "ND_FREE_HIDDEN",
                                              MAKE_IDNODE (StringCopy (IDS_NAME (ids))),
                                              MAKE_IDNODE (StringCopy (
                                                GenericFun (1, VARDEC_TYPE (
                                                                 IDS_VARDEC (ids))))));
                            APPEND_ASSIGNS (first_assign, next_assign);
                        } else {
                            CREATE_1_ARY_ICM (next_assign, "ND_FREE_ARRAY",
                                              MAKE_IDNODE (StringCopy (IDS_NAME (ids))));
                            APPEND_ASSIGNS (first_assign, next_assign);
                        }
                    }
                }

            } else {
                if ((tmp == NULL) || (IsUnique (VARDEC_TYPE (IDS_VARDEC (ids))))) {
                    MAKENODE_ID_REUSE_IDS (id_node, ids);
                } else {
                    MAKENODE_ID (id_node, TmpVar ());

                    arg_info->node[3]
                      = MakeVardec (StringCopy (ID_NAME (id_node)),
                                    DuplicateTypes (VARDEC_TYPE (IDS_VARDEC (ids)), 1),
                                    arg_info->node[3]);

/*************************************************************/
#ifndef NEWTREE
                    if (VARDEC_NEXT (arg_info->node[3]) == NULL)
                        arg_info->node[3]->nnode = 0;
#endif
                    /*************************************************************/

                    if (IsArray (VARDEC_TYPE (IDS_VARDEC (ids)))) {
                        CREATE_2_ARY_ICM (next_assign, "ND_KS_NO_RC_ASSIGN_ARRAY",
                                          MAKE_IDNODE (StringCopy (ID_NAME (id_node))),
                                          MAKE_IDNODE (StringCopy (IDS_NAME (ids))));
                    } else {
                        CREATE_2_ARY_ICM (next_assign, "ND_NO_RC_ASSIGN_HIDDEN",
                                          MAKE_IDNODE (StringCopy (ID_NAME (id_node))),
                                          MAKE_IDNODE (StringCopy (IDS_NAME (ids))));
                    }

                    APPEND_ASSIGNS (first_assign, next_assign);
                }

                if (IDS_REFCNT (ids) > 0) {
                    CREATE_1_ARY_ICM (next_assign, "ND_ALLOC_RC",
                                      MAKE_IDNODE (StringCopy (IDS_NAME (ids))));
                    APPEND_ASSIGNS (first_assign, next_assign);

                    CREATE_2_ARY_ICM (next_assign, "ND_SET_RC",
                                      MAKE_IDNODE (StringCopy (IDS_NAME (ids))),
                                      MakeNum (IDS_REFCNT (ids)));
                    APPEND_ASSIGNS (first_assign, next_assign);
                } else {
                    if (IsNonUniqueHidden (VARDEC_TYPE (IDS_VARDEC (ids)))) {
                        CREATE_2_ARY_ICM (next_assign, "ND_NO_RC_FREE_HIDDEN",
                                          MAKE_IDNODE (StringCopy (IDS_NAME (ids))),
                                          MAKE_IDNODE (StringCopy (
                                            GenericFun (1, VARDEC_TYPE (
                                                             IDS_VARDEC (ids))))));
                        APPEND_ASSIGNS (first_assign, next_assign);
                    } else {
                        CREATE_1_ARY_ICM (next_assign, "ND_NO_RC_FREE_ARRAY",
                                          MAKE_IDNODE (StringCopy (IDS_NAME (ids))));
                        APPEND_ASSIGNS (first_assign, next_assign);
                    }
                }

                MAKENODE_ID (tag_node, "out");
            }
        } else {
            MAKENODE_ID_REUSE_IDS (id_node, ids);
            MAKENODE_ID (tag_node, "out");
        }

        if (ids_for_dots) {
            MAKE_NEXT_ICM_ARG (icm_arg, tag_node);
            MAKE_NEXT_ICM_ARG (icm_arg, id_node);

            if (IDS_NEXT (ids) == NULL) {
                InsertApDotsParam (icm_tab, icm_tab_entry);
            }
        } else {
            if (TYPES_BASETYPE (fundef_rettypes) == T_dots) {
                ids_for_dots = 1;
                MAKE_ICM_ARG (icm_arg, tag_node);
                icm_tab_entry = icm_arg;
                MAKE_NEXT_ICM_ARG (icm_arg, id_node);

                if (IDS_NEXT (ids) == NULL) {
                    InsertApDotsParam (icm_tab, icm_tab_entry);
                }
                cnt_param++;
            } else {
                MAKE_ICM_ARG (icm_arg, tag_node);
                icm_tab_entry = icm_arg;

                MAKE_NEXT_ICM_ARG (icm_arg, id_node);

                InsertApReturnParam (icm_tab, icm_tab_entry,
                                     VARDEC_TYPE (IDS_VARDEC (ids)),
                                     FUNDEF_PRAGMA (AP_FUNDEF (arg_node)) == NULL
                                       ? NULL
                                       : FUNDEF_LINKSIGN (AP_FUNDEF (arg_node)),
                                     cnt_param);

                fundef_rettypes = TYPES_NEXT (fundef_rettypes);
                cnt_param++;
            }
        }

        ids = IDS_NEXT (ids);
    }

    /*
     *  Now, do the arguments of the function
     */

    exprs = AP_ARGS (arg_node);
    fundef_args = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    ids_for_dots = 0;

    if (cnt_param == 0) {
        cnt_param = 1;
    }

    while (NULL != exprs) {
        DBUG_PRINT ("COMP", ("Handling argument #%d", cnt_param));

        next = EXPRS_NEXT (exprs);

        if ((N_id == exprs->node[0]->nodetype)
            && (MUST_REFCOUNT (ID, EXPRS_EXPR (exprs)))) {
            if (FUN_DOES_REFCOUNT (AP_FUNDEF (arg_node), cnt_param)) {
                tag = ID_ATTRIB (EXPRS_EXPR (exprs)) == ST_inout ? "inout_rc" : "in_rc";
            } else {
                if (ID_ATTRIB (EXPRS_EXPR (exprs)) == ST_inout) {
                    if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_Cfun) {
                        if (IsBoxed (ARG_TYPE (fundef_args)))
                            tag = "upd_bx";
                        else
                            tag = "upd";
                    } else
                        tag = "inout";
                } else
                    tag = "in";

                if (ID_REFCNT (EXPRS_EXPR (exprs)) > 1) {
                    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC",
                                      MAKE_IDNODE (
                                        StringCopy (ID_NAME (EXPRS_EXPR (exprs)))),
                                      MakeNum (1));
                } else {
                    if (IsNonUniqueHidden (
                          VARDEC_TYPE (ID_VARDEC (EXPRS_EXPR (exprs))))) {
                        CREATE_3_ARY_ICM (next_assign, "ND_DEC_RC_FREE_HIDDEN",
                                          MAKE_IDNODE (
                                            StringCopy (ID_NAME (EXPRS_EXPR (exprs)))),
                                          MakeNum (1),
                                          MAKE_IDNODE (StringCopy (
                                            GenericFun (1, VARDEC_TYPE (ID_VARDEC (
                                                             EXPRS_EXPR (exprs)))))));
                    } else {
                        CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY",
                                          MAKE_IDNODE (
                                            StringCopy (ID_NAME (EXPRS_EXPR (exprs)))),
                                          MakeNum (1));
                    }
                }

                ASSIGN_NEXT (next_assign) = ASSIGN_NEXT (add_assigns_after);
                ASSIGN_NEXT (add_assigns_after) = next_assign;

                if (first_assign == add_assigns_after) {
                    first_assign = next_assign;
                }

                /*
                 *  These refcount-ICMs must be inserted at the beginning
                 *  of the new ICMs chain.
                 */

/*****************************************************************/
#ifndef NEWTREE
                if (ASSIGN_NEXT (next_assign) == NULL)
                    next_assign->nnode = 1;
                else
                    next_assign->nnode = 2;
#endif
                /*****************************************************************/
            }
        } else {
            if ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_id)
                && (ID_ATTRIB (EXPRS_EXPR (exprs)) == ST_inout)) {
                if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_Cfun) {
                    if (IsBoxed (ARG_TYPE (fundef_args)))
                        tag = "upd_bx";
                    else
                        tag = "upd";
                } else
                    tag = "inout";
            } else
                tag = "in";
        }

        if (ids_for_dots) {
            icm_arg = save_icm_arg;

            MAKE_NEXT_ICM_ARG (icm_arg, MAKE_IDNODE (tag));
            EXPRS_NEXT (icm_arg) = exprs;
#ifndef NEWTREE
            icm_arg->nnode = 2;
#endif
            EXPRS_NEXT (exprs) = NULL;
#ifndef NEWTREE
            exprs->nnode = 1;
#endif
            icm_arg = exprs;

            save_icm_arg = icm_arg;
            /*
             *  The value of icm_arg has to be saved here because a
             *  very intelligent programmer used this variable implicitly(!!)
             *  in macros like CREATE_3_ARY_ICM.
             */

            if (next == NULL)
                InsertApDotsParam (icm_tab, icm_tab_entry);
        } else {
            if (ARG_BASETYPE (fundef_args) == T_dots) {
                ids_for_dots = 1;

                MAKE_ICM_ARG (icm_arg, MAKE_IDNODE (tag));
                icm_tab_entry = icm_arg;
                EXPRS_NEXT (icm_arg) = exprs;
#ifndef NEWTREE
                icm_arg->nnode = 2;
#endif
                EXPRS_NEXT (exprs) = NULL;
#ifndef NEWTREE
                exprs->nnode = 1;
#endif
                icm_arg = exprs;

                save_icm_arg = icm_arg;

                if (next == NULL)
                    InsertApDotsParam (icm_tab, icm_tab_entry);
            } else {
                MAKE_ICM_ARG (icm_tab_entry, MAKE_IDNODE (tag));
                EXPRS_NEXT (icm_tab_entry) = exprs;
#ifndef NEWTREE
                icm_tab_entry->nnode = 2;
#endif
                EXPRS_NEXT (exprs) = NULL;
#ifndef NEWTREE
                exprs->nnode = 1;
#endif

                InsertApArgParam (icm_tab, icm_tab_entry, ARG_TYPE (fundef_args),
                                  ID_REFCNT (EXPRS_EXPR (exprs)), add_assigns_before,
                                  FUNDEF_PRAGMA (AP_FUNDEF (arg_node)) == NULL
                                    ? NULL
                                    : FUNDEF_LINKSIGN (AP_FUNDEF (arg_node)),
                                  cnt_param);

                fundef_args = ARG_NEXT (fundef_args);
            }
        }

        exprs = next;
        cnt_param++;
    }

    INFO_LASTLET (arg_info)
      = CreateApIcm (INFO_LASTLET (arg_info), FUNDEF_NAME (AP_FUNDEF (arg_node)), icm_tab,
                     tab_size);

    AdjustAddedAssigns (add_assigns_before, add_assigns_after);

    /*
     *  First, new assign-nodes are inserted behind the current one.
     */

    last_assign = LAST_ASSIGN (arg_info);

    if (NULL != ASSIGN_NEXT (add_assigns_after)) {
        ASSIGN_NEXT (first_assign) = NEXT_ASSIGN (arg_info);

#ifndef NEWTREE
        if (NULL != ASSIGN_NEXT (first_assign))
            first_assign->nnode = 2;
        else
            first_assign->nnode = 1;
#endif

        ASSIGN_NEXT (last_assign) = ASSIGN_NEXT (add_assigns_after);

#ifndef NEWTREE
        last_assign->nnode = 2;
#endif
    }

    /*
     *  Second, new assign-nodes are inserted before the current one.
     */

    if (ASSIGN_NEXT (add_assigns_before) != NULL) {
        INSERT_BEFORE (arg_info, ASSIGN_NEXT (add_assigns_before));

        tmp = ASSIGN_NEXT (add_assigns_before);

        while (ASSIGN_NEXT (tmp) != NULL)
            tmp = ASSIGN_NEXT (tmp);

        ASSIGN_NEXT (tmp) = last_assign;
#ifndef NEWTREE
        tmp->nnode = 2;
#endif
    }

    FREE (add_assigns_after);
    FREE (add_assigns_before);
    FREE (icm_tab);

    DBUG_RETURN (ICM_ARGS (INFO_LASTLET (arg_info)));
}

/*
 *
 *  functionname  : CompWithReturn
 *  arguments     : 1) N_return node
 *                  2) info node
 *  description   : generates N_icms of a with-loop
 *  global vars   :
 *  internal funs : RenameReturn
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : if N_return node contains to a with_loop, then
 *                   arg_info->node[2] will point to the first argument(N_exprs)
 *                   of corresponding N_icm for start of with_loop
 *                  arg_info->node[0] contains pointer to node before last
 *                    assign_node
 *
 */
node *
CompWithReturn (node *arg_node, node *arg_info)
{
    node *ret_val, *with_icm_arg, *icm_arg, *index_length, *tmp_with_icm_arg, *dec_rc,
      *res, *res_dim, *exprs, *from, *to, *index_node, *next_assign, *first_assign,
      *n_node, *mod_array = NULL;
    int is_array, con_type = 0;

#define MOD_A 1
#define GEN 2
#define FOLD 3

    /* arg_node is a N_return of a with_loop.
     * arg_info->node[2] points to the N_icm that desribes the begin of
     * a with_loop.
     * The arguments of this N_icm are usefull to get the name of
     * the resulting array, the dimension of this array, and the
     * length of the index_vector.
     * The name of the N_icm is used to detect whether N_return node is part
     * of a 'genarray' or 'modarray' with-loop.
     */

    DBUG_ENTER ("CompWithReturn");

    exprs = arg_node->node[0];
    is_array = IsArray (exprs->node[0]->IDS_NODE->TYPES);
    switch (arg_info->node[2]->info.id[9]) {
    case 'F':
        con_type = FOLD;
        break;
    case 'M':
        con_type = MOD_A;
        break;
    case 'G':
        con_type = GEN;
        break;
    default:
        DBUG_ASSERT (0, "unknown Con-expr ");
    }

    /* N_return will be converted to N_icm node */
    arg_node->nodetype = N_icm;
    ret_val = arg_node->node[0]->node[0]; /* store return_value node */
    with_icm_arg = arg_info->node[2]->node[0];
    /* 'dec_rc' points to a list of variables whose refcount has to be
     * decremented
     */
    dec_rc = arg_info->node[2]->node[3]->node[0];

    /* store name of resulting array */
    res = with_icm_arg->node[0];
    with_icm_arg = with_icm_arg->node[1];
    /* store dimension of resulting array */
    res_dim = with_icm_arg->node[0];
    tmp_with_icm_arg = with_icm_arg->node[1];

    if (MOD_A == con_type) {
        mod_array = tmp_with_icm_arg->node[0];
        tmp_with_icm_arg = tmp_with_icm_arg->node[1];
    }

    /* now we store some information:
     * from        : N_id ( the left border of a mod/genarray)
     * to          : N_id ( the right  border of a mod/genarray)
     * index_node  : N_id ( name of index-vector of mod/genarray)
     * index_length: N_num ( length of index-vector )
     *
     * all informations are shared with belonging mod/genarray N_icm
     */
    from = tmp_with_icm_arg->node[0];
    DBUG_ASSERT (N_id == from->nodetype, " wrong nodetype for 'from'");
    tmp_with_icm_arg = tmp_with_icm_arg->node[1];
    to = tmp_with_icm_arg->node[0];
    DBUG_ASSERT (N_id == to->nodetype, " wrong nodetype for 'to'");
    tmp_with_icm_arg = tmp_with_icm_arg->node[1];
    index_node = tmp_with_icm_arg->node[0];
    tmp_with_icm_arg = tmp_with_icm_arg->node[1];
    DBUG_ASSERT (N_id == index_node->nodetype, " wrong nodetype for 'index_node'");
    index_length = tmp_with_icm_arg->node[0];
    DBUG_ASSERT (N_num == index_length->nodetype, " wrong nodetype for 'index_length'");

    if (MOD_A == con_type) {
        MAKE_ICM_ARG (arg_node->node[0], res);
        icm_arg = arg_node->node[0];
        MAKE_NEXT_ICM_ARG (icm_arg, res_dim);
        /* N_return belongs to a 'modarray' with-loop */
        /* add name of modified array */
        MAKE_NEXT_ICM_ARG (icm_arg, mod_array);
        /* add name of return value */
        MAKE_NEXT_ICM_ARG (icm_arg, ret_val);
        if (0 == is_array) {
            MAKE_ICM_NAME (arg_node, "ND_END_MODARRAY_S");
        } else {
            MAKE_ICM_NAME (arg_node, "ND_END_MODARRAY_A");

            /* add length of index_vector */
            MAKE_NEXT_ICM_ARG (icm_arg, index_length);
        }
    } else if (GEN == con_type) {
        MAKE_ICM_ARG (arg_node->node[0], res);
        icm_arg = arg_node->node[0];
        MAKE_NEXT_ICM_ARG (icm_arg, res_dim);
        /* N_return belongs to a 'genarray' with-loop */
        /* add name of return value */
        MAKE_NEXT_ICM_ARG (icm_arg, ret_val);
        if (0 == is_array) {
            MAKE_ICM_NAME (arg_node, "ND_END_GENARRAY_S");
        } else {
            MAKE_ICM_NAME (arg_node, "ND_END_GENARRAY_A");
            MAKE_NEXT_ICM_ARG (icm_arg, index_length);
        }
    } else if (FOLD == con_type) {
        node *let, *exprs1, *exprs2, *last_assign, *tmp_res;

        arg_node->nodetype = N_let;
        let = arg_node;
        /* in arg_info->node[2]->node[2] the kind of the fold-function
         * is stored (as N_prf or as N_ap)
         */
        let->node[0] = arg_info->node[2]->node[2];
        LET_IDS (let) = MakeIds (res->IDS_ID, NULL, ST_regular);
        IDS_REFCNT (LET_IDS (let)) = ID_REFCNT (res) == -1 ? -1 : 1;
        let->IDS_NODE = res->IDS_NODE;
/*            IDS_REFCNT(LET_IDS(let))=1; */
#ifndef NEWTREE
        let->nnode = 1;
#endif
        exprs1 = MakeNode (N_exprs);
        MAKENODE_ID (tmp_res, StringCopy (res->IDS_ID));
        ID_REFCNT (tmp_res) = ID_REFCNT (res);
        tmp_res->IDS_NODE = res->IDS_NODE;
        exprs1->node[0] = tmp_res;
        exprs2 = MakeNode (N_exprs);
        exprs2->node[0] = ret_val;
#ifndef NEWTREE
        exprs2->nnode = 1;
#endif
        exprs1->node[1] = exprs2;
#ifndef NEWTREE
        exprs1->nnode = 2;
#endif
        let->node[0]->node[0] = exprs1;
#ifndef NEWTREE
        let->node[0]->nnode = 1;
#endif
        arg_info->IDS = let->IDS;
        arg_info->node[1] = let;
        let->node[0] = Trav (let->node[0], arg_info);

        last_assign = LAST_ASSIGN (arg_info);
#ifndef NEWTREE
        while (2 == last_assign->nnode)
#else
        while (last_assign->node[1] != NULL)
#endif
            last_assign = last_assign->node[1];

        MAKE_ICM (next_assign);
        MAKE_ICM_NAME (next_assign->node[0], "ND_END_FOLD");
        MAKE_ICM_ARG (next_assign->node[0]->node[0], index_length);

        /* now insert next_assign */
        last_assign->node[1] = next_assign;
#ifndef NEWTREE
        last_assign->nnode = 2;
#endif
        first_assign = next_assign;
    }

    /* now create N_icm to decrement refcount of index_vector ,
     * left(from) and right(to) border of mod/genarray
     */
    MAKENODE_NUM (n_node, 1);
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY", index_node, n_node);
    if (FOLD != con_type) {
        first_assign = next_assign;
        /* now insert next_assign */
        arg_info->node[0]->node[1]->node[1] = next_assign;
#ifndef NEWTREE
        arg_info->node[0]->node[1]->nnode = 2;
#endif
    } else {
        APPEND_ASSIGNS (first_assign, next_assign);
    }
    DEC_OR_FREE_RC_ND (from, n_node);
    DEC_OR_FREE_RC_ND (to, n_node);
    if (NULL != mod_array) {
        DEC_RC_FREE_ND (mod_array, n_node);
    }
    while (NULL != dec_rc) {
        if ((0 != strcmp (index_node->IDS_ID, dec_rc->IDS_ID))
            && ((NULL == mod_array)
                  ? 1
                  : (0 != strcmp (mod_array->IDS_ID, dec_rc->IDS_ID)))) {
            DEC_RC_FREE_ND (dec_rc, n_node);
        }
        dec_rc = dec_rc->node[0];
    }
    if (FOLD == con_type) {
        /* return contains to N_foldfun or N_foldprf, so the refcount of the
         * result is stored in the last_argument of the BEGIN N_icm.
         * It will be used for the END N_icm and removed form BEGIN N_icm
         */
        node *last_but_least_icm_arg;
        GOTO_LAST_BUT_LEAST_N_EXPRS (tmp_with_icm_arg, last_but_least_icm_arg);
        if (0 < last_but_least_icm_arg->node[1]->node[0]->info.cint) {
            INC_RC_ND (res, last_but_least_icm_arg->node[1]->node[0]);
        } else {
            FREE (last_but_least_icm_arg->node[1]->node[0]);
        }
#ifndef NEWTREE
        last_but_least_icm_arg->nnode = 1;
#endif
        FREE (last_but_least_icm_arg->node[1]);
    }
#undef MOD_A
#undef GEN
#undef FOLD

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompReturn
 *  arguments     : 1) N_return node
 *                  2) info node
 *  description   : generates N_icms for N_return of a function or of a
 *                  with-loop
 *  global vars   :
 *  internal funs : RenameReturn
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : if N_return node belongs to a with_loop, then
 *                   arg_info->node[2] will point to the first argument
 *                                     (N_exprs)
 *                   of corresponding N_icm for start of with_loop
 *                  arg_info->node[0] contains pointer to node before last
 *                    assign_node
 *
 */

node *
CompReturn (node *arg_node, node *arg_info)
{
    node *tmp, *next, *exprs, *last;
    int cnt_param;

    DBUG_ENTER ("CompReturn");

    if (NULL == arg_info->node[2]) {
        /* this is a N_return of a function-body */

        node *ret;

        ret = RenameReturn (arg_node, arg_info);
        exprs = arg_node->node[0];
        last = arg_node;
        /*
         * The new N_exprs chain will be stored in arg_node->node[1]
         * temporarily due to INSERT_ID_NODE and initial setting of 'last'.
         */

        /*
         * First, the real return values are traversed.
         */

        DBUG_PRINT ("COMP", ("Starting evaluation of return parameters"));

        DBUG_ASSERT (arg_node->node[1] == NULL, ("node[1] already used"));

        cnt_param = 0;

        while (NULL != exprs) {
            DBUG_ASSERT ((N_id == exprs->node[0]->nodetype), " wrong node (!= N_id)");

            DBUG_PRINT ("COMP", ("Current return id: %s", ID_NAME (EXPRS_EXPR (exprs))));

            next = exprs->node[1];

            if (MUST_REFCOUNT (ID, exprs->node[0])) {
                INSERT_ID_NODE (exprs, last, "out_rc");
            } else {
                INSERT_ID_NODE (exprs, last, "out");
            }

            /*
             *  In CompReturn, we don't have to distinguish between functions
             *  that do the refcounting on their own and those that do not
             *  because we're definitely inside a SAC function and these
             *  always do their own refcounting.
             */

            last = exprs;
            exprs = next;
            cnt_param++;
        }

        DBUG_PRINT ("COMP", ("Handled original return values finished"));

        exprs = RETURN_REFERENCE (arg_node);

        /*
         * Second, the counterparts of reference parameters are traversed
         * and added to the chain.
         */

        while (NULL != exprs) {
            DBUG_ASSERT ((N_id == exprs->node[0]->nodetype), " wrong node (!= N_id)");

            DBUG_PRINT ("COMP", ("Current return id: %s", ID_NAME (EXPRS_EXPR (exprs))));

#ifndef NEWTREE
            if (NODE_TYPE (last) == N_exprs)
                last->nnode = 2;
#endif /* NEWTREE */

            next = exprs->node[1];

            if (MUST_REFCOUNT (ID, exprs->node[0])) {
                INSERT_ID_NODE (exprs, last, "inout_rc");
            } else {
                INSERT_ID_NODE (exprs, last, "inout");
            }

            last = exprs;
            exprs = next;
        }

        DBUG_PRINT ("COMP", ("Handling counterparts of reference parameters finished"));

        arg_node->nodetype = N_icm;

        if (arg_node->node[1] == NULL) {
            MAKE_ICM_NAME (arg_node, "NOOP");
        } else {
            MAKE_ICM_NAME (arg_node, "ND_FUN_RET");

            exprs = MakeNode (N_exprs);
            MAKENODE_NUM (exprs->node[0], 0);
            exprs->node[1] = arg_node->node[1];
            /* put number of ret-values in front */
#ifndef NEWTREE
            exprs->nnode = 2;
#endif
            arg_node->node[0] = exprs;
            arg_node->node[1] = NULL; /* was only used temporarily */

            exprs = MakeNode (N_exprs);
            MAKENODE_ID (exprs->node[0], "");
            exprs->node[1] = arg_node->node[0];
#ifndef NEWTREE
            exprs->nnode = 2;
#endif
            arg_node->node[0] = exprs;

            /*
             *  Function ReorganizeParameters is called later by CompFunf.
             */
        }

        arg_node = ret; /* set new return_value of current function
                         * (N_let node if at least one variable in the "return"
                         * statement has been renamed, or  N_return otherwise)
                         */
    } else
        arg_node = CompWithReturn (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompWith
 *  arguments     : 1) N_with node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to last but one
 *                    assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *                  arg_info->node[2] will be set to  pointer to N_icm
 *                  of with_loop begin
 *
 */
node *
CompWith (node *arg_node, node *arg_info)
{
    node *old_info2, *first_assign, *next_assign, *n_node, *inc_rc, *icm_arg, *from, *to,
      *type_id_node, *arg, *res, *res_ref, *res_dim_node, *index, *indexlen,
      *old_arg_node, *last_assign, *fun_node, *res_size_node;
    int res_dim, is_foldprf, res_size;
    simpletype s_type;

    DBUG_ENTER ("CompWith");

    /* store arg_info->node[2] */
    old_info2 = arg_info->node[2];

    /* store res as N_id */
    MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);
    ID_REFCNT (res) = IDS_REFCNT (arg_info->IDS);

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_LASTIDS (arg_info)));

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type, arg_info->IDS_NODE->TYPES);
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* compute dimension of res */
    GET_DIM (res_dim, arg_info->IDS_NODE->TYPES);
    MAKENODE_NUM (res_dim_node, res_dim);

    /* compute size of res */
    GET_LENGTH (res_size, arg_info->IDS_NODE->TYPES);
    MAKENODE_NUM (res_size_node, res_size);

    /* store index_vector as N_id */
    MAKENODE_ID_REUSE_IDS (index, arg_node->node[0]->IDS);

    /* store length of index-vector */
    MAKENODE_NUM (indexlen, arg_node->node[0]->IDS_NODE->SHP[0]);

    /* set 'from' to left range of index-vector */
    from = arg_node->node[0]->node[0];

    /* set 'to' to left range of index-vector */
    to = arg_node->node[0]->node[1];
    if (res_dim > 0) {
        /* first create N_icm to allocate memeory */
        BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
        SET_VARS_FOR_MORE_ICMS;
        if ((N_modarray == old_arg_node->node[1]->nodetype)
            || (N_genarray == old_arg_node->node[1]->nodetype)) {
            MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
        } else {
            /* in this case (N_foldfun , N_foldprf) the refcount will be set
             * behind the with_loop
             */
            MAKENODE_NUM (n_node, 0);
            MAKE_NEXT_ICM_ARG (icm_arg, n_node);
        }
        MAKENODE_NUM (n_node, 1);
        MAKENODE_ID (type_id_node, "int");
        CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, index, n_node);
        APPEND_ASSIGNS (first_assign, next_assign);
    } else {
        MAKENODE_NUM (n_node, 1);
        MAKENODE_ID (type_id_node, "int");
        BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, index);
        MAKE_NEXT_ICM_ARG (icm_arg, n_node);
        SET_VARS_FOR_MORE_ICMS;
    }

    if (N_modarray == old_arg_node->node[1]->nodetype) {
        arg = old_arg_node->node[1]->node[0];
        DBUG_ASSERT (N_id == arg->nodetype, "wrong nodetype != N_id");
        CREATE_7_ARY_ICM (next_assign, "ND_BEGIN_MODARRAY", res, res_dim_node, arg, from,
                          to, index, indexlen);
        /* store pointer to N_icm ND_BEGIN.. in arg_info->node[2] */
        arg_info->node[2] = next_assign->node[0];
        /* store pointer to variables that have to be increased in
         * in arg_info->node[2]->node[3] ( it will be used in CompReturn )
         */
        arg_info->node[2]->node[3] = old_arg_node->node[2];

        APPEND_ASSIGNS (first_assign, next_assign);
    } else if (N_genarray == old_arg_node->node[1]->nodetype) {
        CREATE_6_ARY_ICM (next_assign, "ND_BEGIN_GENARRAY", res, res_dim_node, from, to,
                          index, indexlen);
        /* store pointer to N_icm ND_BEGIN.. in arg_info->node[2] */
        arg_info->node[2] = next_assign->node[0];
        /* store pointer to variables that have to be increased in
         * in arg_info->node[2]->node[3] ( it will be used in CompReturn )
         */
        arg_info->node[2]->node[3] = old_arg_node->node[2];
        APPEND_ASSIGNS (first_assign, next_assign);
    } else if (N_foldprf == old_arg_node->node[1]->nodetype) {
        fun_node = MakeNode (N_prf);
        fun_node->info.prf = old_arg_node->node[1]->info.prf;
        is_foldprf = 1;
    } else if (N_foldfun == old_arg_node->node[1]->nodetype) {
        fun_node = MakeNode (N_ap);
        fun_node->FUN_NAME = old_arg_node->node[1]->FUN_NAME;
        fun_node->FUN_MOD_NAME = old_arg_node->node[1]->FUN_MOD_NAME;
        fun_node->node[1] = old_arg_node->node[1]->node[2];
        DBUG_ASSERT (N_fundef == fun_node->node[1]->nodetype,
                     "wrong nodetype != N_fundef ");
        is_foldprf = 0;
    } else {
        DBUG_ASSERT (0, "wrong nodetype != N_foldfun, ... ");
    }
    if ((N_foldfun == old_arg_node->node[1]->nodetype)
        || (N_foldprf == old_arg_node->node[1]->nodetype)) {
        node *neutral_node, *n_neutral;
        int length;

        /* old_arg_node->node[1]->node[1] is the neutral element of
         * fold-function
         */
        neutral_node = old_arg_node->node[1]->node[1];
        DBUG_ASSERT (NULL != neutral_node, " neutral element is missing");
#ifndef NEWTREE
        DBUG_ASSERT (2 == old_arg_node->node[1]->nnode, " nnode != 2 ");
#endif
        if (N_array == neutral_node->nodetype) {
            COUNT_ELEMS (length, neutral_node->node[0]);
            MAKENODE_NUM (n_neutral, length);
            CREATE_7_ARY_ICM (next_assign,
                              (is_foldprf) ? "ND_BEGIN_FOLDPRF" : "ND_BEGIN_FOLDFUN", res,
                              res_size_node, from, to, index, indexlen, n_neutral);
            icm_arg->node[1] = neutral_node->node[0];
#ifndef NEWTREE
            icm_arg->nnode = 2;
#endif
            GOTO_LAST_N_EXPRS (neutral_node->node[0], icm_arg);
        } else {
            length = 1;
            MAKENODE_NUM (n_neutral, length);
            CREATE_7_ARY_ICM (next_assign,
                              (is_foldprf) ? "ND_BEGIN_FOLDPRF" : "ND_BEGIN_FOLDFUN", res,
                              res_size_node, from, to, index, indexlen, n_neutral);
            MAKE_NEXT_ICM_ARG (icm_arg, neutral_node);
        }
        /* append res_ref to N_icm temporarily. It will be used and
         * removed in CompReturn.
         */
        res_ref->info.cint -= 1;
        MAKE_NEXT_ICM_ARG (icm_arg, res_ref);

        /* store pointer to N_icm ND_BEGIN.. in arg_info->node[2] */
        arg_info->node[2] = next_assign->node[0];

        /* store pointer to variables that have to be increased in
         * in arg_info->node[2]->node[3] ( it will be used in CompReturn )
         */
        arg_info->node[2]->node[3] = old_arg_node->node[2];

        /* Store  N_prf or N_ap  in node[2] of current N_icm.
         * It will be used in CompReturn and than eliminated.
         */
        arg_info->node[2]->node[2] = fun_node;
        APPEND_ASSIGNS (first_assign, next_assign);
    }

    /* now add some INC_RC's */
    inc_rc = old_arg_node->node[2]->node[0];
    while (NULL != inc_rc) {
        MAKENODE_NUM (n_node, inc_rc->refcnt);
        INC_RC_ND (inc_rc, n_node);
        inc_rc = inc_rc->node[0];
    }
    /* update arg_info, after inserting new N_assign nodes */
    if ((N_foldprf == old_arg_node->node[1]->nodetype)
        || (N_foldfun == old_arg_node->node[1]->nodetype))
        next_assign->node[1] = old_arg_node->node[1]->node[0]->node[0];
    else
        next_assign->node[1] = old_arg_node->node[1]->node[1]->node[0];
#ifndef NEWTREE
    next_assign->nnode = 2;
#endif
    arg_info->node[0] = next_assign;
    DBUG_PRINT ("COMP", ("set info->node[0] to :" P_FORMAT " (node[0]:" P_FORMAT,
                         arg_info->node[0], arg_info->node[0]->node[0]));
    if ((N_foldprf == old_arg_node->node[1]->nodetype)
        || (N_foldfun == old_arg_node->node[1]->nodetype))
        next_assign = Trav (old_arg_node->node[1]->node[0]->node[0], arg_info);
    else
        next_assign = Trav (old_arg_node->node[1]->node[1]->node[0], arg_info);
    arg_info->node[2] = old_info2;
    APPEND_ASSIGNS (first_assign, next_assign);

    /* set first_assign to the last N_assign node, that is generated by Trav() */
    first_assign = LAST_ASSIGN (arg_info);
#ifndef NEWTREE
    while (2 == first_assign->nnode)
#else
    while (first_assign->node[1] != NULL)
#endif
        first_assign = first_assign->node[1];

    INSERT_ASSIGN;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompArg
 *  arguments     : 1) N_arg node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->node[0] contains pointer to N_block
 *                  arg_info->node[1] contains pointer to N_fundef
 *                  arg_info->lineno  contains parameter number
 *
 */
node *
CompArg (node *arg_node, node *arg_info)
{
    node *icm_arg, *id_node, *new_assign, *icm_tab_entry, *type_id_node;
    types *fulltype;
    char *tag;

    DBUG_ENTER ("CompArg");

    GET_BASIC_TYPE (fulltype, ARG_TYPE (arg_node), 042);

    id_node = MAKE_IDNODE (NULL != ARG_NAME (arg_node) ? ARG_NAME (arg_node) : "");
    /* store name of formal parameter */

    type_id_node = MAKE_IDNODE (MakeTypeString (fulltype));

    if ((MUST_REFCOUNT (ARG, arg_node))
        && (FUN_DOES_REFCOUNT (INFO_FUNDEF (arg_info), INFO_CNTPARAM (arg_info)))) {
        tag = ARG_ATTRIB (arg_node) == ST_inout ? "inout_rc" : "in_rc";

        /* put ND_INC_RC at beginning of function block */

        if (1 < ARG_REFCNT (arg_node)) {
            CREATE_2_ARY_ICM (new_assign, "ND_INC_RC", id_node,
                              MakeNum (ARG_REFCNT (arg_node) - 1));

            new_assign->node[1] = arg_info->node[0];
#ifndef NEWTREE
            new_assign->nnode += 1;
#endif
            arg_info->node[0] = new_assign;
        } else if (0 == ARG_REFCNT (arg_node)) {
            CREATE_2_ARY_ICM (new_assign, "ND_DEC_RC_FREE_ARRAY", id_node, MakeNum (1));

            new_assign->node[1] = arg_info->node[0];
#ifndef NEWTREE
            new_assign->nnode += 1;
#endif
            arg_info->node[0] = new_assign;
        }
    } else {
        if ((FUNDEF_PRAGMA (INFO_FUNDEF (arg_info)) != NULL)
            && (FUNDEF_REFCOUNTING (INFO_FUNDEF (arg_info)) != NULL)
            && (FUNDEF_REFCOUNTING (INFO_FUNDEF (arg_info))[INFO_CNTPARAM (arg_info)]
                == 1)) {
            WARN (NODE_LINE (arg_node), ("Pragma 'refcounting` illegal"));
            CONT_WARN (("Function wants to do refcounting on non-refcounted "
                        "parameter no. %d",
                        INFO_CNTPARAM (arg_info)));
        }

        if (ARG_ATTRIB (arg_node) == ST_inout) {
            if (FUNDEF_STATUS (INFO_FUNDEF (arg_info)) == ST_Cfun) {
                if (IsBoxed (ARG_TYPE (arg_node))) {
                    tag = "upd_bx";
                } else {
                    tag = "upd";
                }
            } else {
                tag = "inout";
            }
        } else {
            tag = "in";
        }
    }

    MAKE_ICM_ARG (icm_arg, MAKE_IDNODE (tag));
    icm_tab_entry = icm_arg;

    MAKE_NEXT_ICM_ARG (icm_arg, type_id_node);

    MAKE_NEXT_ICM_ARG (icm_arg, id_node);

    if (ARG_BASETYPE (arg_node) == T_dots) {
        InsertDefDotsParam (INFO_ICMTAB (arg_info), icm_tab_entry);
    } else {
        InsertDefArgParam (INFO_ICMTAB (arg_info), icm_tab_entry, INFO_TYPETAB (arg_info),
                           ARG_TYPE (arg_node),
                           NULL == FUNDEF_PRAGMA (INFO_FUNDEF (arg_info))
                             ? NULL
                             : FUNDEF_LINKSIGN (INFO_FUNDEF (arg_info)),
                           INFO_CNTPARAM (arg_info), NODE_LINE (arg_node));
    }

    INFO_CNTPARAM (arg_info) += 1;

    if (NULL != ARG_NEXT (arg_node)) {
        Trav (ARG_NEXT (arg_node), arg_info);
    }

    /*
     * Additional icms for the function body are generated regardless of the
     * existence of such a block, but CompFundef only inserts them if a
     * block already exists.
     */

    if (TYPES_DIM (fulltype)
        > 0) { /* put N_icm "ND_KS_DECL_ARRAY_ARG" at beginning of function block */

        node *dim_node, *shape;
        int i;

        /* store dim and shape */
        MAKENODE_NUM (dim_node, TYPES_DIM (fulltype));
        CREATE_2_ARY_ICM (new_assign, "ND_KS_DECL_ARRAY_ARG", id_node, dim_node);

        for (i = 0; i < TYPES_DIM (fulltype); i++) {
            MAKENODE_NUM (shape, TYPES_SHAPE (fulltype, i));
            MAKE_NEXT_ICM_ARG (icm_arg, shape);
        }

        /* now put node at beginning of block of function */
        new_assign->node[1] = arg_info->node[0];
#ifndef NEWTREE
        new_assign->nnode += 1;
#endif
        arg_info->node[0] = new_assign;
    }

    if (ARG_ATTRIB (arg_node) == ST_inout) {
        /*
         * put N_icm "ND_DECL_INOUT_PARAM" or "ND_DECL_INOUT_PARAM_RC"
         * respectively at beginning of function block
         */

        if (MUST_REFCOUNT (ARG, arg_node)) {
            CREATE_2_ARY_ICM (new_assign, "ND_DECL_INOUT_PARAM_RC", type_id_node,
                              id_node);
        } else {
            CREATE_2_ARY_ICM (new_assign, "ND_DECL_INOUT_PARAM", type_id_node, id_node);
        }

        /*
         * now put node at beginning of block of function
         */

        new_assign->node[1] = arg_info->node[0];
#ifndef NEWTREE
        new_assign->nnode += 1;
#endif
        arg_info->node[0] = new_assign;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompLoop
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :
 *                  arg_info->node[0] contains pointer to node before last
 *                    assign_node (use macro LAST_ASSIGN or NEXT_ASSIGN to
 *                     get these nodes)
 *
 *
 */
node *
CompLoop (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *icm_arg, *n_node, *v1, *v2, *label,
      *dummy_assign = NULL, *V1, *V2, *loop_assign;
    int found;

    DBUG_ENTER ("CompLoop");

    /* first compile termination condition and body of loop */
    loop_assign = LAST_ASSIGN (arg_info);
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    arg_node->node[1] = Trav (arg_node->node[1], arg_info);

    /* now add some  DEC_RC at begining of and after the loop */
    dummy_assign = MakeNode (N_assign);
    first_assign = dummy_assign;
    V1 = arg_node->node[2]->node[0];
    V2 = arg_node->node[2]->node[1];
    v1 = V1;
    v2 = V2;
    if ((NULL != v2) && (NULL == v1)) {
        MAKENODE_NUM (n_node, 1);
        while (NULL != v2) {
            DEC_RC_FREE_ND (v2, n_node); /* we don`t know the refcount of v2
                                          * in the current context, so we use
                                          * DEC_RC_FREE_ND
                                          */
            v2 = v2->node[0];
        }
    } else if ((NULL != v2) && (NULL != v1)) {
        node *v1_tmp;
        MAKENODE_NUM (n_node, 1);
        while (NULL != v2) {
            /* looking if v2 is in V2/V1 */
            v1_tmp = v1;
            found = 0;
            while ((0 == found) && (NULL != v1_tmp))
                if (0 == strcmp (v1_tmp->IDS_ID, v2->IDS_ID))
                    found = 1;
                else
                    v1_tmp = v1_tmp->node[0];

            if (0 == found) {
                DEC_RC_FREE_ND (v2, n_node);
            }

            v2 = v2->node[0];
        }
    }

    if (N_do == arg_node->nodetype) {
        label_nr++;
        MAKENODE_ID (label, GenName (label_nr, LABEL_NAME));
        CREATE_1_ARY_ICM (next_assign, "ND_LABEL", label);
        APPEND_ASSIGNS (first_assign, next_assign);
    }
    v1 = V1;
    if (NULL != v1) {
        /* now add some  INC_RC at begining of and after the loop */
        while (NULL != v1) {
            if (1 <= (v1->refcnt - 1)) {
                MAKENODE_NUM (n_node, v1->refcnt - 1);
                INC_RC_ND (v1, n_node);
            }
            v1 = v1->node[0];
        }
    }

    /* now insert INC's and DEC's at beginning of the loop */
    if (NULL != dummy_assign->node[1]) {
        first_assign->node[1] = arg_node->node[1]->node[0];
#ifndef NEWTREE
        first_assign->nnode = 2;
#endif
        arg_node->node[1]->node[0] = dummy_assign->node[1];
        dummy_assign->node[1] = NULL;
    }

    /* now create DEC_RC`s that have to be done after termination
     * of the loop
     */
    v1 = V1;
    v2 = V2;
    first_assign = dummy_assign; /* will be used in some macros */
    if (NULL != v1) {
        MAKENODE_NUM (n_node, 1);
        if (NULL != v2) {
            node *v2_tmp;
            while (NULL != v1) {
                /* looking if v1 is in V1/V2 */
                v2_tmp = v2;
                found = 0;
                while ((0 == found) && (NULL != v2_tmp))
                    if (0 == strcmp (v1->IDS_ID, v2_tmp->IDS_ID))
                        found = 1;
                    else
                        v2_tmp = v2_tmp->node[0];

                if (0 == found) {
                    DEC_RC_FREE_ND (v1, n_node);
                }
                v1 = v1->node[0];
            }
        } else
            while (NULL != v1) {
                DEC_OR_FREE_RC_ND (v1, n_node);
                v1 = v1->node[0];
            }
    }
    /* now increase RC of arrays that are defined in the  loop and are
     * used after it.
     */
    v2 = V2;
    if (NULL != v2)
        while (NULL != v2) {
            if (1 < v2->refcnt) {
                MAKENODE_NUM (n_node, v2->refcnt - 1);
                INC_RC_ND (v2, n_node);
            }
            v2 = v2->node[0];
        }

    if (NULL != dummy_assign->node[1]) {
        /* now put dummy_assign->node[1] behind while_loop */
        if (NULL != loop_assign->node[1]) {
            first_assign->node[1] = loop_assign->node[1];
#ifndef NEWTREE
            first_assign->nnode = 2;
#endif
        }
        loop_assign->node[1] = dummy_assign->node[1];
    }
    FREE (dummy_assign);

    if (N_do == arg_node->nodetype) {
        /* put N_icm 'ND_GOTO', in front of N_do node */
        CREATE_1_ARY_ICM (first_assign, "ND_GOTO", label);
        if (NULL != loop_assign->node[1]) {
            first_assign->node[1] = loop_assign->node[1]; /* next assign after do-loop */
#ifndef NEWTREE
            first_assign->nnode = 2;
#endif
        }
        first_assign->node[2] = loop_assign->node[0]; /* only temporary used (N_do) */
        loop_assign->node[0] = first_assign->node[0]; /* N_icm (ND_GOTO) node */
        loop_assign->node[1] = first_assign;
#ifndef NEWTREE
        loop_assign->nnode = 2;
#endif
        arg_node = first_assign->node[0];
        first_assign->node[0] = first_assign->node[2]; /* put N_do node */
        first_assign->node[2] = NULL;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompCond
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :
 *
 *
 */
node *
CompCond (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *icm_arg, *n_node, *id_node, *dummy_assign = NULL;
    int i;

    DBUG_ENTER ("CompCond");

    /* compile condition, then and else part */
#ifndef NEWTREE
    for (i = 0; i < arg_node->nnode; i++)
#else
    for (i = 0; i < nnode[arg_node->nodetype]; i++)
        if (arg_node->node[i] != NULL)
#endif
        arg_node->node[i] = Trav (arg_node->node[i], arg_info);

    /* insert N_icms to correct refcounts of then and else part */
    dummy_assign = MakeNode (N_assign);
#ifndef NEWTREE
    for (i = 0; i < arg_node->nnode; i++) /* ??? i<arg_node->node[3]->nnode ??? */
#else
    for (i = 0; i < MAX_SONS; i++)
#endif
        if (NULL != arg_node->node[3]->node[i]) {
            id_node = arg_node->node[3]->node[i];
            first_assign = dummy_assign;
            do {
                MAKENODE_NUM (n_node, id_node->refcnt);
                DBUG_PRINT ("COMP", ("%d:create DEC_RC(%s, %d)", i, id_node->IDS_ID,
                                     id_node->refcnt));
                DEC_OR_FREE_RC_ND (id_node, n_node);
                id_node = id_node->node[0];
            } while (NULL != id_node);
            first_assign->node[1] = arg_node->node[i + 1]->node[0];
#ifndef NEWTREE
            first_assign->nnode = 2;
#endif
            arg_node->node[i + 1]->node[0] = dummy_assign->node[1];
        }
    FREE (dummy_assign);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompTypedef
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   : transforms N_typedef to N_icm if it is a definition of an
 *                  array
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :
 *
 */
node *
CompTypedef (node *arg_node, node *arg_info)
{
#ifdef NEWTREE
    node *tmp_node;
#endif

    DBUG_ENTER ("CompTypedef");

    if (0 != arg_node->DIM) {
        char *typename, *new_typename;
        node *type1, *type2, *icm_arg;

        typename = type_string[arg_node->SIMPLETYPE];

        new_typename = StringCopy (arg_node->ID);
        FREE_TYPE (arg_node->TYPES);
        arg_node->nodetype = N_icm;
        MAKE_ICM_NAME (arg_node, "ND_TYPEDEF_ARRAY");
#ifndef NEWTREE
        if (1 == arg_node->nnode) {
            arg_node->node[1] = arg_node->node[0];
        }
#else
        if (arg_node->node[1] == NULL)
            tmp_node = arg_node->node[0];
        else
            tmp_node = NULL;
#endif
        MAKENODE_ID (type1, typename);
        MAKE_ICM_ARG (arg_node->node[0], type1);
        icm_arg = arg_node->node[0];
        MAKENODE_ID (type2, new_typename);
        MAKE_NEXT_ICM_ARG (icm_arg, type2);

#ifndef NEWTREE
        if (1 == arg_node->nnode) {
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);
            arg_node->nnode = 2;
        }
#else
        if (tmp_node != NULL)
            arg_node->node[1] = Trav (tmp_node, arg_info);
#endif
    } else {
        if (arg_node->node[0] != NULL) {
            arg_node->node[0] = Trav (arg_node->node[0], arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompObjdef
 *  arguments     : 1) N_objdef node
 *                  2) arg_info unused
 *  description   : The N_objdef node is replaced if the object's type
 *                  is an array.
 *  global vars   :
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,, NULL
 *  remarks       :
 *
 */

node *
CompObjdef (node *arg_node, node *arg_info)
{
    node *icm, *icm_arg, *type_id_node, *id_node;
    int i;
    types *full_type;

    DBUG_ENTER ("CompObjdef");

    if (IsArray (OBJDEF_TYPE (arg_node))) {
        icm = MakeNode (N_icm);

        GET_BASIC_TYPE (full_type, OBJDEF_TYPE (arg_node), 042);

        MAKENODE_ID (type_id_node, type_string[TYPES_BASETYPE (full_type)]);
        MAKE_ICM_ARG (icm_arg, type_id_node);
        ICM_ARGS (icm) = icm_arg;

        MAKENODE_ID (id_node, OBJDEF_NAME (arg_node));
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);

        MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (TYPES_DIM (full_type)));

        if (OBJDEF_STATUS (arg_node) == ST_imported) {
            MAKE_ICM_NAME (icm, "ND_KD_DECL_EXTERN_ARRAY");
        } else {
            MAKE_ICM_NAME (icm, "ND_KS_DECL_GLOBAL_ARRAY");

            for (i = 0; i < TYPES_DIM (full_type); i++) {
                MAKE_NEXT_ICM_ARG (icm_arg, MakeNum (TYPES_SHAPE (full_type, i)));
            }
        }

        OBJDEF_ICM (arg_node) = icm;
    } else {
        OBJDEF_ICM (arg_node) = NULL;
    }

    OBJDEF_EXPR (arg_node) = NULL;
    /*
     *  The initialization expression is not freed because it may be used
     *  in the main function.
     */

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
