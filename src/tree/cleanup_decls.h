/*
 *
 * $Log$
 * Revision 1.2  2000/12/15 18:31:35  dkr
 * initial revision
 *
 * Revision 3.1  2000/11/20 17:59:18  sacbase
 * new release made
 *
 * Revision 1.2  2000/03/17 20:44:34  dkr
 * all the traversal functions added
 *
 * Revision 1.1  2000/03/17 15:55:06  dkr
 * Initial revision
 *
 */

#ifndef _sac_cleanup_decls_h
#define _sac_cleanup_decls_h

extern node *CleanupDecls (node *syntaxtree);

extern node *CUDfundef (node *arg_node, node *arg_info);
extern node *CUDblock (node *arg_node, node *arg_info);
extern node *CUDvardec (node *arg_node, node *arg_info);
extern node *CUDlet (node *arg_node, node *arg_info);
extern node *CUDid (node *arg_node, node *arg_info);
extern node *CUDwithid (node *arg_node, node *arg_info);

#endif /* _sac_cleanup_decls_h */
