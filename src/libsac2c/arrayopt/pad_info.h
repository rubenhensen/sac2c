/*
 * $Log$
 * Revision 3.2  2004/11/22 16:34:41  sbs
 * SacDevCamp04
 *
 * Revision 3.1  2000/11/20 18:01:53  sacbase
 * new release made
 *
 * Revision 1.9  2000/10/27 13:24:56  cg
 * Added functions PInoteResults() and PIpaddingOverhead().
 *
 * Revision 1.8  2000/08/03 15:35:24  mab
 * completed implementation of data structure
 *
 * Revision 1.7  2000/07/21 14:40:50  mab
 * added PIlinearizeAccessVector, PIgetArrayType*, PIgetPatternShape
 *
 * Revision 1.6  2000/07/19 12:42:55  mab
 * added data structures for storing access patterns and unsupported shapes
 * added functions for accessing new data structures
 * pad_info.[ch] seems to be complete now (may be it needs some debugging)
 *
 * Revision 1.5  2000/07/06 12:44:25  mab
 * added PIgetOldTypes
 *
 * Revision 1.4  2000/07/05 09:13:10  mab
 * fixed problem with global data structure pad_info
 *
 * Revision 1.3  2000/06/28 10:43:10  mab
 * made some code modifications according to code review
 *
 * Revision 1.2  2000/06/14 10:45:04  mab
 * added methods for accessing data structure
 *
 * Revision 1.1  2000/06/08 11:20:16  mab
 * Initial revision
 *
 */

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

/* used in pad.c */
extern void PIinit ();
extern void PIfree ();

/* used in pad_collect.c */
extern pattern_t *PIconcatPatterns (pattern_t *pattern, shpseg *shape);
extern void PIaddAccessPattern (simpletype type, int dim, shpseg *shape, shpseg *group,
                                accessdir_t direction, pattern_t *patterns);
extern void PIprintShpSeg (int dim, shpseg *shape);
extern void PIprintArrayTypeElement (array_type_t *at_ptr);
extern void PIprintConflictGroupElement (array_type_t *at_ptr, conflict_group_t *cg_ptr);
extern void PIprintPatternElement (array_type_t *at_ptr, pattern_t *pt_ptr);
extern void PIprintAccessPatterns ();

extern bool PIaddUnsupportedShape (types *array_type);
extern bool PIisUnsupportedShape (types *array_type);
extern void PIprintUnsupportedShapes ();

extern void PItidyAccessPattern ();
extern void PIremoveUnsupportedShapes ();

/* used in pad_infer.c */
extern int PIlinearizeVector (int dim, shpseg *shape, shpseg *vect);
extern int PIgetArrayTypeDim (array_type_t *at_ptr);
extern shpseg *PIgetArrayTypeShape (array_type_t *at_ptr);
extern simpletype PIgetArrayTypeBasetype (array_type_t *at_ptr);
extern shpseg *PIgetPatternShape (pattern_t *pt_ptr);
extern array_type_t *PIgetFirstArrayType ();
extern array_type_t *PIgetNextArrayType (array_type_t *at_ptr);
extern conflict_group_t *PIgetFirstConflictGroup (array_type_t *at_ptr);
extern conflict_group_t *PIgetNextConflictGroup (conflict_group_t *cg_ptr);
extern pattern_t *PIgetFirstPattern (conflict_group_t *cg_ptr);
extern pattern_t *PIgetNextPattern (pattern_t *pt_ptr);
extern void PIaddInferredShape (simpletype type, int dim, shpseg *old_shape,
                                shpseg *new_shape, shpseg *padding);
extern int PIpaddingOverhead (int dim, shpseg *shape, shpseg *padding);
extern void PInoteResults ();

/* used in pad_transform.c */
extern void PIprintPadInfo ();
extern types *PIgetNewType (types *old_type);
extern types *PIgetOldType (types *old_type);
extern node *PIgetFundefPad (types *old_type);
extern node *PIgetFundefUnpad (types *old_type);

#endif /* _SAC_PAD_INFO_H_ */
