/*
 *
 * $Log$
 * Revision 3.8  2004/08/06 13:02:16  skt
 * added FUNDEF_DATAFLOWGRAPH temporary
 *
 * Revision 3.7  2004/08/06 10:45:41  skt
 * MUTHDecodeExecmode added
 *
 * Revision 3.6  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 3.5  2004/08/05 12:04:55  skt
 * MUTHassugn added & removed some trash
 *
 * Revision 3.4  2004/07/29 00:40:54  skt
 * added support for creation of dataflowgraph (mtmode 3)
 *
 * Revision 3.3  2004/06/23 15:44:03  skt
 * changed MUTH_SINGLE into MUTH_EXCLUSIVE
 * and MUTH_ONCE into MUTH_SINGLE
 *
 * Revision 3.2  2004/06/08 14:40:56  skt
 * definition of execution modes added
 *
 * Revision 3.1  2000/11/20 18:03:11  sacbase
 * new release made
 *
 * Revision 1.2  2000/01/21 14:28:09  jhs
 * Added MUTHmodul and MUTHfundef.
 *
 * Revision 1.1  2000/01/21 13:11:16  jhs
 * Initial revision
 *
 *
 */

/** <!--********************************************************************-->
 *
 * @file multithread.h
 *
 * prefix:      MUTH
 *
 * description:
 *
 *   header file for multithread.c
 *
 *****************************************************************************/

#ifndef MULTITHREAD_H

#define MULTITHREAD_H

/* definition of the execution modes */
#define MUTH_ANY 0
#define MUTH_EXCLUSIVE 1
#define MUTH_SINGLE 2
#define MUTH_MULTI 3

/* definitions for the handling of a dataflowgraph
 *
 *   char*     DFGNAME            (name of the dataflownode; NOT substantial
 *                                 for the functionality, but helpful to print
 *                                 and understand the dataflowgraph)
 *   node*     DFGINNERASSIGNMENT (the corresponding assignment)
 *   node*     DFGOUTERASSIGNMENT (the assignment in the list of assignments,
 *                                 that has to be taken for rearranging the
 *                                 list. Usually the same as INNERASSIGNMENT,
 *                                 but neccessary to handle EX-/ST-/MT-cells
 *                                 see CDFGassign for details)
 *   int       DFGEXECUTIONMODE   (some info about the execution of the
 *                                 corresponding assignment;
 *                                 any-, exclusive-, single- or multithreaded)
 *   nodelist* DFGDEPENDENT       (list of (dataflow-) nodes that depend on
 *                                 this node)
 */
#define INFO_MUTH_DFGNAME(n) ((char *)(n->dfmask[1]))
#define INFO_MUTH_DFGINNERASSIGN(n) (n->node[0])
#define INFO_MUTH_DFGOUTERASSIGN(n) (n->node[1])
#define INFO_MUTH_DFGEXECUTIONMODE(n) (n->flag)
#define INFO_MUTH_DFGNODEREFCOUNT(n) (n->refcnt)
#define INFO_MUTH_DFGDEPENDENT(n) ((nodelist *)(n->dfmask[0]))
#define FUNDEF_DATAFLOWGRAPH(n) ((node *)(n->dfmask[1]))

extern node *BuildMultiThread (node *syntax_tree);

extern node *MUTHmodul (node *arg_node, info *arg_info);

extern node *MUTHfundef (node *arg_node, info *arg_info);

extern node *MUTHassign (node *arg_node, info *arg_info);

extern char *MUTHDecodeExecmode (int execmode);

#endif /* MULTITHREAD_H */
