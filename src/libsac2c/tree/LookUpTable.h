/*
 *
 *  Look-Up-Table (LUT) for Pointers and Strings
 *
 *  See LookUpTable.c for documentation.
 */

#ifndef _SAC_LOOKUPTABLE_H_
#define _SAC_LOOKUPTABLE_H_

#include "types.h"

/******************************************************************************
 *
 * Look up table
 *
 * Prefix: LUT
 *
 *****************************************************************************/
extern lut_t *LUTgenerateLut (void);
extern lut_t *LUTduplicateLut (lut_t *lut);
extern lut_t *LUTremoveContentLut (lut_t *lut);
extern lut_t *LUTremoveLut (lut_t *lut);
extern void LUTtouchContentLut (lut_t *lut, info *arg_info);
extern void LUTtouchLut (lut_t *lut, info *arg_info);

extern bool LUTisEmptyLut (lut_t *lut);

extern void LUTprintLut (FILE *handle, lut_t *lut);

extern void **LUTsearchInLutP (lut_t *lut, void *old_item);
extern void **LUTsearchInLutS (lut_t *lut, char *old_item);

extern void **LUTsearchInLutNextP (void);
extern void **LUTsearchInLutNextS (void);

extern void *LUTsearchInLutPp (lut_t *lut, void *old_item);
extern char *LUTsearchInLutSs (lut_t *lut, char *old_item);

extern lut_t *LUTinsertIntoLutP (lut_t *lut, void *old_item, void *new_item);
extern lut_t *LUTinsertIntoLutS (lut_t *lut, char *old_item, void *new_item);

extern lut_t *LUTupdateLutP (lut_t *lut, void *old_item, void *new_item,
                             void **found_item);
extern lut_t *LUTupdateLutS (lut_t *lut, char *old_item, void *new_item,
                             void **found_item);

extern lut_t *LUTmapLutS (lut_t *lut, void *(*fun) (void *, void *));
extern lut_t *LUTmapLutP (lut_t *lut, void *(*fun) (void *, void *));
extern void *LUTfoldLutS (lut_t *lut, void *init, void *(*fun) (void *, void *, void *));
extern void *LUTfoldLutP (lut_t *lut, void *init, void *(*fun) (void *, void *, void *));

#endif /* _SAC_LOOKUPTABLE_H_ */
