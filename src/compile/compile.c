/*
 *
 * $Log$
 * Revision 1.146  1998/05/08 00:44:52  dkr
 * with-loop:
 *   index-vector is now set up correctly, if needed.
 *   RCO of with-loop arguments added.
 *
 * Revision 1.145  1998/05/07 16:18:06  dkr
 * added INC_RCs in COMPNwith2
 *
 * Revision 1.144  1998/05/07 10:15:44  dkr
 * changed compiling of new with-loop
 *
 * Revision 1.143  1998/05/04 16:21:51  dkr
 * fixed a bug: args for WL_BEGIN are now in correct order :)
 *
 * Revision 1.142  1998/05/04 15:36:37  dkr
 * changed usage of WL_ASSIGN
 *
 * Revision 1.141  1998/05/03 14:02:40  dkr
 * changed COMPWL...
 *
 * Revision 1.140  1998/04/30 14:04:04  dkr
 * changed COMPSpmd
 *
 * Revision 1.139  1998/04/29 19:55:12  dkr
 * changed COMPSpmd, COMPSync
 *
 * Revision 1.138  1998/04/26 21:53:53  dkr
 * fixed a bug in COMPSpmd
 *
 * Revision 1.137  1998/04/25 14:57:39  dkr
 * traversal of vardec is now started by COMPBlock instead of COMPFundef
 *
 * Revision 1.136  1998/04/25 13:20:13  dkr
 * extended COMPSPMD
 *
 * Revision 1.134  1998/04/24 01:15:33  dkr
 * added CompSync
 *
 * Revision 1.133  1998/04/21 13:31:57  dkr
 * added funs CompWL...
 *
 * Revision 1.132  1998/04/21 12:28:28  dkr
 * fixed a bug in CompSPMD
 *
 * Revision 1.131  1998/04/20 14:07:25  sbs
 * revised CompTypedef !!
 *
 * Revision 1.130  1998/04/20 12:46:50  dkr
 * fixed a bug in CompSPMD
 *
 * Revision 1.129  1998/04/20 02:39:47  dkr
 * fixed a bug with shared icm-args
 * (but there are many more bugs of this sort 8-((
 *
 * Revision 1.128  1998/04/17 17:26:53  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.126  1998/04/16 15:58:40  dkr
 * compilation of N_modul nodes moved from 'Compile' to 'CompModul'
 * modified 'CompConc'
 *
 * Revision 1.125  1998/04/14 22:48:10  dkr
 * some renamed access macros
 *
 * Revision 1.124  1998/04/14 21:44:24  dkr
 * fixed a bug with COND_VARINFO, DO_VARINFO, ...
 * changed CompConc
 *
 * Revision 1.122  1998/04/07 14:50:03  dkr
 * added CompNcode
 *
 * Revision 1.120  1998/04/02 18:47:21  dkr
 * added CompConc
 *
 * Revision 1.119  1998/03/24 13:45:48  cg
 * Primitive function abs() is now compiled to ICM ND_ABS
 *
 * Revision 1.118  1998/03/17 12:22:40  cg
 * Now, an alternative way of initializing character arrays derived from
 * strings is implemented. This uses the new ICM ND_CREATE_CONST_ARRAY_C
 * which in turn calls the libsac function String2Array.
 *
 * Revision 1.116  1998/03/02 22:25:30  dkr
 * code for new with-loop moved to precompile
 *
 * Revision 1.113  1998/02/16 01:11:52  dkr
 * bugs fixed
 *
 * Revision 1.110  1998/02/15 04:08:38  dkr
 * partitioning of index-projections added (experimental !!)
 *
 * Revision 1.109  1998/02/11 16:39:46  dkr
 * now the type cmp_types is used (CMP_equal, CmpTypes())
 *
 * Revision 1.108  1998/02/11 16:30:36  dkr
 * removed NEWTREE, access-macros used
 *
 * Revision 1.107  1998/02/10 14:57:54  dkr
 * macro WITH_USEDVARS is used
 *
 * Revision 1.106  1997/11/27 10:41:06  dkr
 * removed old access-macros
 *
 * Revision 1.105  1997/11/25 10:33:42  dkr
 * rudimental CompNWith added
 *
 * Revision 1.104  1997/11/22 23:07:52  dkr
 * removed a bug in CompPrf() with F_{add,sub,...}_AxA, arg{1,2}->refcnt==-1
 *   - previous N_let replaced by ND_ALLOC_ARRAY (reuse)
 *   - old_arg_node now correctly set (-> FREE(old_arg_node) !!)
 *
 * Revision 1.103  1997/11/11 00:43:13  dkr
 * removed a bug with NEWTREE in CompTypedef
 *
 * Revision 1.102  1997/11/10 16:09:47  dkr
 * removed a bug with DBUG_ASSERT in GOTO_LAST_BUT_LEAST_N_EXPRS
 * (with defined NEWTREE)
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
 * - changed macro CHECK_REUSE__ALLOC_ARRAY_ND (reuse only if old and new array
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
 * bug fixed in CompAssign (arg_info->node[0] will be set correctly now)
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

#include "tree.h"

#include "print.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"
#include "Error.h"
#include "traverse.h"
#include "DataFlowMask.h"
#include "compile.h"
#include "convert.h"
#include "DupTree.h"
#include "refcount.h"
#include "typecheck.h" /* to use LookupType() */
#include "free.h"

#define TYP_IF(n, d, p, f, sz) sz

int basetype_size[] = {
#include "type_info.mac"
};

#undef TYP_IF

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

#define MAKE_IDNODE(id) MakeId (StringCopy (id), NULL, ST_regular)

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
        for (i = 0; i < num; i++) {                                                      \
            var = EXPRS_NEXT (var);                                                      \
        }                                                                                \
    }

#define EQUAL_SIMPLETYPES(stype1, stype2) (stype1 == stype2)

#define SET_VARS_FOR_MORE_ICMS                                                           \
    first_assign = CURR_ASSIGN (arg_info);                                               \
    old_arg_node = arg_node;                                                             \
    last_assign = NEXT_ASSIGN (arg_info);                                                \
    arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info))

#define INSERT_ASSIGN                                                                    \
    if (NULL != last_assign) {                                                           \
        APPEND_ASSIGNS (first_assign, last_assign);                                      \
    }

#define CHECK_REUSE(test)                                                                \
    BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_CHECK_REUSE_ARRAY", test, res);     \
    SET_VARS_FOR_MORE_ICMS

#define CHECK_REUSE__ALLOC_ARRAY_ND(new, stype_new, old, stype_old)                      \
    if ((1 == old->refcnt) && (EQUAL_SIMPLETYPES (stype_new, stype_old))) {              \
        /* create ND_CHECK_REUSE_ARRAY  */                                               \
        node *num;                                                                       \
        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_CHECK_REUSE_ARRAY", old, new);  \
        first_assign = CURR_ASSIGN (arg_info);                                           \
        DBUG_PRINT ("COMP", ("first:" P_FORMAT, first_assign));                          \
        old_arg_node = arg_node;                                                         \
        arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));                              \
        /* create ND_ALLOC_ARRAY */                                                      \
        MAKENODE_NUM (num, 0);                                                           \
        CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, new, num);        \
        APPEND_ASSIGNS (first_assign, next_assign);                                      \
    } else {                                                                             \
        /* create ND_ALLOC_ARRAY */                                                      \
        node *num;                                                                       \
        DBUG_ASSERT (((old->refcnt == -1) || (opt_rco == 0)),                            \
                     "Refcnt of prf-arg neither -1 nor 1 !");                            \
        MAKENODE_NUM (num, 0);                                                           \
        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", type_id_node,     \
                       new);                                                             \
        MAKE_NEXT_ICM_ARG (icm_arg, num);                                                \
        first_assign = CURR_ASSIGN (arg_info);                                           \
        DBUG_PRINT ("COMP", ("first:" P_FORMAT, first_assign));                          \
        old_arg_node = arg_node;                                                         \
        arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));                              \
    }

