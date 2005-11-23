/*
 *
 * $Log$
 * Revision 1.11  2005/07/16 15:48:52  sah
 * the CAST is back, yeah!
 *
 * Revision 1.10  2005/05/31 18:13:13  sah
 * even more namespaces are set now...
 *
 * Revision 1.9  2005/04/10 14:35:07  sah
 * now for while/do loops namespaces are annotated correctly
 *
 * Revision 1.8  2005/03/17 14:02:26  sah
 * corrected handling of mops
 *
 * Revision 1.7  2004/12/07 17:01:24  sah
 * fixed withloop handling
 *
 * Revision 1.6  2004/12/05 21:05:20  sah
 * added namespace detection for ids aka global objects
 *
 * Revision 1.5  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 1.4  2004/11/22 16:57:41  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.3  2004/10/22 13:23:51  sah
 * working implementation for fundefs
 *
 * Revision 1.2  2004/10/22 09:02:16  sah
 * added ANSSymbol
 *
 * Revision 1.1  2004/10/22 08:50:04  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_ANNOTATENAMESPACES_H_
#define _SAC_ANNOTATENAMESPACES_H_

#include "types.h"

/******************************************************************************
 *
 * Annotate namespaces traversal ( ans_tab)
 *
 * Prefix: ANS
 *
 *****************************************************************************/
extern node *ANSdoAnnotateNamespace (node *module);

extern node *ANSsymbol (node *arg_node, info *arg_info);
extern node *ANSuse (node *arg_node, info *arg_info);
extern node *ANSimport (node *arg_node, info *arg_info);
extern node *ANSexport (node *arg_node, info *arg_info);
extern node *ANSprovide (node *arg_node, info *arg_info);
extern node *ANSfundef (node *arg_node, info *arg_info);
extern node *ANStypedef (node *arg_node, info *arg_info);
extern node *ANSobjdef (node *arg_node, info *arg_info);
extern node *ANSspap (node *arg_node, info *arg_info);
extern node *ANSwhile (node *arg_node, info *arg_info);
extern node *ANSdo (node *arg_node, info *arg_info);
extern node *ANSavis (node *arg_node, info *arg_info);
extern node *ANSarray (node *arg_node, info *arg_info);
extern node *ANSarg (node *arg_node, info *arg_info);
extern node *ANSret (node *arg_node, info *arg_info);
extern node *ANSspids (node *arg_node, info *arg_info);
extern node *ANSspid (node *arg_node, info *arg_info);
extern node *ANSspmop (node *arg_node, info *arg_info);
extern node *ANSlet (node *arg_node, info *arg_info);
extern node *ANSwith (node *arg_node, info *arg_info);
extern node *ANSfold (node *arg_node, info *arg_info);
extern node *ANSvardec (node *arg_node, info *arg_info);
extern node *ANScast (node *arg_node, info *arg_info);
extern node *ANSmodule (node *arg_node, info *arg_info);

#endif /* _SAC_ANNOTATENAMESPACES_H_ */
