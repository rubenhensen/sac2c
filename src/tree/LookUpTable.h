/*
 *
 * $Log$
 * Revision 1.1  2000/01/28 12:33:16  dkr
 * Initial revision
 *
 *
 */

#ifndef _sac_LookUpTable_h
#define _sac_LookUpTable_h

typedef void *lut_t;

extern lut_t *GenLUT (void);
extern lut_t *RemoveLUT (lut_t *lut);
extern lut_t *InsertIntoLUT (lut_t *lut, void *old_entry, void *new_entry);
extern void *SearchInLUT (lut_t *lut, void *old_entry);
extern void PrintLUT (FILE *handle, lut_t *lut);

#endif /* _sac_LookUpTable_h */