#define DEC_OR_FREE_RC_ND(array, num_node)                                               \
    if (1 < array->refcnt) { /* create ND_DEC_RC */                                      \
        DEC_RC_ND (array, num_node);                                                     \
    } else {                                                                             \
        if (array->refcnt > 0) {                                                         \
            DEC_RC_FREE_ND (array, num_node);                                            \
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
    COUNT_ELEMS (n_elems, ARRAY_AELEMS (array))                                          \
    MAKENODE_NUM (n_node, n_elems);                                                      \
    MAKENODE_NUM (n_node1, 1);                                                           \
    MAKENODE_ID (tmp_array1, "__TMP");                                                   \
    NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block; /* reuse previous N_let*/        \
    CREATE_4_ARY_ICM (first_assign, "ND_KS_DECL_ARRAY", type_id_node, tmp_array1,        \
                      n_node1, n_node);                                                  \
    arg_node = first_assign;                                                             \
    CREATE_CONST_ARRAY (array, tmp_array1, type_id_node, rc);                            \
    array = tmp_array1 /* set array to __TMP */

#define CREATE_CONST_ARRAY(array, name, type, rc)                                        \
    CREATE_3_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type, name, rc);                    \
    APPEND_ASSIGNS (first_assign, next_assign);                                          \
    COUNT_ELEMS (n_elems, ARRAY_AELEMS (array));                                         \
    MAKENODE_NUM (n_node, n_elems);                                                      \
    CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", name, n_node);             \
    EXPRS_NEXT (icm_arg) = ARRAY_AELEMS (array);                                         \
    APPEND_ASSIGNS (first_assign, next_assign)

#define DECL_ARRAY(assign, node, var_str, var_str_node)                                  \
    COUNT_ELEMS (n_elems, node);                                                         \
    MAKENODE_NUM (n_elems_node, n_elems);                                                \
    MAKENODE_NUM (n_node1, 1);                                                           \
    MAKENODE_ID (var_str_node, var_str);                                                 \
    CREATE_4_ARY_ICM (assign, "ND_KS_DECL_ARRAY", type_id_node, var_str_node, n_node1,   \
                      n_elems_node)

#define INSERT_ID_NODE(no, last, str)                                                    \
    tmp = MakeNode (N_exprs);                                                            \
    MAKENODE_ID (EXPRS_EXPR (tmp), str);                                                 \
    EXPRS_NEXT (tmp) = no;                                                               \
    EXPRS_NEXT (last) = tmp;

/*
 * following macros are used to compute last but one or next N_assign from
 *  a 'arg_info' node
 */

#define CURR_ASSIGN(arg_info)                                                            \
    ((N_block == NODE_TYPE (INFO_COMP_LASTASSIGN (arg_info)))                            \
       ? BLOCK_INSTR (INFO_COMP_LASTASSIGN (arg_info))                                   \
       : ASSIGN_NEXT (INFO_COMP_LASTASSIGN (arg_info)))

#define NEXT_ASSIGN(arg_info) ASSIGN_NEXT (CURR_ASSIGN (arg_info))

#define INSERT_BEFORE(arg_info, node)                                                    \
    if (N_block == NODE_TYPE (INFO_COMP_LASTASSIGN (arg_info))) {                        \
        BLOCK_INSTR (INFO_COMP_LASTASSIGN (arg_info)) = node;                            \
    } else {                                                                             \
        ASSIGN_NEXT (INFO_COMP_LASTASSIGN (arg_info)) = node;                            \
    }

#define GOTO_LAST_N_EXPRS(exprs, last_exprs)                                             \
    last_exprs = exprs;                                                                  \
    while (EXPRS_NEXT (last_exprs) != NULL) {                                            \
        DBUG_ASSERT (N_exprs == NODE_TYPE (last_exprs), "nodetype != N_expr");           \
        last_exprs = EXPRS_NEXT (last_exprs);                                            \
    }

#define GOTO_LAST_BUT_LEAST_N_EXPRS(exprs, lbl_exprs)                                    \
    {                                                                                    \
        node *tmp;                                                                       \
        lbl_exprs = exprs;                                                               \
        DBUG_ASSERT (N_exprs == NODE_TYPE (lbl_exprs), "nodetype != N_expr");            \
        DBUG_ASSERT (EXPRS_NEXT (lbl_exprs) != NULL,                                     \
                     "there is NO N_exprs-chain contains only one element");             \
        tmp = EXPRS_NEXT (lbl_exprs);                                                    \
        while (EXPRS_NEXT (tmp) != NULL) {                                               \
            DBUG_ASSERT (N_exprs == NODE_TYPE (tmp), "nodetype != N_expr");              \
            lbl_exprs = tmp;                                                             \
            tmp = EXPRS_NEXT (tmp);                                                      \
        }                                                                                \
    }

#define FREE_TYPE(a)                                                                     \
    if (NULL != a->shpseg) {                                                             \
        FREE (a->shpseg)                                                                 \
    }                                                                                    \
    FREE (a)

#define FREE_VARDEC(a)                                                                   \
    FREE_TYPE (VARDEC_TYPE (a))                                                          \
    FREE (a)

#define FREE_IDS(a)                                                                      \
    {                                                                                    \
        DBUG_PRINT ("FREE", (P_FORMAT, a));                                              \
        FreeAllIds (a);                                                                  \
    }

static int label_nr = 0;

/******************************************************************************
 *
 * function:
 *   node *AppendAssign( node *assigns, node *assign)
 *
 * description:
 *   appends 'assign' to the N_assign-chain 'assings' and returns the new
 *    chain.
 *
 ******************************************************************************/

node *
AppendAssign (node *assigns, node *assign)
{
    node *tmp;

    DBUG_ENTER ("AppendAssign");

    if (assigns != NULL) {
        tmp = assigns;
        while (ASSIGN_NEXT (tmp) != NULL) {
            tmp = ASSIGN_NEXT (tmp);
        }
        ASSIGN_NEXT (tmp) = assign;
    } else {
        assigns = assign;
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *AppendExpr( node *exprs, node *expr)
 *
 * description:
 *   appends 'expr' to the N_exprs-chain 'assings' and returns the new chain.
 *
 ******************************************************************************/

node *
AppendExpr (node *exprs, node *expr)
{
    node *tmp;

    DBUG_ENTER ("AppendExpr");

    if (exprs != NULL) {
        tmp = exprs;
        while (EXPRS_NEXT (tmp) != NULL) {
            tmp = EXPRS_NEXT (tmp);
        }
        EXPRS_NEXT (tmp) = expr;
    } else {
        exprs = expr;
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   ids *GetIndexIds(ids *index_ids, int dim)
 *
 * description:
 *   returns the index-ids for dimension 'dim' found in 'index_ids'.
 *   'index_ids' is a vector of index-ids (e.g. NWITHID_IDS(...)) containing
 *    at least 'dim' elements.
 *
 ******************************************************************************/

ids *
GetIndexIds (ids *index_ids, int dim)
{
    int i;

    DBUG_ENTER ("GetIndexIds");

    for (i = 0; i < dim; i++) {
        DBUG_ASSERT ((index_ids != NULL), "not enough ids found");
        index_ids = IDS_NEXT (index_ids);
    }

    DBUG_RETURN (index_ids);
}

/******************************************************************************
 *
 * function:
 *   node *MakeAllocArrayICMs( ids *mm_ids)
 *
 * description:
 *   builds a ND_ALLOC_ARRAY icm for each RC-ids in 'mm_ids'.
 *
 ******************************************************************************/

node *
MakeAllocArrayICMs (ids *mm_ids)
{
    node *assigns = NULL;
    node *assign, *last_assign;
    simpletype s_type;

    DBUG_ENTER ("MakeAllocArrayICMs");

    while (mm_ids != NULL) {
        if (IDS_REFCNT (mm_ids) >= 0) {
            GET_BASIC_SIMPLETYPE (s_type, IDS_TYPE (mm_ids));
            assign
              = MakeAssign (MakeIcm ("ND_ALLOC_ARRAY",
                                     MakeExprs (MakeId (StringCopy (type_string[s_type]),
                                                        NULL, ST_regular),
                                                MakeExprs (MakeId2 (mm_ids),
                                                           MakeExprs (MakeNum (
                                                                        IDS_REFCNT (
                                                                          mm_ids)),
                                                                      NULL))),
                                     NULL),
                            NULL);

            if (assigns == NULL) {
                assigns = assign;
            } else {
                ASSIGN_NEXT (last_assign) = assign;
            }
            last_assign = assign;

            mm_ids = IDS_NEXT (mm_ids);
        } else {
            mm_ids = FreeOneIds (mm_ids);
        }
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *MakeIncRcICMs( ids *mm_ids)
 *
 * description:
 *   builds a ND_INC_RC icm for each ids in 'mm_ids', which rc is >0.
 *
 ******************************************************************************/

node *
MakeIncRcICMs (ids *mm_ids)
{
    node *assigns = NULL;
    node *assign, *last_assign;
    simpletype s_type;

    DBUG_ENTER ("MakeIncRcICMs");

    while (mm_ids != NULL) {
        if (IDS_REFCNT (mm_ids) > 0) {
            GET_BASIC_SIMPLETYPE (s_type, IDS_TYPE (mm_ids));
            assign
              = MakeAssign (MakeIcm ("ND_INC_RC",
                                     MakeExprs (MakeId2 (mm_ids),
                                                MakeExprs (MakeNum (IDS_REFCNT (mm_ids)),
                                                           NULL)),
                                     NULL),
                            NULL);

            if (assigns == NULL) {
                assigns = assign;
            } else {
                ASSIGN_NEXT (last_assign) = assign;
            }
            last_assign = assign;

            mm_ids = IDS_NEXT (mm_ids);
        } else {
            mm_ids = FreeOneIds (mm_ids);
        }
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *MakeDecRcFreeICMs( ids *mm_ids)
 *
 * description:
 *   builds a ND_DEC_RC_FREE icm for each ids in 'mm_ids', which rc is >=0.
 *
 ******************************************************************************/

node *
MakeDecRcFreeICMs (ids *mm_ids)
{
    node *assigns = NULL;
    node *assign, *last_assign;
    simpletype s_type;

    DBUG_ENTER ("MakeDecRcFreeICMs");

    while (mm_ids != NULL) {
        if (IDS_REFCNT (mm_ids) >= 0) {
            GET_BASIC_SIMPLETYPE (s_type, IDS_TYPE (mm_ids));
            assign = MakeAssign (MakeIcm ("ND_DEC_RC_FREE_ARRAY",
                                          MakeExprs (MakeId2 (mm_ids),
                                                     MakeExprs (MakeNum (1), NULL)),
                                          NULL),
                                 NULL);

            if (assigns == NULL) {
                assigns = assign;
            } else {
                ASSIGN_NEXT (last_assign) = assign;
            }
            last_assign = assign;

            mm_ids = IDS_NEXT (mm_ids);
        } else {
            mm_ids = FreeOneIds (mm_ids);
        }
    }

    DBUG_RETURN (assigns);
}

/*
 *
 *  functionname  : AddVardec
 *  description   :
 *  remarks       :
 *
 */

node *
AddVardec (node *vardec, types *type, char *name, node *fundef)
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

    /*
     * we must update FUNDEF_DFM_BASE!!
     */
    FUNDEF_DFM_BASE (fundef)
      = DFMUpdateMaskBase (FUNDEF_DFM_BASE (fundef), FUNDEF_ARGS (fundef),
                           FUNDEF_VARDEC (fundef));

    DBUG_RETURN (vardec);
}

/*
 *
 *  functionname  : AdjustAddedAssigns
 *  description   :
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
                                               new_id)))) {
                            ASSIGN_NEXT (last) = ASSIGN_NEXT (tmp);
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
 *  description   :
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
 *  remarks       :
 *
 */

char *
GenericFun (int which, types *type)
{
    node *tdef;
    char *ret = NULL;

    DBUG_ENTER ("GenericFun");

    DBUG_PRINT ("COMP", ("Looking for generic fun %d (0==copy/1==free)"));

    if (TYPES_BASETYPE (type) == T_user) {
        tdef = LookupType (TYPES_NAME (type), TYPES_MOD (type), 042);
        /* 042 is only a dummy argument */
        DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");

        if ((TYPEDEF_BASETYPE (tdef) == T_hidden)
            || (TYPEDEF_BASETYPE (tdef) == T_user)) {
            if (TYPEDEF_TNAME (tdef) != NULL) {
                tdef = LookupType (TYPEDEF_TNAME (tdef), TYPEDEF_TMOD (tdef), 042);
                DBUG_ASSERT ((tdef != NULL), "Failed attempt to look up typedef");
            }

            switch (which) {
            case 0:
                ret = TYPEDEF_COPYFUN (tdef);
                break;
            case 1:
                ret = TYPEDEF_FREEFUN (tdef);
                break;
            default:
                DBUG_ASSERT ((0), "Unknown kind if generic function requested");
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
        }
    }

    DBUG_PRINT ("COMP", ("Merging icm args of \"ND_FUN_AP\", new tag=\"%s\"",
                         ID_NAME (EXPRS_EXPR (out_icm))));

    DBUG_RETURN (new_assign);
}

/*
 *
 *  functionname  : MergeIcmsFundef
 *  description   :
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

    if (CMP_equal == CmpTypes (out_type, in_type)) {
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
 *  description   :
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
                cnt++;
            }
        }
        NUM_VAL (EXPRS_EXPR (EXPRS_NEXT (icm_arg))) = cnt;
    }

    DBUG_RETURN (icm_arg);
}

/*
 *
 *  functionname  : CreateApIcm
 *  description   :
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
    ICM_NAME (icm) = "ND_FUN_AP";

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
            cnt_icm++;
        }
    }

    tmp = icm_tab[0];

    while (tmp != NULL) {
        cnt_icm++;
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

/******************************************************************************
 *
 * function:
 *   node *CreateFundefIcm(char *name, statustype status,
 *                         node **icm_tab, int tab_size)
 *
 * description:
 *   creates a fundef ICM.
 *     status == ST_spmdfun: MT_SPMD_FUN_DEC
 *     otherwise:            ND_FUN_DEC
 *
 ******************************************************************************/

node *
CreateFundefIcm (char *name, statustype status, node **icm_tab, int tab_size)
{
    node *icm, *icm_arg;
    int i;
    int cnt_icm = 0;

    DBUG_ENTER ("CreateFundefIcm");

    DBUG_PRINT ("COMP", ("Creating ICM \"ND_FUN_DEC\""));

    if (status == ST_spmdfun) {
        icm = MakeIcm ("MT_SPMD_FUN_DEC", NULL, NULL);
    } else {
        icm = MakeIcm ("ND_FUN_DEC", NULL, NULL);
    }

    MAKE_ICM_ARG (icm_arg, MAKE_IDNODE (StringCopy (name)));
    ICM_ARGS (icm) = icm_arg;

    if (icm_tab[1] == NULL) {
        MAKE_NEXT_ICM_ARG (icm_arg, MAKE_IDNODE ("void"));
    } else {
        APPEND_ICM_ARG (icm_arg, EXPRS_NEXT (icm_tab[1]));
    }

    for (i = 0; i < tab_size; i++) {
        if ((i != 1) && (icm_tab[i] != NULL)) {
            cnt_icm++;
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
 *  description   :
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
 *  description   :
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
                ASSIGN_NEXT (collect_assigns) = new_assign;
            }
        }
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : InsertApReturnParam
 *  description   :
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
 *  description   :
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
 *  description   :
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
 *  description   :
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
 *  description   : 
 *  remarks       : 
 *
 */

node **GenStandardIcmTab(node *vars_arg_start, int n_vars, int varblock_length)
{
  node *tmp;
  int i;
  
  DBUG_ENTER("GenStandardIcmTab");

  icm_arg_tab=(node**)Malloc(sizeof(node*)*(n_vars+1));

  for (i = 0; i <= n_vars; i++) {
    icm_arg_tab[i]=NULL;
  }
  
  /*
   *  A table for variable icm args is created and initialized.
   */

  tmp = vars_arg_start;
  i = 1;
    
  while (tmp != NULL) {
    /*
     *  All variable icm args are inserted into the table.
     *  The first "out" parameter is inserted at position 0, all other
     *  parameters are inserted in their original sequence starting with
     *  position 1.
     */

    if (0 == strcmp(ID_NAME(EXPRS_EXPR(tmp)), "out_bx")) {
      ID_NAME(EXPRS_EXPR(tmp)) = "out";
    }
    else {
      if (0 == strcmp(ID_NAME(EXPRS_EXPR(tmp)), "in_bx")) {
        ID_NAME(EXPRS_EXPR(tmp)) = "in";
      }
    }
    
    if ((0 == strcmp(ID_NAME(EXPRS_EXPR(tmp)), "out")) &&
        (icm_arg_tab[0] == NULL)) {
      icm_arg_tab[0]=tmp;

      DBUG_PRINT("COMP", ("First OUT parameter moved to special position"));
    }
    else {
      icm_arg_tab[i] = tmp;
      i++;
    }
      
    SHIFT_EXPRS_PTR(tmp, varblock_length);
  }

  DBUG_RETURN(icm_arg_tab);
}



/*
 *
 *  functionname  : GenSpecialIcmTabFundef
 *  description   : 
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
  
  linksign = PRAGMA_LINKSIGN(FUNDEF_PRAGMA(fundef));
  
  /*
   *  A table for variable icm args is created and initialized.
   */

  icm_arg_tab=(node**)Malloc(sizeof(node*)*(n_vars+1));
  arg_type_tab=(types**)Malloc(sizeof(types*)*(n_vars+1));

  for (i=0; i<=n_vars; i++) {
    icm_arg_tab[i] = NULL;
    arg_type_tab[i] = NULL;
  }
  
  /*
   *  First, the local variables current_arg and current_type are
   *  initialized. These are traversed simultaniously to the icms
   *  in order to know each parameter's type.
   */

  if (FUNDEF_BASETYPE(fundef) == T_void) {
    current_arg=FUNDEF_ARGS(fundef);
    if (current_arg != NULL) {
      current_type=ARG_TYPE(current_arg);
    }
  }
  else {
    current_arg=NULL;
    current_type=FUNDEF_TYPES(fundef);
  }

  tmp = vars_arg_start;
  i = 0;
  
  while (tmp!=NULL) {
    /*
     *  All variable icm args are inserted into the table
     *  depending on pragma linksign.
     */

    if (icm_arg_tab[linksign[i]] == NULL) {
      if (0==strcmp(ID_NAME(EXPRS_EXPR(tmp)), "out_bx")) {
        ID_NAME(EXPRS_EXPR(tmp))="out";
      }
      else {
        if (0==strcmp(ID_NAME(EXPRS_EXPR(tmp)), "in_bx")) {
          ID_NAME(EXPRS_EXPR(tmp))="in";
        }    
      }
      
      icm_arg_tab[linksign[i]]=tmp;
      arg_type_tab[linksign[i]]=current_type;

      DBUG_PRINT("COMP",
                 ("Parameter #%d moved to link position #%d",
                  i, linksign[i]));
    }
    else {
      MergeIcmsFundef(icm_arg_tab[linksign[i]], tmp, 
                      arg_type_tab[linksign[i]], current_type,
                      NODE_LINE(fundef), mapping_info[2*i]);
    }      
      
    i++;
    SHIFT_EXPRS_PTR(tmp, varblock_length);

    /*
     *  Finally, current_type and current_arg are shifted.
     */

    if (current_arg==NULL) {
      if (TYPES_NEXT(current_type)==NULL) {
        current_arg=FUNDEF_ARGS(fundef);
        if (current_arg!=NULL) {
          current_type=ARG_TYPE(current_arg);
        }
      }
      else {
        current_type=TYPES_NEXT(current_type);
      }
    }
    else {
      current_arg=ARG_NEXT(current_arg);
      if (current_arg != NULL) {
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
 *  description   : 
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

  for (i=0; i<=n_vars; i++) {
    icm_arg_tab[i]=NULL;
  }
  

  tmp=vars_arg_start;
  i=0;
  
  while (tmp!=NULL) {
    /*
     *  All variable icm args are inserted into the table
     *  depending on pragma linksign.
     */

    if (icm_arg_tab[linksign[i]]==NULL) {
      if (0==strcmp(ID_NAME(EXPRS_EXPR(tmp)), "out_bx")) {
        ID_NAME(EXPRS_EXPR(tmp))="out";
      }
      else {
        if (0==strcmp(ID_NAME(EXPRS_EXPR(tmp)), "in_bx")) {
          ID_NAME(EXPRS_EXPR(tmp))="in";
        }    
      }
      
      icm_arg_tab[linksign[i]]=tmp;

      DBUG_PRINT("COMP",
                 ("Parameter #%d moved to link position #%d",
                  i, linksign[i]));
    }
    else {
      new_assign=MergeIcmsAp(icm_arg_tab[linksign[i]], tmp, 
                             mapping_info[2*i], mapping_info[2*i+1]);

      if (new_assign!=NULL)
      {
        ASSIGN_NEXT(new_assign)=ASSIGN_NEXT(collect_assigns);
        ASSIGN_NEXT(collect_assigns)=new_assign;
      }
    }      
      
    i++;
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
  
  if (FUNDEF_PRAGMA(fundef) == NULL) {
    linksign=NULL;
  }
  else {
    linksign=PRAGMA_LINKSIGN(FUNDEF_PRAGMA(fundef));
  }
  
  header_length=(tag==2)?2:3;
    /* number of static icm args (before ICM_VAR) */
 
  varblock_length=(tag==1)?3:2;
    /* number of dynamic icm args which belong together */
  
  DBUG_PRINT("COMP",
             ("Reorganizing parameters (tag=%d, pragma=%d)", tag, linksign));
  
  if (tag == 2) {
    tmp = icm_args;           /* ND_FUN_RET */
  }
  else {
    tmp = EXPRS_NEXT(icm_args);
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
  
  while (tmp != NULL) {
    n_vars++;
    SHIFT_EXPRS_PTR(tmp, varblock_length);
  }

  DBUG_PRINT("COMP", ("%d icm args found", n_vars));
  
  /*
   *  The number of following variable icm args is counted 
   *  and stored in n_vars.
   */


  if (linksign==NULL) {
    icm_arg_tab=GenStandardIcmTab(vars_arg_start, n_vars, varblock_length);
  }
  else {
    if (tag==1) {
      icm_arg_tab=GenSpecialIcmTabFundef(vars_arg_start, n_vars, fundef,
                                         mapping_info);
    }
    else {
      collect_assigns=MakeAssign(NULL, NULL);
      icm_arg_tab=GenSpecialIcmTabNonFundef(vars_arg_start, n_vars,
                                            linksign, collect_assigns,
                                            mapping_info);
    }
  }
  
  
  /*
   *  The real return parameter is moved to its special position.
   */

  if (icm_arg_tab[0] != NULL) {
    ID_NAME(ret_arg)=ID_NAME(EXPRS_EXPR(EXPRS_NEXT(icm_arg_tab[0])));
    NUM_VAL(n_vars_arg) = n_vars - 1;
  }
  else {
    NUM_VAL(n_vars_arg)=n_vars;
  }
    
  /*
   *  All other parameters are put together into a chain again.
   */

  last=last_header_arg;
  
  for (i = 1; i <= NUM_VAL(n_vars_arg); i++) {
    if (icm_arg_tab[i]==NULL) {
      ERROR(NODE_LINE(fundef), ("Pragma 'linksign` illegal"));
      CONT_ERROR(("No SAC parameter mapped to C parameter %d", i));
    }
    else {
      EXPRS_NEXT(last) = icm_arg_tab[i];
      SHIFT_EXPRS_PTR(last, varblock_length);
    }
  }
 
  EXPRS_NEXT(last)=NULL;

  if (collect_assigns != NULL) {
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
 *                  - inserts new assignments (after CURR_ASSIGN(arg_info))
 *  remarks       : - pointer to variable declaration is stored in
 *                    INFO_COMP_VARDECS(arg_info)
 *                  - returns N_let of last inserted new assign if
 *                     there renameing had to be done
 *                    returns N_return if no renaming had to be done
 *                  - puts new assignments after CURR_ASSIGN(arg_info).
 *                    node[0] of CURR_ASSIGN(arg_info) will be set in
 *                    last COMPAssign (return value of COMPReturn)) again
 *
 */

node *
RenameReturn (node *return_node, node *arg_info)
{
    node *exprs, *tmp_exprs, *assign, *let, *next_assign, *vardec;
    int i;
    char *old_id, *new_id;

    DBUG_ENTER ("RenameReturn");

    exprs = RETURN_EXPRS (return_node);
    next_assign = MakeAssign (return_node, NULL);
    vardec = INFO_COMP_VARDECS (arg_info);

    while (NULL != exprs) {
        tmp_exprs = EXPRS_NEXT (exprs);
        i = 1;
        old_id = ID_NAME (EXPRS_EXPR (exprs));
        while (NULL != tmp_exprs) {
            if (0 == strcmp (ID_NAME (EXPRS_EXPR (tmp_exprs)), old_id)) {
                /* generates new nodes */
                new_id = RenameVar (old_id, i);
                let = MakeLet (MakeId (ID_NAME (EXPRS_EXPR (tmp_exprs)), NULL, 0),
                               MakeIds (new_id, NULL, 0));
                assign = MakeAssign (let, next_assign);
                next_assign = assign;
                vardec = AddVardec (vardec, ID_TYPE (EXPRS_EXPR (tmp_exprs)), new_id,
                                    INFO_COMP_FUNDEF (arg_info));

                /* rename variable in return-statement */
                ID_NAME (EXPRS_EXPR (tmp_exprs)) = StringCopy (new_id);
            }
            tmp_exprs = EXPRS_NEXT (tmp_exprs);
            i++;
        }
        exprs = EXPRS_NEXT (exprs);
    }
    if (ASSIGN_INSTR (next_assign) != return_node) {
        /* new nodes have been inserted */
        node *last_assign = CURR_ASSIGN (arg_info);
        ASSIGN_INSTR (last_assign) = ASSIGN_INSTR (assign);
        ASSIGN_NEXT (last_assign) = ASSIGN_NEXT (assign);
        INFO_COMP_VARDECS (arg_info) = vardec;

        return_node = ASSIGN_INSTR (assign);
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

/*
 *
 *  functionname  : ShapeToArray
 *  arguments     : 1) N_vardec node
 *  description   : computes the shape of corresponding type and stores it
 *                  as N_exprs - chain
 *  remarks       : ----
 *
 */

node *
ShapeToArray (node *vardec_node)
{
    node *ret_node = NULL, *tmp, *basic_type_node;
    int i;

    DBUG_ENTER ("ShapeToArray");
    if (T_user != TYPES_BASETYPE (VARDEC_TYPE (vardec_node))) {
        ret_node = MakeNode (N_exprs);
        MAKENODE_NUM (EXPRS_EXPR (ret_node),
                      SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)), 0));
        tmp = ret_node;
        for (i = 1; i < TYPES_DIM (VARDEC_TYPE (vardec_node)); i++) {
            EXPRS_NEXT (tmp) = MakeNode (N_exprs);
            MAKENODE_NUM (EXPRS_EXPR (EXPRS_NEXT (tmp)),
                          SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)), i));
            tmp = EXPRS_NEXT (tmp);
        }
    } else {
        basic_type_node = LookupType (TYPES_NAME (VARDEC_TYPE (vardec_node)),
                                      TYPES_MOD (VARDEC_TYPE (vardec_node)),
                                      042); /* 042 is dummy argument */
        if (1 <= TYPES_DIM (VARDEC_TYPE (vardec_node))) {
            ret_node = MakeNode (N_exprs);
            MAKENODE_NUM (EXPRS_EXPR (ret_node),
                          SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)), 0));
            tmp = ret_node;
            for (i = 1; i < TYPES_DIM (VARDEC_TYPE (vardec_node)); i++) {
                EXPRS_NEXT (tmp) = MakeNode (N_exprs);
                MAKENODE_NUM (EXPRS_EXPR (EXPRS_NEXT (tmp)),
                              SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (vardec_node)), i));
                tmp = EXPRS_NEXT (tmp);
            }
            for (i = 0; i < TYPES_DIM (VARDEC_TYPE (basic_type_node)); i++) {
                EXPRS_NEXT (tmp) = MakeNode (N_exprs);
                MAKENODE_NUM (EXPRS_EXPR (EXPRS_NEXT (tmp)),
                              SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (basic_type_node)),
                                            i));
                tmp = EXPRS_NEXT (tmp);
            }
        } else {
            ret_node = MakeNode (N_exprs);
            MAKENODE_NUM (EXPRS_EXPR (ret_node),
                          SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (basic_type_node)), 0));
            tmp = ret_node;
            for (i = 1; i < TYPES_DIM (VARDEC_TYPE (basic_type_node)); i++) {
                EXPRS_NEXT (tmp) = MakeNode (N_exprs);
                MAKENODE_NUM (EXPRS_EXPR (EXPRS_NEXT (tmp)),
                              SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (basic_type_node)),
                                            i));
                tmp = EXPRS_NEXT (tmp);
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

    arg_node = Trav (arg_node, info);

    FREE (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPModul( node *arg_node, node *arg_info)
 *
 * description:
 *   compiles an N_modul node:
 *     - traverses sons.
 *
 ******************************************************************************/

node *
COMPModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("COMPModul");

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }
    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }
    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPFundef
 *  arguments     : 1) N_fundef node
 *                  2) info node
 *  description   : traverses child-nodes.
 *  remarks       :- sets INFO_COMP_FIRSTASSIGN(arg_info) to first N_assign
 *                    of function before traversing the function's
 *                    arguments
 *                 - sets INFO_COMP_VARDECS(arg_info) to variable declaration
 *                 - calls Trav to `compile` variable declarations.
 */

node *
COMPFundef (node *arg_node, node *arg_info)
{
    node *return_node, *icm_arg, *type_id_node, *var_name_node, *tag_node, **icm_tab,
      *icm_tab_entry;
    types *rettypes, *fulltype, **type_tab;
    int cnt_param, tab_size, i;

    DBUG_ENTER ("COMPFundef");

    INFO_COMP_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_COMP_VARDECS (arg_info) = FUNDEF_VARDEC (arg_node);
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    rettypes = FUNDEF_TYPES (arg_node);

    if ((NULL != FUNDEF_RETURN (arg_node)) && (TYPES_BASETYPE (rettypes) != T_void)) {
        /*
         * FUNDEF_RETURN(arg_node) points to a N_icm (ND_FUN_RET).
         * return_node will point to the first N_exprs belonging to a
         * return_value. This exists only for functions with at least
         * one return value.
         */

        return_node
          = EXPRS_NEXT (EXPRS_NEXT (EXPRS_NEXT (ICM_ARGS (FUNDEF_RETURN (arg_node)))));
    }

    cnt_param = 0;

    tab_size = CountFunctionParams (arg_node) + 2;
    icm_tab = (node **)Malloc (sizeof (node *) * tab_size);
    type_tab = (types **)Malloc (sizeof (types *) * tab_size);

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

        if (NULL == FUNDEF_BODY (arg_node)) {
            /* it is an extern declaration */
#ifdef IMPORTED_WITH_NAME
            MAKENODE_ID (var_name_node, GenName (i, DUMMY_NAME));
#else
            MAKENODE_ID (var_name_node, "");
#endif

        } else {
            DBUG_ASSERT ((return_node != NULL), "no return icm found");
            DBUG_ASSERT ((N_id == NODE_TYPE (EXPRS_EXPR (return_node))),
                         "wrong nodetype != N_id");
            MAKENODE_ID_REUSE_IDS (var_name_node, LET_IDS (EXPRS_EXPR (return_node)));
            if (EXPRS_NEXT (return_node) != NULL) {
                /*
                 * put return_node to next N_exprs where a function return_value
                 * is behind
                 */
                return_node = EXPRS_NEXT (EXPRS_NEXT (return_node));
            }
        }

        MAKE_NEXT_ICM_ARG (icm_arg, var_name_node);

        InsertDefReturnParam (icm_tab, icm_tab_entry, type_tab, rettypes,
                              (FUNDEF_PRAGMA (arg_node) == NULL)
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
        if (NULL != FUNDEF_BODY (arg_node)) {
            /* first assign of body */
            INFO_COMP_FIRSTASSIGN (arg_info) = BLOCK_INSTR (FUNDEF_BODY (arg_node));
        }

        INFO_COMP_CNTPARAM (arg_info) = (cnt_param == 0) ? 1 : cnt_param;
        INFO_COMP_ICMTAB (arg_info) = icm_tab;
        INFO_COMP_TYPETAB (arg_info) = type_tab;

        /*
         *  the arg_node is needed while compiling args as argument for
         *  FUN_DOES_REFCOUNT
         */

        /* traverse formal parameters (N_arg) */
        Trav (FUNDEF_ARGS (arg_node), arg_info);

        if (NULL != FUNDEF_BODY (arg_node)) {
            /* new first assign of body */
            BLOCK_INSTR (FUNDEF_BODY (arg_node)) = INFO_COMP_FIRSTASSIGN (arg_info);
            INFO_COMP_FIRSTASSIGN (arg_info) = NULL;
        }

        INFO_COMP_CNTPARAM (arg_info) = 0;
        INFO_COMP_ICMTAB (arg_info) = NULL;
        INFO_COMP_TYPETAB (arg_info) = NULL;
    }

    if ((FUNDEF_RETURN (arg_node) != NULL)
        && (ICM_ARGS (FUNDEF_RETURN (arg_node)) != NULL)) {
        ReorganizeReturnIcm (ICM_ARGS (FUNDEF_RETURN (arg_node)));
    }

    FUNDEF_ICM (arg_node) = CreateFundefIcm (FUNDEF_NAME (arg_node),
                                             FUNDEF_STATUS (arg_node), icm_tab, tab_size);

    /*
     * From now on FUNDEF_RETURN(fundef) points to N_icm instead of function's
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

/******************************************************************************
 *
 * function:
 *   node *COMPBlock( node *arg_node, node *arg_info)
 *
 * description:
 *   stacks INFO_COMP_LASTASSIGN and sets it to the current node while
 *    traversal.
 *
 ******************************************************************************/

node *
COMPBlock (node *arg_node, node *arg_info)
{
    node *old_info;

    DBUG_ENTER ("COMPBlock");

    /* stacking of old info! (nested blocks) */
    old_info = INFO_COMP_LASTASSIGN (arg_info);

    INFO_COMP_LASTASSIGN (arg_info) = arg_node;
    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    /* restoring old info! (nested blocks) */
    INFO_COMP_LASTASSIGN (arg_info) = old_info;

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPAssign( node *arg_node, node *arg_info)
 *
 * description:
 *   Compiles a N_assign node.
 *
 * remarks:
 *   - INFO_COMP_LASTASSIGN(arg_info) contains a pointer to the last assigment
 *     (predecessor of 'arg_node').
 *     During traversal of ASSIGN_INSTR(arg_node) via this pointer some ICMs
 *     are inserted before *and* after 'arg_node'!
 *     Because this is a very ugly behaviour, newer routines (new with-loop/
 *     SPMD) *return* an assign-chain. In this case 'COMPAssign' inserts this
 *     assignments into the syntax-tree!
 *     With this mechanismus it is not neccessary to manipulate remote parts
 *     of the tree via 'arg_info'.
 *
 ******************************************************************************/

node *
COMPAssign (node *arg_node, node *arg_info)
{
    node *old_next_assign;
    node *old_last_assign;
    node *instr;

    DBUG_ENTER ("COMPAssign");

    old_last_assign = INFO_COMP_LASTASSIGN (arg_info);
    old_next_assign = ASSIGN_NEXT (arg_node);

    instr = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (NODE_TYPE (instr) == N_assign) {
        /*
         * an assignment-chain was returned.
         *  -> insert it at the current position into the tree.
         */
        ASSIGN_INSTR (arg_node) = ASSIGN_INSTR (instr);
        ASSIGN_INSTR (instr) = NULL;
        instr = FreeNode (instr);
        ASSIGN_NEXT (arg_node) = AppendAssign (instr, ASSIGN_NEXT (arg_node));
    } else {
        ASSIGN_INSTR (arg_node) = instr;
    }

    if (old_next_assign != NULL) {
        INFO_COMP_LASTASSIGN (arg_info) = arg_node;
        /*
         * Now, we skip all those assigns that were inserted during the
         * traversal of 'ASSIGN_INSTR(arg_node)' !!
         */
        while (old_next_assign != ASSIGN_NEXT (INFO_COMP_LASTASSIGN (arg_info))) {
            INFO_COMP_LASTASSIGN (arg_info)
              = ASSIGN_NEXT (INFO_COMP_LASTASSIGN (arg_info));
        }

        old_next_assign = Trav (old_next_assign, arg_info);
    }

    /*
     * although the new assigns are already inserted correctly into
     * the chain of assignments, we have to return the correct pointer,
     * since the normal insertion mechanism probably is used !!!
     */
    arg_node = BLOCK_INSTR_OR_ASSIGN_NEXT (old_last_assign);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPLet(node *arg_node, node *arg_info)
 *
 * description:
 *   Compiles a N_let node.
 *
 * remarks:
 *   - In LET_EXPR() many routines use INFO_COMP_LASTLET to recycle the let
 *     node as an ICM, if necessary.
 *     Because this is a very ugly behaviour, newer routines (new with-loop/
 *     SPMD) *return* an assign-chain. In this case 'COMPLet' frees the old
 *     'arg_node' and returns the assign-chain to 'COMPAssign', where this
 *     assignments are inserted into the syntax-tree! (see 'COMPAssign').
 *     With this mechanismus it is not neccessary to manipulate remote parts
 *     of the tree via 'arg_info'.
 *
 ******************************************************************************/

node *
COMPLet (node *arg_node, node *arg_info)
{
    node *expr;

    DBUG_ENTER ("COMPLet");

    INFO_COMP_LASTLET (arg_info) = arg_node;
    INFO_COMP_LASTIDS (arg_info) = LET_IDS (arg_node);

    expr = Trav (LET_EXPR (arg_node), arg_info);

    if (NODE_TYPE (expr) == N_assign) {
        LET_EXPR (arg_node) = NULL;
        arg_node = FreeTree (arg_node);
        arg_node = expr;
    } else {
        LET_EXPR (arg_node) = expr;
    }

    INFO_COMP_LASTLET (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPVardec
 *  arguments     : 1) N_vardec node
 *                  2) NULL
 *  description   : transforms N_vardec to N_icm node if it is the declaration
 *                  of an array
 *  remarks       : if it is a declaration of an array, a N_assign
 *                  node will be inserted to get the new N_icm node into the
 *                  chain of N_vardecs
 *
 */

node *
COMPVardec (node *arg_node, node *arg_info)
{
    node *assign, *id_node, *n_node, *n_dim, *id_type, *icm_arg;
    int i;

    types *full_type;

    DBUG_ENTER ("COMPVardec");

    GET_BASIC_TYPE (full_type, VARDEC_TYPE (arg_node), 0);

    if (TYPES_DIM (full_type) > SCALAR) {
        /*
         * full_type is an array with known shape.
         */

        MAKENODE_ID (id_type, type_string[TYPES_BASETYPE (full_type)]);
        /* declared type */
        MAKENODE_ID (id_node, VARDEC_NAME (arg_node)); /* name of variable */
        MAKENODE_NUM (n_dim, TYPES_DIM (full_type));

        /* now create N_icm */
        assign
          = MakeAssign (MakeIcm ("ND_KS_DECL_ARRAY", NULL, NULL), VARDEC_NEXT (arg_node));

        MAKE_ICM_ARG (ICM_ARGS (ASSIGN_INSTR (assign)), id_type);
        icm_arg = ICM_ARGS (ASSIGN_INSTR (assign));
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);
        MAKE_NEXT_ICM_ARG (icm_arg, n_dim);

        for (i = 0; i < TYPES_DIM (full_type); i++) {
            /* the shape information will be converted & added */
            MAKENODE_NUM (n_node, full_type->shpseg->shp[i]);
            MAKE_NEXT_ICM_ARG (icm_arg, n_node);
        }

        /* now free some nodes */
        if (T_user == TYPES_BASETYPE (VARDEC_TYPE (arg_node))) {
            FREE_TYPE (full_type);
        }

        FREE_VARDEC (arg_node);
        arg_node = assign; /* set arg_node, because this node will be returned */

        if (NULL != VARDEC_TYPEDEF (arg_node)) {
            VARDEC_TYPEDEF (arg_node) = Trav (VARDEC_TYPEDEF (arg_node), NULL);
            /* dkr: Trav(...) with (arg_info == NULL) !?!? */
        }
    } else if (TYPES_DIM (full_type) == UNKNOWN_SHAPE) {
        /*
         *  full_type is an array with unknown shape and dimension.
         */

        MAKENODE_ID (id_type, type_string[TYPES_BASETYPE (full_type)]);
        /* declared type */
        MAKENODE_ID (id_node, VARDEC_NAME (arg_node)); /* name of variable */

        /* now create N_icm */
        assign
          = MakeAssign (MakeIcm ("ND_DECL_ARRAY", NULL, NULL), VARDEC_NEXT (arg_node));

        MAKE_ICM_ARG (ICM_ARGS (ASSIGN_INSTR (assign)), id_type);
        icm_arg = ICM_ARGS (ASSIGN_INSTR (assign));
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);

        /* now free some nodes */
        if (T_user == TYPES_BASETYPE (VARDEC_TYPE (arg_node))) {
            FREE_TYPE (full_type);
        }

        FREE_VARDEC (arg_node);
        arg_node = assign; /* set arg_node, because this node will be returned */

        if (NULL != VARDEC_NEXT (arg_node)) {
            VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), NULL);
            /* dkr: Trav(...) with (arg_info == NULL) !?!? */
        }
    } else if (TYPES_DIM (full_type) < KNOWN_DIM_OFFSET) {
        /*
         *  full_type is an array with unknown shape and known dimension.
         */

        MAKENODE_ID (id_type, type_string[TYPES_BASETYPE (full_type)]);
        /* declared type */
        MAKENODE_ID (id_node, VARDEC_NAME (arg_node)); /* name of variable */
        MAKENODE_NUM (n_dim, KNOWN_DIM_OFFSET - TYPES_DIM (full_type));

        /* now create N_icm */
        assign
          = MakeAssign (MakeIcm ("ND_KD_DECL_ARRAY", NULL, NULL), VARDEC_NEXT (arg_node));

        MAKE_ICM_ARG (ICM_ARGS (ASSIGN_INSTR (assign)), id_type);
        icm_arg = ICM_ARGS (ASSIGN_INSTR (assign));
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);
        MAKE_NEXT_ICM_ARG (icm_arg, n_dim);

        /* now free some nodes */
        if (T_user == TYPES_BASETYPE (VARDEC_TYPE (arg_node))) {
            FREE_TYPE (full_type);
        }

        FREE_VARDEC (arg_node);
        arg_node = assign; /* set arg_node, because this node will be returned */

        if (NULL != VARDEC_TYPEDEF (arg_node)) {
            VARDEC_TYPEDEF (arg_node) = Trav (VARDEC_TYPEDEF (arg_node), NULL);
            /* dkr: Trav(...) with (arg_info == NULL) !?!? */
        }
    } else {
        if (IsNonUniqueHidden (VARDEC_TYPE (arg_node))) {
            CREATE_2_ARY_ICM (assign, "ND_DECL_RC", MAKE_IDNODE ("void*"),
                              MAKE_IDNODE (StringCopy (VARDEC_NAME (arg_node))));

            if (VARDEC_NEXT (arg_node) != NULL) {
                ASSIGN_NEXT (assign) = Trav (VARDEC_NEXT (arg_node), NULL);
                /* dkr: Trav(...) with (arg_info == NULL) !?!? */
            }
            FREE_VARDEC (arg_node);
            arg_node = assign;
        } else {
            if (TYPES_DIM (VARDEC_TYPE (arg_node)) < 0) {
                /* current vardec-node has unknown shape and will be removed */
                node *tmp;
                tmp = arg_node;
                if (VARDEC_NEXT (arg_node) != NULL) {
                    arg_node = Trav (VARDEC_NEXT (arg_node), NULL);
                    /* dkr: Trav(...) with (arg_info == NULL) !?!? */
                } else {
                    arg_node = NULL;
                }
                FREE_VARDEC (tmp);
            } else {
                /* traverse next N_vardec node if any */
                if (VARDEC_NEXT (arg_node) != NULL) {
                    VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), NULL);
                    /* dkr: Trav(...) with (arg_info == NULL) !?!? */
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPPrfModarray
 *  arguments     : 1) N_prf node
 *                  2)
 *  description   : transforms N_prf node F_modarray to N_icm nodes
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before
 *                   last assign_node (to get last or next assign node use
 *                                     macros LAST_ASSIGN or NEXT_ASSIGN)
 *                  INFO_COMP_LASTASSIGN(arg_info) is used to insert new assign_nodes
 *                   in front of or after last assign node
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to last N_let
 */

