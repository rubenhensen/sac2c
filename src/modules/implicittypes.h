/*
 *
 * $Log$
 * Revision 1.2  1995/10/26 16:09:59  cg
 * function SearchImplementation is now exported
 * (used by checkdec.c).
 *
 * Revision 1.1  1995/10/05  16:12:45  cg
 * Initial revision
 *
 *
 */

#ifndef _sac_implicittypes_h
#define _sac_implicittypes_h

extern node *RetrieveImplicitTypeInfo (node *arg_node);
extern node *IMPLmodul (node *arg_node, node *arg_info);
extern types *SearchImplementation (types *type, node *alltypes);

#endif /* _sac_implicittypes_h */
