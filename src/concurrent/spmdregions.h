/*
 *
 * $Log$
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

/* init SPMD/sync-regions */
extern node *SpmdInitFundef (node *arg_node, node *arg_info);
extern node *SpmdInitAssign (node *arg_node, node *arg_info);

/* optimize SPMD/sync-regions */
/* not yet implemented */

extern node *SpmdRegions (node *syntax_tree);

#endif /* _sac_spmdregions_h */
