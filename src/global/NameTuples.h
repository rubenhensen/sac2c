/*
 *
 * $Log$
 * Revision 1.1  2000/08/17 10:16:09  dkr
 * Initial revision
 *
 */

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

extern void PrintNT (char *name, types *type);
extern unq_class_t GetUnqFromTypes (types *type);
extern data_class_t GetClassFromTypes (types *type);
