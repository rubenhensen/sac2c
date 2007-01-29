/*
 *
 * $Log$
 * Revision 1.5  2004/11/25 12:19:31  skt
 * big compiler switch during SACDevCampDK 2k4
 *
 * Revision 1.4  2004/11/22 20:51:06  ktr
 * SacDevCamp 04
 *
 * Revision 1.3  2004/08/01 16:11:32  sah
 * switch to new INFO structure
 * PHASE I
 *
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
