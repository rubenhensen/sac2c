/*
 *
 * $Log$
 * Revision 3.4  2001/04/06 14:56:20  dkr
 * minor changes done
 *
 * Revision 3.3  2001/03/23 17:59:34  dkr
 * functions UpdateLUT_? added
 *
 * Revision 3.2  2001/03/22 13:30:39  dkr
 * Support for strings added:
 * InsertIntoLUT renamed into InsertIntoLUT_P,
 * SearchInLUT renamed into SearchInLUT_P,
 * InsertIntoLUT_S, SearchInLUT_S added.
 *
 * Revision 3.1  2000/11/20 18:03:21  sacbase
 * new release made
 *
 * Revision 1.4  2000/03/17 18:31:06  dkr
 * type lut_t* replaced by LUT_t
 *
 * Revision 1.3  2000/02/03 08:35:10  dkr
 * GenLUT renamed to GenerateLUT
 *
 * Revision 1.2  2000/01/31 20:18:48  dkr
 * support for hashing added
 *
 * Revision 1.1  2000/01/28 12:33:16  dkr
 * Initial revision
 *
 */

/*
 *  Look-Up-Table (LUT) for Pointers and Strings
 *  --------------------------------------------
 *
 *  Each entry of a LUT can hold a pair [old, new] of pointers (void*) or
 *  strings (char*).
 *
 *  To insert a pair of pointers or strings into the LUT use the function
 *  InsertIntoLUT_P()  or  InsertIntoLUT_S()  respectively.
 *
 *  The function  SearchInLUT_?( lut, old)  searches the LUT for an entry
 *  [old, new]. If the LUT contains such an entry the associated data
 *  'new' is returned. Otherwise the return value equals 'old'.
 *  SearchInLUT_P()  searches for a pointer (pointer compare),
 *  SearchInLUT_S()  searches for a string (string compare).
 *
 *  *** CAUTION ***
 *  - InsertIntoLUT_S() copies the strings before inserting them into the LUT.
 *  - RemoveLUT() removes all the stored strings from heap memory.
 *  - SearchInLUT_S() returns a pointer to the argument string (if no string
 *    entry of the same name is found in the LUT) or a *pointer* to the found
 *    look-up string. In the latter case the returned pointer will be undefined
 *    if RemoveLUT() has been called.
 *    Therefore you should not forget to use StringCopy() here ... :-/
 */

#ifndef _sac_LookUpTable_h_
#define _sac_LookUpTable_h_

typedef void *LUT_t;

extern LUT_t GenerateLUT (void);
extern LUT_t RemoveLUT (LUT_t lut);

extern void PrintLUT (FILE *handle, LUT_t lut);

extern void *SearchInLUT_P (LUT_t lut, void *old_item);
extern void *SearchInLUT_S (LUT_t lut, char *old_item);

extern LUT_t InsertIntoLUT_P (LUT_t lut, void *old_item, void *new_item);
extern LUT_t InsertIntoLUT_S (LUT_t lut, char *old_item, char *new_item);

extern LUT_t UpdateLUT_P (LUT_t lut, void *old_item, void *new_item, void **found_item);
extern LUT_t UpdateLUT_S (LUT_t lut, char *old_item, char *new_item, char **found_item);

#endif /* _sac_LookUpTable_h_ */
