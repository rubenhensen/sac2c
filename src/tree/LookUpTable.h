/*
 *
 * $Log$
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

#ifndef _sac_LookUpTable_h
#define _sac_LookUpTable_h

typedef void *LUT_t;

extern LUT_t GenerateLUT (void);
extern LUT_t RemoveLUT (LUT_t lut);
extern LUT_t InsertIntoLUT (LUT_t lut, void *old_entry, void *new_entry);
extern void *SearchInLUT (LUT_t *lut, void *old_entry);
extern void PrintLUT (FILE *handle, LUT_t lut);

#endif /* _sac_LookUpTable_h */
