/*
 *
 * $Log$
 * Revision 1.1  2004/10/17 17:01:58  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_SERIALIZE_H
#define _SAC_SERIALIZE_H

extern void *SHLPMakeNode (int ntype, int lineno, char *sfile, ...);
extern void *SHLPLookupFunction (const char *name);
extern void *CODeserializeConstant (int type, void *shp, int vlen, char *vec);
extern void *SHCreateShape (int dim, ...);
extern void *TYDeserializeType (int con, ...);
extern void *MakeTypes1 (int type);
extern void *SymbolTableInit ();
extern void SymbolTableAdd (char *s1, char *s2, int i, void *table);

#endif /* _SAC_SERIALIZE_H */
