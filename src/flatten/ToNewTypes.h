/*
 * $Log$
 * Revision 1.6  2004/12/19 19:32:48  sbs
 * eliminated CheckAvis reminiscents
 * Now, TNT does compute new types from old types only!
 * .
 *
 * Revision 1.5  2004/12/08 17:59:15  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.4  2004/11/24 17:18:19  mwe
 * interface changes
 *
 * Revision 1.3  2004/11/22 12:37:33  ktr
 * Ismop SacDevCamp 04
 * ,.
 *
 * Revision 1.2  2004/11/21 20:10:20  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.1  2004/11/18 14:34:08  mwe
 * Initial revision
 *
 * Revision 1.2  2004/07/16 14:41:34  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.1  2004/01/28 17:00:17  skt
 * Initial revision
 *
 *
 *
 *
 *
 ************ Attention! ************
 * File was moved from ../tree
 * following older Revisions can be found there
 *
 *
 * Revision 1.5  2002/10/16 14:33:20  sbs
 * CAVobjdef added.
 *
 * Revision 1.4  2001/04/24 16:08:12  nmw
 * CheckAvisSingleFundef renamed to CheckAvisOneFunction
 * CheckAvisOneFundef added
 *
 * Revision 1.3  2001/04/18 12:58:47  nmw
 * additional traversal setup function for single fundef traversal added
 *
 * Revision 1.2  2001/02/13 15:16:34  nmw
 * CheckAvis traversal implemented
 *
 * Revision 1.1  2001/02/12 16:58:52  nmw
 * Initial revision
 *
 *
 */

#ifndef _SAC_TONEWTYPES_H_
#define _SAC_TONEWTYPES_H_

#include "types.h"

/******************************************************************************
 *
 * To new types traversal ( tonewtypes_tab)
 *
 * Prefix: TNT
 *
 * description:
 *
 *   This module restores the AVIS attribute in N_id, N_vardec/N_arg
 *   when old code did not updates all references correctly.
 *
 *****************************************************************************/
extern node *TNTdoToNewTypes (node *syntax_tree);
extern node *TNTdoToNewTypesOneFunction (node *fundef);
extern node *TNTdoToNewTypesOneFundef (node *fundef);

extern node *TNTfundef (node *arg_node, info *arg_info);
extern node *TNTarg (node *arg_node, info *arg_info);
extern node *TNTret (node *arg_node, info *arg_info);
extern node *TNTvardec (node *arg_node, info *arg_info);
extern node *TNTblock (node *arg_node, info *arg_info);
extern node *TNTap (node *arg_node, info *arg_info);

#endif /* _SAC_TONEWTYPES_H_ */
