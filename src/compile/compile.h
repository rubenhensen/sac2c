/*
 *
 * $Log$
 * Revision 3.15  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 3.14  2004/07/17 17:07:16  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.13  2004/03/09 23:52:27  dkrHH
 * file compile.tagged.h renamed into compile.h
 * old backend removed
 *
 * Revision 1.7  2003/09/25 10:53:33  dkr
 * GetFoldCode() added
 *
 * Revision 1.6  2002/08/07 13:49:46  dkr
 * COMPWith added
 *
 * Revision 1.5  2002/06/02 21:39:02  dkr
 * some more stuff for new backend added
 *
 * Revision 1.4  2002/04/03 14:42:56  dkr
 * COMPArg removed
 *
 * Revision 1.3  2001/12/13 11:49:08  dkr
 * some more functions removed
 *
 * Revision 1.2  2001/12/13 11:12:06  dkr
 * some functions removed
 *
 * Revision 1.1  2001/12/10 15:34:16  dkr
 * Initial revision
 *
 */

#ifndef _SAC_COMPILE_H_
#define _SAC_COMPILE_H_

#include "types.h"

extern node *COMPgetFoldCode (node *fundef);

/******************************************************************************
 *
 * Compilation traversal ( comp_tab)
 *
 * Prefix: COMP
 *
 *****************************************************************************/
extern node *COMPdoCompile (node *arg_node);

extern node *COMPap (node *arg_node, info *arg_info);
extern node *COMParray (node *arg_node, info *arg_info);
extern node *COMPassign (node *arg_node, info *arg_info);
extern node *COMPblock (node *arg_node, info *arg_info);
extern node *COMPcast (node *arg_node, info *arg_info);
extern node *COMPcond (node *arg_node, info *arg_info);
extern node *COMPfundef (node *arg_node, info *arg_info);
extern node *COMPicm (node *arg_node, info *arg_info);
extern node *COMPid (node *arg_node, info *arg_info);
extern node *COMPlet (node *arg_node, info *arg_info);
extern node *COMPloop (node *arg_node, info *arg_info);
extern node *COMPmodul (node *arg_node, info *arg_info);
extern node *COMPobjdef (node *arg_node, info *arg_info);
extern node *COMPprf (node *arg_node, info *arg_info);
extern node *COMPreturn (node *arg_node, info *arg_info);
extern node *COMPscalar (node *arg_node, info *arg_info);
extern node *COMPspmd (node *arg_node, info *arg_info);
extern node *COMPsync (node *arg_node, info *arg_info);
extern node *COMPtypedef (node *arg_node, info *arg_info);
extern node *COMPvardec (node *arg_node, info *arg_info);

extern node *COMPwith (node *arg_node, info *arg_info);
extern node *COMPwith2 (node *arg_node, info *arg_info);
extern node *COMPwlsegx (node *arg_node, info *arg_info);
extern node *COMPwlxblock (node *arg_node, info *arg_info);
extern node *COMPwlstridex (node *arg_node, info *arg_info);
extern node *COMPwlgridx (node *arg_node, info *arg_info);
extern node *COMPwlcode (node *arg_node, info *arg_info);

extern node *COMPmt (node *arg_node, info *arg_info);
extern node *COMPst (node *arg_node, info *arg_info);
extern node *COMPmtsignal (node *arg_node, info *arg_info);
extern node *COMPmtalloc (node *arg_node, info *arg_info);
extern node *COMPmtsync (node *arg_node, info *arg_info);

#endif /* _SAC_COMPILE_H_ */
