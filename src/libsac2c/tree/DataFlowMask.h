/*****************************************************************************
 *
 * file:   DataFlowMask.h
 *
 * prefix: DFM
 *
 * description:
 *
 *   A data flow mask (DFM) is a bit vector that has as many elements
 *   as the fundef has N_vardec and N_arg entries, and may be
 *   thought of as a compression vector for that entire set
 *   of names. A typical use of a DFM is to mark the set
 *   of variables that are inputs to a block of code, such as
 *   a WL.
 *
 *   Data flow masks are a storage-efficient implementation of local
 *   (within one function) variable sets. Data flow masks are created
 *   relative to a Fundef-specific mask base that defines the mappings
 *   between bits in the masks and actual parameters and local variables
 *   in the function definition. Thus, whenever the underlying set of
 *   variables changes, the mask base and hence all masks must be
 *   re-computed.
 *
 *   While the name clearly hints on their original purpose, data flow
 *   masks can be used anywhere sets of local identifiers are needed.
 *   Their use is not confined to attaching data flow masks to the abstract
 *   syntax tree, but they may likewise be used as auxiliary data within
 *   some transformation. This kind of use also solves the problem of
 *   re-computing masks stored within the AST when new variables are
 *   added or existing ones are removed from a function definition.
 *
 *   This is the header file of a module providing sophisticated support
 *   for binary data flow masks. These data flow masks consist of a mask
 *   data base which has to be generated for each function individually
 *   in advance and of single masks which may be assigned to arbitrary
 *   nodes, usually those of type N_assign or N_block respectively.
 *   Therefore, two abstract data types are provided for mask data bases
 *   (DFMmask_base_t) and for single masks (DFMmask_t).
 *
 *   A new mask data base may be generated by applying the function
 *   DFMgenMaskBase() to a function's parameter list and local variable
 *   declarations, thereby storing the required data in an internal format
 *   suitable for further processing. An existing mask data base may be
 *   updated when additional parameters or local variables have been newly
 *   introduced or removed by applying the function DFMUpdateMaskBase() to the
 *   perhaps modified representations of parameters and local variables as well
 *   as the existing mask data base. Each data flow mask that belongs to the
 *   updated mask data base will be updated automatically.
 *
 *   Special update functions are provided for two particular situations.
 *   DFMUpdateMaskBaseAfterRenaming() is used during precompilation. Here,
 *   the variable names may change but their declaration nodes are otherwise
 *   untouched. Additionally variable declarations may be removed but no
 *   new declarations may occur. DFMUpdateMaskBaseAfterCompiling() may be
 *   used after having compiled variable declarations. In this situation, the
 *   number of variable declarations remains unchanged as well as the names
 *   of identifiers. Only the representation of the declaration may have been
 *   switched to an ICM.
 *
 *   An existing mask data base may also be removed by a call to the function
 *   DFMremoveMaskBase() which always returns the NULL pointer.
 *
 *   There are 5 ways to generate a new data flow mask. This can either be
 *   done from scratch or based on one or two existing masks. In the first
 *   case a mask data base must be provided to a call to one of the functions
 *   DFMgenMaskClear() or DFMgenMaskSet() which generate a new mask from scratch
 *   whose mask entries are either all cleared or all set. In the second
 *   case, an existing mask may either be copied or inverted or two existing
 *   masks are combined into a single new one by a binary operator.
 *
 *   Analogously to the genMask family of functions the setMask family of
 *   functions modifies the first or only given mask rather than generating
 *   a new mask.
 *
 *   The function DFMtestMask() counts how many bits in the given mask are set.
 *   The functions DFMtest2Masks() and DFMtest3Masks() count how many bits
 *   are set at the same positions in every mask (and-combination).
 *   The function DFMnumIds() returns the maximum number of possible ids in a mask.
 *
 *   The function DFMremoveMask() de-allocates storage for a single data flow
 *   mask. It always returns the NULL pointer.
 *
 *   The function DFMprintMask() prints a list of all those
 *   identifiers whose bit in the data flow mask is set. An open output
 *   stream must be provided as first argument. The second argument must be
 *   custom format string which contains exactly one conversion specifier '%s'.
 *
 *   The functions DFMsetMaskEntry() and DFMClearMaskEntry() respectively set or
 *   clear the data flow mask bit assigned to a particular identifier whereas
 *   DFMtestMaskEntry() tests whether this bit is set or not. The identifier
 *   to be set, cleared, or tested may either be specified directly or as a
 *   pointer to its declaration (N_arg or N_vardec node). The latter option
 *   provides more efficiency in accessing the data flow masks. The unused
 *   argument has to be NULL.
 *
 *   The functions DFMGetMaskEntryXXXXSet() and DFMGetMaskEntryXXXXClear() provide
 *   access to local identifiers depending on their state in the data flow
 *   mask specified. These functions work similarly to the C library function
 *   strtok(). When calling one of these functions for the first time, a data
 *   flow mask has to be specified. The first local identifier whose bit is
 *   set or cleared respectively is returned by DFMGetMaskEntryNameSet() or
 *   DFMGetMaskEntryNameClear(). A pointer to the respective declaration node
 *   is returned by DFMGetMaskEntryDeclSet() or DFMGetMaskEntryDeclClear().
 *   On subsequent calls the value
 *   NULL can be given as an argument to extract further identifiers from
 *   the mask. The mask itself as well as the state of its traversal are
 *   stored internally by these functions.
 *
 *   The data flow masks may also be used to get a reference to a variable's
 *   declaration, using the function DFMVar2Decl().
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
extern dfmask_base_t *DFMupdateMaskBaseAfterRenaming (dfmask_base_t *mask_base,
                                                      node *arguments, node *vardecs);
extern dfmask_base_t *DFMupdateMaskBaseAfterCompiling (dfmask_base_t *mask_base,
                                                       node *arguments, node *vardecs);
extern dfmask_base_t *DFMremoveMaskBase (dfmask_base_t *mask_base);
extern void DFMtouchMaskBase (dfmask_base_t *mask_base, info *arg_info);

extern dfmask_base_t *DFMgetMaskBase (dfmask_t *mask);

extern dfmask_t *DFMgenMaskClear (dfmask_base_t *mask_base);
extern dfmask_t *DFMgenMaskSet (dfmask_base_t *mask_base);
extern dfmask_t *DFMgenMaskCopy (dfmask_t *mask);
extern dfmask_t *DFMgenMaskAnd (dfmask_t *mask1, dfmask_t *mask2);
extern dfmask_t *DFMgenMaskOr (dfmask_t *mask1, dfmask_t *mask2);
extern dfmask_t *DFMgenMaskMinus (dfmask_t *mask1, dfmask_t *mask2);
extern dfmask_t *DFMgenMaskInv (dfmask_t *mask);

extern void DFMsetMaskClear (dfmask_t *mask);
extern void DFMsetMaskSet (dfmask_t *mask);
extern void DFMsetMaskCopy (dfmask_t *mask, dfmask_t *mask2);
extern void DFMsetMaskAnd (dfmask_t *mask, dfmask_t *mask2);
extern void DFMsetMaskOr (dfmask_t *mask, dfmask_t *mask2);
extern void DFMsetMaskMinus (dfmask_t *mask, dfmask_t *mask2);
extern void DFMsetMaskInv (dfmask_t *mask);

extern size_t DFMnumIds (dfmask_base_t *mask);
extern size_t DFMnumBitFields (dfmask_base_t *mask);
extern const char *DFMgetId (dfmask_base_t *mask, size_t num);
extern bool DFMtest2MaskBases (dfmask_t *mask1, dfmask_t *mask2);
extern int DFMtestMask (dfmask_t *mask);
extern int DFMtest2Masks (dfmask_t *mask1, dfmask_t *mask2);
extern int DFMtest3Masks (dfmask_t *mask1, dfmask_t *mask2, dfmask_t *mask3);

extern dfmask_t *DFMremoveMask (dfmask_t *mask);
extern void DFMtouchMask (dfmask_t *mask, info *arg_info);

extern void DFMprintMaskNums (FILE *handle, dfmask_t *mask);
extern void DFMprintMask (FILE *handle, const char *format, dfmask_t *mask);
extern void DFMprintMaskDetailed (FILE *handle, dfmask_t *mask);

extern void DFMsetMaskEntryClear (dfmask_t *mask, const char *id, node *avis);
extern void DFMsetMaskEntrySet (dfmask_t *mask, const char *id, node *avis);
extern bool DFMtestMaskEntry (dfmask_t *mask, const char *id, node *avis);

extern char *DFMgetMaskEntryNameSet (dfmask_t *mask);
extern char *DFMgetMaskEntryNameClear (dfmask_t *mask);
extern node *DFMgetMaskEntryDeclSet (dfmask_t *mask);
extern node *DFMgetMaskEntryDeclClear (dfmask_t *mask);
extern node *DFMgetMaskEntryAvisSet (dfmask_t *mask);
extern node *DFMgetMaskEntryAvisClear (dfmask_t *mask);

#endif /* _SAC_DATAFLOWMASK_H_ */
