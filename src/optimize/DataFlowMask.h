/*
 *
 * $Log$
 * Revision 1.7  1998/06/03 14:35:05  cg
 * added function DFMUpdateMaskBaseAfterRenaming for special update
 * during precompiling .
 *
 * Revision 1.6  1998/05/19 08:53:26  cg
 * added strtok() like functions for retrieving variables from masks
 *
 * Revision 1.5  1998/05/07 15:36:04  cg
 * mechanism added that allows general updates of data flow masks,
 * i.e. with newly introduced identifiers as well as old ones removed.
 *
 * Revision 1.4  1998/05/06 21:19:03  dkr
 * corrected signature of DFMSetMaskCopy
 *
 * Revision 1.3  1998/05/06 17:19:34  dkr
 * added DFMGenMaskMinus(), DFMSetMaskMinus()
 *
 * Revision 1.2  1998/05/05 15:54:00  cg
 * first running revision
 *
 * Revision 1.1  1998/05/04 15:53:12  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   DataFlowMask.h
 *
 * prefix: DFM
 *
 * description:
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
 *   DFMGenMaskBase() to a function's parameter list and local variable
 *   declarations, thereby storing the required data in an internal format
 *   suitable for further processing. An existing mask data base may be
 *   updated when additional parameters or local variables have been newly
 *   introduced or removed by applying the function DFMUpdateMaskBase() to the perhaps
 *   modified representations of parameters and local variables as well as
 *   the existing mask data base. Each data flow mask that belongs to the
 *   updated mask data base will be updated automatically.
 *
 *   An existing mask data base may also be removed by a call to the function
 *   DFMRemoveMaskBase() which always returns the NULL pointer.
 *
 *   There are 5 ways to generate a new data flow mask. This can either be
 *   done from scratch or based on one or two existing masks. In the first
 *   case a mask data base must be provided to a call to one of the functions
 *   DFMGenMaskClear() or DFMGenMaskSet() which generate a new mask from scratch
 *   whose mask entries are either all cleared or all set. In the second
 *   case, an existing mask may either be copied or inverted or two existing
 *   masks are combined into a single new one by a binary operator.
 *
 *   Analogously to the GenMask family of functions the SetMask family of
 *   functions modifies the first or only given mask rather than generating
 *   a new mask.
 *
 *   The function DFMTestMask() checks whether any bit in the given mask is set.
 *   The functions DFMTest2Masks() and DFMTest3Masks() check whether any bit is set
 *   in each given mask.
 *
 *   The function DFMRemoveMask() de-allocates storage for a single data flow
 *   mask. It always returns the NULL pointer.
 *
 *   The function DFMPrintMask() prints a list of all those
 *   identifiers whose bit in the data flow mask is set. An open output
 *   stream must be provided as first argument. The second argument must be
 *   custom format string which contains exactly one conversion specifier '%s'.
 *
 *   The functions DFMSetMaskEntry() and DFMClearMaskEntry() respectively set or
 *   clear the data flow mask bit assigned to a particular identifier whereas
 *   DFMTestMaskEntry() tests whether this bit is set or not. The identifier
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
 *   DFMGetMaskEntryNameSet(). A pointer to the respective declaration node
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

#ifndef DATAFLOWMASK_H

#define DATAFLOWMASK_H

#include <stdio.h>
#include "types.h"

/*
 * abstract data types
 */

typedef void *DFMmask_base_t;
typedef void *DFMmask_t;

/*
 * function declarations
 */

extern DFMmask_base_t DFMGenMaskBase (node *arguments, node *vardecs);
extern DFMmask_base_t DFMUpdateMaskBase (DFMmask_base_t mask_base, node *arguments,
                                         node *vardecs);
extern DFMmask_base_t DFMUpdateMaskBaseAfterRenaming (DFMmask_base_t mask_base,
                                                      node *arguments, node *vardecs);
extern DFMmask_base_t DFMRemoveMaskBase (DFMmask_base_t mask_base);

extern DFMmask_t DFMGenMaskClear (DFMmask_base_t mask_base);
extern DFMmask_t DFMGenMaskSet (DFMmask_base_t mask_base);
extern DFMmask_t DFMGenMaskCopy (DFMmask_t mask);
extern DFMmask_t DFMGenMaskAnd (DFMmask_t mask1, DFMmask_t mask2);
extern DFMmask_t DFMGenMaskOr (DFMmask_t mask1, DFMmask_t mask2);
extern DFMmask_t DFMGenMaskMinus (DFMmask_t mask1, DFMmask_t mask2);
extern DFMmask_t DFMGenMaskInv (DFMmask_t mask);

extern void DFMSetMaskClear (DFMmask_t mask);
extern void DFMSetMaskSet (DFMmask_t mask);
extern void DFMSetMaskCopy (DFMmask_t mask, DFMmask_t mask2);
extern void DFMSetMaskAnd (DFMmask_t mask, DFMmask_t mask2);
extern void DFMSetMaskOr (DFMmask_t mask, DFMmask_t mask2);
extern void DFMSetMaskMinus (DFMmask_t mask, DFMmask_t mask2);
extern void DFMSetMaskInv (DFMmask_t mask);

extern int DFMTestMask (DFMmask_t mask);
extern int DFMTest2Masks (DFMmask_t mask1, DFMmask_t mask2);
extern int DFMTest3Masks (DFMmask_t mask1, DFMmask_t mask2, DFMmask_t mask3);

extern DFMmask_t DFMRemoveMask (DFMmask_t mask);

extern void DFMPrintMask (FILE *handle, const char *format, DFMmask_t mask);

extern void DFMSetMaskEntryClear (DFMmask_t mask, char *id, node *decl);
extern void DFMSetMaskEntrySet (DFMmask_t mask, char *id, node *decl);
extern int DFMTestMaskEntry (DFMmask_t mask, char *id, node *decl);

extern char *DFMGetMaskEntryNameSet (DFMmask_t mask);
extern char *DFMGetMaskEntryNameClear (DFMmask_t mask);
extern node *DFMGetMaskEntryDeclSet (DFMmask_t mask);
extern node *DFMGetMaskEntryDeclClear (DFMmask_t mask);

extern node *DFMVar2Decl (DFMmask_t mask, char *var);

#endif /* DATAFLOWMASK_H */
