/*
 *
 * $Log$
 * Revision 3.5  2004/11/21 21:28:36  ktr
 * moved some typedefs from NameTuples.h to types.h
 *
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

#ifndef _SAC_NAMETUPLES_H_
#define _SAC_NAMETUPLES_H_

/*
 * These character arrays are the macro-name-parts used to select
 * data class and uniqueness class properties.
 */
extern char *nt_shape_string[];
extern char *nt_hidden_string[];
extern char *nt_unique_string[];

#endif /* _SAC_NAMETUPLES_H_ */