node *
COMPPrfModarray (node *arg_node, node *arg_info)
{
    node *res, *length, *n_node, *res_ref, *type_id_node, *dim_res, *line, *first_assign,
      *next_assign, *icm_arg, *old_arg_node, *last_assign;
    node *arg1 = PRF_ARG1 (arg_node);
    node *arg2 = PRF_ARG2 (arg_node);
    node *arg3 = PRF_ARG3 (arg_node);
    int n, dim;
    simpletype s_type;

    DBUG_ENTER ("COMPPrfModarray");

    MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type,
                          VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* store dimension of result */
    GET_DIM (dim, VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    MAKENODE_NUM (dim_res, dim);

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

    /* store line of prf function */
    MAKENODE_NUM (line, arg_node->lineno);

    if (NODE_TYPE (arg2) == N_array) {     /* index is constant! */
        if (NODE_TYPE (arg3) == N_array) { /* value is constant! */
            DBUG_ASSERT (0, "sorry compilation of ND_PRF_MODARRAY_AxCxC not yet done");
        } else {
            COUNT_ELEMS (n, ARRAY_AELEMS (arg2));
            MAKENODE_NUM (length, n);

            if ((N_id == NODE_TYPE (arg3))
                && (1 == IsArray (VARDEC_TYPE (ID_VARDEC (arg3))))) {
                char *icm_name;

                if (1 == arg1->refcnt)
                    icm_name = "ND_PRF_MODARRAY_AxCxA_CHECK_REUSE";
                else
                    icm_name = "ND_PRF_MODARRAY_AxCxA";

                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line,
                               type_id_node);
                MAKE_NEXT_ICM_ARG (icm_arg, dim_res);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                MAKE_NEXT_ICM_ARG (icm_arg, arg3);
                MAKE_NEXT_ICM_ARG (icm_arg, length);
                icm_arg->node[1] = ARRAY_AELEMS (arg2);
                SET_VARS_FOR_MORE_ICMS;
                MAKENODE_NUM (n_node, 1);
                DEC_OR_FREE_RC_ND (arg3, n_node);
            } else {
                char *icm_name;

                if (1 == arg1->refcnt)
                    icm_name = "ND_PRF_MODARRAY_AxCxS_CHECK_REUSE";
                else
                    icm_name = "ND_PRF_MODARRAY_AxCxS";

                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line,
                               type_id_node);
                MAKE_NEXT_ICM_ARG (icm_arg, dim_res);
                MAKE_NEXT_ICM_ARG (icm_arg, res);
                MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                MAKE_NEXT_ICM_ARG (icm_arg, arg3);
                MAKE_NEXT_ICM_ARG (icm_arg, length);
                icm_arg->node[1] = ARRAY_AELEMS (arg2);
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

            if ((N_id == NODE_TYPE (arg3)) && (1 == IsArray (ID_TYPE (arg3)))) {
                char *icm_name;

                if (1 == arg1->refcnt)
                    icm_name = "ND_PRF_MODARRAY_AxVxA_CHECK_REUSE";
                else
                    icm_name = "ND_PRF_MODARRAY_AxVxA";

                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line,
                               type_id_node);
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

                if (1 == arg1->refcnt) {
                    icm_name = "ND_PRF_MODARRAY_AxVxS_CHECK_REUSE";
                } else {
                    icm_name = "ND_PRF_MODARRAY_AxVxS";
                }

                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line,
                               type_id_node);
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
 *  functionname  : COMPIdxModarray
 *  arguments     : 1) N_prf node
 *                  2)
 *  description   : transforms N_prf node F_idx_modarray to N_icm nodes
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before
 *                   last assign_node (to get last or next assign node use
 *                                     macros LAST_ASSIGN or NEXT_ASSIGN )
 *                  INFO_COMP_LASTASSIGN(arg_info) is used to insert new assign_nodes
 *                   in front of or after last assign node
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to last N_let
 */

node *
COMPIdxModarray (node *arg_node, node *arg_info)
{
    node *res, *n_node, *res_ref, *type_id_node, *line, *first_assign, *next_assign,
      *icm_arg, *old_arg_node, *last_assign;
    node *arg1 = PRF_ARG1 (arg_node);
    node *arg2 = PRF_ARG2 (arg_node);
    node *arg3 = PRF_ARG3 (arg_node);

    simpletype s_type;

    DBUG_ENTER ("COMPIdxModarray");

    MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type,
                          VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

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

        if ((N_id == NODE_TYPE (arg3)) && (1 == IsArray (ID_TYPE (arg3)))) {
            char *icm_name;

            if (1 == arg1->refcnt)
                icm_name = "ND_IDX_MODARRAY_AxVxA_CHECK_REUSE";
            else
                icm_name = "ND_IDX_MODARRAY_AxVxA";

            BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line, type_id_node);
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

            BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), icm_name, line, type_id_node);
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
 *  functionname  : COMPConvert
 *  arguments     : 1) N_prf nodei (F_toi, F_tod, F_tof, F_toi_A, F_tof_A,
 *                                  F_tod_A)
 *                  2) NULL
 *  description   :
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 */

