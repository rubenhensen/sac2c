/*
 *
 * $Log$
 * Revision 3.6  2004/10/05 17:36:41  khf
 * MT_ADJUST_SCHEDULER__OFFSET removed
 *
 * Revision 3.5  2003/09/19 15:32:01  dkr
 * postfix _nt of varnames renamed into _NT
 *
 * Revision 3.4  2001/06/27 14:35:57  ben
 * modified for cooperation with tasksel-pragma
 *
 * Revision 3.3  2001/06/20 12:25:43  ben
 * some minor bug in definition of Self rmoved
 *
 * Revision 3.2  2001/06/19 12:32:06  ben
 * SCHEDULER_Self modified  with parameter first_task
 *
 * Revision 3.1  2001/05/11 14:35:09  cg
 * Initial revision.
 *
 */

/*****************************************************************************
 *
 * file:   icm2c_sched.h
 *
 * prefix: ICMCompile
 *
 * description:
 *
 *   This file contains the prototypes of those functions that actually
 *   implement the C implemented ICMs.
 *
 *****************************************************************************/

#ifndef _SAC_ICM2C_SCHED_H_
#define _SAC_ICM2C_SCHED_H_

extern void ICMCompileMT_SCHEDULER_BEGIN (int sched_id, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_END (int sched_id, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_INIT (int sched_id, int dim, char **vararg);

extern void ICMCompileMT_SCHEDULER_Block_BEGIN (int sched_id, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_Block_END (int sched_id, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_Block_INIT (int sched_id, int dim, char **vararg);

extern void ICMCompileMT_SCHEDULER_BlockVar_BEGIN (int sched_id, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_BlockVar_END (int sched_id, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_BlockVar_INIT (int sched_id, int dim, char **vararg);

extern void ICMCompileMT_SCHEDULER_Static_BEGIN (int sched_id, char *ts_name, int ts_dims,
                                                 int ts_arg_num, char **ts_args, int dim,
                                                 char **vararg);
extern void ICMCompileMT_SCHEDULER_Static_END (int sched_id, char *ts_name, int ts_dims,
                                               int ts_arg_num, char **ts_args, int dim,
                                               char **vararg);
extern void ICMCompileMT_SCHEDULER_Static_INIT (int sched_id, char *ts_name, int ts_dims,
                                                int ts_arg_num, char **ts_args, int dim,
                                                char **vararg);

extern void ICMCompileMT_SCHEDULER_Self_BEGIN (int sched_id, char *first_task,
                                               char *ts_name, int ts_dims, int ts_arg_num,
                                               char **ts_args, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_Self_END (int sched_id, char *first_task,
                                             char *ts_name, int ts_dims, int ts_arg_num,
                                             char **ts_args, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_Self_INIT (int sched_id, char *first_task,
                                              char *ts_name, int ts_dims, int ts_arg_num,
                                              char **ts_args, int dim, char **vararg);

extern void ICMCompileMT_SCHEDULER_Affinity_BEGIN (int sched_id, char *ts_name,
                                                   int ts_dims, int ts_arg_num,
                                                   char **ts_args,

                                                   int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_Affinity_END (int sched_id, char *ts_name, int ts_dims,
                                                 int ts_arg_num, char **ts_args,

                                                 int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_Affinity_INIT (int sched_id, char *ts_name,
                                                  int ts_dims, int ts_arg_num,
                                                  char **ts_args,

                                                  int dim, char **vararg);

#endif /* _SAC_ICM2C_SCHED_H_ */
