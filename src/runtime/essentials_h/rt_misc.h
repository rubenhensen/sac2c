/*****************************************************************************
 *
 * file:   rt_misc.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It defines miscellaneous ICMs
 *
 *****************************************************************************/

#ifndef _SAC_RT_MISC_H_
#define _SAC_RT_MISC_H_

/*****************************************************************************
 *
 * Miscellaneous ICMs
 * ==================
 *
 * ICM_UNDEF()   : handling of undefined ICMs
 *
 * NOOP()        : noop ICM
 * NOTHING()     : empty ICM
 *
 * BLOCK_BEGIN() : begin of new block
 * BLOCK_END()   : end of new block
 *
 * MIN( a, b)    : minimum of 'a' and 'b'
 * MAX( a, b)    : maximum of 'a' and 'b'
 * ABS( a)       : (a<0) ? -a : a
 *
 *****************************************************************************/

#define SAC_ICM_UNDEF()                                                                  \
    _ICM_IS_UNDEFINED_ /* CC will report undefined symbol _ICM_IS_UNDEFINED_ */

#define SAC_NOOP() /* noop */;

#define SAC_NOTHING()

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif

#define SAC_BLOCK_BEGIN() {
#define SAC_BLOCK_END() }

#define SAC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SAC_MAX(a, b) ((a) > (b) ? (a) : (b))
#define SAC_ABS(a) (((a) < 0) ? (-(a)) : (a))

/*****************************************************************************
 *
 * ICMs for do-loop
 * ================
 *
 * ND_LABEL( label)
 * ND_GOTO( label)
 *
 *****************************************************************************/

#define SAC_ND_LABEL(label)                                                              \
    label:;
#define SAC_ND_GOTO(label) goto label;

#define SAC_BREAK_ON_GUARD(guard, label)                                                 \
    if (guard)                                                                           \
        goto label;

#endif /* _SAC_RT_MISC_H_ */

