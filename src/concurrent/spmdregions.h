/*
 *
 * $Log$
 * Revision 1.6  1998/04/29 20:15:40  dkr
 * *** empty log message ***
 *
 * Revision 1.5  1998/04/24 17:16:53  dkr
 * renamed Spmd...() to SPMD...()
 *
 * Revision 1.4  1998/04/24 12:13:12  dkr
 * removed SpmdInitNcode, SpmdInitNpart
 *
 * Revision 1.3  1998/04/24 01:17:39  dkr
 * added second phase
 * Spmd..() renamed to SpmdInit..()
 *
 * Revision 1.2  1998/04/17 17:50:59  dkr
 * modified header
 *
 * Revision 1.1  1998/04/17 17:22:04  dkr
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_spmdregions_h

#define _sac_spmdregions_h

/* build SPMD-regions */
extern node *SPMDInitAssign (node *arg_node, node *arg_info);

/* optimize SPMD-regions */
/* not yet implemented */

/* lift SPMD-regions */
extern node *SPMDLiftFundef (node *arg_node, node *arg_info);
extern node *SPMDLiftSpmd (node *arg_node, node *arg_info);
extern node *SPMDLiftId (node *arg_node, node *arg_info);

/* build sync-regions */
extern node *SYNCInitAssign (node *arg_node, node *arg_info);

/* optimize sync-regions */
/* not yet implemented */

extern node *SpmdRegions (node *syntax_tree);

#endif /* _sac_spmdregions_h */
