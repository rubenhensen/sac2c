/*
 *
 * $Log$
 * Revision 1.8  2005/06/10 17:34:33  sbs
 * Now, SAC_ASSURE_TYPE_LINE is used rather than SAC_ASSURE_TYPE.
 *
 * Revision 1.7  2003/11/10 20:22:56  dkrHH
 * debug output: NT objs are converted into strings correctly now
 *
 * Revision 1.6  2003/09/19 12:26:54  dkr
 * postfixes _nt, _any of varnames renamed into _NT, _ANY
 *
 * Revision 1.5  2002/08/05 18:22:49  dkr
 * SAC_ASSURE_TYPE modified
 *
 * Revision 1.4  2002/07/11 13:37:51  dkr
 * SAC_ASSURE_TYPE is an expression instead of an instruction now
 *
 * Revision 1.3  2002/07/08 20:38:14  dkr
 * fixed a bug in definition of SAC_ASSURE_TYPE
 *
 * Revision 1.2  2002/07/03 14:01:10  dkr
 * header modified
 *
 * Revision 1.1  2002/07/03 13:58:29  dkr
 * Initial revision
 *
 *
 * [ file renamed: sac_boundcheck.h -> sac_runtimecheck.h ]
 *
 *
 * Revision 3.1  2000/11/20 18:02:11  sacbase
 * new release made
 *
 * Revision 2.4  1999/10/25 15:38:08  sbs
 * changed SAC_BC_WRITE and SAC_BC_READ:
 * ( check_bound ? 0 : SAC_RuntimeError(...))    is not correct,
 * since 0 is int and SAC_RuntimeError(...) is void!!!
 * This causes problems on the alpha!!!
 * The new solution is:
 * ( check_bound ? 0 : (SAC_RuntimeError(...), 0))    which is ugly
 * but ANSI compliant and as efficient as the former solution!
 *
 * Revision 2.3  1999/07/06 11:00:57  sbs
 * SAC_DO_BOUNDCHECK changed in SAC_DO_CHECK_BOUNDARY !
 *
 * Revision 2.2  1999/04/12 09:41:44  cg
 * Boundchecking is completely redesigned.
 * Now, any C array access can be checked on demand.
 *
 * Revision 2.1  1999/02/23 12:43:49  sacbase
 * new release made
 *
 * Revision 1.2  1998/05/07 08:17:51  cg
 * SAC header files converted to new naming conventions.
 *
 * Revision 1.1  1998/03/19 16:37:58  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   sac_runtimecheck.h
 *
 * description:
 *   This file is part of the SAC standard header file sac.h.
 *   It provides macros for runtime checks.
 *
 *****************************************************************************/

#ifndef _SAC_RUNTIMECHECK_H_
#define _SAC_RUNTIMECHECK_H_

/******************************************************************************
 *
 * ICMs for dynamic type checks
 * ============================
 *
 * ASSURE_TYPE( cond, message)
 *
 ******************************************************************************/

#if SAC_DO_CHECK_TYPE

#define SAC_ASSURE_TYPE(cond, message)                                                   \
    if (!(cond)) {                                                                       \
        SAC_RuntimeError message;                                                        \
    }
/* yes, in C '0;' is indeed a legal statement 8-)) */

#define SAC_ASSURE_TYPE_LINE(cond, line, message)                                        \
    if (!(cond)) {                                                                       \
        SAC_RuntimeErrorLine (line, message);                                            \
    }

#else /* SAC_DO_CHECK_TYPE */

#define SAC_ASSURE_TYPE(cond, message) /* nothing */

#define SAC_ASSURE_TYPE_LINE(cond, line, message) /* nothing */

#endif /* SAC_DO_CHECK_TYPE */

/******************************************************************************
 *
 * ICMs for boundary checks
 * ========================
 *
 * BC_READ( var_NT, pos)
 * BC_WRITE( var_NT, pos)
 *
 ******************************************************************************
 *
 * REMARK:
 *   The current way of boundcchecking is not yet satisfying.
 *
 *   Bounds are not checked in each dimension, but only accesses
 *   are determined which exceed the memory allocated for the accessed
 *   array. However, doing boundary checking correctly would require
 *   much more effort.
 *
 ******************************************************************************/

#if SAC_DO_CHECK_BOUNDARY

#define SAC_BC_READ(var_NT, pos)                                                         \
    (((pos >= 0) && (pos < SAC_ND_A_SIZE (var_NT)))                                      \
       ? 0                                                                               \
       : (SAC_RuntimeError ("Memory access violation on reading from array %s\n"         \
                            "*** with size %d at index position %d !\n",                 \
                            NT_STR (var_NT), SAC_ND_A_SIZE (var_NT), pos),               \
          0)),

#define SAC_BC_WRITE(var_NT, pos)                                                        \
    (((pos >= 0) && (pos < SAC_ND_A_SIZE (var_NT)))                                      \
       ? 0                                                                               \
       : (SAC_RuntimeError ("Memory access violation on writing into array %s\n"         \
                            "*** with size %d at index position %d !\n",                 \
                            NT_STR (var_NT), SAC_ND_A_SIZE (var_NT), pos),               \
          0)),

#else /* SAC_DO_CHECK_BOUNDARY */

#define SAC_BC_WRITE(var_NT, pos)
#define SAC_BC_READ(var_NT, pos)

#endif /* SAC_DO_CHECK_BOUNDARY */

#endif /* _SAC_RUNTIMECHECK_H_ */
