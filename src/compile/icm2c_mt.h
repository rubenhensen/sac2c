/*
 *
 * $Log$
 * Revision 2.4  1999/07/20 16:55:06  jhs
 * Added comments.
 * Changed behaviour of MT_SPMD_SETUP, so shared[_rc] variables are no longer setuped.
 * Changed signature of MT_SYNC_FOLD, added barrier_id.
 *
 * Revision 2.3  1999/06/30 16:00:11  jhs
 * Expanded backend, so compilation of fold-with-loops is now possible
 * during SPMD-Blocks containing more than one SYNC-Block.
 *
 * Revision 2.2  1999/06/03 13:09:04  jhs
 * Changed ICMCompileMT_CONTINUE to handle exchanges of new allocated
 * arrays between master and slaves threads.
 *
 * Revision 2.1  1999/02/23 12:42:39  sacbase
 * new release made
 *
 * Revision 1.8  1998/08/27 14:48:45  cg
 * ICM ADJUST_SCHEDULER now gets also upper bound of respective block.
 *
 * Revision 1.7  1998/08/07 16:04:25  dkr
 * MT_SCHEDULER_BEGIN, MT_SCHEDULER_END added
 *
 * Revision 1.6  1998/08/07 07:09:17  cg
 * added declaration of ICMCompileMT_ADJUST_SCHEDULER()
 *
 * Revision 1.5  1998/07/03 10:18:15  cg
 * Super ICM MT_SPMD_BLOCK replaced by combinations of new ICMs
 * MT_SPMD_[STATIC|DYNAMIC]_MODE_[BEGIN|ALTSEQ|END]
 * MT_SPMD_SETUP and MT_SPMD_EXECUTE
 *
 * Revision 1.4  1998/06/23 12:49:48  cg
 * added implementations of scheduling ICMs
 * MT_SCHEDULER_Block_BEGIN and MT_SCHEDULER_Block_END
 *
 * Revision 1.3  1998/05/15 09:19:29  cg
 * added ICMs MT_SYNC_NONFOLD and MT_SYNC_ONEFOLD_NONFOLD
 *
 * Revision 1.2  1998/05/13 14:48:03  cg
 * added ICM MT_SYNC_ONEFOLD
 *
 * Revision 1.1  1998/05/13 07:22:57  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   icm2c_mt.h
 *
 * prefix: ICMCompile
 *
 * description:
 *
 *   This file contains the prototypes of those functions that actually
 *   implement the C implemented ICMs.
 *
 *
 *
 *****************************************************************************/

#ifndef ICM2C_MT_H

#define ICM2C_MT_H

extern void ICMCompileMT_SPMD_FUN_DEC (char *name, char *from, int narg, char **vararg);
extern void ICMCompileMT_SPMD_FUN_RET (int barrier_id, int narg, char **vararg);
extern void ICMCompileMT_START_SYNCBLOCK (int barrier_id, int narg, char **vararg);
extern void ICMCompileMT_SYNC_FOLD (int barrier_id, int narg, char **vararg);
extern void ICMCompileMT_SYNC_NONFOLD (int barrier_id);
extern void ICMCompileMT_SYNC_ONEFOLD (int barrier_id, char *foldtype, char *accu_var,
                                       char *tmp_var, char *foldop);
extern void ICMCompileMT_SYNC_ONEFOLD_NONFOLD (char *foldtype, char *accu_var,
                                               char *tmp_var, char *foldop);
extern void ICMCompileMT_SYNC_FOLD_NONFOLD (int narg, char **vararg);

extern void ICMCompileMT_MASTER_SEND_FOLDRESULTS (int nfoldargs, char **foldargs);
extern void ICMCompileMT_MASTER_RECEIVE_FOLDRESULTS (int nfoldargs, char **foldargs);
extern void ICMCompileMT_MASTER_SEND_SYNCARGS (int nsyncargs, char **syncargs);
extern void ICMCompileMT_MASTER_RECEIVE_SYNCARGS (int nsyncargs, char **syncargs);

extern void ICMCompileMT_CONTINUE (int nfoldargs, char **vararg, int nsyncargs,
                                   char **syncargs);
extern void ICMCompileMT_SPMD_SETUP (char *name, int narg, char **vararg);
extern void ICMCompileMT_SPMD_PRESET (char *name, int narg, char **vararg);

extern void ICMCompileMT_SPMD_STATIC_MODE_BEGIN (char *name);
extern void ICMCompileMT_SPMD_STATIC_MODE_ALTSEQ (char *name);
extern void ICMCompileMT_SPMD_STATIC_MODE_END (char *name);
extern void ICMCompileMT_SPMD_DYNAMIC_MODE_BEGIN (char *name);
extern void ICMCompileMT_SPMD_DYNAMIC_MODE_ALTSEQ (char *name);
extern void ICMCompileMT_SPMD_DYNAMIC_MODE_END (char *name);

extern void ICMCompileMT_ADJUST_SCHEDULER (int current_dim, int array_dim, int lower,
                                           int upper, int unrolling, char *array);

extern void ICMCompileMT_SCHEDULER_BEGIN (int dim, int *vararg);
extern void ICMCompileMT_SCHEDULER_END (int dim, int *vararg);

extern void ICMCompileMT_SCHEDULER_Block_BEGIN (int dim, int *vararg);
extern void ICMCompileMT_SCHEDULER_Block_END (int dim, int *vararg);

#endif /* ICM2C_MT_H */
