/*
 *
 * $Log$
 * Revision 1.2  2004/10/22 15:38:19  ktr
 * Ongoing implementation.
 *
 * Revision 1.1  2004/10/21 16:18:25  ktr
 * Initial revision
 *
 */
#ifndef _staticreuse_h
#define _staticreuse_h

/******************************************************************************
 *
 * Static reuse traversal
 *
 * Prefix: EMSR
 *
 *****************************************************************************/
extern node *EMSRStaticReuse (node *syntax_tree);

extern node *EMSRprf (node *arg_node, info *arg_info);

#endif
