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

extern void ICMCompileMT_SCHEDULER_BlockDist_BEGIN (int sched_id, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_BlockDist_END (int sched_id, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_BlockDist_INIT (int sched_id, int dim, char **vararg);

extern void ICMCompileMT_SCHEDULER_BlockVarDist_BEGIN (int sched_id, int dim,
                                                       char **vararg);
extern void ICMCompileMT_SCHEDULER_BlockVarDist_END (int sched_id, int dim,
                                                     char **vararg);
extern void ICMCompileMT_SCHEDULER_BlockVarDist_INIT (int sched_id, int dim,
                                                      char **vararg);

extern void ICMCompileMT_SCHEDULER_Static_BEGIN (int sched_id, char *ts_name, int ts_dims,
                                                 unsigned int ts_arg_num, char **ts_args, int dim,
                                                 char **vararg);
extern void ICMCompileMT_SCHEDULER_Static_END (int sched_id, char *ts_name, int ts_dims,
                                               unsigned int ts_arg_num, char **ts_args, int dim,
                                               char **vararg);
extern void ICMCompileMT_SCHEDULER_Static_INIT (int sched_id, char *ts_name, int ts_dims,
                                                unsigned int ts_arg_num, char **ts_args, int dim,
                                                char **vararg);

extern void ICMCompileMT_SCHEDULER_Self_BEGIN (int sched_id, char *first_task,
                                               char *ts_name, int ts_dims, unsigned int ts_arg_num,
                                               char **ts_args, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_Self_END (int sched_id, char *first_task,
                                             char *ts_name, int ts_dims, unsigned int ts_arg_num,
                                             char **ts_args, int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_Self_INIT (int sched_id, char *first_task,
                                              char *ts_name, int ts_dims, unsigned int ts_arg_num,
                                              char **ts_args, int dim, char **vararg);

extern void ICMCompileMT_SCHEDULER_Affinity_BEGIN (int sched_id, char *ts_name,
                                                   int ts_dims, unsigned int ts_arg_num,
                                                   char **ts_args,

                                                   int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_Affinity_END (int sched_id, char *ts_name, int ts_dims,
                                                 unsigned int ts_arg_num, char **ts_args,

                                                 int dim, char **vararg);
extern void ICMCompileMT_SCHEDULER_Affinity_INIT (int sched_id, char *ts_name,
                                                  int ts_dims, unsigned int ts_arg_num,
                                                  char **ts_args,

                                                  int dim, char **vararg);

#endif /* _SAC_ICM2C_SCHED_H_ */
