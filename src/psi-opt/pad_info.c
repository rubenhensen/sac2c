/*
 * $Log$
 * Revision 1.7  2000/07/19 12:42:55  mab
 * added data structures for storing access patterns and unsupported shapes
 * added functions for accessing new data structures
 * pad_info.[ch] seems to be complete now (may be it needs some debugging)
 *
 * Revision 1.6  2000/07/06 12:44:25  mab
 * added PIgetOldTypes
 *
 * Revision 1.5  2000/07/05 09:13:10  mab
 * fixed problem with global data structure pad_info
 *
 * Revision 1.4  2000/06/30 15:21:01  mab
 * *** empty log message ***
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

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "DupTree.h"

#include "pad_info.h"

/* access macros for array_type_t */
#define AT_DIM(p) p->dim
#define AT_TYPE(p) p->type
#define AT_SHAPE(p) p->shape
#define AT_GROUPS(p) p->groups
#define AT_NEXT(p) p->next

/* access macros for conflict_group_t */
#define CG_GROUP(p) p->group
#define CG_PATTERNS(p) p->patterns
#define CG_NEXT(p) p->next

/* access macros for pattern_t */
#define PT_PATTERN(p) p->pattern
#define PT_NEXT(p) p->next

/* access macros for unsupported_shapes_t */
#define US_DIM(p) p->dim
#define US_TYPE(p) p->type
#define US_SHAPE(p) p->shape
#define US_NEXT(p) p->next

/* access_macros for pad_info_t */
#define PI_DIM(p) p->dim
#define PI_TYPE(p) p->type
#define PI_OLD_SHAPE(p) p->old_shape
#define PI_NEW_SHAPE(p) p->new_shape
#define PI_FUNDEF_PAD(p) p->fundef_pad
#define PI_FUNDEF_UNPAD(p) p->fundef_unpad
#define PI_NEXT(p) p->next

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

#define TYP_IFpr_str(str) str
static char *ctype_string[] = {
#include "type_info.mac"
};

static pad_info_t *pad_info;
static array_type_t *array_type;
static unsupported_shape_t *unsupported_shape;

/*****************************************************************************
 *
 * function:
 *   static int EqualShapes(int dim, shpseg* shape2, shpseg* shape1)
 *
 * description:
 *   compares two shapes, result is TRUE, if shapes are equal
 *
 *****************************************************************************/

static int
EqualShapes (int dim, shpseg *shape2, shpseg *shape1)
{

    int equalShapes;
    int i;

    DBUG_ENTER ("EqualShapes");

    equalShapes = TRUE;

    i = 0;
    while (i < dim && equalShapes) {
        if (SHPSEG_SHAPE (shape1, i) != SHPSEG_SHAPE (shape2, i)) {
            equalShapes = FALSE;
        }
        i++;
    }

    DBUG_RETURN (equalShapes);
}

/*****************************************************************************
 *
 * function:
 *   static pad_info_t *GetNewTableEntry(types *old_type)
 *
 * description:
 *   returns pointer to pad_info table entry corrsponding to old_type or NULL
 *
 *****************************************************************************/

