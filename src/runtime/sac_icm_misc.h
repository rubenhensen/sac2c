/*
 *
 * $Log$
 * Revision 1.1  1998/03/19 16:39:24  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sac_icm_misc.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *****************************************************************************/

#ifndef SAC_ICM_MISC_H

#define SAC_ICM_MISC_H

/*
 * Macros used for compilation of do-loop:
 */

#define ND_GOTO(label) goto label;
#define ND_LABEL(label)                                                                  \
    label:

/*
 * Macro for typedefs of arrays:
 */

#define ND_TYPEDEF_ARRAY(basetype, name) typedef basetype name;

#endif /* SAC_ICM_MISC_H */
