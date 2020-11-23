#define DBUG_PREFIX "API"
#include "debug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "DupTree.h"
#include "ctinfo.h"
#include "type_utils.h"
#include "str.h"
#include "memory.h"
#include "convert.h"

#include "pad_info.h"
#include "pad.h"

/* access macros for array_type_t */
#define AT_TYPE(p) (p)->type
#define AT_DIM(p) (p)->dim
#define AT_SHAPE(p) (p)->shp
#define AT_GROUPS(p) (p)->groups
#define AT_NEXT(p) (p)->next

/* access macros for conflict_group_t */
#define CG_GROUP(p) (p)->group
#define CG_DIR(p) (p)->direction
#define CG_PATTERNS(p) (p)->patterns
#define CG_NEXT(p) (p)->next

/* access macros for pattern_t */
#define PT_PATTERN(p) (p)->pattern
#define PT_NEXT(p) (p)->next

/* access macros for unsupported_shapes_t */
#define US_TYPE(p) (p)->type
#define US_DIM(p) (p)->dim
#define US_SHAPE(p) (p)->shp
#define US_NEXT(p) (p)->next

/* access_macros for pad_info_t */
#define PI_TYPE(p) (p)->type
#define PI_DIM(p) (p)->dim
#define PI_OLD_SHAPE(p) (p)->old_shape
#define PI_NEW_SHAPE(p) (p)->new_shape
#define PI_PADDING(p) (p)->padding
#define PI_FUNDEF_PAD(p) (p)->fundef_pad
#define PI_FUNDEF_UNPAD(p) (p)->fundef_unpad
#define PI_NEXT(p) (p)->next

/*****************************************************************************
 *
 * file:   pad_info.c
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

static pad_info_t *pad_info;
static array_type_t *array_type;
static unsupported_shape_t *unsupported_shape;

/*****************************************************************************
 *
 * function:
 *   static pad_info_t *GetNewTableEntry(ntype *old_type)
 *
 * description:
 *   returns pointer to pad_info table entry corrsponding to old_type or NULL
 *
 *****************************************************************************/

static pad_info_t *
GetNewTableEntry (ntype *old_type)
{

    pad_info_t *tmp;
    pad_info_t *matching_entry;

    DBUG_ENTER ();

    tmp = pad_info;
    matching_entry = NULL;

    while (tmp != NULL) {
        if ((PI_TYPE (tmp) == TUgetSimpleImplementationType (old_type))
            && (PI_DIM (tmp) == TYgetDim (old_type))
            && SHcompareShapes (PI_OLD_SHAPE (tmp),
                                TYgetShape (old_type))) {

            matching_entry = tmp;
            tmp = NULL;
        } else {
            tmp = PI_NEXT (tmp);
        }
    }

    DBUG_RETURN (matching_entry);
}

/*****************************************************************************
 *
 * function:
 *   static pad_info_t *GetOldTableEntry(ntype* new_type)
 *
 * description:
 *   returns pointer to pad_info table entry corrsponding to new_type or NULL
 *
 *****************************************************************************/

static pad_info_t *
GetOldTableEntry (ntype *new_type)
{

    pad_info_t *tmp;
    pad_info_t *matching_entry;

    DBUG_ENTER ();

    tmp = pad_info;
    matching_entry = NULL;

    while (tmp != NULL) {
        if ((PI_TYPE (tmp) == TUgetSimpleImplementationType (new_type))
            && (PI_DIM (tmp) == TYgetDim (new_type))
            && SHcompareShapes (PI_NEW_SHAPE (tmp),
                                TYgetShape (new_type))) {

            matching_entry = tmp;
            tmp = NULL;
        } else {
            tmp = PI_NEXT (tmp);
        }
    }

    DBUG_RETURN (matching_entry);
}

/*****************************************************************************
 *
 * function:
 *   static array_type_t* GetArrayTypeEntry(simpletype type, int dim, shape* shp)
 *
 * description:
 *   get entry from access_pattern with specified type and class
 *   returns NULL, if no matching entry was found
 *
 *****************************************************************************/

static array_type_t *
GetArrayTypeEntry (simpletype type, int dim, shape *shp)
{

    array_type_t *array_type_ptr;
    bool matched;

    DBUG_ENTER ();

    array_type_ptr = array_type;
    matched = FALSE;

    while ((array_type_ptr != NULL) && (!matched)) {
        if ((dim == AT_DIM (array_type_ptr)) && (type == AT_TYPE (array_type_ptr))
            && (SHcompareShapes (shp, AT_SHAPE (array_type_ptr)))) {
            matched = TRUE;
        } else {
            array_type_ptr = AT_NEXT (array_type_ptr);
        }
    }

    if (matched == FALSE) {
        array_type_ptr = NULL;
    }

    DBUG_RETURN (array_type_ptr);
}

/*****************************************************************************
 *
 * function:
 *   static unsupported_shape_t* GetUnsupportedShapeEntry(simpletype type, int dim,
 *shape* shape)
 *
 * description:
 *   get entry from unsupported_shape with specified type
 *   returns NULL, if no matching entry was found
 *
 *****************************************************************************/

static unsupported_shape_t *
GetUnsupportedShapeEntry (simpletype type, int dim, shape *shape)
{

    unsupported_shape_t *unsupported_shape_ptr;
    bool matched;

    DBUG_ENTER ();

    unsupported_shape_ptr = unsupported_shape;
    matched = FALSE;

    while ((unsupported_shape_ptr != NULL) && (!matched)) {
        if ((dim == US_DIM (unsupported_shape_ptr))
            && (type == US_TYPE (unsupported_shape_ptr))
            && (SHcompareShapes (shape, US_SHAPE (unsupported_shape_ptr)))) {
            matched = TRUE;
        } else {
            unsupported_shape_ptr = US_NEXT (unsupported_shape_ptr);
        }
    }

    if (matched == FALSE) {
        unsupported_shape_ptr = NULL;
    }

    DBUG_RETURN (unsupported_shape_ptr);
}