node *
COMPConvert (node *arg_node, node *arg_info)
{
    int convert = 0;

    DBUG_ENTER ("COMPConvert");

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
        MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));
        /* compute basic type */
        GET_BASIC_SIMPLETYPE (s_type,
                              VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
        MAKENODE_ID (type_id_node, type_string[s_type]);
        MAKENODE_NUM (res_rc, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));
        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", type_id_node, res);
        MAKE_NEXT_ICM_ARG (icm_arg, res_rc);
        SET_VARS_FOR_MORE_ICMS;

        if (N_id == NODE_TYPE (arg1)) {
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
            DBUG_ASSERT (N_array == NODE_TYPE (arg1), "wrong node != N_array");
            DBUG_ASSERT (NULL != ARRAY_TYPE (arg1), " info.types is NULL");
            COUNT_ELEMS (length, ARRAY_AELEMS (arg1));
            MAKENODE_NUM (n_node, length);
            if (1 < TYPES_DIM (ARRAY_TYPE (arg1))) {
                node *dummy;
                /* it is an array of arrays, so we have to use
                 * ND_CREATE_CONST_ARRAY_A
                 */
                DBUG_ASSERT (N_id == NODE_TYPE (arg1->node[0]->node[0]),
                             "wrong node != N_id");
                GET_LENGTH (length, VARDEC_TYPE (ID_VARDEC (arg1->node[0]->node[0])));
                MAKENODE_NUM (n_length, length);

                CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_A", res, n_length,
                                  n_node);
                icm_arg->node[1] = ARRAY_AELEMS (arg1);
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
 *  functionname  : COMPPrf
 *  arguments     : 1) N_prf node
 *                  2) NULL
 *  description   : transforms N_prf node to N_icm nodes if prf works on
 *                  arrays
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before
 *                   last assign_node (to get last or next assign node use
 *                                     macros LAST_ASSIGN or NEXT_ASSIGN )
 *                  INFO_COMP_LASTASSIGN(arg_info) is used to insert new assign_nodes
 *                   in front of or after last assign node
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to last N_let
 */

node *
COMPPrf (node *arg_node, node *arg_info)
{
    node *array, *scalar, *tmp, *res, *res_ref, *n_node, *icm_arg, *prf_id_node,
      *type_id_node, *arg1, *arg2, *arg3, *n_node1, *n_elems_node, *first_assign,
      *next_assign, *last_assign, *length_node, *tmp_array1, *tmp_array2, *dim_node,
      *tmp_rc, *exprs;
    node *old_arg_node;
    simpletype res_stype = LET_BASETYPE (INFO_COMP_LASTLET (arg_info));
    int dim, is_SxA = 0, n_elems = 0, is_drop = 0, array_is_const = 0, convert = 0;
    simpletype s_type;

    DBUG_ENTER ("COMPPrf");

    DBUG_PRINT ("COMP", ("%s line: %d", mdb_prf[PRF_PRF (arg_node)], arg_node->lineno));

    /*
     * NOTE:  F_neq should be the last "function enumerator" that hasn't
     *        arrays as arguments.
     */
    if (PRF_PRF (arg_node) > F_neq) {
        ids *let_ids = INFO_COMP_LASTIDS (arg_info);
        node *new_name, *new_assign, *old_name;
        int insert_assign = 0;

        exprs = PRF_ARGS (arg_node);
        /* test whether an identifier occures on the right and left side of a
         * let. In this case rename the one on the rigth side ,assign old and new
         * variable and add vardec for the new vaibale.
         * (e.g: A=A+1 => __A=A; A=__A+1; )
         */
        while (NULL != exprs) {
            if (N_id == NODE_TYPE (EXPRS_EXPR (exprs))) {
                if (0 == strcmp (IDS_NAME (let_ids), ID_NAME (EXPRS_EXPR (exprs)))) {
                    if (0 == insert_assign) {
                        MAKENODE_ID (new_name, RenameVar (IDS_NAME (let_ids), 0));
                        MAKENODE_ID_REUSE_IDS (old_name, let_ids);
                        CREATE_2_ARY_ICM (new_assign, "ND_KS_ASSIGN_ARRAY", old_name,
                                          new_name);
                        ASSIGN_NEXT (new_assign) = CURR_ASSIGN (arg_info);
                        INSERT_BEFORE (arg_info, new_assign);

                        /* set info_node to right node (update info_node )*/
                        INFO_COMP_LASTASSIGN (arg_info) = new_assign;

                        insert_assign = 1;

                        /* now insert vardec if necessary */
                        INFO_COMP_VARDECS (arg_info)
                          = AddVardec (INFO_COMP_VARDECS (arg_info),
                                       VARDEC_TYPE (IDS_VARDEC (let_ids)),
                                       ID_NAME (new_name), INFO_COMP_FUNDEF (arg_info));
                    }

                    /* now rename N_id */
                    FREE (ID_NAME (EXPRS_EXPR (exprs)));
                    ID_NAME (EXPRS_EXPR (exprs)) = RenameVar (IDS_NAME (let_ids), 0);
                }
            }

            exprs = EXPRS_NEXT (exprs);
        }

        switch (PRF_PRF (arg_node)) {
        case F_modarray:
            arg_node = COMPPrfModarray (arg_node, arg_info);
            break;
        case F_idx_modarray:
            arg_node = COMPIdxModarray (arg_node, arg_info);
            break;
        case F_add_SxA:
        case F_div_SxA:
        case F_sub_SxA:
        case F_mul_SxA:
            is_SxA = 1;
            /* here is NO break missing */
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
            MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));

            /* store prf as N_id */
            MAKENODE_ID (prf_id_node, prf_string[arg_node->info.prf]);

            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

            if (N_id == NODE_TYPE (array)) {
                last_assign = NEXT_ASSIGN (arg_info);

                CHECK_REUSE__ALLOC_ARRAY_ND (res, res_stype, array, array_stype);
            } else {
                /* array is constant, so make a block , declare a temporary
                 * variable __TMP and create a constant array
                 */

                array_is_const = 1;
                old_arg_node = arg_node;

                /* count number of elements */
                tmp = ARRAY_AELEMS (array);
                while (NULL != tmp) {
                    n_elems++;
                    tmp = EXPRS_NEXT (tmp);
                }

                MAKENODE_NUM (n_node, n_elems);
                MAKENODE_NUM (n_node1, 1);
                MAKENODE_ID (tmp_array1, "__TMP");

                /* reuse previous N_let */
                NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block;
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
            MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));

            /* store prf as N_id */
            MAKENODE_ID (prf_id_node, prf_string[arg_node->info.prf]);

            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* compute basic_type of arg1 and arg2 */
            GET_BASIC_SIMPLETYPE_OF_NODE (arg1_stype, arg1);
            GET_BASIC_SIMPLETYPE_OF_NODE (arg2_stype, arg2);

            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

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
                } else {
                    if (1 == arg1->refcnt) {
                        CHECK_REUSE__ALLOC_ARRAY_ND (res, res_stype, arg1, arg1_stype);
                    } else {
                        if (1 == arg2->refcnt) {
                            CHECK_REUSE__ALLOC_ARRAY_ND (res, res_stype, arg2,
                                                         arg2_stype);
                        } else {
                            node *num;

                            MAKENODE_NUM (num, 0);
                            TRI_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                                           type_id_node, res, num);
                            first_assign = CURR_ASSIGN (arg_info);
                            old_arg_node = arg_node;
                            last_assign = NEXT_ASSIGN (arg_info);
                            arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));

                            DBUG_ASSERT ((((-1 == arg1->refcnt) && (-1 == arg2->refcnt))
                                          || (opt_rco == 0)),
                                         "Refcnt of BINOP_A_A arg neither -1 nor 1 !");
                        }
                    }
                }
            } else {
                /* reuse previous N_let */
                NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block;
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
                } else {
                    if (N_array == arg1->nodetype) {
                        array_is_const = 1;
                        DECL_ARRAY (first_assign, arg1->node[0], "__TMP1", tmp_array1);
                        arg_node = first_assign;
                        CREATE_CONST_ARRAY (arg1, tmp_array1, type_id_node, res_ref);
                        CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array1,
                                          res);
                        APPEND_ASSIGNS (first_assign, next_assign);
                        /* set arg1 for later use as parameters of BIN_OP */
                        arg1 = tmp_array1;
                    } else {
                        array_is_const = 2;
                        DECL_ARRAY (first_assign, arg2->node[0], "__TMP2", tmp_array2);
                        arg_node = first_assign;
                        CREATE_CONST_ARRAY (arg2, tmp_array2, type_id_node, res_ref);
                        CREATE_2_ARY_ICM (next_assign, "ND_KS_ASSIGN_ARRAY", tmp_array2,
                                          res);
                        APPEND_ASSIGNS (first_assign, next_assign);
                        /* set arg2 for later use as parameters of BIN_OP */
                        arg2 = tmp_array2;
                    }
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
             * vardec ( don't free INFO_COMP_LASTIDS(arg_info) !!! )
             *
             * if first argument of prf is a scalar (N_um), it will be compiled
             * like an vector (array) with one element
             */
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            DBUG_ASSERT (((N_array == arg1->nodetype) || (N_num == arg1->nodetype)),
                         "first argument of take/drop isn't an array or scalar");

            MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));

            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

            MAKENODE_NUM (num, 0);
            if (N_id == arg2->nodetype) {
                GET_DIM (dim, VARDEC_TYPE (ID_VARDEC (arg2)));
                MAKENODE_NUM (dim_node, dim); /* store dimension of argument-array */
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                               type_id_node, res);
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
            FreeTree (arg1);
            FREE (arg_node->node[0]);
            if (N_array == NODE_TYPE (arg2)) {
                arg_node = COMPArray (arg2, arg_info);
            } else {
                DBUG_ASSERT ((N_id == NODE_TYPE (arg2)), "wrong nodetype");
                if (0
                    == strcmp (ID_NAME (arg2), IDS_NAME (INFO_COMP_LASTIDS (arg_info)))) {
                    FREE_IDS (LET_IDS (arg2));
                    FREE (arg2);
                    FREE (arg_node->node[0]->node[1]);
                    arg_node = NULL;
                    NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_icm;
                    /*
                     * don't free LET_IDS( INFO_COMP_LASTLET( arg_info)),
                     *  because it is shared with vardec!
                     */
                    ICM_NAME (INFO_COMP_LASTLET (arg_info)) = "NOOP";
                } else {
                    MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));
                    BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_KS_ASSIGN_ARRAY",
                                   arg2, res);
                    SET_VARS_FOR_MORE_ICMS;
                    if (ID_REFCNT (res) > 1) {
                        MAKENODE_NUM (n_node, ID_REFCNT (res) - 1);
                        INC_RC_ND (res, n_node);
                        INSERT_ASSIGN;
                    } else {
                        if (0 == ID_REFCNT (res)) {
                            MAKENODE_NUM (n_node, 1);
                            DEC_OR_FREE_RC_ND (res, n_node);
                            INSERT_ASSIGN;
                        }
                    }
                    FREE (old_arg_node);
                }
            }
            break;
        }
        case F_psi: {
            /* store arguments and result (res contains refcount and pointer to
             * vardec ( don't free INFO_COMP_LASTIDS(arg_info) !!! )
             */
            node *line, *arg1_length;

            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));
            MAKENODE_NUM (line, arg_node->lineno);

            last_assign = NEXT_ASSIGN (arg_info);

            /* compute basic type */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* compute length of arg1 */
            if (N_id == NODE_TYPE (arg1)) {
                GET_LENGTH (n_elems, VARDEC_TYPE (ID_VARDEC (arg1)));
                MAKENODE_NUM (arg1_length, n_elems);
            } else {
                n_elems = 0;
                tmp = arg1->node[0];
                do {
                    n_elems++;
                    tmp = tmp->node[1];
                } while (NULL != tmp);
                MAKENODE_NUM (arg1_length, n_elems);
            }

            if (0 == IsArray (VARDEC_TYPE (ID_VARDEC (res)))) {
                if (N_id == arg2->nodetype) {
                    if (N_id == arg1->nodetype) {
                        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_KD_PSI_VxA_S",
                                       line, arg2);
                        MAKE_NEXT_ICM_ARG (icm_arg, res);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1_length);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                    } else {
                        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_KD_PSI_CxA_S",
                                       line, arg2);
                        MAKE_NEXT_ICM_ARG (icm_arg, res);
                        MAKE_NEXT_ICM_ARG (icm_arg, arg1_length);
                        icm_arg->node[1] = arg1->node[0];
                        /*
                         * FREE(arg1);
                         *
                         * arg1 cannot be freed here since it is used 6 lines
                         * farther.
                         */
                    }
                    first_assign = CURR_ASSIGN (arg_info);
                    old_arg_node = arg_node;
                    arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));
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
                        FREE (arg1);
                    }
                    APPEND_ASSIGNS (first_assign, next_assign);
                }
            } else {
                node *num;

                DBUG_ASSERT ((N_id == NODE_TYPE (arg2)), "arg2 != N_id");
                GET_DIM (dim, VARDEC_TYPE (ID_VARDEC (arg2)));
                MAKENODE_NUM (dim_node, dim);
                /* store refcount of res as N_num */
                MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

                MAKENODE_NUM (num, 0);
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                               type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, num);
                SET_VARS_FOR_MORE_ICMS;

                if (N_id == arg1->nodetype) {
                    CREATE_6_ARY_ICM (next_assign, "ND_KD_PSI_VxA_A", line, dim_node,
                                      arg2, res, arg1_length, arg1);
                } else {
                    CREATE_5_ARY_ICM (next_assign, "ND_KD_PSI_CxA_A", line, dim_node,
                                      arg2, res, arg1_length);
                    icm_arg->node[1] = arg1->node[0];
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
            DBUG_ASSERT (N_id == arg2->nodetype, "wrong second arg of idx_psi");

#ifdef OLD_IDX_PSI
            /* reuse last N_let node */
            NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_icm;
            MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));
            if (1 == IsArray (VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))))) {
#if 0
             types *b_type;
             int length, i;
             node *next_icm_arg;

             if (T_user == TYPES_BASETYPE(VARDEC_TYPE(IDS_VARDEC(
                                        INFO_COMP_LASTIDS(arg_info))))) {
               GET_BASIC_TYPE(b_type, VARDEC_TYPE(IDS_VARDEC(
                                        INFO_COMP_LASTIDS(arg_info))), 0);
             }
             else {
               b_type=VARDEC_TYPE(IDS_VARDEC(INFO_COMP_LASTIDS(arg_info)));
             }

             length=1;
             for (i=0; i < b_type->dim; i++) {
               length *= b_type->shpseg->shp[i];
             }
             MAKENODE_NUM(n_node, length);
#endif
                ICM_NAME (INFO_COMP_LASTLET (arg_info)) = "ND_IDX_PSI_A";

                MAKE_ICM_ARG (icm_arg, res);
#if 0
             next_icm_arg=icm_arg;
             MAKE_NEXT_ICM_ARG(next_icm_arg, n_node);
#endif
            } else {
                ICM_NAME (INFO_COMP_LASTLET (arg_info)) = "ND_IDX_PSI_S";
                MAKE_ICM_ARG (icm_arg, res);
            }
            /* append res to arguments of current node  */
            arg_node->node[0]->node[1]->node[1] = icm_arg;

            /* set arg_node, because arg_node will be returned */
            old_arg_node = arg_node;
            arg_node = arg_node->node[0];
            FREE (old_arg_node);
            break;
