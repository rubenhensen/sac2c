/*****************************************************************************
 *
 * file:   mt_beehive.h
 *
 * prefix: SAC_MT_
 *
 * description:
 *    This file is part of the SAC standard header file sac.h
 *    Common parts for the bee-hives based MT backends: LPEL and PThreads (PTH).
 *
 *
 *****************************************************************************/

#ifndef _SAC_RT_BEEHIVE_H_
#define _SAC_RT_BEEHIVE_H_

/* NOTE: SAC_DO_MT_BEEHIVE is defined only when including from mt_beehive.c */
#if SAC_DO_MULTITHREAD && (SAC_DO_MT_BEEHIVE || SAC_DO_MT_PTHREAD || SAC_DO_MT_LPEL)

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

/*****************************************************************************/

/*
 * Macros for implementing the SPMD frame used to move data into SPMD function
 *
 * SAC MT FRAME, used to pass in and inout arguments to SPMD functions.
 * Defined as a union of structs, one struct for every SPMD function.
 */

/* The single global frame variable. */
// #define SAC_DO_MT_GLOBAL_FRAME         1
/* The Fork-Join mode: create a new hive for each SPMD call, then destroy
 * it immediatelly afterwards */
// #define SAC_DO_MT_CREATE_JOIN          1

#if SAC_DO_MT_GLOBAL_FRAME

/* Define a global union type SAC_spmd_frame_t to contain all SPMD-specific frame types.
 * The union type name SAC_spmd_frame_t is currently not used. */
#define SAC_MT_SPMD_FRAME_BEGIN() union SAC_spmd_frame_t {

/* Define a single global frame variable, called SAC_spmd_frame */
#define SAC_MT_SPMD_FRAME_END()                                                          \
    int _dummy;                                                                          \
    }                                                                                    \
    SAC_spmd_frame;

#else /* SAC_DO_MT_GLOBAL_FRAME else */

/* Local frames allocated at spmd call points. */

#define SAC_MT_SPMD_FRAME_BEGIN() /* empty */
#define SAC_MT_SPMD_FRAME_END()   /* empty */

#endif /* SAC_DO_MT_GLOBAL_FRAME */

/* Define a new struct type, specific for the SPMD function. */
#define SAC_MT_SPMD_FRAME_ELEMENT_BEGIN(spmdfun) struct spmdfun##_FT {

#if SAC_DO_MT_GLOBAL_FRAME

/* _dummy: C99 does not allow empty structs/unions */
/* this is a field in the global union */
#define SAC_MT_SPMD_FRAME_ELEMENT_END(spmdfun)                                           \
    int _dummy;                                                                          \
    }                                                                                    \
    spmdfun##_FV;

#else /* SAC_DO_MT_GLOBAL_FRAME else */

/* _dummy: C99 does not allow empty structs/unions */
/* this is only a struct type definition */
#define SAC_MT_SPMD_FRAME_ELEMENT_END(spmdfun)                                           \
    int _dummy;                                                                          \
    }                                                                                    \
    ;

#endif /* SAC_DO_MT_GLOBAL_FRAME */

#define SAC_MT_FRAME_ELEMENT_in__NODESC(name, num, basetype, var_NT)                     \
    SAC_ND_TYPE (var_NT, basetype) in_##num;

#define SAC_MT_FRAME_ELEMENT_in__DESC(name, num, basetype, var_NT)                       \
    SAC_ND_TYPE (var_NT, basetype) in_##num;                                             \
    SAC_ND_DESC_TYPE (var_NT) in_##num##_desc;

#define SAC_MT_FRAME_ELEMENT_inout__NODESC(name, num, basetype, var_NT)                  \
    SAC_ND_TYPE (var_NT, basetype) * in_##num;

#define SAC_MT_FRAME_ELEMENT_inout__DESC(name, num, basetype, var_NT)                    \
    SAC_ND_TYPE (var_NT, basetype) * in_##num;                                           \
    SAC_ND_DESC_TYPE (var_NT) * in_##num##_desc;

#define SAC_MT_FRAME_ELEMENT__NOOP(name, num, basetype, var_NT)

/*****************************************************************************/

/*
 * Macros for sending data to the SPMD frame
 */

/* NORC: first switch, then ship. Because the mode may be stored in the pointer LSBs! */

#define SAC_MT_SEND_PARAM_in__NODESC(spmdfun, num, var_NT)                               \
    frame->in_##num = SAC_ND_A_FIELD (var_NT);

