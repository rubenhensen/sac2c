/*
 *
 * $Log$
 * Revision 1.71  1999/01/15 17:00:19  sbs
 * freemask_tab added
 *
 * Revision 1.70  1999/01/15 15:14:32  cg
 * added tsi_tab for new compiler module tile size inference.
 *
 * Revision 1.69  1999/01/07 14:02:33  sbs
 * new tab opt_tab inserted and old "opt_tab" renamed to genmask_tab!
 *
 * Revision 1.68  1998/10/29 16:56:31  cg
 * Implementation of Trav() slightly modified in order to
 * simplify debugging.
 *
 * Revision 1.67  1998/06/18 13:42:11  cg
 * added traversal function tables conc_tab and sched_tab
 *
 * Revision 1.66  1998/06/07 18:36:36  dkr
 * added new fun_tab (reuse_tab)
 *
 * Revision 1.65  1998/05/11 09:43:44  cg
 * added syntax tree traversal for generating startup code.
 *
 * Revision 1.64  1998/04/29 19:53:22  dkr
 * added funptr tabs
 *
 * Revision 1.63  1998/04/28 15:43:52  srs
 * added tcwl_tab
 *
 * Revision 1.62  1998/04/23 18:58:33  dkr
 * added tabs
 * changed usage of NIF
 *
 * Revision 1.61  1998/04/17 17:28:29  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.60  1998/04/03 11:27:55  dkr
 * concregs renamed to concregions
 *
 * Revision 1.59  1998/04/02 16:11:29  dkr
 * added new traverse tabular concregs_tab
 * changed signature for NIF
 *
 * Revision 1.58  1998/03/22 18:06:33  srs
 * added new tab wlt_tab and included new WL-files WLT.h, WLI.h and WLF.h
 *
 * Revision 1.57  1998/03/06 13:20:40  srs
 * added wli_tab (gather information for WLF)
 *
 * Revision 1.56  1998/02/06 16:51:08  srs
 * included WithloopFolding.h
 *
 * Revision 1.55  1998/02/05 17:07:42  srs
 * removed wr_tab and changed fusion_tab into wlf_tab.
 * wr(work reduction) and fusion are unused optimizations.
 *
 * Revision 1.54  1997/12/02 18:42:56  srs
 * removed NEWTREE
 *
 * Revision 1.52  1997/11/19 19:40:05  dkr
 * added o2nWith_tab
 *
 * Revision 1.51  1997/11/10 23:37:06  dkr
 * removed a bug with NEWTREE
 *
 * Revision 1.50  1997/11/10 19:28:16  dkr
 * removed a bug with NEWTREE
 *
 * Revision 1.49  1997/11/05 16:29:46  dkr
 * moved nnode[] from tree_compound.[ch] to traverse.[ch]
 *
 * Revision 1.48  1997/11/05 09:55:19  dkr
 * fixed a bug with nnode[]
 *
 * Revision 1.47  1997/11/05 09:35:00  dkr
 * usage of NIF-macro has changed
 * uses the new array nnode[]: TravSons traverses the sons 0 .. nnode[node->type]
 *
 * Revision 1.46  1997/10/30 12:17:32  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.45  1997/03/19 13:37:55  cg
 * The entire link_tab is removed
 *
 * Revision 1.44  1996/08/09  16:44:12  asi
 * dead function removal added
 *
 * Revision 1.43  1996/01/22  08:15:23  asi
 * includes CSE.h now
 *
 * Revision 1.42  1996/01/17  16:49:21  asi
 * added common subexpression elimination
 *
 * Revision 1.41  1996/01/02  15:49:35  cg
 * added link_tab, macro NIF extended.
 *
 * Revision 1.40  1995/12/29  10:27:33  cg
 * added readsib_tab
 *
 * Revision 1.39  1995/12/21  13:23:18  asi
 * changed dead_tab to dcr_tab and added active_tab
 *
 * Revision 1.38  1995/12/07  14:15:09  cg
 * removed DummyFun2
 * renamed DummyFun to TravSons
 *
 * Revision 1.37  1995/12/01  17:08:14  cg
 * new fun table 'precomp_tab'
 *
 * Revision 1.36  1995/11/16  19:38:34  cg
 * added new tab: rmvoid_tab.
 * NIF macro extended by 4 new parameters.
 *
 * Revision 1.35  1995/11/06  09:21:59  cg
 * added unique_tab
 *
 * Revision 1.34  1995/11/01  08:02:02  cg
 * added obj_tab and include of objects.h
 *
 * Revision 1.33  1995/10/22  17:26:20  cg
 * added checkdec_tab and writedec_tab
 *  renamed sib_tab to writesib_tab
 *
 * Revision 1.32  1995/10/20  09:25:35  cg
 * added 'analy_tab`.
 * DummyFun now traverses only sub nodes != NULL
 *
 * Revision 1.31  1995/10/16  12:03:24  cg
 * added new function table objinit_tab.
 *
 * Revision 1.30  1995/10/05  16:03:28  cg
 * new traversal tab: impltype_tab
 *
 * Revision 1.29  1995/09/01  07:48:13  cg
 * now sib.h is included for call of function SIBmodul
 *
 * Revision 1.28  1995/08/03  14:52:45  cg
 * NIF-macros adjusted to 26 parameters.
 * sib_tab and obj_tab inserted.
 *
 * Revision 1.27  1995/07/24  11:47:35  asi
 * added ae_tab for array elimination
 *
 * Revision 1.26  1995/07/07  14:58:38  asi
 * added loop unswitching - basic version
 *
 * Revision 1.25  1995/06/23  12:04:13  hw
 * changed name in DBUG_ENTER of function DummyFun2
 *
 * Revision 1.24  1995/06/06  15:19:36  sbs
 * DummyFun2 inserted
 *
 * Revision 1.23  1995/06/02  12:13:35  sbs
 * NIF macro prolongated
 *
 * Revision 1.22  1995/05/26  14:23:42  asi
 * function inlineing and loop unrolling added
 *
 * Revision 1.21  1995/05/01  15:34:57  asi
 * dup_tab inserted
 *
 * Revision 1.20  1995/04/11  15:57:47  asi
 * NIF macro enlarged
 *
 * Revision 1.19  1995/04/07  10:16:26  hw
 * added function NoTrav
 *
 * Revision 1.18  1995/04/05  15:52:38  asi
 * loop invariant removal added
 *
 * Revision 1.17  1995/04/04  12:21:56  asi
 * added include files WorkReduction.h
 *
 * Revision 1.16  1995/03/29  12:00:31  hw
 * comp_tab inserted
 *
 * Revision 1.15  1995/03/17  17:41:48  asi
 * added work reduction
 *
 * Revision 1.14  1995/03/10  10:45:25  hw
 * refcnt_tab inserted
 *
 * Revision 1.13  1995/02/13  17:22:28  asi
 * added include files ConstantFolding.h and DeadCodeRemoval.h
 *
 * Revision 1.12  1995/02/07  10:59:23  asi
 * renamed opt1_tab -> opt_tab, opt2_tab -> dead_tab, opt3_tab -> lir.tab and
 * added functionlist cf_tab for constant folding
 *
 * Revision 1.11  1995/01/31  14:59:33  asi
 * opt4_tab inserted and NIF macro enlarged
 *
 * Revision 1.10  1995/01/18  17:37:16  asi
 * added include free.h
 *
 * Revision 1.9  1995/01/16  10:54:50  asi
 * added opt3_tab for loop independent removal
 * and free_tree for deletion of a syntax(sub)-tree
 *
 * Revision 1.8  1995/01/11  13:25:14  sbs
 * traverse.c:145: warning: unsigned value >= 0 is always 1 fixed
 *
 * Revision 1.7  1995/01/02  16:04:46  asi
 * Renamed opt_tab in opt1_tab and all OPT.. in OPT1..
 * Added OPT1while, OPT1do, OPT1cond, OPT1cc
 * Added opt2_tab
 *
 * Revision 1.6  1994/12/31  14:09:00  sbs
 * DBUG_ASSERT inserted checking the range of node-types!
 *
 * Revision 1.5  1994/12/16  14:22:10  sbs
 * imp_tab inserted and NIF macro enlarged
 *
 * Revision 1.4  1994/12/09  10:13:19  sbs
 * optimize inserted
 *
 * Revision 1.3  1994/12/01  17:41:40  hw
 * added funptr type_tab[]
 * changed parameters of NIF
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "flatten.h"
#include "print.h"
#include "typecheck.h"
#include "typecheck_WL.h"
#include "optimize.h"
#include "free.h"
#include "generatemasks.h"
#include "freemasks.h"
#include "ConstantFolding.h"
#include "DeadCodeRemoval.h"
#include "DeadFunctionRemoval.h"
#include "LoopInvariantRemoval.h"
#include "CSE.h"
#include "import.h"
#include "DupTree.h"
#include "Inline.h"
#include "Unroll.h"
#include "Unswitch.h"
#include "ArrayElimination.h"
#include "index.h"
#include "writesib.h"
#include "readsib.h"
#include "implicittypes.h"
#include "objinit.h"
#include "analysis.h"
#include "checkdec.h"
#include "objects.h"
#include "uniquecheck.h"
#include "rmvoidfun.h"
#include "refcount.h"
#include "wltransform.h"
#include "precompile.h"
#include "compile.h"
#include "ReuseWithArrays.h"
#include "cccall.h"
#include "Old2NewWith.h"
#include "WithloopFolding.h"
#include "WLT.h"
#include "WLI.h"
#include "WLF.h"
#include "gen_startup_code.h"
#include "scheduling.h"
#include "concurrent.h"
#include "spmd_init.h"
#include "spmd_opt.h"
#include "spmd_lift.h"
#include "sync_init.h"
#include "sync_opt.h"
#include "schedule.h"
#include "tile_size_inference.h"

#include "traverse.h"

funptr *act_tab;

/*
 *
 * global definitions of all funtabs needed for Trav
 *
 *
 * 1) imp_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t1

funptr imp_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 2) flat_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t2

funptr flat_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 3) print_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t3

funptr print_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 4) type_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t4

funptr type_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 5) genmask_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t5

funptr genmask_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 6) dead_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t6

funptr dcr_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 7) wlf_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t7

funptr wlf_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 8) free_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t8

funptr free_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 9) cf_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t9

funptr cf_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 10) refcnt_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t10

funptr refcnt_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 11) comp_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t11

funptr comp_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 12) lir_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t12

funptr lir_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 13) dup_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t13

funptr dup_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 14) inline_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t14

funptr inline_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 15) unroll_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t15

funptr unroll_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 16) lir_mov_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t16

funptr lir_mov_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 17) idx_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t17

funptr idx_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 18) unswitch_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t18

funptr unswitch_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 19) wli_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t19

funptr wli_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 20) ae_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t20

funptr ae_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 21) writesib_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t21

funptr writesib_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 22) obj_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t22

funptr obj_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 23) impltype_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t23

funptr impltype_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 24) objinit_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t24

funptr objinit_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 25) analy_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t25

funptr analy_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 26) checkdec_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t26

funptr checkdec_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 27) writedec_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t27

funptr writedec_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 28) unique_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t28

funptr unique_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 29) rmvoid_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t29

funptr rmvoid_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 30) precomp_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t30

funptr precomp_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 31) active_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t31

funptr active_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 32) readsib_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t32

funptr readsib_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 33) wlt_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t33

funptr wlt_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 34) cse_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t34

funptr cse_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 35) dfr_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t35

funptr dfr_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 36) tcwl_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t36

funptr tcwl_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 37) spmdinit_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t37

funptr spmdinit_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 38) spmdopt_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t38

funptr spmdopt_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 39) spmdlift_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t39

funptr spmdlift_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 40) syncinit_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t40

funptr syncinit_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 41) syncopt_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t41

funptr syncopt_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 42) wltrans_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t42

funptr wltrans_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 43) gsc_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t43

funptr gsc_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 44) reuse_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t44

funptr reuse_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 45) o2nWith_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t45

funptr o2nWith_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 46) sched_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t46

funptr sched_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 47) conc_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t47

funptr conc_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 48) opt_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t48

funptr opt_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 49) tsi_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t49

funptr tsi_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 *  50) freemask_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t50

funptr freemask_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 *  nnode
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    nn

int nnode[] = {
#include "node_info.mac"
};
#undef NIF

/*
**
**  functionname  : Trav
**  arguments     : 1) pointer to actual node
**                  2) pointer to further (top down) info's
**  description   : higher order traverse function; selects particular
**                  function from the globaly available funtab
**                  pointed to by "act_tab" with respect to the
**                  actual node-type.
**  global vars   : act_tab
**  internal funs : ---
**  external funs : all funs from flatten.c
**
**  remarks       : could be done as macro as well!!
**
*/

