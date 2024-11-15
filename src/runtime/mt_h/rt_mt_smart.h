/*****************************************************************************
 *
 * file:   rt_mt_smart.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *   This file is part of the SAC standard header file sac.h.
 *   This header file contains the function primitives for the smart decision
 *   tool. The smart decision tool tries to find the "optimal" amount of
 *   threads to solve a parallel task. The tool is used to optimize both
 *   the performance and energy consumption of parallel with loops. The smart
 *   decisions consist of two phases: the training phase and the decision
 *   phase. Each phase requires a recompilation of the sac file, using a
 *   special compilation mode. To compile for the training phase one has to
 *   set the compiler option -mt_smart_mode to 'train'. To compile for the
 *   decision phase the -mt_smart_mode option should be set to 'on'. The
 *   default for the -mt_smart_mode option is 'off', meaning that the smart
 *   decision tool is not being used. The training phase binary has to be
 *   executed before the decision binary is executed. During the training phase
 *   data is collected to create a performance profile of the current machine.
 *   The training phase can be done multiple times in order to collect more
 *   data to construct a more accurate performance profile. The profile is
 *   stored in a separate database (.db) file. After training the decision
 *   binary can be executed, during this phase smart decisions are used to
 *   optimize for the number of threads that are used for each parallel with
 *   loop. The decision phase uses the database file in order to predict the
 *   most optimal settings. If the program has to be executed on several
 *   machines, one has to make sure that a profile is created for each of
 *   these machines.
 *
 * important files:
 *   - mt_smart.c: contains the function implementations of this file
 *   - compile.c: contains the compile time components of this library
 *     (initialization of compile time components of smart tool: COMPdoPrepareSmart
 *function executing compile time components of smart tool: COMPdoDecideSmart function
 *		finalization of compile time components of smart tool: COMPdoFinalizeSmart
 *function there are also a few helper functions in compile.c, such as: rank,
 *create_smart_decision_data, and destroy_smart_decision_data)
 *
 *****************************************************************************/

#ifndef _SAC_RT_MT_SMART_H_
#define _SAC_RT_MT_SMART_H_

#ifdef SAC_SET_SMART_DECISIONS

#if SAC_SET_SMART_DECISIONS == 1
#define SAC_MT_SMART_INIT(nr_threads)                                                    \
    SAC_MT_smart_init (SAC_SET_SMART_DECISIONS, SAC_SET_SMART_FILENAME,                  \
                       SAC_SET_SMART_ARCH, nr_threads)
#define SAC_MT_SMART_BEGIN(spmd_id)                                                      \
    while (SAC_MT_smart_train (spmd_id, SAC_SET_SMART_PERIOD)) {                         \
        clock_gettime (CLOCK_REALTIME, &begin);                                          \
        for (unsigned i = 0; i < smart_sample_size; i++) {
#define SAC_MT_SMART_END()                                                               \
    }                                                                                    \
    clock_gettime (CLOCK_REALTIME, &end);                                                \
    }
#define SAC_MT_SMART_FINALIZE() SAC_MT_smart_finalize ()
#elif SAC_SET_SMART_DECISIONS == 2
#define SAC_MT_SMART_DATA_BEGIN(data_size)                                               \
    {                                                                                    \
        int size = data_size;                                                            \
        const int recommendations[data_size][2] = {
#define SAC_MT_SMART_DATA_ADD(problem_size, nr_threads) {problem_size, nr_threads},
#define SAC_MT_SMART_DATA_END()                                                          \
    }                                                                                    \
    ;
#define SAC_MT_SMART_INIT(nr_threads)                                                    \
    SAC_MT_smart_init (SAC_SET_SMART_DECISIONS, SAC_SET_SMART_FILENAME,                  \
                       SAC_SET_SMART_ARCH, nr_threads)
#define SAC_MT_SMART_BEGIN(spmd_id) SAC_MT_smart_decide (size, recommendations);
#define SAC_MT_SMART_END() }
#define SAC_MT_SMART_FINALIZE()
#else
#define SAC_MT_SMART_INIT(nr_threads)
#define SAC_MT_SMART_BEGIN(spmd_id)
#define SAC_MT_SMART_END()
#define SAC_MT_SMART_FINALIZE()
#endif
#endif

#endif /* _SAC_RT_MT_SMART_H_ */