/*****************************************************************************
 *
 * function:
 *   static pad_info_t* RemovePadInfoElement(pad_info_t* element)
 *
 * description:
 *   remove pad_info_t element from list, return next element (may be NULL)
 *
 *****************************************************************************/

static pad_info_t *
RemovePadInfoElement (pad_info_t *element)
{

    pad_info_t *pi_next_ptr;

    DBUG_ENTER ();

    SHfreeShape (PI_OLD_SHAPE (element));
    SHfreeShape (PI_NEW_SHAPE (element));
    pi_next_ptr = PI_NEXT (element);
    element = MEMfree (element);

    DBUG_RETURN (pi_next_ptr);
}

/*****************************************************************************
 *
 * function:
 *   static unsupported_shape_t* RemoveUnsupportedShapeElement(unsupported_shape_t*
 *element)
 *
 * description:
 *   remove unsupported_shape_t element from list, return next element (may be NULL)
 *
 *****************************************************************************/

static unsupported_shape_t *
RemoveUnsupportedShapeElement (unsupported_shape_t *element)
{

    unsupported_shape_t *us_next_ptr;

    DBUG_ENTER ();

    SHfreeShape (US_SHAPE (element));
    us_next_ptr = US_NEXT (element);
    element = MEMfree (element);

    DBUG_RETURN (us_next_ptr);
}

/*****************************************************************************
 *
 * function:
 *   static array_type_t* RemoveArrayTypeElement(array_type_t* element)
 *
 * description:
 *   remove array_type_t element from list, return next element (may be NULL)
 *
 *****************************************************************************/

static array_type_t *
RemoveArrayTypeElement (array_type_t *element)
{

    array_type_t *at_next_ptr;

    DBUG_ENTER ();

    SHfreeShape (AT_SHAPE (element));
    at_next_ptr = AT_NEXT (element);
    element = MEMfree (element);

    DBUG_RETURN (at_next_ptr);
}

/*****************************************************************************
 *
 * function:
 *   static conflict_group_t* RemoveConflictGroupElement(conflict_group_t*element)
 *
 * description:
 *   remove conflict_group_t element from list, return next element (may be NULL)
 *
 *****************************************************************************/

static conflict_group_t *
RemoveConflictGroupElement (conflict_group_t *element)
{

    conflict_group_t *cg_next_ptr;

    DBUG_ENTER ();

    SHfreeShape (CG_GROUP (element));
    cg_next_ptr = CG_NEXT (element);
    element = MEMfree (element);

    DBUG_RETURN (cg_next_ptr);
}

/*****************************************************************************
 *
 * function:
 *   static pattern_t* RemovePatternElement(pattern_t* element)
 *
 * description:
 *   remove pattern_t element from list, return next element (may be NULL)
 *
 *****************************************************************************/

static pattern_t *
RemovePatternElement (pattern_t *element)
{

    pattern_t *pt_next_ptr;

    DBUG_ENTER ();

    SHfreeShape (PT_PATTERN (element));
    pt_next_ptr = PT_NEXT (element);
    element = MEMfree (element);

    DBUG_RETURN (pt_next_ptr);
}

/*****************************************************************************
 *
 * function:
 *   static void SortAccesses()
 *
 * description:
 *   sort accesses within each conflict group in lexicographical order
 *   uses PIlinearizeAccessVector to determine order
 *
 *****************************************************************************/

