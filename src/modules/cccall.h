/*
 *
 * $Log$
 * Revision 1.4  1996/09/11 06:21:34  cg
 * Converted to new lib-file format.
 * Added facilities for updating makefiles with dependencies
 * and creating libstat information.
 *
 * Revision 1.3  1996/01/05  12:35:35  cg
 * added extern declaration for CreateLibrary
 *
 * Revision 1.2  1996/01/02  07:57:55  cg
 * first working revision
 *
 * Revision 1.1  1995/12/29  17:19:26  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_cccall_h

#define _sac_cccall_h

extern void InvokeCC (node *modul);
extern void CreateLibrary (node *syntax_tree);
extern void PrintLibStat ();
extern void UpdateMakefile ();

extern node *PrepareLinking (node *syntax_tree);

extern node *LINKfundef (node *arg_node, node *arg_info);
extern node *LINKobjdef (node *arg_node, node *arg_info);
extern node *LINKmodul (node *arg_node, node *arg_info);

extern strings *imported_decs;
extern strings *dependencies;

#endif /* _sac_cccall_h */
