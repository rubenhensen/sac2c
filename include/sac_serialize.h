/*
 *
 * $Id$
 *
 */

#ifndef _SAC_SERIALIZE_H_
#define _SAC_SERIALIZE_H_

#ifndef NULL
#define NULL ((void *)0)
#endif

extern void *SHLPmakeNode (int ntype, int lineno, char *sfile, ...);
extern void *SHLPfixLink (void *stack, int a, int b, int c);
extern void *COdeserializeConstant (int type, void *shp, int vlen, char *vec);
extern void *SHcreateShape (int dim, ...);
extern void *TYdeserializeType (int con, ...);
extern void *STRcpy (void *s1);
extern void *MEMcopy (int size, void *mem);
extern void *SERbuildSerStack (void *node);
extern void *DSlookupFunction (const char *s, const char *t);
extern void *DSlookupObject (const char *s, const char *t);
extern void *STinit ();
extern void STadd (char *s1, int l, char *s2, int i, void *table);
extern void *STRSadd (char *s1, int i, void *p);
extern void *DSfetchArgAvis (int i);
extern void *NSdeserializeNamespace (int i);
extern void *NSdeserializeView (char *s, int i, void *p);
extern int NSaddMapping (char *s1, void *p);
extern double DShex2Double (char *s);
extern float DShex2Float (char *s);

#endif /* _SAC_SERIALIZE_H_ */
