/*
 *
 * $Log$
 * Revision 1.3  2004/07/16 17:36:23  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.2  2002/02/22 09:26:19  sbs
 * INSVDwithid added .
 *
 * Revision 1.1  2002/02/21 15:12:39  sbs
 * Initial revision
 *
 *
 */

#ifndef _sac_insert_vardec_h
#define _sac_insert_vardec_h

extern node *InsertVardec (node *syntaxtree);

extern node *INSVDfundef (node *arg_node, info *arg_info);
extern node *INSVDid (node *arg_node, info *arg_info);
extern node *INSVDwithid (node *arg_node, info *arg_info);
extern node *INSVDlet (node *arg_node, info *arg_info);

#endif /* _sac_insert_vardec_h */
