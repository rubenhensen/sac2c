/*
 *
 * $Log$
 * Revision 1.6  1998/05/15 23:55:40  dkr
 * added SAC_WL_GRIDVAR_LOOP_... macros
 * fixed some minor bugs
 *
 * Revision 1.5  1998/05/14 21:37:20  dkr
 * removed WL_NOOP
 *
 * Revision 1.4  1998/05/12 22:46:54  dkr
 * added SAC_WL_NOOP
 *
 * Revision 1.3  1998/05/08 00:50:25  dkr
 * added macro WL_GRID_SET_IDX
 *
 * Revision 1.2  1998/05/07 16:21:03  dkr
 * new name conventions
 *
 * Revision 1.1  1998/05/07 08:38:05  cg
 * Initial revision
 *
 * Revision 1.5  1998/05/04 16:21:11  dkr
 * removed MT_SPMD_BLOCK macro (temporary used only)
 *
 * Revision 1.4  1998/05/04 15:36:06  dkr
 * WL_ASSIGN is now a C-ICM
 *
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
 * file:   sac_wl.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *****************************************************************************/

#ifndef _sac_wl_h

#define _sac_wl_h

/*
 * Macros used for compilation of with-loop:
 */

#define VAR(type, level, idx_scalar) __##type##level##_##idx_scalar

#define MIN(x, y) SAC_ND_MIN (x, y)

#define MAX(x, y) SAC_ND_MAX (x, y)

#define SAC_WL_BLOCK_LOOP0_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,    \
                                 bound2, step)                                           \
    {                                                                                    \
        int VAR (start, level, idx_scalar) = MAX (bound1, __start_##idx_scalar);         \
        int VAR (stop, level, idx_scalar) = MIN (bound2, __stop_##idx_scalar);           \
        for (idx_scalar = VAR (start, level, idx_scalar);                                \
             idx_scalar < VAR (stop, level, idx_scalar);) {                              \
            int VAR (start, next_level, idx_scalar) = idx_scalar;                        \
            int VAR (stop, next_level, idx_scalar)                                       \
              = MIN (VAR (start, next_level, idx_scalar) + step, __stop_##idx_scalar);

#define SAC_WL_BLOCK_LOOP_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,     \
                                bound2, step)                                            \
    {                                                                                    \
        for (idx_scalar = VAR (start, level, idx_scalar);                                \
             idx_scalar < VAR (stop, level, idx_scalar);) {                              \
            int VAR (start, next_level, idx_scalar) = idx_scalar;                        \
            int VAR (stop, next_level, idx_scalar)                                       \
              = MIN (VAR (start, next_level, idx_scalar) + step, __stop_##idx_scalar);

#define SAC_WL_BLOCK_LOOP_END(level, next_level, dim, idx_vec, idx_scalar, bound1,       \
                              bound2, step)                                              \
    }                                                                                    \
    }

#define SAC_WL_UBLOCK_LOOP0_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,   \
                                  bound2, step)                                          \
    {                                                                                    \
        int VAR (start, level, idx_scalar) = MAX (bound1, __start_##idx_scalar);         \
        int VAR (stop, level, idx_scalar) = MIN (bound2, __stop_##idx_scalar);           \
        for (idx_scalar = VAR (start, level, idx_scalar);                                \
             idx_scalar < VAR (stop, level, idx_scalar);) {                              \
            int VAR (start, next_level, idx_scalar) = idx_scalar;

#define SAC_WL_UBLOCK_LOOP_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,    \
                                 bound2, step)                                           \
    {                                                                                    \
        for (idx_scalar = VAR (start, level, idx_scalar);                                \
             idx_scalar < VAR (stop, level, idx_scalar);) {                              \
            int VAR (start, next_level, idx_scalar) = idx_scalar;

#define SAC_WL_UBLOCK_LOOP_END(level, next_level, dim, idx_vec, idx_scalar, bound1,      \
                               bound2, step)                                             \
    }                                                                                    \
    }

#define SAC_WL_STRIDE_LOOP0_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,   \
                                  bound2, step)                                          \
    {                                                                                    \
        int VAR (start, level, idx_scalar) = MAX (bound1, __start_##idx_scalar);         \
        int VAR (stop, level, idx_scalar) = MIN (bound2, __stop_##idx_scalar);           \
        for (idx_scalar = VAR (start, level, idx_scalar);                                \
             idx_scalar < VAR (stop, level, idx_scalar);) {

#define SAC_WL_STRIDE_LOOP_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,    \
                                 bound2, step)                                           \
    {                                                                                    \
        for (idx_scalar = VAR (start, level, idx_scalar);                                \
             idx_scalar < VAR (stop, level, idx_scalar);) {

#define SAC_WL_STRIDE_LOOP_END(level, next_level, dim, idx_vec, idx_scalar, bound1,      \
                               bound2, step)                                             \
    }                                                                                    \
    }

#define SAC_WL_GRID_LOOP_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,      \
                               bound2)                                                   \
    {                                                                                    \
        int grid_##idx_scalar = idx_scalar + bound2 - bound1;                            \
        for (; idx_scalar < grid_##idx_scalar; idx_scalar++) {

#define SAC_WL_GRID_SET_IDX(level, next_level, dim, idx_vec, idx_scalar, bound1, bound2) \
    SAC_ND_A_FIELD (idx_vec)[dim] = idx_scalar;

#define SAC_WL_GRID_LOOP_END(level, next_level, dim, idx_vec, idx_scalar, bound1,        \
                             bound2)                                                     \
    }                                                                                    \
    }

#define SAC_WL_GRIDVAR_LOOP_BEGIN(level, next_level, dim, idx_vec, idx_scalar, bound1,   \
                                  bound2)                                                \
    {                                                                                    \
        int grid_##idx_scalar                                                            \
          = MIN (idx_scalar + bound2 - bound1, VAR (stop, level, idx_scalar));           \
        for (; idx_scalar < grid_##idx_scalar; idx_scalar++) {

#define SAC_WL_GRIDVAR_LOOP_END(level, next_level, dim, idx_vec, idx_scalar, bound1,     \
                                bound2)                                                  \
    }                                                                                    \
    }

#endif /* _sac_wl_h */
