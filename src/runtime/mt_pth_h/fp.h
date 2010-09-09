/*
 * $Id$
 */

/*****************************************************************************
 *
 * file:   fp.h
 *
 * prefix: SAC_FP_
 *
 * description:
 *
 *   Contains the ICMs for Functional Parallelism
 *
 *****************************************************************************/

#ifndef _SAC_FP_H_
#define _SAC_FP_H_

#if SAC_DO_FP

/*
 * Definitions for creating the task frame
 */

#define SAC_FP_FRAME_START()                                                             \
    typedef struct _SAC_fp_frame {                                                       \
        int entry;                                                                       \
        int done;                                                                        \
        int dqstate;                                                                     \
        void (*slow_clone) (struct _SAC_fp_frame * f);                                   \
        union {

#define SAC_FP_FRAME_END()                                                               \
    }                                                                                    \
    calls;                                                                               \
    }                                                                                    \
    SAC_fp_frame;

#define SAC_FP_FRAME_FUNC_START() struct {
#define SAC_FP_FRAME_FUNC_END(name)                                                      \
    }                                                                                    \
    name;

// TODO: use fp_gen.h.m4 to create different versions of livevar and result

#define SAC_FP_FRAME_LIVEVAR(type, name) type name;
#define SAC_FP_FRAME_SYNC(name)                                                          \
    int name##_ready;                                                                    \
    int name##_waiting;

#define SAC_FP_FRAME_RESULT(type, n) type res_##n;

/*
 * Definitions for setting up a task frame
 */

#define SAC_FP_SETUP_FAST(clone)                                                         \
    int _fp_dqstate;                                                                     \
    SAC_fp_frame *_fp_frame = malloc (sizeof (SAC_fp_frame));                            \
    _fp_frame->slow_clone = &clone;                                                      \
    _fp_frame->done = 0;                                                                 \
    _fp_frame->dqstate = 0;

#define SAC_FP_SETUP_SLOW_START()                                                        \
    int _fp_dqstate;                                                                     \
    switch (_fp_frame->dqstate) {
#define SAC_FP_CASE(n)                                                                   \
    case n:                                                                              \
        goto label##n;
#define SAC_FP_SETUP_SLOW_END()                                                          \
    }                                                                                    \
    label0:;

#define SAC_FP_SET_LIVEVAR(func, name) _fp_frame->calls.func.name = name;
#define SAC_FP_GET_LIVEVAR(func, name) name = _fp_frame->calls.func.name;

/*
 * Definitions for spawning and syncing
 */

#define SAC_FP_SPAWN_START(point)                                                        \
    _fp_frame->entry = point;                                                            \
    _fp_dqstate = _fp_frame->dqstate;                                                    \
    _fp_pushframe (_fp_frame);

#define SAC_FP_SPAWN_END_FAST(function, sync, point)                                     \
    _fp_frame->calls.function.sync##_ready = 1;                                          \
    if (_fp_dqstate != _fp_frame->dqstate) {                                             \
        if (_fp_frame->calls.function.sync##_waiting) {                                  \
            _fp_frame->entry = point;                                                    \
            _fp_pushframe (_fp_frame);                                                   \
        }                                                                                \
        _fp_saveframe (_fp_frame);                                                       \
        return;                                                                          \
    }                                                                                    \
    _fp_popframe ();

#define SAC_FP_SPAWN_END_SLOW(function, sync, point, jump)                               \
    _fp_frame->calls.function.sync##_ready = 1;                                          \
    if (_fp_dqstate != _fp_frame->dqstate) {                                             \
        if (_fp_frame->calls.function.sync##_waiting) {                                  \
            _fp_frame->entry = point; /* TODO: jump */                                   \
            _fp_pushframe (_fp_frame);                                                   \
        }                                                                                \
        _fp_saveframe (_fp_frame);                                                       \
        return;                                                                          \
    }                                                                                    \
    _fp_popframe ();                                                                     \
    label##entry:;

#define SAC_FP_SYNC_START(function, sync, entry)                                         \
    if (!_fp_frame->calls.function.sync##_ready) {                                       \
        _fp_frame->calls.function.sync##_waiting = 1;                                    \
        _fp_saveframe (_fp_frame);                                                       \
        return;                                                                          \
    }                                                                                    \
    if (0) {                                                                             \
        label##entry:;

#define SAC_FP_SYNC_END() }

/*
 * Definitions for other things
 */

#define SAC_FP_SAVE_RESULT__NOOP(function, n, var_NT) SAC_NOOP ()

#define SAC_FP_SAVE_RESULT__NODESC(function, n, var_NT)                                  \
    _fp_frame->calls.function.res_##n = SAC_ND_A_FIELD (var_NT);

#define SAC_FP_RETURN()                                                                  \
    _fp_frame->done = 1;                                                                 \
    _fp_saveframe (_fp_frame);

#define SAC_FP_AP_CHECK_START(name)                                                      \
    SAC_fp_frame *f_##name = _fp_getframe ();                                            \
    while (!f_##name->done) {                                                            \
        _fp_doonething (-1);                                                             \
    }

// TODO: call right free()
#define SAC_FP_AP_CHECK_END(name) free (f_##name);

#define SAC_FP_GET_RESULT__NOOP(frame, function, n, var_NT) SAC_NOOP ()

#define SAC_FP_GET_RESULT__NODESC(frame, function, n, var_NT)                            \
    SAC_ND_A_FIELD (var_NT) = f_##frame->calls.function.res_##n;

/*****************************************************************************/

#endif /* SAC_DO_FP */

#endif /* _SAC_FP_H_ */