static void
SortAccesses (void)
{

    array_type_t *at_ptr;
    conflict_group_t *cg_ptr;
    pattern_t *pt_ptr;
    pattern_t *pt_next_ptr;
    pattern_t *pt_tmp_list;
    pattern_t *pt_tmp_ptr;
    pattern_t *pt_tmp_prv_ptr;
    bool matched;

    DBUG_ENTER ();

    DBUG_PRINT ("  sorting accesses...");
    APprintDiag ("  sorting accesses...\n");

    /* for every array type... */
    at_ptr = array_type;
    while (at_ptr != NULL) {

        /* for all conflict groups... */
        cg_ptr = AT_GROUPS (at_ptr);
        while (cg_ptr != NULL) {

            /* sort sort all accesses... */

            pt_tmp_list = NULL;
            pt_ptr = CG_PATTERNS (cg_ptr);
            while (pt_ptr != NULL) {

                pt_next_ptr = PT_NEXT (pt_ptr);

                /* into temporary list */
                pt_tmp_ptr = pt_tmp_list;
                pt_tmp_prv_ptr = NULL;
                matched = FALSE;
                while (!matched) {

                    /* ascending order */
                    if ((pt_tmp_ptr == NULL)
                        || (PIlinearizeVector (AT_DIM (at_ptr), AT_SHAPE (at_ptr),
                                               PT_PATTERN (pt_ptr))
                            <= PIlinearizeVector (AT_DIM (at_ptr), AT_SHAPE (at_ptr),
                                                  PT_PATTERN (pt_tmp_ptr)))) {
                        if (pt_tmp_prv_ptr == NULL) {
                            /* insert first element of list */
                            PT_NEXT (pt_ptr) = pt_tmp_list;
                            pt_tmp_list = pt_ptr;
                        } else {
                            /* insert other element */
                            PT_NEXT (pt_ptr) = pt_tmp_ptr;
                            PT_NEXT (pt_tmp_prv_ptr) = pt_ptr;
                            /* inserted pt_ptr inbetween pt_tmp_prv_ptr and pt_tmp_ptr */
                        }
                        matched = TRUE;
                    } else {
                        pt_tmp_prv_ptr = pt_tmp_ptr;
                        pt_tmp_ptr = PT_NEXT (pt_tmp_ptr);
                    }
                }

                pt_ptr = pt_next_ptr;
            }
            CG_PATTERNS (cg_ptr) = pt_tmp_list;

            cg_ptr = CG_NEXT (cg_ptr);
        }

        at_ptr = AT_NEXT (at_ptr);
    }

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   static void RemoveSingleAccessPatterns()
 *
 * description:
 *   remove conflict groups containing only one access pattern
 *   attention: RemoveDuplicateAccesses has to be applied first!
 *
 *****************************************************************************/

static void
RemoveSingleAccessPatterns (void)
{

    array_type_t *at_ptr;
    array_type_t *at_prv_ptr;
    conflict_group_t *cg_ptr;
    conflict_group_t *cg_prv_ptr;

    DBUG_ENTER ();

    DBUG_PRINT ("  removing conflict groups with single access patterns...");
    APprintDiag ("  removing conflict groups with single access patterns...\n");

    at_ptr = array_type;
    at_prv_ptr = NULL;

    while (at_ptr != NULL) {

        cg_ptr = AT_GROUPS (at_ptr);
        cg_prv_ptr = NULL;

        while (cg_ptr != NULL) {
            /* check, if only one access pattern */
            if (PT_NEXT (CG_PATTERNS (cg_ptr)) == NULL) {
                CG_PATTERNS (cg_ptr) = RemovePatternElement (CG_PATTERNS (cg_ptr));
                if (cg_prv_ptr == NULL) {
                    /* remove first element from list */
                    AT_GROUPS (at_ptr) = RemoveConflictGroupElement (cg_ptr);
                    cg_ptr = AT_GROUPS (at_ptr);
                } else {
                    /* remove other element */
                    CG_NEXT (cg_prv_ptr) = RemoveConflictGroupElement (cg_ptr);
                    cg_ptr = CG_NEXT (cg_prv_ptr);
                }
            } else {
                cg_prv_ptr = cg_ptr;
                cg_ptr = CG_NEXT (cg_ptr);
            }
        }

        /* check, if current array_type has no conflict groups remaining */
        if (AT_GROUPS (at_ptr) == NULL) {
            if (at_prv_ptr == NULL) {
                /* remove first element from list */
                array_type = RemoveArrayTypeElement (at_ptr);
                at_ptr = array_type;
            } else {
                /* remove other element */
                AT_NEXT (at_prv_ptr) = RemoveArrayTypeElement (at_ptr);
                at_ptr = AT_NEXT (at_prv_ptr);
            }
        } else {
            at_prv_ptr = at_ptr;
            at_ptr = AT_NEXT (at_ptr);
        }
    }

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   static void RemoveDuplicateAccesses()
 *
 * description:
 *   remove duplicate accesses within one conflict group
 *   attention: SortAccesses has to be applied first!
 *
 *****************************************************************************/

static void
RemoveDuplicateAccesses (void)
{

    array_type_t *at_ptr;
    conflict_group_t *cg_ptr;
    pattern_t *pt_ptr;

    DBUG_ENTER ();

    DBUG_PRINT ("  removing duplicate accesses from conflict groups...");
    APprintDiag ("  removing duplicate accesses from conflict groups...\n");

    /* for every array type... */
    at_ptr = array_type;
    while (at_ptr != NULL) {

        /* for all conflict groups... */
        cg_ptr = AT_GROUPS (at_ptr);
        while (cg_ptr != NULL) {

            /* check for duplicate accesses */

            pt_ptr = CG_PATTERNS (cg_ptr);
            while (pt_ptr != NULL) {

                if (PT_NEXT (pt_ptr) != NULL) {
                    if (SHcompareShapes (PT_PATTERN (pt_ptr),
                                         PT_PATTERN (PT_NEXT (pt_ptr)))) {
                        /* remove duplicate */
                        PT_NEXT (pt_ptr) = RemovePatternElement (PT_NEXT (pt_ptr));

                    } else {
                        pt_ptr = PT_NEXT (pt_ptr);
                    }
                } else {
                    pt_ptr = PT_NEXT (pt_ptr);
                }
            }

            cg_ptr = CG_NEXT (cg_ptr);
        }

        at_ptr = AT_NEXT (at_ptr);
    }

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   static void RemoveIdenticalConflictGroups()
 *
 * description:
 *   remove duplicate conflict groups (groups with identical access patterns)
 *   attention: SortAccesses has to be applied first!
 *
 *****************************************************************************/

static void
RemoveIdenticalConflictGroups (void)
{

    array_type_t *at_ptr;
    conflict_group_t *cg_ptr;
    conflict_group_t *cg_check_ptr;
    conflict_group_t *cg_prv_check_ptr;
    pattern_t *pt_ptr;
    pattern_t *pt_check_ptr;
    bool identical;

    DBUG_ENTER ();

    DBUG_PRINT ("  removing conflict groups with identical access patterns...");
    APprintDiag ("  removing conflict groups with identical access patterns...\n");

    /* for every array type... */
    at_ptr = array_type;
    while (at_ptr != NULL) {

        /* check all conflict groups... */
        cg_ptr = AT_GROUPS (at_ptr);
        while (cg_ptr != NULL) {

            /* for existence of identical conflict group */

            /* NOTE: We may check here CG_DIR to handle read|write accesses differently */
            cg_check_ptr = CG_NEXT (cg_ptr);
            cg_prv_check_ptr = cg_ptr;
            while (cg_check_ptr != NULL) {

                /* check, if all patterns are matching */

                pt_ptr = CG_PATTERNS (cg_ptr);
                pt_check_ptr = CG_PATTERNS (cg_check_ptr);
                identical = TRUE;

                while (identical && (pt_ptr != NULL) && (pt_check_ptr != NULL)) {

                    if (SHcompareShapes (PT_PATTERN (pt_ptr),
                                         PT_PATTERN (pt_check_ptr))) {
                        pt_ptr = PT_NEXT (pt_ptr);
                        pt_check_ptr = PT_NEXT (pt_check_ptr);
                    } else {
                        identical = FALSE;
                    }

                    if (((pt_ptr == NULL) && (pt_check_ptr != NULL))
                        || ((pt_ptr != NULL) && (pt_check_ptr == NULL))) {
                        /* unequal length of pattern list */
                        identical = FALSE;
                    }
                }

                /* remove conflict group, if all access patterns are identical */
                if (identical) {

                    DBUG_ASSERT (cg_prv_check_ptr != NULL,
                                 "NULL pointer to conflict group!");

                    pt_ptr = CG_PATTERNS (cg_check_ptr);
                    while (pt_ptr != NULL) {
                        pt_ptr = RemovePatternElement (pt_ptr);
                    }
                    CG_NEXT (cg_prv_check_ptr)
                      = RemoveConflictGroupElement (cg_check_ptr);
                    cg_check_ptr = CG_NEXT (cg_prv_check_ptr);
                } else {
                    cg_prv_check_ptr = cg_check_ptr;
                    cg_check_ptr = CG_NEXT (cg_check_ptr);
                }
            }

            cg_ptr = CG_NEXT (cg_ptr);
        }

        at_ptr = AT_NEXT (at_ptr);
    }

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   void PIprintShpSeg(int dim, shape* shp)
 *
 * description:
 *   print shape to apdiag-file
 *
 *****************************************************************************/

void
PIprintShpSeg (int dim, shape *shp)
{

    int i;

    DBUG_ENTER ();

    DBUG_ASSERT (dim <= SHP_SEG_SIZE, " dimension out of range in PrintVect()!");

    APprintDiag ("[");
    for (i = 0; i < dim - 1; i++) {
        APprintDiag ("%3d, ", SHgetExtent (shp, i));
    }
    APprintDiag ("%3d]", SHgetExtent (shp, dim - 1));

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   void PIprintArrayTypeElement(array_type_t* at_ptr)
 *
 * description:
 *   print element of structure array_type to outfile
 *
 *****************************************************************************/

void
PIprintArrayTypeElement (array_type_t *at_ptr)
{

    DBUG_ENTER ();

    APprintDiag ("\tarray type: %s\t%i\t", CVbasetype2String (AT_TYPE (at_ptr)),
                 AT_DIM (at_ptr));
    PIprintShpSeg (AT_DIM (at_ptr), AT_SHAPE (at_ptr));
    APprintDiag ("\n");

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   void PIprintConflictGroupElement(array_type_t* at_ptr, conflict_group_t* cg_ptr)
 *
 * description:
 *   print element of structure conflict_group to outfile
 *   need array_type refering to conflict group to access dimension
 *
 *****************************************************************************/

void
PIprintConflictGroupElement (array_type_t *at_ptr, conflict_group_t *cg_ptr)
{

    DBUG_ENTER ();

    DBUG_ASSERT (((CG_DIR (cg_ptr) == ADIR_read) || (CG_DIR (cg_ptr) == ADIR_write)),
                 "unknown access direction (read|write expected)");
    if (CG_DIR (cg_ptr) == ADIR_read) {
        APprintDiag ("\t\tconflict group: READ\t");
    } else {
        APprintDiag ("\t\tconflict group: WRITE\t");
    }

    PIprintShpSeg (AT_DIM (at_ptr), CG_GROUP (cg_ptr));
    APprintDiag ("\n");

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   void PIprintPatternElement(array_type_t* at_ptr, pattern_t* pt_ptr)
 *
 * description:
 *   print element of structure pattern to outfile
 *   needs array_type refering to pattern to access dimension
 *
 *****************************************************************************/

void
PIprintPatternElement (array_type_t *at_ptr, pattern_t *pt_ptr)
{

    DBUG_ENTER ();

    APprintDiag ("\t\t\t\taccess vector: ");
    PIprintShpSeg (AT_DIM (at_ptr), PT_PATTERN (pt_ptr));
    APprintDiag ("\n");

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   static void PrintUnsupportedShapeElement(unsupported_shape_t* us_ptr)
 *
 * description:
 *   print element of structure unsupported_shape to outfile
 *
 *****************************************************************************/

static void
PrintUnsupportedShapeElement (unsupported_shape_t *us_ptr)
{

    DBUG_ENTER ();

    APprintDiag ("\t%s\t%i\t", CVbasetype2String (US_TYPE (us_ptr)), US_DIM (us_ptr));
    PIprintShpSeg (US_DIM (us_ptr), US_SHAPE (us_ptr));
    APprintDiag ("\n");

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   static void PrintPadInfoElement(pad_info_t* pi_ptr)
 *
 * description:
 *   print element of structure pad_info to outfile
 *
 *****************************************************************************/

static void
PrintPadInfoElement (pad_info_t *pi_ptr)
{

    DBUG_ENTER ();

    APprintDiag ("\t%i\t%s\t", PI_DIM (pi_ptr), CVbasetype2String (PI_TYPE (pi_ptr)));
    PIprintShpSeg (PI_DIM (pi_ptr), PI_OLD_SHAPE (pi_ptr));
    APprintDiag ("\t");
    PIprintShpSeg (PI_DIM (pi_ptr), PI_NEW_SHAPE (pi_ptr));
    APprintDiag ("\t");
    if (PI_FUNDEF_PAD (pi_ptr) != NULL) {
        APprintDiag ("%s\t", FUNDEF_NAME (PI_FUNDEF_PAD (pi_ptr)));
    } else {
        APprintDiag (" - \t");
    }
    if (PI_FUNDEF_UNPAD (pi_ptr) != NULL) {
        APprintDiag ("%s\n", FUNDEF_NAME (PI_FUNDEF_UNPAD (pi_ptr)));
    } else {
        APprintDiag (" - \n");
    }

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   void PIinit()
 *
 * description:
 *   initialize abstract data structure
 *   !!! call this before using any other function !!!
 *
 *****************************************************************************/

void
PIinit ()
{

    DBUG_ENTER ();

    pad_info = NULL;
    array_type = NULL;
    unsupported_shape = NULL;

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   pattern_t* PIconcatPatterns(pattern_t* pattern, shape* shp)
 *
 * description:
 *   concat existing list of patterns (pattern) and new pattern (shape)
 *   attention: shape is set free within pad_info.c !!!
 *
 *****************************************************************************/

pattern_t *
PIconcatPatterns (pattern_t *pattern, shape *shp)
{

    pattern_t *result;

    DBUG_ENTER ();

    result = (pattern_t *)MEMmalloc (sizeof (pattern_t));
    PT_PATTERN (result) = shp;
    PT_NEXT (result) = pattern;

    DBUG_RETURN (result);
}

/*****************************************************************************
 *
 * function:
 *   void PIaddAccessPattern(simpletype type, int dim, shape* shp, shape* group,
 *accessdir_t direction, pattern_t* patterns)
 *
 * description:
 *   add a new entry to access_pattern
 *   patterns have to belong to one group only (specified by 'group')
 *   attention: arguments are set free within pad_info.c !!!
 *
 *****************************************************************************/

void
PIaddAccessPattern (simpletype type, int dim, shape *shp, shape *group,
                    accessdir_t direction, pattern_t *patterns)
{

    array_type_t *at_ptr;
    array_type_t *at_next_ptr;
    conflict_group_t *cg_ptr;
    conflict_group_t *cg_next_ptr;

    DBUG_ENTER ();

    DBUG_ASSERT (patterns != NULL, " unexpected empty access pattern!");

    /* check existence of array type */
    at_ptr = GetArrayTypeEntry (type, dim, shp);

    /* add new type */
    if (at_ptr == NULL) {
        at_next_ptr = array_type;
        array_type = (array_type_t *)MEMmalloc (sizeof (array_type_t));
        AT_DIM (array_type) = dim;
        AT_TYPE (array_type) = type;
        AT_SHAPE (array_type) = shp;
        AT_GROUPS (array_type) = NULL;
        AT_NEXT (array_type) = at_next_ptr;
        at_ptr = array_type;
    } else {
        SHfreeShape (shp);
    }

    /* add new conflict group with patterns to type */
    cg_next_ptr = AT_GROUPS (at_ptr);
    AT_GROUPS (at_ptr) = (conflict_group_t *)MEMmalloc (sizeof (conflict_group_t));
    cg_ptr = AT_GROUPS (at_ptr);
    CG_GROUP (cg_ptr) = group;
    CG_DIR (cg_ptr) = direction;
    CG_PATTERNS (cg_ptr) = patterns;
    CG_NEXT (cg_ptr) = cg_next_ptr;

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   void PIprintAccessPatterns()
 *
 * description:
 *   print complete contents of structure array_type to outfile
 *   (includes print out of sub-structures!)
 *
 *****************************************************************************/

void
PIprintAccessPatterns ()
{

    array_type_t *at_ptr;
    conflict_group_t *cg_ptr;
    pattern_t *pt_ptr;

    DBUG_ENTER ();

    APprintDiag ("\nAccess Patterns:\n");

    at_ptr = array_type;
    while (at_ptr != NULL) {
        PIprintArrayTypeElement (at_ptr);

        cg_ptr = AT_GROUPS (at_ptr);
        while (cg_ptr != NULL) {
            PIprintConflictGroupElement (at_ptr, cg_ptr);

            pt_ptr = CG_PATTERNS (cg_ptr);
            while (pt_ptr != NULL) {
                PIprintPatternElement (at_ptr, pt_ptr);

                pt_ptr = PT_NEXT (pt_ptr);
            }

            cg_ptr = CG_NEXT (cg_ptr);
        }

        at_ptr = AT_NEXT (at_ptr);
    }

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   bool PIaddUnsupportedShape(ntype * array_type)
 *
 * description:
 *   add a new entry to unsupported_shape, if no entry for specified type, class
 *   and pattern does not exist already
 *   attention: types are set free within pad_info.c !!!
 *   returns TRUE, if shape was added, FALSE, if shape already existed
 *
 *****************************************************************************/

bool
PIaddUnsupportedShape (ntype  *array_type)
{

    unsupported_shape_t *unsupported_shape_ptr;
    unsupported_shape_t *us_next_ptr;
    bool added;

    DBUG_ENTER ();

    /* check, if entry for array_type and class already exists */
    unsupported_shape_ptr
      = GetUnsupportedShapeEntry (TUgetSimpleImplementationType (array_type),
                                  TUgetFullDimEncoding (array_type),
                                  TYgetShape (array_type));

    if (unsupported_shape_ptr == NULL) {
        /* create new entry */
        us_next_ptr = unsupported_shape;
        unsupported_shape
          = (unsupported_shape_t *)MEMmalloc (sizeof (unsupported_shape_t));
        US_DIM (unsupported_shape) = TUgetFullDimEncoding (array_type);
        US_TYPE (unsupported_shape) = TUgetSimpleImplementationType (array_type);
        US_SHAPE (unsupported_shape) = TYgetShape (array_type);
        US_NEXT (unsupported_shape) = us_next_ptr;
        added = TRUE;
    } else {
        added = FALSE;
    }

    DBUG_RETURN (added);
}

/*****************************************************************************
 *
 * function:
 *   bool PIisUnsupportedShape(ntype* array_type)
 *
 * description:
 *   check, if specified type already exists in list of unsupported shapes
 *
 *****************************************************************************/

bool
PIisUnsupportedShape (ntype *array_type)
{

    unsupported_shape_t *unsupported_shape_ptr;
    bool is_unsupported;

    DBUG_ENTER ();

    unsupported_shape_ptr
      = GetUnsupportedShapeEntry (TUgetSimpleImplementationType (array_type),
                                  TUgetFullDimEncoding (array_type),
                                  TYgetShape (array_type));
    if (unsupported_shape_ptr == NULL) {
        is_unsupported = FALSE;
    } else {
        is_unsupported = TRUE;
    }

    DBUG_RETURN (is_unsupported);
}

/*****************************************************************************
 *
 * function:
 *   void PIprintUnsupportedShapes()
 *
 * description:
 *   print complete contents of structure unsupported_shape to outfile
 *
 *****************************************************************************/

void
PIprintUnsupportedShapes ()
{

    unsupported_shape_t *us_ptr;

    DBUG_ENTER ();

    APprintDiag ("\nUnsupported Shapes:\n");

    us_ptr = unsupported_shape;

    while (us_ptr != NULL) {
        PrintUnsupportedShapeElement (us_ptr);
        us_ptr = US_NEXT (us_ptr);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   int PIlinearizeVector (int dim, shape* shp, shape* vect)
 *
 * description
 *
 *   calculate linearized for vect according to an array specified
 *   by dim and shape
 *
 ******************************************************************************/

int
PIlinearizeVector (int dim, shape *shp, shape *vect)
{

    int offset;
    int i;

    DBUG_ENTER ();

    /* Horner scheme */
    offset = SHgetExtent (vect, 0);
    for (i = 1; i < dim; i++) {
        offset *= SHgetExtent (shp, i);
        offset += SHgetExtent (vect, i);
    }

    DBUG_RETURN (offset);
}

/*****************************************************************************
 *
 * function:
 *   void PItidyAccessPattern()
 *
 * description:
 *   removes all information that is not required for inferring new shapes
 *    - sort accesses in lexicographical order
 *    - remove duplicate accesses
 *    - remove conflict groups with only one access pattern
 *    - remove conflict groups with identical access patterns
 *
 *****************************************************************************/

void
PItidyAccessPattern ()
{

    DBUG_ENTER ();

    DBUG_PRINT ("Cleaning up access patterns...");
    APprintDiag ("\nCleaning up access patterns...\n");

    SortAccesses ();

    RemoveDuplicateAccesses ();

    RemoveSingleAccessPatterns ();

    RemoveIdenticalConflictGroups ();

    /* some other procedures may follow:
     *   - possibly sorting the entries for an array type has an impact
     *     on the inferred shape
     */

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   void PIremoveUnsupportedShapes()
 *
 * description:
 *   remove unsupported shapes from list of inferred shapes
 *   This is done _after_ the inference phase, because it can tell us,
 *   which array types should be padded, but are not yet supported.
 *
 *****************************************************************************/

void
PIremoveUnsupportedShapes ()
{

    unsupported_shape_t *us_ptr;
    pad_info_t *pi_ptr;
    pad_info_t *pi_prv_ptr;

    DBUG_ENTER ();

    DBUG_PRINT ("Removing unsupported shapes...");

    APprintDiag ("\nRemoving unsupported shapes...\n");

    /* for all unsupported shapes */
    us_ptr = unsupported_shape;
    while (us_ptr != NULL) {

        /* check existence in pad_info */
        pi_ptr = pad_info;
        pi_prv_ptr = NULL;
        while (pi_ptr != NULL) {
            if ((US_TYPE (us_ptr) == PI_TYPE (pi_ptr))
                && (US_DIM (us_ptr) == PI_DIM (pi_ptr))
                && (SHcompareShapes (US_SHAPE (us_ptr),
                                     PI_OLD_SHAPE (pi_ptr)))) {
                /* unsupported shape found in pad_info */

                PrintPadInfoElement (pi_ptr);

                if (pi_prv_ptr == NULL) {
                    /* remove first element */
                    pad_info = RemovePadInfoElement (pi_ptr);
                } else {
                    /* remove other element */
                    PI_NEXT (pi_prv_ptr) = RemovePadInfoElement (pi_ptr);
                }
                global.optcounters.ap_unsupported++;
            }

            pi_prv_ptr = pi_ptr;
            pi_ptr = PI_NEXT (pi_ptr);
        }
        us_ptr = US_NEXT (us_ptr);
    }

    global.optcounters.ap_padded -= global.optcounters.ap_unsupported;

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   int PIgetArrayTypeDim(array_type_t* at_ptr)
 *
 * description:
 *   return dim of specified array type
 *
 *****************************************************************************/

int
PIgetArrayTypeDim (array_type_t *at_ptr)
{

    DBUG_ENTER ();

    DBUG_ASSERT (at_ptr != NULL, " unexpected NULL pointer!");

    DBUG_RETURN (AT_DIM (at_ptr));
}

/*****************************************************************************
 *
 * function:
 *   shape* PIgetArrayTypeShape(array_type_t* at_ptr)
 *
 * description:
 *   return shape of specified array type
 *
 *****************************************************************************/

shape *
PIgetArrayTypeShape (array_type_t *at_ptr)
{

    DBUG_ENTER ();

    DBUG_ASSERT (at_ptr != NULL, " unexpected NULL pointer!");

    DBUG_RETURN (AT_SHAPE (at_ptr));
}

/*****************************************************************************
 *
 * function:
 *   simepltype PIgetArrayTypeBasetype(array_type_t* at_ptr)
 *
 * description:
 *   return basetype of specified array type
 *
 *****************************************************************************/

simpletype
PIgetArrayTypeBasetype (array_type_t *at_ptr)
{

    DBUG_ENTER ();

    DBUG_ASSERT (at_ptr != NULL, " unexpected NULL pointer!");

    DBUG_RETURN (AT_TYPE (at_ptr));
}

/*****************************************************************************
 *
 * function:
 *   shape* PIgetPatternShape(pattern_t* pt_ptr)
 *
 * description:
 *   return shape of specified pattern
 *
 *****************************************************************************/

shape *
PIgetPatternShape (pattern_t *pt_ptr)
{

    DBUG_ENTER ();

    DBUG_ASSERT (pt_ptr != NULL, " unexpected NULL pointer!");

    DBUG_RETURN (PT_PATTERN (pt_ptr));
}

/*****************************************************************************
 *
 * function:
 *   void PIgetFirstArrayType()
 *
 * description:
 *   return first array type
 *
 *****************************************************************************/

array_type_t *
PIgetFirstArrayType ()
{

    DBUG_ENTER ();

    DBUG_RETURN (array_type);
}

/*****************************************************************************
 *
 * function:
 *   void PIgetNextArrayType(array_type_t* at_ptr)
 *
 * description:
 *   return array type following the specified array type
 *
 *****************************************************************************/

array_type_t *
PIgetNextArrayType (array_type_t *at_ptr)
{

    array_type_t *at_next_ptr;

    DBUG_ENTER ();

    if (at_ptr == NULL) {
        at_next_ptr = NULL;
    } else {
        at_next_ptr = AT_NEXT (at_ptr);
    }

    DBUG_RETURN (at_next_ptr);
}

/*****************************************************************************
 *
 * function:
 *   void PIgetFirstConflictGroup(array_type_t* at_ptr)
 *
 * description:
 *   return array types first conflict group
 *
 *****************************************************************************/

conflict_group_t *
PIgetFirstConflictGroup (array_type_t *at_ptr)
{

    conflict_group_t *cg_ptr;

    DBUG_ENTER ();

    if (at_ptr == NULL) {
        cg_ptr = NULL;
    } else {
        cg_ptr = AT_GROUPS (at_ptr);
    }

    DBUG_RETURN (cg_ptr);
}

/*****************************************************************************
 *
 * function:
 *   void PIgetNextConflictGroup(conflict_group_t* cg_ptr)
 *
 * description:
 *   return conflict group following the specified conflict group
 *
 *****************************************************************************/

conflict_group_t *
PIgetNextConflictGroup (conflict_group_t *cg_ptr)
{

    conflict_group_t *cg_next_ptr;

    DBUG_ENTER ();

    if (cg_ptr == NULL) {
        cg_next_ptr = NULL;
    } else {
        cg_next_ptr = CG_NEXT (cg_ptr);
    }

    DBUG_RETURN (cg_next_ptr);
}

/*****************************************************************************
 *
 * function:
 *   void PIgetFirstPattern(conflict_group_t* cg_ptr)
 *
 * description:
 *   return conflict groups first acces pattern
 *
 *****************************************************************************/

pattern_t *
PIgetFirstPattern (conflict_group_t *cg_ptr)
{

    pattern_t *pt_ptr;

    DBUG_ENTER ();

    if (cg_ptr == NULL) {
        pt_ptr = NULL;
    } else {
        pt_ptr = CG_PATTERNS (cg_ptr);
    }

    DBUG_RETURN (pt_ptr);
}

/*****************************************************************************
 *
 * function:
 *   void PIgetNextPattern(pattern_t* pt_ptr)
 *
 * description:
 *   return access pattern following the specified pattern
 *
 *****************************************************************************/

pattern_t *
PIgetNextPattern (pattern_t *pt_ptr)
{

    pattern_t *pt_next_ptr;

    DBUG_ENTER ();

    if (pt_ptr == NULL) {
        pt_next_ptr = NULL;
    } else {
        pt_next_ptr = PT_NEXT (pt_ptr);
    }

    DBUG_RETURN (pt_next_ptr);
}

/*****************************************************************************
 *
 * function:
 *   void PIaddInferredShape(simpletype type, int dim,
 *                           shape* old_shape, shape* new_shape
 *                           shape *padding)
 *
 * description:
 *   add a new entry to the data structure for a newly inferred type
 *   attention: shape are set free within pad_info.c !!!
 *
 *****************************************************************************/

void
PIaddInferredShape (simpletype type, int dim, shape *old_shape, shape *new_shape,
                    shape *padding)
{
    pad_info_t *tmp;

    DBUG_ENTER ();

    tmp = (pad_info_t *)MEMmalloc (sizeof (pad_info_t));
    PI_DIM (tmp) = dim;
    PI_TYPE (tmp) = type;
    PI_OLD_SHAPE (tmp) = old_shape;
    PI_NEW_SHAPE (tmp) = new_shape;
    PI_PADDING (tmp) = padding;
    PI_FUNDEF_PAD (tmp) = NULL;
    PI_FUNDEF_UNPAD (tmp) = NULL;
    PI_NEXT (tmp) = pad_info;
    pad_info = tmp;

    global.optcounters.ap_padded++;

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   int PIpaddingOverhead(int dim,
 *                         shape* orig_shape,
 *                         shape* padding)
 *
 * description
 *
 *   This function computes the overhead in memory consumption
 *   caused by a specific padding as percentage of the original
 *   array size.
 *
 ******************************************************************************/

int
PIpaddingOverhead (int dim, shape *orig_shape, shape *padding)
{
    int i, overhead;
    unsigned long int orig_size, padding_size;

    DBUG_ENTER ();

    orig_size = 1;
    padding_size = 1;

    for (i = 0; i < dim; i++) {
        orig_size *= SHgetExtent (orig_shape, i);
        padding_size *= (SHgetExtent (orig_shape, i) + SHgetExtent (padding, i));
    }

    if ((padding_size < orig_size) || (orig_size == 0)) {
        /*
         * Probably, a numerical overflow has occurred.
         */
        overhead = global.padding_overhead_limit + 1;
    } else {
        overhead = (int)((padding_size - orig_size) * 100) / orig_size;
        if (overhead * orig_size < (padding_size - orig_size) * 100) {
            overhead++;
        }
    }

    DBUG_RETURN (overhead);
}

/*****************************************************************************
 *
 * function:
 *   void PInoteResults()
 *
 * description:
 *
 *   print padding resume to display
 *
 *****************************************************************************/

void
PInoteResults ()
{
    pad_info_t *pi_ptr;
    char *basetype, *old, *xnew, *pad;
    int overhead;

    DBUG_ENTER ();

    pi_ptr = pad_info;

    while (pi_ptr != NULL) {
        basetype = CVbasetype2String (PI_TYPE (pi_ptr));
        old = SHshape2String (PI_DIM (pi_ptr), PI_OLD_SHAPE (pi_ptr));
        xnew = SHshape2String (PI_DIM (pi_ptr), PI_NEW_SHAPE (pi_ptr));
        pad = SHshape2String (PI_DIM (pi_ptr), PI_PADDING (pi_ptr));
        overhead = PIpaddingOverhead (PI_DIM (pi_ptr), PI_OLD_SHAPE (pi_ptr),
                                      PI_PADDING (pi_ptr));

        CTInote ("%s%s  by  %s", basetype, old, pad);
        CTInote ("   ->  %s%s    <= %d%% overhead", basetype, xnew, overhead);

        old = MEMfree (old);
        xnew = MEMfree (xnew);
        pad = MEMfree (pad);

        pi_ptr = PI_NEXT (pi_ptr);
    }

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   void PIprintPadInfo()
 *
 * description:
 *   print complete contents of structure pad_info to outfile
 *
 *****************************************************************************/

void
PIprintPadInfo ()
{

    pad_info_t *pi_ptr;

    DBUG_ENTER ();

    APprintDiag ("\nInferred Shapes:\n");

    pi_ptr = pad_info;

    while (pi_ptr != NULL) {
        PrintPadInfoElement (pi_ptr);
        pi_ptr = PI_NEXT (pi_ptr);
    }

    DBUG_RETURN ();
}

/*****************************************************************************
 *
 * function:
 *   ntype* PIgetNewType(ntype* old_type)
 *
 * description:
 *   returns type-structure with padded shape
 *   or NULL, if old_type can't be padded
 *   renaming has to be done separately, because only N_arg and N_vardec
 *   store their names in types-structure, N_id and ids-attribute do not!
 *   !!! old_type is set free here !!!
 *
 *****************************************************************************/

ntype *
PIgetNewType (ntype *old_type)
{

    ntype *new_type;
    pad_info_t *table_entry;

    DBUG_ENTER ();

    new_type = NULL;

    table_entry = GetNewTableEntry (old_type);

    if (table_entry != NULL) {
        new_type = TYmakeAKS (TYcopyType (TYgetScalar( old_type)),
                              SHcopyShape (PI_NEW_SHAPE (table_entry)));
        TYfreeType (old_type);
    }

    DBUG_RETURN (new_type);
}

/*****************************************************************************
 *
 * function:
 *   ntype* PIgetOldType(ntype* new_type)
 *
 * description:
 *   returns type-structure with unpadded shape
 *   or NULL, if new_type has no padded shape
 *   renaming has to be done separately, because only N_arg and N_vardec
 *   store their names in types-structure, N_id and ids-attribute do not!
 *   !!! new_type is set free here !!!
 *
 *****************************************************************************/

ntype *
PIgetOldType (ntype *new_type)
{

    ntype *old_type;
    pad_info_t *table_entry;

    DBUG_ENTER ();

    old_type = NULL;

    table_entry = GetOldTableEntry (new_type);

    if (table_entry != NULL) {
        old_type = TYmakeAKS (TYcopyType (TYgetScalar( new_type)),
                              SHcopyShape (PI_OLD_SHAPE (table_entry)));
        TYfreeType (new_type);
    }

    DBUG_RETURN (old_type);
}

/*****************************************************************************
 *
 * function:
 *   node* PIgetFUndefPad(ntype *old_type)
 *
 * description:
 *   return pointer to fundef-node of padding-function
 *
 *****************************************************************************/

node *
PIgetFundefPad (ntype *old_type)
{

    pad_info_t *table_entry;

    DBUG_ENTER ();

    table_entry = GetNewTableEntry (old_type);

    DBUG_RETURN ((table_entry != NULL) ? PI_FUNDEF_PAD (table_entry) : NULL);
}

/*****************************************************************************
 *
 * function:
 *   node* PIgetFundefUnpad(pad_info_t* pi)
 *
 * description:
 *   return pointer to fundef-node of unpadding-function
 *
 *****************************************************************************/

node *
PIgetFundefUnpad (ntype *old_type)
{

    pad_info_t *table_entry;

    DBUG_ENTER ();

    table_entry = GetNewTableEntry (old_type);

    DBUG_RETURN ((table_entry != NULL) ? PI_FUNDEF_UNPAD (table_entry) : NULL);
}

/*****************************************************************************
 *
 * function:
 *   void PIfree()
 *
 * description:
 *   free abstract data structures 'array_type', 'unsupported_shape' and 'pad_info'
 *   call this after having finished all work !!!
 *
 *****************************************************************************/

void
PIfree ()
{

    pad_info_t *current_pi;
    array_type_t *current_at;
    conflict_group_t *current_cg;
    pattern_t *current_pt;
    unsupported_shape_t *current_us;

    DBUG_ENTER ();

    /* free pad_info structure */
    current_pi = pad_info;
    while (current_pi != NULL) {
        current_pi = RemovePadInfoElement (current_pi);
    }
    pad_info = NULL;

    /* free array_type and substructures */
    current_at = array_type;
    while (current_at != NULL) {
        current_cg = AT_GROUPS (current_at);
        while (current_cg != NULL) {
            current_pt = CG_PATTERNS (current_cg);
            while (current_pt != NULL) {
                current_pt = RemovePatternElement (current_pt);
            }
            current_cg = RemoveConflictGroupElement (current_cg);
        }
        current_at = RemoveArrayTypeElement (current_at);
    }
    array_type = NULL;

    /* free unsupportes_shape structure */
    current_us = unsupported_shape;
    while (current_us != NULL) {
        current_us = RemoveUnsupportedShapeElement (current_us);
    }
    unsupported_shape = NULL;

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
