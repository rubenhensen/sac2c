/*
 *
 * $Log$
 * Revision 1.8  2004/11/14 15:24:04  sah
 * DeserializeLookupFunction now uses internal state
 *
 * Revision 1.7  2004/11/07 18:07:31  sah
 * added two more funtions
 *
 * Revision 1.6  2004/11/04 14:53:43  sah
 * implemented dependencies between modules
 *
 * Revision 1.5  2004/11/02 12:15:08  sah
 * added MemCopy
 *
 * Revision 1.4  2004/10/26 09:34:26  sah
 * interface changes
 *
 * Revision 1.3  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.2  2004/10/19 14:05:17  sah
 * added CreateIds
 *
 * Revision 1.1  2004/10/17 17:01:58  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_SERIALIZE_H
#define _SAC_SERIALIZE_H

extern void *SHLPMakeNode (int ntype, int lineno, char *sfile, ...);
extern void *SHLPFixLink (void *stack, int a, int b, int c);
extern void *CODeserializeConstant (int type, void *shp, int vlen, char *vec);
extern void *SHCreateShape (int dim, ...);
extern void *TYDeserializeType (int con, ...);
extern void *StringCopy (void *s1);
extern void *MemCopy (int size, void *mem);
extern void *CreateIds (char *s1, char *s2, int a, int b, int c, int d, void *p1);
extern void *CreateNums (int s, ...);
extern void *CreateIntegerArray (int s, ...);
extern void *SerializeBuildSerStack (void *node);
extern void *DeserializeLookupFunction (const char *s, const char *t);
extern void *STInit ();
extern void STAdd (char *s1, char *s2, int i, void *table);
extern void *SSAdd (char *s1, int i, void *p);

#endif /* _SAC_SERIALIZE_H */
