/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:54  sacbase
 * new release made
 *
 * Revision 1.2  1998/05/07 08:17:51  cg
 * SAC header files converted to new naming conventions.
 *
 * Revision 1.1  1998/03/19 16:54:34  cg
 * Initial revision
 *
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

#ifndef SAC_MISC_H

#define SAC_MISC_H

extern void SAC_String2Array (char *array, const char *string);

/*
 * Macros used for compilation of do-loop:
 */

#define SAC_ND_GOTO(label) goto label;
#define SAC_ND_LABEL(label)                                                              \
    label:

/*
 * Macro for typedefs of arrays:
 */

#define SAC_ND_TYPEDEF_ARRAY(basetype, name) typedef basetype name;

#endif /* SAC_MISC_H */
