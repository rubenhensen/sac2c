/*
 *
 * $Log$
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
 * The following defines indicate the position of tags
 * within name tuples. They should be kept in synch with the
 * NAME_NAME, NAME_CLASS and NAME_UNI macros in sac_std.h
 */
#define NT_NAME_INDEX 0
#define NT_CLASS_INDEX 1
#define NT_UNQ_INDEX 2

/*
 * Enumerated types for data class and uniqueness class
 */

typedef enum {
#define ATTRIB 1
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} data_class_t;

typedef enum {
#define ATTRIB 2
#define NTIFtype(it_type) it_type
#include "nt_info.mac"
} unq_class_t;

/*
 * These character arrays are the macro-name-parts used to select
 * array class and array uniqueness properties.
 */
extern char *nt_class_string[];
extern char *nt_unq_string[];

#endif /* _NameTuples_h_ */
