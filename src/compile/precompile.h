/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:01:27  sacbase
 * new release made
 *
 * Revision 2.11  2000/10/16 13:43:33  dkr
 * PREC1block added
 *
 * Revision 2.10  2000/10/16 11:19:15  dkr
 * PREC1assign added
 *
 * Revision 2.9  2000/10/09 19:16:04  dkr
 * prototype for AdjustFoldFundef() removed
 *
 * Revision 2.8  2000/08/17 10:12:25  dkr
 * all the NT stuff is now in a separate modul (NameTuples.[ch])
 *
 * Revision 2.7  2000/07/14 14:46:05  nmw
 * PRECObjInitFunctionName added
 *
 * Revision 2.6  2000/07/11 09:02:39  dkr
 * minor changes done
 *
 * Revision 2.5  2000/05/29 14:30:53  dkr
 * functions PREC... renamed into PREC2...
 * PREC1let added
 *
 * Revision 2.4  2000/05/26 19:25:37  dkr
 * signature of AdjustFoldFundef() modified
 *
 * Revision 2.3  2000/05/25 23:03:40  dkr
 * prototype for AdjustFoldFundef added
 *
 * Revision 2.2  1999/06/25 14:52:25  rob
 * Introduce definitions and utility infrastructure for tagged array support.
 *
 * Revision 2.1  1999/02/23 12:42:54  sacbase
 * new release made
 *
 * Revision 1.23  1998/08/07 14:39:45  dkr
 * PRECWLsegVar added
 *
 * Revision 1.22  1998/07/03 10:14:49  cg
 * function PRECspmd removed because attribute INOUT_IDS is no longer needed.
 *
 * Revision 1.21  1998/06/23 12:53:19  cg
 * added traversal function PRECspmd in order to correctly rename
 * the identifiers stored in SPMD_INOUT_IDS.
 *
 * Revision 1.20  1998/06/18 13:44:04  cg
 * file is now able to deal correctly with data objects of
 * the abstract data type for the representation of schedulings.
 *
 * Revision 1.19  1998/06/12 14:06:37  cg
 * core renaming of local identifiers moved to new function
 * PRECRenameLocalIdentifier() which is also exported for
 * usage in other compiler modules.
 *
 * Revision 1.18  1998/06/04 17:00:54  cg
 * information about refcounted variables in the context of loops,
 * conditionals and the old with-loop are now stored in ids-chains
 * instead of N_exprs lists.
 *
 * Revision 1.17  1998/06/03 14:53:41  cg
 * Now, all identifiers including local ones are systematically renamed.
 *
 * Revision 1.16  1998/04/29 17:20:09  dkr
 * with-loop transformation moved to wltransform.[ch]
 *
 * Revision 1.15  1998/04/26 21:49:52  dkr
 * PRECSPMD renamed to PRECSpmd
 *
 * Revision 1.14  1998/04/24 17:14:51  dkr
 * renamed Prec...() to PREC...()
 *
 * Revision 1.13  1998/04/23 17:33:16  dkr
 * added PrecSync
 *
 * Revision 1.12  1998/04/17 17:27:06  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.11  1998/04/17 15:28:57  dkr
 * InsertWLnodes is now external
 *
 * Revision 1.10  1998/04/17 02:14:24  dkr
 * exported GridOffset()
 *
 * [...]
 *
 * Revision 1.1  1995/11/28  12:23:34  cg
 * Initial revision
 *
 */

#ifndef _sac_precompile_h
#define _sac_precompile_h

extern node *Precompile (node *syntax_tree);

extern node *PREC1fundef (node *arg_node, node *arg_info);
extern node *PREC1block (node *arg_node, node *arg_info);
extern node *PREC1assign (node *arg_node, node *arg_info);
extern node *PREC1let (node *arg_node, node *arg_info);

extern node *PREC2modul (node *arg_node, node *arg_info);
extern node *PREC2objdef (node *arg_node, node *arg_info);
extern node *PREC2fundef (node *arg_node, node *arg_info);
extern node *PREC2arg (node *arg_node, node *arg_info);
extern node *PREC2let (node *arg_node, node *arg_info);
extern node *PREC2ap (node *arg_node, node *arg_info);
extern node *PREC2assign (node *arg_node, node *arg_info);
extern node *PREC2return (node *arg_node, node *arg_info);
extern node *PREC2id (node *arg_node, node *arg_info);
extern node *PREC2vardec (node *arg_node, node *arg_info);
extern node *PREC2typedef (node *arg_node, node *arg_info);
extern node *PREC2generator (node *arg_node, node *arg_info);
extern node *PREC2Nwithid (node *arg_node, node *arg_info);
extern node *PREC2do (node *arg_node, node *arg_info);
extern node *PREC2while (node *arg_node, node *arg_info);
extern node *PREC2cond (node *arg_node, node *arg_info);
extern node *PREC2Nwith2 (node *arg_node, node *arg_info);
extern node *PREC2Ncode (node *arg_node, node *arg_info);
extern node *PREC2sync (node *arg_node, node *arg_info);
extern node *PREC2WLseg (node *arg_node, node *arg_info);
extern node *PREC2WLsegVar (node *arg_node, node *arg_info);

extern char *ObjInitFunctionName ();

extern char *RenameLocalIdentifier (char *id);

#endif /* _sac_precompile_h */
