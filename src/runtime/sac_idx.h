/*
 *
 * $Log$
 * Revision 3.14  2005/08/24 10:18:16  ktr
 * removed ICM USE_GENVAR_OFFSET
 *
 * Revision 3.13  2004/03/09 23:56:15  dkrHH
 * old backend removed
 *
 * Revision 3.12  2003/09/25 13:44:59  dkr
 * ND_WRITE replaced by ND_WRITE_COPY
 *
 * Revision 3.11  2003/09/19 12:26:40  dkr
 * postfixes _nt, _any of varnames renamed into _NT, _ANY
 *
 * Revision 3.10  2002/07/31 16:34:25  dkr
 * parameter 'copyfun' added for several ICMs
 *
 * Revision 3.9  2002/07/24 15:04:33  dkr
 * SAC_ND_USE_GENVAR_OFFSET modified
 *
 * Revision 3.8  2002/07/24 13:46:20  dkr
 * SAC_ND_USE_GENVAR_OFFSET: argument renamed
 *
 * Revision 3.7  2002/07/16 12:44:20  dkr
 * ICMs ND_PRF_IDX_... moved from sac_prf.h to sac_idx.h
 *
 * Revision 3.6  2002/07/11 09:24:27  dkr
 * all macros deactivated for new backend
 *
 * Revision 3.5  2002/04/30 08:45:57  dkr
 * no changes done
 *
 * Revision 3.4  2001/12/21 13:33:37  dkr
 * ALLOC_ARRAY, CHECK_REUSE ICMs seperated
 * (they no longer occur in other ICMs)
 *
 * Revision 3.3  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.2  2001/01/19 11:57:07  dkr
 * SAC_WL_DEST renamed into SAC_WL_OFFSET
 *
 * Revision 3.1  2000/11/20 18:02:15  sacbase
 * new release made
 *
 * Revision 2.4  2000/07/26 18:20:19  dkr
 * SAC_ND_KS_USE_GENVAR_OFFSET: macro SAC_WL_DEST used instead of
 * ##__destptr
 *
 * Revision 2.3  2000/07/25 13:55:36  dkr
 * superfluous parameter 'line' in ICMs for array-prfs removed
 *
 * Revision 2.2  1999/04/12 09:37:48  cg
 * All accesses to C arrays are now performed through the new ICMs
 * ND_WRITE_ARRAY and ND_READ_ARRAY. This allows for an integration
 * of cache simulation as well as boundary checking.
 *
 * Revision 2.1  1999/02/23 12:43:51  sacbase
 * new release made
 *
 * Revision 1.6  1998/06/29 08:52:19  cg
 * streamlined tracing facilities
 * tracing on new with-loop and multi-threading operations implemented
 *
 * Revision 1.5  1998/05/18 09:43:57  dkr
 * fixed a bug in SAC_ND_IDX_MODARRAY_AxVxA_CHECK_REUSE
 *   'i' renamed to '__i'
 *
 * Revision 1.4  1998/05/07 14:16:02  cg
 * converted to new naming conventions
 *
 * Revision 1.3  1998/05/07 14:10:09  cg
 * converted to new naming conventions
 *
 * Revision 1.2  1998/05/07 11:14:59  cg
 * converted to new naming conventions
 *
 * Revision 1.1  1998/05/07 08:38:05  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   sac_idx.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions of Intermediate Code Macros (ICM)
 *   implemented as real macros.
 *
 *****************************************************************************/

#ifndef _SAC_IDX_H
#define _SAC_IDX_H

/******************************************************************************
 *
 * ICMs for primitive functions after IVE
 * ======================================
 *
 * ND_PRF_IDX_SEL__SHAPE( to_NT, to_sdim, from_NT, from_sdim, idx_ANY, copyfun)
 * ND_PRF_IDX_SEL__DATA( to_NT, to_sdim, from_NT, from_sdim, idx_ANY, copyfun)
 *
 * ND_PRF_IDX_MODARRAY__DATA( to_NT, to_sdim, from_NT, from_sdim, idx, val,
 *                            copyfun)
 *
 * ND_USE_GENVAR_OFFSET( offset, wl)
 *
 ******************************************************************************/

/* ND_PRF_IDX_SEL__SHAPE( ...) is a C-ICM */
/* ND_PRF_IDX_SEL__DATA( ...) is a C-ICM */

/* ND_PRF_IDX_MODARRAY__DATA( ...) is a C-ICM */

#endif /* _SAC_IDX_H */
