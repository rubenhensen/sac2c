/*****************************************************************************
 *
 * file:   DataFlowMask.h
 *
 * prefix: DFM
 *
 * description:
 *
 *   Data Flow Masks are boolean vectors with one bit for each variable or
 *   argument (here: decl node) used in a function. They can be used to
 *   represent boolean properties of all identifiers, or as an identification
 *   of subsets.
 *   The mask-bits can be manipulated individually or through set-combining 
 *   operations:
 *   The operations are named differently than what one might be used to:
 *   - Union (mask1, mask2)                 -> DFMgenMaskOr (mask1, mask2)
 *   - Intersection (mask1, mask2)          -> DFMgenMaskAnd (mask1, mask2)
 *   - Difference (mask1, mask2)            -> DFMgenMaskMinus (mask1, mask2)
 *   - SymmetricDifference (mask1, mask2)   -> DFMgenMaskXor (mask1, mask2)
 *   - IsMember (mask, avis)                -> DFMtestMaskEntry (mask, avis)
 *   - Add (mask, avis)                     -> DFMsetMaskEntrySet (mask, avis)
 *   - Remove (mask, avis)                  -> DFMsetMaskEntryClear (mask, avis)
 *
 *   Each mask is created from a mask base.
 *   From a mask base, multiple masks can be created.
 *   A mask base holds the decl nodes so the masks only have to store the 
 *   boolean vector.
 *
 *   Each decl node can only be part of one mask base.
 *   To this end, a mask base should be created over all decl nodes in a fundef,
 *   or over a disjoint set of nodes, although the latter is not recommended.
 *
 *   The the mask base should be stored in the DFM_BASE attribute of the fundef 
 *   such that nested traversals can reuse the same mask without issue.
 *   It's recommended to create the following compound access macro:
 *   #define INFO_DFM_BASE(arg_info) FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info))
 *
 *   Mask bases, and by extension, masks, can be extended or have declarations
 *   removed. This can be done by calling DFMupdateMaskBase() with an updated
 *   list of arguments and vardecls.
 *   Existing masks will implicitly update when their mask base updates,
 *   extending or removing declarations as necessary. 
 *
 *   Masks and mask bases can be freed by calling DFMremoveMask and 
 *   DFMremoveMaskBase respectively.
 *
 *   Masks can be created in a few ways:
 *   - All bits initialized at as clear (false), or as set (true)
 *   - By copying another mask
 *   - By inverting another mask
 *   - By combining multiple masks into a new mask with an
 *     (inverted) bitwise AND/OR/XOR operation.
 *
 *   Instead of generating a new mask to store the result of the above
 *   operations, the result can also be stored in an existing mask
 *   using the DFMsetMask() family of functions.
 *
 *   The function DFMtestMask() counts how many bits in the given mask are set.
 *   The functions DFMtest2Masks() and DFMtest3Masks() count how many bits
 *   are set at the same positions in every mask.
 *
 *   There are some printing functions dedicated to printing masks and mask
 *   bases for debugging purposes.
 *
 *   The functions DFMsetMaskEntry() and DFMClearMaskEntry() respectively set or
 *   clear the bit assigned to a decl node.
 *   DFMtestMaskEntry() tests whether the bit assigned to a decl node is set.
 *
 *   The function family DFMGetMaskEntry(Decl/Avis)(Set/Clear) iterates over
 *   all declarations or avises that are set or clear, similar to how the
 *   C library functino strtok() allows iterating over a string.
 *   See the definition of DFMgetMaskEntryDeclClear() for an extended
 *   description and example code. 
 *
 *****************************************************************************/

#ifndef _SAC_DATAFLOWMASK_H_
#define _SAC_DATAFLOWMASK_H_

#include "types.h"

/*
 * function declarations
 */

extern dfmask_base_t *DFMgenMaskBase (node *arguments, node *vardecs);
extern dfmask_base_t *DFMupdateMaskBase (dfmask_base_t *mask_base, node *arguments,
                                         node *vardecs);
extern dfmask_base_t *DFMremoveMaskBase (dfmask_base_t *mask_base);
extern void DFMrepairMaskBaseBackrefs (dfmask_base_t *mask_base);
extern void DFMtouchMaskBase (dfmask_base_t *mask_base, info *arg_info);

extern dfmask_base_t *DFMgetMaskBase (dfmask_t *mask);

extern dfmask_t *DFMgenMaskClear (dfmask_base_t *mask_base);
extern dfmask_t *DFMgenMaskSet (dfmask_base_t *mask_base);
extern dfmask_t *DFMgenMaskCopy (dfmask_t *mask);
extern dfmask_t *DFMgenMaskAnd (dfmask_t *mask1, dfmask_t *mask2);
extern dfmask_t *DFMgenMaskOr (dfmask_t *mask1, dfmask_t *mask2);
extern dfmask_t *DFMgenMaskMinus (dfmask_t *mask1, dfmask_t *mask2);
extern dfmask_t *DFMgenMaskXor (dfmask_t *mask1, dfmask_t *mask2);
extern dfmask_t *DFMgenMaskInv (dfmask_t *mask);

extern void DFMsetMaskClear (dfmask_t *mask);
extern void DFMsetMaskSet (dfmask_t *mask);
extern void DFMsetMaskCopy (dfmask_t *mask, dfmask_t *mask2);
extern void DFMsetMaskAnd (dfmask_t *mask, dfmask_t *mask2);
extern void DFMsetMaskOr (dfmask_t *mask, dfmask_t *mask2);
extern void DFMsetMaskMinus (dfmask_t *mask, dfmask_t *mask2);
extern void DFMsetMaskXor (dfmask_t *mask1, dfmask_t *mask2);
extern void DFMsetMaskInv (dfmask_t *mask);

extern size_t DFMnumDecls (dfmask_base_t *mask);
extern size_t DFMnumBitFields (dfmask_base_t *mask);
extern bool DFMtest2MaskBases (dfmask_t *mask1, dfmask_t *mask2);
extern int DFMtestMask (dfmask_t *mask);
extern int DFMtest2Masks (dfmask_t *mask1, dfmask_t *mask2);
extern int DFMtest3Masks (dfmask_t *mask1, dfmask_t *mask2, dfmask_t *mask3);

extern dfmask_t *DFMremoveMask (dfmask_t *mask);
extern void DFMtouchMask (dfmask_t *mask, info *arg_info);

extern void DFMprintMaskNums (FILE *handle, dfmask_t *mask);
extern void DFMprintMask (FILE *handle, const char *format, dfmask_t *mask);
extern void DFMprintMaskBase (FILE *handle, dfmask_base_t *mask_base);
extern void DFMprintMaskDetailed (FILE *handle, dfmask_t *mask);

extern void DFMsetMaskEntryClear (dfmask_t *mask, node *avis);
extern void DFMsetMaskEntrySet (dfmask_t *mask, node *avis);
extern bool DFMtestMaskEntry (dfmask_t *mask, node *avis);

extern node *DFMgetMaskEntryDeclSet (dfmask_t *mask);
extern node *DFMgetMaskEntryDeclClear (dfmask_t *mask);
extern node *DFMgetMaskEntryAvisSet (dfmask_t *mask);
extern node *DFMgetMaskEntryAvisClear (dfmask_t *mask);

#endif /* _SAC_DATAFLOWMASK_H_ */
