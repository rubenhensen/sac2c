/*
 *
 * $Log$
 * Revision 2.5  1999/06/16 10:18:22  jhs
 * Added spmdtrav_tab.
 *
 * Revision 2.4  1999/05/10 10:54:13  bs
 * tsi_tab renamed to wlaa_tab
 *
 * Revision 2.3  1999/05/06 15:35:42  sbs
 * filename- mechanism added for creating better error-messages
 * when multiple files are involved!
 *
 * Revision 2.2  1999/05/06 12:10:12  sbs
 * implicit linenum-setting mechanism added to Trav !!
 * => Makexxx-calls might yield better line-numbers now...
 *
 * Revision 2.1  1999/02/23 12:39:44  sacbase
 * new release made
 *
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
 * ... [eliminated] ...
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
#include "wl_access_analyze.h"
#include "scnprs.h" /* needed for linenum only!!! */

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
 * 49) wlaa_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t49

funptr wlaa_tab[] = {
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
 *  51) spmdtrav_tab
 */

#define NIF(n, s, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, \
            t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31,   \
            t32, t33, t34, t35, t36, t37, t38, t39, t40, t41, t42, t43, t44, t45, t46,   \
            t47, t48, t49, t50, t51, t52, nn)                                            \
    t51

funptr spmdtrav_tab[] = {
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

node *
Trav (node *arg_node, node *arg_info)
{
    int old_linenum = linenum;
    char *old_filename = filename;
    node *result;

    DBUG_ENTER ("Trav");

    /*
     * Make sure the line-number will be set
     * correctly in case MakeXxx is called.
     */
    linenum = NODE_LINE (arg_node);
    filename = NODE_FILE (arg_node);

#ifndef DBUG_OFF
    /*
     * This ifndef construction is ugly(!) but it simplifies debugging
     * because it allows to set break points "within" the assertions...
     */

    if (arg_node == NULL) {
        DBUG_ASSERT (0, "Trav: tried to traverse into subtree NULL !");
    }

    if (arg_node->nodetype >= N_ok) {
        DBUG_ASSERT (0, "Trav: illegal node type !");
    }

    DBUG_PRINT ("TRAV", ("case %s: node adress: %06x", mdb_nodetype[arg_node->nodetype],
                         arg_node));
#endif /* not DBUG_OFF */

    result = (*act_tab[arg_node->nodetype]) (arg_node, arg_info);
    linenum = old_linenum;
    filename = old_filename;

    DBUG_RETURN (result);
}

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
