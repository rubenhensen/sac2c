/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:42:14  sacbase
 * new release made
 *
 * Revision 1.5  1997/04/24 10:04:25  cg
 * function PrintDependencies moved to import.[ch]
 *
 * Revision 1.4  1997/03/19  13:52:48  cg
 * new function PrintDependencies() which corresponds to -M compiler option
 * (moved from import.c to this place)
 *
 * Revision 1.3  1996/01/05  12:39:04  cg
 * added functions CreateArchive and RSIBobjdef
 *
 * Revision 1.2  1995/12/29  10:41:52  cg
 * first running revision for new SIBs
 *
 * Revision 1.1  1995/12/23  17:28:39  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_readsib_h

#define _sac_readsib_h

extern node *ReadSib (node *);

extern node *RSIBmodul (node *arg_node, node *arg_info);
extern node *RSIBtypedef (node *arg_node, node *arg_info);
extern node *RSIBfundef (node *arg_node, node *arg_info);
extern node *RSIBobjdef (node *arg_node, node *arg_info);

#endif /* _sac_readsib_h  */