#if 0

node *Trav(node *arg_node, node *arg_info)
{
  DBUG_ENTER("Trav");

  DBUG_ASSERT((NULL != arg_node), "wrong argument: NULL pointer");
  DBUG_ASSERT((arg_node->nodetype <= N_ok),
               "wrong argument: Type-tag out of range!");
  DBUG_PRINT("TRAV",("case %s: node adress: %06x",
                mdb_nodetype[arg_node->nodetype],arg_node));
  DBUG_RETURN((*act_tab[arg_node->nodetype]) (arg_node, arg_info));
}

#else

/*
 * This version of Trav is functionally identical to the above one,
 * but simplifies debugging because it allows to set break points
 * within the assertions.
 */

node *
Trav (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("Trav");

#ifndef DBUG_OFF
    if (arg_node == NULL) {
        DBUG_ASSERT (0, "Trav: tried to traverse into subtree NULL !");
    }

    if (arg_node->nodetype >= N_ok) {
        DBUG_ASSERT (0, "Trav: illegal node type !");
    }

    DBUG_PRINT ("TRAV", ("case %s: node adress: %06x", mdb_nodetype[arg_node->nodetype],
                         arg_node));
#endif /* not DBUG_OFF */

    DBUG_RETURN ((*act_tab[arg_node->nodetype]) (arg_node, arg_info));
}

#endif

/*
**
**  functionname  : TravSons
**  arguments     : 1) pointer to actual node
**                  2) pointer to further (top down) info's
**  description   : traverses all son nodes.
**  global vars   : ---
**  internal funs : Trav
**  external funs : ---
**
**  remarks       : TravSons can be used as dummy function for fun_tab entries
**                  where a specific function for a particular node type is
**                  not yet implemented or not necessary.
**
*/

node *
TravSons (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("TravSons");

    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i] != NULL)
            arg_node->node[i] = Trav (arg_node->node[i], arg_info);

    DBUG_RETURN (arg_node);
}

/*
**
**  functionname  : NoTrav
**  arguments     : 1) pointer to actual node
**                  2) pointer to further (top down) info's
**  description   : does nothing on the given syntax tree,
**                  especially no further sons are traversed.
**                  The given son is returned unmodified.
**  global vars   : ---
**  internal funs : ---
**  external funs : ---
**
**  remarks       : NoTrav can be used as fun_tab entry where no further
**                  traversal of the syntax tree is needed in order
**                  to avoid unnecessary work.
**
*/

node *
NoTrav (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("NoTrav");
    DBUG_RETURN (arg_node);
}