#endif
            MAKENODE_NUM (arg2_ref, 1);
            MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));
            if (1 == IsArray (VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))))) {
                MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));
                GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (IDS_VARDEC (
                                                INFO_COMP_LASTIDS (arg_info))));
                MAKENODE_ID (type_id_node, type_string[s_type]);
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                               type_id_node, res);
                SET_VARS_FOR_MORE_ICMS;
                MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
                CREATE_3_ARY_ICM (next_assign, "ND_IDX_PSI_A", arg1, arg2, res);
                APPEND_ASSIGNS (first_assign, next_assign);
            } else {
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_IDX_PSI_S", arg1, arg2);
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
            if (N_array == arg1->nodetype) {
                arg_node->info.cint = 1;
            } else {
                node *id_node;

                GET_DIM (arg_node->info.cint, VARDEC_TYPE (ID_VARDEC (arg1)));
                first_assign = CURR_ASSIGN (arg_info);
                last_assign = NEXT_ASSIGN (arg_info);
                MAKENODE_NUM (n_node, 1);
                MAKENODE_ID (id_node, ID_NAME (arg1));
                ID_REFCNT (id_node) = ID_REFCNT (arg1);
                DEC_OR_FREE_RC_ND (id_node, n_node);
                INSERT_ASSIGN;
            }
            FreeTree (arg_node->node[0]);
            break;
        }
        case F_shape: {
            int dim;
            arg1 = arg_node->node[0]->node[0];
            /* store type of new array */
            MAKENODE_ID (type_id_node, "int");
            /* store name of new array */
            MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));
            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));
            BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", type_id_node,
                           res);
            MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
            SET_VARS_FOR_MORE_ICMS;
            if (N_id == arg1->nodetype) {
                GET_DIM (dim, VARDEC_TYPE (ID_VARDEC (arg1)));
                tmp_array1 = ShapeToArray (ID_VARDEC (arg1));
            } else {
                DBUG_ASSERT ((N_array == NODE_TYPE (arg1)), "wrong nodetype");
                COUNT_ELEMS (n_elems, arg1->node[0]);
                tmp_array1 = MakeNode (N_exprs);
                MAKENODE_NUM (tmp_array1->node[0], n_elems);
                dim = 1;
            }
            MAKENODE_NUM (length_node, dim); /* store length of shape_vector */
            CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res, length_node);
            icm_arg->node[1] = tmp_array1; /* append shape_vector */
            APPEND_ASSIGNS (first_assign, next_assign);
            if (N_id == arg1->nodetype) {
                node *id_node;
                MAKENODE_NUM (n_node, 1);
                MAKENODE_ID (id_node, ID_NAME (arg1));
                ID_REFCNT (id_node) = ID_REFCNT (arg1);

                DEC_OR_FREE_RC_ND (id_node, n_node);
            }
            FreeTree (old_arg_node);
            INSERT_ASSIGN;
            break;
        }
        case F_cat: {
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            arg3 = arg_node->node[0]->node[1]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));
            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            MAKENODE_ID (type_id_node, type_string[s_type]);
            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

            if ((N_id == arg2->nodetype) && (N_id == arg3->nodetype)) {
                GET_DIM (dim, VARDEC_TYPE (ID_VARDEC (arg2)));
                MAKENODE_NUM (dim_node, dim);
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                               type_id_node, res);
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
                /* reuse previous N_let */
                NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block;
                old_arg_node = arg_node;
                MAKENODE_NUM (tmp_rc, 0);
                if ((N_array == NODE_TYPE (arg2)) && (N_array == NODE_TYPE (arg3))) {
                    array_is_const = 3;
                    GET_DIM (dim, ARRAY_TYPE (arg2));
                    DECL_ARRAY (first_assign, arg2->node[0], "__TMP1", tmp_array1);
                    arg_node = first_assign; /* set new arg_node */
                    DECL_ARRAY (next_assign, arg3->node[0], "__TMP2", tmp_array2);
                    APPEND_ASSIGNS (first_assign, next_assign);
                    CREATE_CONST_ARRAY (arg2, tmp_array1, type_id_node, tmp_rc);
                    CREATE_CONST_ARRAY (arg3, tmp_array2, type_id_node, tmp_rc);
                    arg2 = tmp_array1;
                    arg3 = tmp_array2;
                } else {
                    if (N_array == arg2->nodetype) {
                        array_is_const = 1;
                        GET_DIM (dim, ARRAY_TYPE (arg2));
                        DECL_ARRAY (first_assign, arg2->node[0], "__TMP1", tmp_array1);
                        arg_node = first_assign;
                        CREATE_CONST_ARRAY (arg2, tmp_array1, type_id_node, tmp_rc);
                        /* set arg2 for later use as parameters of ND_KD_CAT*/
                        arg2 = tmp_array1;
                    } else {
                        array_is_const = 2;
                        GET_DIM (dim, ARRAY_TYPE (arg3));
                        DECL_ARRAY (first_assign, arg3->node[0], "__TMP2", tmp_array2);
                        arg_node = first_assign;
                        CREATE_CONST_ARRAY (arg3, tmp_array2, type_id_node, tmp_rc);
                        /* set arg3 for later use as parameters of ND_KD_CAT*/
                        arg3 = tmp_array2;
                    }
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
                case 1:
                    DEC_OR_FREE_RC_ND (arg3, n_node);
                    DEC_RC_FREE_ND (arg2, n_node);
                    break;
                case 2:
                    DEC_OR_FREE_RC_ND (arg3, n_node);
                    DEC_RC_FREE_ND (arg3, n_node);
                    break;
                case 3:
                    DEC_RC_FREE_ND (arg2, n_node);
                    DEC_RC_FREE_ND (arg3, n_node);
                    break;
                default:
                    DBUG_ASSERT (0, "array_is_const is out of range");
                    break;
                }
                FREE (old_arg_node);
            }
            break;
        }
        case F_rotate:
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            arg3 = arg_node->node[0]->node[1]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));
            /* compute basic_type of result */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            MAKENODE_ID (type_id_node, type_string[s_type]);
            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));
            MAKENODE_NUM (n_node, 1);

            if (N_id == arg3->nodetype) {
                GET_DIM (dim,
                         VARDEC_TYPE (ID_VARDEC (arg3))); /* dim will be used later */
                BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY",
                               type_id_node, res);
                MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
                SET_VARS_FOR_MORE_ICMS;
            } else {
                array_is_const = 1;
                /* reuse previous N_let */
                NODE_TYPE (INFO_COMP_LASTLET (arg_info)) = N_block;
                old_arg_node = arg_node;
                GET_DIM (dim, ARRAY_TYPE (arg3)); /* dim will be used later */
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
            MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));
            /* compute basic type */
            GET_BASIC_SIMPLETYPE (s_type, VARDEC_TYPE (
                                            IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
            MAKENODE_ID (type_id_node, type_string[s_type]);
            MAKENODE_NUM (res_rc, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));
            BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", type_id_node,
                           res);
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
                DBUG_ASSERT (NULL != ARRAY_TYPE (arg1), " info.types is NULL");
                COUNT_ELEMS (length, arg1->node[0]);
                MAKENODE_NUM (n_node, length);
                if (1 < TYPES_DIM (ARRAY_TYPE (arg1))) {
                    node *dummy;
                    /* it is an array of arrays, so we have to use
                     * ND_CREATE_CONST_ARRAY_A
                     */
                    DBUG_ASSERT (N_id == NODE_TYPE (ARRAY_AELEMS (ARRAY_AELEMS (arg1))),
                                 "wrong node != N_id");
                    GET_LENGTH (length, VARDEC_TYPE (ID_VARDEC (
                                          ARRAY_AELEMS (ARRAY_AELEMS (arg1)))));
                    MAKENODE_NUM (n_length, length);

                    CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_A", res,
                                      n_length, n_node);
                    icm_arg->node[1] = arg1->node[0];
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
                    APPEND_ASSIGNS (first_assign, next_assign);
                }
                INSERT_ASSIGN;
            }
            break;
        }
        case F_toi_A:
        case F_tod_A:
        case F_tof_A: {
            arg_node = COMPConvert (arg_node, arg_info);
            break;
        }
        default:
#if 0
         DBUG_ASSERT(0,"wrong prf");
#endif
            break;
        }
    } else { /* (arg_node->info.prf > F_neq) */
        switch (PRF_PRF (arg_node)) {
        case F_ftoi:
        case F_ftod:
        case F_itof:
        case F_itod:
        case F_dtof:
        case F_dtoi: {
            node *dummy = arg_node;
            arg_node = arg_node->node[0]->node[0];
            FREE (dummy->node[0]); /* free N_exprs node */
            FREE (dummy);          /* free N_prf node */
            break;
        }

        case F_toi:
        case F_tof:
        case F_tod:
            arg_node = COMPConvert (arg_node, arg_info);
            break;

        case F_min:
            NODE_TYPE (arg_node) = N_icm;
            ICM_NAME (arg_node) = "ND_MIN";
            break;

        case F_max:
            NODE_TYPE (arg_node) = N_icm;
            ICM_NAME (arg_node) = "ND_MAX";
            break;

        case F_abs:
            NODE_TYPE (arg_node) = N_icm;
            ICM_NAME (arg_node) = "ND_ABS";
            break;

        default:
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPArray
 *  arguments     : 1) N_arrray node
 *                  2) info node
 *  description   :
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before
 *                    last assign_node
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to previous N_let
 *
 */

node *
COMPArray (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *exprs, *res, *type_id_node, *old_arg_node,
      *icm_arg, *n_node, *res_ref, *last_assign;

    int n_elems = 0;
    int icm_created = 0;

    simpletype s_type;

    DBUG_ENTER ("COMPArray");

    /* store next assign */
    last_assign = NEXT_ASSIGN (arg_info);

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type,
                          VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* store result as N_id */
    MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

    /* create ND_ALLOC_ARRAY */
    BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", type_id_node, res);
    MAKE_NEXT_ICM_ARG (icm_arg, res_ref);
    first_assign = CURR_ASSIGN (arg_info);
    old_arg_node = arg_node;
    arg_node = LET_EXPR (INFO_COMP_LASTLET (arg_info));

    /* create ND_CREATE_CONST_ARRAY */

    exprs = ARRAY_AELEMS (old_arg_node);
#if 0
   if (ARRAY_BASETYPE(old_arg_node)==T_char) {
     /*
      * When defining a constant array of char, we try to use a string notation.
      * This is only possible if the character array really represents a string,
      * i.e. it does not contain intermediate null-characters and the last character
      * actually is a null-character. In this cases the ICM ND_CREATE_CONST_ARRAY_C
      * is used instead of ND_CREATE_CONST_ARRAY_S.
      */

     use_string_notation = 1;
     do {
       if (((CHAR_VAL(EXPRS_EXPR(exprs))=='\0')
	       && (EXPRS_NEXT(exprs)!=NULL))
	   || ((CHAR_VAL(EXPRS_EXPR(exprs))!='\0')
	       && (EXPRS_NEXT(exprs)==NULL))
	   || ((!isprint(CHAR_VAL(EXPRS_EXPR(exprs))))
	       &&) {
	 use_string_notation = 0;
       }
       
       n_elems ++;
       exprs = EXPRS_NEXT(exprs);
     }
     while (NULL != exprs);
   }
#endif

    do {
        n_elems++;
        exprs = EXPRS_NEXT (exprs);
    } while (NULL != exprs);

    MAKENODE_NUM (n_node, n_elems);
    DBUG_ASSERT ((NULL != old_arg_node->node[0]), " NULL pointer ");

    if (N_id == old_arg_node->node[0]->node[0]->nodetype) {
        if (1 == IsArray (VARDEC_TYPE (ID_VARDEC (old_arg_node->node[0]->node[0])))) {
            node *length;
            int len;
            GET_LENGTH (len, VARDEC_TYPE (ID_VARDEC (old_arg_node->node[0]->node[0])));
            MAKENODE_NUM (length, len);
            CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_A", res, length,
                              n_node);
            icm_created = 1;
        } else {
            if (IsNonUniqueHidden (
                  VARDEC_TYPE (ID_VARDEC (old_arg_node->node[0]->node[0])))) {
                CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_H", res,
                                  MAKE_IDNODE (StringCopy (
                                    GenericFun (0, VARDEC_TYPE (ID_VARDEC (
                                                     old_arg_node->node[0]->node[0]))))),
                                  n_node);
                icm_created = 1;
            }
        }
    }

    if (0 == icm_created) {
        if (ARRAY_STRING (old_arg_node) != NULL) {

            CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_C", res,
                              MakeStr (ARRAY_STRING (old_arg_node)));
        } else {
            CREATE_2_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY_S", res, n_node);
            /* now append the elements of the array to the last N_icm */
            icm_arg->node[1] = old_arg_node->node[0];
        }
    } else {
        /* now append the elements of the array to the last N_icm */
        EXPRS_NEXT (icm_arg) = old_arg_node->node[0];
    }

    APPEND_ASSIGNS (first_assign, next_assign);

    INSERT_ASSIGN;

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPId
 *  arguments     : 1) N_id node
 *                  2) N_info with VARDECS of fun, LASTASSIGN, LASTLET
 *                     and LASTIDS!
 *  description   :
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to to node before
 *                   last assign_node
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to previous N_let
 *
 */

node *
COMPId (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *last_assign, *old_arg_node, *icm_arg, *res,
      *n_node, *icm_node;

    DBUG_ENTER ("COMPId");

    if (MUST_REFCOUNT (ID, arg_node)) {
        if (NULL != arg_info) {
            MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));

            if (ID_MAKEUNIQUE (arg_node)) {
                if (ID_REFCNT (arg_node) == 1) {
                    if (IsArray (VARDEC_TYPE (ID_VARDEC (arg_node)))) {
                        TRI_ICM_REUSE (INFO_COMP_LASTLET (arg_info),
                                       "ND_KS_MAKE_UNIQUE_ARRAY", arg_node, res,
                                       MakeNum (BasetypeSize (
                                         VARDEC_TYPE (ID_VARDEC (arg_node)))));

                        SET_VARS_FOR_MORE_ICMS;
                        SET_RC_ND (res,
                                   MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info))));
                        INSERT_ASSIGN;
                    } else {
                        TRI_ICM_REUSE (INFO_COMP_LASTLET (arg_info),
                                       "ND_NO_RC_MAKE_UNIQUE_HIDDEN", arg_node, res,
                                       MAKE_IDNODE (StringCopy (
                                         GenericFun (0, VARDEC_TYPE (
                                                          ID_VARDEC (arg_node))))));
                    }
                } else {
                    if (IsArray (VARDEC_TYPE (ID_VARDEC (arg_node)))) {
                        TRI_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_KS_COPY_ARRAY",
                                       arg_node, res,
                                       MakeNum (BasetypeSize (
                                         VARDEC_TYPE (ID_VARDEC (arg_node)))));

                        SET_VARS_FOR_MORE_ICMS;

                        CREATE_1_ARY_ICM (next_assign, "ND_ALLOC_RC",
                                          MAKE_IDNODE (StringCopy (ID_NAME (res))));
                        APPEND_ASSIGNS (first_assign, next_assign);

                        SET_RC_ND (res,
                                   MakeNum (IDS_REFCNT (INFO_COMP_LASTIDS (arg_info))));
                        DEC_RC_ND (MAKE_IDNODE (StringCopy (ID_NAME (old_arg_node))),
                                   MakeNum (1));

                        INSERT_ASSIGN;
                    } else {
                        TRI_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_COPY_HIDDEN",
                                       arg_node, res,
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
                    BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_KS_ASSIGN_ARRAY",
                                   arg_node, res);
                } else {
                    BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ASSIGN_HIDDEN",
                                   arg_node, res);
                }

                SET_VARS_FOR_MORE_ICMS;

                if (0 == IDS_REFCNT (INFO_COMP_LASTIDS (arg_info))) {
                    MAKENODE_NUM (n_node, 1);
                    DEC_RC_FREE_ND (res, n_node);
                } else {
                    if (1 < IDS_REFCNT (INFO_COMP_LASTIDS (arg_info))) {
                        MAKENODE_NUM (n_node,
                                      IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)) - 1);
                        INC_RC_ND (res, n_node);
                    }
                }

                INSERT_ASSIGN;
            }
        } else {
            DBUG_ASSERT ((arg_info != NULL), "corrupted arg-info");

            icm_node = MakeNode (N_icm);
            ICM_NAME (icm_node) = "ND_KS_RET_ARRAY";
            MAKE_ICM_ARG (ICM_ARGS (icm_node), arg_node);
            arg_node = icm_node;
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPAp
 *  arguments     : 1) N_Ap node
 *                  2) info node
 *  description   : - creates N_icm for function application
 *                  - insert N_icm to decrement the refcount of functions
 *                    arguments
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable.
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before
 *                    last assign_node.
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to previous N_let.
 *                  INFO_COMP_VARDECS(arg_info) contains pointer to vardecs.
 *
 */

node *
COMPAp (node *arg_node, node *arg_info)
{
    node *tmp, *next, *exprs, *icm_arg, *save_icm_arg, *id_node, *tag_node, *next_assign,
      *first_assign, *add_assigns_before, *fundef_args, *add_assigns_after, *last_assign,
      **icm_tab, *icm_tab_entry;
    ids *ids;
    int i, cnt_param, tab_size, ids_for_dots = 0;
    types *fundef_rettypes;
    char *tag;

    DBUG_ENTER ("COMPAp");

    ids = LET_IDS (INFO_COMP_LASTLET (arg_info));
    fundef_rettypes = FUNDEF_TYPES (AP_FUNDEF (arg_node));

    add_assigns_before = MakeNode (N_assign);
    add_assigns_after = MakeNode (N_assign);
    /*
     * will be used to store N_icms
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
    icm_tab = (node **)Malloc (sizeof (node *) * tab_size);

    for (i = 0; i < tab_size; i++) {
        icm_tab[i] = NULL;
    }

    DBUG_PRINT ("COMP", ("COMPiling application of function %s",
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
                    /*
                     * create N_icm to increment refcount of function result.
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

                    INFO_COMP_VARDECS (arg_info)
                      = MakeVardec (StringCopy (ID_NAME (id_node)),
                                    DuplicateTypes (VARDEC_TYPE (IDS_VARDEC (ids)), 1),
                                    INFO_COMP_VARDECS (arg_info));

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
                                     (FUNDEF_PRAGMA (AP_FUNDEF (arg_node)) == NULL)
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

        if ((N_id == NODE_TYPE (EXPRS_EXPR (exprs)))
            && (MUST_REFCOUNT (ID, EXPRS_EXPR (exprs)))) {
            if (FUN_DOES_REFCOUNT (AP_FUNDEF (arg_node), cnt_param)) {
                tag = ID_ATTRIB (EXPRS_EXPR (exprs)) == ST_inout ? "inout_rc" : "in_rc";
            } else {
                if (ID_ATTRIB (EXPRS_EXPR (exprs)) == ST_inout) {
                    if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_Cfun) {
                        if (IsBoxed (ARG_TYPE (fundef_args))) {
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
            }
        } else {
            if ((NODE_TYPE (EXPRS_EXPR (exprs)) == N_id)
                && (ID_ATTRIB (EXPRS_EXPR (exprs)) == ST_inout)) {
                if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_Cfun) {
                    if (IsBoxed (ARG_TYPE (fundef_args))) {
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

        if (ids_for_dots) {
            icm_arg = save_icm_arg;

            MAKE_NEXT_ICM_ARG (icm_arg, MAKE_IDNODE (tag));
            EXPRS_NEXT (icm_arg) = exprs;
            EXPRS_NEXT (exprs) = NULL;
            icm_arg = exprs;

            save_icm_arg = icm_arg;
            /*
             *  The value of icm_arg has to be saved here because a
             *  very intelligent programmer used this variable implicitly(!!)
             *  in macros like CREATE_3_ARY_ICM.
             */

            if (next == NULL) {
                InsertApDotsParam (icm_tab, icm_tab_entry);
            }
        } else {
            if (ARG_BASETYPE (fundef_args) == T_dots) {
                ids_for_dots = 1;

                MAKE_ICM_ARG (icm_arg, MAKE_IDNODE (tag));
                icm_tab_entry = icm_arg;
                EXPRS_NEXT (icm_arg) = exprs;
                EXPRS_NEXT (exprs) = NULL;
                icm_arg = exprs;

                save_icm_arg = icm_arg;

                if (next == NULL)
                    InsertApDotsParam (icm_tab, icm_tab_entry);
            } else {
                MAKE_ICM_ARG (icm_tab_entry, MAKE_IDNODE (tag));
                EXPRS_NEXT (icm_tab_entry) = exprs;
                EXPRS_NEXT (exprs) = NULL;

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

    INFO_COMP_LASTLET (arg_info)
      = CreateApIcm (INFO_COMP_LASTLET (arg_info), FUNDEF_NAME (AP_FUNDEF (arg_node)),
                     icm_tab, tab_size);

    AdjustAddedAssigns (add_assigns_before, add_assigns_after);

    /*
     *  First, new assign-nodes are inserted behind the current one.
     */

    last_assign = CURR_ASSIGN (arg_info);

    if (NULL != ASSIGN_NEXT (add_assigns_after)) {
        ASSIGN_NEXT (first_assign) = NEXT_ASSIGN (arg_info);
        ASSIGN_NEXT (last_assign) = ASSIGN_NEXT (add_assigns_after);
    }

    /*
     *  Second, new assign-nodes are inserted before the current one.
     */

    if (ASSIGN_NEXT (add_assigns_before) != NULL) {
        INSERT_BEFORE (arg_info, ASSIGN_NEXT (add_assigns_before));

        ASSIGN_NEXT (add_assigns_before)
          = AppendAssign (ASSIGN_NEXT (add_assigns_before), last_assign);
    }

    FREE (add_assigns_after);
    FREE (add_assigns_before);
    FREE (icm_tab);

    DBUG_RETURN (ICM_ARGS (INFO_COMP_LASTLET (arg_info)));
}

/*
 *  functionname  : COMPWithReturn
 *  arguments     : 1) N_return node
 *                  2) info node
 *  description   : generates N_icms of a with-loop
 *  remarks       : if N_return node contains to a with_loop, then
 *                   INFO_COMP_WITHBEGIN(arg_info) will point to the first argument
 *                   (N_exprs) of corresponding N_icm for start of with_loop.
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to node before last
 *                    assign_node.
 */

node *
COMPWithReturn (node *arg_node, node *arg_info)
{
    node *ret_val, *with_icm_arg, *icm_arg, *index_length, *tmp_with_icm_arg, *dec_rc,
      *res, *res_dim, *exprs, *from, *to, *index_node, *next_assign, *first_assign,
      *n_node, *mod_array = NULL;
    int is_array, con_type = 0;

#define MOD_A 1
#define GEN 2
#define FOLD 3

    /* arg_node is a N_return of a with_loop.
     * INFO_COMP_WITHBEGIN(arg_info) points to the N_icm that desribes the begin of
     * a with_loop.
     * The arguments of this N_icm are usefull to get the name of
     * the resulting array, the dimension of this array, and the
     * length of the index_vector.
     * The name of the N_icm is used to detect whether N_return node is part
     * of a 'genarray' or 'modarray' with-loop.
     */

    DBUG_ENTER ("COMPWithReturn");

    exprs = arg_node->node[0];
    is_array = IsArray (VARDEC_TYPE (ID_VARDEC ((exprs->node[0]))));
    switch (INFO_COMP_WITHBEGIN (arg_info)->info.id[9]) {
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
        DBUG_ASSERT (0, "unknown con-expr");
    }

    /* N_return will be converted to N_icm node */
    arg_node->nodetype = N_icm;
    ret_val = arg_node->node[0]->node[0]; /* store return_value node */
    with_icm_arg = INFO_COMP_WITHBEGIN (arg_info)->node[0];
    /* 'dec_rc' points to a list of variables whose refcount has to be
     * decremented
     */
    dec_rc = INFO_COMP_WITHBEGIN (arg_info)->node[3]->node[0];

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
            ICM_NAME (arg_node) = "ND_END_MODARRAY_S";
        } else {
            ICM_NAME (arg_node) = "ND_END_MODARRAY_A";

            /* add length of index_vector */
            MAKE_NEXT_ICM_ARG (icm_arg, index_length);
        }
    } else {
        if (GEN == con_type) {
            MAKE_ICM_ARG (arg_node->node[0], res);
            icm_arg = arg_node->node[0];
            MAKE_NEXT_ICM_ARG (icm_arg, res_dim);
            /* N_return belongs to a 'genarray' with-loop */
            /* add name of return value */
            MAKE_NEXT_ICM_ARG (icm_arg, ret_val);
            if (0 == is_array) {
                ICM_NAME (arg_node) = "ND_END_GENARRAY_S";
            } else {
                ICM_NAME (arg_node) = "ND_END_GENARRAY_A";
                MAKE_NEXT_ICM_ARG (icm_arg, index_length);
            }
        } else {
            if (FOLD == con_type) {
                node *let, *exprs1, *exprs2, *last_assign, *tmp_res;

                arg_node->nodetype = N_let;
                let = arg_node;
                /*
                 * in INFO_COMP_WITHBEGIN(arg_info)->node[2] the kind of the fold-function
                 *  is stored (as N_prf or as N_ap)
                 */
                let->node[0] = INFO_COMP_WITHBEGIN (arg_info)->node[2];
                LET_IDS (let) = MakeIds (ID_NAME (res), NULL, ST_regular);
                IDS_REFCNT (LET_IDS (let)) = (ID_REFCNT (res) == -1) ? -1 : 1;
                ID_VARDEC (let) = ID_VARDEC (res);
#if 0
            IDS_REFCNT(LET_IDS(let)) = 1;
#endif
                exprs1 = MakeNode (N_exprs);
                MAKENODE_ID (tmp_res, StringCopy (ID_NAME (res)));
                ID_REFCNT (tmp_res) = ID_REFCNT (res);
                ID_VARDEC (tmp_res) = ID_VARDEC (res);
                exprs1->node[0] = tmp_res;
                exprs2 = MakeNode (N_exprs);
                exprs2->node[0] = ret_val;
                exprs1->node[1] = exprs2;
                let->node[0]->node[0] = exprs1;
                INFO_COMP_LASTIDS (arg_info) = LET_IDS (let);
                INFO_COMP_LASTLET (arg_info) = let;
                let->node[0] = Trav (let->node[0], arg_info);

                last_assign = CURR_ASSIGN (arg_info);
                while (last_assign->node[1] != NULL)
                    last_assign = last_assign->node[1];

                next_assign = MakeAssign (MakeIcm ("ND_END_FOLD", NULL, NULL), NULL);
                MAKE_ICM_ARG (next_assign->node[0]->node[0], index_length);

                /* now insert next_assign */
                last_assign->node[1] = next_assign;
                first_assign = next_assign;
            }
        }
    }

    /* now create N_icm to decrement refcount of index_vector ,
     * left(from) and right(to) border of mod/genarray
     */
    MAKENODE_NUM (n_node, 1);
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE_ARRAY", index_node, n_node);
    if (FOLD != con_type) {
        first_assign = next_assign;
        /* now insert next_assign */
        ASSIGN_NEXT (ASSIGN_NEXT (INFO_COMP_LASTASSIGN (arg_info))) = next_assign;
    } else {
        APPEND_ASSIGNS (first_assign, next_assign);
    }
    DEC_OR_FREE_RC_ND (from, n_node);
    DEC_OR_FREE_RC_ND (to, n_node);
    if (NULL != mod_array) {
        DEC_RC_FREE_ND (mod_array, n_node);
    }

    while (NULL != dec_rc) {
        if ((0 != strcmp (ID_NAME (index_node), ID_NAME (dec_rc)))
            && ((NULL == mod_array)
                || (0 != strcmp (ID_NAME (mod_array), ID_NAME (dec_rc))))) {
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
        FREE (last_but_least_icm_arg->node[1]);
    }
#undef MOD_A
#undef GEN
#undef FOLD

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPReturn
 *  arguments     : 1) N_return node
 *                  2) info node
 *  description   : generates N_icms for N_return of a function (ND or MT)
 *                   or of a with-loop
 *  remarks       : if N_return node belongs to a with_loop, then
 *                  INFO_COMP_WITHBEGIN will point to the first argument (N_exprs)
 *                  of corresponding N_icm for start of with_loop.
 *
 */

