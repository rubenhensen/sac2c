/*
 *
 * $Log$
 * Revision 1.3  1998/05/04 09:35:37  dkr
 * fixed a bug in BLOCK_LOOP_BEGIN...
 *
 * Revision 1.2  1998/05/03 13:10:07  dkr
 * added macros
 *
 * Revision 1.1  1998/05/02 17:49:52  dkr
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_icm_wl.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *****************************************************************************/

#ifndef _sac_icm_wl_h

#define _sac_icm_wl_h

/*
 * Macros used for compilation of with-loop:
 */

#if 1
#define MT_SPMD_BLOCK(a, b, c, d) /* temporary needed */
#endif

#define VAR(type, level, idx_scalar) __##type##level##_##idx_scalar

#define MIN(x, y) ND_MIN (x, y)

#define WL_BLOCK_LOOP0_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,        \
                             bound2, step)                                               \
    {                                                                                    \
        int VAR (stop, level, idx_scalar) = MIN (bound2, __stop_##idx_scalar);           \
        for (idx_scalar = bound1; idx_scalar < VAR (stop, level, idx_scalar);) {         \
            int VAR (start, next_level, idx_scalar) = idx_scalar;                        \
            int VAR (stop, next_level, idx_scalar)                                       \
              = MIN (VAR (start, next_level, idx_scalar) + step, __stop_##idx_scalar);

#define WL_BLOCK_LOOP_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1, bound2, \
                            step)                                                        \
    {                                                                                    \
        for (idx_scalar = VAR (start, level, idx_scalar);                                \
             idx_scalar < VAR (stop, level, idx_scalar);) {                              \
            int VAR (start, next_level, idx_scalar) = idx_scalar;                        \
            int VAR (stop, next_level, idx_scalar)                                       \
              = MIN (VAR (start, next_level, idx_scalar) + step, __stop_##idx_scalar);

#define WL_BLOCK_LOOP_END(level, next_level, dim, idx_vec, idx_scalar, bound1, bound2,   \
                          step)                                                          \
    }                                                                                    \
    }

#define WL_UBLOCK_LOOP0_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,       \
                              bound2, step)                                              \
    {                                                                                    \
        int VAR (stop, level, idx_scalar) = MIN (bound2, __stop_##idx_scalar);           \
        for (idx_scalar = bound1; idx_scalar < VAR (stop, level, idx_scalar);) {         \
            int VAR (start, next_level, idx_scalar) = idx_scalar;

#define WL_UBLOCK_LOOP_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,        \
                             bound2, step)                                               \
    {                                                                                    \
        for (idx_scalar = VAR (start, level, idx_scalar);                                \
             idx_scalar < VAR (stop, level, idx_scalar);) {                              \
            int VAR (start, next_level, idx_scalar) = idx_scalar;

#define WL_UBLOCK_LOOP_END(level, next_level, dim, idx_vec, idx_scalar, bound1, bound2,  \
                           step)                                                         \
    }                                                                                    \
    }

#define WL_STRIDE_LOOP0_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,       \
                              bound2, step)                                              \
    {                                                                                    \
        int VAR (stop, level, idx_scalar) = MIN (bound2, __stop_##idx_scalar);           \
        for (idx_scalar = bound1; idx_scalar < VAR (stop, level, idx_scalar);) {

#define WL_STRIDE_LOOP_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,        \
                             bound2, step)                                               \
    {                                                                                    \
        for (idx_scalar = VAR (start, level, idx_scalar);                                \
             idx_scalar < VAR (stop, level, idx_scalar);) {

#define WL_STRIDE_LOOP_END(level, next_level, dim, idx_vec, idx_scalar, bound1, bound2,  \
                           step)                                                         \
    }                                                                                    \
    }

#define WL_GRID_LOOP_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1, bound2)  \
    {                                                                                    \
        int grid_##idx_scalar = idx_scalar + bound2 - bound1;                            \
        for (; idx_scalar < grid_##idx_scalar; idx_scalar++) {

#define WL_GRID_LOOP_END(level, next_level, dim, idx_vec, idx_scalar, bound1, bound2)    \
    }                                                                                    \
    }

#define WL_ASSIGN(wl_id, idx_vec, idx_scalar, offset, expr)                              \
    wl_id[offset] = expr;                                                                \
    offset++;

#endif /* _sac_icm_wl_h */
