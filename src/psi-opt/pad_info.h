/*
 * $Log$
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

#ifndef sac_pad_info_h

#define sac_pad_info_h

/* structure for storing access patterns */
typedef struct PATTERN_T {
    shpseg *pattern;
    struct PATTERN_T *next;
} pattern_t;

/* structure for grouping access patterns by conflict groups */
typedef struct CONFLICT_GROUP_T {
    shpseg *group;
    pattern_t *patterns;
    struct CONFLICT_GROUP_T *next;
} conflict_group_t;

/* strcture for grouping conflict groups by array types */
typedef struct ARRAY_TYPE_T {
    simpletype type;
    int dim;
    shpseg *shape;
    conflict_group_t *groups;
    struct ARRAY_TYPE_T *next;
} array_type_t;

/* structure containing shapes of unsupported operations */
typedef struct UNSUPPORTED_SHAPE_T {
    simpletype type;
    int dim;
    shpseg *shape;
    struct UNSUPPORTED_SHAPE_T *next;
} unsupported_shape_t;

/* structure containing old and infered array shape */
typedef struct PAD_INFO_T {
    simpletype type;
    int dim;
    shpseg *old_shape;
    shpseg *new_shape;
    node *fundef_pad;
    node *fundef_unpad;
    struct PAD_INFO_T *next;
} pad_info_t;

extern void PIinit ();

/* used in pad_collect.c */
extern pattern_t *PIconcatPatterns (pattern_t *pattern, shpseg *shape);
extern void PIaddAccessPattern (simpletype type, int dim, shpseg *shape, shpseg *group,
                                pattern_t *patterns);
extern void PIprintAccessPatterns ();
extern void PIaddUnsupportedShape (types *array_type);
extern void PIprintUnsupportedShapes ();
extern void PItidyAccessPattern ();

/* used in pad_infer.c */
extern int PIlinearizeAccessVector (int dim, shpseg *shape, shpseg *vect);
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
                                shpseg *new_shape);

/* used in pad_transform.c */
extern void PIPrintPadInfo ();
extern types *PIgetNewType (types *old_type);
extern types *PIgetOldType (types *old_type);
extern node *PIgetFundefPad (types *old_type);
extern node *PIgetFundefUnpad (types *old_type);

extern void PIfree ();

#endif /* sac_pad_info_h */