#define SAC_MT_SEND_PARAM_in__DESC_AKD(spmdfun, num, var_NT)                             \
    SAC_ND_RC_TO_NORC (var_NT);                                                          \
    DESC_DIM (SAC_ND_A_DESC (var_NT)) = SAC_ND_A_DIM (var_NT);                           \
    frame->in_##num = SAC_ND_A_FIELD (var_NT);                                           \
    frame->in_##num##_desc = SAC_ND_A_DESC (var_NT);

#define SAC_MT_SEND_PARAM_in__DESC_AUD(spmdfun, num, var_NT)                             \
    SAC_ND_RC_TO_NORC (var_NT);                                                          \
    frame->in_##num = SAC_ND_A_FIELD (var_NT);                                           \
    frame->in_##num##_desc = SAC_ND_A_DESC (var_NT);

#define SAC_MT_SEND_PARAM_inout__NODESC(spmdfun, num, var_NT)                            \
    frame->in_##num = &SAC_ND_A_FIELD (var_NT);

#define SAC_MT_SEND_PARAM_inout__DESC_AKD(spmdfun, num, var_NT)                          \
    SAC_ND_RC_TO_NORC (var_NT);                                                          \
    DESC_DIM (SAC_ND_A_DESC (var_NT)) = SAC_ND_A_DIM (var_NT);                           \
    frame->in_##num = &SAC_ND_A_FIELD (var_NT);                                          \
    frame->in_##num##_desc = &SAC_ND_A_DESC (var_NT);

#define SAC_MT_SEND_PARAM_inout__DESC_AUD(spmdfun, num, var_NT)                          \
    SAC_ND_RC_TO_NORC (var_NT);                                                          \
    frame->in_##num = &SAC_ND_A_FIELD (var_NT);                                          \
    frame->in_##num##_desc = &SAC_ND_A_DESC (var_NT);

#define SAC_MT_SEND_PARAM__NOOP(spmdfun, num, var_NT) SAC_NOOP ()

/*****************************************************************************/

/*
 * Macros for receiving data from the SPMD frame
 */

#define SAC_MT_SELF_FRAME(spmdfun) ((struct spmdfun##_FT *)SAC_MT_self->c.hive->framedata)

/* no descriptor */
#define SAC_MT_RECEIVE_PARAM_in__NODESC(spmdfun, num, basetype, var_NT)                  \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT) = SAC_MT_SELF_FRAME (spmdfun)->in_##num;

#if 0
/* (not used) create a new but fake descriptor */
#define SAC_MT_RECEIVE_PARAM_in__NODESC__FAKERC(spmdfun, num, basetype, var_NT)          \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT) = SAC_MT_SELF_FRAME (spmdfun)->in_##num;                     \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    SAC_ND_A_DESC (var_NT) = (SAC_ND_DESC_TYPE (var_NT))alloca (BYTE_SIZE_OF_DESC (0));  \
    DESC_RC (SAC_ND_A_DESC (var_NT)) = 2;
#endif

#if SAC_HAS_RC_NORC
/* the RC method supports the NORC mode, hence we can just take the descriptor from the
 * frame */
