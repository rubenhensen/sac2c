/*
 *
 * $Log$
 * Revision 3.22  2003/10/20 15:34:42  dkr
 * MT_SYNCBLOCK_CLEANUP
 *
 * Revision 3.21  2003/10/15 17:30:06  dkrHH
 * MT_CREATE_LOCAL_DESC added
 *
 * Revision 3.20  2003/10/15 12:31:00  dkrHH
 * MT_START_SYNCBLOCK renamed into MT_SYNCBLOCK_BEGIN.
 * MT_SYNCBLOCK_END added.
 *
 * Revision 3.19  2003/10/14 23:44:11  dkrHH
 * SPMD_PRESET removed
 *
 * Revision 3.18  2003/09/17 14:17:27  dkr
 * some function parameters renamed
 *
 * Revision 3.17  2003/09/17 13:03:03  dkr
 * postfixes _nt, _any renamed into _NT, _ANY
 *
 * Revision 3.16  2003/08/04 16:56:36  dkr
 * argument of MT_SPMD_FUN_DEC, MT_SPMD_FUN_RET renamed
 *
 * Revision 3.15  2001/05/11 14:36:56  cg
 * Implementations of ICMs concerned with loop scheduling
 * are now moved to new specific file icm2c_sched.c
 *
 * Revision 3.14  2001/05/09 15:13:00  cg
 * All scheduling ICMs get an additional first parameter,
 * i.e. the segment ID. This is required to identify the appropriate
 * set of scheduler internal variables.
 *
 * Revision 3.13  2001/05/04 11:48:23  ben
 *  MT_SCHEDULER_Even_... deleted
 *  MT_SCHEDULER_Cyclic_... renamed to _Static_
 *  MT_SCHEDULER_Afs_... renamed to Affinity
 *
 * Revision 3.12  2001/04/03 19:41:07  dkr
 * MT_ADJUST_SCHEDULER renamed into MT_ADJUST_SCHEDULER__OFFSET.
 * signature for MT_ADJUST_SCHEDULER_... icms modified.
 * MT_ADJUST_SCHEDULER icm is not a c- but a h-icm now.
 *
 * Revision 3.11  2001/03/28 12:51:25  ben
 * param added to MT_SCHEDULER_(Cyclic,Self,AFS)_...
 *
 * Revision 3.10  2001/03/27 11:51:51  ben
 * MT_SCHEDULER_Afs_ added
 *
 * Revision 3.9  2001/03/22 17:40:06  ben
 * ICMs MT_SCHEDULER_Self_... added
 *
 * Revision 3.8  2001/03/22 12:45:02  ben
 * ICMs MT_SCHEDULER_Cyclic_... added
 *
 * Revision 3.7  2001/03/20 16:11:38  ben
 * Just implemented Static renamed to Even, because of existing Static
 * scheduling
 *
 * Revision 3.6  2001/03/20 13:18:39  ben
 * ICMs MT_SCHEDULER_Static_... (first version) implemented
 * SelectTask implemented
 *
 * Revision 3.5  2001/03/14 16:25:02  dkr
 * signature (parameter types) of icm MT_ADJUST_SCHEDULER modified
 *
 * Revision 3.4  2001/03/14 10:13:49  ben
 *  ICMs MT_SCHEDULER_BlockVar_... implemented
 *
 * Revision 3.3  2001/01/24 23:38:37  dkr
 * type of arguments of ICMs MT_SCHEDULER_..._BEGIN, MT_SCHEDULER_..._END
 * changed from int* to char**
 *
 * Revision 3.2  2001/01/22 13:45:09  dkr
 * signature of ICMCompileMT_ADJUST_SCHEDULER modified
 *
 * Revision 3.1  2000/11/20 18:01:16  sacbase
 * new release made
 *
 * Revision 2.5  2000/01/17 16:25:58  cg
 * Removed static and dynamic versions of the ICMs
 * MT_SPMD_[STATIC|DYNAMIC]_MODE_[BEGIN|ALTSEQ|END].
 * General version now is identical with the former dynamic
 * version.
 *
 * Revision 2.4  1999/07/20 16:55:06  jhs
 * Added comments.
 * Changed behaviour of MT_SPMD_SETUP, so shared[_rc] variables are no longer
 * setuped.
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
 *****************************************************************************/

#ifndef _SAC_ICM2C_MT_H_
#define _SAC_ICM2C_MT_H_

extern void ICMCompileMT_SPMD_FUN_DEC (char *name, char *from, int vararg_cnt,
                                       char **vararg);
extern void ICMCompileMT_SPMD_FUN_RET (int barrier_id, int vararg_cnt, char **vararg);

extern void ICMCompileMT_SYNCBLOCK_BEGIN (int barrier_id, int vararg_cnt, char **vararg);
extern void ICMCompileMT_SYNCBLOCK_CLEANUP (int barrier_id, int vararg_cnt,
                                            char **vararg);
extern void
ICMCompileMT_SYNCBLOCK_END
(int barrier_id, int vararg_cnt, char **vararg);

extern void ICMCompileMT_SYNC_FOLD (int barrier_id, int vararg_cnt, char **vararg);
extern void ICMCompileMT_SYNC_NONFOLD (int barrier_id);
extern void ICMCompileMT_SYNC_ONEFOLD (int barrier_id, char *foldtype, char *accu_var,
                                       char *tmp_var, char *foldop);
extern void ICMCompileMT_SYNC_ONEFOLD_NONFOLD (char *foldtype, char *accu_var,
                                               char *tmp_var, char *foldop);
extern void ICMCompileMT_SYNC_FOLD_NONFOLD (int vararg_cnt, char **vararg);

extern void ICMCompileMT_MASTER_SEND_FOLDRESULTS (int foldargs_cnt, char **foldargs);
extern void ICMCompileMT_MASTER_RECEIVE_FOLDRESULTS (int foldargs_cnt, char **foldargs);
extern void ICMCompileMT_MASTER_SEND_SYNCARGS (int syncargs_cnt, char **syncargs);
extern void ICMCompileMT_MASTER_RECEIVE_SYNCARGS (int syncargs_cnt, char **syncargs);

extern void ICMCompileMT_SPMD_SETUP (char *name, int vararg_cnt, char **vararg);

extern void ICMCompileMT_SPMD_BEGIN (char *name);
extern void ICMCompileMT_SPMD_ALTSEQ (char *name);
extern void ICMCompileMT_SPMD_END (char *name);

extern void ICMCompileMT_CREATE_LOCAL_DESC (char *var_NT, int dim);

#endif /* _SAC_ICM2C_MT_H_ */
