/*
 *
 * $Log$
 * Revision 3.4  2002/07/31 15:34:49  dkr
 * new hidden tag added
 *
 * Revision 3.3  2002/06/02 21:42:42  dkr
 * symbols renamed
 *
 * Revision 3.2  2002/05/31 17:17:38  dkr
 * functions now in NameTuplesUtils.h
 *
 * Revision 3.1  2000/11/20 17:59:27  sacbase
 * new release made
 *
 * Revision 1.2  2000/08/17 11:10:55  dkr
 * signature of PrintNT changed
 *
 * Revision 1.1  2000/08/17 10:16:09  dkr
 * Initial revision
 *
 */

#ifndef _NameTuples_h_
#define _NameTuples_h_

/*
 * The following defines indicate the position of tags within name tuples.
 * They should be kept in synch with the NT_NAME, NT_SHP, NT_HID and NT_UNQ
 * macros in sac_std.h
 */
#define NT_NAME_INDEX 0
#define NT_SHAPE_INDEX 1
#define NT_HIDDEN_INDEX 2
#define NT_UNIQUE_INDEX 3

/*
 * Enumerated types for data class and uniqueness class
 */

typedef enum {
#define ATTRIB NT_SHAPE_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} shape_class_t;

typedef enum {
#define ATTRIB NT_HIDDEN_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} hidden_class_t;

typedef enum {
#define ATTRIB NT_UNIQUE_INDEX
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} unique_class_t;

/*
 * These character arrays are the macro-name-parts used to select
 * data class and uniqueness class properties.
 */
extern char *nt_shape_string[];
extern char *nt_hidden_string[];
extern char *nt_unique_string[];

#endif /* _NameTuples_h_ */
