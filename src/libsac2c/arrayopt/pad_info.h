/*****************************************************************************
 *
 * file:   pad_info.h
 *
 * prefix: PI
 *
 * description:
 *
 *   This is an abstract data structure for storing information needed by
 *   the array padding.
 *
 *
 *****************************************************************************/

#ifndef _SAC_PAD_INFO_H_
#define _SAC_PAD_INFO_H_

#include "shape.h"
#include "new_types.h"

/* used in pad.c */
extern void PIinit (void);
extern void PIfree (void);

/* used in pad_collect.c */
extern pattern_t *PIconcatPatterns (pattern_t *pattern, shape *shp);
extern void PIaddAccessPattern (simpletype type, int dim, shape *shp, shape *group,
                                accessdir_t direction, pattern_t *patterns);
extern void PIprintShpSeg (int dim, shape *shp);
extern void PIprintArrayTypeElement (array_type_t *at_ptr);
extern void PIprintConflictGroupElement (array_type_t *at_ptr, conflict_group_t *cg_ptr);
extern void PIprintPatternElement (array_type_t *at_ptr, pattern_t *pt_ptr);
extern void PIprintAccessPatterns (void);

extern bool PIaddUnsupportedShape (ntype *array_type);
extern bool PIisUnsupportedShape (ntype *array_type);
extern void PIprintUnsupportedShapes (void);

extern void PItidyAccessPattern (void);
extern void PIremoveUnsupportedShapes (void);

/* used in pad_infer.c */
extern int PIlinearizeVector (int dim, shape *shp, shape *vect);
extern int PIgetArrayTypeDim (array_type_t *at_ptr);
extern shape *PIgetArrayTypeShape (array_type_t *at_ptr);
extern simpletype PIgetArrayTypeBasetype (array_type_t *at_ptr);
extern shape *PIgetPatternShape (pattern_t *pt_ptr);
extern array_type_t *PIgetFirstArrayType (void);
extern array_type_t *PIgetNextArrayType (array_type_t *at_ptr);
extern conflict_group_t *PIgetFirstConflictGroup (array_type_t *at_ptr);
extern conflict_group_t *PIgetNextConflictGroup (conflict_group_t *cg_ptr);
extern pattern_t *PIgetFirstPattern (conflict_group_t *cg_ptr);
extern pattern_t *PIgetNextPattern (pattern_t *pt_ptr);
extern void PIaddInferredShape (simpletype type, int dim, shape *old_shape,
                                shape *new_shape, shape *padding);
extern int PIpaddingOverhead (int dim, shape *shp, shape *padding);
extern void PInoteResults (void);

/* used in pad_transform.c */
extern void PIprintPadInfo (void);
extern ntype *PIgetNewType (ntype *old_type);
extern ntype *PIgetOldType (ntype *old_type);
extern node *PIgetFundefPad (ntype *old_type);
extern node *PIgetFundefUnpad (ntype *old_type);

#endif /* _SAC_PAD_INFO_H_ */
