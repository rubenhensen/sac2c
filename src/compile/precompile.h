/*
 *
 * $Log$
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
 * Revision 1.9  1998/04/02 18:47:08  dkr
 * added PRECconc
 *
 * Revision 1.8  1998/03/03 22:57:38  dkr
 * added PRECncode()
 *
 * Revision 1.7  1998/03/02 22:26:52  dkr
 * added PRECnwith()
 *
 * Revision 1.6  1997/09/05 13:46:04  cg
 * All cast expressions are now removed by rmvoidfun.c. Therefore,
 * the respective attempts in precompile.c and ConstantFolding.c
 * are removed. Cast expressions are only used by the type checker.
 * Afterwards, they are useless, and they are not supported by
 * Constant Folding as well as code generation.
 *
 * Revision 1.5  1997/04/30 11:55:34  cg
 * new function PRECassign added
 *
 * Revision 1.4  1995/12/18  16:31:38  cg
 * declaration of PRECexprs removed
 *
 * Revision 1.3  1995/12/04  17:00:04  cg
 * added function PRECcast
 * All casts are now eliminated by the precompiler
 *
 * Revision 1.2  1995/12/01  20:29:00  cg
 * added declarations of PRECvardec and PRECtypedef
 *
 * Revision 1.1  1995/11/28  12:23:34  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_precompile_h

#define _sac_precompile_h

extern node *precompile (node *syntax_tree);

extern node *PRECmodul (node *arg_node, node *arg_info);
extern node *PRECobjdef (node *arg_node, node *arg_info);
extern node *PRECfundef (node *arg_node, node *arg_info);
extern node *PRECarg (node *arg_node, node *arg_info);
extern node *PREClet (node *arg_node, node *arg_info);
extern node *PRECap (node *arg_node, node *arg_info);
extern node *PRECassign (node *arg_node, node *arg_info);
extern node *PRECreturn (node *arg_node, node *arg_info);
extern node *PRECid (node *arg_node, node *arg_info);
extern node *PRECvardec (node *arg_node, node *arg_info);
extern node *PRECtypedef (node *arg_node, node *arg_info);
extern node *PRECgenerator (node *arg_node, node *arg_info);
extern node *PRECNwithid (node *arg_node, node *arg_info);
extern node *PRECdo (node *arg_node, node *arg_info);
extern node *PRECwhile (node *arg_node, node *arg_info);
extern node *PRECcond (node *arg_node, node *arg_info);
extern node *PRECwith (node *arg_node, node *arg_info);
extern node *PRECNwith2 (node *arg_node, node *arg_info);
extern node *PRECNcode (node *arg_node, node *arg_info);
extern node *PRECsync (node *arg_node, node *arg_info);
extern node *PRECWLseg (node *arg_node, node *arg_info);
extern node *PRECWLsegVar (node *arg_node, node *arg_info);

#ifdef TAGGED_ARRAYS

extern char *PRECRenameLocalIdentifier (char *id, data_class_t d_class,
                                        uniqueness_class_t u_class);

extern uniqueness_class_t GetUniFromTypes (types *typ);
extern data_class_t GetClassFromTypes (types *typ);

#else /* TAGGED_ARRAYS */

extern char *PRECRenameLocalIdentifier (char *id);

#endif /* TAGGED_ARRAYS */

#endif /* _sac_precompile_h */