static pad_info_t *
GetNewTableEntry (types *old_type)
{

    pad_info_t *tmp;
    pad_info_t *matching_entry;

    DBUG_ENTER ("GetNewTableEntry");

    tmp = pad_info;
    matching_entry = NULL;

    while (tmp != NULL) {
        if ((PI_TYPE (tmp) == TYPES_BASETYPE (old_type))
            && (PI_DIM (tmp) == TYPES_DIM (old_type))
            && EqualShapes (PI_DIM (tmp), PI_OLD_SHAPE (tmp), TYPES_SHPSEG (old_type))) {

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
 *   static pad_info_t *GetOldTableEntry(types* new_type)
 *
 * description:
 *   returns pointer to pad_info table entry corrsponding to new_type or NULL
 *
 *****************************************************************************/

static pad_info_t *
GetOldTableEntry (types *new_type)
{

    pad_info_t *tmp;
    pad_info_t *matching_entry;

    DBUG_ENTER ("GetOldTableEntry");

    tmp = pad_info;
    matching_entry = NULL;

    while (tmp != NULL) {
        if ((PI_TYPE (tmp) == TYPES_BASETYPE (new_type))
            && (PI_DIM (tmp) == TYPES_DIM (new_type))
            && EqualShapes (PI_DIM (tmp), PI_NEW_SHAPE (tmp), TYPES_SHPSEG (new_type))) {

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
 *   static array_type_t* GetArrayTypeEntry(int dim, int type, shpseg* shape)
 *
 * description:
 *   get entry from access_pattern with specified type and class
 *   returns NULL, if no matching entry was found
 *
 *****************************************************************************/

static array_type_t *
GetArrayTypeEntry (int dim, int type, shpseg *shape)
{

    array_type_t *array_type_ptr;
    bool matched;

    DBUG_ENTER ("GetArrayTypeEntry");

    array_type_ptr = array_type;
    matched = FALSE;

    while ((array_type_ptr != NULL) && (!matched)) {
        if ((dim == AT_DIM (array_type_ptr)) && (type == AT_TYPE (array_type_ptr))
            && (EqualShapes (dim, shape, AT_SHAPE (array_type_ptr)))) {
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
 *   static unsupported_shape_t* GetUnsupportedShapeEntry(int dim, simpletype type,
 *shpseg* shape)
 *
 * description:
 *   get entry from unsupported_shape with specified type
 *   returns NULL, if no matching entry was found
 *
 *****************************************************************************/

static unsupported_shape_t *
GetUnsupportedShapeEntry (int dim, simpletype type, shpseg *shape)
{

    unsupported_shape_t *unsupported_shape_ptr;
    bool matched;

    DBUG_ENTER ("GetUnsupportedShapeEntry");

    unsupported_shape_ptr = unsupported_shape;
    matched = FALSE;

    while ((unsupported_shape_ptr != NULL) && (!matched)) {
        if ((dim == US_DIM (unsupported_shape_ptr))
            && (type == US_TYPE (unsupported_shape_ptr))
            && (EqualShapes (dim, shape, US_SHAPE (unsupported_shape_ptr)))) {
            matched = TRUE;
        } else {
            unsupported_shape_ptr = US_NEXT (unsupported_shape);
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

    DBUG_ENTER ("RemovePadInfoElement");

    FreeShpseg (PI_OLD_SHAPE (element));
    FreeShpseg (PI_NEW_SHAPE (element));
    pi_next_ptr = PI_NEXT (element);
    FREE (element);

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

    DBUG_ENTER ("RemoveUnsupportedShapeElement");

    FreeShpseg (US_SHAPE (element));
    us_next_ptr = US_NEXT (element);
    FREE (element);

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

    DBUG_ENTER ("RemoveArrayTypeElement");

    FreeShpseg (AT_SHAPE (element));
    at_next_ptr = AT_NEXT (element);
    FREE (element);

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

    DBUG_ENTER ("RemoveConflictGroupElement");

    FreeShpseg (CG_GROUP (element));
    cg_next_ptr = CG_NEXT (element);
    FREE (element);

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

    DBUG_ENTER ("RemovePatternElement");

    FreeShpseg (PT_PATTERN (element));
    pt_next_ptr = PT_NEXT (element);
    FREE (element);

    DBUG_RETURN (pt_next_ptr);
}

/*****************************************************************************
 *
 * function:
 *   static void RemoveUnsupportedShapes()
 *
 * description:
 *   remove shapes from array_type listed in unsupported_shape
 *
 *****************************************************************************/

static void
RemoveUnsupportedShapes ()
{

    array_type_t *at_ptr;
    array_type_t *at_prv_ptr;
    unsupported_shape_t *us_ptr;

    DBUG_ENTER ("RemoveUnsupportedShapes");

    /* for all unsupported shapes */
    us_ptr = unsupported_shape;
    while (us_ptr != NULL) {

        /* check existence in array_type */
        at_ptr = array_type;
        at_prv_ptr = NULL;
        while (at_ptr != NULL) {
            /* compare types */
            if ((AT_DIM (at_ptr) == US_DIM (us_ptr))
                && (AT_TYPE (at_ptr) == US_TYPE (us_ptr))
                && EqualShapes (AT_DIM (at_ptr), AT_SHAPE (at_ptr), US_SHAPE (us_ptr))) {
                /* remove matching entry in array_type */
                if (at_prv_ptr == NULL) {
                    /* remove first element of array_type */
                    array_type = RemoveArrayTypeElement (at_ptr);
                } else {
                    /* remove other element */
                    AT_NEXT (at_prv_ptr) = RemoveArrayTypeElement (at_ptr);
                    at_ptr = AT_NEXT (at_prv_ptr);
                }
            } else {
                at_prv_ptr = AT_NEXT (at_ptr);
                at_ptr = AT_NEXT (at_ptr);
            }
        }
        us_ptr = US_NEXT (us_ptr);
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   static void RemoveSingleAccessPatterns()
 *
 * description:
 *   remove conflict groups containing only one access pattern
 *
 *****************************************************************************/

static void
RemoveSingleAccessPatterns ()
{

    array_type_t *at_ptr;
    array_type_t *at_prv_ptr;
    conflict_group_t *cg_ptr;
    conflict_group_t *cg_prv_ptr;

    DBUG_ENTER ("RemoveSingleAccessPattern");

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
                AT_NEXT (at_prv_ptr) = RemoveArrayTypeElement (at_ptr);
                at_ptr = AT_NEXT (at_prv_ptr);
            } else {
                /* remove other element */
                array_type = RemoveArrayTypeElement (at_ptr);
                at_ptr = array_type;
            }
        } else {
            at_prv_ptr = at_ptr;
            at_ptr = AT_NEXT (at_ptr);
        }
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   static void RemoveIdenticalConflictGroups()
 *
 * description:
 *   remove duplicate conflict groups (groups with identical access patterns)
 *
 *****************************************************************************/

static void
RemoveIdenticalConflictGroups ()
{

    array_type_t *at_ptr;
    conflict_group_t *cg_ptr;
    conflict_group_t *cg_check_ptr;
    conflict_group_t *cg_prv_check_ptr;
    pattern_t *pt_ptr;
    pattern_t *pt_check_ptr;
    bool identical;
    bool match;

    DBUG_ENTER ("RemoveIdenticalConflictGroups");

    /* for every array type... */
    at_ptr = array_type;
    while (at_ptr != NULL) {

        /* check all conflict groups... */
        cg_ptr = AT_GROUPS (at_ptr);
        while (cg_ptr != NULL) {

            /* for existence of identical conflict group */
            cg_check_ptr = CG_NEXT (cg_ptr);
            cg_prv_check_ptr = cg_ptr;
            while (cg_check_ptr != NULL) {

                /* check, if all patterns are matching */
                identical = TRUE;
                pt_ptr = CG_PATTERNS (cg_ptr);
                while ((pt_ptr != NULL) && identical) {

                    match = FALSE;
                    pt_check_ptr = CG_PATTERNS (cg_check_ptr);
                    while ((pt_check_ptr != NULL) && !match) {
                        if (EqualShapes (AT_DIM (at_ptr), PT_PATTERN (pt_ptr),
                                         PT_PATTERN (pt_check_ptr))) {
                            match = TRUE;
                        } else {
                            pt_check_ptr = PT_NEXT (pt_check_ptr);
                        }
                    }

                    if (!match) {
                        identical = FALSE;
                    } else {
                        pt_ptr = PT_NEXT (pt_ptr);
                    }
                }

                /* remove conflict group, if all access patterns are identical */
                if (identical) {

                    DBUG_ASSERT ((cg_prv_check_ptr != NULL),
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

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   static void PrintShpSeg(int dim, shpseg* shape)
 *
 * description:
 *   print shpseg to outfile
 *
 *****************************************************************************/

static void
PrintShpSeg (int dim, shpseg *shape)
{

    int i;

    DBUG_ENTER ("PrintShpSeg");

    fprintf (outfile, "[");
    for (i = 0; i < dim; i++) {
        fprintf (outfile, "%i", SHPSEG_SHAPE (shape, i));
        if ((i + 1) != dim) {
            fprintf (outfile, ",");
        }
    }
    fprintf (outfile, "]");

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   static void PrintArrayTypeElement(array_type_t* at_ptr)
 *
 * description:
 *   print element of structure array_type to outfile
 *
 *****************************************************************************/

static void
PrintArrayTypeElement (array_type_t *at_ptr)
{

    DBUG_ENTER ("PrintArrayTypeElement");

    fprintf (outfile, "\t%i\t%s\t", AT_DIM (at_ptr), ctype_string[AT_TYPE (at_ptr)]);
    PrintShpSeg (AT_DIM (at_ptr), AT_SHAPE (at_ptr));
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   static void PrintConflictGroupElement(array_type_t* at_ptr, conflict_group_t* cg_ptr)
 *
 * description:
 *   print element of structure conflict_group to outfile
 *   need array_type refering to conflict group to access dimension
 *
 *****************************************************************************/

static void
PrintConflictGroupElement (array_type_t *at_ptr, conflict_group_t *cg_ptr)
{

    DBUG_ENTER ("PrintConflictGroupElement");

    fprintf (outfile, "\t\t");
    PrintShpSeg (AT_DIM (at_ptr), CG_GROUP (cg_ptr));
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   static void PrintPatternElement(array_type_t* at_ptr, pattern_t* pt_ptr)
 *
 * description:
 *   print element of structure pattern to outfile
 *   needs array_type refering to pattern to access dimension
 *
 *****************************************************************************/

static void
PrintPatternElement (array_type_t *at_ptr, pattern_t *pt_ptr)
{

    DBUG_ENTER ("PrintPatternElement");

    fprintf (outfile, "\t\t\t");
    PrintShpSeg (AT_DIM (at_ptr), PT_PATTERN (pt_ptr));
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("PrintUnsupportedShapeElement");

    fprintf (outfile, "\t%i\t%s\t", US_DIM (us_ptr), ctype_string[US_TYPE (us_ptr)]);
    PrintShpSeg (US_DIM (us_ptr), US_SHAPE (us_ptr));
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("PrintPadInfoElement");

    fprintf (outfile, "\t%i\t%s\t", PI_DIM (pi_ptr), ctype_string[PI_TYPE (pi_ptr)]);
    PrintShpSeg (PI_DIM (pi_ptr), PI_OLD_SHAPE (pi_ptr));
    fprintf (outfile, "\t");
    PrintShpSeg (PI_DIM (pi_ptr), PI_NEW_SHAPE (pi_ptr));
    fprintf (outfile, "\t");
    if (PI_FUNDEF_PAD (pi_ptr) != NULL) {
        fprintf (outfile, "%s\t", FUNDEF_NAME (PI_FUNDEF_PAD (pi_ptr)));
    } else {
        fprintf (outfile, " - \t");
    }
    if (PI_FUNDEF_UNPAD (pi_ptr) != NULL) {
        fprintf (outfile, "%s\n", FUNDEF_NAME (PI_FUNDEF_UNPAD (pi_ptr)));
    } else {
        fprintf (outfile, " - \n");
    }

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("PIinit");

    pad_info = NULL;

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   pattern_t* PIconcatPatterns(pattern_t* pattern, shpseg* shape)
 *
 * description:
 *   concat existing list of patterns (pattern) and new pattern (shape)
 *   attention: shape is set free within pad_info.c !!!
 *
 *****************************************************************************/

pattern_t *
PIconcatPatterns (pattern_t *pattern, shpseg *shape)
{

    pattern_t *result;

    DBUG_ENTER ("PIconcatPatterns");

    result = (pattern_t *)MALLOC (sizeof (pattern_t));
    PT_PATTERN (result) = shape;
    PT_NEXT (result) = pattern;

    DBUG_RETURN (result);
}

/*****************************************************************************
 *
 * function:
 *   void PIaddAccessPattern(int dim, int type, shpseg* shape, shpseg* group, pattern_t*
 *patterns)
 *
 * description:
 *   add a new entry to access_pattern
 *   patterns have to belong to one group only (specified by 'group')
 *   attention: arguments are set free within pad_info.c !!!
 *
 *****************************************************************************/

void
PIaddAccessPattern (int dim, int type, shpseg *shape, shpseg *group, pattern_t *patterns)
{

    array_type_t *at_ptr;
    array_type_t *at_next_ptr;
    conflict_group_t *cg_ptr;
    conflict_group_t *cg_next_ptr;

    DBUG_ENTER ("PIaddAccessPattern");

    DBUG_ASSERT ((patterns != NULL), " unexpected empty access pattern!");

    /* check existence of array type */
    at_ptr = GetArrayTypeEntry (dim, type, shape);

    /* add new type */
    if (at_ptr == NULL) {
        at_next_ptr = array_type;
        array_type = (array_type_t *)MALLOC (sizeof (array_type_t));
        AT_DIM (array_type) = dim;
        AT_TYPE (array_type) = type;
        AT_SHAPE (array_type) = shape;
        AT_GROUPS (array_type) = NULL;
        AT_NEXT (array_type) = at_next_ptr;
        at_ptr = array_type;
    } else {
        FreeShpseg (shape);
    }

    /* add new conflict group with patterns to type */
    cg_next_ptr = AT_GROUPS (at_ptr);
    AT_GROUPS (at_ptr) = (conflict_group_t *)MALLOC (sizeof (conflict_group_t));
    cg_ptr = AT_GROUPS (at_ptr);
    CG_GROUP (cg_ptr) = group;
    CG_PATTERNS (cg_ptr) = patterns;
    CG_NEXT (cg_ptr) = cg_next_ptr;

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   void PIPrintAccessPatterns()
 *
 * description:
 *   print complete contents of structure array_type to outfile
 *   (includes print out of sub-structures!)
 *
 *****************************************************************************/

void
PIPrintAccessPatterns ()
{

    array_type_t *at_ptr;
    conflict_group_t *cg_ptr;
    pattern_t *pt_ptr;

    DBUG_ENTER ("PIPrintAccessPatterns");

    at_ptr = array_type;
    while (at_ptr != NULL) {
        PrintArrayTypeElement (at_ptr);

        cg_ptr = AT_GROUPS (at_ptr);
        while (cg_ptr != NULL) {
            PrintConflictGroupElement (at_ptr, cg_ptr);

            pt_ptr = CG_PATTERNS (cg_ptr);
            while (pt_ptr != NULL) {
                PrintPatternElement (at_ptr, pt_ptr);

                pt_ptr = PT_NEXT (pt_ptr);
            }

            cg_ptr = CG_NEXT (cg_ptr);
        }

        at_ptr = AT_NEXT (at_ptr);
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   void PIaddUnsupportedShape(types* array_type)
 *
 * description:
 *   add a new entry to unsupported_shape, if no entry for specified type, class
 *   and pattern does not exist already
 *   attention: types and are set free within pad_info.c !!!
 *
 *****************************************************************************/

void
PIaddUnsupportedShape (types *array_type)
{

    unsupported_shape_t *unsupported_shape_ptr;
    unsupported_shape_t *us_next_ptr;

    DBUG_ENTER ("PIaddUnsupportedShape");

    /* check, entry for array_type and class already exists */
    unsupported_shape_ptr
      = GetUnsupportedShapeEntry (TYPES_DIM (array_type), TYPES_BASETYPE (array_type),
                                  TYPES_SHPSEG (array_type));

    if (unsupported_shape_ptr == NULL) {
        /* create new entry */
        us_next_ptr = unsupported_shape;
        unsupported_shape = (unsupported_shape_t *)MALLOC (sizeof (unsupported_shape_t));
        US_DIM (unsupported_shape) = TYPES_DIM (array_type);
        US_TYPE (unsupported_shape) = TYPES_BASETYPE (array_type);
        US_SHAPE (unsupported_shape) = TYPES_SHPSEG (array_type);
        US_NEXT (unsupported_shape) = us_next_ptr;
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   void PIPrintUnsupportedShapes()
 *
 * description:
 *   print complete contents of structure unsupported_shape to outfile
 *
 *****************************************************************************/

void
PIPrintUnsupportedShapes ()
{

    unsupported_shape_t *us_ptr;

    DBUG_ENTER ("PIPrintUnsupportedTypes");

    us_ptr = unsupported_shape;

    while (us_ptr != NULL) {
        PrintUnsupportedShapeElement (us_ptr);
        us_ptr = US_NEXT (us_ptr);
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   void PItidyAccessPattern()
 *
 * description:
 *   removes all information that is not required for inferring new shapes
 *    - remove shapes from array_type listed in unsupported_shape
 *    - remove conflict groups with only one access pattern
 *    - remove conflict groups with identical access patterns
 *
 *****************************************************************************/

void
PItidyAccessPattern ()
{

    DBUG_ENTER ("PItidyAccessPattern");

    RemoveUnsupportedShapes ();

    RemoveSingleAccessPatterns ();

    RemoveIdenticalConflictGroups ();

    /* some other procedures may follow:
     *   - possibly sorting the entries for an array type has an impact
     *     on the inferred shape
     */

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   void PIGetFirstArrayType()
 *
 * description:
 *   return first array type
 *
 *****************************************************************************/

array_type_t *
PIGetFirstArrayType ()
{

    DBUG_ENTER ("PIGetFirstArrayType");

    DBUG_RETURN (array_type);
}

/*****************************************************************************
 *
 * function:
 *   void PIGetNextArrayType(array_type_t* at_ptr)
 *
 * description:
 *   return array type following the specified array type
 *
 *****************************************************************************/

array_type_t *
PIGetNextArrayType (array_type_t *at_ptr)
{

    array_type_t *at_next_ptr;

    DBUG_ENTER ("PIGetNextArrayType");

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
 *   void PIGetFirstConflictGroup(array_type_t* at_ptr)
 *
 * description:
 *   return array types first conflict group
 *
 *****************************************************************************/

conflict_group_t *
PIGetFirstConflictGroup (array_type_t *at_ptr)
{

    conflict_group_t *cg_ptr;

    DBUG_ENTER ("PIGetFirstConflictGroup");

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
 *   void PIGetNextConflictGroup(conflict_group_t* cg_ptr)
 *
 * description:
 *   return conflict group following the specified conflict group
 *
 *****************************************************************************/

conflict_group_t *
PIGetNextConflictGroup (conflict_group_t *cg_ptr)
{

    conflict_group_t *cg_next_ptr;

    DBUG_ENTER ("PIGetNextConflictGroup");

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
 *   void PIGetFirstPattern(conflict_group_t* cg_ptr)
 *
 * description:
 *   return conflict groups first acces pattern
 *
 *****************************************************************************/

pattern_t *
PIGetFirstPattern (conflict_group_t *cg_ptr)
{

    pattern_t *pt_ptr;

    DBUG_ENTER ("PIGetFirstPattern");

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
 *   void PIGetNextPattern(pattern_t* pt_ptr)
 *
 * description:
 *   return access pattern following the specified pattern
 *
 *****************************************************************************/

pattern_t *
PIGetNextPattern (pattern_t *pt_ptr)
{

    pattern_t *pt_next_ptr;

    DBUG_ENTER ("PIGetNextPattern");

    if (pt_ptr == NULL) {
        pt_next_ptr = NULL;
    } else {
        pt_next_ptr = PT_NEXT (pt_ptr);
    }

    DBUG_RETURN (pt_ptr);
}

/*****************************************************************************
 *
 * function:
 *   void PIaddInferredShape(types* old_type, shpseg* new_shape)
 *
 * description:
 *   add a new entry to the data structure for a newly inferred type
 *   attention: types and shpseg are set free within pad_info.c !!!
 *
 *****************************************************************************/

void
PIaddInferredShape (types *old_type, shpseg *new_shape)
{

    pad_info_t *tmp;

    DBUG_ENTER ("PIaddInferredShape");

    tmp = (pad_info_t *)MALLOC (sizeof (pad_info_t));
    PI_DIM (tmp) = TYPES_DIM (old_type);
    PI_TYPE (tmp) = TYPES_BASETYPE (old_type);
    PI_OLD_SHAPE (tmp) = TYPES_SHPSEG (old_type);
    PI_NEW_SHAPE (tmp) = new_shape;
    PI_FUNDEF_PAD (tmp) = NULL;
    PI_FUNDEF_UNPAD (tmp) = NULL;
    PI_NEXT (tmp) = pad_info;
    pad_info = tmp;

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   void PIPrintPadInfo()
 *
 * description:
 *   print complete contents of structure pad_info to outfile
 *
 *****************************************************************************/

void
PIPrintPadInfo ()
{

    pad_info_t *pi_ptr;

    DBUG_ENTER ("PIPrintPadInfo");

    pi_ptr = pad_info;

    while (pi_ptr != NULL) {
        PrintPadInfoElement (pi_ptr);
        pi_ptr = PI_NEXT (pi_ptr);
    }

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   types* PIgetNewType(types* old_type)
 *
 * description:
 *   returns type-structure with padded shape
 *   or NULL, if old_type can't be padded
 *   renaming has to be done separately, because only N_arg and N_vardec
 *   store their names in types-structure, N_id and ids-attribute do not!
 *   !!! old_type is set free here !!!
 *
 *****************************************************************************/

types *
PIgetNewType (types *old_type)
{

    types *new_type;
    pad_info_t *table_entry;

    DBUG_ENTER ("PIgetNewType");

    new_type = NULL;

    table_entry = GetNewTableEntry (old_type);

    if (table_entry != NULL) {
        new_type = DupTypes (old_type);
        FreeShpseg (TYPES_SHPSEG (new_type));
        TYPES_SHPSEG (new_type) = DupShpSeg (PI_NEW_SHAPE (table_entry));
        FreeOneTypes (old_type);
    }

    DBUG_RETURN (new_type);
}

/*****************************************************************************
 *
 * function:
 *   types* PIgetOldType(types* new_type)
 *
 * description:
 *   returns type-structure with unpadded shape
 *   or NULL, if new_type has no padded shape
 *   renaming has to be done separately, because only N_arg and N_vardec
 *   store their names in types-structure, N_id and ids-attribute do not!
 *   !!! new_type is set free here !!!
 *
 *****************************************************************************/

types *
PIgetOldType (types *new_type)
{

    types *old_type;
    pad_info_t *table_entry;

    DBUG_ENTER ("PIgetOldType");

    old_type = NULL;

    table_entry = GetOldTableEntry (new_type);

    if (table_entry != NULL) {
        old_type = DupTypes (new_type);
        FreeShpseg (TYPES_SHPSEG (old_type));
        TYPES_SHPSEG (old_type) = DupShpSeg (PI_OLD_SHAPE (table_entry));
        FreeOneTypes (new_type);
    }

    DBUG_RETURN (old_type);
}

/*****************************************************************************
 *
 * function:
 *   node* PIgetFUndefPad(types *old_type)
 *
 * description:
 *   return pointer to fundef-node of padding-function
 *
 *****************************************************************************/

node *
PIgetFundefPad (types *old_type)
{

    pad_info_t *table_entry;

    DBUG_ENTER ("PIgetFundefPad");

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
PIgetFundefUnpad (types *old_type)
{

    pad_info_t *table_entry;

    DBUG_ENTER ("PIgetFundefUnpad");

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

    DBUG_ENTER ("PIfree");

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

    DBUG_VOID_RETURN;
}
