/*
 *
 * $Log$
 * Revision 3.8  2003/09/22 11:59:36  dkr
 * SAC_ABS added
 *
 * Revision 3.7  2003/09/15 13:02:40  dkr
 * SAC_MIN, SAC_MAX added
 *
 * Revision 3.6  2002/07/03 15:32:41  dkr
 * comments beautyfied
 *
 * Revision 3.5  2002/07/02 14:04:34  dkr
 * SAC_BLOCK_BEGIN, SAC_BLOCK_END added
 *
 * Revision 3.4  2002/06/28 13:22:17  dkr
 * ICM_UNDEF -> SAC_ICM_UNDEF()
 * ICM_NOOP -> SAC_NOOP()
 * ICM_NOTHING -> SAC_NOTHING()
 *
 * Revision 3.3  2002/06/28 12:58:32  dkr
 * ICM_UNDEF, ICM_NOOP, ICM_NOTHING added
 *
 * Revision 3.2  2002/04/30 08:39:34  dkr
 * no changes done
 *
 * Revision 3.1  2000/11/20 18:02:16  sacbase
 * new release made
 *
 * Revision 2.2  2000/09/25 15:14:13  dkr
 * ICM ND_TYPEDEF_ARRAY moved to sac_std.h
 *
 * Revision 2.1  1999/02/23 12:43:54  sacbase
 * new release made
 *
 * Revision 1.2  1998/05/07 08:17:51  cg
 * SAC header files converted to new naming conventions.
 *
 * Revision 1.1  1998/03/19 16:54:34  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   sac_misc.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides external declarations for global variables and functions
 *   defined in libsac_misc.c
 *
 *****************************************************************************/

#ifndef _SAC_MISC_H_
#define _SAC_MISC_H_

extern void SAC_String2Array (char *array, const char *string);

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
    label:
#define SAC_ND_GOTO(label) goto label;

#endif /* _SAC_MISC_H_ */