#define SAC_MT_RECEIVE_PARAM_in__DESC(spmdfun, num, basetype, var_NT)                    \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT) = SAC_MT_SELF_FRAME (spmdfun)->in_##num;                     \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    SAC_ND_A_DESC (var_NT) = SAC_MT_SELF_FRAME (spmdfun)->in_##num##_desc;
#else
/* create a local copy of the incomming descriptor on stack */
#define SAC_MT_RECEIVE_PARAM_in__DESC(spmdfun, num, basetype, var_NT)                    \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT) = SAC_MT_SELF_FRAME (spmdfun)->in_##num;                     \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    SAC_ND_A_DESC (var_NT) = (SAC_ND_DESC_TYPE (var_NT))alloca (                         \
      BYTE_SIZE_OF_DESC (DESC_DIM (SAC_MT_SELF_FRAME (spmdfun)->in_##num##_desc)));      \
    memcpy (SAC_ND_A_DESC (var_NT), SAC_MT_SELF_FRAME (spmdfun)->in_##num##_desc,        \
            BYTE_SIZE_OF_DESC (                                                          \
              DESC_DIM (SAC_MT_SELF_FRAME (spmdfun)->in_##num##_desc)));

//    DESC_RC (SAC_ND_A_DESC (var_NT)) = 2;
#endif

/* SCL & HID: create a new descriptor on stack */
#define SAC_MT_RECEIVE_PARAM_in__NEWDESC(spmdfun, num, basetype, var_NT)                 \
    SAC_ND_TYPE (var_NT, basetype)                                                       \
    SAC_ND_A_FIELD (var_NT) = SAC_MT_SELF_FRAME (spmdfun)->in_##num;                     \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    SAC_ND_A_DESC (var_NT)                                                               \
      = (SAC_ND_DESC_TYPE (var_NT))alloca (SIZE_OF_DESC (0) * sizeof (int));             \
    SAC_ND_INIT__RC (var_NT, 2);

/* no descriptor */
#define SAC_MT_RECEIVE_PARAM_inout__NODESC(spmdfun, num, basetype, var_NT)               \
    SAC_ND_TYPE (var_NT, basetype) * SAC_NAMEP (SAC_ND_A_FIELD (var_NT))                 \
      = SAC_MT_SELF_FRAME (spmdfun)->in_##num;

#if SAC_HAS_RC_NORC
/* the RC method supports the NORC mode, hence we can just take the descriptor from the
 * frame */
#define SAC_MT_RECEIVE_PARAM_inout__DESC(spmdfun, num, basetype, var_NT)                 \
    SAC_ND_TYPE (var_NT, basetype) * SAC_NAMEP (SAC_ND_A_FIELD (var_NT))                 \
      = SAC_MT_SELF_FRAME (spmdfun)->in_##num;                                           \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    CAT0 (SAC_ND_A_DESC (var_NT), __s) = *SAC_MT_SELF_FRAME (spmdfun)->in_##num##_desc;  \
    SAC_ND_DESC_TYPE (var_NT) * SAC_NAMEP (SAC_ND_A_DESC (var_NT))                       \
      = &CAT0 (SAC_ND_A_DESC (var_NT), __s);
#else
/* create a local copy of the incomming descriptor on stack */
#define SAC_MT_RECEIVE_PARAM_inout__DESC(spmdfun, num, basetype, var_NT)                 \
    SAC_ND_TYPE (var_NT, basetype) * SAC_NAMEP (SAC_ND_A_FIELD (var_NT))                 \
      = SAC_MT_SELF_FRAME (spmdfun)->in_##num;                                           \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    CAT0 (SAC_ND_A_DESC (var_NT), __s) = (SAC_ND_DESC_TYPE (var_NT))alloca (             \
      BYTE_SIZE_OF_DESC (DESC_DIM (*SAC_MT_SELF_FRAME (spmdfun)->in_##num##_desc)));     \
    memcpy (CAT0 (SAC_ND_A_DESC (var_NT), __s),                                          \
            *SAC_MT_SELF_FRAME (spmdfun)->in_##num##_desc,                               \
            BYTE_SIZE_OF_DESC (                                                          \
              DESC_DIM (*SAC_MT_SELF_FRAME (spmdfun)->in_##num##_desc)));                \
    SAC_ND_DESC_TYPE (var_NT) * SAC_NAMEP (SAC_ND_A_DESC (var_NT))                       \
      = &CAT0 (SAC_ND_A_DESC (var_NT), __s);
#endif

/* SCL & HID: create a new descriptor on stack */
/* TODO: probably should use SAC_ND_INIT__RC instead of memset */
#define SAC_MT_RECEIVE_PARAM_inout__NODESC__FAKERC(spmdfun, num, basetype, var_NT)       \
    SAC_ND_TYPE (var_NT, basetype) * SAC_NAMEP (SAC_ND_A_FIELD (var_NT))                 \
      = SAC_MT_SELF_FRAME (spmdfun)->in_##num;                                           \
    SAC_ND_DESC_TYPE (var_NT)                                                            \
    CAT0 (SAC_ND_A_DESC (var_NT), __s)                                                   \
      = (SAC_ND_DESC_TYPE (var_NT))alloca (FIXED_SIZE_OF_DESC * sizeof (int));           \
    memset (CAT0 (SAC_ND_A_DESC (var_NT), __s), '\0',                                    \
            FIXED_SIZE_OF_DESC * sizeof (int));                                          \
    SAC_ND_DESC_TYPE (var_NT) * SAC_NAMEP (SAC_ND_A_DESC (var_NT))                       \
      = &CAT0 (SAC_ND_A_DESC (var_NT), __s);

#define SAC_MT_RECEIVE_PARAM__NOOP(spmdfun, num, basetype, var_NT) SAC_NOOP ()

/*****************************************************************************/

/*
 * Macros for establishing a fake descriptor for AKS SPMD function arguments
 */

#define SAC_MT_DECL__MIRROR_PARAM__DESC(var_NT, dim)                                     \
    SAC_ND_DESC_TYPE (var_NT) SAC_ND_A_DESC (var_NT) = alloca (BYTE_SIZE_OF_DESC (dim)); \
    SAC_ND_INIT__RC (var_NT, 2);                                                         \
    DESC_DIM (SAC_ND_A_DESC (var_NT)) = 2;

#define SAC_MT_DECL__MIRROR_PARAM__NODESC(var_NT, dim) SAC_NOOP ()

/*****************************************************************************/

/*
 * Macros for defining the synchronisation barrier
 *
 * SAC MT BARRIER, used to pass out parameters out of SPMD functions.
 * Defined as several struct types.
 */

#if SAC_DO_MT_GLOBAL_FRAME

#define SAC_MT_SPMD_BARRIER_BEGIN() union SAC_spmd_barrier_t {

#else /* SAC_DO_MT_GLOBAL_FRAME */

#define SAC_MT_SPMD_BARRIER_BEGIN() /* empty */

#endif /* SAC_DO_MT_GLOBAL_FRAME */

#define SAC_MT_SPMD_BARRIER_ELEMENT_BEGIN(spmdfun) struct spmdfun##_RT {

#define SAC_MT_BARRIER_ELEMENT__NOOP(name, num, basetype, var_NT)

#define SAC_MT_BARRIER_ELEMENT_out__NODESC(name, num, basetype, var_NT)                  \
    SAC_ND_TYPE (var_NT, basetype) in_##num;

#define SAC_MT_BARRIER_ELEMENT_out__DESC(name, num, basetype, var_NT)                    \
    SAC_ND_TYPE (var_NT, basetype) in_##num;                                             \
    SAC_ND_DESC_TYPE (var_NT) in_##num##_desc;

#if SAC_DO_MT_GLOBAL_FRAME
/* Single global frame/barrier */

#define SAC_MT_SPMD_BARRIER_ELEMENT_END(spmdfun)                                         \
    int _dummy; /* C99 does not allow empty structs/unions */                            \
    }                                                                                    \
    spmdfun##_BV;

#define SAC_MT_SPMD_BARRIER_END()                                                        \
    int _dummy;                                                                          \
    }                                                                                    \
    SAC_spmd_barrier[SAC_SET_THREADS_MAX + 1];

#else /* SAC_DO_MT_GLOBAL_FRAME else */
/* Local frame/barrier, needed for sac4c mt */
#define SAC_MT_SPMD_BARRIER_ELEMENT_END(spmdfun)                                         \
    int _dummy; /* C99 does not allow empty structs/unions */                            \
    }                                                                                    \
    ;

#define SAC_MT_SPMD_BARRIER_END() /* empty */

#endif /* SAC_DO_MT_GLOBAL_FRAME */

/*****************************************************************************/

/*
 *  Macros for sending data to the synchronisation barrier
 */

#define SAC_MT_SEND_RESULT__NOOP(spmdfun, local_id, num, var_NT) SAC_NOOP ()

#define SAC_MT_SEND_RESULT_out__NODESC(spmdfun, local_id, num, var_NT)                   \
    rdata[local_id].in_##num = SAC_ND_A_FIELD (var_NT);

#define SAC_MT_SEND_RESULT_out__DESC(spmdfun, local_id, num, var_NT)                     \
    rdata[local_id].in_##num = SAC_ND_A_FIELD (var_NT);                                  \
    rdata[local_id].in_##num##_desc = SAC_ND_A_DESC (var_NT);

/*****************************************************************************/

/*
 *  Macros for receiving data from the synchronisation barrier
 */

#define SAC_MT_RECEIVE_RESULT__NOOP(spmdfun, local_id, num, var_NT) SAC_NOOP ()

#define SAC_MT_RECEIVE_RESULT_in__DESC(spmdfun, local_id, num, var_NT)                   \
    SAC_ND_RC_FROM_NORC (var_NT);

#define SAC_MT_RECEIVE_RESULT_inout__DESC(spmdfun, local_id, num, var_NT)                \
    SAC_ND_RC_FROM_NORC (var_NT);

#define SAC_MT_RECEIVE_RESULT_out__NODESC(spmdfun, local_id, num, var_NT)                \
    SAC_ND_A_FIELD (var_NT) = rdata[local_id].in_##num;

#define SAC_MT_RECEIVE_RESULT_out__DESC(spmdfun, local_id, num, var_NT)                  \
    SAC_ND_A_FIELD (var_NT) = rdata[local_id].in_##num;                                  \
    SAC_ND_A_DESC (var_NT) = rdata[local_id].in_##num##_desc;

/*****************************************************************************/

/*
 * Macros for implementing the barrier synchronisation
 */

#define SAC_MT_SYNC_BEGIN(spmdfun)                                                       \
    {                                                                                    \
        unsigned int SAC_MT_ready_count = SAC_MT_MYWORKERCLASS ();                       \
        unsigned int SAC_MT_son_id;                                                      \
        unsigned int SAC_MT_i;                                                           \
        struct spmdfun##_RT *rdata;                                                      \
        rdata = (struct spmdfun##_RT *)SAC_MT_self->c.hive->retdata;                     \
                                                                                         \
        while (SAC_MT_ready_count > 0) {                                                 \
            SAC_MT_i = SAC_MT_MYWORKERCLASS ();                                          \
                                                                                         \
            do {                                                                         \
                SAC_MT_son_id = SAC_MT_SELF_LOCAL_ID () + SAC_MT_i;                      \
                                                                                         \
                if (SAC_MT_CHECK_BARRIER (spmdfun, SAC_MT_son_id)) {

#define SAC_MT_SYNC_CONT(spmdfun)                                                        \
    SAC_MT_CLEAR_BARRIER (spmdfun, SAC_MT_son_id)                                        \
    SAC_MT_ready_count >>= 1;                                                            \
    if (SAC_MT_ready_count == 0) {                                                       \
        break;                                                                           \
    }                                                                                    \
    }                                                                                    \
    }                                                                                    \
    while (SAC_MT_i >>= 1)                                                               \
        ;                                                                                \
    SAC_MT_SYNC_RELAX ();                                                                \
    }

#define SAC_MT_SYNC_END(spmdfun)                                                         \
    SAC_MT_SET_BARRIER (spmdfun, SAC_MT_SELF_LOCAL_ID ())                                \
    }

#define SAC_MT_SYNC_FOLD__NOOP(spmdfun, num, accu_NT, val_NT, basetype, tag, foldfun)    \
    SAC_NOOP ()

#define SAC_MT_SYNC_FOLD_out__NODESC(spmdfun, num, accu_NT, val_NT, basetype, tag,       \
                                     foldfun)                                            \
    SAC_MT_RECEIVE_RESULT_out__NODESC (spmdfun, SAC_MT_son_id, num, val_NT);             \
    SAC_##tag##_FUNAP2 (foldfun, SAC_ND_ARG_out (accu_NT, basetype),                     \
                        SAC_ND_ARG_in (accu_NT, basetype),                               \
                        SAC_ND_ARG_in (val_NT, basetype));

/* there must not be a norc switch here! */
#define SAC_MT_SYNC_FOLD_out__DESC(spmdfun, num, accu_NT, val_NT, basetype, tag,         \
                                   foldfun)                                              \
    SAC_MT_RECEIVE_RESULT_out__DESC (spmdfun, SAC_MT_son_id, num, val_NT);               \
    SAC_##tag##_FUNAP2 (foldfun, SAC_ND_ARG_out (accu_NT, basetype),                     \
                        SAC_ND_ARG_in (accu_NT, basetype),                               \
                        SAC_ND_ARG_in (val_NT, basetype));

/* used to call the reduction function in a folding code */
#define SAC_MT_FUNAP2(name, ...) name (SAC_MT_self, __VA_ARGS__);

/*****************************************************************************/

/*
 *  Macros for implementing and calling spmd-functions
 */

/* Begin SPMD invocation scope in the master worker. */

#if SAC_DO_MT_GLOBAL_FRAME
/* Use the single global frame variable and barrier.
 * The frame is easy: just extract the field from the union.
 * The barrier is more difficult: it is an array of unions. Just re-type it
 * to the correct type and use as an array subsequently.
 * Use this only for experimental work; it breaks sac4c when used from
 * a multithreaded environment. */
#define SAC_MT_BEGIN_SPMD_INVOCATION(name)                                               \
    {                                                                                    \
        SAC_MT_BEGIN_SPMD_INVOCATION_PROLOG (name)                                       \
        struct name##_FT *const frame = &SAC_spmd_frame.name##_FV;                       \
        memset (frame, 0, sizeof (struct name##_FT));                                    \
        struct name##_RT *rdata = (void *)SAC_spmd_barrier;                              \
        memset (rdata, 0, sizeof (struct name##_RT) * SAC_MT_self->c.hive->num_bees);

#else /* SAC_DO_MT_GLOBAL_FRAME else */
/* Allocate frame locally. This is the default and suggested solution. */
#define SAC_MT_BEGIN_SPMD_INVOCATION(name)                                               \
    {                                                                                    \
        SAC_MT_BEGIN_SPMD_INVOCATION_PROLOG (name)                                       \
        struct name##_FT _frame_st;                                                      \
        struct name##_FT *const frame = &_frame_st;                                      \
        memset (frame, 0, sizeof (struct name##_FT));                                    \
        struct name##_RT rdata[SAC_MT_self->c.hive->num_bees];                           \
        memset (rdata, 0, sizeof (struct name##_RT) * SAC_MT_self->c.hive->num_bees);

#endif /* SAC_DO_MT_GLOBAL_FRAME */

/* end SPMD invocation scope in the master worker */
#define SAC_MT_END_SPMD_INVOCATION(name)                                                 \
    SAC_MT_END_SPMD_INVOCATION_EPILOG (name)                                             \
    }

#define SAC_MT_MYWORKERCLASS() SAC_MT_self->c.b_class

/* generated in main() */
#define SAC_MT_DECL_MYTHREAD()

#if SAC_DO_MT_CREATE_JOIN
/* The Fork-Join mode: create a new hive for each SPMD call, then destroy
 * it immediatelly afterwards */
#define SAC_MT_BEGIN_SPMD_INVOCATION_PROLOG(name)                                        \
    SAC_MT_AttachHive (                                                                  \
      SAC_MT_AllocHive (SAC_MT_GLOBAL_THREADS (), SAC_SET_NUM_SCHEDULERS, NULL, NULL));

#define SAC_MT_END_SPMD_INVOCATION_EPILOG(name) SAC_MT_ReleaseHive (SAC_MT_DetachHive ());

#else

/* The Normal mode: the hive is kept across SPMD calls */
#define SAC_MT_BEGIN_SPMD_INVOCATION_PROLOG(name)
#define SAC_MT_END_SPMD_INVOCATION_EPILOG(name)

#endif

/*****************************************************************************/

/*
 * SAC_PRF_RUNMT* primitive functions. These decide whether to execute the
 * following SPMD block sequentially or in parallel.
 * We compare the number of elements in the result array to a threshold.
 * In the AKS case the same test is performed statically in sac2c.
 */

#define SAC_ND_PRF_RUNMT_GENARRAY__DATA(var_NT, mem_NT, min_parallel_size)               \
    SAC_ND_WRITE (var_NT, 0) = (SAC_ND_A_DESC_SIZE (mem_NT) >= (min_parallel_size));

#define SAC_ND_PRF_RUNMT_MODARRAY__DATA(var_NT, mem_NT, min_parallel_size)               \
    SAC_ND_WRITE (var_NT, 0) = (SAC_ND_A_DESC_SIZE (mem_NT) >= (min_parallel_size));

#define SAC_ND_PRF_RUNMT_FOLD__DATA(var_NT, args) SAC_ND_WRITE (var_NT, 0) = 0;

/*****************************************************************************/

/*
 *  Macros for object access synchronisation within SPMD functions
 */

#define SAC_ND_PROP_OBJ_IN() SAC_MT_ACQUIRE_LOCK (SAC_MT_propagate_lock);

#define SAC_ND_PROP_OBJ_OUT() SAC_MT_RELEASE_LOCK (SAC_MT_propagate_lock);

#define SAC_ND_PROP_OBJ_UNBOX(unboxed, boxed)                                            \
    SAC_ND_A_FIELD (unboxed) = *SAC_NAMEP (SAC_ND_A_FIELD (boxed));

#define SAC_ND_PROP_OBJ_BOX(boxed, unboxed)                                              \
    *SAC_NAMEP (SAC_ND_A_FIELD (boxed)) = SAC_ND_A_FIELD (unboxed);

/*****************************************************************************/

#endif /* SAC_DO_MULTITHREAD && (SAC_DO_MT_BEEHIVE || SAC_DO_MT_PTHREAD ||               \
          SAC_DO_MT_LPEL) */
#endif /* ndef _SAC_RT_BEEHIVE_H_ */
