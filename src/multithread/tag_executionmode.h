/*
 * $Log$
 * Revision 1.12  2004/11/23 20:52:11  skt
 * big compiler brushing during SACDevCampDK 2k4
 *
 * Revision 1.11  2004/11/22 17:59:48  skt
 * code brushing in SACDevCampDK 2004
 *
 * Revision 1.10  2004/08/06 11:14:54  skt
 * DecodeExecmode deleted - it had been moved into multithread.[ch]
 *
 * Revision 1.9  2004/08/05 17:42:19  skt
 * moved TagAllocs into multithread_lib
 *
 * Revision 1.8  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 1.7  2004/07/23 10:05:08  skt
 * TEMfundef added
 *
 * Revision 1.6  2004/07/07 13:17:47  skt
 * parameter of IsGeneratorBigEnough changed from node* to ids*
 *
 * Revision 1.5  2004/07/06 12:37:54  skt
 * TEMreturn removed
 * several functions new implemented
 *
 * Revision 1.4  2004/06/25 09:36:21  skt
 * added TEMlet and some helper functions
 *
 * Revision 1.3  2004/06/23 15:44:47  skt
 * TEMreturn, TEMap & TEMarray added
 *
 * Revision 1.2  2004/06/23 09:42:34  skt
 * TEMprf, TEMexprs & some helper functions added
 *
 * Revision 1.1  2004/06/08 14:16:34  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   tag_executionmode.h
 *
 * description:
 *   header file for tag_executionmode.c
 *
 *****************************************************************************/

#ifndef _SAC_TAG_EXECUTIONMODE_H_
#define _SAC_TAG_EXECUTIONMODE_H_

extern node *TEMdoTagExecutionmode (node *arg_node);

extern node *TEMassign (node *arg_node, info *arg_info);

extern node *TEMwith2 (node *arg_node, info *arg_info);

extern node *TEMprf (node *arg_node, info *arg_info);

extern node *TEMlet (node *arg_node, info *arg_info);

extern node *TEMap (node *arg_node, info *arg_info);

extern node *TEMarray (node *arg_node, info *arg_info);

#endif /* _SAC_TAG_EXECUTIONMODE_H_ */
