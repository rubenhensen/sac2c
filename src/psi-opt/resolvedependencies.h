/*
 *
 * $Log$
 * Revision 1.1  2004/10/20 08:22:54  khf
 * Initial revision
 *
 *
 *
 */

#ifndef _resolvedependencies_h
#define _resolvedependencies_h

extern node *ResolveDependencies (node *assigns, node *cexprs, node *withid,
                                  node *fusionable_wl);

extern node *RDEPENDassign (node *arg_node, info *arg_info);
extern node *RDEPENDprf (node *arg_node, info *arg_info);

#endif