node *
COMPReturn (node *arg_node, node *arg_info)
{
    node *tmp, *next, *exprs, *last;
    int cnt_param;

    DBUG_ENTER ("COMPReturn");

    if (NULL == INFO_COMP_WITHBEGIN (arg_info)) {
        /* this is a N_return of a function-body */

        node *ret;

        ret = RenameReturn (arg_node, arg_info);
        exprs = RETURN_EXPRS (arg_node);
        last = arg_node;
        /*
         * The new N_exprs chain will be stored in arg_node->node[1]
         * temporarily due to INSERT_ID_NODE and initial setting of 'last'.
         */

        /*
         * First, the real return values are traversed.
         */

        DBUG_PRINT ("COMP", ("Starting evaluation of return parameters"));

        DBUG_ASSERT ((arg_node->node[1] == NULL), "node[1] already used");

        cnt_param = 0;

        while (NULL != exprs) {
            DBUG_ASSERT ((N_id == NODE_TYPE (EXPRS_EXPR (exprs))),
                         " wrong node type (!= N_id)");

            DBUG_PRINT ("COMP", ("Current return id: %s", ID_NAME (EXPRS_EXPR (exprs))));

            next = EXPRS_NEXT (exprs);

            if (MUST_REFCOUNT (ID, EXPRS_EXPR (exprs))) {
                INSERT_ID_NODE (exprs, last, "out_rc");
            } else {
                INSERT_ID_NODE (exprs, last, "out");
            }

            /*
             *  In COMPReturn, we don't have to distinguish between functions
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
            DBUG_ASSERT ((N_id == NODE_TYPE (EXPRS_EXPR (exprs))),
                         " wrong node (!= N_id)");

            DBUG_PRINT ("COMP", ("Current return id: %s", ID_NAME (EXPRS_EXPR (exprs))));

            next = EXPRS_NEXT (exprs);

            if (MUST_REFCOUNT (ID, EXPRS_EXPR (exprs))) {
                INSERT_ID_NODE (exprs, last, "inout_rc");
            } else {
                INSERT_ID_NODE (exprs, last, "inout");
            }

            last = exprs;
            exprs = next;
        }

        DBUG_PRINT ("COMP", ("Handling counterparts of reference parameters finished"));

        NODE_TYPE (arg_node) = N_icm;

        if (arg_node->node[1] == NULL) {
            ICM_NAME (arg_node) = "NOOP";
        } else {
            if (FUNDEF_STATUS (INFO_COMP_FUNDEF (arg_info)) == ST_spmdfun) {
                ICM_NAME (arg_node) = "MT_SPMD_FUN_RET";
            } else {
                ICM_NAME (arg_node) = "ND_FUN_RET";
            }

            exprs = MakeNode (N_exprs);
            MAKENODE_NUM (EXPRS_EXPR (exprs), 0);
            EXPRS_NEXT (exprs) = arg_node->node[1];
            /* put number of ret-values in front */
            RETURN_EXPRS (arg_node) = exprs;
            arg_node->node[1] = NULL; /* was only used temporarily */

            exprs = MakeExprs (NULL, NULL);
            MAKENODE_ID (EXPRS_EXPR (exprs), "");
            EXPRS_NEXT (exprs) = RETURN_EXPRS (arg_node);
            RETURN_EXPRS (arg_node) = exprs;

            /*
             *  Function ReorganizeParameters is called later by COMPFunf.
             */
        }

        arg_node = ret; /* set new return_value of current function
                         * (N_let node if at least one variable in the "return"
                         * statement has been renamed, or  N_return otherwise)
                         */
    } else {
        arg_node = COMPWithReturn (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPArg
 *  arguments     : 1) N_arg node
 *                  2) info node
 *  description   :
 *  remarks       : INFO_COMP_FIRSTASSIGN(arg_info) contains pointer to first
 *                  assignment of block.
 *                  INFO_COMP_FUNDEF(arg_info) contains pointer to fundef.
 *                  INFO_COMP_CNTPARAM(arg_info) contains parameter number
 *
 */

node *
COMPArg (node *arg_node, node *arg_info)
{
    node *icm_arg, *id_node, *new_assign, *icm_tab_entry, *type_id_node;
    types *fulltype;
    char *tag;

    DBUG_ENTER ("COMPArg");

    GET_BASIC_TYPE (fulltype, ARG_TYPE (arg_node), 042);

    id_node = MAKE_IDNODE (NULL != ARG_NAME (arg_node) ? ARG_NAME (arg_node) : "");
    /* store name of formal parameter */

    type_id_node = MAKE_IDNODE (MakeTypeString (fulltype));

    if ((MUST_REFCOUNT (ARG, arg_node))
        && (FUN_DOES_REFCOUNT (INFO_COMP_FUNDEF (arg_info),
                               INFO_COMP_CNTPARAM (arg_info)))) {
        tag = (ARG_ATTRIB (arg_node) == ST_inout) ? "inout_rc" : "in_rc";

        /* put ND_INC_RC at beginning of function block */

        if (1 < ARG_REFCNT (arg_node)) {
            CREATE_2_ARY_ICM (new_assign, "ND_INC_RC", id_node,
                              MakeNum (ARG_REFCNT (arg_node) - 1));

            ASSIGN_NEXT (new_assign) = INFO_COMP_FIRSTASSIGN (arg_info);
            INFO_COMP_FIRSTASSIGN (arg_info) = new_assign;
        } else {
            if (0 == ARG_REFCNT (arg_node)) {
                CREATE_2_ARY_ICM (new_assign, "ND_DEC_RC_FREE_ARRAY", id_node,
                                  MakeNum (1));

                ASSIGN_NEXT (new_assign) = INFO_COMP_FIRSTASSIGN (arg_info);
                INFO_COMP_FIRSTASSIGN (arg_info) = new_assign;
            }
        }
    } else {
        if ((FUNDEF_PRAGMA (INFO_COMP_FUNDEF (arg_info)) != NULL)
            && (FUNDEF_REFCOUNTING (INFO_COMP_FUNDEF (arg_info)) != NULL)
            && (FUNDEF_REFCOUNTING (
                  INFO_COMP_FUNDEF (arg_info))[INFO_COMP_CNTPARAM (arg_info)]
                == 1)) {
            WARN (NODE_LINE (arg_node), ("Pragma 'refcounting` illegal"));
            CONT_WARN (("Function wants to do refcounting on non-refcounted "
                        "parameter no. %d",
                        INFO_COMP_CNTPARAM (arg_info)));
        }

        if (ARG_ATTRIB (arg_node) == ST_inout) {
            if (FUNDEF_STATUS (INFO_COMP_FUNDEF (arg_info)) == ST_Cfun) {
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
        InsertDefDotsParam (INFO_COMP_ICMTAB (arg_info), icm_tab_entry);
    } else {
        InsertDefArgParam (INFO_COMP_ICMTAB (arg_info), icm_tab_entry,
                           INFO_COMP_TYPETAB (arg_info), ARG_TYPE (arg_node),
                           NULL == FUNDEF_PRAGMA (INFO_COMP_FUNDEF (arg_info))
                             ? NULL
                             : FUNDEF_LINKSIGN (INFO_COMP_FUNDEF (arg_info)),
                           INFO_COMP_CNTPARAM (arg_info), NODE_LINE (arg_node));
    }

    INFO_COMP_CNTPARAM (arg_info)++;

    if (NULL != ARG_NEXT (arg_node)) {
        Trav (ARG_NEXT (arg_node), arg_info);
    }

    /*
     * Additional icms for the function body are generated regardless of the
     * existence of such a block, but COMPFundef only inserts them if a
     * block already exists.
     */

    if (TYPES_DIM (fulltype) > 0) {
        /* put N_icm "ND_KS_DECL_ARRAY_ARG" at beginning of function block */

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
        ASSIGN_NEXT (new_assign) = INFO_COMP_FIRSTASSIGN (arg_info);
        INFO_COMP_FIRSTASSIGN (arg_info) = new_assign;
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

        ASSIGN_NEXT (new_assign) = INFO_COMP_FIRSTASSIGN (arg_info);
        INFO_COMP_FIRSTASSIGN (arg_info) = new_assign;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPLoop
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   :
 *  remarks       : INFO_COMP_LASTASSIGN(arg_info) contains pointer to last
 *                   assign_node.
 *
 *
 */

node *
COMPLoop (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *icm_arg, *n_node, *v1, *v2, *label, *V1, *V2,
      *loop_assign, *tmp;
    node *dummy_assign = NULL;
    int found;

    DBUG_ENTER ("COMPLoop");

    /* first compile termination condition and body of loop */
    loop_assign = CURR_ASSIGN (arg_info);
    DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);
    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    /* now add some DEC_RC at begining of and after the loop */
    dummy_assign = MakeNode (N_assign);
    first_assign = dummy_assign;
    V1 = DO_USEVARS (arg_node);
    V2 = DO_DEFVARS (arg_node);
    v1 = V1;
    v2 = V2;
    if ((NULL != v2) && (NULL == v1)) {
        MAKENODE_NUM (n_node, 1);
        while (NULL != v2) {
            /*
             * we don`t know the refcount of v2 in the current context,
             * so we use DEC_RC_FREE_ND
             */
            DEC_RC_FREE_ND (EXPRS_EXPR (v2), n_node);

            v2 = EXPRS_NEXT (v2);
        }
    } else {
        if ((NULL != v2) && (NULL != v1)) {
            node *v1_tmp;
            MAKENODE_NUM (n_node, 1);
            while (NULL != v2) {
                /* looking if v2 is in (V2 \ V1) */
                v1_tmp = v1;
                found = 0;
                while ((0 == found) && (NULL != v1_tmp))
                    if (0
                        == strcmp (ID_NAME (EXPRS_EXPR (v1_tmp)),
                                   ID_NAME (EXPRS_EXPR (v2)))) {
                        found = 1;
                    } else {
                        v1_tmp = EXPRS_NEXT (v1_tmp);
                    }

                if (0 == found) {
                    DEC_RC_FREE_ND (EXPRS_EXPR (v2), n_node);
                }

                v2 = EXPRS_NEXT (v2);
            }
        }
    }

    if (N_do == NODE_TYPE (arg_node)) {
        label_nr++;
        MAKENODE_ID (label, GenName (label_nr, LABEL_NAME));
        CREATE_1_ARY_ICM (next_assign, "ND_LABEL", label);
        APPEND_ASSIGNS (first_assign, next_assign);
    }
    v1 = V1;
    if (NULL != v1) {
        /* now add some INC_RC at begining of and after the loop */
        while (NULL != v1) {
            if (1 <= (ID_REFCNT (EXPRS_EXPR (v1)) - 1)) {
                MAKENODE_NUM (n_node, ID_REFCNT (EXPRS_EXPR (v1)) - 1);
                INC_RC_ND (EXPRS_EXPR (v1), n_node);
            }

            v1 = EXPRS_NEXT (v1);
        }
    }

    /* now insert INC's and DEC's at beginning of the loop */
    if (NULL != ASSIGN_NEXT (dummy_assign)) {
        ASSIGN_NEXT (first_assign) = BLOCK_INSTR (DO_BODY (arg_node));
        BLOCK_INSTR (DO_BODY (arg_node)) = ASSIGN_NEXT (dummy_assign);
        ASSIGN_NEXT (dummy_assign) = NULL;
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
                /* looking if v1 is in (V1 \ V2) */
                v2_tmp = v2;
                found = 0;
                while ((0 == found) && (NULL != v2_tmp)) {
                    if (0
                        == strcmp (ID_NAME (EXPRS_EXPR (v1)),
                                   ID_NAME (EXPRS_EXPR (v2_tmp)))) {
                        found = 1;
                    } else {
                        v2_tmp = EXPRS_NEXT (v2_tmp);
                    }
                }

                if (0 == found) {
                    DEC_RC_FREE_ND (EXPRS_EXPR (v1), n_node);
                }

                v1 = EXPRS_NEXT (v1);
            }
        } else {
            while (NULL != v1) {
                DEC_OR_FREE_RC_ND (EXPRS_EXPR (v1), n_node);

                v1 = EXPRS_NEXT (v1);
            }
        }
    }
    /* now increase RC of arrays that are defined in the  loop and are
     * used after it.
     */
    v2 = V2;
    if (NULL != v2) {
        while (NULL != v2) {
            if (1 < ID_REFCNT (EXPRS_EXPR (v2))) {
                MAKENODE_NUM (n_node, ID_REFCNT (EXPRS_EXPR (v2)) - 1);
                INC_RC_ND (v2, n_node);
            }

            v2 = EXPRS_NEXT (v2);
        }
    }

    if (NULL != ASSIGN_NEXT (dummy_assign)) {
        /* now put ASSIGN_NEXT(dummy_assign) behind while_loop */
        if (NULL != ASSIGN_NEXT (loop_assign)) {
            ASSIGN_NEXT (first_assign) = ASSIGN_NEXT (loop_assign);
        }
        ASSIGN_NEXT (loop_assign) = ASSIGN_NEXT (dummy_assign);
    }
    FREE (dummy_assign);

    if (N_do == NODE_TYPE (arg_node)) {
        /* put N_icm 'ND_GOTO', in front of N_do node */
        CREATE_1_ARY_ICM (first_assign, "ND_GOTO", label);
        if (NULL != ASSIGN_NEXT (loop_assign)) {
            /* next assign after do-loop */
            ASSIGN_NEXT (first_assign) = ASSIGN_NEXT (loop_assign);
        }

        /* only temporary used (N_do): */
        tmp = ASSIGN_INSTR (loop_assign);
        /* N_icm (ND_GOTO) node: */
        ASSIGN_INSTR (loop_assign) = ASSIGN_INSTR (first_assign);
        ASSIGN_NEXT (loop_assign) = first_assign;
        arg_node = ASSIGN_INSTR (first_assign);
        /* put N_do node: */
        ASSIGN_INSTR (first_assign) = tmp;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPCond
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   :
 *  remarks       :
 *
 *
 */

node *
COMPCond (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *icm_arg, *n_node, *id_exprs, *dummy_assign = NULL;
    int i;

    DBUG_ENTER ("COMPCond");

    /* compile condition, then and else part */
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    /*
     * insert N_icms to correct refcounts of then and else part
     */

    dummy_assign = MakeNode (N_assign);

    if (NULL != COND_THENVARS (arg_node)) {
        id_exprs = COND_THENVARS (arg_node);
        first_assign = dummy_assign;
        do {
            MAKENODE_NUM (n_node, ID_REFCNT (EXPRS_EXPR (id_exprs)));
            DBUG_PRINT ("COMP",
                        ("%d:create DEC_RC(%s, %d)", i, ID_NAME (EXPRS_EXPR (id_exprs)),
                         ID_REFCNT (EXPRS_EXPR (id_exprs))));
            DEC_OR_FREE_RC_ND (EXPRS_EXPR (id_exprs), n_node);
            id_exprs = EXPRS_NEXT (id_exprs);
        } while (NULL != id_exprs);
        ASSIGN_NEXT (first_assign) = BLOCK_INSTR (COND_THEN (arg_node));
        BLOCK_INSTR (COND_THEN (arg_node)) = ASSIGN_NEXT (dummy_assign);
    }

    if (NULL != COND_ELSEVARS (arg_node)) {
        id_exprs = COND_ELSEVARS (arg_node);
        first_assign = dummy_assign;
        do {
            MAKENODE_NUM (n_node, ID_REFCNT (EXPRS_EXPR (id_exprs)));
            DBUG_PRINT ("COMP",
                        ("%d:create DEC_RC(%s, %d)", i, ID_NAME (EXPRS_EXPR (id_exprs)),
                         ID_REFCNT (EXPRS_EXPR (id_exprs))));
            DEC_OR_FREE_RC_ND (EXPRS_EXPR (id_exprs), n_node);
            id_exprs = EXPRS_NEXT (id_exprs);
        } while (NULL != id_exprs);
        ASSIGN_NEXT (first_assign) = BLOCK_INSTR (COND_ELSE (arg_node));
        BLOCK_INSTR (COND_ELSE (arg_node)) = ASSIGN_NEXT (dummy_assign);
    }

    FREE (dummy_assign);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPTypedef
 *  arguments     : 1) arg node
 *                  2) info node
 *  description   : transforms N_typedef to N_icm if it is a definition of an
 *                  array
 *  remarks       :
 *
 */

node *
COMPTypedef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("COMPTypedef");

    if (0 != TYPEDEF_DIM (arg_node)) {
        /*
         * Here, a new type is defined as a composite type
         * (for the time being an array type!)
         * Therefore, we have to translate the typedef-node
         * into an ICM-node "ND_TYPEDEF_ARRAY" ....
         */
        node *type1, *type2, *icm_node;

        type1 = MakeId (StringCopy (type_string[TYPEDEF_BASETYPE (arg_node)]), NULL,
                        ST_regular);

        type2 = MakeId (StringCopy (TYPEDEF_NAME (arg_node)), NULL, ST_regular);

        icm_node = MakeIcm ("ND_TYPEDEF_ARRAY",
                            MakeExprs (type1, MakeExprs (type2, NULL)), NULL);

        /*
         * Now, we free the old typedef node and implicitly set arg_node to the
         * TYPEDEF_NEXT(arg_node)!
         */
        arg_node = FreeNode (arg_node);

        if (arg_node != NULL) {
            ICM_NEXT (icm_node) = Trav (arg_node, arg_info);
        }
        arg_node = icm_node; /* for returning the new node 8-) */
    } else {
        /*
         * arg_node defines a type-synonym of a simple type and
         * therefore has not to be translated into an ICM!!
         * Hence, we simply traverse the next typedef!
         */
        if (TYPEDEF_NEXT (arg_node) != NULL) {
            TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : COMPObjdef
 *  arguments     : 1) N_objdef node
 *                  2) arg_info unused
 *  description   : The N_objdef node is replaced if the object's type
 *                  is an array.
 *  remarks       :
 *
 */

node *
COMPObjdef (node *arg_node, node *arg_info)
{
    node *icm, *icm_arg, *type_id_node, *id_node;
    int i;
    types *full_type;

    DBUG_ENTER ("COMPObjdef");

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
            ICM_NAME (icm) = "ND_KD_DECL_EXTERN_ARRAY";
        } else {
            ICM_NAME (icm) = "ND_KS_DECL_GLOBAL_ARRAY";

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

/*
 *
 *  functionname  : COMPWith
 *  arguments     : 1) N_with node
 *                  2) info node
 *  description   :
 *  remarks       : INFO_COMP_LASTIDS(arg_info) contains name of assigned variable
 *                  INFO_COMP_LASTASSIGN(arg_info) contains pointer to last but one
 *                    assign_node
 *                  INFO_COMP_LASTLET(arg_info) contains pointer to previous N_let
 *                  INFO_COMP_WITHBEGIN(arg_info) will be set to  pointer to N_icm
 *                  of with_loop begin
 *
 */

node *
COMPWith (node *arg_node, node *arg_info)
{
    node *old_info2, *first_assign, *next_assign, *n_node, *inc_rc, *icm_arg, *from, *to,
      *type_id_node, *arg, *res, *res_ref, *res_dim_node, *index, *indexlen,
      *old_arg_node, *last_assign, *fun_node, *res_size_node;
    int res_dim, is_foldprf, res_size;
    simpletype s_type;

    DBUG_ENTER ("COMPWith");

    /* store INFO_COMP_WITHBEGIN(arg_info) */
    old_info2 = INFO_COMP_WITHBEGIN (arg_info);

    /* store res as N_id */
    MAKENODE_ID_REUSE_IDS (res, INFO_COMP_LASTIDS (arg_info));
    ID_REFCNT (res) = IDS_REFCNT (INFO_COMP_LASTIDS (arg_info));

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, IDS_REFCNT (INFO_COMP_LASTIDS (arg_info)));

    /* compute basic_type of result */
    GET_BASIC_SIMPLETYPE (s_type,
                          VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* compute dimension of res */
    GET_DIM (res_dim, VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    MAKENODE_NUM (res_dim_node, res_dim);

    /* compute size of res */
    GET_LENGTH (res_size, VARDEC_TYPE (IDS_VARDEC (INFO_COMP_LASTIDS (arg_info))));
    MAKENODE_NUM (res_size_node, res_size);

    /* store index_vector as N_id */
    MAKENODE_ID_REUSE_IDS (index, LET_IDS (WITH_GEN (arg_node)));

    /* store length of index-vector */
    MAKENODE_NUM (indexlen, SHPSEG_SHAPE (TYPES_SHPSEG (VARDEC_TYPE (
                                            GEN_VARDEC (WITH_GEN (arg_node)))),
                                          0));

    /* set 'from' to left range of index-vector */
    from = arg_node->node[0]->node[0];

    /* set 'to' to right range of index-vector */
    to = arg_node->node[0]->node[1];
    if (res_dim > 0) {
        /* first create N_icm to allocate memeory */
        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", type_id_node, res);
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
        BIN_ICM_REUSE (INFO_COMP_LASTLET (arg_info), "ND_ALLOC_ARRAY", type_id_node,
                       index);
        MAKE_NEXT_ICM_ARG (icm_arg, n_node);
        SET_VARS_FOR_MORE_ICMS;
    }

    if (N_modarray == old_arg_node->node[1]->nodetype) {
        arg = old_arg_node->node[1]->node[0];
        DBUG_ASSERT ((N_id == NODE_TYPE (arg)), "wrong nodetype != N_id");
        CREATE_7_ARY_ICM (next_assign, "ND_BEGIN_MODARRAY", res, res_dim_node, arg, from,
                          to, index, indexlen);
        /* store pointer to N_icm ND_BEGIN.. in INFO_COMP_WITHBEGIN(arg_info) */
        INFO_COMP_WITHBEGIN (arg_info) = next_assign->node[0];
        /* store pointer to variables that have to be increased in
         * in INFO_COMP_WITHBEGIN(arg_info)->node[3] (it will be used in COMPReturn)
         */
        INFO_COMP_WITHBEGIN (arg_info)->node[3] = WITH_USEDVARS (old_arg_node);

        APPEND_ASSIGNS (first_assign, next_assign);
    } else {
        if (N_genarray == old_arg_node->node[1]->nodetype) {
            CREATE_6_ARY_ICM (next_assign, "ND_BEGIN_GENARRAY", res, res_dim_node, from,
                              to, index, indexlen);
            /* store pointer to N_icm ND_BEGIN.. in INFO_COMP_WITHBEGIN(arg_info) */
            INFO_COMP_WITHBEGIN (arg_info) = next_assign->node[0];
            /* store pointer to variables that have to be increased in
             * in INFO_COMP_WITHBEGIN(arg_info)->node[3] ( it will be used in COMPReturn )
             */
            INFO_COMP_WITHBEGIN (arg_info)->node[3] = WITH_USEDVARS (old_arg_node);
            APPEND_ASSIGNS (first_assign, next_assign);
        } else {
            if (N_foldprf == NODE_TYPE (old_arg_node->node[1])) {
                fun_node = MakeNode (N_prf);
                fun_node->info.prf = old_arg_node->node[1]->info.prf;
                is_foldprf = 1;
            } else {
                if (N_foldfun == NODE_TYPE (old_arg_node->node[1])) {
                    fun_node = MakeNode (N_ap);
                    AP_NAME (fun_node) = AP_NAME (old_arg_node->node[1]);
                    AP_MOD (fun_node) = AP_MOD (old_arg_node->node[1]);
                    fun_node->node[1] = old_arg_node->node[1]->node[2];
                    DBUG_ASSERT (N_fundef == fun_node->node[1]->nodetype,
                                 "wrong nodetype != N_fundef ");
                    is_foldprf = 0;
                } else {
                    DBUG_ASSERT (0, "wrong nodetype != N_foldfun, ... ");
                }
            }
        }
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
        if (N_array == neutral_node->nodetype) {
            COUNT_ELEMS (length, neutral_node->node[0]);
            MAKENODE_NUM (n_neutral, length);
            CREATE_7_ARY_ICM (next_assign,
                              (is_foldprf) ? "ND_BEGIN_FOLDPRF" : "ND_BEGIN_FOLDFUN", res,
                              res_size_node, from, to, index, indexlen, n_neutral);
            icm_arg->node[1] = neutral_node->node[0];
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
         * removed in COMPReturn.
         */
        res_ref->info.cint -= 1;
        MAKE_NEXT_ICM_ARG (icm_arg, res_ref);

        /* store pointer to N_icm ND_BEGIN.. in INFO_COMP_WITHBEGIN(arg_info) */
        INFO_COMP_WITHBEGIN (arg_info) = next_assign->node[0];

        /* store pointer to variables that have to be increased in
         * in INFO_COMP_WITHBEGIN(arg_info)->node[3] ( it will be used in COMPReturn )
         */
        INFO_COMP_WITHBEGIN (arg_info)->node[3] = WITH_USEDVARS (old_arg_node);

        /* Store  N_prf or N_ap  in node[2] of current N_icm.
         * It will be used in COMPReturn and than eliminated.
         */
        INFO_COMP_WITHBEGIN (arg_info)->node[2] = fun_node;
        APPEND_ASSIGNS (first_assign, next_assign);
    }

    /* now add some INC_RC's */
    inc_rc = WITH_USEDVARS (old_arg_node)->node[0];
    while (NULL != inc_rc) {
        MAKENODE_NUM (n_node, ID_REFCNT (inc_rc));
        INC_RC_ND (inc_rc, n_node);
        inc_rc = inc_rc->node[0];
    }
    /* update arg_info, after inserting new N_assign nodes */
    if ((N_foldprf == old_arg_node->node[1]->nodetype)
        || (N_foldfun == old_arg_node->node[1]->nodetype))
        next_assign->node[1] = old_arg_node->node[1]->node[0]->node[0];
    else {
        next_assign->node[1] = old_arg_node->node[1]->node[1]->node[0];
    }
    INFO_COMP_LASTASSIGN (arg_info) = next_assign;
    if ((N_foldprf == old_arg_node->node[1]->nodetype)
        || (N_foldfun == old_arg_node->node[1]->nodetype)) {
        next_assign = Trav (old_arg_node->node[1]->node[0]->node[0], arg_info);
    } else {
        next_assign = Trav (old_arg_node->node[1]->node[1]->node[0], arg_info);
    }
    INFO_COMP_WITHBEGIN (arg_info) = old_info2;
    APPEND_ASSIGNS (first_assign, next_assign);

    /* set first_assign to the last N_assign node, that is generated by Trav() */
    first_assign = CURR_ASSIGN (arg_info);
    while (first_assign->node[1] != NULL) {
        first_assign = first_assign->node[1];
    }

    INSERT_ASSIGN;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPSpmd( node *arg_node, node *arg_info)
 *
 * description:
 *   compiles a N_spmd node.
 *
 ******************************************************************************/

node *
COMPSpmd (node *arg_node, node *arg_info)
{
    node *fundef, *vardec, *icm_arg, *last_icm_arg, *icm_args = NULL;
    char *tag;
    ids *my_ids;
    int num_args;

    DBUG_ENTER ("COMPSpmd");

    SPMD_REGION (arg_node) = Trav (SPMD_REGION (arg_node), arg_info);

    /****************************************************************************
     * build ICM for SPMD-region
     */

    fundef = INFO_COMP_FUNDEF (arg_info);
    num_args = 0;

    /*
     * in-params
     */

    FOREACH_VARDEC_AND_ARG (fundef, vardec,
                            if (DFMTestMaskEntry (SPMD_IN (arg_node),
                                                  VARDEC_OR_ARG_NAME (vardec))) {
                                if (VARDEC_OR_ARG_REFCNT (vardec) >= 0) {
                                    tag = "in_rc";
                                } else {
                                    tag = "in";
                                }
                                icm_arg
                                  = MakeExprs (MakeId (StringCopy (tag), NULL,
                                                       ST_regular),
                                               MakeExprs (MakeId (StringCopy (
                                                                    VARDEC_OR_ARG_NAME (
                                                                      vardec)),
                                                                  NULL, ST_regular),
                                                          NULL));

                                if (icm_args == NULL) {
                                    icm_args = icm_arg;
                                } else {
                                    EXPRS_NEXT (EXPRS_NEXT (last_icm_arg)) = icm_arg;
                                }
                                last_icm_arg = icm_arg;
                                num_args++;
                            }) /* FOREACH_VARDEC_AND_ARG */

    /*
     * inout-params
     */

    my_ids = SPMD_INOUT_IDS (arg_node);
    while (my_ids != NULL) {
        if (DFMTestMaskEntry (SPMD_INOUT (arg_node), IDS_NAME (my_ids))) {
            tag = (char *)Malloc (10 * sizeof (char));
            sprintf (tag, "inout%d", IDS_REFCNT (my_ids));
            icm_arg = MakeExprs (MakeId (tag, NULL, ST_regular),
                                 MakeExprs (MakeId (StringCopy (IDS_NAME (my_ids)), NULL,
                                                    ST_regular),
                                            NULL));

            if (icm_args == NULL) {
                icm_args = icm_arg;
            } else {
                EXPRS_NEXT (EXPRS_NEXT (last_icm_arg)) = icm_arg;
            }
            last_icm_arg = icm_arg;
            num_args++;
        }
        my_ids = IDS_NEXT (my_ids);
    }

    /*
     * out-params
     */

    FOREACH_VARDEC_AND_ARG (fundef, vardec,
                            if (DFMTestMaskEntry (SPMD_OUT (arg_node),
                                                  VARDEC_OR_ARG_NAME (vardec))) {
                                if (VARDEC_OR_ARG_REFCNT (vardec) >= 0) {
                                    tag = "out_rc";
                                } else {
                                    tag = "out";
                                }
                                icm_arg
                                  = MakeExprs (MakeId (StringCopy (tag), NULL,
                                                       ST_regular),
                                               MakeExprs (MakeId (StringCopy (
                                                                    VARDEC_OR_ARG_NAME (
                                                                      vardec)),
                                                                  NULL, ST_regular),
                                                          NULL));

                                if (icm_args == NULL) {
                                    icm_args = icm_arg;
                                } else {
                                    EXPRS_NEXT (EXPRS_NEXT (last_icm_arg)) = icm_arg;
                                }
                                last_icm_arg = icm_arg;
                                num_args++;
                            }) /* FOREACH_VARDEC_AND_ARG */

    icm_args = MakeExprs (MakeNum (num_args), icm_args);
    icm_args = MakeExprs (MakeId (StringCopy (SPMD_FUNNAME (arg_node)), NULL, ST_regular),
                          icm_args);

#if 0
  SPMD_ICM( arg_node) = MakeIcm( "MT_SPMD_BLOCK", icm_args, NULL);
#endif

    /*
     * build ICM for SPMD-region
     ****************************************************************************/

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPSync( node *arg_node, node *arg_info)
 *
 * description:
 *   compiles a N_sync node.
 *
 * remarks:
 *   - INFO_COMP_MT decides weather the memory managment for with-loops is done
 *     in the N_sync- or the N_with2- code!!
 *
 ******************************************************************************/

node *
COMPSync (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("COMPSync");

#if 0
  /*
   * compile contents of sync-region
   */

  INFO_COMP_MT( arg_info) = 0;
  SYNC_REGION( arg_node) = Trav( SYNC_REGION( arg_node), arg_info);

  /*
   * build the arguments for the ICMs
   */

  icm_args1 = icm_args2 = NULL;
  num_folds = 0;
  sync_ids = SYNC_OUT( arg_node);
  while (sync_ids != NULL) {
    GET_BASIC_SIMPLETYPE( s_type, IDS_TYPE( sync_ids));
    icm_args = MakeExprs( MakeId( StringCopy( type_string[ s_type]),
                                  NULL,
                                  ST_regular),
                          MakeExprs( MakeId2( DupOneIds( sync_ids, NULL)),
                                     NULL));

    icm_args1 = AppendExpr( icm_args1, icm_args);
    icm_args2 = AppendExpr( icm_args2,
                            MakeExprs( DupTree( icm_args, NULL),
                              MakeExprs( MakeId( StringCopy( "?tmp_var?"),
                                                 NULL,
                                                 ST_regular),
                                 MakeExprs( MakeId( StringCopy( "?fold_op?"),
                                                    NULL,
                                                    ST_regular),
                                            NULL))));

    num_folds++;
    sync_ids = IDS_NEXT( sync_ids);
  }

  /*
   * is this sync-region *not* the first one of the current SPMD-region?
   *  -> insert the memory-managment (malloc).
   *  -> insert ICM (MT_CONTINUE),
   */

  if (SYNC_FIRST( arg_node) == 0) {

    /*
     * insert a ND_ALLOC_ARRAY for every RC-object in 'SYNC_INOUT'
     */
    assigns = AppendAssign( assigns,
                            MakeAllocArrayICMs( DupIds( SYNC_INOUT( arg_node), NULL)));

    assigns = AppendAssign( assigns,
                            MakeAssign( MakeIcm( "MT_CONTINUE",
                                                 MakeExprs( MakeNum( num_folds),
                                                            icm_args1),
                                                 NULL),
                                        NULL));

  }
  else {

    if (icm_args1 != NULL) {
      icm_args1 = FreeTree( icm_args1);
    }

  }

  /*
   * insert contents of sync-region-block
   */

  assigns = AppendAssign( assigns,
                          BLOCK_INSTR( SYNC_REGION( arg_node)));
  BLOCK_INSTR( SYNC_REGION( arg_node)) = NULL;

  /*
   * insert ICM (MT_SYNC_...)
   */

  if (SYNC_INOUT( arg_node) == NULL) {
    DBUG_ASSERT( (SYNC_OUT( arg_node) != NULL), "no target found");
    if (IDS_NEXT( SYNC_OUT( arg_node)) == NULL) {
      icm_name = "MT_SYNC_ONEFOLD";
    }
    else {
      icm_name = "MT_SYNC_FOLD";
    }
  }
  else {
    if (SYNC_OUT( arg_node) == NULL) {
      icm_name = "MT_SYNC_NONFOLD";
    }
    else {
      if (IDS_NEXT( SYNC_OUT( arg_node)) == NULL) {
        icm_name = "MT_SYNC_ONEFOLD_NONFOLD";
      }
      else {
        icm_name = "MT_SYNC_FOLD_NONFOLD";
      }
    }
  }

  assigns = AppendAssign( assigns,
                          MakeAssign( MakeIcm( icm_name,
                                               MakeExprs( MakeNum( num_folds),
                                                          icm_args2),
                                               NULL),
                                      NULL));

  /*
   * insert the memory-managment (free),
   */


  /*
   * remove remain of N_sync node
   */

  arg_node = FreeTree( arg_node);

  DBUG_RETURN( assigns);
#else
    DBUG_RETURN (arg_node);
#endif
}

node *wl_icm_args = NULL;
node *wl_withid = NULL;

/******************************************************************************
 *
 * function:
 *   node *COMPNwith2( node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of a N_with2 node.
 *
 * remarks:
 *   - 'wl_withid' points always to the N_withid-node.
 *   - 'wl_icm_args' points to the args-list of the ICMs WL_BEGIN, WL_END.
 *     This is needed to reuse this args for WL_ASSIGN.
 *   - INFO_COMP_MT decides weather the memory managment for with-loops is done
 *     in the N_sync- or the N_with2- code!!
 *
 ******************************************************************************/

node *
COMPNwith2 (node *arg_node, node *arg_info)
{
    node *icm_args, *icm_arg, *last_icm_arg, *vardec;
    ids *wl_ids, *withid_ids, *in_ids, *new_in_ids, *last_in_ids;
    int num_args;
    node *assigns = NULL;

    DBUG_ENTER ("COMPNwith2");

    /*
     * we must store the with-loop ids *before* compiling the codes
     *  because INFO_COMP_LASTIDS is possibly updated afterwards !!!
     */

    wl_ids = INFO_COMP_LASTIDS (arg_info);
    wl_withid = NWITH2_WITHID (arg_node);

    /*
     * if with-loop is compiled to sequential code,
     *  insert ICMs for memory management (ND_ALLOC_ARRAY)
     */

    if (INFO_COMP_MT (arg_info) == 0) {
        assigns = MakeAllocArrayICMs (DupIds (wl_ids, NULL));
    }

    /*
     * insert ICMs to allocate memory for index-vector
     */

    assigns
      = AppendAssign (assigns, MakeAllocArrayICMs (
                                 DupIds (NWITHID_VEC (NWITH2_WITHID (arg_node)), NULL)));

    /*
     * compile all code blocks
     */

    NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);

    /*
     * insert ICM 'WL_BEGIN'
     */

    num_args = 0;
    icm_args = NULL;
    withid_ids = NWITHID_IDS (wl_withid);
    while (withid_ids != NULL) {
        icm_arg = MakeExprs (MakeId2 (DupOneIds (withid_ids, NULL)), NULL);

        if (icm_args == NULL) {
            icm_args = icm_arg;
        } else {
            EXPRS_NEXT (last_icm_arg) = icm_arg;
        }
        last_icm_arg = icm_arg;
        num_args++;
        withid_ids = IDS_NEXT (withid_ids);
    }
    icm_args = MakeExprs (MakeNum (num_args), icm_args);

    icm_args = MakeExprs (MakeId2 (DupOneIds (NWITHID_VEC (wl_withid), NULL)), icm_args);

    icm_args = MakeExprs (MakeId2 (DupOneIds (wl_ids, NULL)), icm_args);

    /*
     * store the ICM args for later use (WL_ASSIGN)
     */
    wl_icm_args = icm_args;

    assigns
      = AppendAssign (assigns, MakeAssign (MakeIcm ("WL_BEGIN", icm_args, NULL), NULL));

    /*
     * compile the with-segments
     *  -> we get an assignment-chain
     */

    assigns = AppendAssign (assigns, Trav (NWITH2_SEGS (arg_node), arg_info));

    /*
     * insert ICM 'WL_END'
     */

    assigns
      = AppendAssign (assigns, MakeAssign (MakeIcm ("WL_END", icm_args, NULL), NULL));

    /*
     * if with-loop is compiled to sequential code,
     *  insert ICMs for memory management (ND_DEC_RC_FREE).
     */

    if ((INFO_COMP_MT (arg_info) == 0) && (NWITH2_RC_IDS (arg_node) != NULL)) {
        assigns
          = AppendAssign (assigns,
                          MakeDecRcFreeICMs (DupIds (NWITH2_RC_IDS (arg_node), NULL)));
    }

    /*
     * insert ICM 'DEC_RC_FREE' for index-vector.
     */

    assigns
      = AppendAssign (assigns, MakeDecRcFreeICMs (
                                 DupIds (NWITHID_VEC (NWITH2_WITHID (arg_node)), NULL)));

    /*
     * old with-loop representation is useless now!
     */

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPNcode( node *arg_node, node *arg_info)
 *
 * description:
 *   compiles a Ncode node.
 *
 ******************************************************************************/

node *
COMPNcode (node *arg_node, node *arg_info)
{
    node *icm_assigns;

    DBUG_ENTER ("COMPNcode");

    /*
     * build a 'ND_INC_RC'-ICM for each ids in 'NCODE_RC_IDS( arg_node)'.
     */
    icm_assigns = MakeIncRcICMs (NCODE_RC_IDS (arg_node));
    NCODE_RC_IDS (arg_node) = NULL;

    /*
     * insert these ICMs as first statement into the code-block
     */
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
        BLOCK_INSTR (NCODE_CBLOCK (arg_node))
          = AppendAssign (icm_assigns, BLOCK_INSTR (NCODE_CBLOCK (arg_node)));
    }
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLseg(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLseg-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wl_withid' points always to the N_withid-node.
 *
 ******************************************************************************/

node *
COMPWLseg (node *arg_node, node *arg_info)
{
    node *assigns;

    DBUG_ENTER ("COMPWLseg");

    /*
     * compile the contents of the segment
     *  -> we get an assignment-chain
     */
    assigns = Trav (WLSEG_CONTENTS (arg_node), arg_info);

    if (WLSEG_NEXT (arg_node) != NULL) {
        /*
         * append compilat (assignment-chain) of next segment to 'assigns'
         */
        assigns = AppendAssign (assigns, Trav (WLSEG_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLblock(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLblock-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wl_withid' points always to the N_withid-node.
 *
 ******************************************************************************/

node *
COMPWLblock (node *arg_node, node *arg_info)
{
    node *icm_args;
    ids *ids_vector, *ids_scalar;
    char *icm_name;
    node *assigns = NULL;

    DBUG_ENTER ("COMPWLblock");

    /* build argument list for ICMs */
    ids_vector = NWITHID_VEC (wl_withid);
    ids_scalar = GetIndexIds (NWITHID_IDS (wl_withid), WLBLOCK_DIM (arg_node));
    icm_args = MakeExprs (
      MakeNum (WLBLOCK_LEVEL (arg_node)),
      MakeExprs (
        MakeNum (WLBLOCK_LEVEL (arg_node) + 1),
        MakeExprs (MakeNum (WLBLOCK_DIM (arg_node)),
                   MakeExprs (MakeId2 (DupOneIds (ids_vector, NULL)),
                              MakeExprs (MakeId2 (DupOneIds (ids_scalar, NULL)),
                                         MakeExprs (MakeNum (WLBLOCK_BOUND1 (arg_node)),
                                                    MakeExprs (MakeNum (WLBLOCK_BOUND2 (
                                                                 arg_node)),
                                                               MakeExprs (MakeNum (
                                                                            WLBLOCK_STEP (
                                                                              arg_node)),
                                                                          NULL))))))));

    /* compile contents */
    if (WLBLOCK_CONTENTS (arg_node) != NULL) {
        DBUG_ASSERT ((WLBLOCK_NEXTDIM (arg_node) == NULL),
                     "contents and nextdim used simultaneous");
        assigns = Trav (WLBLOCK_CONTENTS (arg_node), arg_info);
    }

    /* compile nextdim */
    if (WLBLOCK_NEXTDIM (arg_node) != NULL) {
        DBUG_ASSERT ((WLBLOCK_CONTENTS (arg_node) == NULL),
                     "contents and nextdim used simultaneous");
        assigns = Trav (WLBLOCK_NEXTDIM (arg_node), arg_info);
    }

    /* insert ICMs for current node */
    DBUG_ASSERT ((assigns != NULL), "contents and nextdim are empty");
    if (WLBLOCK_LEVEL (arg_node) == 0) {
        icm_name = "WL_BLOCK_LOOP0_BEGIN";
    } else {
        icm_name = "WL_BLOCK_LOOP_BEGIN";
    }
    assigns = MakeAssign (MakeIcm (icm_name, icm_args, NULL), assigns);
    assigns = AppendAssign (assigns, MakeAssign (MakeIcm ("WL_BLOCK_LOOP_END",
                                                          DupTree (icm_args, NULL), NULL),
                                                 NULL));

    /* compile successor */
    if (WLBLOCK_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLBLOCK_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLublock(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLublock-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wl_withid' points always to the N_withid-node.
 *
 ******************************************************************************/

node *
COMPWLublock (node *arg_node, node *arg_info)
{
    node *icm_args;
    ids *ids_vector, *ids_scalar;
    char *icm_name;
    node *assigns = NULL;

    DBUG_ENTER ("COMPWLublock");

    /* build argument list for ICMs */
    ids_vector = NWITHID_VEC (wl_withid);
    ids_scalar = GetIndexIds (NWITHID_IDS (wl_withid), WLUBLOCK_DIM (arg_node));
    icm_args = MakeExprs (
      MakeNum (WLUBLOCK_LEVEL (arg_node)),
      MakeExprs (
        MakeNum (WLUBLOCK_LEVEL (arg_node) + 1),
        MakeExprs (MakeNum (WLUBLOCK_DIM (arg_node)),
                   MakeExprs (MakeId2 (DupOneIds (ids_vector, NULL)),
                              MakeExprs (MakeId2 (DupOneIds (ids_scalar, NULL)),
                                         MakeExprs (MakeNum (WLUBLOCK_BOUND1 (arg_node)),
                                                    MakeExprs (MakeNum (WLUBLOCK_BOUND2 (
                                                                 arg_node)),
                                                               MakeExprs (MakeNum (
                                                                            WLUBLOCK_STEP (
                                                                              arg_node)),
                                                                          NULL))))))));

    /* compile contents */
    if (WLUBLOCK_CONTENTS (arg_node) != NULL) {
        DBUG_ASSERT ((WLUBLOCK_NEXTDIM (arg_node) == NULL),
                     "contents and nextdim used simultaneous");
        assigns = Trav (WLUBLOCK_CONTENTS (arg_node), arg_info);
    }

    /* compile nextdim */
    if (WLUBLOCK_NEXTDIM (arg_node) != NULL) {
        DBUG_ASSERT ((WLUBLOCK_CONTENTS (arg_node) == NULL),
                     "contents and nextdim used simultaneous");
        assigns = Trav (WLUBLOCK_NEXTDIM (arg_node), arg_info);
    }

    /* insert ICMs for current node */
    DBUG_ASSERT ((assigns != NULL), "contents and nextdim are empty");
    if (WLUBLOCK_LEVEL (arg_node) == 0) {
        icm_name = "WL_UBLOCK_LOOP0_BEGIN";
    } else {
        icm_name = "WL_UBLOCK_LOOP_BEGIN";
    }
    assigns = MakeAssign (MakeIcm (icm_name, icm_args, NULL), assigns);
    assigns = AppendAssign (assigns, MakeAssign (MakeIcm ("WL_UBLOCK_LOOP_END",
                                                          DupTree (icm_args, NULL), NULL),
                                                 NULL));

    /* compile successor */
    if (WLUBLOCK_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLUBLOCK_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLstride(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLstride-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wl_withid' points always to the N_withid-node.
 *
 ******************************************************************************/

node *
COMPWLstride (node *arg_node, node *arg_info)
{
    node *icm_args;
    ids *ids_vector, *ids_scalar;
    char *icm_name;
    node *assigns;

    DBUG_ENTER ("COMPWLstride");

    /* build argument list for ICMs */
    ids_vector = NWITHID_VEC (wl_withid);
    ids_scalar = GetIndexIds (NWITHID_IDS (wl_withid), WLSTRIDE_DIM (arg_node));
    icm_args = MakeExprs (
      MakeNum (WLSTRIDE_LEVEL (arg_node)),
      MakeExprs (
        MakeNum (WLSTRIDE_LEVEL (arg_node) + 1),
        MakeExprs (MakeNum (WLSTRIDE_DIM (arg_node)),
                   MakeExprs (MakeId2 (DupOneIds (ids_vector, NULL)),
                              MakeExprs (MakeId2 (DupOneIds (ids_scalar, NULL)),
                                         MakeExprs (MakeNum (WLSTRIDE_BOUND1 (arg_node)),
                                                    MakeExprs (MakeNum (WLSTRIDE_BOUND2 (
                                                                 arg_node)),
                                                               MakeExprs (MakeNum (
                                                                            WLSTRIDE_STEP (
                                                                              arg_node)),
                                                                          NULL))))))));

    /* compile contents */
    assigns = Trav (WLSTRIDE_CONTENTS (arg_node), arg_info);

    /* insert ICMs for current node */
    if (WLSTRIDE_LEVEL (arg_node) == 0) {
        icm_name = "WL_STRIDE_LOOP0_BEGIN";
    } else {
        icm_name = "WL_STRIDE_LOOP_BEGIN";
    }
    assigns = MakeAssign (MakeIcm (icm_name, icm_args, NULL), assigns);
    assigns = AppendAssign (assigns, MakeAssign (MakeIcm ("WL_STRIDE_LOOP_END",
                                                          DupTree (icm_args, NULL), NULL),
                                                 NULL));

    /* compile successor */
    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLSTRIDE_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLgrid(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLgrid-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *
 * remarks:
 *   - 'wl_withid' points always to the N_withid-node.
 *
 ******************************************************************************/

node *
COMPWLgrid (node *arg_node, node *arg_info)
{
    node *icm_args, *icm_args2;
    ids *ids_vector, *ids_scalar;
    node *assigns = NULL;

    DBUG_ENTER ("COMPWLgrid");

    /* build argument list for ICMs */
    ids_vector = NWITHID_VEC (wl_withid);
    ids_scalar = GetIndexIds (NWITHID_IDS (wl_withid), WLGRID_DIM (arg_node));
    icm_args = MakeExprs (
      MakeNum (WLGRID_LEVEL (arg_node)),
      MakeExprs (MakeNum (WLGRID_LEVEL (arg_node) + 1),
                 MakeExprs (MakeNum (WLGRID_DIM (arg_node)),
                            MakeExprs (MakeId2 (DupOneIds (ids_vector, NULL)),
                                       MakeExprs (MakeId2 (DupOneIds (ids_scalar, NULL)),
                                                  MakeExprs (MakeNum (
                                                               WLGRID_BOUND1 (arg_node)),
                                                             MakeExprs (MakeNum (
                                                                          WLGRID_BOUND2 (
                                                                            arg_node)),
                                                                        NULL)))))));

    /* compile code */
    if (WLGRID_CODE (arg_node) != NULL) {
        DBUG_ASSERT ((WLGRID_NEXTDIM (arg_node) == NULL),
                     "code and nextdim used simultaneous");

        if (NCODE_CBLOCK (WLGRID_CODE (arg_node)) != NULL) {
            assigns = DupTree (BLOCK_INSTR (NCODE_CBLOCK (WLGRID_CODE (arg_node))), NULL);
        }

        DBUG_ASSERT ((NCODE_CEXPR (WLGRID_CODE (arg_node)) != NULL),
                     "no code expr found");
        icm_args2 = MakeExprs (DupTree (NCODE_CEXPR (WLGRID_CODE (arg_node)), NULL),
                               DupTree (wl_icm_args, NULL));

        assigns
          = AppendAssign (assigns,
                          MakeAssign (MakeIcm ("WL_ASSIGN", icm_args2, NULL), NULL));
    }

    /* compile nextdim */
    if (WLGRID_NEXTDIM (arg_node) != NULL) {
        DBUG_ASSERT ((WLGRID_CODE (arg_node) == NULL),
                     "code and nextdim used simultaneous");
        assigns = Trav (WLGRID_NEXTDIM (arg_node), arg_info);
    }

    /* insert ICMs for current node */
    DBUG_ASSERT ((assigns != NULL), "code and nextdim are empty");
    if (IDS_REFCNT (NWITHID_VEC (wl_withid)) > 0) {
        /*
         * if the index-vector is needed somewhere in the code-blocks,
         *  we must add the ICM 'WL_GRID_SET_IDX'.
         */
        assigns = MakeAssign (MakeIcm ("WL_GRID_SET_IDX", icm_args, NULL), assigns);
    }
    assigns = MakeAssign (MakeIcm ("WL_GRID_LOOP_BEGIN", icm_args, NULL), assigns);
    assigns = AppendAssign (assigns, MakeAssign (MakeIcm ("WL_GRID_LOOP_END",
                                                          DupTree (icm_args, NULL), NULL),
                                                 NULL));

    /* compile successor */
    if (WLGRID_NEXT (arg_node) != NULL) {
        assigns = AppendAssign (assigns, Trav (WLGRID_NEXT (arg_node), arg_info));
    }

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLstriVar(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLstriVar-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *   remarks:
 *     'wl_withid' points always to the N_withid-node.
 *
 ******************************************************************************/

node *
COMPWLstriVar (node *arg_node, node *arg_info)
{
    node *assigns;

    DBUG_ENTER ("COMPWLstriVar");

    assigns = NULL;

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * function:
 *   node *COMPWLgridVar(node *arg_node, node *arg_info)
 *
 * description:
 *   compilation of an N_WLgridVar-node:
 *     returns an N_assign-chain with ICMs and leaves 'arg_node' untouched!!
 *     (the whole with-loop-tree should be freed by 'COMPNwith2' only!!)
 *   remarks:
 *     'wl_withid' points always to the N_withid-node.
 *
 ******************************************************************************/

node *
COMPWLgridVar (node *arg_node, node *arg_info)
{
    node *assigns;

    DBUG_ENTER ("COMPWLgridVar");

    assigns = NULL;

    DBUG_RETURN (assigns);
}
