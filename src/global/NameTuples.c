/*
 *
 * $Log$
 * Revision 3.5  2002/07/31 15:34:43  dkr
 * new hidden tag added
 *
 * Revision 3.4  2002/06/02 21:42:42  dkr
 * symbols renamed
 *
 * Revision 3.3  2002/05/31 17:17:22  dkr
 * functions now in NameTuplesUtils.c
 *
 * Revision 3.2  2001/12/12 15:00:30  dkr
 * minor changes in PrintNT() done
 *
 * Revision 3.1  2000/11/20 17:59:26  sacbase
 * new release made
 *
 * Revision 1.3  2000/09/13 15:05:27  dkr
 * C_last? renamed into C_unknown?
 *
 * Revision 1.2  2000/08/17 11:11:46  dkr
 * signature of PrintNT changed
 *
 * Revision 1.1  2000/08/17 10:16:08  dkr
 * Initial revision
 *
 */

#include "NameTuples.h"

/*
 * These character arrays are the macro-name-parts used to select
 * array class and array uniqueness properties.
 */

char *nt_shape_string[] = {
#define ATTRIB NT_SHAPE_INDEX
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
};

char *nt_hidden_string[] = {
#define ATTRIB NT_HIDDEN_INDEX
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
};

char *nt_unique_string[] = {
#define ATTRIB NT_UNIQUE_INDEX
#define NTIFstr(it_str) it_str
#include "nt_info.mac"
};
