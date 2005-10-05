/*
 * $Log$
 * Revision 1.11  2005/10/05 13:27:11  ktr
 * removed common entrypoint for WLI, WLF which is now performed in optimize cycle
 *
 * Revision 1.10  2005/08/26 12:29:51  ktr
 * major brushing, seems to work
 *
 * Revision 1.9  2005/08/19 13:08:30  ktr
 * removed SSAINDEX macro
 *
 * Revision 1.8  2004/11/26 15:51:50  jhb
 * WLFwithloopFoldingWLT changed WLFwithloopFoldingWlt
 *
 * Revision 1.7  2004/11/22 17:29:51  sbs
 * SacDevCamp04
 *
 * Revision 1.6  2004/11/16 16:35:08  mwe
 * code for type upgrade added
 * use ntype-structure instead of type-structure
 * new code deactivated by MWE_NTYPE_READY
 *
 * Revision 1.5  2004/07/14 14:17:36  sah
 * added SSADbugIndexInfo as a replacement for DebugIndexInfo
 * from old WithloopFolding, as that will be gone soon
 *
 * Revision 1.4  2001/05/17 13:29:29  cg
 * De-allocation macros FREE_INTERN_GEN and FREE_INDEX_INFO
 * converted to functions.
 *
 * Revision 1.3  2001/05/16 13:43:08  nmw
 * unused old code removed, comments corrected
 * MALLOC/FREE changed to Malloc/Free
 *
 * Revision 1.2  2001/05/15 16:39:21  nmw
 * SSAWithloopFolding implemented (but not tested)
 *
 * Revision 1.1  2001/05/14 15:55:15  nmw
 * Initial revision
 *
 *
 * created from: WithloopFolding.h, Revision 3.1  on 2001/05/14 by  nmw
 */

#ifndef _SAC_WITHLOOPFOLDING_H_
#define _SAC_WITHLOOPFOLDING_H_

#include "types.h"

/******************************************************************************
 *
 * exported functions
 *
 ******************************************************************************/

/* general functions */
extern int WLFlocateIndexVar (node *idn, node *wln);

extern void WLFarrayST2ArrayInt (node *arrayn, int **iarray, int shape);

/* index_info related functions */
extern index_info *WLFcreateIndex (int vector);
extern index_info *WLFduplicateIndexInfo (index_info *iinfo);
extern index_info *WLFvalidLocalId (node *idn);
extern void WLFdbugIndexInfo (index_info *iinfo);

/* intern_gen related functions */
extern intern_gen *WLFtree2InternGen (node *wln, node *filter);
extern node *WLFinternGen2Tree (node *wln, intern_gen *ig);
extern int WLFnormalizeInternGen (intern_gen *ig);
extern intern_gen *WLFcreateInternGen (int shape, int stepwidth);
extern intern_gen *WLFappendInternGen (intern_gen *, int, node *, int);
extern intern_gen *WLFcopyInternGen (intern_gen *source);
extern intern_gen *WLFfreeInternGen (intern_gen *tmp);
extern intern_gen *WLFfreeInternGenChain (intern_gen *ig);

/******************************************************************************
 *
 * defines
 *
 ******************************************************************************/

/* if not defined, indexes with more than one occurence of an
   index scalar are allowed to be valid transformations, e.g. [i,i,j] */
/* #define TRANSF_TRUE_PERMUTATIONS */

#endif /* _SAC_WITHLOOPFOLDING_H_ */
