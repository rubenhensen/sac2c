/*
 * $Log$
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

#ifndef TAG_EXECUTIONMODE_H

#define TAG_EXECUTIONMODE_H

#define TEM_DEBUG 0
#define TEM_TRAVMODE_DEFAULT 0
#define TEM_TRAVMODE_MUSTEX 1
#define TEM_TRAVMODE_MUSTST 2
#define TEM_TRAVMODE_COULDMT 3

extern node *TagExecutionmode (node *arg_node);

extern node *TEMassign (node *arg_node, info *arg_info);

extern node *TEMwith2 (node *arg_node, info *arg_info);

extern node *TEMprf (node *arg_node, info *arg_info);

extern node *TEMlet (node *arg_node, info *arg_info);

extern node *TEMap (node *arg_node, info *arg_info);

extern node *TEMarray (node *arg_node, info *arg_info);

int IsMTAllowed (node *withloop);

int IsGeneratorBigEnough (ids *test_variables);

int IsMTClever (ids *test_variables);

int IsSTClever (ids *test_variables);

int StrongestRestriction (int execmode1, int execmode2);

void TagAllocs (node *wlops /*, node *arg_info*/);

int MustExecuteExclusive (node *assign, info *arg_info);

int CouldExecuteMulti (node *assign, info *arg_info);

int MustExecuteSingle (node *assign, info *arg_info);

int AnyUniqueTypeInThere (ids *letids);

#if TEM_DEBUG
char *DecodeExecmode (int execmode);
#endif
#endif /* TAG_EXECUTIONMODE_H */
