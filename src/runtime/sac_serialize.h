/*
 *
 * $Log$
 * Revision 1.11  2005/02/08 11:13:27  sbs
 * some naming convention errors fixed.
 *
 * Revision 1.10  2004/12/19 18:09:33  sah
 * post dk fixes
 *
 * Revision 1.9  2004/11/17 19:49:06  sah
 * interface changes
 *
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

#ifndef _SAC_SERIALIZE_H_
#define _SAC_SERIALIZE_H_

extern void *SHLPmakeNode (int ntype, int lineno, char *sfile, ...);
extern void *SHLPfixLink (void *stack, int a, int b, int c);
extern void *COdeserializeConstant (int type, void *shp, int vlen, char *vec);
extern void *SHcreateShape (int dim, ...);
extern void *TYdeserializeType (int con, ...);
extern void *ILIBstringCopy (void *s1);
extern void *ILIBmemCopy (int size, void *mem);
extern void *SERbuildSerStack (void *node);
extern void *DSlookupFunction (const char *s, const char *t);
extern void *STinit ();
extern void STadd (char *s1, int l, char *s2, int i, void *table);
extern void *STRSadd (char *s1, int i, void *p);

#endif /* _SAC_SERIALIZE_H_ */
