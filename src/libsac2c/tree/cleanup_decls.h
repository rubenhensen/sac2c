#ifndef _SAC_CLEANUP_DECLS_H_
#define _SAC_CLEANUP_DECLS_H_

/******************************************************************************
 *
 * Clean up Decls traversal ( cudecls_tab)
 *
 * Prefix: CUD
 *
 *****************************************************************************/
extern node *CUDdoCleanupDecls (node *syntaxtree);

extern node *CUDfundef (node *arg_node, info *arg_info);
extern node *CUDblock (node *arg_node, info *arg_info);
extern node *CUDvardec (node *arg_node, info *arg_info);
extern node *CUDids (node *arg_node, info *arg_info);
extern node *CUDid (node *arg_node, info *arg_info);

#endif /* _SAC_CLEANUP_DECLS_H_ */
