/*
 *
 * $Log$
 * Revision 3.11  2004/08/01 15:51:35  sah
 * changed the type of LUT_t from void*
 * to struct LUT_T *. This change is
 * necessary as the new ast uses no
 * void pointers.
 *
 * Revision 3.10  2002/08/15 18:46:39  dkr
 * functions SearchInLUT_Next?() added
 *
 * Revision 3.9  2002/08/15 11:45:58  dkr
 * - functions ApplyToEach_?() renamed into MapLUT_?()
 * - functions FoldLUT_?() added
 *
 * Revision 3.8  2002/08/13 13:21:49  dkr
 * - !!!! string support modified !!!!
 *   Now, only the compare-data is a string, the associated-data is always
 *   a (void*) pointer!
 * - functions ApplyToEach_?() added.
 *
 * Revision 3.7  2001/05/18 11:39:45  dkr
 * function IsEmptyLUT added
 *
 * Revision 3.6  2001/04/10 09:38:26  dkr
 * DuplicateLUT added
 *
 * Revision 3.5  2001/04/06 15:29:19  dkr
 * function RemoveContentLUT added
 *
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
 *
 *  Look-Up-Table (LUT) for Pointers and Strings
 *  --------------------------------------------
 *
 *  Each entry of a LUT can hold a pair [old, new] where 'old' is either a
 *  pointer (void*) or a string (char*) and 'new' is always a pointer (void*).
 *
 *  To insert a pair [old, new] into the LUT where 'old' represents a pointer
 *  or a string, use the function  InsertIntoLUT_P( lut, old, new)  or
 *  InsertIntoLUT_S( lut, old, new)  respectively.
 *  Note here, that it is possible to put doubles (pairs with identical
 *  compare-data) into the LUT, e.g. pairs (a,b), (a,c).
 *  If doubles are not welcome, it may be better to use the function
 *  UpdateLUT_P() instead.
 *
 *  The function  UpdateLUT_?( lut, old, new, &act_new)  checks whether there
 *  is already a pair [old, act_new] in the LUT. If so, 'act_new' is replaced
 *  by 'new', otherwise  InsertIntoLUT_?()  is called to insert the pair into
 *  the LUT.
 *
 *  The function  SearchInLUT_?( lut, old)  searches the LUT for an entry
 *  [old, new]. If the LUT contains such an entry, a pointer to the associated
 *  data 'new' is returned. Otherwise the return value equals NULL.
 *  Note here, that always the *first* matching entry is returned. If the LUT
 *  contains multiple matching entries, use  SearchInLUT_Next?()  to get
 *  the next match.
 *  For example, let the LUT contain pairs (a,b) and (a,c). When looking-up 'a'
 *  now, the return value is 'b' for the first, 'c' for the second, and NULL
 *  for all subsequent look-ups.
 *  SearchInLUT_P()  searches for a pointer (pointer compare),
 *  SearchInLUT_S()  searches for a string (string compare).
 *
 *  If both values of the pairs [old, new] have the same type (both strings or
 *  both pointers), you may want to use the function  SearchInLUT_SS( lut, old)
 *  or  SearchInLUT_PP( lut, old)  instead.
 *  If the LUT contains an entry [old, new] the associated data 'new' itself
 *  is returned (not a pointer to it). Otherwise the return value equals 'old'
 *  (not NULL).
 *  SearchInLUT_PP()  searches for a pointer (pointer compare) and expects
 *                    to find a pointer,
 *  SearchInLUT_SS()  searches for a string (string compare) and expects
 *                    to find a string.
 *
 *  You can map a function 'fun' to all associated data present in the LUT
 *  by means of  MapLUT( lut, fun). Let  ass_t  be the type of the associated
 *  data in the LUT, then 'fun' should have the signature  ass_t -> ass_t  .
 *  Moreover, the associated data can be folded by using the function
 *  FoldLUT( lut, init, fun)  , where 'init' is the initial value for the
 *  fold operation. Let again  ass_t  be the type of the associated data in
 *  the LUT and let  init_t  be the type of 'init'. Then 'fun' should have
 *  the signature  ( init_t , ass_t ) -> init_t  and the return value of
 *  FoldLUT()  has the type  init_t  .
 *
 *  *** CAUTION ***
 *  - InsertIntoLUT_S()  copies the compare-string ('old') before inserting
 *    it into the LUT. But the associated data is never copied!!
 *    If the associated data is also a string, you may want to duplicate it
 *    with  StringCopy()  first.
 *  - RemoveLUT()  removes all the stored compare-strings from heap memory.
 *  - SearchInLUT_?()  returns a *pointer* to the found associated data. Thus,
 *    the returned pointer will be undefined if  RemoveLUT()  has been called.
 *    Therefore you should not forget to duplicate the data first ... :-/
 *  - The support for multiple entries with identical compare-data is
 *    implemented rather inefficiently :-(
 *
 */

#ifndef _sac_LookUpTable_h_
#define _sac_LookUpTable_h_

typedef struct LUT_T *LUT_t;

extern LUT_t GenerateLUT (void);
extern LUT_t DuplicateLUT (LUT_t lut);
extern LUT_t RemoveContentLUT (LUT_t lut);
extern LUT_t RemoveLUT (LUT_t lut);

extern bool IsEmptyLUT (LUT_t lut);

extern void PrintLUT (FILE *handle, LUT_t lut);

extern void **SearchInLUT_P (LUT_t lut, void *old_item);
extern void **SearchInLUT_S (LUT_t lut, char *old_item);

extern void **SearchInLUT_NextP (void);
extern void **SearchInLUT_NextS (void);

extern void *SearchInLUT_PP (LUT_t lut, void *old_item);
extern char *SearchInLUT_SS (LUT_t lut, char *old_item);

extern LUT_t InsertIntoLUT_P (LUT_t lut, void *old_item, void *new_item);
extern LUT_t InsertIntoLUT_S (LUT_t lut, char *old_item, void *new_item);

extern LUT_t UpdateLUT_P (LUT_t lut, void *old_item, void *new_item, void **found_item);
extern LUT_t UpdateLUT_S (LUT_t lut, char *old_item, void *new_item, void **found_item);

extern LUT_t MapLUT_S (LUT_t lut, void *(*fun) (void *));
extern LUT_t MapLUT_P (LUT_t lut, void *(*fun) (void *));
extern void *FoldLUT_S (LUT_t lut, void *init, void *(*fun) (void *, void *));
extern void *FoldLUT_P (LUT_t lut, void *init, void *(*fun) (void *, void *));

#endif /* _sac_LookUpTable_h_ */
