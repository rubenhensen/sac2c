#ifndef _SAC_SERIALIZE_H_
#define _SAC_SERIALIZE_H_

#include <stdlib.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

/* FIXME Why the hell this file is not auto-generated?  */

extern void *SHLPmakeNode (int ntype, char *sfile, size_t lineno, size_t col, ...);
extern void *SHLPfixLink (void *stack, int a, int b, int c);
extern void *COdeserializeConstant (int type, void *shp, size_t vlen, char *vec);
extern void *SHcreateShape (int dim, ...);
extern void *TYdeserializeType (int con, ...);
extern void *STRcpy (const char *s1);
extern void *MEMcopy (size_t size, void *mem);
extern void *SERbuildSerStack (void *node);
extern void *DSlookupFunction (const char *s, const char *t);
extern void *DSlookupObject (const char *s, const char *t);
extern void *STinit (void);
extern void STadd (const char *s1, int l, const char *s2, int i, void *table, unsigned);
extern void *STRSadd (char *s1, int i, void *p);
extern void *DSfetchArgAvis (int i);
extern void *NSdeserializeNamespace (int i);
extern void *NSdeserializeView (char *s, int i, void *p);
extern int NSaddMapping (const char *s1, void *p);
extern double DShex2Double (char *s);
extern float DShex2Float (char *s);

#endif /* _SAC_SERIALIZE_H_ */
